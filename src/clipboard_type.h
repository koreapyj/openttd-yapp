/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file clipboard_type.h Types related to the clipboard. */

#ifndef CLIPBOARD_TYPE_H
#define CLIPBOARD_TYPE_H

#include "airport.h"
#include "newgrf_station.h"
#include "station_type.h"
#include "tilearea_type.h"

struct ClipboardStation {
	struct Spec {
		StationClassIDByte spec_class;
		byte spec_index;
	};

	struct AirportPart : RawTileArea {
		SimpleTinyEnumT<AirportTypes, byte> type; ///< Airport type
		byte layout;                              ///< Airport layout
	};

	StationID         id;        ///< ID
	AirportPart       airport;   ///< Airport details
	uint8             num_specs; ///< Number of specs in the speclist
	Spec             *speclist;  ///< List of station specs of this station
	ClipboardStation *next;      ///< "Next" pointer to make a linked list

	static ClipboardStation *Get(StationID id, Map *clipboard);
	static ClipboardStation *GetByTile(GenericTileIndex tile);
	static const Spec *GetSpecByTile(GenericTileIndex tile);

	ClipboardStation();
	~ClipboardStation();
};

typedef ClipboardStation *ClipboardStationList;

#endif /* CLIPBOARD_TYPE_H */
