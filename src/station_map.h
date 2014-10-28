/* $Id: station_map.h 25240 2013-05-13 19:18:10Z rubidium $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file station_map.h Maps accessors for stations. */

#ifndef STATION_MAP_H
#define STATION_MAP_H

#include "rail_map.h"
#include "road_map.h"
#include "water_map.h"
#include "station_func.h"
#include "rail.h"

typedef byte StationGfx; ///< Index of station graphics. @see _station_display_datas

/**
 * Get StationID from a tile
 * @param t Tile to query station ID from
 * @pre IsTileType(t, MP_STATION)
 * @return Station ID of the station at \a t
 */
template <bool Tgeneric>
static inline StationID GetStationIndex(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_STATION));
	return (StationID)GetTile(t)->m2;
}
/** @copydoc GetStationIndex(TileIndexT<Tgeneric>::T) */
static inline StationID GetStationIndex(TileIndex t) { return GetStationIndex<false>(t); }
/** @copydoc GetStationIndex(TileIndexT<Tgeneric>::T) */
static inline StationID GetStationIndex(GenericTileIndex t) { return GetStationIndex<true>(t); }


static const int GFX_DOCK_BASE_WATER_PART          =  4; ///< The offset for the water parts.
static const int GFX_TRUCK_BUS_DRIVETHROUGH_OFFSET =  4; ///< The offset for the drive through parts.

/**
 * Get the station type of this tile
 * @param t the tile to query
 * @pre IsTileType(t, MP_STATION)
 * @return the station type
 */
template <bool Tgeneric>
static inline StationType GetStationType(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_STATION));
	return (StationType)GB(GetTileEx(t)->m6, 3, 3);
}
/** @copydoc GetStationType(TileIndexT<Tgeneric>::T) */
static inline StationType GetStationType(TileIndex t) { return GetStationType<false>(t); }
/** @copydoc GetStationType(TileIndexT<Tgeneric>::T) */
static inline StationType GetStationType(GenericTileIndex t) { return GetStationType<true>(t); }

/**
 * Get the road stop type of this tile
 * @param t the tile to query
 * @pre GetStationType(t) == STATION_TRUCK || GetStationType(t) == STATION_BUS
 * @return the road stop type
 */
template <bool Tgeneric>
static inline RoadStopType GetRoadStopType(typename TileIndexT<Tgeneric>::T t)
{
	assert(GetStationType(t) == STATION_TRUCK || GetStationType(t) == STATION_BUS);
	return GetStationType(t) == STATION_TRUCK ? ROADSTOP_TRUCK : ROADSTOP_BUS;
}
/** @copydoc GetRoadStopType(TileIndexT<Tgeneric>::T) */
static inline RoadStopType GetRoadStopType(TileIndex t) { return GetRoadStopType<false>(t); }
/** @copydoc GetRoadStopType(TileIndexT<Tgeneric>::T) */
static inline RoadStopType GetRoadStopType(GenericTileIndex t) { return GetRoadStopType<true>(t); }

/**
 * Get the station graphics of this tile
 * @param t the tile to query
 * @pre IsTileType(t, MP_STATION)
 * @return the station graphics
 */
template <bool Tgeneric>
static inline StationGfx GetStationGfx(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_STATION));
	return GetTile(t)->m5;
}
/** @copydoc GetStationGfx(TileIndexT<Tgeneric>::T) */
static inline StationGfx GetStationGfx(TileIndex t) { return GetStationGfx<false>(t); }
/** @copydoc GetStationGfx(TileIndexT<Tgeneric>::T) */
static inline StationGfx GetStationGfx(GenericTileIndex t) { return GetStationGfx<true>(t); }

/**
 * Set the station graphics of this tile
 * @param t the tile to update
 * @param gfx the new graphics
 * @pre IsTileType(t, MP_STATION)
 */
template <bool Tgeneric>
static inline void SetStationGfx(typename TileIndexT<Tgeneric>::T t, StationGfx gfx)
{
	assert(IsTileType(t, MP_STATION));
	GetTile(t)->m5 = gfx;
}
/** @copydoc SetStationGfx(TileIndexT<Tgeneric>::T,StationGfx) */
static inline void SetStationGfx(TileIndex t, StationGfx gfx) { SetStationGfx<false>(t, gfx); }
/** @copydoc SetStationGfx(TileIndexT<Tgeneric>::T,StationGfx) */
static inline void SetStationGfx(GenericTileIndex t, StationGfx gfx) { SetStationGfx<true>(t, gfx); }

/**
 * Is this station tile a rail station?
 * @param t the tile to get the information from
 * @pre IsTileType(t, MP_STATION)
 * @return true if and only if the tile is a rail station
 */
template <bool Tgeneric>
static inline bool IsRailStation(typename TileIndexT<Tgeneric>::T t)
{
	return GetStationType(t) == STATION_RAIL;
}
/** @copydoc IsRailStation(TileIndexT<Tgeneric>::T) */
static inline bool IsRailStation(TileIndex t) { return IsRailStation<false>(t); }
/** @copydoc IsRailStation(TileIndexT<Tgeneric>::T) */
static inline bool IsRailStation(GenericTileIndex t) { return IsRailStation<true>(t); }

/**
 * Is this tile a station tile and a rail station?
 * @param t the tile to get the information from
 * @return true if and only if the tile is a rail station
 */
template <bool Tgeneric>
static inline bool IsRailStationTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_STATION) && IsRailStation(t);
}
/** @copydoc IsRailStationTile(TileIndexT<Tgeneric>::T) */
static inline bool IsRailStationTile(TileIndex t) { return IsRailStationTile<false>(t); }
/** @copydoc IsRailStationTile(TileIndexT<Tgeneric>::T) */
static inline bool IsRailStationTile(GenericTileIndex t) { return IsRailStationTile<true>(t); }

/**
 * Is this station tile a rail waypoint?
 * @param t the tile to get the information from
 * @pre IsTileType(t, MP_STATION)
 * @return true if and only if the tile is a rail waypoint
 */
template <bool Tgeneric>
static inline bool IsRailWaypoint(typename TileIndexT<Tgeneric>::T t)
{
	return GetStationType(t) == STATION_WAYPOINT;
}
/** @copydoc IsRailWaypoint(TileIndexT<Tgeneric>::T) */
static inline bool IsRailWaypoint(TileIndex t) { return IsRailWaypoint<false>(t); }
/** @copydoc IsRailWaypoint(TileIndexT<Tgeneric>::T) */
static inline bool IsRailWaypoint(GenericTileIndex t) { return IsRailWaypoint<true>(t); }

/**
 * Is this tile a station tile and a rail waypoint?
 * @param t the tile to get the information from
 * @return true if and only if the tile is a rail waypoint
 */
template <bool Tgeneric>
static inline bool IsRailWaypointTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_STATION) && IsRailWaypoint(t);
}
/** @copydoc IsRailWaypointTile(TileIndexT<Tgeneric>::T) */
static inline bool IsRailWaypointTile(TileIndex t) { return IsRailWaypointTile<false>(t); }
/** @copydoc IsRailWaypointTile(TileIndexT<Tgeneric>::T) */
static inline bool IsRailWaypointTile(GenericTileIndex t) { return IsRailWaypointTile<true>(t); }

/**
 * Has this station tile a rail? In other words, is this station
 * tile a rail station or rail waypoint?
 * @param t the tile to check
 * @pre IsTileType(t, MP_STATION)
 * @return true if and only if the tile has rail
 */
template <bool Tgeneric>
static inline bool HasStationRail(typename TileIndexT<Tgeneric>::T t)
{
	return IsRailStation(t) || IsRailWaypoint(t);
}
/** @copydoc HasStationRail(TileIndexT<Tgeneric>::T) */
static inline bool HasStationRail(TileIndex t) { return HasStationRail<false>(t); }
/** @copydoc HasStationRail(TileIndexT<Tgeneric>::T) */
static inline bool HasStationRail(GenericTileIndex t) { return HasStationRail<true>(t); }

/**
 * Has this station tile a rail? In other words, is this station
 * tile a rail station or rail waypoint?
 * @param t the tile to check
 * @return true if and only if the tile is a station tile and has rail
 */
template <bool Tgeneric>
static inline bool HasStationTileRail(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_STATION) && HasStationRail(t);
}
/** @copydoc HasStationTileRail(TileIndexT<Tgeneric>::T) */
static inline bool HasStationTileRail(TileIndex t) { return HasStationTileRail<false>(t); }
/** @copydoc HasStationTileRail(TileIndexT<Tgeneric>::T) */
static inline bool HasStationTileRail(GenericTileIndex t) { return HasStationTileRail<true>(t); }

/**
 * Is this station tile an airport?
 * @param t the tile to get the information from
 * @pre IsTileType(t, MP_STATION)
 * @return true if and only if the tile is an airport
 */
template <bool Tgeneric>
static inline bool IsAirport(typename TileIndexT<Tgeneric>::T t)
{
	return GetStationType(t) == STATION_AIRPORT;
}
/** @copydoc IsAirport(TileIndexT<Tgeneric>::T) */
static inline bool IsAirport(TileIndex t) { return IsAirport<false>(t); }
/** @copydoc IsAirport(TileIndexT<Tgeneric>::T) */
static inline bool IsAirport(GenericTileIndex t) { return IsAirport<true>(t); }

/**
 * Is this tile a station tile and an airport tile?
 * @param t the tile to get the information from
 * @return true if and only if the tile is an airport
 */
template <bool Tgeneric>
static inline bool IsAirportTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_STATION) && IsAirport(t);
}
/** @copydoc IsAirportTile(TileIndexT<Tgeneric>::T) */
static inline bool IsAirportTile(TileIndex t) { return IsAirportTile<false>(t); }
/** @copydoc IsAirportTile(TileIndexT<Tgeneric>::T) */
static inline bool IsAirportTile(GenericTileIndex t) { return IsAirportTile<true>(t); }

bool IsHangar(TileIndex t);

/**
 * Is the station at \a t a truck stop?
 * @param t Tile to check
 * @pre IsTileType(t, MP_STATION)
 * @return \c true if station is a truck stop, \c false otherwise
 */
template <bool Tgeneric>
static inline bool IsTruckStop(typename TileIndexT<Tgeneric>::T t)
{
	return GetStationType(t) == STATION_TRUCK;
}
/** @copydoc IsTruckStop(TileIndexT<Tgeneric>::T) */
static inline bool IsTruckStop(TileIndex t) { return IsTruckStop<false>(t); }
/** @copydoc IsTruckStop(TileIndexT<Tgeneric>::T) */
static inline bool IsTruckStop(GenericTileIndex t) { return IsTruckStop<true>(t); }

/**
 * Is the station at \a t a bus stop?
 * @param t Tile to check
 * @pre IsTileType(t, MP_STATION)
 * @return \c true if station is a bus stop, \c false otherwise
 */
template <bool Tgeneric>
static inline bool IsBusStop(typename TileIndexT<Tgeneric>::T t)
{
	return GetStationType(t) == STATION_BUS;
}
/** @copydoc IsBusStop(TileIndexT<Tgeneric>::T) */
static inline bool IsBusStop(TileIndex t) { return IsBusStop<false>(t); }
/** @copydoc IsBusStop(TileIndexT<Tgeneric>::T) */
static inline bool IsBusStop(GenericTileIndex t) { return IsBusStop<true>(t); }

/**
 * Is the station at \a t a road station?
 * @param t Tile to check
 * @pre IsTileType(t, MP_STATION)
 * @return \c true if station at the tile is a bus top or a truck stop, \c false otherwise
 */
template <bool Tgeneric>
static inline bool IsRoadStop(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsTileType(t, MP_STATION));
	return IsTruckStop(t) || IsBusStop(t);
}
/** @copydoc IsRoadStop(TileIndexT<Tgeneric>::T) */
static inline bool IsRoadStop(TileIndex t) { return IsRoadStop<false>(t); }
/** @copydoc IsRoadStop(TileIndexT<Tgeneric>::T) */
static inline bool IsRoadStop(GenericTileIndex t) { return IsRoadStop<true>(t); }

/**
 * Is tile \a t a road stop station?
 * @param t Tile to check
 * @return \c true if the tile is a station tile and a road stop
 */
template <bool Tgeneric>
static inline bool IsRoadStopTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_STATION) && IsRoadStop(t);
}
/** @copydoc IsRoadStopTile(TileIndexT<Tgeneric>::T) */
static inline bool IsRoadStopTile(TileIndex t) { return IsRoadStopTile<false>(t); }
/** @copydoc IsRoadStopTile(TileIndexT<Tgeneric>::T) */
static inline bool IsRoadStopTile(GenericTileIndex t) { return IsRoadStopTile<true>(t); }

/**
 * Is tile \a t a standard (non-drive through) road stop station?
 * @param t Tile to check
 * @return \c true if the tile is a station tile and a standard road stop
 */
template <bool Tgeneric>
static inline bool IsStandardRoadStopTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsRoadStopTile(t) && GetStationGfx(t) < GFX_TRUCK_BUS_DRIVETHROUGH_OFFSET;
}
/** @copydoc IsStandardRoadStopTile(TileIndexT<Tgeneric>::T) */
static inline bool IsStandardRoadStopTile(TileIndex t) { return IsStandardRoadStopTile<false>(t); }
/** @copydoc IsStandardRoadStopTile(TileIndexT<Tgeneric>::T) */
static inline bool IsStandardRoadStopTile(GenericTileIndex t) { return IsStandardRoadStopTile<true>(t); }

/**
 * Is tile \a t a drive through road stop station?
 * @param t Tile to check
 * @return \c true if the tile is a station tile and a drive through road stop
 */
template <bool Tgeneric>
static inline bool IsDriveThroughStopTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsRoadStopTile(t) && GetStationGfx(t) >= GFX_TRUCK_BUS_DRIVETHROUGH_OFFSET;
}
/** @copydoc IsDriveThroughStopTile(TileIndexT<Tgeneric>::T) */
static inline bool IsDriveThroughStopTile(TileIndex t) { return IsDriveThroughStopTile<false>(t); }
/** @copydoc IsDriveThroughStopTile(TileIndexT<Tgeneric>::T) */
static inline bool IsDriveThroughStopTile(GenericTileIndex t) { return IsDriveThroughStopTile<true>(t); }

/**
 * Get the station graphics of this airport tile
 * @param t the tile to query
 * @pre IsAirport(t)
 * @return the station graphics
 */
static inline StationGfx GetAirportGfx(TileIndex t)
{
	assert(IsAirport(t));
	extern StationGfx GetTranslatedAirportTileID(StationGfx gfx);
	return GetTranslatedAirportTileID(GetStationGfx(t));
}

/**
 * Gets the direction the road stop entrance points towards.
 * @param t the tile of the road stop
 * @pre IsRoadStopTile(t)
 * @return the direction of the entrance
 */
template <bool Tgeneric>
static inline DiagDirection GetRoadStopDir(typename TileIndexT<Tgeneric>::T t)
{
	StationGfx gfx = GetStationGfx(t);
	assert(IsRoadStopTile(t));
	if (gfx < GFX_TRUCK_BUS_DRIVETHROUGH_OFFSET) {
		return (DiagDirection)(gfx);
	} else {
		return (DiagDirection)(gfx - GFX_TRUCK_BUS_DRIVETHROUGH_OFFSET);
	}
}
/** @copydoc GetRoadStopDir(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetRoadStopDir(TileIndex t) { return GetRoadStopDir<false>(t); }
/** @copydoc GetRoadStopDir(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetRoadStopDir(GenericTileIndex t) { return GetRoadStopDir<true>(t); }

/**
 * Is tile \a t part of an oilrig?
 * @param t Tile to check
 * @pre IsTileType(t, MP_STATION)
 * @return \c true if the tile is an oilrig tile
 */
static inline bool IsOilRig(TileIndex t)
{
	return GetStationType(t) == STATION_OILRIG;
}

/**
 * Is tile \a t a dock tile?
 * @param t Tile to check
 * @pre IsTileType(t, MP_STATION)
 * @return \c true if the tile is a dock
 */
template <bool Tgeneric>
static inline bool IsDock(typename TileIndexT<Tgeneric>::T t)
{
	return GetStationType(t) == STATION_DOCK;
}
/** @copydoc IsDock(TileIndexT<Tgeneric>::T) */
static inline bool IsDock(TileIndex t) { return IsDock<false>(t); }
/** @copydoc IsDock(TileIndexT<Tgeneric>::T) */
static inline bool IsDock(GenericTileIndex t) { return IsDock<true>(t); }

/**
 * Is tile \a t a dock tile?
 * @param t Tile to check
 * @return \c true if the tile is a dock
 */
template <bool Tgeneric>
static inline bool IsDockTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_STATION) && GetStationType(t) == STATION_DOCK;
}
/** @copydoc IsDockTile(TileIndexT<Tgeneric>::T) */
static inline bool IsDockTile(TileIndex t) { return IsDockTile<false>(t); }
/** @copydoc IsDockTile(TileIndexT<Tgeneric>::T) */
static inline bool IsDockTile(GenericTileIndex t) { return IsDockTile<true>(t); }

/**
 * Is tile \a t a buoy tile?
 * @param t Tile to check
 * @pre IsTileType(t, MP_STATION)
 * @return \c true if the tile is a buoy
 */
template <bool Tgeneric>
static inline bool IsBuoy(typename TileIndexT<Tgeneric>::T t)
{
	return GetStationType(t) == STATION_BUOY;
}
/** @copydoc IsBuoy(TileIndexT<Tgeneric>::T) */
static inline bool IsBuoy(TileIndex t) { return IsBuoy<false>(t); }
/** @copydoc IsBuoy(TileIndexT<Tgeneric>::T) */
static inline bool IsBuoy(GenericTileIndex t) { return IsBuoy<true>(t); }

/**
 * Is tile \a t a buoy tile?
 * @param t Tile to check
 * @return \c true if the tile is a buoy
 */
template <bool Tgeneric>
static inline bool IsBuoyTile(typename TileIndexT<Tgeneric>::T t)
{
	return IsTileType(t, MP_STATION) && IsBuoy(t);
}
/** @copydoc IsBuoyTile(TileIndexT<Tgeneric>::T) */
static inline bool IsBuoyTile(TileIndex t) { return IsBuoyTile<false>(t); }
/** @copydoc IsBuoyTile(TileIndexT<Tgeneric>::T) */
static inline bool IsBuoyTile(GenericTileIndex t) { return IsBuoyTile<true>(t); }

/**
 * Is tile \a t an hangar tile?
 * @param t Tile to check
 * @return \c true if the tile is an hangar
 */
static inline bool IsHangarTile(TileIndex t)
{
	return IsTileType(t, MP_STATION) && IsHangar(t);
}

/**
 * Get the rail direction of a rail station.
 * @param t Tile to query
 * @pre HasStationRail(t)
 * @return The direction of the rails on tile \a t.
 */
template <bool Tgeneric>
static inline Axis GetRailStationAxis(typename TileIndexT<Tgeneric>::T t)
{
	assert(HasStationRail(t));
	return HasBit(GetStationGfx(t), 0) ? AXIS_Y : AXIS_X;
}
/** @copydoc GetRailStationAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetRailStationAxis(TileIndex t) { return GetRailStationAxis<false>(t); }
/** @copydoc GetRailStationAxis(TileIndexT<Tgeneric>::T) */
static inline Axis GetRailStationAxis(GenericTileIndex t) { return GetRailStationAxis<true>(t); }

/**
 * Get the rail track of a rail station tile.
 * @param t Tile to query
 * @pre HasStationRail(t)
 * @return The rail track of the rails on tile \a t.
 */
template <bool Tgeneric>
static inline Track GetRailStationTrack(typename TileIndexT<Tgeneric>::T t)
{
	return AxisToTrack(GetRailStationAxis(t));
}
/** @copydoc GetRailStationTrack(TileIndexT<Tgeneric>::T) */
static inline Track GetRailStationTrack(TileIndex t) { return GetRailStationTrack<false>(t); }
/** @copydoc GetRailStationTrack(TileIndexT<Tgeneric>::T) */
static inline Track GetRailStationTrack(GenericTileIndex t) { return GetRailStationTrack<true>(t); }

/**
 * Get the trackbits of a rail station tile.
 * @param t Tile to query
 * @pre HasStationRail(t)
 * @return The trackbits of the rails on tile \a t.
 */
template <bool Tgeneric>
static inline TrackBits GetRailStationTrackBits(typename TileIndexT<Tgeneric>::T t)
{
	return AxisToTrackBits(GetRailStationAxis(t));
}
/** @copydoc GetRailStationTrackBits(TileIndexT<Tgeneric>::T) */
static inline TrackBits GetRailStationTrackBits(TileIndex t) { return GetRailStationTrackBits<false>(t); }
/** @copydoc GetRailStationTrackBits(TileIndexT<Tgeneric>::T) */
static inline TrackBits GetRailStationTrackBits(GenericTileIndex t) { return GetRailStationTrackBits<true>(t); }

/**
 * Check if a tile is a valid continuation to a railstation tile.
 * The tile \a test_tile is a valid continuation to \a station_tile, if all of the following are true:
 * \li \a test_tile is a rail station tile
 * \li the railtype of \a test_tile is compatible with the railtype of \a station_tile
 * \li the tracks on \a test_tile and \a station_tile are in the same direction
 * \li both tiles belong to the same station
 * \li \a test_tile is not blocked (@see IsStationTileBlocked)
 * @param test_tile Tile to test
 * @param station_tile Station tile to compare with
 * @pre IsRailStationTile(station_tile)
 * @return true if the two tiles are compatible
 */
static inline bool IsCompatibleTrainStationTile(TileIndex test_tile, TileIndex station_tile)
{
	assert(IsRailStationTile(station_tile));
	return IsRailStationTile(test_tile) && IsCompatibleRail(GetRailType(test_tile), GetRailType(station_tile)) &&
			GetRailStationAxis(test_tile) == GetRailStationAxis(station_tile) &&
			GetStationIndex(test_tile) == GetStationIndex(station_tile) &&
			!IsStationTileBlocked(test_tile);
}

/**
 * Get the reservation state of the rail station
 * @pre HasStationRail(t)
 * @param t the station tile
 * @return reservation state
 */
static inline bool HasStationReservation(TileIndex t)
{
	assert(HasStationRail(t));
	return HasBit(GetTileEx(t)->m6, 2);
}

/**
 * Set the reservation state of the rail station
 * @pre HasStationRail(t)
 * @param t the station tile
 * @param b the reservation state
 */
template <bool Tgeneric>
static inline void SetRailStationReservation(typename TileIndexT<Tgeneric>::T t, bool b)
{
	assert(HasStationRail(t));
	SB(GetTileEx(t)->m6, 2, 1, b ? 1 : 0);
}
/** @copydoc SetRailStationReservation(TileIndexT<Tgeneric>::T) */
static inline void SetRailStationReservation(TileIndex t, bool b) { SetRailStationReservation<false>(t, b); }
/** @copydoc SetRailStationReservation(TileIndexT<Tgeneric>::T) */
static inline void SetRailStationReservation(GenericTileIndex t, bool b) { SetRailStationReservation<true>(t, b); }

/**
 * Get the reserved track bits for a waypoint
 * @pre HasStationRail(t)
 * @param t the tile
 * @return reserved track bits
 */
static inline TrackBits GetStationReservationTrackBits(TileIndex t)
{
	return HasStationReservation(t) ? GetRailStationTrackBits(t) : TRACK_BIT_NONE;
}

/**
 * Test whether a given water dock tile is the land part of the dock
 * @param t the water dock tile
 * @return if the given tile is the land part of a dock
 * @pre IsDockTile(t)
 */
template <bool Tgeneric>
static inline bool IsLandDockSection(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsDockTile(t));
	return GetStationGfx(t) < GFX_DOCK_BASE_WATER_PART;
}
/** @copydoc IsLandDockSection<bool> */
static inline bool IsLandDockSection(TileIndex t) { return IsLandDockSection<false>(t); }
/** @copydoc IsLandDockSection<bool> */
static inline bool IsLandDockSection(GenericTileIndex t) { return IsLandDockSection<true>(t); }

/**
 * Get the direction of a dock.
 * @param t Tile to query
 * @pre IsLandDockSection(t)
 * @pre \a t is the land part of the dock
 * @return The direction of the dock on tile \a t.
 */
template <bool Tgeneric>
static inline DiagDirection GetDockDirection(typename TileIndexT<Tgeneric>::T t)
{
	assert(IsLandDockSection(t));
	return (DiagDirection)GetStationGfx(t);
}
/** @copydoc GetDockDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetDockDirection(TileIndex t) { return GetDockDirection<false>(t); }
/** @copydoc GetDockDirection(TileIndexT<Tgeneric>::T) */
static inline DiagDirection GetDockDirection(GenericTileIndex t) { return GetDockDirection<true>(t); }

/**
 * Get the other tile of a dock.
 * @param t Tile to query
 * @pre IsDockTile(t)
 * @return The other tile of the dock.
 */
template <bool Tgeneric>
static inline typename TileIndexT<Tgeneric>::T GetOtherDockTile(typename TileIndexT<Tgeneric>::T t)
{
	TileIndexDiff delta = ToTileIndexDiff(TileIndexDiffCByDiagDir(AxisToDiagDir((Axis)(GetStationGfx(t) & 0x1))), MapOf(t));
	return IsDockTile(t + delta) ? t + delta : t - delta;
}
/** @copydoc GetOtherDockTile<bool> */
static inline TileIndex GetOtherDockTile(TileIndex t) { return GetOtherDockTile<false>(t); }
/** @copydoc GetOtherDockTile<bool> */
static inline GenericTileIndex GetOtherDockTile(GenericTileIndex t) { return GetOtherDockTile<true>(t); }

/**
 * Get the tileoffset from this tile a ship should target to get to this dock.
 * @param t Tile to query
 * @pre IsTileType(t, MP_STATION)
 * @pre IsBuoy(t) || IsOilRig(t) || IsDock(t)
 * @return The offset from this tile that should be used as destination for ships.
 */
static inline TileIndexDiffC GetDockOffset(TileIndex t)
{
	static const TileIndexDiffC buoy_offset = {0, 0};
	static const TileIndexDiffC oilrig_offset = {2, 0};
	static const TileIndexDiffC dock_offset[DIAGDIR_END] = {
		{-2,  0},
		{ 0,  2},
		{ 2,  0},
		{ 0, -2},
	};
	assert(IsTileType(t, MP_STATION));

	if (IsBuoy(t)) return buoy_offset;
	if (IsOilRig(t)) return oilrig_offset;

	assert(IsDock(t));

	return dock_offset[GetDockDirection(t)];
}

/**
 * Is there a custom rail station spec on this tile?
 * @param t Tile to query
 * @pre HasStationTileRail(t)
 * @return True if this station is part of a newgrf station.
 */
template <bool Tgeneric>
static inline bool IsCustomStationSpecIndex(typename TileIndexT<Tgeneric>::T t)
{
	assert(HasStationTileRail(t));
	return GetTile(t)->m4 != 0;
}
/** @copydoc IsCustomStationSpecIndex(TileIndexT<Tgeneric>::T) */
static inline bool IsCustomStationSpecIndex(TileIndex t) { return IsCustomStationSpecIndex<false>(t); }
/** @copydoc IsCustomStationSpecIndex(TileIndexT<Tgeneric>::T) */
static inline bool IsCustomStationSpecIndex(GenericTileIndex t) { return IsCustomStationSpecIndex<true>(t); }

/**
 * Set the custom station spec for this tile.
 * @param t Tile to set the stationspec of.
 * @param specindex The new spec.
 * @pre HasStationTileRail(t)
 */
template <bool Tgeneric>
static inline void SetCustomStationSpecIndex(typename TileIndexT<Tgeneric>::T t, byte specindex)
{
	assert(HasStationTileRail(t));
	GetTile(t)->m4 = specindex;
}
/** @copydoc SetCustomStationSpecIndex(TileIndexT<Tgeneric>::T,byte) */
static inline void SetCustomStationSpecIndex(TileIndex t, byte specindex) { return SetCustomStationSpecIndex<false>(t, specindex); }
/** @copydoc SetCustomStationSpecIndex(TileIndexT<Tgeneric>::T,byte) */
static inline void SetCustomStationSpecIndex(GenericTileIndex t, byte specindex) { return SetCustomStationSpecIndex<true>(t, specindex); }

/**
 * Get the custom station spec for this tile.
 * @param t Tile to query
 * @pre HasStationTileRail(t)
 * @return The custom station spec of this tile.
 */
template <bool Tgeneric>
static inline uint GetCustomStationSpecIndex(typename TileIndexT<Tgeneric>::T t)
{
	assert(HasStationTileRail(t));
	return GetTile(t)->m4;
}
/** @copydoc GetCustomStationSpecIndex(TileIndexT<Tgeneric>::T) */
static inline uint GetCustomStationSpecIndex(TileIndex t) { return GetCustomStationSpecIndex<false>(t); }
/** @copydoc GetCustomStationSpecIndex(TileIndexT<Tgeneric>::T) */
static inline uint GetCustomStationSpecIndex(GenericTileIndex t) { return GetCustomStationSpecIndex<true>(t); }

/**
 * Set the random bits for a station tile.
 * @param t Tile to set random bits for.
 * @param random_bits The random bits.
 * @pre IsTileType(t, MP_STATION)
 */
static inline void SetStationTileRandomBits(TileIndex t, byte random_bits)
{
	assert(IsTileType(t, MP_STATION));
	SB(GetTile(t)->m3, 4, 4, random_bits);
}

/**
 * Get the random bits of a station tile.
 * @param t Tile to query
 * @pre IsTileType(t, MP_STATION)
 * @return The random bits for this station tile.
 */
static inline byte GetStationTileRandomBits(TileIndex t)
{
	assert(IsTileType(t, MP_STATION));
	return GB(GetTile(t)->m3, 4, 4);
}

/**
 * Make the given tile a station tile.
 * @param t the tile to make a station tile
 * @param o the owner of the station
 * @param sid the station to which this tile belongs
 * @param st the type this station tile
 * @param section the StationGfx to be used for this tile
 * @param wc The water class of the station
 */
template <bool Tgeneric>
static inline void MakeStation(typename TileIndexT<Tgeneric>::T t, Owner o, StationID sid, StationType st, byte section, WaterClass wc = WATER_CLASS_INVALID)
{
	SetTileType(t, MP_STATION);
	SetTileOwner(t, o);
	SetWaterClass(t, wc);
	GetTile(t)->m2 = sid;
	GetTile(t)->m3 = 0;
	GetTile(t)->m4 = 0;
	GetTile(t)->m5 = section;
	SB(GetTileEx(t)->m6, 2, 1, 0);
	SB(GetTileEx(t)->m6, 3, 3, st);
	GetTileEx(t)->m7 = 0;
}
/** @copydoc MakeStation(TileIndexT<Tgeneric>::T,Owner,StationID,StationType,byte,WaterClass) */
static inline void MakeStation(TileIndex t, Owner o, StationID sid, StationType st, byte section, WaterClass wc = WATER_CLASS_INVALID) { MakeStation<false>(t, o, sid, st, section, wc); }
/** @copydoc MakeStation(TileIndexT<Tgeneric>::T,Owner,StationID,StationType,byte,WaterClass) */
static inline void MakeStation(GenericTileIndex t, Owner o, StationID sid, StationType st, byte section, WaterClass wc = WATER_CLASS_INVALID) { MakeStation<true>(t, o, sid, st, section, wc); }

/**
 * Make the given tile a rail station tile.
 * @param t the tile to make a rail station tile
 * @param o the owner of the station
 * @param sid the station to which this tile belongs
 * @param a the axis of this tile
 * @param section the StationGfx to be used for this tile
 * @param rt the railtype of this tile
 */
template <bool Tgeneric>
static inline void MakeRailStation(typename TileIndexT<Tgeneric>::T t, Owner o, StationID sid, Axis a, byte section, RailType rt)
{
	MakeStation(t, o, sid, STATION_RAIL, section + a);
	SetRailType(t, rt);
	SetRailStationReservation(t, false);
}
/** @copydoc MakeRailStation(TileIndexT<Tgeneric>::T,Owner,StationID,Axis,byte,RailType) */
static inline void MakeRailStation(TileIndex t, Owner o, StationID sid, Axis a, byte section, RailType rt) { MakeRailStation<false>(t, o, sid, a, section, rt); }
/** @copydoc MakeRailStation(TileIndexT<Tgeneric>::T,Owner,StationID,Axis,byte,RailType) */
static inline void MakeRailStation(GenericTileIndex t, Owner o, StationID sid, Axis a, byte section, RailType rt) { MakeRailStation<true>(t, o, sid, a, section, rt); }

/**
 * Make the given tile a rail waypoint tile.
 * @param t the tile to make a rail waypoint
 * @param o the owner of the waypoint
 * @param sid the waypoint to which this tile belongs
 * @param a the axis of this tile
 * @param section the StationGfx to be used for this tile
 * @param rt the railtype of this tile
 */
template <bool Tgeneric>
static inline void MakeRailWaypoint(typename TileIndexT<Tgeneric>::T t, Owner o, StationID sid, Axis a, byte section, RailType rt)
{
	MakeStation(t, o, sid, STATION_WAYPOINT, section + a);
	SetRailType(t, rt);
	SetRailStationReservation(t, false);
}
/** @copydoc MakeRailWaypoint(TileIndexT<Tgeneric>::T,Owner,StationID,Axis,byte,RailType) */
static inline void MakeRailWaypoint(TileIndex t, Owner o, StationID sid, Axis a, byte section, RailType rt) { MakeRailWaypoint<false>(t, o, sid, a, section, rt); }
/** @copydoc MakeRailWaypoint(TileIndexT<Tgeneric>::T,Owner,StationID,Axis,byte,RailType) */
static inline void MakeRailWaypoint(GenericTileIndex t, Owner o, StationID sid, Axis a, byte section, RailType rt) { MakeRailWaypoint<true>(t, o, sid, a, section, rt); }

/**
 * Make the given tile a roadstop tile.
 * @param t the tile to make a roadstop
 * @param o the owner of the roadstop
 * @param sid the station to which this tile belongs
 * @param rst the type of roadstop to make this tile
 * @param rt the roadtypes on this tile
 * @param d the direction of the roadstop
 */
template <bool Tgeneric>
static inline void MakeRoadStop(typename TileIndexT<Tgeneric>::T t, Owner o, StationID sid, RoadStopType rst, RoadTypes rt, DiagDirection d)
{
	MakeStation(t, o, sid, (rst == ROADSTOP_BUS ? STATION_BUS : STATION_TRUCK), d);
	SetRoadTypes(t, rt);
	SetRoadOwner(t, ROADTYPE_ROAD, o);
	SetRoadOwner(t, ROADTYPE_TRAM, o);
}
/** @copydoc MakeRoadStop(TileIndexT<Tgeneric>::T,Owner,StationID,RoadStopType,RoadTypes,DiagDirection) */
static inline void MakeRoadStop(TileIndex t, Owner o, StationID sid, RoadStopType rst, RoadTypes rt, DiagDirection d) { MakeRoadStop<false>(t, o, sid, rst, rt, d); }
/** @copydoc MakeRoadStop(TileIndexT<Tgeneric>::T,Owner,StationID,RoadStopType,RoadTypes,DiagDirection) */
static inline void MakeRoadStop(GenericTileIndex t, Owner o, StationID sid, RoadStopType rst, RoadTypes rt, DiagDirection d) { MakeRoadStop<true>(t, o, sid, rst, rt, d); }

/**
 * Make the given tile a drivethrough roadstop tile.
 * @param t the tile to make a roadstop
 * @param station the owner of the roadstop
 * @param road the owner of the road
 * @param tram the owner of the tram
 * @param sid the station to which this tile belongs
 * @param rst the type of roadstop to make this tile
 * @param rt the roadtypes on this tile
 * @param a the direction of the roadstop
 */
template <bool Tgeneric>
static inline void MakeDriveThroughRoadStop(typename TileIndexT<Tgeneric>::T t, Owner station, Owner road, Owner tram, StationID sid, RoadStopType rst, RoadTypes rt, Axis a)
{
	MakeStation(t, station, sid, (rst == ROADSTOP_BUS ? STATION_BUS : STATION_TRUCK), GFX_TRUCK_BUS_DRIVETHROUGH_OFFSET + a);
	SetRoadTypes(t, rt);
	SetRoadOwner(t, ROADTYPE_ROAD, road);
	SetRoadOwner(t, ROADTYPE_TRAM, tram);
}
/** @copydoc MakeDriveThroughRoadStop(TileIndexT<Tgeneric>::T,Owner,Owner,Owner,StationID,RoadStopType,RoadTypes,Axis) */
static inline void MakeDriveThroughRoadStop(TileIndex t, Owner station, Owner road, Owner tram, StationID sid, RoadStopType rst, RoadTypes rt, Axis a) { MakeDriveThroughRoadStop<false>(t, station, road, tram, sid, rst, rt, a); }
/** @copydoc MakeDriveThroughRoadStop(TileIndexT<Tgeneric>::T,Owner,Owner,Owner,StationID,RoadStopType,RoadTypes,Axis) */
static inline void MakeDriveThroughRoadStop(GenericTileIndex t, Owner station, Owner road, Owner tram, StationID sid, RoadStopType rst, RoadTypes rt, Axis a) { MakeDriveThroughRoadStop<true>(t, station, road, tram, sid, rst, rt, a); }

/**
 * Make the given tile an airport tile.
 * @param t the tile to make a airport
 * @param o the owner of the airport
 * @param sid the station to which this tile belongs
 * @param section the StationGfx to be used for this tile
 * @param wc the type of water on this tile
 */
template <bool Tgeneric>
static inline void MakeAirport(typename TileIndexT<Tgeneric>::T t, Owner o, StationID sid, byte section, WaterClass wc)
{
	MakeStation(t, o, sid, STATION_AIRPORT, section, wc);
}
/** @copydoc MakeAirport(TileIndexT<Tgeneric>::T,Owner,StationID,byte,WaterClass) */
static inline void MakeAirport(TileIndex t, Owner o, StationID sid, byte section, WaterClass wc) { MakeAirport<false>(t, o, sid, section, wc); }
/** @copydoc MakeAirport(TileIndexT<Tgeneric>::T,Owner,StationID,byte,WaterClass) */
static inline void MakeAirport(GenericTileIndex t, Owner o, StationID sid, byte section, WaterClass wc) { MakeAirport<true>(t, o, sid, section, wc); }

/**
 * Make the given tile a buoy tile.
 * @param t the tile to make a buoy
 * @param sid the station to which this tile belongs
 * @param wc the type of water on this tile
 */
template <bool Tgeneric>
static inline void MakeBuoy(typename TileIndexT<Tgeneric>::T t, StationID sid, WaterClass wc)
{
	/* Make the owner of the buoy tile the same as the current owner of the
	 * water tile. In this way, we can reset the owner of the water to its
	 * original state when the buoy gets removed. */
	MakeStation(t, GetTileOwner(t), sid, STATION_BUOY, 0, wc);
}
/** @copydoc MakeBuoy(TileIndexT<Tgeneric>::T,StationID,WaterClass) */
static inline void MakeBuoy(TileIndex t, StationID sid, WaterClass wc) { MakeBuoy<false>(t, sid, wc); }
/** @copydoc MakeBuoy(TileIndexT<Tgeneric>::T,StationID,WaterClass) */
static inline void MakeBuoy(GenericTileIndex t, StationID sid, WaterClass wc) { MakeBuoy<true>(t, sid, wc); }

/**
 * Make the given tile a dock tile.
 * @param t the tile to make a dock
 * @param o the owner of the dock
 * @param sid the station to which this tile belongs
 * @param d the direction of the dock
 * @param wc the type of water on this tile
 */
template <bool Tgeneric>
static inline void MakeDock(typename TileIndexT<Tgeneric>::T t, Owner o, StationID sid, DiagDirection d, WaterClass wc)
{
	MakeStation(t, o, sid, STATION_DOCK, d);
	MakeStation(TileAddByDiagDir(t, d), o, sid, STATION_DOCK, GFX_DOCK_BASE_WATER_PART + DiagDirToAxis(d), wc);
}
/** @copydoc MakeDock(TileIndexT<Tgeneric>::T,Owner,StationID,DiagDirection,WaterClass) */
static inline void MakeDock(TileIndex t, Owner o, StationID sid, DiagDirection d, WaterClass wc) { MakeDock<false>(t, o, sid, d, wc); }
/** @copydoc MakeDock(TileIndexT<Tgeneric>::T,Owner,StationID,DiagDirection,WaterClass) */
static inline void MakeDock(GenericTileIndex t, Owner o, StationID sid, DiagDirection d, WaterClass wc) { MakeDock<true>(t, o, sid, d, wc); }

/**
 * Make the given tile an oilrig tile.
 * @param t the tile to make an oilrig
 * @param sid the station to which this tile belongs
 * @param wc the type of water on this tile
 */
static inline void MakeOilrig(TileIndex t, StationID sid, WaterClass wc)
{
	MakeStation(t, OWNER_NONE, sid, STATION_OILRIG, 0, wc);
}

#endif /* STATION_MAP_H */
