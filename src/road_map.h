/* $Id: road_map.h 23595 2011-12-19 17:48:04Z rubidium $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file road_map.h Map accessors for roads. */

#ifndef ROAD_MAP_H
#define ROAD_MAP_H

#include "track_func.h"
#include "depot_type.h"
#include "rail_type.h"
#include "road_func.h"
#include "tile_map.h"


/** The different types of road tiles. */
enum RoadTileType {
	ROAD_TILE_NORMAL,   ///< Normal road
	ROAD_TILE_CROSSING, ///< Level crossing
	ROAD_TILE_DEPOT,    ///< Depot (one entrance)
};

/**
 * Get the type of the road tile.
 * @param t Tile to query.
 * @pre IsTileType(t, MP_ROAD)
 * @return The road tile type.
 */
template <bool Tgeneric>
static inline RoadTileType GetRoadTileType(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_ROAD));
	return (RoadTileType)GB(GetTile(t)->m5, 6, 2);
}
/** @copydoc GetRoadTileType(TileIndexT<Tgeneric>::T) */
static inline RoadTileType GetRoadTileType(TileIndex t) { return GetRoadTileType<false>(t); }
/** @copydoc GetRoadTileType(TileIndexT<Tgeneric>::T) */
static inline RoadTileType GetRoadTileType(GenericTileIndex t) { return GetRoadTileType<true>(t); }

/**
 * Return whether a tile is a normal road.
 * @param t Tile to query.
 * @pre IsTileType(t, MP_ROAD)
 * @return True if normal road.
 */
template <bool Tgeneric>
static inline bool IsNormalRoad(typename TileIndexT<Tgeneric>::T t)
{
	return GetRoadTileType(t) == ROAD_TILE_NORMAL;
}
/** @copydoc IsNormalRoad(TileIndexT<Tgeneric>::T) */
static inline bool IsNormalRoad(TileIndex t) { return IsNormalRoad<false>(t); }
/** @copydoc IsNormalRoad(TileIndexT<Tgeneric>::T) */
static inline bool IsNormalRoad(GenericTileIndex t) { return IsNormalRoad<true>(t); }

/**
 * Return whether a tile is a normal road tile.
 * @param t Tile to query.
 * @return True if normal road tile.
 */
template <bool Tgeneric>
static inline bool IsNormalRoadTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_ROAD) && IsNormalRoad(t);
}
/** @copydoc IsNormalRoadTile(TileIndexT<Tgeneric>::T) */
static inline bool IsNormalRoadTile(TileIndex t) { return IsNormalRoadTile<false>(t); }
/** @copydoc IsNormalRoadTile(TileIndexT<Tgeneric>::T) */
static inline bool IsNormalRoadTile(GenericTileIndex t) { return IsNormalRoadTile<true>(t); }

/**
 * Return whether a tile is a level crossing.
 * @param t Tile to query.
 * @pre IsTileType(t, MP_ROAD)
 * @return True if level crossing.
 */
template <bool Tgeneric>
static inline bool IsLevelCrossing(typename TileIndexT<Tgeneric>::T t)
{
	return GetRoadTileType(t) == ROAD_TILE_CROSSING;
}
/** @copydoc IsLevelCrossing(TileIndexT<Tgeneric>::T) */
static inline bool IsLevelCrossing(TileIndex t) { return IsLevelCrossing<false>(t); }
/** @copydoc IsLevelCrossing(TileIndexT<Tgeneric>::T) */
static inline bool IsLevelCrossing(GenericTileIndex t) { return IsLevelCrossing<true>(t); }

/**
 * Return whether a tile is a level crossing tile.
 * @param t Tile to query.
 * @return True if level crossing tile.
 */
template <bool Tgeneric>
static inline bool IsLevelCrossingTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_ROAD) && IsLevelCrossing(t);
}
/** @copydoc IsLevelCrossingTile(TileIndexT<Tgeneric>::T) */
static inline bool IsLevelCrossingTile(TileIndex t) { return IsLevelCrossingTile<false>(t); }
/** @copydoc IsLevelCrossingTile(TileIndexT<Tgeneric>::T) */
static inline bool IsLevelCrossingTile(GenericTileIndex t) { return IsLevelCrossingTile<true>(t); }

/**
 * Return whether a tile is a road depot.
 * @param t Tile to query.
 * @pre IsTileType(t, MP_ROAD)
 * @return True if road depot.
 */
template <bool Tgeneric>
static inline bool IsRoadDepot(typename TileIndexT<Tgeneric>::T t)
{
	return GetRoadTileType(t) == ROAD_TILE_DEPOT;
}
/** @copydoc IsRoadDepot(TileIndexT<Tgeneric>::T) */
static inline bool IsRoadDepot(TileIndex t) { return IsRoadDepot<false>(t); }
/** @copydoc IsRoadDepot(TileIndexT<Tgeneric>::T) */
static inline bool IsRoadDepot(GenericTileIndex t) { return IsRoadDepot<true>(t); }

/**
 * Return whether a tile is a road depot tile.
 * @param t Tile to query.
 * @return True if road depot tile.
 */
static inline bool IsRoadDepotTile(TileIndex t)
{
	return IsTileType(t, MP_ROAD) && IsRoadDepot(t);
}

/**
 * Get the present road bits for a specific road type.
 * @param t  The tile to query.
 * @param rt Road type.
 * @pre IsNormalRoad(t)
 * @return The present road bits for the road type.
 */
template <bool Tgeneric>
static inline RoadBits GetRoadBits(typename TileIndexT<Tgeneric>::T t, RoadType rt)
{
	assert(IsNormalRoad(t));
	switch (rt) {
		default: NOT_REACHED();
		case ROADTYPE_ROAD: return (RoadBits)GB(GetTile(t)->m5, 0, 4);
		case ROADTYPE_TRAM: return (RoadBits)GB(GetTile(t)->m3, 0, 4);
	}
}
/** @copydoc GetRoadBits(TileIndexT<Tgeneric>::T,RoadType) */
static inline RoadBits GetRoadBits(TileIndex t, RoadType rt) { return GetRoadBits<false>(t, rt); }
/** @copydoc GetRoadBits(TileIndexT<Tgeneric>::T,RoadType) */
static inline RoadBits GetRoadBits(GenericTileIndex t, RoadType rt) { return GetRoadBits<true>(t, rt); }

/**
 * Get all RoadBits set on a tile except from the given RoadType
 *
 * @param t The tile from which we want to get the RoadBits
 * @param rt The RoadType which we exclude from the querry
 * @return all set RoadBits of the tile which are not from the given RoadType
 */
static inline RoadBits GetOtherRoadBits(TileIndex t, RoadType rt)
{
	return GetRoadBits(t, rt == ROADTYPE_ROAD ? ROADTYPE_TRAM : ROADTYPE_ROAD);
}

/**
 * Get all set RoadBits on the given tile
 *
 * @param tile The tile from which we want to get the RoadBits
 * @return all set RoadBits of the tile
 */
static inline RoadBits GetAllRoadBits(TileIndex tile)
{
	return GetRoadBits(tile, ROADTYPE_ROAD) | GetRoadBits(tile, ROADTYPE_TRAM);
}

/**
 * Set the present road bits for a specific road type.
 * @param t  The tile to change.
 * @param r  The new road bits.
 * @param rt Road type.
 * @pre IsNormalRoad(t)
 */
template <bool Tgeneric>
static inline void SetRoadBits(typename TileIndexT<Tgeneric>::T t, RoadBits r, RoadType rt)
{
	assert(IsNormalRoad(t)); // XXX incomplete
	switch (rt) {
		default: NOT_REACHED();
		case ROADTYPE_ROAD: SB(GetTile(t)->m5, 0, 4, r); break;
		case ROADTYPE_TRAM: SB(GetTile(t)->m3, 0, 4, r); break;
	}
}
/** @copydoc SetRoadBits(TileIndexT<Tgeneric>::T,RoadBits,RoadType) */
static inline void SetRoadBits(TileIndex t, RoadBits r, RoadType rt) { SetRoadBits<false>(t, r, rt); }
/** @copydoc SetRoadBits(TileIndexT<Tgeneric>::T,RoadBits,RoadType) */
static inline void SetRoadBits(GenericTileIndex t, RoadBits r, RoadType rt) { SetRoadBits<true>(t, r, rt); }

/**
 * Get the present road types of a tile.
 * @param t The tile to query.
 * @return Present road types.
 */
template <bool Tgeneric>
static inline RoadTypes GetRoadTypes(typename TileIndexT<Tgeneric>::T t)
{
	return (RoadTypes)GB(GetTileEx(t)->m7, 6, 2);
}
/** @copydoc GetRoadTypes(TileIndexT<Tgeneric>::T) */
static inline RoadTypes GetRoadTypes(TileIndex t) { return GetRoadTypes<false>(t); }
/** @copydoc GetRoadTypes(TileIndexT<Tgeneric>::T) */
static inline RoadTypes GetRoadTypes(GenericTileIndex t) { return GetRoadTypes<true>(t); }

/**
 * Set the present road types of a tile.
 * @param t  The tile to change.
 * @param rt The new road types.
 */
template <bool Tgeneric>
static inline void SetRoadTypes(typename TileIndexT<Tgeneric>::T t, RoadTypes rt)
{
	assert(IsTileType(t, MP_ROAD) || IsTileType(t, MP_STATION) || IsTileType(t, MP_TUNNELBRIDGE));
	SB(GetTileEx(t)->m7, 6, 2, rt);
}
/** @copydoc SetRoadTypes(TileIndexT<Tgeneric>::T,RoadTypes) */
static inline void SetRoadTypes(TileIndex t, RoadTypes rt) { SetRoadTypes<false>(t, rt); }
/** @copydoc SetRoadTypes(TileIndexT<Tgeneric>::T,RoadTypes) */
static inline void SetRoadTypes(GenericTileIndex t, RoadTypes rt) { SetRoadTypes<true>(t, rt); }

/**
 * Check if a tile has a specific road type.
 * @param t  The tile to check.
 * @param rt Road type to check.
 * @return True if the tile has the specified road type.
 */
template <bool Tgeneric>
static inline bool HasTileRoadType(typename TileIndexT<Tgeneric>::T t, RoadType rt)
{
	return HasBit(GetRoadTypes(t), rt);
}
/** @copydoc HasTileRoadType(TileIndexT<Tgeneric>::T,RoadType) */
static inline bool HasTileRoadType(TileIndex t, RoadType rt) { return HasTileRoadType<false>(t, rt); }
/** @copydoc HasTileRoadType(TileIndexT<Tgeneric>::T,RoadType) */
static inline bool HasTileRoadType(GenericTileIndex t, RoadType rt) { return HasTileRoadType<true>(t, rt); }

/**
 * Get the owner of a specific road type.
 * @param t  The tile to query.
 * @param rt The road type to get the owner of.
 * @return Owner of the given road type.
 */
template <bool Tgeneric>
static inline Owner GetRoadOwner(typename TileIndexT<Tgeneric>::T t, RoadType rt)
{
	assert(IsTileType(t, MP_ROAD) || IsTileType(t, MP_STATION) || IsTileType(t, MP_TUNNELBRIDGE));
	switch (rt) {
		default: NOT_REACHED();
		case ROADTYPE_ROAD: return (Owner)GB(IsNormalRoadTile(t) ? GetTile(t)->m1 : GetTileEx(t)->m7, 0, 5);
		case ROADTYPE_TRAM: {
			/* Trams don't need OWNER_TOWN, and remapping OWNER_NONE
			 * to OWNER_TOWN makes it use one bit less */
			Owner o = (Owner)GB(GetTile(t)->m3, 4, 4);
			return o == OWNER_TOWN ? OWNER_NONE : o;
		}
	}
}
/** @copydoc GetRoadOwner(TileIndexT<Tgeneric>::T,RoadType) */
static inline Owner GetRoadOwner(TileIndex t, RoadType rt) { return GetRoadOwner<false>(t, rt); }
/** @copydoc GetRoadOwner(TileIndexT<Tgeneric>::T,RoadType) */
static inline Owner GetRoadOwner(GenericTileIndex t, RoadType rt) { return GetRoadOwner<true>(t, rt); }

/**
 * Set the owner of a specific road type.
 * @param t  The tile to change.
 * @param rt The road type to change the owner of.
 * @param o  New owner of the given road type.
 */
template <bool Tgeneric>
static inline void SetRoadOwner(typename TileIndexT<Tgeneric>::T t, RoadType rt, Owner o)
{
	switch (rt) {
		default: NOT_REACHED();
		case ROADTYPE_ROAD: SB(IsNormalRoadTile(t) ? GetTile(t)->m1 : GetTileEx(t)->m7, 0, 5, o); break;
		case ROADTYPE_TRAM: SB(GetTile(t)->m3, 4, 4, o == OWNER_NONE ? OWNER_TOWN : o); break;
	}
}
/** @copydoc SetRoadOwner(TileIndexT<Tgeneric>::T,RoadType,Owner) */
static inline void SetRoadOwner(TileIndex t, RoadType rt, Owner o) { SetRoadOwner<false>(t, rt, o); }
/** @copydoc SetRoadOwner(TileIndexT<Tgeneric>::T,RoadType,Owner) */
static inline void SetRoadOwner(GenericTileIndex t, RoadType rt, Owner o) { SetRoadOwner<true>(t, rt, o); }

/**
 * Check if a specific road type is owned by an owner.
 * @param t  The tile to query.
 * @param rt The road type to compare the owner of.
 * @param o  Owner to compare with.
 * @pre HasTileRoadType(t, rt)
 * @return True if the road type is owned by the given owner.
 */
template <bool Tgeneric>
static inline bool IsRoadOwner(typename TileIndexT<Tgeneric>::T t, RoadType rt, Owner o)
{
	assert(HasTileRoadType(t, rt));
	return (GetRoadOwner(t, rt) == o);
}
/** @copydoc IsRoadOwner(TileIndexT<Tgeneric>::T,RoadType,Owner) */
static inline bool IsRoadOwner(TileIndex t, RoadType rt, Owner o) { return IsRoadOwner<false>(t, rt, o); }
/** @copydoc IsRoadOwner(TileIndexT<Tgeneric>::T,RoadType,Owner) */
static inline bool IsRoadOwner(GenericTileIndex t, RoadType rt, Owner o) { return IsRoadOwner<true>(t, rt, o); }

/**
 * Checks if given tile has town owned road
 * @param t tile to check
 * @pre IsTileType(t, MP_ROAD)
 * @return true iff tile has road and the road is owned by a town
 */
static inline bool HasTownOwnedRoad(TileIndex t)
{
	return HasTileRoadType(t, ROADTYPE_ROAD) && IsRoadOwner(t, ROADTYPE_ROAD, OWNER_TOWN);
}

static inline void MakeTrafficLights(TileIndex t)
{
	assert(IsTileType(t, MP_ROAD));
	assert(GetRoadTileType(t) == ROAD_TILE_NORMAL);
	SetBit(GetTileEx(t)->m7, 4);
}

static inline void ClearTrafficLights(TileIndex t)
{
	assert(IsTileType(t, MP_ROAD));
	assert(GetRoadTileType(t) == ROAD_TILE_NORMAL);
	ClrBit(GetTileEx(t)->m7, 4);
}

/**
 * Check if a tile has traffic lights returns true if tile has traffic lights.
 * @param t The tile to check.
 */
static inline bool HasTrafficLights(TileIndex t)
{
	return (IsTileType(t, MP_ROAD) && (GetRoadTileType(t) == ROAD_TILE_NORMAL) && HasBit(GetTileEx(t)->m7, 4));
}

/** Which directions are disallowed ? */
enum DisallowedRoadDirections {
	DRD_NONE,       ///< None of the directions are disallowed
	DRD_SOUTHBOUND, ///< All southbound traffic is disallowed
	DRD_NORTHBOUND, ///< All northbound traffic is disallowed
	DRD_BOTH,       ///< All directions are disallowed
	DRD_END,        ///< Sentinel
};
DECLARE_ENUM_AS_BIT_SET(DisallowedRoadDirections)
/** Helper information for extract tool. */
template <> struct EnumPropsT<DisallowedRoadDirections> : MakeEnumPropsT<DisallowedRoadDirections, byte, DRD_NONE, DRD_END, DRD_END, 2> {};

/**
 * Gets the disallowed directions
 * @param t the tile to get the directions from
 * @return the disallowed directions
 */
template <bool Tgeneric>
static inline DisallowedRoadDirections GetDisallowedRoadDirections(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsNormalRoad(t));
	return (DisallowedRoadDirections)GB(GetTile(t)->m5, 4, 2);
}
/** @copydoc GetDisallowedRoadDirections(TileIndexT<Tgeneric>::T) */
static inline DisallowedRoadDirections GetDisallowedRoadDirections(TileIndex t) { return GetDisallowedRoadDirections<false>(t); }
/** @copydoc GetDisallowedRoadDirections(TileIndexT<Tgeneric>::T) */
static inline DisallowedRoadDirections GetDisallowedRoadDirections(GenericTileIndex t) { return GetDisallowedRoadDirections<true>(t); }

/**
 * Sets the disallowed directions
 * @param t   the tile to set the directions for
 * @param drd the disallowed directions
 */
template <bool Tgeneric>
static inline void SetDisallowedRoadDirections(typename TileIndexT<Tgeneric>::T t, DisallowedRoadDirections drd)
{
	assert(IsNormalRoad(t));
	assert(drd < DRD_END);
	SB(GetTile(t)->m5, 4, 2, drd);
}
/** @copydoc SetDisallowedRoadDirections(TileIndexT<Tgeneric>::T,DisallowedRoadDirections) */
static inline void SetDisallowedRoadDirections(TileIndex t, DisallowedRoadDirections drd) { SetDisallowedRoadDirections<false>(t, drd); }
/** @copydoc SetDisallowedRoadDirections(TileIndexT<Tgeneric>::T,DisallowedRoadDirections) */
static inline void SetDisallowedRoadDirections(GenericTileIndex t, DisallowedRoadDirections drd) { SetDisallowedRoadDirections<true>(t, drd); }

/**
 * Get the road axis of a level crossing.
 * @param t The tile to query.
 * @pre IsLevelCrossing(t)
 * @return The axis of the road.
 */
template <bool Tgeneric>
static inline Axis GetCrossingRoadAxis(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsLevelCrossing(t));
	return (Axis)GB(GetTile(t)->m5, 0, 1);
}
/** @copydoc GetCrossingRoadAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetCrossingRoadAxis(TileIndex t) { return GetCrossingRoadAxis<false>(t); }
/** @copydoc GetCrossingRoadAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetCrossingRoadAxis(GenericTileIndex t) { return GetCrossingRoadAxis<true>(t); }

/**
 * Get the rail axis of a level crossing.
 * @param t The tile to query.
 * @pre IsLevelCrossing(t)
 * @return The axis of the rail.
 */
template <bool Tgeneric>
static inline Axis GetCrossingRailAxis(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsLevelCrossing(t));
	return OtherAxis((Axis)GetCrossingRoadAxis(t));
}
/** @copydoc GetCrossingRailAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetCrossingRailAxis(TileIndex t) { return GetCrossingRailAxis<false>(t); }
/** @copydoc GetCrossingRailAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetCrossingRailAxis(GenericTileIndex t) { return GetCrossingRailAxis<true>(t); }

/**
 * Get the road bits of a level crossing.
 * @param tile The tile to query.
 * @return The present road bits.
 */
template <bool Tgeneric>
static inline RoadBits GetCrossingRoadBits(typename TileIndexT<Tgeneric>::T tile)
{
	return GetCrossingRoadAxis(tile) == AXIS_X ? ROAD_X : ROAD_Y;
}
/** @copydoc GetCrossingRoadBits(TileIndexT<Tgeneric>::T) */
static inline RoadBits GetCrossingRoadBits(TileIndex tile) { return GetCrossingRoadBits<false>(tile); }
/** @copydoc GetCrossingRoadBits(TileIndexT<Tgeneric>::T) */
static inline RoadBits GetCrossingRoadBits(GenericTileIndex tile) { return GetCrossingRoadBits<true>(tile); }

/**
 * Get the rail track of a level crossing.
 * @param tile The tile to query.
 * @return The rail track.
 */
static inline Track GetCrossingRailTrack(TileIndex tile)
{
	return AxisToTrack(GetCrossingRailAxis(tile));
}

/**
 * Get the rail track bits of a level crossing.
 * @param tile The tile to query.
 * @return The rail track bits.
 */
template <bool Tgeneric>
static inline TrackBits GetCrossingRailBits(typename TileIndexT<Tgeneric>::T tile)
{
	return AxisToTrackBits(GetCrossingRailAxis(tile));
}
/** @copydoc GetCrossingRailBits(TileIndexT<Tgeneric>::T) */
static inline TrackBits GetCrossingRailBits(TileIndex tile) { return GetCrossingRailBits<false>(tile); }
/** @copydoc GetCrossingRailBits(TileIndexT<Tgeneric>::T) */
static inline TrackBits GetCrossingRailBits(GenericTileIndex tile) { return GetCrossingRailBits<true>(tile); }


/**
 * Get the reservation state of the rail crossing
 * @param t the crossing tile
 * @return reservation state
 * @pre IsLevelCrossingTile(t)
 */
static inline bool HasCrossingReservation(TileIndex t)
{
	assert(IsLevelCrossingTile(t));
	return HasBit(GetTile(t)->m5, 4);
}

/**
 * Set the reservation state of the rail crossing
 * @note Works for both waypoints and rail depots
 * @param t the crossing tile
 * @param b the reservation state
 * @pre IsLevelCrossingTile(t)
 */
static inline void SetCrossingReservation(TileIndex t, bool b)
{
	assert(IsLevelCrossingTile(t));
	SB(GetTile(t)->m5, 4, 1, b ? 1 : 0);
}

/**
 * Get the reserved track bits for a rail crossing
 * @param t the tile
 * @pre IsLevelCrossingTile(t)
 * @return reserved track bits
 */
static inline TrackBits GetCrossingReservationTrackBits(TileIndex t)
{
	return HasCrossingReservation(t) ? GetCrossingRailBits(t) : TRACK_BIT_NONE;
}

/**
 * Check if the level crossing is barred.
 * @param t The tile to query.
 * @pre IsLevelCrossing(t)
 * @return True if the level crossing is barred.
 */
static inline bool IsCrossingBarred(TileIndex t)
{
	assert(IsLevelCrossing(t));
	return HasBit(GetTile(t)->m5, 5);
}

/**
 * Set the bar state of a level crossing.
 * @param t The tile to modify.
 * @param barred True if the crossing should be barred, false otherwise.
 * @pre IsLevelCrossing(t)
 */
static inline void SetCrossingBarred(TileIndex t, bool barred)
{
	assert(IsLevelCrossing(t));
	SB(GetTile(t)->m5, 5, 1, barred ? 1 : 0);
}

/**
 * Unbar a level crossing.
 * @param t The tile to change.
 */
static inline void UnbarCrossing(TileIndex t)
{
	SetCrossingBarred(t, false);
}

/**
 * Bar a level crossing.
 * @param t The tile to change.
 */
static inline void BarCrossing(TileIndex t)
{
	SetCrossingBarred(t, true);
}

/** Check if a road tile has snow/desert. */
#define IsOnDesert IsOnSnow
/**
 * Check if a road tile has snow/desert.
 * @param t The tile to query.
 * @return True if the tile has snow/desert.
 */
static inline bool IsOnSnow(TileIndex t)
{
	return HasBit(GetTileEx(t)->m7, 5);
}

/** Toggle the snow/desert state of a road tile. */
#define ToggleDesert ToggleSnow
/**
 * Toggle the snow/desert state of a road tile.
 * @param t The tile to change.
 */
static inline void ToggleSnow(TileIndex t)
{
	ToggleBit(GetTileEx(t)->m7, 5);
}


/** The possible road side decorations. */
enum Roadside {
	ROADSIDE_BARREN           = 0, ///< Road on barren land
	ROADSIDE_GRASS            = 1, ///< Road on grass
	ROADSIDE_PAVED            = 2, ///< Road with paved sidewalks
	ROADSIDE_STREET_LIGHTS    = 3, ///< Road with street lights on paved sidewalks
	ROADSIDE_TREES            = 5, ///< Road with trees on paved sidewalks
	ROADSIDE_GRASS_ROAD_WORKS = 6, ///< Road on grass with road works
	ROADSIDE_PAVED_ROAD_WORKS = 7, ///< Road with sidewalks and road works
};

/**
 * Get the decorations of a road.
 * @param tile The tile to query.
 * @return The road decoration of the tile.
 */
static inline Roadside GetRoadside(TileIndex tile)
{
	return (Roadside)GB(GetTileEx(tile)->m6, 3, 3);
}

/**
 * Set the decorations of a road.
 * @param tile The tile to change.
 * @param s    The new road decoration of the tile.
 */
static inline void SetRoadside(TileIndex tile, Roadside s)
{
	SB(GetTileEx(tile)->m6, 3, 3, s);
}

/**
 * Check if a tile has road works.
 * @param t The tile to check.
 * @return True if the tile has road works in progress.
 */
static inline bool HasRoadWorks(TileIndex t)
{
	return GetRoadside(t) >= ROADSIDE_GRASS_ROAD_WORKS;
}

/**
 * Increase the progress counter of road works.
 * @param t The tile to modify.
 * @return True if the road works are in the last stage.
 */
static inline bool IncreaseRoadWorksCounter(TileIndex t)
{
        AB(GetTileEx(t)->m7, 0, 4, 1);

       return GB(GetTileEx(t)->m7, 0, 4) == 1;
}

static inline byte GetRoadWorksCounter(TileIndex t)
{
	return GB(GetTile(t)->m3, 0, 4);
}

/**
 * Start road works on a tile.
 * @param t The tile to start the work on.
 * @pre !HasRoadWorks(t)
 */
static inline void StartRoadWorks(TileIndex t)
{
	assert(!HasRoadWorks(t));
	/* Remove any trees or lamps in case or roadwork */
	switch (GetRoadside(t)) {
		case ROADSIDE_BARREN:
		case ROADSIDE_GRASS:  SetRoadside(t, ROADSIDE_GRASS_ROAD_WORKS); break;
		default:              SetRoadside(t, ROADSIDE_PAVED_ROAD_WORKS); break;
	}
}

/**
 * Terminate road works on a tile.
 * @param t Tile to stop the road works on.
 * @pre HasRoadWorks(t)
 */
static inline void TerminateRoadWorks(TileIndex t)
{
	assert(HasRoadWorks(t));
	SetRoadside(t, (Roadside)(GetRoadside(t) - ROADSIDE_GRASS_ROAD_WORKS + ROADSIDE_GRASS));
	/* Stop the counter */
	SB(GetTileEx(t)->m7, 0, 4, 0);
}


/**
 * Get the direction of the exit of a road depot.
 * @param t The tile to query.
 * @return Diagonal direction of the depot exit.
 */
template <bool Tgeneric>
static inline DiagDirection GetRoadDepotDirection(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsRoadDepot(t));
	return (DiagDirection)GB(GetTile(t)->m5, 0, 2);
}
/** @copydoc GetRoadDepotDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetRoadDepotDirection(TileIndex t) { return GetRoadDepotDirection<false>(t); }
/** @copydoc GetRoadDepotDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetRoadDepotDirection(GenericTileIndex t) { return GetRoadDepotDirection<true>(t); }


RoadBits GetAnyRoadBits(TileIndex tile, RoadType rt, bool straight_tunnel_bridge_entrance = false);


/**
 * Make a normal road tile.
 * @param t    Tile to make a normal road.
 * @param bits Road bits to set for all present road types.
 * @param rot  New present road types.
 * @param town Town ID if the road is a town-owned road.
 * @param road New owner of road.
 * @param tram New owner of tram tracks.
 */
template <bool Tgeneric>
static inline void MakeRoadNormal(typename TileIndexT<Tgeneric>::T t, RoadBits bits, RoadTypes rot, TownID town, Owner road, Owner tram)
{
	SetTileType(t, MP_ROAD);
	SetTileOwner(t, road);
	GetTile(t)->m2 = town;
	GetTile(t)->m3 = (HasBit(rot, ROADTYPE_TRAM) ? bits : 0);
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = (HasBit(rot, ROADTYPE_ROAD) ? bits : 0) | ROAD_TILE_NORMAL << 6;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = rot << 6;
	SetRoadOwner(t, ROADTYPE_TRAM, tram);
}
/** @copydoc MakeRoadNormal(TileIndexT<Tgeneric>::T,RoadBits,RoadTypes,TownID,Owner,Owner) */
static inline void MakeRoadNormal(TileIndex t, RoadBits bits, RoadTypes rot, TownID town, Owner road, Owner tram) { MakeRoadNormal<false>(t, bits, rot, town, road, tram); }
/** @copydoc MakeRoadNormal(TileIndexT<Tgeneric>::T,RoadBits,RoadTypes,TownID,Owner,Owner) */
static inline void MakeRoadNormal(GenericTileIndex t, RoadBits bits, RoadTypes rot, TownID town, Owner road, Owner tram) { MakeRoadNormal<true>(t, bits, rot, town, road, tram); }

/**
 * Make a level crossing.
 * @param t       Tile to make a level crossing.
 * @param road    New owner of road.
 * @param tram    New owner of tram tracks.
 * @param rail    New owner of the rail track.
 * @param roaddir Axis of the road.
 * @param rat     New rail type.
 * @param rot     New present road types.
 * @param town    Town ID if the road is a town-owned road.
 */
template <bool Tgeneric>
static inline void MakeRoadCrossing(typename TileIndexT<Tgeneric>::T t, Owner road, Owner tram, Owner rail, Axis roaddir, RailType rat, RoadTypes rot, uint town)
{
	SetTileType(t, MP_ROAD);
	SetTileOwner(t, rail);
	GetTile(t)->m2 = town;
	GetTile(t)->m3 = rat;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = ROAD_TILE_CROSSING << 6 | roaddir;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = rot << 6 | road;
	SetRoadOwner(t, ROADTYPE_TRAM, tram);
}
/** @copydoc MakeRoadCrossing(TileIndexT<Tgeneric>::T,Owner,Owner,Owner,Axis,RailType,RoadTypes,uint) */
static inline void MakeRoadCrossing(TileIndex t, Owner road, Owner tram, Owner rail, Axis roaddir, RailType rat, RoadTypes rot, uint town) { MakeRoadCrossing<false>(t, road, tram, rail, roaddir, rat, rot, town); }
/** @copydoc MakeRoadCrossing(TileIndexT<Tgeneric>::T,Owner,Owner,Owner,Axis,RailType,RoadTypes,uint) */
static inline void MakeRoadCrossing(GenericTileIndex t, Owner road, Owner tram, Owner rail, Axis roaddir, RailType rat, RoadTypes rot, uint town) { MakeRoadCrossing<true>(t, road, tram, rail, roaddir, rat, rot, town); }

/**
 * Make a road depot.
 * @param t     Tile to make a level crossing.
 * @param owner New owner of the depot.
 * @param did   New depot ID.
 * @param dir   Direction of the depot exit.
 * @param rt    Road type of the depot.
 */
template <bool Tgeneric>
static inline void MakeRoadDepot(typename TileIndexT<Tgeneric>::T t, Owner owner, DepotID did, DiagDirection dir, RoadType rt)
{
	SetTileType(t, MP_ROAD);
	SetTileOwner(t, owner);
	GetTile(t)->m2 = did;
	GetTile(t)->m3 = 0;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = ROAD_TILE_DEPOT << 6 | dir;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = RoadTypeToRoadTypes(rt) << 6 | owner;
	SetRoadOwner(t, ROADTYPE_TRAM, owner);
}
/** @copydoc MakeRoadDepot(TileIndexT<Tgeneric>::T,Owner,DepotID,DiagDirection,RoadType) */
static inline void MakeRoadDepot(TileIndex t, Owner owner, DepotID did, DiagDirection dir, RoadType rt) { MakeRoadDepot<false>(t, owner, did, dir, rt); }
/** @copydoc MakeRoadDepot(TileIndexT<Tgeneric>::T,Owner,DepotID,DiagDirection,RoadType) */
static inline void MakeRoadDepot(GenericTileIndex t, Owner owner, DepotID did, DiagDirection dir, RoadType rt) { MakeRoadDepot<true>(t, owner, did, dir, rt); }

#endif /* ROAD_MAP_H */
