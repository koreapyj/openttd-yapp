/* $Id: smallmap_gui.cpp 24958 2013-02-02 19:52:56Z rubidium $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file smallmap_gui.cpp GUI that shows a small map of the world with metadata like owner or height. */

#include "stdafx.h"
#include "clear_map.h"
#include "industry.h"
#include "station_map.h"
#include "landscape.h"
#include "window_gui.h"
#include "tree_map.h"
#include "viewport_func.h"
#include "town.h"
#include "blitter/factory.hpp"
#include "tunnelbridge_map.h"
#include "strings_func.h"
#include "core/endian_func.hpp"
#include "vehicle_base.h"
#include "sound_func.h"
#include "window_func.h"
#include "company_base.h"
#include "station_base.h"
#include "company_func.h"
#include "cargotype.h"
#include "core/smallmap_type.hpp"

#include "widgets/smallmap_widget.h"

#include "table/strings.h"

static int _smallmap_industry_count; ///< Number of used industries
static int _smallmap_company_count;  ///< Number of entries in the owner legend.
static int _smallmap_cargo_count;    ///< Number of entries in the cargo legend.

static const int NUM_NO_COMPANY_ENTRIES = 4; ///< Number of entries in the owner legend that are not companies.

static const uint8 PC_ROUGH_LAND      = 0x52; ///< Dark green palette colour for rough land.
static const uint8 PC_GRASS_LAND      = 0x54; ///< Dark green palette colour for grass land.
static const uint8 PC_BARE_LAND       = 0x37; ///< Brown palette colour for bare land.
static const uint8 PC_FIELDS          = 0x25; ///< Light brown palette colour for fields.
static const uint8 PC_TREES           = 0x57; ///< Green palette colour for trees.
static const uint8 PC_WATER           = 0xCA; ///< Dark blue palette colour for water.

/** Macro for ordinary entry of LegendAndColour */
#define MK(a, b) {a, b, INVALID_INDUSTRYTYPE, 0, INVALID_COMPANY, INVALID_CARGO, true, false, false}

/** Macro for a height legend entry with configurable colour. */
#define MC(height)  {0, STR_TINY_BLACK_HEIGHT, INVALID_INDUSTRYTYPE, height, INVALID_COMPANY, INVALID_CARGO, true, false, false}

/** Macro for a height legend entry break marker with configurable colour. */
#define MCS(height)  {0, STR_TINY_BLACK_HEIGHT, INVALID_INDUSTRYTYPE, height, INVALID_COMPANY, true, false, true}

/** Macro for non-company owned property entry of LegendAndColour */
#define MO(a, b) {a, b, INVALID_INDUSTRYTYPE, 0, INVALID_COMPANY, INVALID_CARGO, true, false, false}

/** Macro used for forcing a rebuild of the owner legend the first time it is used. */
#define MOEND() {0, 0, INVALID_INDUSTRYTYPE, 0, OWNER_NONE, INVALID_CARGO, true, true, false}

/** Macro for end of list marker in arrays of LegendAndColour */
#define MKEND() {0, STR_NULL, INVALID_INDUSTRYTYPE, 0, INVALID_COMPANY, INVALID_CARGO, true, true, false}

/**
 * Macro for break marker in arrays of LegendAndColour.
 * It will have valid data, though
 */
#define MS(a, b) {a, b, INVALID_INDUSTRYTYPE, 0, INVALID_COMPANY, INVALID_CARGO, true, false, true}

/** Structure for holding relevant data for legends in small map */
struct LegendAndColour {
	uint8 colour;              ///< Colour of the item on the map.
	StringID legend;           ///< String corresponding to the coloured item.
	IndustryType type;         ///< Type of industry. Only valid for industry entries.
	uint8 height;              ///< Height in tiles. Only valid for height legend entries.
	CompanyID company;         ///< Company to display. Only valid for company entries of the owner legend.
	CargoID cid;               ///< Cargo type to display. Only valid for entries of the cargo legend.
	bool show_on_map;          ///< For filtering industries, if \c true, industry is shown on the map in colour.
	bool end;                  ///< This is the end of the list.
	bool col_break;            ///< Perform a column break and go further at the next column.
};

/** Legend text giving the colours to look for on the minimap */
static LegendAndColour _legend_land_contours[] = {
	/* The colours for the following values are set at BuildLandLegend() based on each colour scheme. */
	MC(0),
	MC(16),
	MC(32),
	MC(48),
	MC(64),
	MC(80),

	MCS(96),
	MC(112),
	MC(128),
	MC(144),
	MC(160),
	MC(176),

	MCS(192),
	MC(208),
	MC(224),
	MC(240),
	MC(255),

	MS(0xD7, STR_SMALLMAP_LEGENDA_ROADS),
	MK(0x0A, STR_SMALLMAP_LEGENDA_RAILROADS),
	MK(0x98, STR_SMALLMAP_LEGENDA_STATIONS_AIRPORTS_DOCKS),
	MK(0xB5, STR_SMALLMAP_LEGENDA_BUILDINGS_INDUSTRIES),
	MK(0x0F, STR_SMALLMAP_LEGENDA_VEHICLES),
	MKEND()
};

static const LegendAndColour _legend_vehicles[] = {
	MK(PC_RED,             STR_SMALLMAP_LEGENDA_TRAINS),
	MK(PC_YELLOW,          STR_SMALLMAP_LEGENDA_ROAD_VEHICLES),
	MK(PC_LIGHT_BLUE,      STR_SMALLMAP_LEGENDA_SHIPS),
	MK(PC_WHITE,           STR_SMALLMAP_LEGENDA_AIRCRAFT),

	MS(PC_BLACK,           STR_SMALLMAP_LEGENDA_TRANSPORT_ROUTES),
	MK(PC_DARK_RED,        STR_SMALLMAP_LEGENDA_BUILDINGS_INDUSTRIES),
	MKEND()
};

static const LegendAndColour _legend_routes[] = {
	MK(PC_BLACK,           STR_SMALLMAP_LEGENDA_ROADS),
	MK(PC_GREY,            STR_SMALLMAP_LEGENDA_RAILROADS),
	MK(PC_DARK_RED,        STR_SMALLMAP_LEGENDA_BUILDINGS_INDUSTRIES),

	MS(PC_VERY_DARK_BROWN, STR_SMALLMAP_LEGENDA_RAILROAD_STATION),
	MK(PC_ORANGE,          STR_SMALLMAP_LEGENDA_TRUCK_LOADING_BAY),
	MK(PC_YELLOW,          STR_SMALLMAP_LEGENDA_BUS_STATION),
	MK(PC_RED,             STR_SMALLMAP_LEGENDA_AIRPORT_HELIPORT),
	MK(PC_LIGHT_BLUE,      STR_SMALLMAP_LEGENDA_DOCK),
	MKEND()
};

static const LegendAndColour _legend_vegetation[] = {
	MK(PC_ROUGH_LAND,      STR_SMALLMAP_LEGENDA_ROUGH_LAND),
	MK(PC_GRASS_LAND,      STR_SMALLMAP_LEGENDA_GRASS_LAND),
	MK(PC_BARE_LAND,       STR_SMALLMAP_LEGENDA_BARE_LAND),
	MK(PC_FIELDS,          STR_SMALLMAP_LEGENDA_FIELDS),
	MK(PC_TREES,           STR_SMALLMAP_LEGENDA_TREES),
	MK(PC_GREEN,           STR_SMALLMAP_LEGENDA_FOREST),

	MS(PC_GREY,            STR_SMALLMAP_LEGENDA_ROCKS),
	MK(PC_ORANGE,          STR_SMALLMAP_LEGENDA_DESERT),
	MK(PC_LIGHT_BLUE,      STR_SMALLMAP_LEGENDA_SNOW),
	MK(PC_BLACK,           STR_SMALLMAP_LEGENDA_TRANSPORT_ROUTES),
	MK(PC_DARK_RED,        STR_SMALLMAP_LEGENDA_BUILDINGS_INDUSTRIES),
	MKEND()
};

static LegendAndColour _legend_land_owners[NUM_NO_COMPANY_ENTRIES + MAX_COMPANIES + 1] = {
	MO(PC_WATER,           STR_SMALLMAP_LEGENDA_WATER),
	MO(0x00,               STR_SMALLMAP_LEGENDA_NO_OWNER), // This colour will vary depending on settings.
	MO(PC_DARK_RED,        STR_SMALLMAP_LEGENDA_TOWNS),
	MO(PC_DARK_GREY,       STR_SMALLMAP_LEGENDA_INDUSTRIES),
	/* The legend will be terminated the first time it is used. */
	MOEND(),
};

#undef MK
#undef MC
#undef MCS
#undef MS
#undef MO
#undef MOEND
#undef MKEND

/**
 * Allow room for all industries, plus a terminator entry
 * This is required in order to have the industry slots all filled up
 */
static LegendAndColour _legend_from_industries[NUM_INDUSTRYTYPES + 1];
/** For connecting industry type to position in industries list(small map legend) */
static uint _industry_to_list_pos[NUM_INDUSTRYTYPES];
/** Legend text for the cargo types in the route link legend. */
static LegendAndColour _legend_from_cargoes[NUM_CARGO + 1];
/** For connecting cargo type to position in route link legend. */
static uint _cargotype_to_list_pos[NUM_CARGO];
/** Show heightmap in industry and owner mode of smallmap window. */
static bool _smallmap_show_heightmap = false;
/** Highlight a specific industry type */
static IndustryType _smallmap_industry_highlight = INVALID_INDUSTRYTYPE;
/** State of highlight blinking */
static bool _smallmap_industry_highlight_state;
/** For connecting company ID to position in owner list (small map legend) */
static uint _company_to_list_pos[MAX_COMPANIES];

/**
 * Fills an array for the industries legends.
 */
void BuildIndustriesLegend()
{
	uint j = 0;

	/* Add each name */
	for (uint8 i = 0; i < NUM_INDUSTRYTYPES; i++) {
		IndustryType ind = _sorted_industry_types[i];
		const IndustrySpec *indsp = GetIndustrySpec(ind);
		if (indsp->enabled) {
			_legend_from_industries[j].legend = indsp->name;
			_legend_from_industries[j].colour = indsp->map_colour;
			_legend_from_industries[j].type = ind;
			_legend_from_industries[j].show_on_map = true;
			_legend_from_industries[j].col_break = false;
			_legend_from_industries[j].end = false;

			/* Store widget number for this industry type. */
			_industry_to_list_pos[ind] = j;
			j++;
		}
	}
	/* Terminate the list */
	_legend_from_industries[j].end = true;

	/* Store number of enabled industries */
	_smallmap_industry_count = j;
}

/** Fills the array for the route link legend. */
void BuildCargoTypesLegend()
{
	uint j = 0;

	/* Add all standard cargo types. */
	const CargoSpec *cs;
	FOR_ALL_SORTED_STANDARD_CARGOSPECS(cs) {
		_legend_from_cargoes[j].legend = cs->name;
		_legend_from_cargoes[j].colour = cs->legend_colour;
		_legend_from_cargoes[j].cid = cs->Index();
		_legend_from_cargoes[j].show_on_map = true;
		_legend_from_cargoes[j].col_break = false;
		_legend_from_cargoes[j].end = false;

		/* Store widget number for this cargo type. */
		_cargotype_to_list_pos[cs->Index()] = j;
		j++;
	}

	/* Terminate list. */
	_legend_from_cargoes[j].end = true;

	/* Store number of enabled cargoes. */
	_smallmap_cargo_count = j;
}

static const LegendAndColour * const _legend_table[] = {
	_legend_land_contours,
	_legend_vehicles,
	_legend_from_industries,
	_legend_from_cargoes,
	_legend_routes,
	_legend_vegetation,
	_legend_land_owners,
};

#define MKCOLOUR(x)         TO_LE32X(x)

#define MKCOLOUR_XXXX(x)    (MKCOLOUR(0x01010101) * (uint)(x))
#define MKCOLOUR_X0X0(x)    (MKCOLOUR(0x01000100) * (uint)(x))
#define MKCOLOUR_0X0X(x)    (MKCOLOUR(0x00010001) * (uint)(x))
#define MKCOLOUR_0XX0(x)    (MKCOLOUR(0x00010100) * (uint)(x))
#define MKCOLOUR_X00X(x)    (MKCOLOUR(0x01000001) * (uint)(x))

#define MKCOLOUR_XYXY(x, y) (MKCOLOUR_X0X0(x) | MKCOLOUR_0X0X(y))
#define MKCOLOUR_XYYX(x, y) (MKCOLOUR_X00X(x) | MKCOLOUR_0XX0(y))

#define MKCOLOUR_0000       MKCOLOUR_XXXX(0x00)
#define MKCOLOUR_0FF0       MKCOLOUR_0XX0(0xFF)
#define MKCOLOUR_F00F       MKCOLOUR_X00X(0xFF)
#define MKCOLOUR_FFFF       MKCOLOUR_XXXX(0xFF)

/** Height map colours for the green colour scheme, ordered by height. */
static const uint32 _green_map_heights[] = {
	MKCOLOUR(0x59595958), // height 0
	MKCOLOUR(0x59595958), // height 1
	MKCOLOUR(0x59595958), // height 2
	MKCOLOUR(0X59595959), // height 3
	MKCOLOUR(0X59595959), // height 4
	MKCOLOUR(0X5959595A), // height 5
	MKCOLOUR(0X5959595A), // height 6
	MKCOLOUR(0X59595A59), // height 7
	MKCOLOUR(0X59595A59), // height 8
	MKCOLOUR(0X59595A5A), // height 9
	MKCOLOUR(0X59595A5A), // height 10
	MKCOLOUR(0X595A5959), // height 11
	MKCOLOUR(0X595A5959), // height 12
	MKCOLOUR(0X595A595A), // height 13
	MKCOLOUR(0X595A595A), // height 14
	MKCOLOUR(0X595A5A59), // height 15
	MKCOLOUR(0X595A5A59), // height 16
	MKCOLOUR(0X595A5A5A), // height 17
	MKCOLOUR(0X595A5A5A), // height 18
	MKCOLOUR(0X5A595959), // height 19
	MKCOLOUR(0X5A595959), // height 20
	MKCOLOUR(0X5A59595A), // height 21
	MKCOLOUR(0X5A59595A), // height 22
	MKCOLOUR(0X5A595A59), // height 23
	MKCOLOUR(0X5A595A59), // height 24
	MKCOLOUR(0X5A595A5A), // height 25
	MKCOLOUR(0X5A595A5A), // height 26
	MKCOLOUR(0X5A5A5959), // height 27
	MKCOLOUR(0X5A5A5959), // height 28
	MKCOLOUR(0X5A5A595A), // height 29
	MKCOLOUR(0X5A5A595A), // height 30
	MKCOLOUR(0X5A5A5A59), // height 31
	MKCOLOUR(0X5A5A5A59), // height 32
	MKCOLOUR(0x5A5A5A5A), // height 33
	MKCOLOUR(0x5A5A5A5A), // height 34
	MKCOLOUR(0x5A5A5A5B), // height 35
	MKCOLOUR(0x5A5A5A5B), // height 36
	MKCOLOUR(0x5A5A5B5A), // height 37
	MKCOLOUR(0x5A5A5B5A), // height 38
	MKCOLOUR(0x5A5A5B5B), // height 39
	MKCOLOUR(0x5A5A5B5B), // height 40
	MKCOLOUR(0x5A5B5A5A), // height 41
	MKCOLOUR(0x5A5B5A5A), // height 42
	MKCOLOUR(0x5A5B5A5B), // height 43
	MKCOLOUR(0x5A5B5A5B), // height 44
	MKCOLOUR(0x5A5B5B5A), // height 45
	MKCOLOUR(0x5A5B5B5A), // height 46
	MKCOLOUR(0x5A5B5B5B), // height 47
	MKCOLOUR(0x5A5B5B5B), // height 48
	MKCOLOUR(0x5B5A5A5A), // height 49
	MKCOLOUR(0x5B5A5A5A), // height 50
	MKCOLOUR(0x5B5A5A5B), // height 51
	MKCOLOUR(0x5B5A5A5B), // height 52
	MKCOLOUR(0x5B5A5B5A), // height 53
	MKCOLOUR(0x5B5A5B5A), // height 54
	MKCOLOUR(0x5B5A5B5B), // height 55
	MKCOLOUR(0x5B5A5B5B), // height 56
	MKCOLOUR(0x5B5B5A5A), // height 57
	MKCOLOUR(0x5B5B5A5A), // height 58
	MKCOLOUR(0x5B5B5A5B), // height 59
	MKCOLOUR(0x5B5B5A5B), // height 60
	MKCOLOUR(0x5B5B5B5B), // height 61
	MKCOLOUR(0x5B5B5B5B), // height 62
	MKCOLOUR(0x5B5B5B5C), // height 63
	MKCOLOUR(0x5B5B5B5C), // height 64
	MKCOLOUR(0x5B5B5C5B), // height 65
	MKCOLOUR(0x5B5B5C5B), // height 66
	MKCOLOUR(0x5B5B5C5C), // height 67
	MKCOLOUR(0x5B5B5C5C), // height 68
	MKCOLOUR(0x5B5C5B5B), // height 69
	MKCOLOUR(0x5B5C5B5B), // height 70
	MKCOLOUR(0x5B5C5B5C), // height 71
	MKCOLOUR(0x5B5C5B5C), // height 72
	MKCOLOUR(0x5B5C5C5B), // height 73
	MKCOLOUR(0x5B5C5C5B), // height 74
	MKCOLOUR(0x5B5C5C5C), // height 75
	MKCOLOUR(0x5B5C5C5C), // height 76
	MKCOLOUR(0x5C5B5B5B), // height 77
	MKCOLOUR(0x5C5B5B5B), // height 78
	MKCOLOUR(0x5C5B5B5C), // height 79
	MKCOLOUR(0x5C5B5B5C), // height 80
	MKCOLOUR(0x5C5B5C5B), // height 81
	MKCOLOUR(0x5C5B5C5B), // height 82
	MKCOLOUR(0x5C5B5C5C), // height 83
	MKCOLOUR(0x5C5B5C5C), // height 84
	MKCOLOUR(0x5C5C5B5B), // height 85
	MKCOLOUR(0x5C5C5B5B), // height 86
	MKCOLOUR(0x5C5C5B5C), // height 87
	MKCOLOUR(0x5C5C5B5C), // height 88
	MKCOLOUR(0x5C5C5C5C), // height 89
	MKCOLOUR(0x5C5C5C5C), // height 90
	MKCOLOUR(0x5C5C5C5D), // height 91
	MKCOLOUR(0x5C5C5C5D), // height 92
	MKCOLOUR(0x5C5C5D5C), // height 93
	MKCOLOUR(0x5C5C5D5C), // height 94
	MKCOLOUR(0x5C5C5D5D), // height 95
	MKCOLOUR(0x5C5C5D5D), // height 96
	MKCOLOUR(0x5C5D5C5C), // height 97
	MKCOLOUR(0x5C5D5C5C), // height 98
	MKCOLOUR(0x5C5D5C5D), // height 99
	MKCOLOUR(0x5C5D5C5D), // height 100
	MKCOLOUR(0x5C5D5D5C), // height 101
	MKCOLOUR(0x5C5D5D5C), // height 102
	MKCOLOUR(0x5C5D5D5D), // height 103
	MKCOLOUR(0x5C5D5D5D), // height 104
	MKCOLOUR(0x5D5C5C5C), // height 105
	MKCOLOUR(0x5D5C5C5C), // height 106
	MKCOLOUR(0x5D5C5C5D), // height 107
	MKCOLOUR(0x5D5C5C5D), // height 108
	MKCOLOUR(0x5D5C5D5C), // height 109
	MKCOLOUR(0x5D5C5D5C), // height 110
	MKCOLOUR(0x5D5C5D5D), // height 111
	MKCOLOUR(0x5D5C5D5D), // height 112
	MKCOLOUR(0x5D5D5C5C), // height 113
	MKCOLOUR(0x5D5D5C5C), // height 114
	MKCOLOUR(0x5D5D5C5D), // height 115
	MKCOLOUR(0x5D5D5C5D), // height 116
	MKCOLOUR(0x5D5D5D5D), // height 117
	MKCOLOUR(0x5D5D5D5D), // height 118
	MKCOLOUR(0x5D5D5D5E), // height 119
	MKCOLOUR(0x5D5D5D5E), // height 120
	MKCOLOUR(0x5D5D5E5D), // height 121
	MKCOLOUR(0x5D5D5E5D), // height 122
	MKCOLOUR(0x5D5D5E5E), // height 123
	MKCOLOUR(0x5D5D5E5E), // height 124
	MKCOLOUR(0x5D5E5D5D), // height 125
	MKCOLOUR(0x5D5E5D5D), // height 126
	MKCOLOUR(0x5D5E5D5E), // height 127
	MKCOLOUR(0x5D5E5D5E), // height 128
	MKCOLOUR(0x5D5E5E5D), // height 129
	MKCOLOUR(0x5D5E5E5D), // height 130
	MKCOLOUR(0x5D5E5E5E), // height 131
	MKCOLOUR(0x5D5E5E5E), // height 132
	MKCOLOUR(0x5E5D5D5D), // height 133
	MKCOLOUR(0x5E5D5D5D), // height 134
	MKCOLOUR(0x5E5D5D5E), // height 135
	MKCOLOUR(0x5E5D5D5E), // height 136
	MKCOLOUR(0x5E5D5E5D), // height 137
	MKCOLOUR(0x5E5D5E5D), // height 138
	MKCOLOUR(0x5E5D5E5E), // height 139
	MKCOLOUR(0x5E5D5E5E), // height 140
	MKCOLOUR(0x5E5D5D5D), // height 141
	MKCOLOUR(0x5E5D5D5D), // height 142
	MKCOLOUR(0x5E5D5D5E), // height 143
	MKCOLOUR(0x5E5D5D5E), // height 144
	MKCOLOUR(0x5E5E5E5E), // height 145
	MKCOLOUR(0x5E5E5E5E), // height 146
	MKCOLOUR(0x5E5E5E5F), // height 147
	MKCOLOUR(0x5E5E5E5F), // height 148
	MKCOLOUR(0x5E5E5F5E), // height 149
	MKCOLOUR(0x5E5E5F5E), // height 150
	MKCOLOUR(0x5E5E5F5F), // height 151
	MKCOLOUR(0x5E5E5F5F), // height 152
	MKCOLOUR(0x5E5F5E5E), // height 153
	MKCOLOUR(0x5E5F5E5E), // height 154
	MKCOLOUR(0x5E5F5E5F), // height 155
	MKCOLOUR(0x5E5F5E5F), // height 156
	MKCOLOUR(0x5E5F5F5E), // height 157
	MKCOLOUR(0x5E5F5F5E), // height 158
	MKCOLOUR(0x5E5F5F5F), // height 159
	MKCOLOUR(0x5E5F5F5F), // height 160
	MKCOLOUR(0x5F5E5E5E), // height 161
	MKCOLOUR(0x5F5E5E5E), // height 162
	MKCOLOUR(0x5F5E5E5F), // height 163
	MKCOLOUR(0x5F5E5E5F), // height 164
	MKCOLOUR(0x5F5E5F5E), // height 165
	MKCOLOUR(0x5F5E5F5E), // height 166
	MKCOLOUR(0x5F5E5F5F), // height 167
	MKCOLOUR(0x5F5E5F5F), // height 168
	MKCOLOUR(0x5F5F5E5E), // height 169
	MKCOLOUR(0x5F5F5E5E), // height 170
	MKCOLOUR(0x5F5F5E5F), // height 171
	MKCOLOUR(0x5F5F5E5F), // height 172
	MKCOLOUR(0x5F5F5F5F), // height 173
	MKCOLOUR(0x5F5F5F5F), // height 174
	MKCOLOUR(0x5F5F5F1F), // height 175
	MKCOLOUR(0x5F5F5F1F), // height 176
	MKCOLOUR(0x5F5F1F5F), // height 177
	MKCOLOUR(0x5F5F1F5F), // height 178
	MKCOLOUR(0x5F5F1F1F), // height 179
	MKCOLOUR(0x5F5F1F1F), // height 180
	MKCOLOUR(0x5F1F5F1F), // height 181
	MKCOLOUR(0x5F1F5F1F), // height 182
	MKCOLOUR(0x5F1F1F1F), // height 183
	MKCOLOUR(0x5F1F1F1F), // height 184
	MKCOLOUR(0x1F5F5F5F), // height 185
	MKCOLOUR(0x1F5F5F5F), // height 186
	MKCOLOUR(0x1F5F5F1F), // height 187
	MKCOLOUR(0x1F5F5F1F), // height 188
	MKCOLOUR(0x1F5F1F5F), // height 189
	MKCOLOUR(0x1F5F1F5F), // height 190
	MKCOLOUR(0x1F5F1F1F), // height 191
	MKCOLOUR(0x1F5F1F1F), // height 192
	MKCOLOUR(0x1F1F5F5F), // height 193
	MKCOLOUR(0x1F1F5F5F), // height 194
	MKCOLOUR(0x1F1F5F1F), // height 195
	MKCOLOUR(0x1F1F5F1F), // height 196
	MKCOLOUR(0x1F1F1F5F), // height 197
	MKCOLOUR(0x1F1F1F5F), // height 198
	MKCOLOUR(0x1F1F1F1F), // height 199
	MKCOLOUR(0x1F1F1F1F), // height 200
	MKCOLOUR(0x1F1F1F27), // height 201
	MKCOLOUR(0x1F1F1F27), // height 202
	MKCOLOUR(0x1F1F271F), // height 203
	MKCOLOUR(0x1F1F271F), // height 204
	MKCOLOUR(0x1F1F2727), // height 205
	MKCOLOUR(0x1F1F2727), // height 206
	MKCOLOUR(0x1F271F1F), // height 207
	MKCOLOUR(0x1F271F1F), // height 208
	MKCOLOUR(0x1F271F27), // height 209
	MKCOLOUR(0x1F271F27), // height 210
	MKCOLOUR(0x1F272727), // height 211
	MKCOLOUR(0x1F272727), // height 212
	MKCOLOUR(0x271F1F1F), // height 213
	MKCOLOUR(0x271F1F1F), // height 214
	MKCOLOUR(0x271F1F27), // height 215
	MKCOLOUR(0x271F1F27), // height 216
	MKCOLOUR(0x271F271F), // height 217
	MKCOLOUR(0x271F271F), // height 218
	MKCOLOUR(0x271F2727), // height 219
	MKCOLOUR(0x271F2727), // height 220
	MKCOLOUR(0x27271F1F), // height 221
	MKCOLOUR(0x27271F1F), // height 222
	MKCOLOUR(0x27271F27), // height 223
	MKCOLOUR(0x27271F27), // height 224
	MKCOLOUR(0x2727271F), // height 225
	MKCOLOUR(0x2727271F), // height 226
	MKCOLOUR(0x27272727), // height 227
	MKCOLOUR(0x27272727), // height 228
	MKCOLOUR(0x27272727), // height 229
	MKCOLOUR(0x27272727), // height 230
	MKCOLOUR(0x1F27AF27), // height 231
	MKCOLOUR(0x1F27AF27), // height 232
	MKCOLOUR(0x1F274FAF), // height 233
	MKCOLOUR(0x1F274FAF), // height 234
	MKCOLOUR(0x4F274FAF), // height 235
	MKCOLOUR(0x4F274FAF), // height 236
	MKCOLOUR(0x4FAF1FAF), // height 237
	MKCOLOUR(0x4FAF1FAF), // height 238
	MKCOLOUR(0x4F2727AF), // height 239
	MKCOLOUR(0x4F2727AF), // height 240
	MKCOLOUR(0x4F27AF27), // height 241
	MKCOLOUR(0x4F27AF27), // height 242
	MKCOLOUR(0x4F27AFAF), // height 243
	MKCOLOUR(0x4F27AFAF), // height 244
	MKCOLOUR(0x4FAF2727), // height 245
	MKCOLOUR(0x4FAF2727), // height 246
	MKCOLOUR(0x4FAF27AF), // height 247
	MKCOLOUR(0x4FAF27AF), // height 248
	MKCOLOUR(0x4FAFAF27), // height 249
	MKCOLOUR(0x4FAFAF27), // height 250
	MKCOLOUR(0x4FAFAFAF), // height 251
	MKCOLOUR(0x4FAFAFAF), // height 252
	MKCOLOUR(0x4FAFAFCF), // height 253
	MKCOLOUR(0x4FAFAFCF), // height 254
	MKCOLOUR(0x4FAFCFAF), // height 255
};
assert_compile(lengthof(_green_map_heights) == MAX_TILE_HEIGHT + 1);

/** Height map colours for the dark green colour scheme, ordered by height. */
static const uint32 _dark_green_map_heights[] = {
	MKCOLOUR(0x60606060), // height 0
	MKCOLOUR(0x60606060), // height 1
	MKCOLOUR(0x60606060),
	MKCOLOUR(0x60606061), // height 3
	MKCOLOUR(0x60606061),
	MKCOLOUR(0x60606160), // height 5
	MKCOLOUR(0x60606160),
	MKCOLOUR(0x60606161), // height 7
	MKCOLOUR(0x60606161),
	MKCOLOUR(0x60616060), // height 9
	MKCOLOUR(0x60616060),
	MKCOLOUR(0x60616061), // height 11
	MKCOLOUR(0x60616061),
	MKCOLOUR(0x60616160), // height 13
	MKCOLOUR(0x60616160),
	MKCOLOUR(0x60616161), // height 15
	MKCOLOUR(0x60616161),
	MKCOLOUR(0x61606060), // height 17
	MKCOLOUR(0x61606060),
	MKCOLOUR(0x61606061), // height 19
	MKCOLOUR(0x61606061),
	MKCOLOUR(0x61606160), // height 21
	MKCOLOUR(0x61606160),
	MKCOLOUR(0x61606161), // height 23
	MKCOLOUR(0x61606161),
	MKCOLOUR(0x61616060), // height 25
	MKCOLOUR(0x61616060),
	MKCOLOUR(0x61616061), // height 27
	MKCOLOUR(0x61616061),
	MKCOLOUR(0x61616160), // height 29
	MKCOLOUR(0x61616160),
	MKCOLOUR(0x61616161), // height 31
	MKCOLOUR(0x61616161),
	MKCOLOUR(0x61616162), // height 33
	MKCOLOUR(0x61616162),
	MKCOLOUR(0x61616261), // height 35
	MKCOLOUR(0x61616261),
	MKCOLOUR(0x61616262), // height 37
	MKCOLOUR(0x61616262),
	MKCOLOUR(0x61626161), // height 39
	MKCOLOUR(0x61626161),
	MKCOLOUR(0x61626162), // height 41
	MKCOLOUR(0x61626162),
	MKCOLOUR(0x61626261), // height 43
	MKCOLOUR(0x61626261),
	MKCOLOUR(0x61626262), // height 45
	MKCOLOUR(0x61626262),
	MKCOLOUR(0x62616161), // height 47
	MKCOLOUR(0x62616161),
	MKCOLOUR(0x62616162), // height 49
	MKCOLOUR(0x62616162),
	MKCOLOUR(0x62616261), // height 51
	MKCOLOUR(0x62616261),
	MKCOLOUR(0x62616262), // height 53
	MKCOLOUR(0x62616262),
	MKCOLOUR(0x62626161), // height 55
	MKCOLOUR(0x62626161),
	MKCOLOUR(0x62626162), // height 57
	MKCOLOUR(0x62626162),
	MKCOLOUR(0x62626261), // height 59
	MKCOLOUR(0x62626261),
	MKCOLOUR(0x62626262), // height 61
	MKCOLOUR(0x62626262),
	MKCOLOUR(0x62626263), // height 63
	MKCOLOUR(0x62626263),
	MKCOLOUR(0x62626362), // height 65
	MKCOLOUR(0x62626362),
	MKCOLOUR(0x62626363), // height 67
	MKCOLOUR(0x62626363),
	MKCOLOUR(0x62636262), // height 69
	MKCOLOUR(0x62636262),
	MKCOLOUR(0x62636263), // height 71
	MKCOLOUR(0x62636263),
	MKCOLOUR(0x62636362), // height 73
	MKCOLOUR(0x62636362),
	MKCOLOUR(0x62636363), // height 75
	MKCOLOUR(0x62636363),
	MKCOLOUR(0x63626262), // height 77
	MKCOLOUR(0x63626262),
	MKCOLOUR(0x63626263), // height 79
	MKCOLOUR(0x63626263),
	MKCOLOUR(0x63626362), // height 81
	MKCOLOUR(0x63626362),
	MKCOLOUR(0x63626363), // height 83
	MKCOLOUR(0x63626363),
	MKCOLOUR(0x63636262), // height 85
	MKCOLOUR(0x63636262),
	MKCOLOUR(0x63636263), // height 87
	MKCOLOUR(0x63636263),
	MKCOLOUR(0x63636362), // height 89
	MKCOLOUR(0x63636362),
	MKCOLOUR(0x63636363), // height 91
	MKCOLOUR(0x63636363),
	MKCOLOUR(0x63636364), // height 93
	MKCOLOUR(0x63636364),
	MKCOLOUR(0x63636463), // height 95
	MKCOLOUR(0x63636463),
	MKCOLOUR(0x63636464), // height 97
	MKCOLOUR(0x63636464),
	MKCOLOUR(0x63646363), // height 99
	MKCOLOUR(0x63646363),
	MKCOLOUR(0x63646364), // height 101
	MKCOLOUR(0x63646364),
	MKCOLOUR(0x63646463), // height 103
	MKCOLOUR(0x63646463),
	MKCOLOUR(0x63646464), // height 105
	MKCOLOUR(0x63646464),
	MKCOLOUR(0x64636363), // height 107
	MKCOLOUR(0x64636363),
	MKCOLOUR(0x64636364), // height 109
	MKCOLOUR(0x64636364),
	MKCOLOUR(0x64636463), // height 111
	MKCOLOUR(0x64636463),
	MKCOLOUR(0x64636464), // height 113
	MKCOLOUR(0x64636464),
	MKCOLOUR(0x64646363), // height 115
	MKCOLOUR(0x64646363),
	MKCOLOUR(0x64646364), // height 117
	MKCOLOUR(0x64646364),
	MKCOLOUR(0x64646463), // height 119
	MKCOLOUR(0x64646463),
	MKCOLOUR(0x64646464), // height 121
	MKCOLOUR(0x64646464),
	MKCOLOUR(0x64646465), // height 123
	MKCOLOUR(0x64646465),
	MKCOLOUR(0x64646564), // height 125
	MKCOLOUR(0x64646564),
	MKCOLOUR(0x64646565), // height 127
	MKCOLOUR(0x64646565),
	MKCOLOUR(0x64656464), // height 129
	MKCOLOUR(0x64656464),
	MKCOLOUR(0x64656465), // height 131
	MKCOLOUR(0x64656465),
	MKCOLOUR(0x64656564), // height 133
	MKCOLOUR(0x64656564),
	MKCOLOUR(0x64656565), // height 135
	MKCOLOUR(0x64656565),
	MKCOLOUR(0x65646464), // height 137
	MKCOLOUR(0x65646464),
	MKCOLOUR(0x65646465), // height 139
	MKCOLOUR(0x65646465),
	MKCOLOUR(0x65646564), // height 141
	MKCOLOUR(0x65646564),
	MKCOLOUR(0x65646565), // height 143
	MKCOLOUR(0x65646565),
	MKCOLOUR(0x65656464), // height 145
	MKCOLOUR(0x65656464),
	MKCOLOUR(0x65656465), // height 147
	MKCOLOUR(0x65656465),
	MKCOLOUR(0x65656564), // height 149
	MKCOLOUR(0x65656564),
	MKCOLOUR(0x65656565), // height 151
	MKCOLOUR(0x65656565),
	MKCOLOUR(0x65656566), // height 153
	MKCOLOUR(0x65656566),
	MKCOLOUR(0x65656665), // height 155
	MKCOLOUR(0x65656665),
	MKCOLOUR(0x65656666), // height 157
	MKCOLOUR(0x65656666),
	MKCOLOUR(0x65666565), // height 159
	MKCOLOUR(0x65666565),
	MKCOLOUR(0x65666566), // height 161
	MKCOLOUR(0x65666566),
	MKCOLOUR(0x65666665), // height 163
	MKCOLOUR(0x65666665),
	MKCOLOUR(0x65666666), // height 165
	MKCOLOUR(0x65666666),
	MKCOLOUR(0x66656565), // height 167
	MKCOLOUR(0x66656565),
	MKCOLOUR(0x66656566), // height 169
	MKCOLOUR(0x66656566),
	MKCOLOUR(0x66656665), // height 171
	MKCOLOUR(0x66656665),
	MKCOLOUR(0x66656666), // height 173
	MKCOLOUR(0x66656666),
	MKCOLOUR(0x66666565), // height 175
	MKCOLOUR(0x66666565),
	MKCOLOUR(0x66666566), // height 177
	MKCOLOUR(0x66666566),
	MKCOLOUR(0x66666665), // height 179
	MKCOLOUR(0x66666665),
	MKCOLOUR(0x66666666), // height 181
	MKCOLOUR(0x66666666),
	MKCOLOUR(0x66666667), // height 183
	MKCOLOUR(0x66666667),
	MKCOLOUR(0x66666766), // height 185
	MKCOLOUR(0x66666766),
	MKCOLOUR(0x66666767), // height 187
	MKCOLOUR(0x66666767),
	MKCOLOUR(0x66676666), // height 189
	MKCOLOUR(0x66676666),
	MKCOLOUR(0x66676667), // height 191
	MKCOLOUR(0x66676667),
	MKCOLOUR(0x66676766), // height 193
	MKCOLOUR(0x66676766),
	MKCOLOUR(0x66676767), // height 195
	MKCOLOUR(0x66676767),
	MKCOLOUR(0x67676767), // height 197
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 199
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 201
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 203
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 205
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 207
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 209
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 211
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 213
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 215
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 217
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 219
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 221
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 223
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 225
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 227
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x67676767), // height 229
	MKCOLOUR(0x67676767),
	MKCOLOUR(0x1F27AF27), // height 231
	MKCOLOUR(0x1F27AF27), // height 232
	MKCOLOUR(0x1F274FAF), // height 233
	MKCOLOUR(0x1F274FAF), // height 234
	MKCOLOUR(0x4F274FAF), // height 235
	MKCOLOUR(0x4F274FAF), // height 236
	MKCOLOUR(0x4FAF1FAF), // height 237
	MKCOLOUR(0x4FAF1FAF), // height 238
	MKCOLOUR(0x4F2727AF), // height 239
	MKCOLOUR(0x4F2727AF), // height 240
	MKCOLOUR(0x4F27AF27), // height 241
	MKCOLOUR(0x4F27AF27), // height 242
	MKCOLOUR(0x4F27AFAF), // height 243
	MKCOLOUR(0x4F27AFAF), // height 244
	MKCOLOUR(0x4FAF2727), // height 245
	MKCOLOUR(0x4FAF2727), // height 246
	MKCOLOUR(0x4FAF27AF), // height 247
	MKCOLOUR(0x4FAF27AF), // height 248
	MKCOLOUR(0x4FAFAF27), // height 249
	MKCOLOUR(0x4FAFAF27), // height 250
	MKCOLOUR(0x4FAFAFAF), // height 251
	MKCOLOUR(0x4FAFAFAF), // height 252
	MKCOLOUR(0x4FAFAFCF), // height 253
	MKCOLOUR(0x4FAFAFCF), // height 254
	MKCOLOUR(0x4FAFCFAF), // height 255
};
assert_compile(lengthof(_dark_green_map_heights) == MAX_TILE_HEIGHT + 1);

/** Height map colours for the violet colour scheme, ordered by height. */
static const uint32 _violet_map_heights[] = {
	MKCOLOUR(0x80808080), // height 0
	MKCOLOUR(0x80808080), // height 1
	MKCOLOUR(0x80808080),
	MKCOLOUR(0x80808081), // height 3
	MKCOLOUR(0x80808081),
	MKCOLOUR(0x80808180), // height 5
	MKCOLOUR(0x80808180),
	MKCOLOUR(0x80808181), // height 7
	MKCOLOUR(0x80808181),
	MKCOLOUR(0x80818080), // height 9
	MKCOLOUR(0x80818080),
	MKCOLOUR(0x80818081), // height 11
	MKCOLOUR(0x80818081),
	MKCOLOUR(0x80818180), // height 13
	MKCOLOUR(0x80818180),
	MKCOLOUR(0x80818181), // height 15
	MKCOLOUR(0x80818181),
	MKCOLOUR(0x81808080), // height 17
	MKCOLOUR(0x81808080),
	MKCOLOUR(0x81808081), // height 19
	MKCOLOUR(0x81808081),
	MKCOLOUR(0x81808180), // height 21
	MKCOLOUR(0x81808180),
	MKCOLOUR(0x81808181), // height 23
	MKCOLOUR(0x81808181),
	MKCOLOUR(0x81818080), // height 25
	MKCOLOUR(0x81818080),
	MKCOLOUR(0x81818081), // height 27
	MKCOLOUR(0x81818081),
	MKCOLOUR(0x81818180), // height 29
	MKCOLOUR(0x81818180),
	MKCOLOUR(0x81818181), // height 31
	MKCOLOUR(0x81818181),
	MKCOLOUR(0x81818182), // height 33
	MKCOLOUR(0x81818182),
	MKCOLOUR(0x81818281), // height 35
	MKCOLOUR(0x81818281),
	MKCOLOUR(0x81818282), // height 37
	MKCOLOUR(0x81818282),
	MKCOLOUR(0x81828181), // height 39
	MKCOLOUR(0x81828181),
	MKCOLOUR(0x81828182), // height 41
	MKCOLOUR(0x81828182),
	MKCOLOUR(0x81828281), // height 43
	MKCOLOUR(0x81828281),
	MKCOLOUR(0x81828282), // height 45
	MKCOLOUR(0x81828282),
	MKCOLOUR(0x82818181), // height 47
	MKCOLOUR(0x82818181),
	MKCOLOUR(0x82818182), // height 49
	MKCOLOUR(0x82818182),
	MKCOLOUR(0x82818281), // height 51
	MKCOLOUR(0x82818281),
	MKCOLOUR(0x82818282), // height 53
	MKCOLOUR(0x82818282),
	MKCOLOUR(0x82828181), // height 55
	MKCOLOUR(0x82828181),
	MKCOLOUR(0x82828182), // height 57
	MKCOLOUR(0x82828182),
	MKCOLOUR(0x82828281), // height 59
	MKCOLOUR(0x82828281),
	MKCOLOUR(0x82828282), // height 61
	MKCOLOUR(0x82828282),
	MKCOLOUR(0x82828283), // height 63
	MKCOLOUR(0x82828283),
	MKCOLOUR(0x82828382), // height 65
	MKCOLOUR(0x82828382),
	MKCOLOUR(0x82828383), // height 67
	MKCOLOUR(0x82828383),
	MKCOLOUR(0x82838282), // height 69
	MKCOLOUR(0x82838282),
	MKCOLOUR(0x82838283), // height 71
	MKCOLOUR(0x82838283),
	MKCOLOUR(0x82838382), // height 73
	MKCOLOUR(0x82838382),
	MKCOLOUR(0x82838383), // height 75
	MKCOLOUR(0x82838383),
	MKCOLOUR(0x83828282), // height 77
	MKCOLOUR(0x83828282),
	MKCOLOUR(0x83828283), // height 79
	MKCOLOUR(0x83828283),
	MKCOLOUR(0x83828382), // height 81
	MKCOLOUR(0x83828382),
	MKCOLOUR(0x83828383), // height 83
	MKCOLOUR(0x83828383),
	MKCOLOUR(0x83838282), // height 85
	MKCOLOUR(0x83838282),
	MKCOLOUR(0x83838283), // height 87
	MKCOLOUR(0x83838283),
	MKCOLOUR(0x83838382), // height 89
	MKCOLOUR(0x83838382),
	MKCOLOUR(0x83838383), // height 91
	MKCOLOUR(0x83838383),
	MKCOLOUR(0x83838384), // height 93
	MKCOLOUR(0x83838384),
	MKCOLOUR(0x83838483), // height 95
	MKCOLOUR(0x83838483),
	MKCOLOUR(0x83838484), // height 97
	MKCOLOUR(0x83838484),
	MKCOLOUR(0x83848383), // height 99
	MKCOLOUR(0x83848383),
	MKCOLOUR(0x83848384), // height 101
	MKCOLOUR(0x83848384),
	MKCOLOUR(0x83848483), // height 103
	MKCOLOUR(0x83848483),
	MKCOLOUR(0x83848484), // height 105
	MKCOLOUR(0x83848484),
	MKCOLOUR(0x84838383), // height 107
	MKCOLOUR(0x84838383),
	MKCOLOUR(0x84838384), // height 109
	MKCOLOUR(0x84838384),
	MKCOLOUR(0x84838483), // height 111
	MKCOLOUR(0x84838483),
	MKCOLOUR(0x84838484), // height 113
	MKCOLOUR(0x84838484),
	MKCOLOUR(0x84848383), // height 115
	MKCOLOUR(0x84848383),
	MKCOLOUR(0x84848384), // height 117
	MKCOLOUR(0x84848384),
	MKCOLOUR(0x84848483), // height 119
	MKCOLOUR(0x84848483),
	MKCOLOUR(0x84848484), // height 121
	MKCOLOUR(0x84848484),
	MKCOLOUR(0x84848485), // height 123
	MKCOLOUR(0x84848485),
	MKCOLOUR(0x84848584), // height 125
	MKCOLOUR(0x84848584),
	MKCOLOUR(0x84848585), // height 127
	MKCOLOUR(0x84848585),
	MKCOLOUR(0x84858484), // height 129
	MKCOLOUR(0x84858484),
	MKCOLOUR(0x84858485), // height 131
	MKCOLOUR(0x84858485),
	MKCOLOUR(0x84858584), // height 133
	MKCOLOUR(0x84858584),
	MKCOLOUR(0x84858585), // height 135
	MKCOLOUR(0x84858585),
	MKCOLOUR(0x85848484), // height 137
	MKCOLOUR(0x85848484),
	MKCOLOUR(0x85848485), // height 139
	MKCOLOUR(0x85848485),
	MKCOLOUR(0x85848584), // height 141
	MKCOLOUR(0x85848584),
	MKCOLOUR(0x85848585), // height 143
	MKCOLOUR(0x85848585),
	MKCOLOUR(0x85858484), // height 145
	MKCOLOUR(0x85858484),
	MKCOLOUR(0x85858485), // height 147
	MKCOLOUR(0x85858485),
	MKCOLOUR(0x85858584), // height 149
	MKCOLOUR(0x85858584),
	MKCOLOUR(0x85858585), // height 151
	MKCOLOUR(0x85858585),
	MKCOLOUR(0x85858586), // height 153
	MKCOLOUR(0x85858586),
	MKCOLOUR(0x85858685), // height 155
	MKCOLOUR(0x85858685),
	MKCOLOUR(0x85858686), // height 157
	MKCOLOUR(0x85858686),
	MKCOLOUR(0x85868585), // height 159
	MKCOLOUR(0x85868585),
	MKCOLOUR(0x85868586), // height 161
	MKCOLOUR(0x85868586),
	MKCOLOUR(0x85868685), // height 163
	MKCOLOUR(0x85868685),
	MKCOLOUR(0x85868686), // height 165
	MKCOLOUR(0x85868686),
	MKCOLOUR(0x85868585), // height 167
	MKCOLOUR(0x85868585),
	MKCOLOUR(0x85868586), // height 169
	MKCOLOUR(0x85868586),
	MKCOLOUR(0x85868685), // height 171
	MKCOLOUR(0x85868685),
	MKCOLOUR(0x85868686), // height 173
	MKCOLOUR(0x85868686),
	MKCOLOUR(0x86868585), // height 175
	MKCOLOUR(0x86868585),
	MKCOLOUR(0x86868586), // height 177
	MKCOLOUR(0x86868586),
	MKCOLOUR(0x86868685), // height 179
	MKCOLOUR(0x86868685),
	MKCOLOUR(0x86868686), // height 181
	MKCOLOUR(0x86868686),
	MKCOLOUR(0x86868687), // height 183
	MKCOLOUR(0x86868687),
	MKCOLOUR(0x86868786), // height 185
	MKCOLOUR(0x86868786),
	MKCOLOUR(0x86868787), // height 187
	MKCOLOUR(0x86868787),
	MKCOLOUR(0x86878686), // height 189
	MKCOLOUR(0x86878686),
	MKCOLOUR(0x86878687), // height 191
	MKCOLOUR(0x86878687),
	MKCOLOUR(0x86878786), // height 193
	MKCOLOUR(0x86878786),
	MKCOLOUR(0x86878787), // height 195
	MKCOLOUR(0x86878787),
	MKCOLOUR(0x87868686), // height 197
	MKCOLOUR(0x87868686),
	MKCOLOUR(0x87868687), // height 199
	MKCOLOUR(0x87868687),
	MKCOLOUR(0x87868786), // height 201
	MKCOLOUR(0x87868786),
	MKCOLOUR(0x87868787), // height 203
	MKCOLOUR(0x87868787),
	MKCOLOUR(0x87878686), // height 205
	MKCOLOUR(0x87878686),
	MKCOLOUR(0x87878687), // height 207
	MKCOLOUR(0x87878687),
	MKCOLOUR(0x87878786), // height 209
	MKCOLOUR(0x87878786),
	MKCOLOUR(0x87878787), // height 211
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 213
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 215
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 217
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 219
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 221
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 223
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 225
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 227
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x87878787), // height 229
	MKCOLOUR(0x87878787),
	MKCOLOUR(0x1F27AF27), // height 231
	MKCOLOUR(0x1F27AF27), // height 232
	MKCOLOUR(0x1F274FAF), // height 233
	MKCOLOUR(0x1F274FAF), // height 234
	MKCOLOUR(0x4F274FAF), // height 235
	MKCOLOUR(0x4F274FAF), // height 236
	MKCOLOUR(0x4FAF1FAF), // height 237
	MKCOLOUR(0x4FAF1FAF), // height 238
	MKCOLOUR(0x4F2727AF), // height 239
	MKCOLOUR(0x4F2727AF), // height 240
	MKCOLOUR(0x4F27AF27), // height 241
	MKCOLOUR(0x4F27AF27), // height 242
	MKCOLOUR(0x4F27AFAF), // height 243
	MKCOLOUR(0x4F27AFAF), // height 244
	MKCOLOUR(0x4FAF2727), // height 245
	MKCOLOUR(0x4FAF2727), // height 246
	MKCOLOUR(0x4FAF27AF), // height 247
	MKCOLOUR(0x4FAF27AF), // height 248
	MKCOLOUR(0x4FAFAF27), // height 249
	MKCOLOUR(0x4FAFAF27), // height 250
	MKCOLOUR(0x4FAFAFAF), // height 251
	MKCOLOUR(0x4FAFAFAF), // height 252
	MKCOLOUR(0x4FAFAFCF), // height 253
	MKCOLOUR(0x4FAFAFCF), // height 254
	MKCOLOUR(0x4FAFCFAF), // height 255
};
assert_compile(lengthof(_violet_map_heights) == MAX_TILE_HEIGHT + 1);

/** Colour scheme of the smallmap. */
struct SmallMapColourScheme {
	const uint32 *height_colours; ///< Colour of each level in a heightmap.
	uint32 default_colour;   ///< Default colour of the land.
};

/** Available colour schemes for height maps. */
static const SmallMapColourScheme _heightmap_schemes[] = {
	{_green_map_heights,      MKCOLOUR_XXXX(0x54)}, ///< Green colour scheme.
	{_dark_green_map_heights, MKCOLOUR_XXXX(0x62)}, ///< Dark green colour scheme.
	{_violet_map_heights,     MKCOLOUR_XXXX(0x82)}, ///< Violet colour scheme.
};

/**
 * (Re)build the colour tables for the legends.
 */
void BuildLandLegend()
{
	for (LegendAndColour *lc = _legend_land_contours; lc->legend == STR_TINY_BLACK_HEIGHT; lc++) {
		lc->colour = _heightmap_schemes[_settings_client.gui.smallmap_land_colour].height_colours[lc->height];
	}
}

/**
 * Completes the array for the owned property legend.
 */
void BuildOwnerLegend()
{
	_legend_land_owners[1].colour = _heightmap_schemes[_settings_client.gui.smallmap_land_colour].default_colour;

	int i = NUM_NO_COMPANY_ENTRIES;
	const Company *c;
	FOR_ALL_COMPANIES(c) {
		_legend_land_owners[i].colour = _colour_gradient[c->colour][5];
		_legend_land_owners[i].company = c->index;
		_legend_land_owners[i].show_on_map = true;
		_legend_land_owners[i].col_break = false;
		_legend_land_owners[i].end = false;
		_company_to_list_pos[c->index] = i;
		i++;
	}

	/* Terminate the list */
	_legend_land_owners[i].end = true;

	/* Store maximum amount of owner legend entries. */
	_smallmap_company_count = i;
}

struct AndOr {
	uint32 mor;
	uint32 mand;
};

static inline uint32 ApplyMask(uint32 colour, const AndOr *mask)
{
	return (colour & mask->mand) | mask->mor;
}


/** Colour masks for "Contour" and "Routes" modes. */
static const AndOr _smallmap_contours_andor[] = {
	{MKCOLOUR_0000               , MKCOLOUR_FFFF}, // MP_CLEAR
	{MKCOLOUR_0XX0(PC_GREY      ), MKCOLOUR_F00F}, // MP_RAILWAY
	{MKCOLOUR_0XX0(PC_BLACK     ), MKCOLOUR_F00F}, // MP_ROAD
	{MKCOLOUR_0XX0(PC_DARK_RED  ), MKCOLOUR_F00F}, // MP_HOUSE
	{MKCOLOUR_0000               , MKCOLOUR_FFFF}, // MP_TREES
	{MKCOLOUR_XXXX(PC_LIGHT_BLUE), MKCOLOUR_0000}, // MP_STATION
	{MKCOLOUR_XXXX(PC_WATER     ), MKCOLOUR_0000}, // MP_WATER
	{MKCOLOUR_0000               , MKCOLOUR_FFFF}, // MP_VOID
	{MKCOLOUR_XXXX(PC_DARK_RED  ), MKCOLOUR_0000}, // MP_INDUSTRY
	{MKCOLOUR_0000               , MKCOLOUR_FFFF}, // MP_TUNNELBRIDGE
	{MKCOLOUR_0XX0(PC_DARK_RED  ), MKCOLOUR_F00F}, // MP_OBJECT
	{MKCOLOUR_0XX0(PC_GREY      ), MKCOLOUR_F00F},
};

/** Colour masks for "Vehicles", "Industry", and "Vegetation" modes. */
static const AndOr _smallmap_vehicles_andor[] = {
	{MKCOLOUR_0000               , MKCOLOUR_FFFF}, // MP_CLEAR
	{MKCOLOUR_0XX0(PC_BLACK     ), MKCOLOUR_F00F}, // MP_RAILWAY
	{MKCOLOUR_0XX0(PC_BLACK     ), MKCOLOUR_F00F}, // MP_ROAD
	{MKCOLOUR_0XX0(PC_DARK_RED  ), MKCOLOUR_F00F}, // MP_HOUSE
	{MKCOLOUR_0000               , MKCOLOUR_FFFF}, // MP_TREES
	{MKCOLOUR_0XX0(PC_BLACK     ), MKCOLOUR_F00F}, // MP_STATION
	{MKCOLOUR_XXXX(PC_WATER     ), MKCOLOUR_0000}, // MP_WATER
	{MKCOLOUR_0000               , MKCOLOUR_FFFF}, // MP_VOID
	{MKCOLOUR_XXXX(PC_DARK_RED  ), MKCOLOUR_0000}, // MP_INDUSTRY
	{MKCOLOUR_0000               , MKCOLOUR_FFFF}, // MP_TUNNELBRIDGE
	{MKCOLOUR_0XX0(PC_DARK_RED  ), MKCOLOUR_F00F}, // MP_OBJECT
	{MKCOLOUR_0XX0(PC_BLACK     ), MKCOLOUR_F00F},
};

/** Mapping of tile type to importance of the tile (higher number means more interesting to show). */
static const byte _tiletype_importance[] = {
	2, // MP_CLEAR
	8, // MP_RAILWAY
	7, // MP_ROAD
	5, // MP_HOUSE
	2, // MP_TREES
	9, // MP_STATION
	2, // MP_WATER
	1, // MP_VOID
	6, // MP_INDUSTRY
	8, // MP_TUNNELBRIDGE
	2, // MP_OBJECT
	0,
};


static inline TileType GetEffectiveTileType(TileIndex tile)
{
	TileType t = GetTileType(tile);

	if (t == MP_TUNNELBRIDGE) {
		TransportType tt = GetTunnelBridgeTransportType(tile);

		switch (tt) {
			case TRANSPORT_RAIL: t = MP_RAILWAY; break;
			case TRANSPORT_ROAD: t = MP_ROAD;    break;
			default:             t = MP_WATER;   break;
		}
	}
	return t;
}

/**
 * Return the colour a tile would be displayed with in the small map in mode "Contour".
 * @param tile The tile of which we would like to get the colour.
 * @param t    Effective tile type of the tile (see #GetEffectiveTileType).
 * @return The colour of tile in the small map in mode "Contour"
 */
static inline uint32 GetSmallMapContoursPixels(TileIndex tile, TileType t)
{
	const SmallMapColourScheme *cs = &_heightmap_schemes[_settings_client.gui.smallmap_land_colour];
	return ApplyMask(cs->height_colours[TileHeight(tile)], &_smallmap_contours_andor[t]);
}

/**
 * Return the colour a tile would be displayed with in the small map in mode "Vehicles".
 *
 * @param tile The tile of which we would like to get the colour.
 * @param t    Effective tile type of the tile (see #GetEffectiveTileType).
 * @return The colour of tile in the small map in mode "Vehicles"
 */
static inline uint32 GetSmallMapVehiclesPixels(TileIndex tile, TileType t)
{
	const SmallMapColourScheme *cs = &_heightmap_schemes[_settings_client.gui.smallmap_land_colour];
	return ApplyMask(cs->default_colour, &_smallmap_vehicles_andor[t]);
}

/**
 * Return the colour a tile would be displayed with in the small map in mode "Industries".
 *
 * @param tile The tile of which we would like to get the colour.
 * @param t    Effective tile type of the tile (see #GetEffectiveTileType).
 * @return The colour of tile in the small map in mode "Industries"
 */
static inline uint32 GetSmallMapIndustriesPixels(TileIndex tile, TileType t)
{
	if (t == MP_INDUSTRY) {
		/* If industry is allowed to be seen, use its colour on the map */
		IndustryType type = Industry::GetByTile(tile)->type;
		if (_legend_from_industries[_industry_to_list_pos[type]].show_on_map &&
				(_smallmap_industry_highlight_state || type != _smallmap_industry_highlight)) {
			return (type == _smallmap_industry_highlight ? PC_WHITE : GetIndustrySpec(Industry::GetByTile(tile)->type)->map_colour) * 0x01010101;
		} else {
			/* Otherwise, return the colour which will make it disappear */
			t = (IsTileOnWater(tile) ? MP_WATER : MP_CLEAR);
		}
	}

	const SmallMapColourScheme *cs = &_heightmap_schemes[_settings_client.gui.smallmap_land_colour];
	return ApplyMask(_smallmap_show_heightmap ? cs->height_colours[TileHeight(tile)] : cs->default_colour, &_smallmap_vehicles_andor[t]);
}

/**
 * Return the colour a tile would be displayed with in the small map in mode "Routes".
 *
 * @param tile The tile of which we would like to get the colour.
 * @param t    Effective tile type of the tile (see #GetEffectiveTileType).
 * @param show_height Whether to show the height of plain tiles.
 * @return The colour of tile  in the small map in mode "Routes"
 */
static inline uint32 GetSmallMapRoutesPixels(TileIndex tile, TileType t, bool show_height = false)
{
	if (t == MP_STATION) {
		switch (GetStationType(tile)) {
			case STATION_RAIL:    return MKCOLOUR_XXXX(PC_VERY_DARK_BROWN);
			case STATION_AIRPORT: return MKCOLOUR_XXXX(PC_RED);
			case STATION_TRUCK:   return MKCOLOUR_XXXX(PC_ORANGE);
			case STATION_BUS:     return MKCOLOUR_XXXX(PC_YELLOW);
			case STATION_DOCK:    return MKCOLOUR_XXXX(PC_LIGHT_BLUE);
			default:              return MKCOLOUR_FFFF;
		}
	} else if (t == MP_RAILWAY) {
		AndOr andor = {
			MKCOLOUR_0XX0(GetRailTypeInfo(GetRailType(tile))->map_colour),
			_smallmap_contours_andor[t].mand
		};

		const SmallMapColourScheme *cs = &_heightmap_schemes[_settings_client.gui.smallmap_land_colour];
		return ApplyMask(cs->default_colour, &andor);
	}

	/* Ground colour */
	const SmallMapColourScheme *cs = &_heightmap_schemes[_settings_client.gui.smallmap_land_colour];
	return ApplyMask(show_height ? cs->height_colours[TileHeight(tile)] : cs->default_colour, &_smallmap_contours_andor[t]);
}


static const uint32 _vegetation_clear_bits[] = {
	MKCOLOUR_XXXX(PC_GRASS_LAND), ///< full grass
	MKCOLOUR_XXXX(PC_ROUGH_LAND), ///< rough land
	MKCOLOUR_XXXX(PC_GREY),       ///< rocks
	MKCOLOUR_XXXX(PC_FIELDS),     ///< fields
	MKCOLOUR_XXXX(PC_LIGHT_BLUE), ///< snow
	MKCOLOUR_XXXX(PC_ORANGE),     ///< desert
	MKCOLOUR_XXXX(PC_GRASS_LAND), ///< unused
	MKCOLOUR_XXXX(PC_GRASS_LAND), ///< unused
};

/**
 * Return the colour a tile would be displayed with in the smallmap in mode "Vegetation".
 *
 * @param tile The tile of which we would like to get the colour.
 * @param t    Effective tile type of the tile (see #GetEffectiveTileType).
 * @return The colour of tile  in the smallmap in mode "Vegetation"
 */
static inline uint32 GetSmallMapVegetationPixels(TileIndex tile, TileType t)
{
	switch (t) {
		case MP_CLEAR:
			return (IsClearGround(tile, CLEAR_GRASS) && GetClearDensity(tile) < 3) ? MKCOLOUR_XXXX(PC_BARE_LAND) : _vegetation_clear_bits[GetClearGround(tile)];

		case MP_INDUSTRY:
			return IsTileForestIndustry(tile) ? MKCOLOUR_XXXX(PC_GREEN) : MKCOLOUR_XXXX(PC_DARK_RED);

		case MP_TREES:
			if (GetTreeGround(tile) == TREE_GROUND_SNOW_DESERT || GetTreeGround(tile) == TREE_GROUND_ROUGH_SNOW) {
				return (_settings_game.game_creation.landscape == LT_ARCTIC) ? MKCOLOUR_XYYX(PC_LIGHT_BLUE, PC_TREES) : MKCOLOUR_XYYX(PC_ORANGE, PC_TREES);
			}
			return MKCOLOUR_XYYX(PC_GRASS_LAND, PC_TREES);

		default:
			return ApplyMask(MKCOLOUR_XXXX(PC_GRASS_LAND), &_smallmap_vehicles_andor[t]);
	}
}

/**
 * Return the colour a tile would be displayed with in the small map in mode "Owner".
 *
 * @param tile The tile of which we would like to get the colour.
 * @param t    Effective tile type of the tile (see #GetEffectiveTileType).
 * @return The colour of tile in the small map in mode "Owner"
 */
static inline uint32 GetSmallMapOwnerPixels(TileIndex tile, TileType t)
{
	Owner o;

	switch (t) {
		case MP_INDUSTRY: return MKCOLOUR_XXXX(PC_DARK_GREY);
		case MP_HOUSE:    return MKCOLOUR_XXXX(PC_DARK_RED);
		default:          o = GetTileOwner(tile); break;
		/* FIXME: For MP_ROAD there are multiple owners.
		 * GetTileOwner returns the rail owner (level crossing) resp. the owner of ROADTYPE_ROAD (normal road),
		 * even if there are no ROADTYPE_ROAD bits on the tile.
		 */
	}

	if ((o < MAX_COMPANIES && !_legend_land_owners[_company_to_list_pos[o]].show_on_map) || o == OWNER_NONE || o == OWNER_WATER) {
		if (t == MP_WATER) return MKCOLOUR_XXXX(PC_WATER);
		const SmallMapColourScheme *cs = &_heightmap_schemes[_settings_client.gui.smallmap_land_colour];
		return _smallmap_show_heightmap ? cs->height_colours[TileHeight(tile)] : cs->default_colour;
	} else if (o == OWNER_TOWN) {
		return MKCOLOUR_XXXX(PC_DARK_RED);
	}

	return MKCOLOUR_XXXX(_legend_land_owners[_company_to_list_pos[o]].colour);
}

/** Vehicle colours in #SMT_VEHICLES mode. Indexed by #VehicleTypeByte. */
static const byte _vehicle_type_colours[6] = {
	PC_RED, PC_YELLOW, PC_LIGHT_BLUE, PC_WHITE, PC_BLACK, PC_RED
};


/** Class managing the smallmap window. */
class SmallMapWindow : public Window {
	/** Types of legends in the #WID_SM_LEGEND widget. */
	enum SmallMapType {
		SMT_CONTOUR,
		SMT_VEHICLES,
		SMT_INDUSTRY,
		SMT_ROUTE_LINKS,
		SMT_ROUTES,
		SMT_VEGETATION,
		SMT_OWNER,
	};

	/** Available kinds of zoomlevel changes. */
	enum ZoomLevelChange {
		ZLC_INITIALIZE, ///< Initialize zoom level.
		ZLC_ZOOM_OUT,   ///< Zoom out.
		ZLC_ZOOM_IN,    ///< Zoom in.
	};

	static SmallMapType map_type; ///< Currently displayed legends.
	static bool show_towns;       ///< Display town names in the smallmap.

	static const uint LEGEND_BLOB_WIDTH = 8;              ///< Width of the coloured blob in front of a line text in the #WID_SM_LEGEND widget.
	static const uint INDUSTRY_MIN_NUMBER_OF_COLUMNS = 2; ///< Minimal number of columns in the #WID_SM_LEGEND widget for the #SMT_INDUSTRY legend.
	uint min_number_of_fixed_rows; ///< Minimal number of rows in the legends for the fixed layouts only (all except #SMT_INDUSTRY).
	uint column_width;             ///< Width of a column in the #WID_SM_LEGEND widget.

	int32 scroll_x;  ///< Horizontal world coordinate of the base tile left of the top-left corner of the smallmap display.
	int32 scroll_y;  ///< Vertical world coordinate of the base tile left of the top-left corner of the smallmap display.
	int32 subscroll; ///< Number of pixels (0..3) between the right end of the base tile and the pixel at the top-left corner of the smallmap display.
	int zoom;        ///< Zoom level. Bigger number means more zoom-out (further away).

	static const uint FORCE_REFRESH_PERIOD = 0x1F; ///< map is redrawn after that many ticks
	static const uint BLINK_PERIOD         = 0x0F; ///< highlight blinking interval
	uint8 refresh; ///< refresh counter, zeroed every FORCE_REFRESH_PERIOD ticks

	inline Point SmallmapRemapCoords(int x, int y) const
	{
		Point pt;
		pt.x = (y - x) * 2;
		pt.y = y + x;
		return pt;
	}

	/**
	 * Remap tile to location on this smallmap.
	 * @param tile_x X coordinate of the tile.
	 * @param tile_y Y coordinate of the tile.
	 * @return Position to draw on.
	 */
	inline Point RemapTile(int tile_x, int tile_y) const
	{
		int x_offset = tile_x - this->scroll_x / (int)TILE_SIZE;
		int y_offset = tile_y - this->scroll_y / (int)TILE_SIZE;

		if (this->zoom == 1) return SmallmapRemapCoords(x_offset, y_offset);

		/* For negative offsets, round towards -inf. */
		if (x_offset < 0) x_offset -= this->zoom - 1;
		if (y_offset < 0) y_offset -= this->zoom - 1;

		return SmallmapRemapCoords(x_offset / this->zoom, y_offset / this->zoom);
	}

	/**
	 * Determine the tile relative to the base tile of the smallmap, and the pixel position at
	 * that tile for a point in the smallmap.
	 * @param px       Horizontal coordinate of the pixel.
	 * @param py       Vertical coordinate of the pixel.
	 * @param sub[out] Pixel position at the tile (0..3).
	 * @param add_sub  Add current #subscroll to the position.
	 * @return Tile being displayed at the given position relative to #scroll_x and #scroll_y.
	 * @note The #subscroll offset is already accounted for.
	 */
	inline Point PixelToTile(int px, int py, int *sub, bool add_sub = true) const
	{
		if (add_sub) px += this->subscroll;  // Total horizontal offset.

		/* For each two rows down, add a x and a y tile, and
		 * For each four pixels to the right, move a tile to the right. */
		Point pt = {((py >> 1) - (px >> 2)) * this->zoom, ((py >> 1) + (px >> 2)) * this->zoom};
		px &= 3;

		if (py & 1) { // Odd number of rows, handle the 2 pixel shift.
			if (px < 2) {
				pt.x += this->zoom;
				px += 2;
			} else {
				pt.y += this->zoom;
				px -= 2;
			}
		}

		*sub = px;
		return pt;
	}

	/**
	 * Compute base parameters of the smallmap such that tile (\a tx, \a ty) starts at pixel (\a x, \a y).
	 * @param tx        Tile x coordinate.
	 * @param ty        Tile y coordinate.
	 * @param x         Non-negative horizontal position in the display where the tile starts.
	 * @param y         Non-negative vertical position in the display where the tile starts.
	 * @param sub [out] Value of #subscroll needed.
	 * @return #scroll_x, #scroll_y values.
	 */
	Point ComputeScroll(int tx, int ty, int x, int y, int *sub)
	{
		assert(x >= 0 && y >= 0);

		int new_sub;
		Point tile_xy = PixelToTile(x, y, &new_sub, false);
		tx -= tile_xy.x;
		ty -= tile_xy.y;

		Point scroll;
		if (new_sub == 0) {
			*sub = 0;
			scroll.x = (tx + this->zoom) * TILE_SIZE;
			scroll.y = (ty - this->zoom) * TILE_SIZE;
		} else {
			*sub = 4 - new_sub;
			scroll.x = (tx + 2 * this->zoom) * TILE_SIZE;
			scroll.y = (ty - 2 * this->zoom) * TILE_SIZE;
		}
		return scroll;
	}

	/**
	 * Initialize or change the zoom level.
	 * @param change  Way to change the zoom level.
	 * @param zoom_pt Position to keep fixed while zooming.
	 * @pre \c *zoom_pt should contain a point in the smallmap display when zooming in or out.
	 */
	void SetZoomLevel(ZoomLevelChange change, const Point *zoom_pt)
	{
		static const int zoomlevels[] = {1, 2, 4, 6, 8}; // Available zoom levels. Bigger number means more zoom-out (further away).
		static const int MIN_ZOOM_INDEX = 0;
		static const int MAX_ZOOM_INDEX = lengthof(zoomlevels) - 1;

		int new_index, cur_index, sub;
		Point tile;
		switch (change) {
			case ZLC_INITIALIZE:
				cur_index = - 1; // Definitely different from new_index.
				new_index = MIN_ZOOM_INDEX;
				break;

			case ZLC_ZOOM_IN:
			case ZLC_ZOOM_OUT:
				for (cur_index = MIN_ZOOM_INDEX; cur_index <= MAX_ZOOM_INDEX; cur_index++) {
					if (this->zoom == zoomlevels[cur_index]) break;
				}
				assert(cur_index <= MAX_ZOOM_INDEX);

				tile = this->PixelToTile(zoom_pt->x, zoom_pt->y, &sub);
				new_index = Clamp(cur_index + ((change == ZLC_ZOOM_IN) ? -1 : 1), MIN_ZOOM_INDEX, MAX_ZOOM_INDEX);
				break;

			default: NOT_REACHED();
		}

		if (new_index != cur_index) {
			this->zoom = zoomlevels[new_index];
			if (cur_index >= 0) {
				Point new_tile = this->PixelToTile(zoom_pt->x, zoom_pt->y, &sub);
				this->SetNewScroll(this->scroll_x + (tile.x - new_tile.x) * TILE_SIZE,
						this->scroll_y + (tile.y - new_tile.y) * TILE_SIZE, sub);
			}
			this->SetWidgetDisabledState(WID_SM_ZOOM_IN,  this->zoom == zoomlevels[MIN_ZOOM_INDEX]);
			this->SetWidgetDisabledState(WID_SM_ZOOM_OUT, this->zoom == zoomlevels[MAX_ZOOM_INDEX]);
			this->SetDirty();
		}
	}

	/**
	 * Decide which colours to show to the user for a group of tiles.
	 * @param ta Tile area to investigate.
	 * @return Colours to display.
	 */
	inline uint32 GetTileColours(const TileArea &ta) const
	{
		int importance = 0;
		TileIndex tile = INVALID_TILE; // Position of the most important tile.
		TileType et = MP_VOID;         // Effective tile type at that position.

		TILE_AREA_LOOP(ti, ta) {
			TileType ttype = GetEffectiveTileType(ti);
			if (_tiletype_importance[ttype] > importance) {
				importance = _tiletype_importance[ttype];
				tile = ti;
				et = ttype;
			}
		}

		switch (this->map_type) {
			case SMT_CONTOUR:
				return GetSmallMapContoursPixels(tile, et);

			case SMT_VEHICLES:
				return GetSmallMapVehiclesPixels(tile, et);

			case SMT_INDUSTRY:
				return GetSmallMapIndustriesPixels(tile, et);

			case SMT_ROUTE_LINKS:
				return GetSmallMapRoutesPixels(tile, et, _smallmap_show_heightmap);

			case SMT_ROUTES:
				return GetSmallMapRoutesPixels(tile, et);

			case SMT_VEGETATION:
				return GetSmallMapVegetationPixels(tile, et);

			case SMT_OWNER:
				return GetSmallMapOwnerPixels(tile, et);

			default: NOT_REACHED();
		}
	}

	/**
	 * Draws one column of tiles of the small map in a certain mode onto the screen buffer, skipping the shifted rows in between.
	 *
	 * @param dst Pointer to a part of the screen buffer to write to.
	 * @param xc The X coordinate of the first tile in the column.
	 * @param yc The Y coordinate of the first tile in the column
	 * @param pitch Number of pixels to advance in the screen buffer each time a pixel is written.
	 * @param reps Number of lines to draw
	 * @param start_pos Position of first pixel to draw.
	 * @param end_pos Position of last pixel to draw (exclusive).
	 * @param blitter current blitter
	 * @note If pixel position is below \c 0, skip drawing.
	 */
	void DrawSmallMapColumn(void *dst, uint xc, uint yc, int pitch, int reps, int start_pos, int end_pos, Blitter *blitter) const
	{
		void *dst_ptr_abs_end = blitter->MoveTo(_screen.dst_ptr, 0, _screen.height);
		uint min_xy = _settings_game.construction.freeform_edges ? 1 : 0;

		do {
			/* Check if the tile (xc,yc) is within the map range */
			if (xc >= MapMaxX() || yc >= MapMaxY()) continue;

			/* Check if the dst pointer points to a pixel inside the screen buffer */
			if (dst < _screen.dst_ptr) continue;
			if (dst >= dst_ptr_abs_end) continue;

			/* Construct tilearea covered by (xc, yc, xc + this->zoom, yc + this->zoom) such that it is within min_xy limits. */
			TileArea ta;
			if (min_xy == 1 && (xc == 0 || yc == 0)) {
				if (this->zoom == 1) continue; // The tile area is empty, don't draw anything.

				ta = TileArea(TileXY(max(min_xy, xc), max(min_xy, yc)), this->zoom - (xc == 0), this->zoom - (yc == 0));
			} else {
				ta = TileArea(TileXY(xc, yc), this->zoom, this->zoom);
			}
			ta.ClampToMap(); // Clamp to map boundaries (may contain MP_VOID tiles!).

			uint32 val = this->GetTileColours(ta);
			uint8 *val8 = (uint8 *)&val;
			int idx = max(0, -start_pos);
			for (int pos = max(0, start_pos); pos < end_pos; pos++) {
				blitter->SetPixel(dst, idx, 0, val8[idx]);
				idx++;
			}
		/* Switch to next tile in the column */
		} while (xc += this->zoom, yc += this->zoom, dst = blitter->MoveTo(dst, pitch, 0), --reps != 0);
	}

	/**
	 * Adds vehicles to the smallmap.
	 * @param dpi the part of the smallmap to be drawn into
	 * @param blitter current blitter
	 */
	void DrawVehicles(const DrawPixelInfo *dpi, Blitter *blitter) const
	{
		const Vehicle *v;
		FOR_ALL_VEHICLES(v) {
			if (v->type == VEH_EFFECT) continue;
			if (v->vehstatus & (VS_HIDDEN | VS_UNCLICKABLE)) continue;

			/* Remap into flat coordinates. */
			Point pt = this->RemapTile(v->x_pos / TILE_SIZE, v->y_pos / TILE_SIZE);

			int y = pt.y - dpi->top;
			if (!IsInsideMM(y, 0, dpi->height)) continue; // y is out of bounds.

			bool skip = false; // Default is to draw both pixels.
			int x = pt.x - this->subscroll - 3 - dpi->left; // Offset X coordinate.
			if (x < 0) {
				/* if x+1 is 0, that means we're on the very left edge,
				 * and should thus only draw a single pixel */
				if (++x != 0) continue;
				skip = true;
			} else if (x >= dpi->width - 1) {
				/* Check if we're at the very right edge, and if so draw only a single pixel */
				if (x != dpi->width - 1) continue;
				skip = true;
			}

			/* Calculate pointer to pixel and the colour */
			byte colour = (this->map_type == SMT_VEHICLES) ? _vehicle_type_colours[v->type] : PC_WHITE;

			/* And draw either one or two pixels depending on clipping */
			blitter->SetPixel(dpi->dst_ptr, x, y, colour);
			if (!skip) blitter->SetPixel(dpi->dst_ptr, x + 1, y, colour);
		}
	}

	/**
	 * Adds town names to the smallmap.
	 * @param dpi the part of the smallmap to be drawn into
	 */
	void DrawTowns(const DrawPixelInfo *dpi) const
	{
		const Town *t;
		FOR_ALL_TOWNS(t) {
			/* Remap the town coordinate */
			Point pt = this->RemapTile(TileX(t->xy), TileY(t->xy));
			int x = pt.x - this->subscroll - (t->cache.sign.width_small >> 1);
			int y = pt.y;

			/* Check if the town sign is within bounds */
			if (x + t->cache.sign.width_small > dpi->left &&
					x < dpi->left + dpi->width &&
					y + FONT_HEIGHT_SMALL > dpi->top &&
					y < dpi->top + dpi->height) {
				/* And draw it. */
				SetDParam(0, t->index);
				DrawString(x, x + t->cache.sign.width_small, y, STR_SMALLMAP_TOWN);
			}
		}
	}

	/**
	 * Adds the route links to the smallmap.
	 */
	void DrawRouteLinks() const
	{
		/* Iterate all shown cargo types. */
		for (int i = 0; i < _smallmap_cargo_count; i++) {
			if (_legend_from_cargoes[i].show_on_map) {
				CargoID cid = _legend_from_cargoes[i].cid;

				/* Iterate all stations. */
				const Station *st;
				FOR_ALL_STATIONS(st) {
					Point src_pt = this->RemapTile(TileX(st->xy), TileY(st->xy));
					src_pt.x -= this->subscroll;

					/* Collect waiting cargo per destination station. */
					std::map<StationID, uint> links;
					for (RouteLinkList::const_iterator l = st->goods[cid].routes.begin(); l != st->goods[cid].routes.end(); ++l) {
						if (IsInteractiveCompany((*l)->GetOwner())) links[(*l)->GetDestination()] += st->goods[cid].cargo.CountForNextHop((*l)->GetOriginOrderId());
					}

					/* Add cargo count on back-links. */
					for (std::map<StationID, uint>::iterator itr = links.begin(); itr != links.end(); ++itr) {
						/* Get destination location. */
						const Station *dest = Station::Get(itr->first);
						Point dest_pt = this->RemapTile(TileX(dest->xy), TileY(dest->xy));
						dest_pt.x -= this->subscroll;

						/* Get total count including back-links. */
						uint count = itr->second;
						for (RouteLinkList::const_iterator j = dest->goods[cid].routes.begin(); j != dest->goods[cid].routes.end(); ++j) {
							if ((*j)->GetDestination() == st->index && IsInteractiveCompany((*j)->GetOwner())) count += dest->goods[cid].cargo.CountForNextHop((*j)->GetOriginOrderId());
						}

						/* Calculate line size from waiting cargo. */
						int size = 1;
						if (count >= 400) size++;
						if (count >= 800) size++;
						if (count >= 1600) size++;
						if (count >= 3200) size++;

						/* Draw black border and cargo coloured line. */
						GfxDrawLine(src_pt.x, src_pt.y, dest_pt.x, dest_pt.y, PC_BLACK, size + 2);
						GfxDrawLine(src_pt.x, src_pt.y, dest_pt.x, dest_pt.y, _legend_from_cargoes[i].colour, size);
					}
				}
			}
		}

		/* Draw station rect. */
		const Station *st;
		FOR_ALL_STATIONS(st) {
			/* Count total cargo and check for links for all shown cargo types. */
			uint total = 0;
			bool show = false;
			for (CargoID cid = 0; cid < NUM_CARGO; cid++) {
				if (_legend_from_cargoes[_cargotype_to_list_pos[cid]].show_on_map) {
					total += st->goods[cid].cargo.Count();
					show |= !st->goods[cid].routes.empty();
				}
			}

			if (!show) continue;

			/* Get rect size from total cargo count. */
			int d = 1;
			if (total >= 200) d++;
			if (total >= 400) d++;
			if (total >= 800) d++;
			if (total >= 1600) d++;
			if (total >= 3200) d++;
			if (total >= 6400) d++;

			/* Get top-left corner of the rect. */
			Point dest_pt = this->RemapTile(TileX(st->xy), TileY(st->xy));
			dest_pt.x -= this->subscroll + d/2;
			dest_pt.y -= d/2;

			/* Draw black border and company-colour inset. */
			byte colour = _colour_gradient[Company::IsValidID(st->owner) ? Company::Get(st->owner)->colour : (byte)COLOUR_GREY][6];
			GfxFillRect(dest_pt.x - 1, dest_pt.y - 1, dest_pt.x + d + 1, dest_pt.y + d + 1, PC_BLACK); // Draw black frame
			GfxFillRect(dest_pt.x, dest_pt.y, dest_pt.x + d, dest_pt.y + d, colour); // Draw colour insert
		}
	}

	Point GetSmallMapCoordIncludingHeight(Point viewport_coord) const
	{
		/* First find out which tile would be there if we ignore height */
		Point pt = InverseRemapCoords(viewport_coord.x, viewport_coord.y);
		Point pt_without_height = {pt.x / TILE_SIZE, pt.y / TILE_SIZE};

		/* Problem: There are mountains.  So the tile actually displayed at the given position
		 * might be the high mountain of 30 tiles south.
		 * Unfortunately, there is no closed formula for finding such a tile.
		 * We call GetRowAtTile originally implemented for the viewport code, which performs
		 * a interval search.  For details, see its documentation. */
		int row_without_height = pt_without_height.x + pt_without_height.y;
		int row_with_height = GetRowAtTile(viewport_coord.y, pt_without_height);
		int row_offset = row_with_height - row_without_height;
		Point pt_with_height = {pt_without_height.x + row_offset / 2, pt_without_height.y + row_offset / 2};
		return pt_with_height;
	}

	/**
	 * Draws vertical part of map indicator
	 * @param x X coord of left/right border of main viewport
	 * @param y Y coord of top border of main viewport
	 * @param y2 Y coord of bottom border of main viewport
	 */
	static inline void DrawVertMapIndicator(int x, int y, int y2)
	{
		GfxFillRect(x, y,      x, y + 3, PC_VERY_LIGHT_YELLOW);
		GfxFillRect(x, y2 - 3, x, y2,    PC_VERY_LIGHT_YELLOW);
	}

	/**
	 * Draws horizontal part of map indicator
	 * @param x X coord of left border of main viewport
	 * @param x2 X coord of right border of main viewport
	 * @param y Y coord of top/bottom border of main viewport
	 */
	static inline void DrawHorizMapIndicator(int x, int x2, int y)
	{
		GfxFillRect(x,      y, x + 3, y, PC_VERY_LIGHT_YELLOW);
		GfxFillRect(x2 - 3, y, x2,    y, PC_VERY_LIGHT_YELLOW);
	}

	/**
	 * Adds map indicators to the smallmap.
	 */
	void DrawMapIndicators() const
	{
		/* Find main viewport. */
		const ViewPort *vp = FindWindowById(WC_MAIN_WINDOW, 0)->viewport;

		Point upper_left_viewport_coord = {vp->virtual_left, vp->virtual_top};
		Point upper_left_small_map_coord = GetSmallMapCoordIncludingHeight(upper_left_viewport_coord);
		Point upper_left = this->RemapTile(upper_left_small_map_coord.x, upper_left_small_map_coord.y);
		/* why do we do this? in my tests subscroll was zero */
		upper_left.x -= this->subscroll;

		Point lower_right_viewport_coord = {vp->virtual_left + vp->virtual_width, vp->virtual_top + vp->virtual_height};
		Point lower_right_smallmap_coord = GetSmallMapCoordIncludingHeight(lower_right_viewport_coord);
		Point lower_right = this->RemapTile(lower_right_smallmap_coord.x, lower_right_smallmap_coord.y);
		/* why do we do this? in my tests subscroll was zero */
		lower_right.x -= this->subscroll;

		SmallMapWindow::DrawVertMapIndicator(upper_left.x, upper_left.y, lower_right.y);
		SmallMapWindow::DrawVertMapIndicator(lower_right.x, upper_left.y, lower_right.y);

		SmallMapWindow::DrawHorizMapIndicator(upper_left.x, lower_right.x, upper_left.y);
		SmallMapWindow::DrawHorizMapIndicator(upper_left.x, lower_right.x, lower_right.y);
	}

	/**
	 * Draws the small map.
	 *
	 * Basically, the small map is draw column of pixels by column of pixels. The pixels
	 * are drawn directly into the screen buffer. The final map is drawn in multiple passes.
	 * The passes are:
	 * <ol><li>The colours of tiles in the different modes.</li>
	 * <li>Town names (optional)</li></ol>
	 *
	 * @param dpi pointer to pixel to write onto
	 */
	void DrawSmallMap(DrawPixelInfo *dpi) const
	{
		Blitter *blitter = BlitterFactoryBase::GetCurrentBlitter();
		DrawPixelInfo *old_dpi;

		old_dpi = _cur_dpi;
		_cur_dpi = dpi;

		/* Clear it */
		GfxFillRect(dpi->left, dpi->top, dpi->left + dpi->width - 1, dpi->top + dpi->height - 1, PC_BLACK);

		/* Which tile is displayed at (dpi->left, dpi->top)? */
		int dx;
		Point tile = this->PixelToTile(dpi->left, dpi->top, &dx);
		int tile_x = this->scroll_x / (int)TILE_SIZE + tile.x;
		int tile_y = this->scroll_y / (int)TILE_SIZE + tile.y;

		void *ptr = blitter->MoveTo(dpi->dst_ptr, -dx - 4, 0);
		int x = - dx - 4;
		int y = 0;

		for (;;) {
			/* Distance from left edge */
			if (x >= -3) {
				if (x >= dpi->width) break; // Exit the loop.

				int end_pos = min(dpi->width, x + 4);
				int reps = (dpi->height - y + 1) / 2; // Number of lines.
				if (reps > 0) {
					this->DrawSmallMapColumn(ptr, tile_x, tile_y, dpi->pitch * 2, reps, x, end_pos, blitter);
				}
			}

			if (y == 0) {
				tile_y += this->zoom;
				y++;
				ptr = blitter->MoveTo(ptr, 0, 1);
			} else {
				tile_x -= this->zoom;
				y--;
				ptr = blitter->MoveTo(ptr, 0, -1);
			}
			ptr = blitter->MoveTo(ptr, 2, 0);
			x += 2;
		}

		/* Draw vehicles */
		if (this->map_type == SMT_CONTOUR || this->map_type == SMT_VEHICLES) this->DrawVehicles(dpi, blitter);

		/* Draw route links. */
		if (this->map_type == SMT_ROUTE_LINKS) this->DrawRouteLinks();

		/* Draw town names */
		if (this->show_towns) this->DrawTowns(dpi);

		/* Draw map indicators */
		this->DrawMapIndicators();

		_cur_dpi = old_dpi;
	}

	/**
	 * Function to set up widgets depending on the information being shown on the smallmap.
	 */
	void SetupWidgetData()
	{
		StringID legend_tooltip;
		StringID enable_all_tooltip;
		StringID disable_all_tooltip;
		int plane;
		switch (this->map_type) {
			case SMT_INDUSTRY:
				legend_tooltip = STR_SMALLMAP_TOOLTIP_INDUSTRY_SELECTION;
				enable_all_tooltip = STR_SMALLMAP_TOOLTIP_ENABLE_ALL_INDUSTRIES;
				disable_all_tooltip = STR_SMALLMAP_TOOLTIP_DISABLE_ALL_INDUSTRIES;
				plane = 0;
				break;

			case SMT_OWNER:
				legend_tooltip = STR_SMALLMAP_TOOLTIP_COMPANY_SELECTION;
				enable_all_tooltip = STR_SMALLMAP_TOOLTIP_ENABLE_ALL_COMPANIES;
				disable_all_tooltip = STR_SMALLMAP_TOOLTIP_DISABLE_ALL_COMPANIES;
				plane = 0;
				break;

			case SMT_ROUTE_LINKS:
				legend_tooltip = STR_SMALLMAP_TOOLTIP_ROUTELINK_SELECTION;
				enable_all_tooltip = STR_SMALLMAP_TOOLTIP_ENABLE_ALL_ROUTELINKS;
				disable_all_tooltip = STR_SMALLMAP_TOOLTIP_DISABLE_ALL_ROUTELINKS;
				plane = 0;
				break;

			default:
				legend_tooltip = STR_NULL;
				enable_all_tooltip = STR_NULL;
				disable_all_tooltip = STR_NULL;
				plane = 1;
				break;
		}

		this->GetWidget<NWidgetCore>(WID_SM_LEGEND)->SetDataTip(STR_NULL, legend_tooltip);
		this->GetWidget<NWidgetCore>(WID_SM_ENABLE_ALL)->SetDataTip(STR_SMALLMAP_ENABLE_ALL, enable_all_tooltip);
		this->GetWidget<NWidgetCore>(WID_SM_DISABLE_ALL)->SetDataTip(STR_SMALLMAP_DISABLE_ALL, disable_all_tooltip);
		this->GetWidget<NWidgetStacked>(WID_SM_SELECT_BUTTONS)->SetDisplayedPlane(plane);
	}

public:
	uint min_number_of_columns;    ///< Minimal number of columns in legends.

	SmallMapWindow(const WindowDesc *desc, int window_number) : Window(), refresh(FORCE_REFRESH_PERIOD)
	{
		_smallmap_industry_highlight = INVALID_INDUSTRYTYPE;
		this->InitNested(desc, window_number);
		this->LowerWidget(this->map_type + WID_SM_CONTOUR);

		BuildLandLegend();
		this->SetWidgetLoweredState(WID_SM_SHOW_HEIGHT, _smallmap_show_heightmap);

		this->SetWidgetLoweredState(WID_SM_TOGGLETOWNNAME, this->show_towns);

		this->SetupWidgetData();

		this->SetZoomLevel(ZLC_INITIALIZE, NULL);
		this->SmallMapCenterOnCurrentPos();
	}

	/**
	 * Compute minimal required width of the legends.
	 * @return Minimally needed width for displaying the smallmap legends in pixels.
	 */
	inline uint GetMinLegendWidth() const
	{
		return WD_FRAMERECT_LEFT + this->min_number_of_columns * this->column_width;
	}

	/**
	 * Return number of columns that can be displayed in \a width pixels.
	 * @return Number of columns to display.
	 */
	inline uint GetNumberColumnsLegend(uint width) const
	{
		return width / this->column_width;
	}

	/**
	 * Compute height given a number of columns.
	 * @param num_columns Number of columns.
	 * @return Needed height for displaying the smallmap legends in pixels.
	 */
	uint GetLegendHeight(uint num_columns) const
	{
		uint num_rows = max(this->min_number_of_fixed_rows, CeilDiv(max(_smallmap_company_count, _smallmap_industry_count), num_columns));
		return WD_FRAMERECT_TOP + WD_FRAMERECT_BOTTOM + num_rows * FONT_HEIGHT_SMALL;
	}

	virtual void SetStringParameters(int widget) const
	{
		switch (widget) {
			case WID_SM_CAPTION:
				SetDParam(0, STR_SMALLMAP_TYPE_CONTOURS + this->map_type);
				break;
		}
	}

	virtual void OnInit()
	{
		uint min_width = 0;
		this->min_number_of_columns = INDUSTRY_MIN_NUMBER_OF_COLUMNS;
		this->min_number_of_fixed_rows = 0;
		for (uint i = 0; i < lengthof(_legend_table); i++) {
			uint height = 0;
			uint num_columns = 1;
			for (const LegendAndColour *tbl = _legend_table[i]; !tbl->end; ++tbl) {
				StringID str;
				if (i == SMT_INDUSTRY) {
					SetDParam(0, tbl->legend);
					SetDParam(1, IndustryPool::MAX_SIZE);
					str = STR_SMALLMAP_INDUSTRY;
				} else if (i == SMT_OWNER) {
					if (tbl->company != INVALID_COMPANY) {
						if (!Company::IsValidID(tbl->company)) {
							/* Rebuild the owner legend. */
							BuildOwnerLegend();
							this->OnInit();
							return;
						}
						/* Non-fixed legend entries for the owner view. */
						SetDParam(0, tbl->company);
						str = STR_SMALLMAP_COMPANY;
					} else {
						str = tbl->legend;
					}
				} else if (i == SMT_ROUTE_LINKS) {
					SetDParam(0, tbl->legend);
					str = STR_SMALLMAP_CARGO;
				} else {
					if (tbl->col_break) {
						this->min_number_of_fixed_rows = max(this->min_number_of_fixed_rows, height);
						height = 0;
						num_columns++;
					}
					height++;
					str = tbl->legend;
				}
				min_width = max(GetStringBoundingBox(str).width, min_width);
			}
			this->min_number_of_fixed_rows = max(this->min_number_of_fixed_rows, height);
			this->min_number_of_columns = max(this->min_number_of_columns, num_columns);
		}

		/* The width of a column is the minimum width of all texts + the size of the blob + some spacing */
		this->column_width = min_width + LEGEND_BLOB_WIDTH + WD_FRAMERECT_LEFT + WD_FRAMERECT_RIGHT;
	}

	virtual void OnPaint()
	{
		if (this->map_type == SMT_OWNER) {
			for (const LegendAndColour *tbl = _legend_table[this->map_type]; !tbl->end; ++tbl) {
				if (tbl->company != INVALID_COMPANY && !Company::IsValidID(tbl->company)) {
					/* Rebuild the owner legend. */
					BuildOwnerLegend();
					this->InvalidateData(1);
					break;
				}
			}
		}

		this->DrawWidgets();
	}

	virtual void DrawWidget(const Rect &r, int widget) const
	{
		switch (widget) {
			case WID_SM_MAP: {
				DrawPixelInfo new_dpi;
				if (!FillDrawPixelInfo(&new_dpi, r.left + 1, r.top + 1, r.right - r.left - 1, r.bottom - r.top - 1)) return;
				this->DrawSmallMap(&new_dpi);
				break;
			}

			case WID_SM_LEGEND: {
				uint columns = this->GetNumberColumnsLegend(r.right - r.left + 1);
				uint number_of_rows = max((this->map_type == SMT_INDUSTRY || this->map_type == SMT_OWNER || this->map_type == SMT_ROUTE_LINKS) ? CeilDiv(max(_smallmap_company_count, max(_smallmap_industry_count, _smallmap_cargo_count)), columns) : 0, this->min_number_of_fixed_rows);
				bool rtl = _current_text_dir == TD_RTL;
				uint y_org = r.top + WD_FRAMERECT_TOP;
				uint x = rtl ? r.right - this->column_width - WD_FRAMERECT_RIGHT : r.left + WD_FRAMERECT_LEFT;
				uint y = y_org;
				uint i = 0; // Row counter for industry legend.
				uint row_height = FONT_HEIGHT_SMALL;

				uint text_left  = rtl ? 0 : LEGEND_BLOB_WIDTH + WD_FRAMERECT_LEFT;
				uint text_right = this->column_width - 1 - (rtl ? LEGEND_BLOB_WIDTH + WD_FRAMERECT_RIGHT : 0);
				uint blob_left  = rtl ? this->column_width - 1 - LEGEND_BLOB_WIDTH : 0;
				uint blob_right = rtl ? this->column_width - 1 : LEGEND_BLOB_WIDTH;

				for (const LegendAndColour *tbl = _legend_table[this->map_type]; !tbl->end; ++tbl) {
					if (tbl->col_break || ((this->map_type == SMT_INDUSTRY || this->map_type == SMT_OWNER || this->map_type == SMT_ROUTE_LINKS) && i++ >= number_of_rows)) {
						/* Column break needed, continue at top, COLUMN_WIDTH pixels
						 * (one "row") to the right. */
						x += rtl ? -(int)this->column_width : this->column_width;
						y = y_org;
						i = 1;
					}

					uint8 legend_colour = tbl->colour;

					if (this->map_type == SMT_INDUSTRY) {
						/* Industry name must be formatted, since it's not in tiny font in the specs.
						 * So, draw with a parameter and use the STR_SMALLMAP_INDUSTRY string, which is tiny font */
						SetDParam(0, tbl->legend);
						SetDParam(1, Industry::GetIndustryTypeCount(tbl->type));
						if (!tbl->show_on_map) {
							/* Simply draw the string, not the black border of the legend colour.
							 * This will enforce the idea of the disabled item */
							DrawString(x + text_left, x + text_right, y, STR_SMALLMAP_INDUSTRY, TC_GREY);
						} else {
							if (tbl->type == _smallmap_industry_highlight) {
								legend_colour = _smallmap_industry_highlight_state ? PC_WHITE : PC_BLACK;
							}
							DrawString(x + text_left, x + text_right, y, STR_SMALLMAP_INDUSTRY, TC_BLACK);
							GfxFillRect(x + blob_left, y + 1, x + blob_right, y + row_height - 1, PC_BLACK); // Outer border of the legend colour
						}
					} else if (this->map_type == SMT_ROUTE_LINKS) {
						/* Cargo name needs formatting for tiny font. */
						SetDParam(0, tbl->legend);
						if (!tbl->show_on_map) {
							/* Draw only the string and not the border of the legend colour. */
							DrawString(x + text_left, x + text_right, y, STR_SMALLMAP_CARGO, TC_GREY);
						} else {
							DrawString(x + text_left, x + text_right, y, STR_SMALLMAP_CARGO, TC_BLACK);
							GfxFillRect(x + blob_left, y + 1, x + blob_right, y + row_height - 1, PC_BLACK); // Outer border of the legend colour
						}
					} else if (this->map_type == SMT_OWNER && tbl->company != INVALID_COMPANY) {
						SetDParam(0, tbl->company);
						if (!tbl->show_on_map) {
							/* Simply draw the string, not the black border of the legend colour.
							 * This will enforce the idea of the disabled item */
							DrawString(x + text_left, x + text_right, y, STR_SMALLMAP_COMPANY, TC_GREY);
						} else {
							DrawString(x + text_left, x + text_right, y, STR_SMALLMAP_COMPANY, TC_BLACK);
							GfxFillRect(x + blob_left, y + 1, x + blob_right, y + row_height - 1, PC_BLACK); // Outer border of the legend colour
						}
					} else {
						if (this->map_type == SMT_CONTOUR) SetDParam(0, tbl->height * TILE_HEIGHT_STEP);

						/* Anything that is not an industry or a company is using normal process */
						GfxFillRect(x + blob_left, y + 1, x + blob_right, y + row_height - 1, PC_BLACK);
						DrawString(x + text_left, x + text_right, y, tbl->legend);
					}
					GfxFillRect(x + blob_left + 1, y + 2, x + blob_right - 1, y + row_height - 2, legend_colour); // Legend colour

					y += row_height;
				}
			}
		}
	}

	/**
	 * Select a new map type.
	 * @param map_type New map type.
	 */
	void SwitchMapType(SmallMapType map_type)
	{
		this->RaiseWidget(this->map_type + WID_SM_CONTOUR);
		this->map_type = map_type;
		this->LowerWidget(this->map_type + WID_SM_CONTOUR);

		this->SetupWidgetData();

		this->SetDirty();
	}

	/**
	 * Determines the mouse position on the legend.
	 * @param pt Mouse position.
	 * @return Legend item under the mouse.
	 */
	int GetPositionOnLegend(Point pt)
	{
		const NWidgetBase *wi = this->GetWidget<NWidgetBase>(WID_SM_LEGEND);
		uint line = (pt.y - wi->pos_y - WD_FRAMERECT_TOP) / FONT_HEIGHT_SMALL;
		uint columns = this->GetNumberColumnsLegend(wi->current_x);
		uint number_of_rows = max(CeilDiv(max(_smallmap_company_count, max(_smallmap_industry_count, _smallmap_cargo_count)), columns), this->min_number_of_fixed_rows);
		if (line >= number_of_rows) return -1;

		bool rtl = _current_text_dir == TD_RTL;
		int x = pt.x - wi->pos_x;
		if (rtl) x = wi->current_x - x;
		uint column = (x - WD_FRAMERECT_LEFT) / this->column_width;

		return (column * number_of_rows) + line;
	}

	virtual void OnMouseOver(Point pt, int widget)
	{
		IndustryType new_highlight = INVALID_INDUSTRYTYPE;
		if (widget == WID_SM_LEGEND && this->map_type == SMT_INDUSTRY) {
			int industry_pos = GetPositionOnLegend(pt);
			if (industry_pos >= 0 && industry_pos < _smallmap_industry_count) {
				new_highlight = _legend_from_industries[industry_pos].type;
			}
		}
		if (new_highlight != _smallmap_industry_highlight) {
			_smallmap_industry_highlight = new_highlight;
			this->refresh = _smallmap_industry_highlight != INVALID_INDUSTRYTYPE ? BLINK_PERIOD : FORCE_REFRESH_PERIOD;
			_smallmap_industry_highlight_state = true;
			this->SetDirty();
		}
	}

	virtual void OnClick(Point pt, int widget, int click_count)
	{
		/* User clicked something, notify the industry chain window to stop sending newly selected industries. */
		InvalidateWindowClassesData(WC_INDUSTRY_CARGOES, NUM_INDUSTRYTYPES);

		switch (widget) {
			case WID_SM_MAP: { // Map window
				/*
				 * XXX: scrolling with the left mouse button is done by subsequently
				 * clicking with the left mouse button; clicking once centers the
				 * large map at the selected point. So by unclicking the left mouse
				 * button here, it gets reclicked during the next inputloop, which
				 * would make it look like the mouse is being dragged, while it is
				 * actually being (virtually) clicked every inputloop.
				 */
				_left_button_clicked = false;

				const NWidgetBase *wid = this->GetWidget<NWidgetBase>(WID_SM_MAP);
				Window *w = FindWindowById(WC_MAIN_WINDOW, 0);
				int sub;
				pt = this->PixelToTile(pt.x - wid->pos_x, pt.y - wid->pos_y, &sub);
				pt = RemapCoords(this->scroll_x + pt.x * TILE_SIZE + this->zoom * (TILE_SIZE - sub * TILE_SIZE / 4),
						this->scroll_y + pt.y * TILE_SIZE + sub * this->zoom * TILE_SIZE / 4, 0);

				/* correct y coordinate according to the height level at the chosen tile
				 * - so far we assumed height zero.  Calculations here according to
				 * TranslateXYToTileCoord in viewport.cpp */
				Point pt_scaled = {pt.x / (int)(4 * TILE_SIZE), pt.y / (int)(2 * TILE_SIZE)};
				Point tile_coord = {pt_scaled.y - pt_scaled.x, pt_scaled.y + pt_scaled.x};

				if (tile_coord.x >= 0 && tile_coord.y >= 0
				 && tile_coord.x < (int)MapMaxX() && tile_coord.y < (int)MapMaxY()) {
					int clicked_tile_height = TileHeight(TileXY(tile_coord.x, tile_coord.y));
					pt.y -= clicked_tile_height * TILE_HEIGHT;
				}

				w->viewport->follow_vehicle = INVALID_VEHICLE;
				w->viewport->dest_scrollpos_x = pt.x - (w->viewport->virtual_width  >> 1);
				w->viewport->dest_scrollpos_y = pt.y - (w->viewport->virtual_height >> 1);

				this->SetDirty();
				break;
			}

			case WID_SM_ZOOM_IN:
			case WID_SM_ZOOM_OUT: {
				const NWidgetBase *wid = this->GetWidget<NWidgetBase>(WID_SM_MAP);
				Point pt = {wid->current_x / 2, wid->current_y / 2};
				this->SetZoomLevel((widget == WID_SM_ZOOM_IN) ? ZLC_ZOOM_IN : ZLC_ZOOM_OUT, &pt);
				if (_settings_client.sound.click_beep) SndPlayFx(SND_15_BEEP);
				break;
			}

			case WID_SM_CONTOUR:    // Show land contours
			case WID_SM_VEHICLES:   // Show vehicles
			case WID_SM_INDUSTRIES: // Show industries
			case WID_SM_ROUTE_LINKS:// Show route links
			case WID_SM_ROUTES:     // Show transport routes
			case WID_SM_VEGETATION: // Show vegetation
			case WID_SM_OWNERS:     // Show land owners
				this->SwitchMapType((SmallMapType)(widget - WID_SM_CONTOUR));
				if (_settings_client.sound.click_beep) SndPlayFx(SND_15_BEEP);
				break;

			case WID_SM_CENTERMAP: // Center the smallmap again
				this->SmallMapCenterOnCurrentPos();
				this->HandleButtonClick(WID_SM_CENTERMAP);
				if (_settings_client.sound.click_beep) SndPlayFx(SND_15_BEEP);
				break;

			case WID_SM_TOGGLETOWNNAME: // Toggle town names
				this->show_towns = !this->show_towns;
				this->SetWidgetLoweredState(WID_SM_TOGGLETOWNNAME, this->show_towns);

				this->SetDirty();
				if (_settings_client.sound.click_beep) SndPlayFx(SND_15_BEEP);
				break;

			case WID_SM_LEGEND: // Legend
				if (this->map_type == SMT_INDUSTRY || this->map_type == SMT_OWNER || this->map_type == SMT_ROUTE_LINKS) {
					/* If industry type small map*/
					if (this->map_type == SMT_INDUSTRY) {
						/* If click on industries label, find right industry type and enable/disable it. */
						int industry_pos = GetPositionOnLegend(pt);
						if (industry_pos >= 0 && industry_pos < _smallmap_industry_count) {
							if (_ctrl_pressed) {
								/* Disable all, except the clicked one. */
								bool changes = false;
								for (int i = 0; i != _smallmap_industry_count; i++) {
									bool new_state = i == industry_pos;
									if (_legend_from_industries[i].show_on_map != new_state) {
										changes = true;
										_legend_from_industries[i].show_on_map = new_state;
									}
								}
								if (!changes) {
									/* Nothing changed? Then show all (again). */
									for (int i = 0; i != _smallmap_industry_count; i++) {
										_legend_from_industries[i].show_on_map = true;
									}
								}
							} else {
								_legend_from_industries[industry_pos].show_on_map = !_legend_from_industries[industry_pos].show_on_map;
							}
						}
					} else if (this->map_type == SMT_OWNER) {
						/* If click on companies label, find right company and enable/disable it. */
						int company_pos = GetPositionOnLegend(pt);
						if (company_pos < NUM_NO_COMPANY_ENTRIES) break;
						if (company_pos < _smallmap_company_count) {
							if (_ctrl_pressed) {
								/* Disable all, except the clicked one */
								bool changes = false;
								for (int i = NUM_NO_COMPANY_ENTRIES; i != _smallmap_company_count; i++) {
									bool new_state = i == company_pos;
									if (_legend_land_owners[i].show_on_map != new_state) {
										changes = true;
										_legend_land_owners[i].show_on_map = new_state;
									}
								}
								if (!changes) {
									/* Nothing changed? Then show all (again). */
									for (int i = NUM_NO_COMPANY_ENTRIES; i != _smallmap_company_count; i++) {
										_legend_land_owners[i].show_on_map = true;
									}
								}
							} else {
								_legend_land_owners[company_pos].show_on_map = !_legend_land_owners[company_pos].show_on_map;
							}
						}
					} else if (this->map_type == SMT_ROUTE_LINKS) {
						/* If click on cargo label, find right cargo type and enable/disable it. */
						int cargo_pos = GetPositionOnLegend(pt);
						if (cargo_pos < _smallmap_cargo_count) {
							if (_ctrl_pressed) {
								/* Disable all, except the clicked one */
								bool changes = false;
								for (int i = 0; i != _smallmap_cargo_count; i++) {
									bool new_state = i == cargo_pos;
									if (_legend_from_cargoes[i].show_on_map != new_state) {
										changes = true;
										_legend_from_cargoes[i].show_on_map = new_state;
									}
								}
								if (!changes) {
									/* Nothing changed? Then show all (again). */
									for (int i = 0; i != _smallmap_cargo_count; i++) {
										_legend_from_cargoes[i].show_on_map = true;
									}
								}
							} else {
								_legend_from_cargoes[cargo_pos].show_on_map = !_legend_from_cargoes[cargo_pos].show_on_map;
							}
						}
					}
					this->SetDirty();
				}
				break;

			case WID_SM_ENABLE_ALL:
				if (this->map_type == SMT_INDUSTRY) {
					for (int i = 0; i != _smallmap_industry_count; i++) {
						_legend_from_industries[i].show_on_map = true;
					}
				} else if (this->map_type == SMT_OWNER) {
					for (int i = NUM_NO_COMPANY_ENTRIES; i != _smallmap_company_count; i++) {
						_legend_land_owners[i].show_on_map = true;
					}
				} else if (this->map_type == SMT_ROUTE_LINKS) {
					for (int i = 0; i != _smallmap_cargo_count; i++) {
						_legend_from_cargoes[i].show_on_map = true;
					}
				}
				this->SetDirty();
				break;

			case WID_SM_DISABLE_ALL:
				if (this->map_type == SMT_INDUSTRY) {
					for (int i = 0; i != _smallmap_industry_count; i++) {
						_legend_from_industries[i].show_on_map = false;
					}
				} else if (this->map_type == SMT_OWNER) {
					for (int i = NUM_NO_COMPANY_ENTRIES; i != _smallmap_company_count; i++) {
						_legend_land_owners[i].show_on_map = false;
					}
				} else if (this->map_type == SMT_ROUTE_LINKS) {
					for (int i = 0; i != _smallmap_cargo_count; i++) {
						_legend_from_cargoes[i].show_on_map = false;
					}
				}
				this->SetDirty();
				break;

			case WID_SM_SHOW_HEIGHT: // Enable/disable showing of heightmap.
				_smallmap_show_heightmap = !_smallmap_show_heightmap;
				this->SetWidgetLoweredState(WID_SM_SHOW_HEIGHT, _smallmap_show_heightmap);
				this->SetDirty();
				break;
		}
	}

	/**
	 * Some data on this window has become invalid.
	 * @param data Information about the changed data.
	 * - data = 0: Displayed industries at the industry chain window have changed.
	 * - data = 1: Companies have changed.
	 * @param gui_scope Whether the call is done from GUI scope. You may not do everything when not in GUI scope. See #InvalidateWindowData() for details.
	 */
	virtual void OnInvalidateData(int data = 0, bool gui_scope = true)
	{
		if (!gui_scope) return;
		switch (data) {
			case 1:
				/* The owner legend has already been rebuilt. */
				this->ReInit();
				break;

			case 0: {
				extern uint64 _displayed_industries;
				if (this->map_type != SMT_INDUSTRY) this->SwitchMapType(SMT_INDUSTRY);

				for (int i = 0; i != _smallmap_industry_count; i++) {
					_legend_from_industries[i].show_on_map = HasBit(_displayed_industries, _legend_from_industries[i].type);
				}
				break;
			}

			default: NOT_REACHED();
		}
		this->SetDirty();
	}

	virtual bool OnRightClick(Point pt, int widget)
	{
		if (widget != WID_SM_MAP || _scrolling_viewport) return false;

		_scrolling_viewport = true;
		return true;
	}

	virtual void OnMouseWheel(int wheel)
	{
		if (_settings_client.gui.scrollwheel_scrolling == 0) {
			const NWidgetBase *wid = this->GetWidget<NWidgetBase>(WID_SM_MAP);
			int cursor_x = _cursor.pos.x - this->left - wid->pos_x;
			int cursor_y = _cursor.pos.y - this->top  - wid->pos_y;
			if (IsInsideMM(cursor_x, 0, wid->current_x) && IsInsideMM(cursor_y, 0, wid->current_y)) {
				Point pt = {cursor_x, cursor_y};
				this->SetZoomLevel((wheel < 0) ? ZLC_ZOOM_IN : ZLC_ZOOM_OUT, &pt);
			}
		}
	}

	virtual void OnTick()
	{
		/* Update the window every now and then */
		if (--this->refresh != 0) return;

		_smallmap_industry_highlight_state = !_smallmap_industry_highlight_state;

		this->refresh = _smallmap_industry_highlight != INVALID_INDUSTRYTYPE ? BLINK_PERIOD : FORCE_REFRESH_PERIOD;
		this->SetDirty();
	}

	/**
	 * Set new #scroll_x, #scroll_y, and #subscroll values after limiting them such that the center
	 * of the smallmap always contains a part of the map.
	 * @param sx  Proposed new #scroll_x
	 * @param sy  Proposed new #scroll_y
	 * @param sub Proposed new #subscroll
	 */
	void SetNewScroll(int sx, int sy, int sub)
	{
		const NWidgetBase *wi = this->GetWidget<NWidgetBase>(WID_SM_MAP);
		Point hv = InverseRemapCoords(wi->current_x * ZOOM_LVL_BASE * TILE_SIZE / 2, wi->current_y * ZOOM_LVL_BASE * TILE_SIZE / 2);
		hv.x *= this->zoom;
		hv.y *= this->zoom;

		if (sx < -hv.x) {
			sx = -hv.x;
			sub = 0;
		}
		if (sx > (int)(MapMaxX() * TILE_SIZE) - hv.x) {
			sx = MapMaxX() * TILE_SIZE - hv.x;
			sub = 0;
		}
		if (sy < -hv.y) {
			sy = -hv.y;
			sub = 0;
		}
		if (sy > (int)(MapMaxY() * TILE_SIZE) - hv.y) {
			sy = MapMaxY() * TILE_SIZE - hv.y;
			sub = 0;
		}

		this->scroll_x = sx;
		this->scroll_y = sy;
		this->subscroll = sub;
	}

	virtual void OnScroll(Point delta)
	{
		_cursor.fix_at = true;

		/* While tile is at (delta.x, delta.y)? */
		int sub;
		Point pt = this->PixelToTile(delta.x, delta.y, &sub);
		this->SetNewScroll(this->scroll_x + pt.x * TILE_SIZE, this->scroll_y + pt.y * TILE_SIZE, sub);

		this->SetDirty();
	}

	void SmallMapCenterOnCurrentPos()
	{
		/* Goal: Given the viewport coordinates of the middle of the map window, find
		 * out which tile is displayed there. */

		/* First find out which tile would be there if we ignore height */
		const ViewPort *vp = FindWindowById(WC_MAIN_WINDOW, 0)->viewport;
		Point viewport_center = {vp->virtual_left + vp->virtual_width  / 2, vp->virtual_top  + vp->virtual_height / 2};
		Point pt_with_height = GetSmallMapCoordIncludingHeight(viewport_center);

		/* And finally scroll to that position. */
		int sub;
		const NWidgetBase *wid = this->GetWidget<NWidgetBase>(WID_SM_MAP);
		Point sxy = this->ComputeScroll(pt_with_height.x, pt_with_height.y,
				max(0, (int)wid->current_x / 2 - 2), wid->current_y / 2, &sub);
		this->SetNewScroll(sxy.x, sxy.y, sub);
		this->SetDirty();
	}
};

SmallMapWindow::SmallMapType SmallMapWindow::map_type = SMT_CONTOUR;
bool SmallMapWindow::show_towns = true;

/**
 * Custom container class for displaying smallmap with a vertically resizing legend panel.
 * The legend panel has a smallest height that depends on its width. Standard containers cannot handle this case.
 *
 * @note The container assumes it has two children, the first is the display, the second is the bar with legends and selection image buttons.
 *       Both children should be both horizontally and vertically resizable and horizontally fillable.
 *       The bar should have a minimal size with a zero-size legends display. Child padding is not supported.
 */
class NWidgetSmallmapDisplay : public NWidgetContainer {
	const SmallMapWindow *smallmap_window; ///< Window manager instance.
public:
	NWidgetSmallmapDisplay() : NWidgetContainer(NWID_VERTICAL)
	{
		this->smallmap_window = NULL;
	}

	virtual void SetupSmallestSize(Window *w, bool init_array)
	{
		NWidgetBase *display = this->head;
		NWidgetBase *bar = display->next;

		display->SetupSmallestSize(w, init_array);
		bar->SetupSmallestSize(w, init_array);

		this->smallmap_window = dynamic_cast<SmallMapWindow *>(w);
		this->smallest_x = max(display->smallest_x, bar->smallest_x + smallmap_window->GetMinLegendWidth());
		this->smallest_y = display->smallest_y + max(bar->smallest_y, smallmap_window->GetLegendHeight(smallmap_window->min_number_of_columns));
		this->fill_x = max(display->fill_x, bar->fill_x);
		this->fill_y = (display->fill_y == 0 && bar->fill_y == 0) ? 0 : min(display->fill_y, bar->fill_y);
		this->resize_x = max(display->resize_x, bar->resize_x);
		this->resize_y = min(display->resize_y, bar->resize_y);
	}

	virtual void AssignSizePosition(SizingType sizing, uint x, uint y, uint given_width, uint given_height, bool rtl)
	{
		this->pos_x = x;
		this->pos_y = y;
		this->current_x = given_width;
		this->current_y = given_height;

		NWidgetBase *display = this->head;
		NWidgetBase *bar = display->next;

		if (sizing == ST_SMALLEST) {
			this->smallest_x = given_width;
			this->smallest_y = given_height;
			/* Make display and bar exactly equal to their minimal size. */
			display->AssignSizePosition(ST_SMALLEST, x, y, display->smallest_x, display->smallest_y, rtl);
			bar->AssignSizePosition(ST_SMALLEST, x, y + display->smallest_y, bar->smallest_x, bar->smallest_y, rtl);
		}

		uint bar_height = max(bar->smallest_y, this->smallmap_window->GetLegendHeight(this->smallmap_window->GetNumberColumnsLegend(given_width - bar->smallest_x)));
		uint display_height = given_height - bar_height;
		display->AssignSizePosition(ST_RESIZE, x, y, given_width, display_height, rtl);
		bar->AssignSizePosition(ST_RESIZE, x, y + display_height, given_width, bar_height, rtl);
	}

	virtual NWidgetCore *GetWidgetFromPos(int x, int y)
	{
		if (!IsInsideBS(x, this->pos_x, this->current_x) || !IsInsideBS(y, this->pos_y, this->current_y)) return NULL;
		for (NWidgetBase *child_wid = this->head; child_wid != NULL; child_wid = child_wid->next) {
			NWidgetCore *widget = child_wid->GetWidgetFromPos(x, y);
			if (widget != NULL) return widget;
		}
		return NULL;
	}

	virtual void Draw(const Window *w)
	{
		for (NWidgetBase *child_wid = this->head; child_wid != NULL; child_wid = child_wid->next) child_wid->Draw(w);
	}
};

/** Widget parts of the smallmap display. */
static const NWidgetPart _nested_smallmap_display[] = {
	NWidget(WWT_PANEL, COLOUR_BROWN, WID_SM_MAP_BORDER),
		NWidget(WWT_INSET, COLOUR_BROWN, WID_SM_MAP), SetMinimalSize(346, 140), SetResize(1, 1), SetPadding(2, 2, 2, 2), EndContainer(),
	EndContainer(),
};

/** Widget parts of the smallmap legend bar + image buttons. */
static const NWidgetPart _nested_smallmap_bar[] = {
	NWidget(WWT_PANEL, COLOUR_BROWN),
		NWidget(NWID_HORIZONTAL),
			NWidget(WWT_EMPTY, INVALID_COLOUR, WID_SM_LEGEND), SetResize(1, 1),
			NWidget(NWID_VERTICAL),
				/* Top button row. */
				NWidget(NWID_HORIZONTAL, NC_EQUALSIZE),
					NWidget(WWT_PUSHIMGBTN, COLOUR_BROWN, WID_SM_ZOOM_IN),
							SetDataTip(SPR_IMG_ZOOMIN, STR_TOOLBAR_TOOLTIP_ZOOM_THE_VIEW_IN), SetFill(1, 1),
					NWidget(WWT_PUSHIMGBTN, COLOUR_BROWN, WID_SM_CENTERMAP),
							SetDataTip(SPR_IMG_SMALLMAP, STR_SMALLMAP_CENTER), SetFill(1, 1),
					NWidget(WWT_IMGBTN, COLOUR_BROWN, WID_SM_CONTOUR),
							SetDataTip(SPR_IMG_SHOW_COUNTOURS, STR_SMALLMAP_TOOLTIP_SHOW_LAND_CONTOURS_ON_MAP), SetFill(1, 1),
					NWidget(WWT_IMGBTN, COLOUR_BROWN, WID_SM_VEHICLES),
							SetDataTip(SPR_IMG_SHOW_VEHICLES, STR_SMALLMAP_TOOLTIP_SHOW_VEHICLES_ON_MAP), SetFill(1, 1),
					NWidget(WWT_IMGBTN, COLOUR_BROWN, WID_SM_INDUSTRIES),
							SetDataTip(SPR_IMG_INDUSTRY, STR_SMALLMAP_TOOLTIP_SHOW_INDUSTRIES_ON_MAP), SetFill(1, 1),
					NWidget(WWT_IMGBTN, COLOUR_BROWN, WID_SM_ROUTE_LINKS),
							SetDataTip(SPR_IMG_SHOW_ROUTES, STR_SMALLMAP_TOOLTIP_SHOW_ROUTE_LINKS_ON_MAP), SetFill(1, 1),
				EndContainer(),
				/* Bottom button row. */
				NWidget(NWID_HORIZONTAL, NC_EQUALSIZE),
					NWidget(WWT_PUSHIMGBTN, COLOUR_BROWN, WID_SM_ZOOM_OUT),
							SetDataTip(SPR_IMG_ZOOMOUT, STR_TOOLBAR_TOOLTIP_ZOOM_THE_VIEW_OUT), SetFill(1, 1),
					NWidget(WWT_IMGBTN, COLOUR_BROWN, WID_SM_TOGGLETOWNNAME),
							SetDataTip(SPR_IMG_TOWN, STR_SMALLMAP_TOOLTIP_TOGGLE_TOWN_NAMES_ON_OFF), SetFill(1, 1),
					NWidget(WWT_IMGBTN, COLOUR_BROWN, WID_SM_ROUTES),
							SetDataTip(SPR_IMG_SHOW_ROUTES, STR_SMALLMAP_TOOLTIP_SHOW_TRANSPORT_ROUTES_ON), SetFill(1, 1),
					NWidget(WWT_IMGBTN, COLOUR_BROWN, WID_SM_VEGETATION),
							SetDataTip(SPR_IMG_PLANTTREES, STR_SMALLMAP_TOOLTIP_SHOW_VEGETATION_ON_MAP), SetFill(1, 1),
					NWidget(WWT_IMGBTN, COLOUR_BROWN, WID_SM_OWNERS),
							SetDataTip(SPR_IMG_COMPANY_GENERAL, STR_SMALLMAP_TOOLTIP_SHOW_LAND_OWNERS_ON_MAP), SetFill(1, 1),
				EndContainer(),
				NWidget(NWID_SPACER), SetResize(0, 1),
			EndContainer(),
		EndContainer(),
	EndContainer(),
};

static NWidgetBase *SmallMapDisplay(int *biggest_index)
{
	NWidgetContainer *map_display = new NWidgetSmallmapDisplay;

	MakeNWidgets(_nested_smallmap_display, lengthof(_nested_smallmap_display), biggest_index, map_display);
	MakeNWidgets(_nested_smallmap_bar, lengthof(_nested_smallmap_bar), biggest_index, map_display);
	return map_display;
}


static const NWidgetPart _nested_smallmap_widgets[] = {
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_CLOSEBOX, COLOUR_BROWN),
		NWidget(WWT_CAPTION, COLOUR_BROWN, WID_SM_CAPTION), SetDataTip(STR_SMALLMAP_CAPTION, STR_TOOLTIP_WINDOW_TITLE_DRAG_THIS),
		NWidget(WWT_SHADEBOX, COLOUR_BROWN),
		NWidget(WWT_STICKYBOX, COLOUR_BROWN),
	EndContainer(),
	NWidgetFunction(SmallMapDisplay), // Smallmap display and legend bar + image buttons.
	/* Bottom button row and resize box. */
	NWidget(NWID_HORIZONTAL),
		NWidget(WWT_PANEL, COLOUR_BROWN),
			NWidget(NWID_HORIZONTAL),
				NWidget(NWID_SELECTION, INVALID_COLOUR, WID_SM_SELECT_BUTTONS),
					NWidget(NWID_HORIZONTAL, NC_EQUALSIZE),
						NWidget(WWT_PUSHTXTBTN, COLOUR_BROWN, WID_SM_ENABLE_ALL), SetDataTip(STR_SMALLMAP_ENABLE_ALL, STR_NULL),
						NWidget(WWT_PUSHTXTBTN, COLOUR_BROWN, WID_SM_DISABLE_ALL), SetDataTip(STR_SMALLMAP_DISABLE_ALL, STR_NULL),
						NWidget(WWT_TEXTBTN, COLOUR_BROWN, WID_SM_SHOW_HEIGHT), SetDataTip(STR_SMALLMAP_SHOW_HEIGHT, STR_SMALLMAP_TOOLTIP_SHOW_HEIGHT),
					EndContainer(),
					NWidget(NWID_SPACER), SetFill(1, 1),
				EndContainer(),
				NWidget(NWID_SPACER), SetFill(1, 0), SetResize(1, 0),
			EndContainer(),
		EndContainer(),
		NWidget(WWT_RESIZEBOX, COLOUR_BROWN),
	EndContainer(),
};

static const WindowDesc _smallmap_desc(
	WDP_AUTO, 446, 314,
	WC_SMALLMAP, WC_NONE,
	0,
	_nested_smallmap_widgets, lengthof(_nested_smallmap_widgets)
);

/**
 * Show the smallmap window.
 */
void ShowSmallMap()
{
	AllocateWindowDescFront<SmallMapWindow>(&_smallmap_desc, 0);
}

/**
 * Scrolls the main window to given coordinates.
 * @param x x coordinate
 * @param y y coordinate
 * @param z z coordinate; -1 to scroll to terrain height
 * @param instant scroll instantly (meaningful only when smooth_scrolling is active)
 * @return did the viewport position change?
 */
bool ScrollMainWindowTo(int x, int y, int z, bool instant)
{
	bool res = ScrollWindowTo(x, y, z, FindWindowById(WC_MAIN_WINDOW, 0), instant);

	/* If a user scrolls to a tile (via what way what so ever) and already is on
	 * that tile (e.g.: pressed twice), move the smallmap to that location,
	 * so you directly see where you are on the smallmap. */

	if (res) return res;

	SmallMapWindow *w = dynamic_cast<SmallMapWindow*>(FindWindowById(WC_SMALLMAP, 0));
	if (w != NULL) w->SmallMapCenterOnCurrentPos();

	return res;
}
