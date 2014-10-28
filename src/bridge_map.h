/* $Id: bridge_map.h 24912 2013-01-13 13:17:12Z frosch $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file bridge_map.h Map accessor functions for bridges. */

#ifndef BRIDGE_MAP_H
#define BRIDGE_MAP_H

#include "road_map.h"
#include "bridge.h"

/**
 * Checks if this is a bridge, instead of a tunnel
 * @param t The tile to analyze
 * @pre IsTileType(t, MP_TUNNELBRIDGE)
 * @return true if the structure is a bridge one
 */
template <bool Tgeneric>
static inline bool IsBridge(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_TUNNELBRIDGE));
	return HasBit(GetTile(t)->m5, 7);
}
/** @copydoc IsBridge(TileIndexT<Tgeneric>::T) */
static inline bool IsBridge(TileIndex t) { return IsBridge<false>(t); }
/** @copydoc IsBridge(TileIndexT<Tgeneric>::T) */
static inline bool IsBridge(GenericTileIndex t) { return IsBridge<true>(t); }

/**
 * checks if there is a bridge on this tile
 * @param t The tile to analyze
 * @return true if a bridge is present
 */
template <bool Tgeneric>
static inline bool IsBridgeTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_TUNNELBRIDGE) && IsBridge(t);
}
/** @copydoc IsBridgeTile(TileIndexT<Tgeneric>::T) */
static inline bool IsBridgeTile(TileIndex t) { return IsBridgeTile<false>(t); }
/** @copydoc IsBridgeTile(TileIndexT<Tgeneric>::T) */
static inline bool IsBridgeTile(GenericTileIndex t) { return IsBridgeTile<true>(t); }

/**
 * checks for the possibility that a bridge may be on this tile
 * These are in fact all the tile types on which a bridge can be found
 * @param t The tile to analyze
 * @return true if a bridge might be present
 */
template <bool Tgeneric>
static inline bool MayHaveBridgeAbove(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_CLEAR) || IsTileType(t, MP_RAILWAY) || IsTileType(t, MP_ROAD) ||
			IsTileType(t, MP_WATER) || IsTileType(t, MP_TUNNELBRIDGE) || IsTileType(t, MP_OBJECT);
}
/** @copydoc MayHaveBridgeAbove(TileIndexT<Tgeneric>::T) */
static inline bool MayHaveBridgeAbove(TileIndex t) { return MayHaveBridgeAbove<false>(t); }
/** @copydoc MayHaveBridgeAbove(TileIndexT<Tgeneric>::T) */
static inline bool MayHaveBridgeAbove(GenericTileIndex t) { return MayHaveBridgeAbove<true>(t); }

/**
 * checks if a bridge is set above the ground of this tile
 * @param t The tile to analyze
 * @pre MayHaveBridgeAbove(t)
 * @return true if a bridge is detected above
 */
template <bool Tgeneric>
static inline bool IsBridgeAbove(typename TileIndexT<Tgeneric>::T t)
{
	assert(MayHaveBridgeAbove(t));
	return GB(GetTileEx(t)->m6, 6, 2) != 0;
}
/** @copydoc IsBridgeAbove(TileIndexT<Tgeneric>::T) */
static inline bool IsBridgeAbove(TileIndex t) { return IsBridgeAbove<false>(t); }
/** @copydoc IsBridgeAbove(TileIndexT<Tgeneric>::T) */
static inline bool IsBridgeAbove(GenericTileIndex t) { return IsBridgeAbove<true>(t); }

/**
 * Determines the type of bridge on a tile
 * @param t The tile to analyze
 * @pre IsBridgeTile(t)
 * @return The bridge type
 */
template <bool Tgeneric>
static inline BridgeType GetBridgeType(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsBridgeTile(t));
	return GB(GetTileEx(t)->m6, 2, 4);
}
/** @copydoc GetBridgeType(TileIndexT<Tgeneric>::T) */
static inline BridgeType GetBridgeType(TileIndex t) { return GetBridgeType<false>(t); }
/** @copydoc GetBridgeType(TileIndexT<Tgeneric>::T) */
static inline BridgeType GetBridgeType(GenericTileIndex t) { return GetBridgeType<true>(t); }

/**
 * Get the axis of the bridge that goes over the tile. Not the axis or the ramp.
 * @param t The tile to analyze
 * @pre IsBridgeAbove(t)
 * @return the above mentioned axis
 */
template <bool Tgeneric>
static inline Axis GetBridgeAxis(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsBridgeAbove(t));
	return (Axis)(GB(GetTileEx(t)->m6, 6, 2) - 1);
}
/** @copydoc GetBridgeAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetBridgeAxis(TileIndex t) { return GetBridgeAxis<false>(t); }
/** @copydoc GetBridgeAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetBridgeAxis(GenericTileIndex t) { return GetBridgeAxis<true>(t); }

TileIndex GetNorthernBridgeEnd(TileIndex t);
TileIndex GetSouthernBridgeEnd(TileIndex t);

template <bool Tgeneric>
typename TileIndexT<Tgeneric>::T GetOtherBridgeEnd(typename TileIndexT<Tgeneric>::T t);
/** @copydoc GetOtherBridgeEnd(TileIndexT<Tgeneric>::T) */
static inline TileIndex GetOtherBridgeEnd(TileIndex t) { return GetOtherBridgeEnd<false>(t); }
/** @copydoc GetOtherBridgeEnd(TileIndexT<Tgeneric>::T) */
static inline GenericTileIndex GetOtherBridgeEnd(GenericTileIndex t) { return GetOtherBridgeEnd<true>(t); }

template <bool Tgeneric>
int GetBridgeHeight(typename TileIndexT<Tgeneric>::T tile);
/** @copydoc GetBridgeHeight(TileIndexT<Tgeneric>::T) */
static inline int GetBridgeHeight(TileIndex t) { return GetBridgeHeight<false>(t); }
/** @copydoc GetBridgeHeight(TileIndexT<Tgeneric>::T) */
static inline int GetBridgeHeight(GenericTileIndex t) { return GetBridgeHeight<true>(t); }

/**
 * Get the height ('z') of a bridge in pixels.
 * @param tile the bridge ramp tile to get the bridge height from
 * @return the height of the bridge in pixels
 */
static inline int GetBridgePixelHeight(TileIndex tile)
{
	return GetBridgeHeight(tile) * TILE_HEIGHT;
}

/**
 * Remove the bridge over the given axis.
 * @param t the tile to remove the bridge from
 * @param a the axis of the bridge to remove
 * @pre MayHaveBridgeAbove(t)
 */
static inline void ClearSingleBridgeMiddle(TileIndex t, Axis a)
{
	assert(MayHaveBridgeAbove(t));
	ClrBit(GetTileEx(t)->m6, 6 + a);
}

/**
 * Removes bridges from the given, that is bridges along the X and Y axis.
 * @param t the tile to remove the bridge from
 * @pre MayHaveBridgeAbove(t)
 */
static inline void ClearBridgeMiddle(TileIndex t)
{
	ClearSingleBridgeMiddle(t, AXIS_X);
	ClearSingleBridgeMiddle(t, AXIS_Y);
}

/**
 * Set that there is a bridge over the given axis.
 * @param t the tile to add the bridge to
 * @param a the axis of the bridge to add
 * @pre MayHaveBridgeAbove(t)
 */
template <bool Tgeneric>
static inline void SetBridgeMiddle(typename TileIndexT<Tgeneric>::T t, Axis a)
{
	assert(MayHaveBridgeAbove(t));
	SetBit(GetTileEx(t)->m6, 6 + a);
}
/** @copydoc SetBridgeMiddle(TileIndexT<Tgeneric>::T,Axis) */
static inline void SetBridgeMiddle(TileIndex t, Axis a) { return SetBridgeMiddle<false>(t, a); }
/** @copydoc SetBridgeMiddle(TileIndexT<Tgeneric>::T,Axis) */
static inline void SetBridgeMiddle(GenericTileIndex t, Axis a) { return SetBridgeMiddle<true>(t, a); }

/**
 * Generic part to make a bridge ramp for both roads and rails.
 * @param t          the tile to make a bridge ramp
 * @param o          the new owner of the bridge ramp
 * @param bridgetype the type of bridge this bridge ramp belongs to
 * @param d          the direction this ramp must be facing
 * @param tt         the transport type of the bridge
 * @param rt         the road or rail type
 * @note this function should not be called directly.
 */
template <bool Tgeneric>
static inline void MakeBridgeRamp(typename TileIndexT<Tgeneric>::T t, Owner o, BridgeType bridgetype, DiagDirection d, TransportType tt, uint rt)
{
	SetTileType(t, MP_TUNNELBRIDGE);
	SetTileOwner(t, o);
	GetTile(t)->m2 = 0;
	GetTile(t)->m3 = rt;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = 1 << 7 | tt << 2 | d;
	SB(GetTileEx(t)->m6, 2, 4, bridgetype);
	GetTileEx(t)->m7 = 0;
}
/** @copydoc MakeBridgeRamp(TileIndexT<Tgeneric>::T,Owner,BridgeType,DiagDirection,TransportType,uint)*/
static inline void MakeBridgeRamp(TileIndex t, Owner o, BridgeType bridgetype, DiagDirection d, TransportType tt, uint rt) { return MakeBridgeRamp<false>(t, o, bridgetype, d, tt, rt); }
/** @copydoc MakeBridgeRamp(TileIndexT<Tgeneric>::T,Owner,BridgeType,DiagDirection,TransportType,uint)*/
static inline void MakeBridgeRamp(GenericTileIndex t, Owner o, BridgeType bridgetype, DiagDirection d, TransportType tt, uint rt) { return MakeBridgeRamp<true>(t, o, bridgetype, d, tt, rt); }

/**
 * Make a bridge ramp for roads.
 * @param t          the tile to make a bridge ramp
 * @param o          the new owner of the bridge ramp
 * @param owner_road the new owner of the road on the bridge
 * @param owner_tram the new owner of the tram on the bridge
 * @param bridgetype the type of bridge this bridge ramp belongs to
 * @param d          the direction this ramp must be facing
 * @param r          the road type of the bridge
 */
template <bool Tgeneric>
static inline void MakeRoadBridgeRamp(typename TileIndexT<Tgeneric>::T t, Owner o, Owner owner_road, Owner owner_tram, BridgeType bridgetype, DiagDirection d, RoadTypes r)
{
	MakeBridgeRamp(t, o, bridgetype, d, TRANSPORT_ROAD, 0);
	SetRoadOwner(t, ROADTYPE_ROAD, owner_road);
	if (owner_tram != OWNER_TOWN) SetRoadOwner(t, ROADTYPE_TRAM, owner_tram);
	SetRoadTypes(t, r);
}
/** @copydoc MakeRoadBridgeRamp(TileIndexT<Tgeneric>::T,Owner,Owner,Owner,BridgeType,DiagDirection,RoadTypes) */
static inline void MakeRoadBridgeRamp(TileIndex t, Owner o, Owner owner_road, Owner owner_tram, BridgeType bridgetype, DiagDirection d, RoadTypes r) { return MakeRoadBridgeRamp<false>(t, o, owner_road, owner_tram, bridgetype, d, r); }
/** @copydoc MakeRoadBridgeRamp(TileIndexT<Tgeneric>::T,Owner,Owner,Owner,BridgeType,DiagDirection,RoadTypes) */
static inline void MakeRoadBridgeRamp(GenericTileIndex t, Owner o, Owner owner_road, Owner owner_tram, BridgeType bridgetype, DiagDirection d, RoadTypes r) { return MakeRoadBridgeRamp<true>(t, o, owner_road, owner_tram, bridgetype, d, r); }

/**
 * Make a bridge ramp for rails.
 * @param t          the tile to make a bridge ramp
 * @param o          the new owner of the bridge ramp
 * @param bridgetype the type of bridge this bridge ramp belongs to
 * @param d          the direction this ramp must be facing
 * @param r          the rail type of the bridge
 */
template <bool Tgeneric>
static inline void MakeRailBridgeRamp(typename TileIndexT<Tgeneric>::T t, Owner o, BridgeType bridgetype, DiagDirection d, RailType r)
{
	MakeBridgeRamp(t, o, bridgetype, d, TRANSPORT_RAIL, r);
}
/** @copydoc MakeRailBridgeRamp(TileIndexT<Tgeneric>::T,Owner,BridgeType,DiagDirection,RailType) */
static inline void MakeRailBridgeRamp(TileIndex t, Owner o, BridgeType bridgetype, DiagDirection d, RailType r) { return MakeRailBridgeRamp<false>(t, o, bridgetype, d, r); }
/** @copydoc MakeRailBridgeRamp(TileIndexT<Tgeneric>::T,Owner,BridgeType,DiagDirection,RailType) */
static inline void MakeRailBridgeRamp(GenericTileIndex t, Owner o, BridgeType bridgetype, DiagDirection d, RailType r) { return MakeRailBridgeRamp<true>(t, o, bridgetype, d, r); }

/**
 * Make a bridge ramp for aqueducts.
 * @param t          the tile to make a bridge ramp
 * @param o          the new owner of the bridge ramp
 * @param d          the direction this ramp must be facing
 */
template <bool Tgeneric>
static inline void MakeAqueductBridgeRamp(typename TileIndexT<Tgeneric>::T t, Owner o, DiagDirection d)
{
	MakeBridgeRamp(t, o, 0, d, TRANSPORT_WATER, 0);
}
/** @copydoc MakeAqueductBridgeRamp(TileIndexT<Tgeneric>::T,Owner,DiagDirection) */
static inline void MakeAqueductBridgeRamp(TileIndex t, Owner o, DiagDirection d) { return MakeAqueductBridgeRamp<false>(t, o, d); }
/** @copydoc MakeAqueductBridgeRamp(TileIndexT<Tgeneric>::T,Owner,DiagDirection) */
static inline void MakeAqueductBridgeRamp(GenericTileIndex t, Owner o, DiagDirection d) { return MakeAqueductBridgeRamp<true>(t, o, d); }

#endif /* BRIDGE_MAP_H */
