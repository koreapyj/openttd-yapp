/* $Id: road_func.h 24900 2013-01-08 22:46:42Z planetmaker $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file road_func.h Functions related to roads. */

#ifndef ROAD_FUNC_H
#define ROAD_FUNC_H

#include "core/bitmath_func.hpp"
#include "road_type.h"
#include "direction_func.h"
#include "economy_func.h"

/**
 * Iterate through each set RoadType in a RoadTypes value.
 * For more informations see FOR_EACH_SET_BIT_EX.
 *
 * @param var Loop index variable that stores fallowing set road type. Must be of type RoadType.
 * @param road_types The value to iterate through (any expression).
 *
 * @see FOR_EACH_SET_BIT_EX
 */
#define FOR_EACH_SET_ROADTYPE(var, road_types) FOR_EACH_SET_BIT_EX(RoadType, var, RoadTypes, road_types)

/**
 * Whether the given roadtype is valid.
 * @param rt the roadtype to check for validness
 * @return true if and only if valid
 */
static inline bool IsValidRoadType(RoadType rt)
{
	return rt == ROADTYPE_ROAD || rt == ROADTYPE_TRAM;
}

/**
 * Maps a RoadType to the corresponding RoadTypes value
 *
 * @param rt the roadtype to get the roadtypes from
 * @return the roadtypes with the given roadtype
 */
static inline RoadTypes RoadTypeToRoadTypes(RoadType rt)
{
	return (RoadTypes)(1 << rt);
}

/**
 * Returns the RoadTypes which are not present in the given RoadTypes
 *
 * This function returns the complement of a given RoadTypes.
 *
 * @param r The given RoadTypes
 * @return The complement of the given RoadTypes
 */
static inline RoadTypes ComplementRoadTypes(RoadTypes r)
{
	return (RoadTypes)(ROADTYPES_ALL ^ r);
}


/**
 * Calculate the complement of a RoadBits value
 *
 * Simply flips all bits in the RoadBits value to get the complement
 * of the RoadBits.
 *
 * @param r The given RoadBits value
 * @return the complement
 */
static inline RoadBits ComplementRoadBits(RoadBits r)
{
	return (RoadBits)(ROAD_ALL ^ r);
}

/**
 * Calculate rotated RoadBits
 *
 * Move the RoadBits clockwise to their new position.
 *
 * @param r The given RoadBits value
 * @param rot The given rotation angle
 * @return The rotated RoadBits
 */
static inline RoadBits RotateRoadBits(RoadBits r, DiagDirDiff rot)
{
	return (RoadBits)((r | (r << DIAGDIR_END)) >> rot) & ROAD_ALL;
}

/**
 * Calculate the mirrored RoadBits
 *
 * Simply move the bits to their new position.
 *
 * @param r The given RoadBits value
 * @return the mirrored
 */
static inline RoadBits MirrorRoadBits(RoadBits r)
{
	return RotateRoadBits(r, DIAGDIRDIFF_REVERSE);
}

/**
 * Transform RoadBits by given transformation.
 * @param road_bits The RoadBits to transform.
 * @param transformation Transformation to perform.
 * @return The transformed RoadBits.
 */
static inline RoadBits TransformRoadBits(RoadBits road_bits, DirTransformation transformation)
{
	/* reflect agains X axis before rotating */
	if (transformation & DTR_REFLECTION_BIT) {
		/* firstly reflect against W-E axis by swapping odd and even bits (the numbers are bit positions)
		 *
		 * [ROAD_NW] [ROAD_NE]    0   3                            1   2      /N\
		 * -------------------    -----  --reflect-against-W-E-->  -----     W-+-E
		 * [ROAD_SW] [ROAD_SE]    1   2                            0   3      \S/
		 *
		 * bit 0 (ROAD_NW) swaps with bit 1 (ROAD_SW)
		 * bit 2 (ROAD_SE) swaps with bit 3 (ROAD_NE) */
		road_bits = SwapOddEvenBits(road_bits);
		/* Now we have reflection agains W-E axis. To get reflection agains X axis we must rotate the
		 * result left by 90 degree. To do that we can simply subtract 1 from the number of 90-degree
		 * right rotations that we will be doing in the next step. We can safely overflow. */
		transformation = (DirTransformation)(transformation - 1);
	}

	/* rotate */
	return RotateRoadBits(road_bits, (DiagDirDiff)(transformation & DTR_ROTATION_MASK));
}

/**
 * Check if we've got a straight road
 *
 * @param r The given RoadBits
 * @return true if we've got a straight road
 */
static inline bool IsStraightRoad(RoadBits r)
{
	return (r == ROAD_X || r == ROAD_Y);
}

/**
 * Create the road-part which belongs to the given DiagDirection
 *
 * This function returns a RoadBits value which belongs to
 * the given DiagDirection.
 *
 * @param d The DiagDirection
 * @return The result RoadBits which the selected road-part set
 */
static inline RoadBits DiagDirToRoadBits(DiagDirection d)
{
	return (RoadBits)(ROAD_NW << (3 ^ d));
}

/**
 * Create the road-part which belongs to the given Axis
 *
 * This function returns a RoadBits value which belongs to
 * the given Axis.
 *
 * @param a The Axis
 * @return The result RoadBits which the selected road-part set
 */
static inline RoadBits AxisToRoadBits(Axis a)
{
	return a == AXIS_X ? ROAD_X : ROAD_Y;
}


/**
 * Calculates the maintenance cost of a number of road bits.
 * @param roadtype Road type to get the cost for.
 * @param num Number of road bits.
 * @return Total cost.
 */
static inline Money RoadMaintenanceCost(RoadType roadtype, uint32 num)
{
	assert(roadtype < ROADTYPE_END);
	return (_price[PR_INFRASTRUCTURE_ROAD] * (roadtype == ROADTYPE_TRAM ? 3 : 2) * num * (1 + IntSqrt(num))) >> 9; // 2 bits fraction for the multiplier and 7 bits scaling.
}

bool HasRoadTypesAvail(const CompanyID company, const RoadTypes rts);
bool ValParamRoadType(const RoadType rt);
RoadTypes GetCompanyRoadtypes(const CompanyID company);

void UpdateLevelCrossing(TileIndex tile, bool sound = true);

#endif /* ROAD_FUNC_H */