/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file clipboard_func.h Functions related to the clipboad. */

#ifndef CLIPBOARD_FUNC_H
#define CLIPBOARD_FUNC_H

#include "clipboard_type.h"

void FreeClipboardStationList(ClipboardStationList *list);

/** Helper class to build a station list while copying to the clipboard. */
class ClipboardStationsBuilder {
protected:
	ClipboardStationList stations; ///< the list

	ClipboardStation **FindStation(StationID sid);
	ClipboardStation *AddStation(StationID sid);
	uint AddSpecToStation(ClipboardStation *st, StationClassID station_class, byte station_type);

public:
	ClipboardStationsBuilder() : stations(NULL) {}
	~ClipboardStationsBuilder() { FreeClipboardStationList(&this->stations); }

	uint AddRailStationPart(StationID sid, StationClassID station_class, byte station_type);
	uint AddWaypointPart(StationID sid, StationClassID station_class, byte station_type);
	void AddRoadStopPart(StationID sid);
	void AddDockPart(StationID sid);
	void AddBuoyPart(StationID sid);
	void AddAirportPart(RawTileIndex tile, StationID sid, AirportTypes type, byte layout);

	void BuildDone(Map *clipboard);
};

static const uint NUM_CLIPBOARD_BUFFERS = 5; ///< Total amount of clipboard buffers

bool IsClipboardBuffer(const Map *map);
Map *GetClipboardBuffer(uint index);
uint GetClipboardBufferIndex(const Map *clipboard);
void AllocateClipboardBuffer(Map *clipboard, uint size_x, uint size_y);
bool IsClipboardBufferEmpty(const Map *clipboard);
void EmptyClipboardBuffer(Map *clipboard);
void ClearClipboard();

#endif /* CLIPBOARD_FUNC_H */
