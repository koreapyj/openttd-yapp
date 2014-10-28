/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tilearea_type.h Functions related to tile areas. */

#ifndef TILEAREA_FUNC_H
#define TILEAREA_FUNC_H

#include "core/math_func.hpp"
#include "direction_func.h"
#include "tilearea_type.h"

/**
 * Transform a tile area.
 * @param trackdir The area to transform.
 * @param transformation Transformation to perform.
 * @return The transformed area.
 */
template <bool TgenericSrc, bool TgenericDst>
static TileAreaT<TgenericDst> TransformTileArea(const TileAreaT<TgenericSrc> &ta, typename TileIndexT<TgenericDst>::T dst_area_north, DirTransformation transformation)
{
	TileAreaT<TgenericDst> ret(dst_area_north, ta.w, ta.h);
	if (TransformAxis(AXIS_X, transformation) != AXIS_X) Swap(ret.w, ret.h);
	return ret;
}

/** @copydoc TransformTileArea<bool, bool> */
static inline TileArea        TransformTileArea(const TileArea        &ta, TileIndex        dst_area_north, DirTransformation transformation) { return TransformTileArea<false, false>(ta, dst_area_north, transformation); }
/** @copydoc TransformTileArea<bool, bool> */
static inline GenericTileArea TransformTileArea(const TileArea        &ta, GenericTileIndex dst_area_north, DirTransformation transformation) { return TransformTileArea<false, true>(ta, dst_area_north, transformation); }
/** @copydoc TransformTileArea<bool, bool> */
static inline TileArea        TransformTileArea(const GenericTileArea &ta, TileIndex        dst_area_north, DirTransformation transformation) { return TransformTileArea<true, false>(ta, dst_area_north, transformation); }
/** @copydoc TransformTileArea<bool, bool> */
static inline GenericTileArea TransformTileArea(const GenericTileArea &ta, GenericTileIndex dst_area_north, DirTransformation transformation) { return TransformTileArea<true, true>(ta, dst_area_north, transformation); }

#endif /* TILEAREA_FUNC_H */
