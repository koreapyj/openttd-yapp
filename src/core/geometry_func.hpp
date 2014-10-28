/* $Id: geometry_func.hpp 17248 2009-08-21 20:21:05Z rubidium $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file geometry_func.hpp Geometry functions. */

#ifndef GEOMETRY_FUNC_HPP
#define GEOMETRY_FUNC_HPP

#include "geometry_type.hpp"
#include "math_func.hpp"
#include "../direction_func.h"

Dimension maxdim(const Dimension &d1, const Dimension &d2);

/**
 * Transform a given Dimension.
 *
 * The width and the height will be swapped or will stay unchanged based on used transformation.
 *
 * @param dim The Dimension to transform.
 * @param transformation Transformation to perform.
 * @return The transformed Dimension.
 */
static inline Dimension TransformDimension(Dimension dim, DirTransformation transformation)
{
	if (TransformAxis(AXIS_X, transformation) != AXIS_X) Swap(dim.width, dim.height);
	return dim;
}

Point TransformPoint(Point point, DirTransformation transformation);

#endif /* GEOMETRY_FUNC_HPP */
