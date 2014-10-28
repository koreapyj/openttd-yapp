/* $Id: tile_type.h 24900 2013-01-08 22:46:42Z planetmaker $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tile_type.h Types related to tiles. */

#ifndef TILE_TYPE_H
#define TILE_TYPE_H

#include "map_type.h"

static const uint TILE_SIZE      = 16;            ///< Tiles are 16x16 "units" in size
static const uint TILE_UNIT_MASK = TILE_SIZE - 1; ///< For masking in/out the inner-tile units.
static const uint TILE_PIXELS    = 32;            ///< a tile is 32x32 pixels
static const uint TILE_HEIGHT    =  8;            ///< The standard height-difference between tiles on two levels is 8 (z-diff 8)

static const uint MAX_TILE_HEIGHT     = 255;                    ///< Maximum allowed tile height

static const uint MIN_MAX_HEIGHTLEVEL = 15;                     ///< Lower bound of maximal allowed heightlevel (in the construction settings)
static const uint DEF_MAX_HEIGHTLEVEL = 30;                     ///< Default maximal allowed heightlevel (in the construction settings)
static const uint MAX_MAX_HEIGHTLEVEL = 255;                    ///< Upper bound of maximal allowed heightlevel (in the construction settings)

static const uint MIN_SNOWLINE_HEIGHT = 2;   ///< Minimum snowline height
static const uint DEF_SNOWLINE_HEIGHT = 24;   ///< Default snowline height
static const uint MAX_SNOWLINE_HEIGHT = 253;  ///< Maximum allowed snowline height


/**
 * The different types of tiles.
 *
 * Each tile belongs to one type, according whatever is build on it.
 *
 * @note A railway with a crossing street is marked as MP_ROAD.
 */
enum TileType {
	MP_CLEAR,               ///< A tile without any structures, i.e. grass, rocks, farm fields etc.
	MP_RAILWAY,             ///< A railway
	MP_ROAD,                ///< A tile with road (or tram tracks)
	MP_HOUSE,               ///< A house by a town
	MP_TREES,               ///< Tile got trees
	MP_STATION,             ///< A tile of a station
	MP_WATER,               ///< Water tile
	MP_VOID,                ///< Invisible tiles at the SW and SE border
	MP_INDUSTRY,            ///< Part of an industry
	MP_TUNNELBRIDGE,        ///< Tunnel entry/exit and bridge heads
	MP_OBJECT,              ///< Contains objects such as transmitters and owned land
};

/**
 * Additional infos of a tile on a tropic game.
 *
 * The tropiczone is not modified during gameplay. It mainly affects tree growth. (desert tiles are visible though)
 *
 * In randomly generated maps:
 *  TROPICZONE_DESERT: Generated everywhere, if there is neither water nor mountains (TileHeight >= 4) in a certain distance from the tile.
 *  TROPICZONE_RAINFOREST: Generated everywhere, if there is no desert in a certain distance from the tile.
 *  TROPICZONE_NORMAL: Everywhere else, i.e. between desert and rainforest and on sea (if you clear the water).
 *
 * In scenarios:
 *  TROPICZONE_NORMAL: Default value.
 *  TROPICZONE_DESERT: Placed manually.
 *  TROPICZONE_RAINFOREST: Placed if you plant certain rainforest-trees.
 */
enum TropicZone {
	TROPICZONE_NORMAL     = 0,      ///< Normal tropiczone
	TROPICZONE_DESERT     = 1,      ///< Tile is desert
	TROPICZONE_RAINFOREST = 2,      ///< Rainforest tile
};

typedef uint32 RawTileIndex; ///< general purpose tile index, not bounded to any map
static const RawTileIndex INVALID_TILE_INDEX = (RawTileIndex)-1;

/**
 * The index/ID of a Tile on the main map.
 *
 * While this is just another name for RawTileIndex type, it should be used
 * in context of tiles of the main tile array.
 */
typedef RawTileIndex TileIndex;

/**
 * The very nice invalid tile marker
 */
static const TileIndex INVALID_TILE = (TileIndex)-1;

/** The index/ID of a tile bounded to a given map. */
struct GenericTileIndex {
	RawTileIndex index; ///< position of the tile in array
	Map *map;           ///< the map that this index is bounded to

	inline GenericTileIndex() : map(NULL) { }
	inline GenericTileIndex(const GenericTileIndex &tile) : index(tile.index), map(tile.map) { }
	inline GenericTileIndex(RawTileIndex index, Map *map) : index(index), map(map) { }

	inline explicit GenericTileIndex(const TileIndex &tile) : index(tile)
	{
		extern MainMap _main_map;
		this->map = &_main_map;
	}

	inline GenericTileIndex &operator += (TileIndexDiff diff) { return this->index += diff, *this; }
	inline GenericTileIndex &operator -= (TileIndexDiff diff) { return this->index -= diff, *this; }
	inline GenericTileIndex operator + (TileIndexDiff diff) const { return GenericTileIndex(this->index + diff, this->map); }
	inline GenericTileIndex operator - (TileIndexDiff diff) const { return GenericTileIndex(this->index - diff, this->map); }

	inline GenericTileIndex &operator ++ () { return ++this->index, *this; }
	inline GenericTileIndex &operator -- () { return --this->index, *this; }
	inline GenericTileIndex operator ++ (int) { return GenericTileIndex(this->index++, this->map); }
	inline GenericTileIndex operator -- (int) { return GenericTileIndex(this->index--, this->map); }

	inline bool operator == (const GenericTileIndex &tile) const { return this->index == tile.index && this->map == tile.map; }
	inline bool operator != (const GenericTileIndex &tile) const { return this->index != tile.index || this->map != tile.map; }

	inline bool operator <= (const GenericTileIndex &tile) const
	{
		assert(this->map == tile.map);
		return this->index <= tile.index;
	}

	inline bool operator >= (const GenericTileIndex &tile) const
	{
		assert(this->map == tile.map);
		return this->index >= tile.index;
	}

	inline bool operator < (const GenericTileIndex &tile) const
	{
		assert(this->map == tile.map);
		return this->index < tile.index;
	}

	inline bool operator > (const GenericTileIndex &tile) const
	{
		assert(this->map == tile.map);
		return this->index > tile.index;
	}

};

/**
 * Helper class to construct templatized functions operating on different
 * types of tile indices.
 */
template <bool Tgeneric>
struct TileIndexT;

template <>
struct TileIndexT<false> {
	typedef TileIndex T;
};

template <>
struct TileIndexT<true> {
	typedef GenericTileIndex T;
};

#endif /* TILE_TYPE_H */
