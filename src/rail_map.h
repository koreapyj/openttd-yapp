/* $Id: rail_map.h 24367 2012-07-01 23:12:50Z michi_cc $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file rail_map.h Hides the direct accesses to the map array with map accessors */

#ifndef RAIL_MAP_H
#define RAIL_MAP_H

#include "rail_type.h"
#include "depot_type.h"
#include "signal_func.h"
#include "track_func.h"
#include "tile_map.h"
#include "signal_type.h"


/** Different types of Rail-related tiles */
enum RailTileType {
	RAIL_TILE_NORMAL   = 0, ///< Normal rail tile without signals
	RAIL_TILE_SIGNALS  = 1, ///< Normal rail tile with signals
	RAIL_TILE_DEPOT    = 3, ///< Depot (one entrance)
};

/**
 * Returns the RailTileType (normal with or without signals,
 * waypoint or depot).
 * @param t the tile to get the information from
 * @pre IsTileType(t, MP_RAILWAY)
 * @return the RailTileType
 */
template <bool Tgeneric>
static inline RailTileType GetRailTileType(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_RAILWAY));
	return (RailTileType)GB(GetTile(t)->m5, 6, 2);
}
/** @copydoc GetRailTileType(TileIndexT<Tgeneric>::T) */
static inline RailTileType GetRailTileType(TileIndex t) { return GetRailTileType<false>(t); }
/** @copydoc GetRailTileType(TileIndexT<Tgeneric>::T) */
static inline RailTileType GetRailTileType(GenericTileIndex t) { return GetRailTileType<true>(t); }

/**
 * Returns whether this is plain rails, with or without signals. Iow, if this
 * tiles RailTileType is RAIL_TILE_NORMAL or RAIL_TILE_SIGNALS.
 * @param t the tile to get the information from
 * @pre IsTileType(t, MP_RAILWAY)
 * @return true if and only if the tile is normal rail (with or without signals)
 */
template <bool Tgeneric>
static inline bool IsPlainRail(typename TileIndexT<Tgeneric>::T t)
{
	RailTileType rtt = GetRailTileType(t);
	return rtt == RAIL_TILE_NORMAL || rtt == RAIL_TILE_SIGNALS;
}
/** @copydoc IsPlainRail(TileIndexT<Tgeneric>::T) */
static inline bool IsPlainRail(TileIndex t) { return IsPlainRail<false>(t); }
/** @copydoc IsPlainRail(TileIndexT<Tgeneric>::T) */
static inline bool IsPlainRail(GenericTileIndex t) { return IsPlainRail<true>(t); }

/**
 * Checks whether the tile is a rail tile or rail tile with signals.
 * @param t the tile to get the information from
 * @return true if and only if the tile is normal rail (with or without signals)
 */
template <bool Tgeneric>
static inline bool IsPlainRailTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_RAILWAY) && IsPlainRail(t);
}
/** @copydoc IsPlainRailTile(TileIndexT<Tgeneric>::T) */
static inline bool IsPlainRailTile(TileIndex t) { return IsPlainRailTile<false>(t); }
/** @copydoc IsPlainRailTile(TileIndexT<Tgeneric>::T) */
static inline bool IsPlainRailTile(GenericTileIndex t) { return IsPlainRailTile<true>(t); }


/**
 * Checks if a rail tile has signals.
 * @param t the tile to get the information from
 * @pre IsTileType(t, MP_RAILWAY)
 * @return true if and only if the tile has signals
 */
static inline bool HasSignals(TileIndex t)
{
	return GetRailTileType(t) == RAIL_TILE_SIGNALS;
}

/**
 * Add/remove the 'has signal' bit from the RailTileType
 * @param tile the tile to add/remove the signals to/from
 * @param signals whether the rail tile should have signals or not
 * @pre IsPlainRailTile(tile)
 */
template <bool Tgeneric>
static inline void SetHasSignals(typename TileIndexT<Tgeneric>::T tile, bool signals)
{
	assert(IsPlainRailTile(tile));
	SB(GetTile(tile)->m5, 6, 1, signals);
}
/** @copydoc SetHasSignals(TileIndexT<Tgeneric>::T,bool) */
static inline void SetHasSignals(TileIndex tile, bool signals) { SetHasSignals<false>(tile, signals); }
/** @copydoc SetHasSignals(TileIndexT<Tgeneric>::T,bool) */
static inline void SetHasSignals(GenericTileIndex tile, bool signals) { SetHasSignals<true>(tile, signals); }

/**
 * Is this rail tile a rail depot?
 * @param t the tile to get the information from
 * @pre IsTileType(t, MP_RAILWAY)
 * @return true if and only if the tile is a rail depot
 */
template <bool Tgeneric>
static inline bool IsRailDepot(typename TileIndexT<Tgeneric>::T t)
{
	return GetRailTileType(t) == RAIL_TILE_DEPOT;
}
/** @copydoc IsRailDepot(TileIndexT<Tgeneric>::T) */
static inline bool IsRailDepot(TileIndex t) { return IsRailDepot<false>(t); }
/** @copydoc IsRailDepot(TileIndexT<Tgeneric>::T) */
static inline bool IsRailDepot(GenericTileIndex t) { return IsRailDepot<true>(t); }

/**
 * Is this tile rail tile and a rail depot?
 * @param t the tile to get the information from
 * @return true if and only if the tile is a rail depot
 */
template <bool Tgeneric>
static inline bool IsRailDepotTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_RAILWAY) && IsRailDepot(t);
}
/** @copydoc IsRailDepotTile(TileIndexT<Tgeneric>::T) */
static inline bool IsRailDepotTile(TileIndex t) { return IsRailDepotTile<false>(t); }
/** @copydoc IsRailDepotTile(TileIndexT<Tgeneric>::T) */
static inline bool IsRailDepotTile(GenericTileIndex t) { return IsRailDepotTile<true>(t); }

/**
 * Gets the rail type of the given tile
 * @param t the tile to get the rail type from
 * @return the rail type of the tile
 */
template <bool Tgeneric>
static inline RailType GetRailType(typename TileIndexT<Tgeneric>::T t)
{
	return (RailType)GB(GetTile(t)->m3, 0, 4);
}
/** @copydoc GetRailType(TileIndexT<Tgeneric>::T) */
static inline RailType GetRailType(TileIndex t) { return GetRailType<false>(t); }
/** @copydoc GetRailType(TileIndexT<Tgeneric>::T) */
static inline RailType GetRailType(GenericTileIndex t) { return GetRailType<true>(t); }

/**
 * Sets the rail type of the given tile
 * @param t the tile to set the rail type of
 * @param r the new rail type for the tile
 */
template <bool Tgeneric>
static inline void SetRailType(typename TileIndexT<Tgeneric>::T t, RailType r)
{
	SB(GetTile(t)->m3, 0, 4, r);
}
/** @copydoc SetRailType(TileIndexT<Tgeneric>::T,RailType) */
static inline void SetRailType(TileIndex t, RailType r) { SetRailType<false>(t, r); }
/** @copydoc SetRailType(TileIndexT<Tgeneric>::T,RailType) */
static inline void SetRailType(GenericTileIndex t, RailType r) { SetRailType<true>(t, r); }


/**
 * Gets the track bits of the given tile
 * @param tile the tile to get the track bits from
 * @return the track bits of the tile
 */
template <bool Tgeneric>
static inline TrackBits GetTrackBits(typename TileIndexT<Tgeneric>::T tile)
{
	assert(IsPlainRailTile(tile));
	return (TrackBits)GB(GetTile(tile)->m5, 0, 6);
}
/** @copydoc GetTrackBits(TileIndexT<Tgeneric>::T) */
static inline TrackBits GetTrackBits(TileIndex tile) { return GetTrackBits<false>(tile); }
/** @copydoc GetTrackBits(TileIndexT<Tgeneric>::T) */
static inline TrackBits GetTrackBits(GenericTileIndex tile) { return GetTrackBits<true>(tile); }

/**
 * Sets the track bits of the given tile
 * @param t the tile to set the track bits of
 * @param b the new track bits for the tile
 */
template <bool Tgeneric>
static inline void SetTrackBits(typename TileIndexT<Tgeneric>::T t, TrackBits b)
{
	assert(IsPlainRailTile(t));
	SB(GetTile(t)->m5, 0, 6, b);
}
/** @copydoc SetTrackBits(TileIndexT<Tgeneric>::T,TrackBits) */
static inline void SetTrackBits(TileIndex t, TrackBits b) { SetTrackBits<false>(t, b); }
/** @copydoc SetTrackBits(TileIndexT<Tgeneric>::T,TrackBits) */
static inline void SetTrackBits(GenericTileIndex t, TrackBits b) { SetTrackBits<true>(t, b); }

/**
 * Returns whether the given track is present on the given tile.
 * @param tile  the tile to check the track presence of
 * @param track the track to search for on the tile
 * @pre IsPlainRailTile(tile)
 * @return true if and only if the given track exists on the tile
 */
template <bool Tgeneric>
static inline bool HasTrack(typename TileIndexT<Tgeneric>::T tile, Track track)
{
	return HasBit(GetTrackBits(tile), track);
}
/** @copydoc HasTrack(TileIndexT<Tgeneric>::T,Track) */
static inline bool HasTrack(TileIndex tile, Track track) { return HasTrack<false>(tile, track); }
/** @copydoc HasTrack(TileIndexT<Tgeneric>::T,Track) */
static inline bool HasTrack(GenericTileIndex tile, Track track) { return HasTrack<true>(tile, track); }

/**
 * Returns the direction the depot is facing to
 * @param t the tile to get the depot facing from
 * @pre IsRailDepotTile(t)
 * @return the direction the depot is facing
 */
template <bool Tgeneric>
static inline DiagDirection GetRailDepotDirection(typename TileIndexT<Tgeneric>::T t)
{
	return (DiagDirection)GB(GetTile(t)->m5, 0, 2);
}
/** @copydoc GetRailDepotDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetRailDepotDirection(TileIndex t) { return GetRailDepotDirection<false>(t); }
/** @copydoc GetRailDepotDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetRailDepotDirection(GenericTileIndex t) { return GetRailDepotDirection<true>(t); }

/**
 * Returns the track of a depot, ignoring direction
 * @pre IsRailDepotTile(t)
 * @param t the tile to get the depot track from
 * @return the track of the depot
 */
template <bool Tgeneric>
static inline Track GetRailDepotTrack(typename TileIndexT<Tgeneric>::T t)
{
	return DiagDirToDiagTrack(GetRailDepotDirection(t));
}
/** @copydoc GetRailDepotTrack(TileIndexT<Tgeneric>::T) */
static inline Track GetRailDepotTrack(TileIndex t) { return GetRailDepotTrack<false>(t); }
/** @copydoc GetRailDepotTrack(TileIndexT<Tgeneric>::T) */
static inline Track GetRailDepotTrack(GenericTileIndex t) { return GetRailDepotTrack<true>(t); }


/**
 * Returns the reserved track bits of the tile
 * @pre IsPlainRailTile(t)
 * @param t the tile to query
 * @return the track bits
 */
static inline TrackBits GetRailReservationTrackBits(TileIndex t)
{
	assert(IsPlainRailTile(t));
	byte track_b = GB(GetTile(t)->m2, 8, 3);
	Track track = (Track)(track_b - 1);    // map array saves Track+1
	if (track_b == 0) return TRACK_BIT_NONE;
	return (TrackBits)(TrackToTrackBits(track) | (HasBit(GetTile(t)->m2, 11) ? TrackToTrackBits(TrackToOppositeTrack(track)) : 0));
}

/**
 * Sets the reserved track bits of the tile
 * @pre IsPlainRailTile(t) && !TracksOverlap(b)
 * @param t the tile to change
 * @param b the track bits
 */
static inline void SetTrackReservation(TileIndex t, TrackBits b)
{
	assert(IsPlainRailTile(t));
	assert(b != INVALID_TRACK_BIT);
	assert(!TracksOverlap(b));
	Track track = RemoveFirstTrack(&b);
	SB(GetTile(t)->m2, 8, 3, track == INVALID_TRACK ? 0 : track + 1);
	SB(GetTile(t)->m2, 11, 1, (byte)(b != TRACK_BIT_NONE));
}

/**
 * Try to reserve a specific track on a tile
 * @pre IsPlainRailTile(t) && HasTrack(tile, t)
 * @param tile the tile
 * @param t the rack to reserve
 * @return true if successful
 */
static inline bool TryReserveTrack(TileIndex tile, Track t)
{
	assert(HasTrack(tile, t));
	TrackBits bits = TrackToTrackBits(t);
	TrackBits res = GetRailReservationTrackBits(tile);
	if ((res & bits) != TRACK_BIT_NONE) return false;  // already reserved
	res |= bits;
	if (TracksOverlap(res)) return false;  // crossing reservation present
	SetTrackReservation(tile, res);
	return true;
}

/**
 * Lift the reservation of a specific track on a tile
 * @pre IsPlainRailTile(t) && HasTrack(tile, t)
 * @param tile the tile
 * @param t the track to free
 */
static inline void UnreserveTrack(TileIndex tile, Track t)
{
	assert(HasTrack(tile, t));
	TrackBits res = GetRailReservationTrackBits(tile);
	res &= ~TrackToTrackBits(t);
	SetTrackReservation(tile, res);
}

/**
 * Get the reservation state of the depot
 * @pre IsRailDepot(t)
 * @param t the depot tile
 * @return reservation state
 */
static inline bool HasDepotReservation(TileIndex t)
{
	assert(IsRailDepot(t));
	return HasBit(GetTile(t)->m5, 4);
}

/**
 * Set the reservation state of the depot
 * @pre IsRailDepot(t)
 * @param t the depot tile
 * @param b the reservation state
 */
static inline void SetDepotReservation(TileIndex t, bool b)
{
	assert(IsRailDepot(t));
	SB(GetTile(t)->m5, 4, 1, (byte)b);
}

/**
 * Get the reserved track bits for a depot
 * @pre IsRailDepot(t)
 * @param t the tile
 * @return reserved track bits
 */
static inline TrackBits GetDepotReservationTrackBits(TileIndex t)
{
	return HasDepotReservation(t) ? TrackToTrackBits(GetRailDepotTrack(t)) : TRACK_BIT_NONE;
}


static inline bool IsPbsSignal(SignalType s)
{
	return s == SIGTYPE_PBS || s == SIGTYPE_PBS_ONEWAY;
}

template <bool Tgeneric>
static inline SignalType GetSignalType(typename TileIndexT<Tgeneric>::T t, Track track)
{
	assert(GetRailTileType(t) == RAIL_TILE_SIGNALS);
	byte pos = (track == TRACK_LOWER || track == TRACK_RIGHT) ? 4 : 0;
	return (SignalType)GB(GetTile(t)->m2, pos, 3);
}
/** @copydoc GetSignalType(TileIndexT<Tgeneric>::T,Track) */
static inline SignalType GetSignalType(TileIndex t, Track track) { return GetSignalType<false>(t, track); }
/** @copydoc GetSignalType(TileIndexT<Tgeneric>::T,Track) */
static inline SignalType GetSignalType(GenericTileIndex t, Track track) { return GetSignalType<true>(t, track); }

template <bool Tgeneric>
static inline void SetSignalType(typename TileIndexT<Tgeneric>::T t, Track track, SignalType s)
{
	assert(GetRailTileType(t) == RAIL_TILE_SIGNALS);
	byte pos = (track == TRACK_LOWER || track == TRACK_RIGHT) ? 4 : 0;
	SB(GetTile(t)->m2, pos, 3, s);
	if (track == INVALID_TRACK) SB(GetTile(t)->m2, 4, 3, s);
}
/** @copydoc SetSignalType(TileIndexT<Tgeneric>::T,Track,SignalType) */
static inline void SetSignalType(TileIndex t, Track track, SignalType s) { SetSignalType<false>(t, track, s); }
/** @copydoc SetSignalType(TileIndexT<Tgeneric>::T,Track,SignalType) */
static inline void SetSignalType(GenericTileIndex t, Track track, SignalType s) { SetSignalType<true>(t, track, s); }

static inline bool IsPresignalEntry(TileIndex t, Track track)
{
	return GetSignalType(t, track) == SIGTYPE_ENTRY || GetSignalType(t, track) == SIGTYPE_COMBO;
}

static inline bool IsPresignalExit(TileIndex t, Track track)
{
	return GetSignalType(t, track) == SIGTYPE_EXIT || GetSignalType(t, track) == SIGTYPE_COMBO;
}

/** One-way signals can't be passed the 'wrong' way. */
static inline bool IsOnewaySignal(TileIndex t, Track track)
{
	return GetSignalType(t, track) != SIGTYPE_PBS;
}

template <bool Tgeneric>
static inline void CycleSignalSide(typename TileIndexT<Tgeneric>::T t, Track track)
{
	byte sig;
	byte pos = (track == TRACK_LOWER || track == TRACK_RIGHT) ? 4 : 6;

	sig = GB(GetTile(t)->m3, pos, 2);
	if (--sig == 0) sig = IsPbsSignal(GetSignalType(t, track)) ? 2 : 3;
	SB(GetTile(t)->m3, pos, 2, sig);
}
/** @copydoc CycleSignalSide(TileIndexT<Tgeneric>::T,Track) */
static inline void CycleSignalSide(TileIndex t, Track track) { CycleSignalSide<false>(t, track); }
/** @copydoc CycleSignalSide(TileIndexT<Tgeneric>::T,Track) */
static inline void CycleSignalSide(GenericTileIndex t, Track track) { CycleSignalSide<true>(t, track); }

template <bool Tgeneric>
static inline SignalVariant GetSignalVariant(typename TileIndexT<Tgeneric>::T t, Track track)
{
	byte pos = (track == TRACK_LOWER || track == TRACK_RIGHT) ? 7 : 3;
	return (SignalVariant)GB(GetTile(t)->m2, pos, 1);
}
/** @copydoc GetSignalVariant(TileIndexT<Tgeneric>::T,Track) */
static inline SignalVariant GetSignalVariant(TileIndex t, Track track) { return GetSignalVariant<false>(t, track); }
/** @copydoc GetSignalVariant(TileIndexT<Tgeneric>::T,Track) */
static inline SignalVariant GetSignalVariant(GenericTileIndex t, Track track) { return GetSignalVariant<true>(t, track); }

template <bool Tgeneric>
static inline void SetSignalVariant(typename TileIndexT<Tgeneric>::T t, Track track, SignalVariant v)
{
	byte pos = (track == TRACK_LOWER || track == TRACK_RIGHT) ? 7 : 3;
	SB(GetTile(t)->m2, pos, 1, v);
	if (track == INVALID_TRACK) SB(GetTile(t)->m2, 7, 1, v);
}
/** @copydoc SetSignalVariant(TileIndexT<Tgeneric>::T,Track,SignalVariant) */
static inline void SetSignalVariant(TileIndex t, Track track, SignalVariant v) { SetSignalVariant<false>(t, track, v); }
/** @copydoc SetSignalVariant(TileIndexT<Tgeneric>::T,Track,SignalVariant) */
static inline void SetSignalVariant(GenericTileIndex t, Track track, SignalVariant v) { SetSignalVariant<true>(t, track, v); }

/**
 * Set the states of the signals (Along/AgainstTrackDir)
 * @param tile  the tile to set the states for
 * @param state the new state
 */
static inline void SetSignalStates(TileIndex tile, uint state)
{
	SB(GetTile(tile)->m4, 4, 4, state);
}

/**
 * Set the states of the signals (Along/AgainstTrackDir)
 * @param tile  the tile to set the states for
 * @return the state of the signals
 */
static inline uint GetSignalStates(TileIndex tile)
{
	return GB(GetTile(tile)->m4, 4, 4);
}

/**
 * Get the state of a single signal
 * @param t         the tile to get the signal state for
 * @param signalbit the signal
 * @return the state of the signal
 */
static inline SignalState GetSingleSignalState(TileIndex t, byte signalbit)
{
	return (SignalState)HasBit(GetSignalStates(t), signalbit);
}

/**
 * Set whether the given signals are present (Along/AgainstTrackDir)
 * @param tile    the tile to set the present signals for
 * @param signals the signals that have to be present
 */
template <bool Tgeneric>
static inline void SetPresentSignals(typename TileIndexT<Tgeneric>::T tile, uint signals)
{
	SB(GetTile(tile)->m3, 4, 4, signals);
}
/** @copydoc SetPresentSignals(TileIndexT<Tgeneric>::T,uint) */
static inline void SetPresentSignals(TileIndex tile, uint signals) { SetPresentSignals<false>(tile, signals); }
/** @copydoc SetPresentSignals(TileIndexT<Tgeneric>::T,uint) */
static inline void SetPresentSignals(GenericTileIndex tile, uint signals) { SetPresentSignals<true>(tile, signals); }

/**
 * Get whether the given signals are present (Along/AgainstTrackDir)
 * @param tile the tile to get the present signals for
 * @return the signals that are present
 */
template <bool Tgeneric>
static inline uint GetPresentSignals(typename TileIndexT<Tgeneric>::T tile)
{
	return GB(GetTile(tile)->m3, 4, 4);
}
/** @copydoc GetPresentSignals(TileIndexT<Tgeneric>::T) */
static inline uint GetPresentSignals(TileIndex tile) { return GetPresentSignals<false>(tile); }
/** @copydoc GetPresentSignals(TileIndexT<Tgeneric>::T) */
static inline uint GetPresentSignals(GenericTileIndex tile) { return GetPresentSignals<true>(tile); }

/**
 * Checks whether the given signals is present
 * @param t         the tile to check on
 * @param signalbit the signal
 * @return true if and only if the signal is present
 */
static inline bool IsSignalPresent(TileIndex t, byte signalbit)
{
	return HasBit(GetPresentSignals(t), signalbit);
}

/**
 * Checks for the presence of signals (either way) on the given track on the
 * given rail tile.
 */
template <bool Tgeneric>
static inline bool HasSignalOnTrack(typename TileIndexT<Tgeneric>::T tile, Track track)
{
	assert(IsValidTrack(track));
	return GetRailTileType(tile) == RAIL_TILE_SIGNALS && (GetPresentSignals(tile) & SignalOnTrack(track)) != 0;
}
/** @copydoc HasSignalOnTrack(TileIndexT<Tgeneric>::T,Track) */
static inline bool HasSignalOnTrack(TileIndex tile, Track track) { return HasSignalOnTrack<false>(tile, track); }
/** @copydoc HasSignalOnTrack(TileIndexT<Tgeneric>::T,Track) */
static inline bool HasSignalOnTrack(GenericTileIndex tile, Track track) { return HasSignalOnTrack<true>(tile, track); }

/**
 * Checks for the presence of signals along the given trackdir on the given
 * rail tile.
 *
 * Along meaning if you are currently driving on the given trackdir, this is
 * the signal that is facing us (for which we stop when it's red).
 */
template <bool Tgeneric>
static inline bool HasSignalOnTrackdir(typename TileIndexT<Tgeneric>::T tile, Trackdir trackdir)
{
	assert(IsValidTrackdir(trackdir));
	return GetRailTileType(tile) == RAIL_TILE_SIGNALS && GetPresentSignals(tile) & SignalAlongTrackdir(trackdir);
}
/** @copydoc HasSignalOnTrackdir(TileIndexT<Tgeneric>::T,Trackdir) */
static inline bool HasSignalOnTrackdir(TileIndex tile, Trackdir trackdir) { return HasSignalOnTrackdir<false>(tile, trackdir); }
/** @copydoc HasSignalOnTrackdir(TileIndexT<Tgeneric>::T,Trackdir) */
static inline bool HasSignalOnTrackdir(GenericTileIndex tile, Trackdir trackdir) { return HasSignalOnTrackdir<true>(tile, trackdir); }

/**
 * Gets the state of the signal along the given trackdir.
 *
 * Along meaning if you are currently driving on the given trackdir, this is
 * the signal that is facing us (for which we stop when it's red).
 */
static inline SignalState GetSignalStateByTrackdir(TileIndex tile, Trackdir trackdir)
{
	assert(IsValidTrackdir(trackdir));
	assert(HasSignalOnTrack(tile, TrackdirToTrack(trackdir)));
	return GetSignalStates(tile) & SignalAlongTrackdir(trackdir) ?
		SIGNAL_STATE_GREEN : SIGNAL_STATE_RED;
}

/**
 * Sets the state of the signal along the given trackdir.
 */
static inline void SetSignalStateByTrackdir(TileIndex tile, Trackdir trackdir, SignalState state)
{
	if (state == SIGNAL_STATE_GREEN) { // set 1
		SetSignalStates(tile, GetSignalStates(tile) | SignalAlongTrackdir(trackdir));
	} else {
		SetSignalStates(tile, GetSignalStates(tile) & ~SignalAlongTrackdir(trackdir));
	}
}

/**
 * Is a pbs signal present along the trackdir?
 * @param tile the tile to check
 * @param td the trackdir to check
 */
static inline bool HasPbsSignalOnTrackdir(TileIndex tile, Trackdir td)
{
	return IsTileType(tile, MP_RAILWAY) && HasSignalOnTrackdir(tile, td) &&
			IsPbsSignal(GetSignalType(tile, TrackdirToTrack(td)));
}

/**
 * Is a one-way signal blocking the trackdir? A one-way signal on the
 * trackdir against will block, but signals on both trackdirs won't.
 * @param tile the tile to check
 * @param td the trackdir to check
 */
static inline bool HasOnewaySignalBlockingTrackdir(TileIndex tile, Trackdir td)
{
	return IsTileType(tile, MP_RAILWAY) && HasSignalOnTrackdir(tile, ReverseTrackdir(td)) &&
			!HasSignalOnTrackdir(tile, td) && IsOnewaySignal(tile, TrackdirToTrack(td));
}


template <bool Tgeneric>
RailType GetTileRailType(typename TileIndexT<Tgeneric>::T tile);
/** @copydoc GetTileRailType(TileIndexT<Tgeneric>::T) */
static inline RailType GetTileRailType(TileIndex tile) { return GetTileRailType<false>(tile); }
/** @copydoc GetTileRailType(TileIndexT<Tgeneric>::T) */
static inline RailType GetTileRailType(GenericTileIndex tile) { return GetTileRailType<true>(tile); }

/** The ground 'under' the rail */
enum RailGroundType {
	RAIL_GROUND_BARREN       =  0, ///< Nothing (dirt)
	RAIL_GROUND_GRASS        =  1, ///< Grassy
	RAIL_GROUND_FENCE_NW     =  2, ///< Grass with a fence at the NW edge
	RAIL_GROUND_FENCE_SE     =  3, ///< Grass with a fence at the SE edge
	RAIL_GROUND_FENCE_SENW   =  4, ///< Grass with a fence at the NW and SE edges
	RAIL_GROUND_FENCE_NE     =  5, ///< Grass with a fence at the NE edge
	RAIL_GROUND_FENCE_SW     =  6, ///< Grass with a fence at the SW edge
	RAIL_GROUND_FENCE_NESW   =  7, ///< Grass with a fence at the NE and SW edges
	RAIL_GROUND_FENCE_VERT1  =  8, ///< Grass with a fence at the eastern side
	RAIL_GROUND_FENCE_VERT2  =  9, ///< Grass with a fence at the western side
	RAIL_GROUND_FENCE_HORIZ1 = 10, ///< Grass with a fence at the southern side
	RAIL_GROUND_FENCE_HORIZ2 = 11, ///< Grass with a fence at the northern side
	RAIL_GROUND_ICE_DESERT   = 12, ///< Icy or sandy
	RAIL_GROUND_WATER        = 13, ///< Grass with a fence and shore or water on the free halftile
	RAIL_GROUND_HALF_SNOW    = 14, ///< Snow only on higher part of slope (steep or one corner raised)
};

static inline void SetRailGroundType(TileIndex t, RailGroundType rgt)
{
	SB(GetTile(t)->m4, 0, 4, rgt);
}

static inline RailGroundType GetRailGroundType(TileIndex t)
{
	return (RailGroundType)GB(GetTile(t)->m4, 0, 4);
}

static inline bool IsSnowRailGround(TileIndex t)
{
	return GetRailGroundType(t) == RAIL_GROUND_ICE_DESERT;
}


template <bool Tgeneric>
static inline void MakeRailNormal(typename TileIndexT<Tgeneric>::T t, Owner o, TrackBits b, RailType r)
{
	SetTileType(t, MP_RAILWAY);
	SetTileOwner(t, o);
	GetTile(t)->m2 = 0;
	GetTile(t)->m3 = r;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = RAIL_TILE_NORMAL << 6 | b;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = 0;
}
/** @copydoc MakeRailNormal(TileIndexT<Tgeneric>::T,Owner,TrackBits,RailType) */
static inline void MakeRailNormal(TileIndex t, Owner o, TrackBits b, RailType r) { MakeRailNormal<false>(t, o, b, r); }
/** @copydoc MakeRailNormal(TileIndexT<Tgeneric>::T,Owner,TrackBits,RailType) */
static inline void MakeRailNormal(GenericTileIndex t, Owner o, TrackBits b, RailType r) { MakeRailNormal<true>(t, o, b, r); }

template <bool Tgeneric>
inline void MakeRailDepot(typename TileIndexT<Tgeneric>::T t, Owner o, DepotID did, DiagDirection d, RailType r)
{
	SetTileType(t, MP_RAILWAY);
	SetTileOwner(t, o);
	GetTile(t)->m2 = did;
	GetTile(t)->m3 = r;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = RAIL_TILE_DEPOT << 6 | d;
	SB(GetTileEx(t)->m6, 2, 4, 0);
	GetTileEx(t)->m7 = 0;
}
/** @copydoc MakeRailDepot(TileIndexT<Tgeneric>::T,Owner,DepotID,DiagDirection,RailType) */
static inline void MakeRailDepot(TileIndex t, Owner o, DepotID did, DiagDirection d, RailType r) { MakeRailDepot<false>(t, o, did, d, r); }
/** @copydoc MakeRailDepot(TileIndexT<Tgeneric>::T,Owner,DepotID,DiagDirection,RailType) */
static inline void MakeRailDepot(GenericTileIndex t, Owner o, DepotID did, DiagDirection d, RailType r) { MakeRailDepot<true>(t, o, did, d, r); }

#endif /* RAIL_MAP_H */
