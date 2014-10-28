/* $Id: water_map.h 23512 2011-12-13 23:01:36Z planetmaker $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file water_map.h Map accessors for water tiles. */

#ifndef WATER_MAP_H
#define WATER_MAP_H

#include "depot_type.h"
#include "tile_map.h"

/**
 * Bit field layout of m5 for water tiles.
 */
enum WaterTileTypeBitLayout {
	WBL_TYPE_BEGIN        = 4,   ///< Start of the 'type' bitfield.
	WBL_TYPE_COUNT        = 4,   ///< Length of the 'type' bitfield.

	WBL_TYPE_NORMAL       = 0x0, ///< Clear water or coast ('type' bitfield).
	WBL_TYPE_LOCK         = 0x1, ///< Lock ('type' bitfield).
	WBL_TYPE_DEPOT        = 0x8, ///< Depot ('type' bitfield).

	WBL_COAST_FLAG        = 0,   ///< Flag for coast.

	WBL_LOCK_ORIENT_BEGIN = 0,   ///< Start of lock orientiation bitfield.
	WBL_LOCK_ORIENT_COUNT = 2,   ///< Length of lock orientiation bitfield.
	WBL_LOCK_PART_BEGIN   = 2,   ///< Start of lock part bitfield.
	WBL_LOCK_PART_COUNT   = 2,   ///< Length of lock part bitfield.

	WBL_DEPOT_PART        = 0,   ///< Depot part flag.
	WBL_DEPOT_AXIS        = 1,   ///< Depot axis flag.
};

/** Available water tile types. */
enum WaterTileType {
	WATER_TILE_CLEAR, ///< Plain water.
	WATER_TILE_COAST, ///< Coast.
	WATER_TILE_LOCK,  ///< Water lock.
	WATER_TILE_DEPOT, ///< Water Depot.
};

/** classes of water (for #WATER_TILE_CLEAR water tile type). */
enum WaterClass {
	WATER_CLASS_SEA,     ///< Sea.
	WATER_CLASS_CANAL,   ///< Canal.
	WATER_CLASS_RIVER,   ///< River.
	WATER_CLASS_INVALID, ///< Used for industry tiles on land (also for oilrig if newgrf says so).
};
/** Helper information for extract tool. */
template <> struct EnumPropsT<WaterClass> : MakeEnumPropsT<WaterClass, byte, WATER_CLASS_SEA, WATER_CLASS_INVALID, WATER_CLASS_INVALID, 2> {};

/** Sections of the water depot. */
enum DepotPart {
	DEPOT_PART_NORTH = 0, ///< Northern part of a depot.
	DEPOT_PART_SOUTH = 1, ///< Southern part of a depot.
	DEPOT_PART_END
};

/** Sections of the water lock. */
enum LockPart {
	LOCK_PART_MIDDLE = 0, ///< Middle part of a lock.
	LOCK_PART_LOWER  = 1, ///< Lower part of a lock.
	LOCK_PART_UPPER  = 2, ///< Upper part of a lock.
};

/**
 * Get the water tile type at a tile.
 * @param t Water tile to query.
 * @return Water tile type at the tile.
 */
template <bool Tgeneric>
static inline WaterTileType GetWaterTileType(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_WATER));

	switch (GB(GetTile(t)->m5, WBL_TYPE_BEGIN, WBL_TYPE_COUNT)) {
		case WBL_TYPE_NORMAL: return HasBit(GetTile(t)->m5, WBL_COAST_FLAG) ? WATER_TILE_COAST : WATER_TILE_CLEAR;
		case WBL_TYPE_LOCK:   return WATER_TILE_LOCK;
		case WBL_TYPE_DEPOT:  return WATER_TILE_DEPOT;
		default: NOT_REACHED();
	}
}
/** @copydoc GetWaterTileType(TileIndexT<Tgeneric>::T) */
static inline WaterTileType GetWaterTileType(TileIndex t) { return GetWaterTileType<false>(t); }
/** @copydoc GetWaterTileType(TileIndexT<Tgeneric>::T) */
static inline WaterTileType GetWaterTileType(GenericTileIndex t) { return GetWaterTileType<true>(t); }

/**
 * Checks whether the tile has an waterclass associated.
 * You can then subsequently call GetWaterClass().
 * @param t Tile to query.
 * @return True if the tiletype has a waterclass.
 */
template <bool Tgeneric>
static inline bool HasTileWaterClass(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_WATER) || IsTileType(t, MP_STATION) || IsTileType(t, MP_INDUSTRY) || IsTileType(t, MP_OBJECT);
}
/** @copydoc HasTileWaterClass(TileIndexT<Tgeneric>::T) */
static inline bool HasTileWaterClass(TileIndex t) { return HasTileWaterClass<false>(t); }
/** @copydoc HasTileWaterClass(TileIndexT<Tgeneric>::T) */
static inline bool HasTileWaterClass(GenericTileIndex t) { return HasTileWaterClass<true>(t); }

/**
 * Get the water class at a tile.
 * @param t Water tile to query.
 * @pre IsTileType(t, MP_WATER) || IsTileType(t, MP_STATION) || IsTileType(t, MP_INDUSTRY) || IsTileType(t, MP_OBJECT)
 * @return Water class at the tile.
 */
template <bool Tgeneric>
static inline WaterClass GetWaterClass(typename TileIndexT<Tgeneric>::T t)
{
	assert(HasTileWaterClass(t));
	return (WaterClass)GB(GetTile(t)->m1, 5, 2);
}
/** @copydoc GetWaterClass(TileIndexT<Tgeneric>::T) */
static inline WaterClass GetWaterClass(TileIndex t) { return GetWaterClass<false>(t); }
/** @copydoc GetWaterClass(TileIndexT<Tgeneric>::T) */
static inline WaterClass GetWaterClass(GenericTileIndex t) { return GetWaterClass<true>(t); }

/**
 * Set the water class at a tile.
 * @param t  Water tile to change.
 * @param wc New water class.
 * @pre IsTileType(t, MP_WATER) || IsTileType(t, MP_STATION) || IsTileType(t, MP_INDUSTRY) || IsTileType(t, MP_OBJECT)
 */
template <bool Tgeneric>
static inline void SetWaterClass(typename TileIndexT<Tgeneric>::T t, WaterClass wc)
{
	assert(HasTileWaterClass(t));
	SB(GetTile(t)->m1, 5, 2, wc);
}
/** @copydoc SetWaterClass(TileIndexT<Tgeneric>::T,WaterClass) */
static inline void SetWaterClass(TileIndex t, WaterClass wc) { return SetWaterClass<false>(t, wc); }
/** @copydoc SetWaterClass(TileIndexT<Tgeneric>::T,WaterClass) */
static inline void SetWaterClass(GenericTileIndex t, WaterClass wc) { return SetWaterClass<true>(t, wc); }

/**
 * Tests if the tile was built on water.
 * @param t the tile to check
 * @pre IsTileType(t, MP_WATER) || IsTileType(t, MP_STATION) || IsTileType(t, MP_INDUSTRY) || IsTileType(t, MP_OBJECT)
 * @return true iff on water
 */
static inline bool IsTileOnWater(TileIndex t)
{
	return (GetWaterClass(t) != WATER_CLASS_INVALID);
}

/**
 * Is it a plain water tile?
 * @param t Water tile to query.
 * @return \c true if any type of clear water like ocean, river, or canal.
 * @pre IsTileType(t, MP_WATER)
 */
template <bool Tgeneric>
static inline bool IsWater(typename TileIndexT<Tgeneric>::T t)
{
	return GetWaterTileType(t) == WATER_TILE_CLEAR;
}
/** @copydoc IsWater(TileIndexT<Tgeneric>::T) */
static inline bool IsWater(TileIndex t) { return IsWater<false>(t); }
/** @copydoc IsWater(TileIndexT<Tgeneric>::T) */
static inline bool IsWater(GenericTileIndex t) { return IsWater<true>(t); }

/**
 * Is it a sea water tile?
 * @param t Water tile to query.
 * @return \c true if it is a sea water tile.
 * @pre IsTileType(t, MP_WATER)
 */
static inline bool IsSea(TileIndex t)
{
	return IsWater(t) && GetWaterClass(t) == WATER_CLASS_SEA;
}

/**
 * Is it a canal tile?
 * @param t Water tile to query.
 * @return \c true if it is a canal tile.
 * @pre IsTileType(t, MP_WATER)
 */
template <bool Tgeneric>
static inline bool IsCanal(typename TileIndexT<Tgeneric>::T t)
{
	return IsWater(t) && GetWaterClass(t) == WATER_CLASS_CANAL;
}
/** @copydoc IsCanal(TileIndexT<Tgeneric>::T) */
static inline bool IsCanal(TileIndex t) { return IsCanal<false>(t); }
/** @copydoc IsCanal(TileIndexT<Tgeneric>::T) */
static inline bool IsCanal(GenericTileIndex t) { return IsCanal<true>(t); }

/**
 * Is it a river water tile?
 * @param t Water tile to query.
 * @return \c true if it is a river water tile.
 * @pre IsTileType(t, MP_WATER)
 */
static inline bool IsRiver(TileIndex t)
{
	return IsWater(t) && GetWaterClass(t) == WATER_CLASS_RIVER;
}

/**
 * Is it a water tile with plain water?
 * @param t Tile to query.
 * @return \c true if it is a plain water tile.
 */
template <bool Tgeneric>
static inline bool IsWaterTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_WATER) && IsWater(t);
}
/** @copydoc IsWaterTile(TileIndexT<Tgeneric>::T) */
static inline bool IsWaterTile(TileIndex t) { return IsWaterTile<false>(t); }
/** @copydoc IsWaterTile(TileIndexT<Tgeneric>::T) */
static inline bool IsWaterTile(GenericTileIndex t) { return IsWaterTile<true>(t); }

/**
 * Is it a coast tile?
 * @param t Water tile to query.
 * @return \c true if it is a sea water tile.
 * @pre IsTileType(t, MP_WATER)
 */
template <bool Tgeneric>
static inline bool IsCoast(typename TileIndexT<Tgeneric>::T t)
{
	return GetWaterTileType(t) == WATER_TILE_COAST;
}
/** @copydoc IsCoast(TileIndexT<Tgeneric>::T) */
static inline bool IsCoast(TileIndex t) { return IsCoast<false>(t); }
/** @copydoc IsCoast(TileIndexT<Tgeneric>::T) */
static inline bool IsCoast(GenericTileIndex t) { return IsCoast<true>(t); }

/**
 * Is it a coast tile
 * @param t Tile to query.
 * @return \c true if it is a coast.
 */
static inline bool IsCoastTile(TileIndex t)
{
	return IsTileType(t, MP_WATER) && IsCoast(t);
}

/**
 * Is it a water tile with a ship depot on it?
 * @param t Water tile to query.
 * @return \c true if it is a ship depot tile.
 * @pre IsTileType(t, MP_WATER)
 */
template <bool Tgeneric>
static inline bool IsShipDepot(typename TileIndexT<Tgeneric>::T t)
{
	return GetWaterTileType(t) == WATER_TILE_DEPOT;
}
/** @copydoc IsShipDepot(TileIndexT<Tgeneric>::T) */
static inline bool IsShipDepot(TileIndex t) { return IsShipDepot<false>(t); }
/** @copydoc IsShipDepot(TileIndexT<Tgeneric>::T) */
static inline bool IsShipDepot(GenericTileIndex t) { return IsShipDepot<true>(t); }

/**
 * Is it a ship depot tile?
 * @param t Tile to query.
 * @return \c true if it is a ship depot tile.
 */
template <bool Tgeneric>
static inline bool IsShipDepotTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_WATER) && IsShipDepot(t);
}
/** @copydoc IsShipDepotTile(TileIndexT<Tgeneric>::T) */
static inline bool IsShipDepotTile(TileIndex t) { return IsShipDepotTile<false>(t); }
/** @copydoc IsShipDepotTile(TileIndexT<Tgeneric>::T) */
static inline bool IsShipDepotTile(GenericTileIndex t) { return IsShipDepotTile<true>(t); }

/**
 * Get the axis of the ship depot.
 * @param t Water tile to query.
 * @return Axis of the depot.
 * @pre IsShipDepotTile(t)
 */
template <bool Tgeneric>
static inline Axis GetShipDepotAxis(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsShipDepotTile(t));
	return (Axis)GB(GetTile(t)->m5, WBL_DEPOT_AXIS, 1);
}
/** @copydoc GetShipDepotAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetShipDepotAxis(TileIndex t) { return GetShipDepotAxis<false>(t); }
/** @copydoc GetShipDepotAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetShipDepotAxis(GenericTileIndex t) { return GetShipDepotAxis<true>(t); }

/**
 * Get the part of a ship depot.
 * @param t Water tile to query.
 * @return Part of the depot.
 * @pre IsShipDepotTile(t)
 */
template <bool Tgeneric>
static inline DepotPart GetShipDepotPart(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsShipDepotTile(t));
	return (DepotPart)GB(GetTile(t)->m5, WBL_DEPOT_PART, 1);
}
/** @copydoc GetShipDepotPart(TileIndexT<Tgeneric>::T) */
static inline DepotPart GetShipDepotPart(TileIndex t) { return GetShipDepotPart<false>(t); }
/** @copydoc GetShipDepotPart(TileIndexT<Tgeneric>::T) */
static inline DepotPart GetShipDepotPart(GenericTileIndex t) { return GetShipDepotPart<true>(t); }

/**
 * Get the direction of the ship depot.
 * @param t Water tile to query.
 * @return Direction of the depot.
 * @pre IsShipDepotTile(t)
 */
template <bool Tgeneric>
static inline DiagDirection GetShipDepotDirection(typename TileIndexT<Tgeneric>::T t)
{
	return XYNSToDiagDir(GetShipDepotAxis(t), GetShipDepotPart(t));
}
/** @copydoc GetShipDepotDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetShipDepotDirection(TileIndex t) { return GetShipDepotDirection<false>(t); }
/** @copydoc GetShipDepotDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetShipDepotDirection(GenericTileIndex t) { return GetShipDepotDirection<true>(t); }

/**
 * Get the other tile of the ship depot.
 * @param t Tile to query, containing one section of a ship depot.
 * @return Tile containing the other section of the depot.
 * @pre IsShipDepotTile(t)
 */
template <bool Tgeneric>
static inline typename TileIndexT<Tgeneric>::T GetOtherShipDepotTile(typename TileIndexT<Tgeneric>::T t)
{
	return t + (GetShipDepotPart(t) != DEPOT_PART_NORTH ? -1 : 1) * (GetShipDepotAxis(t) != AXIS_X ? TileDiffXY(0, 1, MapOf(t)) : TileDiffXY(1, 0, MapOf(t)));
}
/** @copydoc GetOtherShipDepotTile(TileIndexT<Tgeneric>::T) */
static inline TileIndex GetOtherShipDepotTile(TileIndex t) { return GetOtherShipDepotTile<false>(t); }
/** @copydoc GetOtherShipDepotTile(TileIndexT<Tgeneric>::T) */
static inline GenericTileIndex GetOtherShipDepotTile(GenericTileIndex t) { return GetOtherShipDepotTile<true>(t); }

/**
 * Get the most northern tile of a ship depot.
 * @param t One of the tiles of the ship depot.
 * @return The northern tile of the depot.
 * @pre IsShipDepotTile(t)
 */
static inline TileIndex GetShipDepotNorthTile(TileIndex t)
{
	assert(IsShipDepot(t));
	TileIndex tile2 = GetOtherShipDepotTile(t);

	return t < tile2 ? t : tile2;
}

/**
 * Is there a lock on a given water tile?
 * @param t Water tile to query.
 * @return \c true if it is a water lock tile.
 * @pre IsTileType(t, MP_WATER)
 */
template <bool Tgeneric>
static inline bool IsLock(typename TileIndexT<Tgeneric>::T t)
{
	return GetWaterTileType(t) == WATER_TILE_LOCK;
}
/** @copydoc IsLock(TileIndexT<Tgeneric>::T) */
static inline bool IsLock(TileIndex t) { return IsLock<false>(t); }
/** @copydoc IsLock(TileIndexT<Tgeneric>::T) */
static inline bool IsLock(GenericTileIndex t) { return IsLock<true>(t); }

/**
 * Get the direction of the water lock.
 * @param t Water tile to query.
 * @return Direction of the lock.
 * @pre IsTileType(t, MP_WATER) && IsLock(t)
 */
template <bool Tgeneric>
static inline DiagDirection GetLockDirection(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsLock(t));
	return (DiagDirection)GB(GetTile(t)->m5, WBL_LOCK_ORIENT_BEGIN, WBL_LOCK_ORIENT_COUNT);
}
/** @copydoc GetLockDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetLockDirection(TileIndex t) { return GetLockDirection<false>(t); }
/** @copydoc GetLockDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetLockDirection(GenericTileIndex t) { return GetLockDirection<true>(t); }

/**
 * Get the part of a lock.
 * @param t Water tile to query.
 * @return The part.
 * @pre IsTileType(t, MP_WATER) && IsLock(t)
 */
template <bool Tgeneric>
static inline byte GetLockPart(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsLock(t));
	return GB(GetTile(t)->m5, WBL_LOCK_PART_BEGIN, WBL_LOCK_PART_COUNT);
}
/** @copydoc GetLockPart(TileIndexT<Tgeneric>::T) */
static inline byte GetLockPart(TileIndex t) { return GetLockPart<false>(t); }
/** @copydoc GetLockPart(TileIndexT<Tgeneric>::T) */
static inline byte GetLockPart(GenericTileIndex t) { return GetLockPart<true>(t); }

/**
 * Get the random bits of the water tile.
 * @param t Water tile to query.
 * @return Random bits of the tile.
 * @pre IsTileType(t, MP_WATER)
 */
static inline byte GetWaterTileRandomBits(TileIndex t)
{
	assert(IsTileType(t, MP_WATER));
	return GetTile(t)->m4;
}

/**
 * Checks whether the tile has water at the ground.
 * That is, it is either some plain water tile, or a object/industry/station/... with water under it.
 * @return true iff the tile has water at the ground.
 * @note Coast tiles are not considered waterish, even if there is water on a halftile.
 */
static inline bool HasTileWaterGround(TileIndex t)
{
	return HasTileWaterClass(t) && IsTileOnWater(t) && !IsCoastTile(t);
}


/**
 * Helper function to make a coast tile.
 * @param t The tile to change into water
 */
static inline void MakeShore(TileIndex t)
{
	SetTileType(t, MP_WATER);
	SetTileOwner(t, OWNER_WATER);
	SetWaterClass(t, WATER_CLASS_SEA);
	GetTile(t)->m2 = 0;
	GetTile(t)->m3 = 0;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = WBL_TYPE_NORMAL << WBL_TYPE_BEGIN | 1 << WBL_COAST_FLAG;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = 0;
}

/**
 * Helper function for making a watery tile.
 * @param t The tile to change into water
 * @param o The owner of the water
 * @param wc The class of water the tile has to be
 * @param random_bits Eventual random bits to be set for this tile
 */
template <bool Tgeneric>
static inline void MakeWater(typename TileIndexT<Tgeneric>::T t, Owner o, WaterClass wc, uint8 random_bits)
{
	SetTileType(t, MP_WATER);
	SetTileOwner(t, o);
	SetWaterClass(t, wc);
	GetTile(t)->m2 = 0;
	GetTile(t)->m3 = 0;
	GetTile(t)->m4 = random_bits;
	GetTile(t)->m5 = WBL_TYPE_NORMAL << WBL_TYPE_BEGIN;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = 0;
}
/** @copydoc MakeWater(TileIndexT<Tgeneric>::T,Owner,WaterClass,uint8) */
static inline void MakeWater(TileIndex t, Owner o, WaterClass wc, uint8 random_bits) { MakeWater<false>(t, o, wc, random_bits); }
/** @copydoc MakeWater(TileIndexT<Tgeneric>::T,Owner,WaterClass,uint8) */
static inline void MakeWater(GenericTileIndex t, Owner o, WaterClass wc, uint8 random_bits) { MakeWater<true>(t, o, wc, random_bits); }

/**
 * Make a sea tile.
 * @param t The tile to change into sea
 */
static inline void MakeSea(TileIndex t)
{
	MakeWater(t, OWNER_WATER, WATER_CLASS_SEA, 0);
}

/**
 * Make a river tile
 * @param t The tile to change into river
 * @param random_bits Random bits to be set for this tile
 */
static inline void MakeRiver(TileIndex t, uint8 random_bits)
{
	MakeWater(t, OWNER_WATER, WATER_CLASS_RIVER, random_bits);
}

/**
 * Make a canal tile
 * @param t The tile to change into canal
 * @param o The owner of the canal
 * @param random_bits Random bits to be set for this tile
 */
template <bool Tgeneric>
static inline void MakeCanal(typename TileIndexT<Tgeneric>::T t, Owner o, uint8 random_bits)
{
	assert(o != OWNER_WATER);
	MakeWater(t, o, WATER_CLASS_CANAL, random_bits);
}
/** @copydoc MakeCanal(TileIndexT<Tgeneric>::T,Owner,uint) */
static inline void MakeCanal(TileIndex t, Owner o, uint8 random_bits) { MakeCanal<false>(t, o, random_bits); }
/** @copydoc MakeCanal(TileIndexT<Tgeneric>::T,Owner,uint) */
static inline void MakeCanal(GenericTileIndex t, Owner o, uint8 random_bits) { MakeCanal<true>(t, o, random_bits); }

/**
 * Make a ship depot section.
 * @param t    Tile to place the ship depot section.
 * @param o    Owner of the depot.
 * @param did  Depot ID.
 * @param part Depot part (either #DEPOT_PART_NORTH or #DEPOT_PART_SOUTH).
 * @param a    Axis of the depot.
 * @param original_water_class Original water class.
 */
template <bool Tgeneric>
static inline void MakeShipDepot(typename TileIndexT<Tgeneric>::T t, Owner o, DepotID did, DepotPart part, Axis a, WaterClass original_water_class)
{
	SetTileType(t, MP_WATER);
	SetTileOwner(t, o);
	SetWaterClass(t, original_water_class);
	GetTile(t)->m2 = did;
	GetTile(t)->m3 = 0;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = WBL_TYPE_DEPOT << WBL_TYPE_BEGIN | part << WBL_DEPOT_PART | a << WBL_DEPOT_AXIS;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = 0;
}
/** @copydoc MakeShipDepot(TileIndexT<Tgeneric>::T,Owner,DepotID,DepotPart,Axis,WaterClass) */
static inline void MakeShipDepot(TileIndex t, Owner o, DepotID did, DepotPart part, Axis a, WaterClass original_water_class) { MakeShipDepot<false>(t, o, did, part, a, original_water_class); }
/** @copydoc MakeShipDepot(TileIndexT<Tgeneric>::T,Owner,DepotID,DepotPart,Axis,WaterClass) */
static inline void MakeShipDepot(GenericTileIndex t, Owner o, DepotID did, DepotPart part, Axis a, WaterClass original_water_class) { MakeShipDepot<true>(t, o, did, part, a, original_water_class); }

/**
 * Make a lock section.
 * @param t Tile to place the water lock section.
 * @param o Owner of the lock.
 * @param part Part to place.
 * @param dir Lock orientation
 * @param original_water_class Original water class.
 * @see MakeLock
 */
template <bool Tgeneric>
static inline void MakeLockTile(typename TileIndexT<Tgeneric>::T t, Owner o, LockPart part, DiagDirection dir, WaterClass original_water_class)
{
	SetTileType(t, MP_WATER);
	SetTileOwner(t, o);
	SetWaterClass(t, original_water_class);
	GetTile(t)->m2 = 0;
	GetTile(t)->m3 = 0;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = WBL_TYPE_LOCK << WBL_TYPE_BEGIN | part << WBL_LOCK_PART_BEGIN | dir << WBL_LOCK_ORIENT_BEGIN;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = 0;
}
/** @copydoc MakeLockTile(TileIndexT<Tgeneric>::T,Owner,LockPart,DiagDirection,WaterClass) */
static inline void MakeLockTile(TileIndex t, Owner o, LockPart part, DiagDirection dir, WaterClass original_water_class) { MakeLockTile<false>(t, o, part, dir, original_water_class); }
/** @copydoc MakeLockTile(TileIndexT<Tgeneric>::T,Owner,LockPart,DiagDirection,WaterClass) */
static inline void MakeLockTile(GenericTileIndex t, Owner o, LockPart part, DiagDirection dir, WaterClass original_water_class) { MakeLockTile<true>(t, o, part, dir, original_water_class); }

/**
 * Make a water lock.
 * @param t Tile to place the water lock section.
 * @param o Owner of the lock.
 * @param d Direction of the water lock.
 * @param wc_lower Original water class of the lower part.
 * @param wc_upper Original water class of the upper part.
 * @param wc_middle Original water class of the middle part.
 */
template <bool Tgeneric>
static inline void MakeLock(typename TileIndexT<Tgeneric>::T t, Owner o, DiagDirection d, WaterClass wc_lower, WaterClass wc_upper, WaterClass wc_middle)
{
	TileIndexDiff delta = TileOffsByDiagDir<Tgeneric>(d, MapOf(t));

	/* Keep the current waterclass and owner for the tiles.
	 * It allows to restore them after the lock is deleted */
	MakeLockTile(t, o, LOCK_PART_MIDDLE, d, wc_middle);
	MakeLockTile(t - delta, IsWaterTile(t - delta) ? GetTileOwner(t - delta) : o, LOCK_PART_LOWER, d, wc_lower);
	MakeLockTile(t + delta, IsWaterTile(t + delta) ? GetTileOwner(t + delta) : o, LOCK_PART_UPPER, d, wc_upper);
}
/** @copydoc MakeLock(TileIndexT<Tgeneric>::T,Owner,DiagDirection,WaterClass,WaterClass,WaterClass) */
static inline void MakeLock(TileIndex t, Owner o, DiagDirection d, WaterClass wc_lower, WaterClass wc_upper, WaterClass wc_middle) { MakeLock<false>(t, o, d, wc_lower, wc_upper, wc_middle); }
/** @copydoc MakeLock(TileIndexT<Tgeneric>::T,Owner,DiagDirection,WaterClass,WaterClass,WaterClass) */
static inline void MakeLock(GenericTileIndex t, Owner o, DiagDirection d, WaterClass wc_lower, WaterClass wc_upper, WaterClass wc_middle) { MakeLock<true>(t, o, d, wc_lower, wc_upper, wc_middle); }

#endif /* WATER_MAP_H */
