/* $Id: tilearea.cpp 24900 2013-01-08 22:46:42Z planetmaker $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tilearea.cpp Handling of tile areas. */

#include "stdafx.h"

#include "core/geometry_func.hpp"
#include "tilearea_type.h"

/**
 * Construct this tile area based on two points.
 * @param start the start of the area
 * @param end   the end of the area
 */
template <bool Tgeneric>
TileAreaT<Tgeneric>::TileAreaT(TileIndexType start, TileIndexType end)
{
	assert(IsSameMap(start, end));

	uint sx = TileX(start);
	uint sy = TileY(start);
	uint ex = TileX(end);
	uint ey = TileY(end);

	if (sx > ex) Swap(sx, ex);
	if (sy > ey) Swap(sy, ey);

	this->tile = TileXY<Tgeneric>(sx, sy, MapOf(start));
	this->w    = ex - sx + 1;
	this->h    = ey - sy + 1;
}

/**
 * Add a single tile to a tile area; enlarge if needed.
 * @param to_add The tile to add
 */
template <bool Tgeneric>
void TileAreaT<Tgeneric>::Add(TileIndexType to_add)
{
	if (!IsValidTileIndex(this->tile)) {
		this->tile = to_add;
		this->w = 1;
		this->h = 1;
		return;
	}

	uint sx = TileX(this->tile);
	uint sy = TileY(this->tile);
	uint ex = sx + this->w - 1;
	uint ey = sy + this->h - 1;

	uint ax = TileX(to_add);
	uint ay = TileY(to_add);

	sx = min(ax, sx);
	sy = min(ay, sy);
	ex = max(ax, ex);
	ey = max(ay, ey);

	this->tile = TileXY<Tgeneric>(sx, sy, MapOf(to_add));
	this->w    = ex - sx + 1;
	this->h    = ey - sy + 1;
}

/**
 * Does this tile area intersect with another?
 * @param ta the other tile area to check against.
 * @return true if they intersect.
 */
template <bool Tgeneric>
bool TileAreaT<Tgeneric>::Intersects(const TileAreaT<Tgeneric> &ta) const
{
	if (ta.w == 0 || this->w == 0) return false;

	assert(ta.w != 0 && ta.h != 0 && this->w != 0 && this->h != 0);
	assert(IsSameMap(this->tile, ta.tile));

	uint left1   = TileX(this->tile);
	uint top1    = TileY(this->tile);
	uint right1  = left1 + this->w - 1;
	uint bottom1 = top1  + this->h - 1;

	uint left2   = TileX(ta.tile);
	uint top2    = TileY(ta.tile);
	uint right2  = left2 + ta.w - 1;
	uint bottom2 = top2  + ta.h - 1;

	return !(
			left2   > right1  ||
			right2  < left1   ||
			top2    > bottom1 ||
			bottom2 < top1
		);
}

/**
 * Does this tile area contains another?
 * @param ta the other tile area to check against.
 * @return true if the other area is fully contained.
 */
template <bool Tgeneric>
bool TileAreaT<Tgeneric>::Contains(const TileAreaT<Tgeneric> &ta) const
{
	if (ta.w == 0 || this->w == 0) return false;

	assert(ta.w != 0 && ta.h != 0 && this->w != 0 && this->h != 0);
	assert(IsSameMap(this->tile, ta.tile));

	uint left1   = TileX(this->tile);
	uint top1    = TileY(this->tile);
	uint right1  = left1 + this->w - 1;
	uint bottom1 = top1  + this->h - 1;

	uint left2   = TileX(ta.tile);
	uint top2    = TileY(ta.tile);
	uint right2  = left2 + ta.w - 1;
	uint bottom2 = top2  + ta.h - 1;

	return
			left2   >= left1  &&
			right2  <= right1 &&
			top2    >= top1   &&
			bottom2 <= bottom1;
}

/**
 * Does this tile area contain a tile?
 * @param tile Tile to test for.
 * @return True if the tile is inside the area.
 */
template <bool Tgeneric>
bool TileAreaT<Tgeneric>::Contains(TileIndexType tile) const
{
	if (this->w == 0) return false;

	assert(this->w != 0 && this->h != 0);
	assert(IsSameMap(this->tile, tile));

	uint left   = TileX(this->tile);
	uint top    = TileY(this->tile);
	uint tile_x = TileX(tile);
	uint tile_y = TileY(tile);

	return IsInsideBS(tile_x, left, this->w) && IsInsideBS(tile_y, top, this->h);
}

/**
 * Clamp the tile area to map borders.
 */
template <bool Tgeneric>
void TileAreaT<Tgeneric>::ClampToMap()
{
	assert(IsValidTileIndex(this->tile));
	this->w = min(this->w, MapSizeX(MapOf(this->tile)) - TileX(this->tile));
	this->h = min(this->h, MapSizeY(MapOf(this->tile)) - TileY(this->tile));
}

/**
 * Get coordinates of transformed nothern tile of this area relative to the
 * northern tile of transformed area.
 *
 * When transforming this area into another, the northern tile becomes some other
 * tile in the transformed area. The function returns coordinates of this other tile
 * relative to the transformed area.
 *
 * Note that calculation are independent from desired position of the transformed area.
 *
 * @param transformation transformation to perform
 * @return the distance
 */
template <bool Tgeneric>
TileIndexDiffC TileAreaT<Tgeneric>::TransformedNorthOffset(DirTransformation transformation) const
{
	Dimension distance = { this->w - 1, this->h - 1 };
	distance = TransformDimension(distance, transformation);
	TileIndexDiffC ret = TransformedNorthCornerDiffC(transformation);
	ret.x *= distance.width;
	ret.y *= distance.height;
	return ret;
}

/**
 * Get coordinates of a transformed tile of this area relative to the transformed
 * northern tile of this area.
 *
 * The function takes x/y coordinates of a tile relative to this area and performs
 * a transformation on them.
 *
 * Note that calculation are independent from desired position of the transformed area.
 *
 * @param tile the tile to transform
 * @param transformation transformation to perform
 * @return the distance
 */
template <bool Tgeneric>
TileIndexDiffC TileAreaT<Tgeneric>::TransformedTileOffset(typename TileAreaT<Tgeneric>::TileIndexType tile, DirTransformation transformation) const
{
	assert(IsSameMap(this->tile, tile));

	/* calculate coordinates of the tile relative to the northern tile of the area */
	Point coords = { TileX(tile) - TileX(this->tile), TileY(tile) - TileY(this->tile) };
	/* transform coordinates, now they will be relative to the transformed northern tile of the area */
	coords = TransformPoint(coords, transformation);
	TileIndexDiffC ret = { (uint16)coords.x, (uint16)coords.y };
	return ret;
}

/**
 * Initialize iteration.
 * @param my_index Pointer to the tile index of the iterator. The index must be set to the first corner of the iteration before you call Init.
 * @param opposite_corner Second (opposite) corner of the iteration.
 * @param my_map The map that iterator iterates through.
 */
void DiagonalTileIteratorController::Init(RawTileIndex *my_index, RawTileIndex opposite_corner, Map *my_map)
{
	assert(IsValidTileIndex(GenericTileIndex(*my_index, my_map)));
	assert(IsValidTileIndex(GenericTileIndex(opposite_corner, my_map)));

	this->base_x = TileX(GenericTileIndex(*my_index, my_map));
	this->base_y = TileY(GenericTileIndex(*my_index, my_map));
	this->a_cur = 0;
	this->b_cur = 0;
	int dist_x = TileX(GenericTileIndex(opposite_corner, my_map)) - this->base_x;
	int dist_y = TileY(GenericTileIndex(opposite_corner, my_map)) - this->base_y;
	this->a_max = dist_x + dist_y;
	this->b_max = dist_y - dist_x;

	/* Unfortunately we can't find a new base and make all a and b positive because
	 * the new base might be a "flattened" corner where there actually is no single
	 * tile. If we try anyway the result is either inaccurate ("one off" half of the
	 * time) or the code gets much more complex;
	 *
	 * We also need to increment here to have equality as marker for the end of a row or
	 * column. Like that it's shorter than having another if/else in operator++
	 */
	if (this->a_max > 0) {
		this->a_max++;
	} else {
		this->a_max--;
	}

	if (this->b_max > 0) {
		this->b_max++;
	} else {
		this->b_max--;
	}
}

/**
 * Perform single iteration step.
 * @param my_index Pointer to the tile index of the iterator.
 * @param my_map The map that iterator iterates through.
 */
void DiagonalTileIteratorController::Advance(RawTileIndex *my_index, Map *my_map)
{
	assert(*my_index != INVALID_TILE_INDEX);

	/* Determine the next tile, while clipping at map borders */
	bool new_line = false;
	do {
		/* Iterate using the rotated coordinates. */
		if (this->a_max == 1 || this->a_max == -1) {
			/* Special case: Every second column has zero length, skip them completely */
			this->a_cur = 0;
			if (this->b_max > 0) {
				this->b_cur = min(this->b_cur + 2, this->b_max);
			} else {
				this->b_cur = max(this->b_cur - 2, this->b_max);
			}
		} else {
			/* Every column has at least one tile to process */
			if (this->a_max > 0) {
				this->a_cur += 2;
				new_line = this->a_cur >= this->a_max;
			} else {
				this->a_cur -= 2;
				new_line = this->a_cur <= this->a_max;
			}
			if (new_line) {
				/* offset of initial a_cur: one tile in the same direction as a_max
				 * every second line.
				 */
				this->a_cur = abs(this->a_cur) % 2 ? 0 : (this->a_max > 0 ? 1 : -1);

				if (this->b_max > 0) {
					++this->b_cur;
				} else {
					--this->b_cur;
				}
			}
		}

		/* And convert the coordinates back once we've gone to the next tile. */
		uint x = this->base_x + (this->a_cur - this->b_cur) / 2;
		uint y = this->base_y + (this->b_cur + this->a_cur) / 2;
		/* Prevent wrapping around the map's borders. */
		*my_index = x >= MapSizeX(my_map) || y >= MapSizeY(my_map) ? INVALID_TILE_INDEX : TileXY<true>(x, y, my_map).index;
	} while (!IsValidTileIndex(GenericTileIndex(*my_index, my_map)) && this->b_max != this->b_cur);

	if (this->b_max == this->b_cur) *my_index = INVALID_TILE_INDEX;
}

/**
 * Initialize iteration.
 * @param src_index Pointer to the source tile index of the iterator. The index must be set to the northern tile of the source area before you call Init.
 * @param dst_index Pointer to the destination tile index of the iterator. The index must be set to the transformed northern tile of the source area before you call Init.
 * @param src_w The width of the source area.
 * @param src_h The height of the source area.
 * @param transformation Transformation to perform.
 */
void TransformationTileIteratorController::Init(RawTileIndex *src_index, RawTileIndex *dst_index, uint16 src_w, uint16 src_h, DirTransformation transformation)
{
	assert((*src_index != INVALID_TILE_INDEX) == (*dst_index != INVALID_TILE_INDEX));

	this->OrthogonalTileIteratorController::Init(src_index, src_w, src_h);
	this->transformation = transformation;
}

/**
 * Perform single iteration step.
 * @param src_index Pointer to the source tile index of the iterator.
 * @param src_map The source map that iterator iterates through.
 * @param dst_index Pointer to the destination tile index of the iterator.
 * @param dst_map The destination map that iterator iterates through.
 */
void TransformationTileIteratorController::Advance(RawTileIndex *src_index, Map *src_map, RawTileIndex *dst_index, Map *dst_map)
{
	assert(*src_index != INVALID_TILE_INDEX);

	if (--this->x > 0) {
		++*src_index;
		*dst_index += ToTileIndexDiff<true>(TileIndexDiffCByDiagDir(TransformDiagDir(DIAGDIR_SW, this->transformation)), dst_map);
	} else if (--this->y > 0) {
		this->x = this->w;
		*src_index += TileDiffXY(1, 1, src_map) - this->w;
		*dst_index -= ToTileIndexDiff<true>(TileIndexDiffCByDiagDir(TransformDiagDir(DIAGDIR_SW, this->transformation)), dst_map) * (this->w - 1);
		*dst_index += ToTileIndexDiff<true>(TileIndexDiffCByDiagDir(TransformDiagDir(DIAGDIR_SE, this->transformation)), dst_map);
	} else {
		*src_index = INVALID_TILE_INDEX;
		*dst_index = INVALID_TILE_INDEX;
	}
}

/* instantiate */
template struct TileAreaT<false>;
template struct TileAreaT<true>;
