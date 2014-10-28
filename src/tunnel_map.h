/* $Id: tunnel_map.h 23167 2011-11-08 19:44:41Z rubidium $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file tunnel_map.h Map accessors for tunnels. */

#ifndef TUNNEL_MAP_H
#define TUNNEL_MAP_H

#include "road_map.h"


/**
 * Is this a tunnel (entrance)?
 * @param t the tile that might be a tunnel
 * @pre IsTileType(t, MP_TUNNELBRIDGE)
 * @return true if and only if this tile is a tunnel (entrance)
 */
template <bool Tgeneric>
static inline bool IsTunnel(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_TUNNELBRIDGE));
	return !HasBit(GetTile(t)->m5, 7);
}
/** @copydoc IsTunnel(TileIndexT<Tgeneric>::T) */
static inline bool IsTunnel(TileIndex t) { return IsTunnel<false>(t); }
/** @copydoc IsTunnel(TileIndexT<Tgeneric>::T) */
static inline bool IsTunnel(GenericTileIndex t) { return IsTunnel<true>(t); }

/**
 * Is this a tunnel (entrance)?
 * @param t the tile that might be a tunnel
 * @return true if and only if this tile is a tunnel (entrance)
 */
template <bool Tgeneric>
static inline bool IsTunnelTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_TUNNELBRIDGE) && IsTunnel(t);
}
/** @copydoc IsTunnelTile(TileIndexT<Tgeneric>::T) */
static inline bool IsTunnelTile(TileIndex t) { return IsTunnelTile<false>(t); }
/** @copydoc IsTunnelTile(TileIndexT<Tgeneric>::T) */
static inline bool IsTunnelTile(GenericTileIndex t) { return IsTunnelTile<true>(t); }

template <bool Tgeneric>
typename TileIndexT<Tgeneric>::T GetOtherTunnelEnd(typename TileIndexT<Tgeneric>::T t);
/** @copydoc GetOtherTunnelEnd(TileIndexT<Tgeneric>::T) */
static inline TileIndex GetOtherTunnelEnd(TileIndex t) { return GetOtherTunnelEnd<false>(t); }
/** @copydoc GetOtherTunnelEnd(TileIndexT<Tgeneric>::T) */
static inline GenericTileIndex GetOtherTunnelEnd(GenericTileIndex t) { return GetOtherTunnelEnd<true>(t); }

bool IsTunnelInWay(TileIndex, int z);
bool IsTunnelInWayDir(TileIndex tile, int z, DiagDirection dir);

/**
 * Makes a road tunnel entrance
 * @param t the entrance of the tunnel
 * @param o the owner of the entrance
 * @param d the direction facing out of the tunnel
 * @param r the road type used in the tunnel
 */
template <bool Tgeneric>
static inline void MakeRoadTunnel(typename TileIndexT<Tgeneric>::T t, Owner o, DiagDirection d, RoadTypes r)
{
	SetTileType(t, MP_TUNNELBRIDGE);
	SetTileOwner(t, o);
	GetTile(t)->m2 = 0;
	GetTile(t)->m3 = 0;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = TRANSPORT_ROAD << 2 | d;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = 0;
	SetRoadOwner(t, ROADTYPE_ROAD, o);
	if (o != OWNER_TOWN) SetRoadOwner(t, ROADTYPE_TRAM, o);
	SetRoadTypes(t, r);
}
/** @copydoc MakeRoadTunnel(TileIndexT<Tgeneric>::T,Owner,DiagDirection,RoadTypes) */
static inline void MakeRoadTunnel(TileIndex t, Owner o, DiagDirection d, RoadTypes r) { MakeRoadTunnel<false>(t, o, d, r); }
/** @copydoc MakeRoadTunnel(TileIndexT<Tgeneric>::T,Owner,DiagDirection,RoadTypes) */
static inline void MakeRoadTunnel(GenericTileIndex t, Owner o, DiagDirection d, RoadTypes r) { MakeRoadTunnel<true>(t, o, d, r); }

/**
 * Makes a rail tunnel entrance
 * @param t the entrance of the tunnel
 * @param o the owner of the entrance
 * @param d the direction facing out of the tunnel
 * @param r the rail type used in the tunnel
 */
template <bool Tgeneric>
static inline void MakeRailTunnel(typename TileIndexT<Tgeneric>::T t, Owner o, DiagDirection d, RailType r)
{
	SetTileType(t, MP_TUNNELBRIDGE);
	SetTileOwner(t, o);
	GetTile(t)->m2 = 0;
	GetTile(t)->m3 = r;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = TRANSPORT_RAIL << 2 | d;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = 0;
}
/** @copydoc MakeRailTunnel(TileIndexT<Tgeneric>::T,Owner,DiagDirection,RailType) */
static inline void MakeRailTunnel(TileIndex t, Owner o, DiagDirection d, RailType r) { MakeRailTunnel<false>(t, o, d, r); }
/** @copydoc MakeRailTunnel(TileIndexT<Tgeneric>::T,Owner,DiagDirection,RailType) */
static inline void MakeRailTunnel(GenericTileIndex t, Owner o, DiagDirection d, RailType r) { MakeRailTunnel<true>(t, o, d, r); }

#endif /* TUNNEL_MAP_H */
