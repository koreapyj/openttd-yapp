/* $Id: geometry_func.cpp 17248 2009-08-21 20:21:05Z rubidium $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file geometry_func.cpp Geometry functions. */

#include "../stdafx.h"
#include "geometry_func.hpp"
#include "math_func.hpp"
#include "../map_func.h"

/**
 * Compute bounding box of both dimensions.
 * @param d1 First dimension.
 * @param d2 Second dimension.
 * @return The bounding box of both dimensions, the smallest dimension that surrounds both arguments.
 */
Dimension maxdim(const Dimension &d1, const Dimension &d2)
{
	Dimension d;
	d.width  = max(d1.width,  d2.width);
	d.height = max(d1.height, d2.height);
	return d;
}

/**
 * Transform a given Point.
 *
 * The center point of the transformation is (0, 0).
 * For example, point (1, 2) rotated 90 degree left is (-2, 1).
 *
 * @param point The Point to transform.
 * @param transformation Transformation to perform.
 * @return The transformed Point.
 */
Point TransformPoint(Point point, DirTransformation transformation)
{
	TileIndexDiffC diff_from_x = TileIndexDiffCByDiagDir(TransformDiagDir(DIAGDIR_SW, transformation));
	TileIndexDiffC diff_from_y = TileIndexDiffCByDiagDir(TransformDiagDir(DIAGDIR_SE, transformation));
	Point ret = {
		point.x * diff_from_x.x + point.y * diff_from_y.x,
		point.x * diff_from_x.y + point.y * diff_from_y.y
	};
	return ret;
}
