/* $Id: map_type.h 24776 2012-12-01 13:12:39Z alberth $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file map_type.h Types related to maps. */

#ifndef MAP_TYPE_H
#define MAP_TYPE_H

/**
 * Data that is stored per tile. Also used TileExtended for this.
 * Look at docs/landscape.html for the exact meaning of the members.
 */
struct Tile {
	byte   type;        ///< The type
	byte   height;      ///< The height of the northern corner
	byte   m1;          ///< Primarily used for ownership information
	uint16 m2;          ///< Primarily used for indices to towns, industries and stations
	byte   m3;          ///< General purpose
	byte   m4;          ///< General purpose
	byte   m5;          ///< General purpose
};

/**
 * Data that is stored per tile. Also used Tile for this.
 * Look at docs/landscape.html for the exact meaning of the members.
 */
struct TileExtended {
	byte m6;    ///< Primarily used for bridges and rainforest/desert
	byte m7;    ///< Primarily used for newgrf support
};

/** Tile array. */
struct Map {
	uint size_x;      ///< Size of the map along the X
	uint size_y;      ///< Size of the map along the Y
	uint size;        ///< The number of tiles on the map
	Tile *m;          ///< Tiles of the map
	TileExtended *me; ///< Extended Tiles of the map
};

/** Main tile array. */
struct MainMap : Map {
	uint log_x;     ///< 2^log_x == size_x
	uint log_y;     ///< 2^log_y == size_y
	uint tile_mask; ///< size - 1 (to mask the mapsize)
};

/**
 * An offset value between to tiles.
 *
 * This value is used for the difference between
 * to tiles. It can be added to a tileindex to get
 * the resulting tileindex of the start tile applied
 * with this saved difference.
 *
 * @see TileDiffXY(int, int)
 */
typedef int32 TileIndexDiff;

/**
 * A pair-construct of a TileIndexDiff.
 *
 * This can be used to save the difference between to
 * tiles as a pair of x and y value.
 */
struct TileIndexDiffC {
	int16 x;        ///< The x value of the coordinate
	int16 y;        ///< The y value of the coordinate
};

/** Minimal and maximal map width and height */
static const uint MIN_MAP_SIZE_BITS = 6;                      ///< Minimal size of map is equal to 2 ^ MIN_MAP_SIZE_BITS
static const uint MAX_MAP_SIZE_BITS = 20;                     ///< Maximal size of map is equal to 2 ^ MAX_MAP_SIZE_BITS
static const uint MAX_MAP_TILES_BITS = 26;                    ///< Maximal number of tiles in a map is equal to 2 ^ MAX_MAP_TILES_BITS.
static const uint MIN_MAP_SIZE      = 1 << MIN_MAP_SIZE_BITS; ///< Minimal map size = 64
static const uint MAX_MAP_SIZE      = 1 << MAX_MAP_SIZE_BITS; ///< Maximal map size = 8192
static const uint MAX_MAP_TILES     = 1 << MAX_MAP_TILES_BITS;///< Maximal number of tiles in a map = 2048 * 2048

/**
 * Approximation of the length of a straight track, relative to a diagonal
 * track (ie the size of a tile side).
 *
 * #defined instead of const so it can
 * stay integer. (no runtime float operations) Is this needed?
 * Watch out! There are _no_ brackets around here, to prevent intermediate
 * rounding! Be careful when using this!
 * This value should be sqrt(2)/2 ~ 0.7071
 */
#define STRAIGHT_TRACK_LENGTH 7071/10000

/** Argument for CmdLevelLand describing what to do. */
enum LevelMode {
	LM_LEVEL, ///< Level the land.
	LM_LOWER, ///< Lower the land.
	LM_RAISE, ///< Raise the land.
};

#endif /* MAP_TYPE_H */