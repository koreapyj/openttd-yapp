/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file clipboard_gui.h GUIs related to the clipboard. */

#ifndef CLIPBOARD_GUI_H
#define CLIPBOARD_GUI_H

#include "tile_type.h"
#include "road_type.h"
#include "track_type.h"

struct TileContentPastePreview {
	bool highlight_tile_rect; ///< Whether to highlight tile borders
	TrackBitsByte highlight_track_bits; ///< Rail tracks to highlight
};

struct TilePastePreview : TileContentPastePreview {
	int tile_height; ///< Destination height of the tile
};

void GetTilePastePreview(TileIndex tile, TilePastePreview *ret);

#endif /* CLIPBOARD_GUI_H */
