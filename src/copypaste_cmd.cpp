/* $Id$ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file clipboard_cmd.cpp Helper functions for copy/paste commands. */

#include "stdafx.h"
#include "core/alloc_func.hpp"
#include "core/geometry_func.hpp"
#include "network/network.h"
#include "clear_map.h"
#include "clipboard_func.h"
#include "command_func.h"
#include "company_base.h"
#include "company_func.h"
#include "copypaste_cmd.h"
#include "error.h"
#include "gui.h"
#include "rail.h"
#include "rail_map.h"
#include "strings_func.h"
#include "tile_cmd.h"
#include "tilearea_func.h"
#include "tunnelbridge_map.h"
#include <cmath>

#include "table/strings.h"

static const uint INSTANT_COPY_PASTE_BUFFER = NUM_CLIPBOARD_BUFFERS - 1; ///< Index of the buffer reserved for the CmdInstantCopyPaste (temporary buffer)

PastingState *_current_pasting = NULL; ///< State of currently executed paste command
TileIndex _paste_err_tile = INVALID_TILE; ///< Tile where occured error of the last paste command

/**
 * Importance of an error in context of pasting. Bigger value is bigger importance.
 *
 * Various command errors may be encountered when copy/pasting. The importance decides which one
 * to show to the user - it will be one of most important errors, the one that was encountered
 * first. Errors with importance PEI_CRITICAL cancel a paste operation e.g. company run out of money.
 */
typedef int PasteErrorImportance;

static const PasteErrorImportance PEI_CRITICAL = 0x100; ///< Critical paste error

/**
 * Get importance of a certain error message.
 *
 * @param error_msg The error.
 * @return Importance of the error.
 *
 * @see PastingState::DoCommand
 * @see PastingState::IsInterrupted
 */
static PasteErrorImportance GetPasteErrorImportance(StringID error_msg)
{
	switch (error_msg) {
		/* Ignored errors, these will be newer stored as they are less important then the default error. */
		case STR_ERROR_ALREADY_LEVELLED:
		case STR_ERROR_ALREADY_BUILT:
			return -1;

		/* The default error which is set initially right before copy/pasting. */
		case STR_ERROR_NOTHING_TO_DO:
			return 0;

		/* "Can't distant join" must be the least important error among all non-ignored and non-default
		 * errors. We must be able to reset it to the default one (see AfterPasteingStations). */
		case STR_ERROR_CAN_T_DISTANT_JOIN:
			return 1;

		/* Messageless CMD_ERROR, it's not descriptive so it has a very low importance. */
		case INVALID_STRING_ID:
			return 2;

		/* Low importance errors */
		case STR_ERROR_MUST_REMOVE_RAILWAY_STATION_FIRST:
		case STR_ERROR_BUILDING_MUST_BE_DEMOLISHED:
		case STR_ERROR_MUST_DEMOLISH_AIRPORT_FIRST:
		case STR_ERROR_MUST_REMOVE_ROAD_STOP_FIRST:
		case STR_ERROR_MUST_DEMOLISH_DOCK_FIRST:
		case STR_ERROR_BUOY_IN_THE_WAY:
			return 3;

		/* High importance errors */
		default:
			return 4;

		/* Critical errors */
		case STR_ERROR_NOT_ENOUGH_CASH_REQUIRES_CURRENCY:
			return PEI_CRITICAL;
	}
}

/**
 * Check if the current paste operations is interrupted.
 *
 * @return True if pasting is interrupted, false if we can continue pasting.
 *
 * @see PasteErrorImportance
 * @see PastingState::DoCommand
 */
bool PastingState::IsInterrupted() const
{
	return GetPasteErrorImportance(this->err_message) >= PEI_CRITICAL;
}

/**
 * Call a given command as an ingredient of a paste operation.
 *
 * Costs and possible errors will be aggregated. After return, call PastingState::IsInterrupted to
 * test if the paste operation is disallowed to be continued.
 *
 * @param tile The tile to apply the command on.
 * @param p1 Additional data for the command.
 * @param p2 Additional data for the command.
 * @param cmd The command-id to execute (a value of the CMD_* enums) and the error summary message (CMD_MSG).
 * @return The cost of this operation or an error.
 *
 * @pre The command is not flagged with CMD_NO_TEST.
 * @pre The type of the command is CMDT_LANDSCAPE_CONSTRUCTION.
 *
 * @see PastingState::IsInterrupted
 * @see PastingState::CollectCost
 * @see PastingState::CollectError
 */
void PastingState::DoCommand(TileIndex tile, uint32 p1, uint32 p2, uint32 cmd)
{
	/* make sure we are still allowed to paste */
	if (this->IsInterrupted()) {
		this->last_result = CMD_ERROR; // mark that the command didn't succeed
		return;
	}

	/* PastingState::DoCommand can handle only fully predictable commands, those without
	 * CMD_NO_TEST flag. Unpredictable command have to be handled separately. */
	assert(!(GetCommandFlags(cmd) & CMD_NO_TEST));

	/* ignore some of the given flags, instead use those from the command proc table */
	DoCommandFlag flags = this->dc_flags;
	flags &= ~DC_AUTO & ~DC_NO_WATER & ~DC_ALL_TILES;
	flags |= CommandFlagsToDCFlags(GetCommandFlags(cmd));

	/* use given error message or the default one */
	StringID summary_error_msg = GB(cmd, 16, 16);
	if (summary_error_msg == 0) summary_error_msg = STR_ERROR_CAN_T_PASTE_HERE;

	/* test the command, output is the return value */
	CommandCost ret = ::DoCommand(tile, p1, p2, flags & ~DC_EXEC, cmd);

	/* apply if exec'ing */
	if (ret.Succeeded() && (flags & DC_EXEC)) {
		/* check if there is enough money */
		if (ret.GetCost() > 0 && this->GetAvailableMoney() < ret.GetCost()) {
			SetDParam(0, ret.GetCost());
			ret = CommandCost(STR_ERROR_NOT_ENOUGH_CASH_REQUIRES_CURRENCY);
		} else {
			CommandCost ret2 = ::DoCommand(tile, p1, p2, flags, cmd);
			assert(ret == ret2);
		}
	}

	/* aggregate costs */
	this->CollectCost(ret, tile, summary_error_msg);
}

/**
 * Aggreagate paste command costs without calling PastingState::DoCommand.
 *
 * The function works similarly to the PastingState::DoCommand but doesn't actually execute any
 * commands, it just collects a given result.
 *
 * When collecting a success, cost must be of type EXPENSES_CONSTRUCTION. A success also makes
 * STR_ERROR_NOTHING_TO_DO no more applies (we "did" something).
 *
 * Call PastingState::IsInterrupted to test whether the paste operation can be continued.
 *
 * @param cost The return value of the command, a cost or an error.
 * @param tile The tile the error concerns.
 * @param error_message Summary message of the error.
 *
 * @pre The company has enough money if DC_EXEC'ing.
 *
 * @see PastingState::IsInterrupted
 * @see PastingState::CollectError
 * @see PastingState::DoCommand
 */
void PastingState::CollectCost(const CommandCost &cost, TileIndex tile, StringID error_summary)
{
	if (cost.Succeeded()) {
		assert(!this->IsInterrupted());
		/* Currently only EXPENSES_CONSTRUCTION expenses are allowed when copy/pasting. If this
		 * is not sufficient, some upgrade will be required. To allow proper update of finacial
		 * statistics, the overal cost of paste operation will have to be stored separatly for
		 * each supported type of expenses. */
		assert(cost.GetExpensesType() == EXPENSES_CONSTRUCTION);

		/* make sure we are not expending too much */
		assert(!(this->dc_flags & DC_EXEC) || cost.GetCost() <= 0 || this->GetAvailableMoney() >= 0);

		this->had_success = true; // mark that we had a succes and STR_ERROR_NOTHING_TO_DO no more applies
		this->overal_cost += cost.GetCost();
		this->last_result = cost;
	} else {
		this->CollectError(tile, cost.GetErrorMessage(), error_summary);
	}
}

/**
 * Collect a paste error without calling PastingState::DoCommand or PastingState::CollectCost.
 *
 * The function works similary to PastingState::DoCommand and PastingState::CollectCost,
 * but it only generates an error. After return, call PastingState::IsInterrupted to test whether
 * the paste operation is allowd to be continued.
 *
 * @param tile The tile the error concerns.
 * @param error_message Error message.
 * @param error_message Summary error message.
 *
 * @see PastingState::IsInterrupted
 * @see PastingState::CollectCost
 * @see PastingState::DoCommand
 */
void PastingState::CollectError(TileIndex tile, StringID error_message, StringID error_summary)
{
	/* store the error only if it is more important then the previous one */
	if (GetPasteErrorImportance(error_message) > GetPasteErrorImportance(this->err_message)) {
		this->err_tile = IsValidTile(tile) ? tile : INVALID_TILE;
		this->err_message = error_message;
		this->err_summary = error_summary;
		CopyOutDParam(this->err_params, 0, lengthof(this->err_params));
	}

	this->last_result = CommandCost(error_message);
}

/**
 * Calculate how far tiles can be altered beyond a given paste area bound.
 *
 * When pasting, some tiles around the paste area may be altered (during terraforming).
 * The function return the limit on how far it can happen. Calculations are not exact,
 * the goal is to give a safe range that will include any possible case.
 *
 * Result is based on current and desired heights at neighbour corners of the paste area.
 *
 * @param curr_h1 Current height on the first corner.
 * @param curr_h2 Current height on the second corner.
 * @param new_h1 Desired height on the first corner.
 * @param new_h2 Desired height on the second corner.
 * @param length Distance (in tiles) between corners.
 * @return How far (in tiles) terraforming can reach beyond the given bound.
 *
 * @pre Tile heights and the length can't create an impossible layout, heights can't differ
 *      too much: \n
 *      <tt> Delta(curr_h1, curr_h2) <= length </tt> \n
 *      <tt> Delta(new_h1, new_h2) <= length </tt> \n
 *
 * @see CopyPasteAreasMayColide
 */
static uint CalcMaxPasteRange(uint curr_h1, uint new_h1, uint curr_h2, uint new_h2, uint length)
{
	uint min_curr_h = CeilDiv(max<int>(curr_h1 + curr_h2 - length, 0), 2);
	uint max_curr_h = min((curr_h1 + curr_h2 + length) / 2, MAX_TILE_HEIGHT);
	uint min_new_h = CeilDiv(max<int>(new_h1 + new_h2 - length, 0), 2);
	uint max_new_h = min((new_h1 + new_h2 + length) / 2, MAX_TILE_HEIGHT);

	return max(Delta(max_new_h, min_curr_h), Delta(max_curr_h, min_new_h));
}

/**
 * Test if this is safe to copy and paste contents of the map instantly, without
 * using an intermediate buffer.
 *
 * If the copy and the paste areas are close enough (especially when they intersect),
 * sequential copy-pasting may alter at some point of time those tile of the copy
 * area which hasn't been copied yet. In this case, further copy-pasting will read
 * modified values, not the original and this is somthing we don't want to happen.
 * We can deal with it by firstly copying all the content to the clipboard buffer and
 * then pasting it onto the map. This function tels us whether we should use the
 * clipboard as an intermediate buffer because there may happen such a colision.
 *
 * @param copy_paste What, where and how we are copying.
 * @return \c true if intermediate buffer might be required, \c false if it's surely not required
 *
 * @pre booth the source area and the destination area are on the main map
 *
 * @see CalcMaxPasteRange
 */
static bool CopyPasteAreasMayColide(const CopyPasteParams &copy_paste)
{
	/* No need to check surroundings if we are not terraforming. Just test for content intersection. */
	if ((copy_paste.mode & CPM_TERRAFORM_MASK) == CPM_TERRAFORM_NONE) return copy_paste.src_area.Intersects(copy_paste.dst_area);

	/* As we are interested in tile heights, increase areas to include all tile
	 * corners, also these at SW and SE borders. */
	TileArea src_corner_area(AsMainMapTile(copy_paste.src_area.tile), copy_paste.src_area.w + 1, copy_paste.src_area.h + 1);
	TileArea dst_corner_area(AsMainMapTile(copy_paste.dst_area.tile), copy_paste.dst_area.w + 1, copy_paste.dst_area.h + 1);

	DirTransformation inv_transformation = InvertDirTransform(copy_paste.transformation);
	/* source of the destination area most northern tile corner */
	TileIndex source_of_north = dst_corner_area.TransformedNorth(src_corner_area.tile, inv_transformation);

	/* calculate current and new heights on destination area corners */
	/* N */
	TileIndex dst_corner = dst_corner_area.tile;
	TileIndex src_corner = source_of_north;
	uint curr_n = TileHeight(dst_corner);
	uint new_n = TileHeight(src_corner) + copy_paste.height_delta;
	/* W */
	dst_corner = TILE_ADDXY(dst_corner_area.tile, dst_corner_area.w, 0);
	src_corner = dst_corner_area.TransformTile(dst_corner, source_of_north, inv_transformation);
	uint curr_w = TileHeight(dst_corner);
	uint new_w = TileHeight(src_corner) + copy_paste.height_delta;
	/* S */
	dst_corner = TILE_ADDXY(dst_corner_area.tile, dst_corner_area.w, dst_corner_area.h);
	src_corner = dst_corner_area.TransformTile(dst_corner, source_of_north, inv_transformation);
	uint curr_s = TileHeight(dst_corner);
	uint new_s = TileHeight(src_corner) + copy_paste.height_delta;
	/* E */
	dst_corner = TILE_ADDXY(dst_corner_area.tile, 0, dst_corner_area.h);
	src_corner = dst_corner_area.TransformTile(dst_corner, source_of_north, inv_transformation);
	uint curr_e = TileHeight(dst_corner);
	uint new_e = TileHeight(src_corner) + copy_paste.height_delta;

	/* calculate how far tiles can be altered beyon the paste area (safe approximation) */
	uint range_ne = CalcMaxPasteRange(curr_n, new_n, curr_e, new_e, dst_corner_area.h - 1);
	uint range_sw = CalcMaxPasteRange(curr_s, new_s, curr_w, new_w, dst_corner_area.h - 1);
	uint range_nw = CalcMaxPasteRange(curr_n, new_n, curr_w, new_w, dst_corner_area.w - 1);
	uint range_se = CalcMaxPasteRange(curr_s, new_s, curr_e, new_e, dst_corner_area.w - 1);

	/* calculate the exact area which may be altered by the paste process */
	uint x = TileX(dst_corner_area.tile);
	uint y = TileY(dst_corner_area.tile);
	range_ne = max(range_ne, x); // cut the area at the NE border (don't let x to go below 0)
	range_nw = max(range_nw, y); // cut the area at the NW border (don't let y to go below 0)
	TileArea forbidden_area(
			TileXY(x - range_ne, y - range_nw),
			dst_corner_area.w + range_ne + range_sw,
			dst_corner_area.h + range_nw + range_se);

	/* test if the source area is within the paste range */
	return src_corner_area.Intersects(forbidden_area);
}

/**
 * Calculate how much to add to each height of a tile while copy-pasteing.
 * @param src_area Area we are copying from.
 * @param dst_area Area we are pasting at.
 * @param transformation Transformation to perform.
 * @param additional_height Extra amount of units to add.
 */
static inline int CalcCopyPasteHeightDelta(const GenericTileArea &src_area, const GenericTileArea &dst_area, DirTransformation transformation, int additional_height)
{
	GenericTileArea dst_corners(dst_area.tile, dst_area.w + 1, dst_area.h + 1);
	GenericTileIndex source_of_north = dst_corners.TransformedNorth(src_area.tile, InvertDirTransform(transformation));
	return TileHeight(dst_corners.tile) - TileHeight(source_of_north) + additional_height;
}

/**
 * Do a sequential copy-pasting by calling appropriate CopyPasteCommandProc on each selected tile.
 * @param copy_paste What, where and how we are copying.
 */
static inline void DoCopyPaste(const CopyPasteParams &copy_paste)
{
	/* Copying to the clipboard buffer should always success.
	 * Some content may be intransformable (e.g. airport) so we can't use any transformation. */
	assert(IsMainMapTile(copy_paste.dst_area.tile) || (copy_paste.transformation == DTR_IDENTITY && (copy_paste.mode & CPM_TERRAFORM_MASK) == CPM_TERRAFORM_FULL));

	if ((copy_paste.mode & CPM_TERRAFORM_MASK) == CPM_TERRAFORM_FULL) {
		CopyPasteHeights(copy_paste.src_area, copy_paste.dst_area.tile, copy_paste.transformation, copy_paste.height_delta);
		if (IsPastingInterrupted()) return;
	}

	for (TransformationTileIteratorT<true, true> ti(copy_paste.src_area, copy_paste.src_area.TransformedNorth(copy_paste.dst_area.tile, copy_paste.transformation), copy_paste.transformation); IsValidTileIndex(ti); ++ti) {
		CopyPasteTileProc *proc = _tile_type_procs[GetTileType(ti.SrcTile())]->copy_paste_tile_proc;
		if (proc == NULL) continue;
		proc(ti.SrcTile(), ti.DstTile(), copy_paste);
		if (IsPastingInterrupted()) break;
	}

	if (IsMainMapTile(copy_paste.dst_area.tile)) {
		AfterPastingStations(copy_paste);
	} else {
		AfterCopyingStations(copy_paste);
	}
}

/**
 * Test if a given TileArea is valid.
 * @return \c true if the area is non-empty and fits inside the map, \c false otherwise.
 */
static CommandCost ValParamCopyPasteArea(const GenericTileArea &ta)
{
	if (!IsValidTile(ta.tile) ||
			!IsInsideBS(ta.w, 1, _settings_game.construction.clipboard_capacity) ||
			!IsInsideBS(ta.h, 1, _settings_game.construction.clipboard_capacity)) {
		return CMD_ERROR;
	}

	if (TileX(ta.tile) + ta.w > MapMaxX(MapOf(ta.tile)) ||
			TileY(ta.tile) + ta.h > MapMaxY(MapOf(ta.tile))) {
		return_cmd_error(STR_ERROR_TOO_CLOSE_TO_EDGE_OF_MAP_SUB);
	}

	return CommandCost();
}

/**
 * Test if a CopyPasteMode is valid.
 * @return \c true if does, \c false otherwise
 */
static bool ValParamCopyPasteMode(CopyPasteMode mode)
{
	if (mode & ~CPM_MASK) return false;
	mode &= CPM_TERRAFORM_MASK;
	return mode == CPM_TERRAFORM_NONE || mode == CPM_TERRAFORM_MINIMAL || mode == CPM_TERRAFORM_FULL;
}

/**
 * Copy content of a given tile area into the clipboard buffer.
 * @param ta The area to copy
 * @param buffer Clipboard buffer to copy into
 */
static void CopyToClipboard(Map *buffer, const TileArea &ta)
{
	AllocateClipboardBuffer(buffer, ta.w, ta.h);

	CopyPasteParams copy_paste = {
		GenericTileArea(ta),                         // src_area
		GenericTileArea(TileXY(0, 0, buffer), ta.w, ta.h), // dst_area
		CPM_ALL_TRANSPORT_MASK | CPM_TERRAFORM_FULL, // mode
		INVALID_RAILTYPE,                            // railtype
		DTR_IDENTITY,                                // transformation
		0                                            // height_delta
	};

	DoCopyPaste(copy_paste);
}

/**
 * Begin a paste process.
 * @param flags Flags for the command.
 * @param params What, where and how to paste.
 */
static void InitializePasting(DoCommandFlag flags, const CopyPasteParams &params)
{
	assert(_current_pasting == NULL);
	_current_pasting = new PastingState;

	_current_pasting->dc_flags    = flags | DC_PASTE;
	_current_pasting->overal_cost = 0;
	_current_pasting->had_success = false;
	_current_pasting->err_summary = STR_ERROR_CAN_T_PASTE_HERE;
	_current_pasting->err_message = STR_ERROR_NOTHING_TO_DO;
	MemSetT(_current_pasting->err_params, 0, lengthof(_current_pasting->err_params));
	_current_pasting->err_tile    = INVALID_TILE;
	_current_pasting->last_result = CommandCost(STR_ERROR_NOTHING_TO_DO);
}

/**
 * Finish a paste process.
 * @return Total cost.
 */
static CommandCost FinalizePasting()
{
	/* Set error string parameters */
	CopyInDParam(0, _current_pasting->err_params, lengthof(_current_pasting->err_params));
	/* Set error summary message (see COPY_PASTE_ERR_SUMMARY_PARAM for details). */
	SetDParam(COPY_PASTE_ERR_SUMMARY_PARAM, _current_pasting->err_summary);
	/* Store the error tile so the GUI (CcPaste) can highlight it. */
	_paste_err_tile = _current_pasting->err_tile;

	CommandCost ret;
	if (_current_pasting->had_success) {
		/* Return overal cost of the operation */
		ret = CommandCost(EXPENSES_CONSTRUCTION, _current_pasting->overal_cost);
		/* Here we are about to return a success. However, there could occured some meaningful
		 * errors (those except "already built", "already leveled" etc.) and we should inform
		 * the user that not everything went right. Show the message now. */
		if ((_current_pasting->dc_flags & DC_EXEC) && IsLocalCompany() && GetPasteErrorImportance(_current_pasting->err_message) > GetPasteErrorImportance(STR_ERROR_NOTHING_TO_DO)) {
			ShowErrorMessage(_current_pasting->err_summary, _current_pasting->err_message, WL_INFO);
		} else {
			/* If we are not showing error message then clear the error tile to prevent GUI
			 * (CcPaste) from higlighting it. */
			_paste_err_tile = INVALID_TILE;
		}
	} else {
		/* Return an error if we didn't have any success. */
		ret = CommandCost(_current_pasting->err_message);
	}

	/* cleanup */
	delete _current_pasting;
	_current_pasting = NULL;

	return ret;
}

/**
 * Paste onto the main map content of a clipboard buffer.
 * @param buffer The buffer.
 * @param tile Tile where to paste (most northern).
 * @param flags Flags for the command.
 * @param mode Copy-paste mode.
 * @param transformation Transformation to perform.
 * @param railtype #RailType to convert to.
 * @param additional_height_delta Additional amount of units to add to each tile height.
 * @return Total cost.
 */
static CommandCost PasteFromClipboard(Map *buffer, TileIndex tile, DoCommandFlag flags, CopyPasteMode mode, DirTransformation transformation, RailType railtype, int additional_height_delta)
{
	assert(!IsClipboardBufferEmpty(buffer));

	CopyPasteParams copy_paste;

	/* calculate and validate copy/paste area */
	copy_paste.src_area = GenericTileArea(TileXY(0, 0, buffer), MapMaxX(buffer), MapMaxY(buffer));
	copy_paste.dst_area = TransformTileArea(copy_paste.src_area, GenericTileIndex(tile), transformation);
	CommandCost ret = ValParamCopyPasteArea(copy_paste.dst_area);
	if (ret.Failed()) return ret;

	copy_paste.mode = mode;
	copy_paste.railtype = railtype;
	copy_paste.transformation = transformation;
	copy_paste.height_delta = CalcCopyPasteHeightDelta(copy_paste.src_area, copy_paste.dst_area, transformation, additional_height_delta);

	/* do sequential copy-pasting */
	InitializePasting(flags, copy_paste);
	DoCopyPaste(copy_paste);
	return FinalizePasting();
}

/**
 * Copy tile area into clipboard.
 *
 * @param tile Northern tile of the area to copy.
 * @param flags Command flags.
 * @param p1 Various bits:
 *    \li bits  0..1   [2] - clipboard buffer index
 *    \li bits  2..31 [30] - [ unused ]
 * @param p2 Various bits:
 *    \li bits  0..5   [6] - width of area to copy
 *    \li bits  6..11  [6] - height of area to copy
 *    \li bits 12..31 [20] - [ unused ]
 * @param text Unused.
 * @return The cost of this operation or an error.
 */
CommandCost CmdCopyToClipboard(TileIndex tile, DoCommandFlag flags, uint32 p1, uint32 p2, const char *text)
{
	/* clipboard is available only in a sigle player game and only to the local company */
	if (_networking || !IsLocalCompany()) return CMD_ERROR;

	/* extract and validate clipboard buffer index */
	uint index = GB(p1, 0, 2);
	if (index >= NUM_CLIPBOARD_BUFFERS || index == INSTANT_COPY_PASTE_BUFFER) return CMD_ERROR;

	/* calculate and validate source area */
	TileArea src_area(tile, GB(p2, 0, 6), GB(p2, 6, 6));
	CommandCost ret = ValParamCopyPasteArea(src_area);
	if (ret.Failed()) return ret;

	/* copy to clipboard only when executing (DC_EXEC) */
	if (flags & DC_EXEC) CopyToClipboard(GetClipboardBuffer(index), src_area);

	/* return the cost */
	return CommandCost(INVALID_EXPENSES, 0);
}

/**
 * Paste clipboard contents onto the map.
 *
 * @param tile Tile where to paste (northern).
 * @param flags Command flags.
 * @param p1 Various bits:
 *    \li bits  0..1   [2] - clipboard buffer index
 *    \li bits  2..27 [26] - [ unused ]
 *    \li bits 28..31  [4] - rail type (RailType) to convert to, ignored if CPM_CONVERT_RAILTYPE mode is off
 * @param p2 Various bits:
 *    \li bits  0..11 [12] - [ unused ]
 *    \li bits 12..15  [4] - additional amount of tile heights to add to each tile (-8..7)
 *    \li bits 16..18  [3] - transformation to perform (DirTransformation)
 *    \li bits 19..27  [9] - mode (CopyPasteMode)
 *    \li bits 28..31  [4] - [ unused ]
 * @param text Unused.
 * @return The cost of this operation or an error.
 */
CommandCost CmdPasteFromClipboard(TileIndex tile, DoCommandFlag flags, uint32 p1, uint32 p2, const char *text)
{
	/* extract and validate clipboard buffer index */
	uint index = GB(p1, 0, 2);
	if (index >= NUM_CLIPBOARD_BUFFERS || index == INSTANT_COPY_PASTE_BUFFER) return CMD_ERROR;

	/* clipboard is available only in a sigle player game and only to the local company */
	if (_networking || !IsLocalCompany() || IsClipboardBufferEmpty(GetClipboardBuffer(index))) return CMD_ERROR;

	/* extract and validate copy/paste mode */
	CopyPasteMode mode = (CopyPasteMode)GB(p2, 19, 9);
	if (!ValParamCopyPasteMode(mode)) return CMD_ERROR;

	/* extract and validate rail type */
	RailType railtype = (RailType)GB(p1, 28,  4);
	if (!ValParamRailtype(railtype)) return CMD_ERROR;

	/* extract transformation and additional height delta */
	DirTransformation transformation = (DirTransformation)GB(p2, 16,  3);
	int additional_height_delta = GB(p2, 12, 4);
	additional_height_delta |= -(additional_height_delta & (1 << 3)); // propagate sign bit

	return PasteFromClipboard(GetClipboardBuffer(index), tile, flags, mode, transformation, railtype, additional_height_delta);
}

/**
 * Copy a piece of map and instantly paste at given location.
 *
 * @param tile Tile where to paste (northern).
 * @param flags Command flags.
 * @param p1 Various bits:
 *    \li bits  0..27 [28] - northern tile of the source area
 *    \li bits 28..31  [4] - rail type (RailType) to convert to, ignored if CPM_CONVERT_RAILTYPE mode is off
 * @param p2 Various bits:
 *    \li bits  0..5   [6] - source area width
 *    \li bits  6..11  [6] - source area height
 *    \li bits 12..15  [4] - additional amount of tile heights to add to each tile (-8..7)
 *    \li bits 16..18  [3] - transformation to perform (DirTransformation)
 *    \li bits 19..27  [9] - mode (CopyPasteMode)
 *    \li bits 28..31  [4] - [ unused ]
 * @param text Unused.
 * @return The cost of this operation or an error.
 */
CommandCost CmdInstantCopyPaste(TileIndex tile, DoCommandFlag flags, uint32 p1, uint32 p2, const char *text)
{
	CopyPasteParams copy_paste;

	/* extract and validate source area */
	copy_paste.src_area.tile = GenericTileIndex(TileIndex(GB(p1, 0, 28)));
	copy_paste.src_area.w = GB(p2, 0, 6);
	copy_paste.src_area.h = GB(p2, 6, 6);
	CommandCost ret = ValParamCopyPasteArea(copy_paste.src_area);
	if (ret.Failed()) return ret;

	/* calculate and validate destination area */
	copy_paste.dst_area = TransformTileArea(copy_paste.src_area, GenericTileIndex(tile), copy_paste.transformation);
	ret = ValParamCopyPasteArea(copy_paste.dst_area);
	if (ret.Failed()) return ret;

	/* extract and validate copy/paste mode */
	copy_paste.mode = (CopyPasteMode)GB(p2, 19, 9);
	if (!ValParamCopyPasteMode(copy_paste.mode)) return CMD_ERROR;

	/* extract and validate rail type */
	copy_paste.railtype = (RailType)GB(p1, 28,  4);
	if (!ValParamRailtype(copy_paste.railtype)) return CMD_ERROR;

	/* extract transformation */
	copy_paste.transformation = (DirTransformation)GB(p2, 16,  3);

	/* extract the additional number of height units */
	int additional_height_delta = GB(p2, 12, 4); // this is a 4-bit SIGNED integer (-8..7)
	additional_height_delta |= -(additional_height_delta & (1 << 3)); // propagate the sign bit

	/* calculate the height */
	copy_paste.height_delta = CalcCopyPasteHeightDelta(copy_paste.src_area, copy_paste.dst_area, copy_paste.transformation, additional_height_delta);

	/* when copy and paste areas are too close each other, firstly
	 * copy to the clipboard and then from the clipboard to the map */
	if (CopyPasteAreasMayColide(copy_paste)) {
		Map *clipboard = GetClipboardBuffer(INSTANT_COPY_PASTE_BUFFER);
		/* Copy to a buffer, but only in the first stage of the command.
		 * In a single player game and also while we are a server, the first one is non-DC_EXEC
		 * stage (which is fallowed then by a DC_EXEC stage). When we are a client, there is only
		 * one stage which is either a single non-DC_EXEC stage (shift pressed), or a single DC_EXEC
		 * stage (command comming from the network). */
		if ((_networking && !_network_server) || !(flags & DC_EXEC)) {
			CopyToClipboard(clipboard, copy_paste.src_area);
		}
		/* paste from the clipboard */
		ret = PasteFromClipboard(clipboard, tile, flags, copy_paste.mode, copy_paste.transformation, copy_paste.railtype, additional_height_delta);
	} else {
		/* copy/paste directly */
		InitializePasting(flags, copy_paste);
		DoCopyPaste(copy_paste);
		ret = FinalizePasting();
	}
	return ret;
}
