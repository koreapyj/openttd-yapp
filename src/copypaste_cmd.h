/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file copypaste_cmd.h Helper functions for copy/paste commands. */

#ifndef COPYPASTE_CMD_H
#define COPYPASTE_CMD_H

#include "core/enum_type.hpp"
#include "command_func.h"
#include "rail_type.h"
#include "tilearea_type.h"
#include "track_type.h"
#include "station_type.h"
#include "newgrf_station.h"
#include "water_map.h"

#include "table/strings.h"

/** Pasting modifiers. */
enum CopyPasteMode {
	CPM_WITH_RAIL_TRANSPORT  =   1 << 0, ///< copy-paste rail transport infrastructure
	CPM_WITH_ROAD_TRANSPORT  =   1 << 1, ///< copy-paste road transport infrastructure
	CPM_WITH_WATER_TRANSPORT =   1 << 2, ///< copy-paste water transport infrastructure
	CPM_WITH_AIR_TRANSPORT   =   1 << 3, ///< copy-paste air transport infrastructure
	CPM_ALL_TRANSPORT_MASK   = 0xF << 0, ///< bitmask with all transport types

	CPM_TERRAFORM_NONE       =   0 << 4, ///< do not alter tile heights
	CPM_TERRAFORM_MINIMAL    =   1 << 4, ///< terraform as less as possible to paste all objects at right heights
	CPM_TERRAFORM_FULL       =   2 << 4, ///< copy-paste all tile heights
	CPM_TERRAFORM_MASK       = 0x3 << 4, ///< bitmask to extract terraforming modes

	CPM_CONVERT_RAILTYPE     =   1 << 6, ///< convert rails to a given rail type
	CPM_MIRROR_SIGNALS       =   1 << 7, ///< mirror signals direction
	CPM_UPGRADE_BRIDGES      =   1 << 8, ///< upgrade bridge types to fastes possible
	CPM_FLAGS_MASK           = 0x7 << 6, ///< bitmask to mask all flag-like bits

	CPM_NONE                 =        0, ///< empty set of modes
	CPM_MASK = CPM_ALL_TRANSPORT_MASK | CPM_FLAGS_MASK | CPM_TERRAFORM_MASK, ///< all possible bits
	CPM_DEFAULT = CPM_ALL_TRANSPORT_MASK | CPM_TERRAFORM_MINIMAL, ///< defult mode
};
DECLARE_ENUM_AS_BIT_SET(CopyPasteMode)

/** Land leveling type used in e.g. #LevelPasteLand. */
enum CopyPasteLevelVariant {
	CPLV_LEVEL_ABOVE, ///< Lower the land until a given height reached.
	CPLV_LEVEL_BELOW, ///< Raise the land until a given height reached.
};

/** Parameters of a copy/paste command. */
struct CopyPasteParams {
	GenericTileArea src_area;         ///< The area we are copying from
	GenericTileArea dst_area;         ///< The area we are pasting at
	CopyPasteMode mode;               ///< Various flags telling what to copy and how to paste
	RailType railtype;                ///< Convert all rails to a given rail type (only in #CPM_CONVERT_RAILTYPE mode)
	DirTransformation transformation; ///< Transformation to perform on the content while copy-pasting
	int height_delta;                 ///< Amount of units to add to the height of each tile (appropriate terraforming mode must be set e.g. #CPM_TERRAFORM_FULL)
};

/** Summary error message for copy/paste command may vary depending on encountered errors.
 * While firing copy/paste command the summary messsage given with CMD_MSG is expected to
 * be STR_COPY_PASTE_ERROR_SUMMARY (which is "{8:STRING}") so a true message can be set
 * later through param #8. The constant below is the index of the param. */
static const int COPY_PASTE_ERR_SUMMARY_PARAM = 8;

/** Executes commands and gathers results of a paste process. */
struct PastingState {
	DoCommandFlag dc_flags;        ///< Flags to use when executing commands
	Money         overal_cost;     ///< Overal cost of currently executed paste command.
	CommandCost   last_result;     ///< Result of the most recent PastingState::DoCommand / PastingState::CollectCost / PastingState::CollectError.
	bool          had_success;     ///< If currently executed paste command had a successful action (at least once).
	StringID      err_summary;     ///< Summary message of the paste error.
	StringID      err_message;     ///< Detailed message of the paste error.
	TileIndex     err_tile;        ///< Tile where the last paste error occured.
	uint64        err_params[COPY_PASTE_ERR_SUMMARY_PARAM]; ///< Parameters for the paste error

	void DoCommand(TileIndex tile, uint32 p1, uint32 p2, uint32 cmd);
	void CollectCost(const CommandCost &cost, TileIndex tile, StringID error_summary = STR_ERROR_CAN_T_PASTE_HERE);
	void CollectError(TileIndex tile, StringID error_message, StringID error_summary = STR_ERROR_CAN_T_PASTE_HERE);
	bool IsInterrupted() const;

	inline Money GetAvailableMoney() const
	{
		return GetAvailableMoneyForCommand() - this->overal_cost;
	}
};

extern PastingState *_current_pasting;
extern TileIndex _paste_err_tile;

/**
 * Check if it is allowed to continue pasting.
 * @return True if pasting is interrupted, false otherwise
 */
static inline bool IsPastingInterrupted()
{
	return _current_pasting != NULL && _current_pasting->IsInterrupted();
}

void LevelPasteLand(const TileArea &ta, uint height, CopyPasteLevelVariant variant);
void CopyPasteHeights(const GenericTileArea &src_area, GenericTileIndex dst_area_north, DirTransformation transformation, int height_delta);

void CopyPastePlaceTracks(GenericTileIndex tile, RailType railtype, TrackBits tracks);
void CopyPastePlaceCannal(GenericTileIndex tile);
void CopyPastePlaceRailWaypoint(GenericTileIndex tile, StationID sid, Axis axis, byte gfx, StationClassID spec_class, byte spec_index, RailType rt, bool adjacent);
void CopyPastePlaceBuoy(GenericTileIndex tile, StationID sid, WaterClass wc);

void AfterCopyingStations(const CopyPasteParams &copy_paste);
void AfterPastingStations(const CopyPasteParams &copy_paste);

#endif /* COPYPASTE_CMD_H */
