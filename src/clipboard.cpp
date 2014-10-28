/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file clipboard.cpp Implementaion of clipboard related to both copying and pasting. */

#include "stdafx.h"
#include "core/alloc_func.hpp"
#include "core/mem_func.hpp"
#include "clipboard_func.h"
#include "tilearea_type.h"
#include "station_map.h"
#include "void_map.h"
#include "newgrf_airport.h"

static Map _clipboard_buffers[NUM_CLIPBOARD_BUFFERS];
static ClipboardStationList _clipboard_stations[NUM_CLIPBOARD_BUFFERS];

/**
 * Get the list of stations associated to a given clipboard buffer.
 * @param buffer the buffer
 * @return the list
 *
 * @pre IsClipboardBuffer(buffer)
 */
static ClipboardStationList GetClipboardStationList(Map *buffer)
{
    uint index = GetClipboardBufferIndex(buffer);
    assert(index < lengthof(_clipboard_stations));
    return _clipboard_stations[index];
}

/**
 * Associate a list of stations to a given clipboard buffer.
 * @param list the list
 * @param buffer the buffer
 *
 * @pre IsClipboardBuffer(buffer)
 */
static void SetClipboardStationList(ClipboardStationList list, Map *buffer)
{
    uint index = GetClipboardBufferIndex(buffer);
    assert(index < lengthof(_clipboard_stations));
    FreeClipboardStationList(&_clipboard_stations[index]);
    _clipboard_stations[index] = list;
}

/**
 * Free a list of clipboard stations.
 * @param list the list
 */
void FreeClipboardStationList(ClipboardStationList *list)
{
	for (ClipboardStation *item = *list, *next; item != NULL; item = next) {
		next = item->next;
		delete item;
	}
	*list = NULL;
}

/**
 * Test whether a given #Map is a clipboard buffer.
 * @return if the map a clipboard buffer
 */
bool IsClipboardBuffer(const Map *map)
{
	return (size_t)(map - _clipboard_buffers) < NUM_CLIPBOARD_BUFFERS;
}

/**
 * Get a clipboard buffer by it's index.
 * @param index the index
 * @return the buffer
 *
 * @pre index < NUM_CLIPBOARD_BUFFERS
 */
Map *GetClipboardBuffer(uint index)
{
	assert(index < NUM_CLIPBOARD_BUFFERS);
	return &_clipboard_buffers[index];
}

/**
 * Get the index of a clipboard buffer.
 * @param buffer the buffer
 * @return the index
 *
 * @pre IsClipboardBuffer(buffer)
 */
uint GetClipboardBufferIndex(const Map *buffer)
{
	assert(IsClipboardBuffer(buffer));
	return buffer - _clipboard_buffers;
}

/**
 * Test if a clipboard buffer is empty.
 * @param buffer the buffer
 * @return true iff there is no content in the buffer
 *
 * @pre IsClipboardBuffer(buffer)
 */
bool IsClipboardBufferEmpty(const Map *buffer)
{
	assert(IsClipboardBuffer(buffer));
	return buffer->m == NULL;
};

/**
 * Clear content of a clipboard buffer.
 * @param buffer the buffer
 *
 * @pre IsClipboardBuffer(buffer)
 */
void EmptyClipboardBuffer(Map *buffer)
{
	if (IsClipboardBufferEmpty(buffer)) return;

	SetClipboardStationList(NULL, buffer);

	buffer->size_x = 0;
	buffer->size_y = 0;
	buffer->size = 0;

	free(buffer->m);
	buffer->m = NULL;
	free(buffer->me);
	buffer->me = NULL;
}

/**
 * Allocate space in a clipboard buffer.
 * @param buffer the buffer
 * @param content_size_x X size of the content (excluding MP_VOID tiles on southern borders)
 * @param content_size_y Y size of the content (excluding MP_VOID tiles on southern borders)
 *
 * @pre IsClipboardBuffer(buffer)
 */
void AllocateClipboardBuffer(Map *buffer, uint content_size_x, uint content_size_y)
{
	assert(IsClipboardBuffer(buffer));
	assert(IsInsideMM(content_size_x, 1, INT_MAX - 1));
	assert(IsInsideMM(content_size_y, 1, INT_MAX - 1));

	SetClipboardStationList(NULL, buffer);

	buffer->size_x = content_size_x + 1;
	buffer->size_y = content_size_y + 1;
	buffer->size = buffer->size_x * buffer->size_y;

	free(buffer->m);
	free(buffer->me);
	buffer->m = CallocT<Tile>(buffer->size);
	buffer->me = CallocT<TileExtended>(buffer->size);

	GENERIC_TILE_AREA_LOOP(iter, GenericTileArea(TileXY(buffer->size_x - 1, 0, buffer), 1, buffer->size_y)) {
		MakeVoid(iter);
	}
	GENERIC_TILE_AREA_LOOP(iter, GenericTileArea(TileXY(0, buffer->size_y - 1, buffer), buffer->size_x - 1, 1)) {
		MakeVoid(iter);
	}
}

/**
 * Get #ClipboardStation by a given ID.
 * @param id the ID of the station
 * @param buffer clipboard buffer to get the station from
 *
 * @pre IsClipboardBuffer(buffer)
 */
/* static */ ClipboardStation *ClipboardStation::Get(StationID id, Map *buffer)
{
	for (ClipboardStation *ret = GetClipboardStationList(buffer); ret != NULL; ret = ret->next) {
		if (ret->id == id) return ret;
	}
	return NULL;
}

/**
 * Get #ClipboardStation by a given tile.
 * @param tile any tile that belongs to the station
 * @return station pointer or NULL if the tile is not a station
 *
 * @pre IsClipboardBuffer(MapOf(tile))
 */
/* static */ ClipboardStation *ClipboardStation::GetByTile(GenericTileIndex tile)
{
	return ClipboardStation::Get(GetStationIndex(tile), MapOf(tile));
}

/**
 * Get station specification of a station tile.
 * @param the tile
 * @return pointer to specification or NULL if the tile is not a station
 *
 * @pre IsClipboardBuffer(MapOf(tile))
 */
/* static */ const ClipboardStation::Spec *ClipboardStation::GetSpecByTile(GenericTileIndex tile)
{
	ClipboardStation *st = ClipboardStation::GetByTile(tile);
	if (st == NULL || !IsCustomStationSpecIndex(tile)) return NULL;
	byte custom_specindex = GetCustomStationSpecIndex(tile);
	assert(IsInsideBS(custom_specindex, 1, st->num_specs));
	return &st->speclist[custom_specindex - 1];
}

ClipboardStation::ClipboardStation()
{
	this->id             = INVALID_STATION;
	this->airport.tile   = INVALID_TILE_INDEX;
	this->airport.w      = 0;
	this->airport.h      = 0;
	this->airport.type   = AT_INVALID;
	this->airport.layout = 0;
	this->num_specs      = 0;
	this->speclist       = NULL;
	this->next           = NULL;
}

ClipboardStation::~ClipboardStation()
{
	free(this->speclist);
}

ClipboardStation **ClipboardStationsBuilder::FindStation(StationID sid)
{
	ClipboardStation **ret = &this->stations;
	while (*ret != NULL) {
		if ((*ret)->id == sid) break;
		ret = &((*ret)->next);
	}
	return ret;
}

ClipboardStation *ClipboardStationsBuilder::AddStation(StationID sid)
{
	ClipboardStation **st_link = this->FindStation(sid);
	ClipboardStation *st = *st_link;
	if (st == NULL) {
		st = new ClipboardStation;
		st->id = sid;
		*st_link = st; // put new item on the back of the list
	}
	return st;
}

uint ClipboardStationsBuilder::AddSpecToStation(ClipboardStation *st, StationClassID station_class, byte station_type)
{
	if (station_class == STAT_CLASS_DFLT || (station_class == STAT_CLASS_WAYP && station_type == 0)) return 0;

	for (int i = 0; i < st->num_specs; i++) {
		if (st->speclist[i].spec_class == station_class && st->speclist[i].spec_index == station_type) return i + 1;
	}

	st->speclist = ReallocT(st->speclist, st->num_specs + 1);
	st->speclist[st->num_specs].spec_class = station_class;
	st->speclist[st->num_specs].spec_index = station_type;
	st->num_specs++;

	return st->num_specs;
}

/**
 * Add a rail station part to the set of stations.
 * @param sid id of the station
 * @param station_class custom station class
 * @param station_type type within the custom station class
 * @return index of the given station spec in the list of specs of this station (aka custom station spec index)
 */
uint ClipboardStationsBuilder::AddRailStationPart(StationID sid, StationClassID station_class, byte station_type)
{
	return this->AddSpecToStation(this->AddStation(sid), station_class, station_type);
}

/**
 * Add a rail waypoint part to the set of stations.
 * @param sid id of the station
 * @param station_class custom station class
 * @param station_type type within the custom station class
 * @return index of the given station spec in the list of specs of this station (aka custom station spec index)
 */
uint ClipboardStationsBuilder::AddWaypointPart(StationID sid, StationClassID station_class, byte station_type)
{
	return this->AddSpecToStation(this->AddStation(sid), station_class, station_type);
}

/**
 * Add a road stop part to the set of stations.
 * @param sid id of the station
 */
void ClipboardStationsBuilder::AddRoadStopPart(StationID sid)
{
	this->AddStation(sid);
}

/**
 * Add a dock part to the set of stations.
 * @param sid id of the station
 */
void ClipboardStationsBuilder::AddDockPart(StationID sid)
{
	this->AddStation(sid);
}

/**
 * Add a buoy part to the set of stations.
 * @param sid id of the station
 */
void ClipboardStationsBuilder::AddBuoyPart(StationID sid)
{
	this->AddStation(sid);
}

/**
 * Add an airport to the set of stations.
 *
 * @param tile northern tile of the airport
 * @param sid id of the station
 * @param type airport type
 * @param layout airport layout
 */
void ClipboardStationsBuilder::AddAirportPart(RawTileIndex tile, StationID sid, AirportTypes type, byte layout)
{
	ClipboardStation *st = this->AddStation(sid);

	assert(st->airport.type == AT_INVALID);
	const AirportSpec *spec = AirportSpec::Get(type);
	st->airport.tile = tile;
	if (spec->rotation[layout] != DIR_E && spec->rotation[layout] != DIR_W) {
		st->airport.w = spec->size_x;
		st->airport.h = spec->size_y;
	} else {
		st->airport.w = spec->size_y;
		st->airport.h = spec->size_x;
	}
	st->airport.type = type;
	st->airport.layout = layout;
}

/**
 * Finish building and store results.
 * @param buffer clipboard buffer to store the list in
 *
 * @pre IsClipboardBuffer(MapOf(tile))
 */
void ClipboardStationsBuilder::BuildDone(Map *buffer)
{
	SetClipboardStationList(this->stations, buffer);
	this->stations = NULL;
}
