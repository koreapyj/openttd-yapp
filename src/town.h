/* $Id: town.h 24900 2013-01-08 22:46:42Z planetmaker $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file town.h Base of the town class. */

#ifndef TOWN_H
#define TOWN_H

#include "viewport_type.h"
#include "town_map.h"
#include "subsidy_type.h"
#include "openttd.h"
#include "table/strings.h"
#include "company_func.h"
#include "newgrf_storage.h"
#include "cargotype.h"
#include "tilematrix_type.hpp"
#include "cargodest_base.h"
#include <list>

template <typename T>
struct BuildingCounts {
	T id_count[HOUSE_MAX];
	T class_count[HOUSE_CLASS_MAX];
};

typedef TileMatrix<uint32, 4> AcceptanceMatrix;

static const uint CUSTOM_TOWN_NUMBER_DIFFICULTY  = 4; ///< value for custom town number in difficulty settings
static const uint CUSTOM_TOWN_MAX_NUMBER = 5000;  ///< this is the maximum number of towns a user can specify in customisation

static const uint INVALID_TOWN = 0xFFFF;

static const uint TOWN_GROWTH_WINTER = 0xFFFFFFFE; ///< The town only needs this cargo in the winter (any amount)
static const uint TOWN_GROWTH_DESERT = 0xFFFFFFFF; ///< The town needs the cargo for growth when on desert (any amount)
static const uint16 TOWN_GROW_RATE_CUSTOM = 0x8000; ///< If this mask is applied to Town::grow_counter, the grow_counter will not be calculated by the system (but assumed to be set by scripts)

typedef Pool<Town, TownID, 64, 64000> TownPool;
extern TownPool _town_pool;

/** Data structure with cached data of towns. */
struct TownCache {
	uint32 num_houses;                        ///< Amount of houses
	uint32 population;                        ///< Current population of people
	ViewportSign sign;                        ///< Location of name sign, UpdateVirtCoord updates this
	PartOfSubsidyByte part_of_subsidy;        ///< Is this town a source/destination of a subsidy?
	uint32 squared_town_zone_radius[HZB_END]; ///< UpdateTownRadius updates this given the house count
	BuildingCounts<uint16> building_counts;   ///< The number of each type of building in the town
};

/** Town data structure. */
struct Town : TownPool::PoolItem<&_town_pool>, CargoSourceSink {
	TileIndex xy;                  ///< town center tile
	TileIndex xy_aligned; ///< NOSAVE: Town centre aligned to the #AcceptanceMatrix grid.

	TownCache cache; ///< Container for all cacheable data.

	/* Town name */
	uint32 townnamegrfid;
	uint16 townnametype;
	uint32 townnameparts;
	char *name;

	/* Makes sure we don't build certain house types twice.
	 * bit 0 = Building funds received
	 * bit 1 = CHURCH
	 * bit 2 = STADIUM */
	byte flags;

	uint16 noise_reached;          ///< level of noise that all the airports are generating

	CompanyMask statues;           ///< which companies have a statue?

	/* Company ratings. */
	CompanyMask have_ratings;      ///< which companies have a rating
	uint8 unwanted[MAX_COMPANIES]; ///< how many months companies aren't wanted by towns (bribe)
	CompanyByte exclusivity;       ///< which company has exclusivity
	uint8 exclusive_counter;       ///< months till the exclusivity expires
	int16 ratings[MAX_COMPANIES];  ///< ratings of each company for this town
	StringID town_label;           ///< Label dependent on _local_company rating.

	TransportedCargoStat<uint32> supplied[NUM_CARGO]; ///< Cargo statistics about supplied cargo.
	TransportedCargoStat<uint16> received[NUM_TE];    ///< Cargo statistics about received cargotypes.
	uint32 goal[NUM_TE];                              ///< Amount of cargo required for the town to grow.

	char *text; ///< General text with additional information.

	inline byte GetPercentTransported(CargoID cid) const { return this->supplied[cid].old_act * 256 / (this->supplied[cid].old_max + 1); }

	/* Cargo production and acceptance stats. */
	uint32 cargo_produced;           ///< Bitmap of all cargoes produced by houses in this town.
	AcceptanceMatrix cargo_accepted; ///< Bitmap of cargoes accepted by houses for each 4*4 map square of the town.
	uint32 cargo_accepted_total;     ///< NOSAVE: Bitmap of all cargoes accepted by houses in this town.

	uint16 time_until_rebuild;     ///< time until we rebuild a house

	uint16 grow_counter;           ///< counter to count when to grow
	uint16 growth_rate;            ///< town growth rate

	byte fund_buildings_months;    ///< fund buildings program in action?
	byte road_build_months;        ///< fund road reconstruction in action?

	bool larger_town;              ///< if this is a larger town and should grow more quickly
	TownLayoutByte layout;         ///< town specific road layout

	std::list<PersistentStorage *> psa_list;

	/* Current cargo acceptance and production. */
	uint32 cargo_accepted_weights[NUM_CARGO]; ///< NOSAVE: Weight sum of accepting squares per cargo.
	uint32 cargo_accepted_max_weight; ///< NOSAVE: Cached maximum weight for an accepting square.

	void UpdateLabel();

	/**
	 * Returns the correct town label, based on rating.
	 */
	inline StringID Label() const{
		if (!(_game_mode == GM_EDITOR) && (_local_company < MAX_COMPANIES)) {
			return STR_VIEWPORT_TOWN_POP_VERY_POOR_RATING + this->town_label;
		} else {
			return _settings_client.gui.population_in_label ? STR_VIEWPORT_TOWN_POP : STR_VIEWPORT_TOWN;
		}
	}

	/**
	 * Returns the correct town small label, based on rating.
	 */
	inline StringID SmallLabel() const{
		if (!(_game_mode == GM_EDITOR) && (_local_company < MAX_COMPANIES)) {
			return STR_VIEWPORT_TOWN_TINY_VERY_POOR_RATING + this->town_label;
		} else {
			return STR_VIEWPORT_TOWN_TINY_WHITE;
		}
	}

	/**
	 * Creates a new town.
	 * @param tile center tile of the town
	 */
	Town(TileIndex tile = INVALID_TILE) : xy(tile) { }

	/** Destroy the town. */
	~Town();

	void InitializeLayout(TownLayout layout);

	/* virtual */ SourceType GetType() const
	{
		return ST_TOWN;
	}

	/* virtual */ SourceID GetID() const
	{
		return this->index;
	}

	/* virtual */ bool AcceptsCargo(CargoID cid) const
	{
		return HasBit(this->cargo_accepted_total, cid);
	}

	/* virtual */ bool SuppliesCargo(CargoID cid) const
	{
		return HasBit(this->cargo_produced, cid);
	}

	/* virtual */ uint GetDestinationWeight(CargoID cid, byte weight_mod) const;
	/* virtual */ void CreateSpecialLinks(CargoID cid);
	/* virtual */ TileArea GetTileForDestination(CargoID cid);

	/**
	 * Calculate the max town noise.
	 * The value is counted using the population divided by the content of the
	 * entry in town_noise_population corresponding to the town's tolerance.
	 * @return the maximum noise level the town will tolerate.
	 */
	inline uint16 MaxTownNoise() const
	{
		if (this->cache.population == 0) return 0; // no population? no noise

		/* 3 is added (the noise of the lowest airport), so the  user can at least build a small airfield. */
		return (this->cache.population / _settings_game.economy.town_noise_population[_settings_game.difficulty.town_council_tolerance]) + 3;
	}

	void UpdateVirtCoord();

	static inline Town *GetByTile(TileIndex tile)
	{
		return Town::Get(GetTownIndex(tile));
	}

	/** Callback function for #Town::GetRandom. */
	typedef bool (*EnumTownProc)(const Town *t, void *data);

	static Town *GetRandom(EnumTownProc enum_proc = NULL, TownID skip = INVALID_TOWN, void *data = NULL);
	static void PostDestructor(size_t index);
};

uint32 GetWorldPopulation();

void UpdateAllTownVirtCoords();
void ShowTownViewWindow(TownID town);
void ExpandTown(Town *t);

/**
 * Action types that a company must ask permission for to a town authority.
 * @see CheckforTownRating
 */
enum TownRatingCheckType {
	ROAD_REMOVE         = 0,      ///< Removal of a road owned by the town.
	TUNNELBRIDGE_REMOVE = 1,      ///< Removal of a tunnel or bridge owned by the towb.
	TOWN_RATING_CHECK_TYPE_COUNT, ///< Number of town checking action types.
};

/**
 * This enum is used in conjunction with town->flags.
 * IT simply states what bit is used for.
 * It is pretty unrealistic (IMHO) to only have one church/stadium
 * per town, NO MATTER the population of it.
 * And there are 5 more bits available on flags...
 */
enum TownFlags {
	TOWN_IS_FUNDED      = 0,   ///< Town has received some funds for
	TOWN_HAS_CHURCH     = 1,   ///< There can be only one church by town.
	TOWN_HAS_STADIUM    = 2,   ///< There can be only one stadium by town.
};

CommandCost CheckforTownRating(DoCommandFlag flags, Town *t, TownRatingCheckType type);


TileIndexDiff GetHouseNorthPart(HouseID &house);

Town *CalcClosestTownFromTile(TileIndex tile, uint threshold = UINT_MAX);

#define FOR_ALL_TOWNS_FROM(var, start) FOR_ALL_ITEMS_FROM(Town, town_index, var, start)
#define FOR_ALL_TOWNS(var) FOR_ALL_TOWNS_FROM(var, 0)

void ResetHouses();

void ClearTownHouse(Town *t, TileIndex tile);
void UpdateTownMaxPass(Town *t);
void UpdateTownRadius(Town *t);
void UpdateTownCargoes(Town *t);
void UpdateTownCargoTotal(Town *t);
void UpdateTownCargoBitmap();
CommandCost CheckIfAuthorityAllowsNewStation(TileIndex tile, DoCommandFlag flags);
Town *ClosestTownFromTile(TileIndex tile, uint threshold);
void ChangeTownRating(Town *t, int add, int max, DoCommandFlag flags);
HouseZonesBits GetTownRadiusGroup(const Town *t, TileIndex tile);
void SetTownRatingTestMode(bool mode);
uint GetMaskOfTownActions(int *nump, CompanyID cid, const Town *t);
bool GenerateTowns(TownLayout layout);
const CargoSpec *FindFirstCargoWithTownEffect(TownEffect effect);


/** Town actions of a company. */
enum TownActions {
	TACT_NONE             = 0x00, ///< Empty action set.

	TACT_ADVERTISE_SMALL  = 0x01, ///< Small advertising campaign.
	TACT_ADVERTISE_MEDIUM = 0x02, ///< Medium advertising campaign.
	TACT_ADVERTISE_LARGE  = 0x04, ///< Large advertising campaign.
	TACT_ROAD_REBUILD     = 0x08, ///< Rebuild the roads.
	TACT_BUILD_STATUE     = 0x10, ///< Build a statue.
	TACT_FUND_BUILDINGS   = 0x20, ///< Fund new buildings.
	TACT_BUY_RIGHTS       = 0x40, ///< Buy exclusive transport rights.
	TACT_BRIBE            = 0x80, ///< Try to bribe the council.

	TACT_COUNT            = 8,    ///< Number of available town actions.

	TACT_ADVERTISE        = TACT_ADVERTISE_SMALL | TACT_ADVERTISE_MEDIUM | TACT_ADVERTISE_LARGE, ///< All possible advertising actions.
	TACT_CONSTRUCTION     = TACT_ROAD_REBUILD | TACT_BUILD_STATUE | TACT_FUND_BUILDINGS,         ///< All possible construction actions.
	TACT_FUNDS            = TACT_BUY_RIGHTS | TACT_BRIBE,                                        ///< All possible funding actions.
	TACT_ALL              = TACT_ADVERTISE | TACT_CONSTRUCTION | TACT_FUNDS,                     ///< All possible actions.
};
DECLARE_ENUM_AS_BIT_SET(TownActions)

extern const byte _town_action_costs[TACT_COUNT];
extern TownID _new_town_id;

/**
 * Set the default name for a depot/waypoint
 * @tparam T The type/class to make a default name for
 * @param obj The object/instance we want to find the name for
 */
template <class T>
void MakeDefaultName(T *obj)
{
	/* We only want to set names if it hasn't been set before, or when we're calling from afterload. */
	assert(obj->name == NULL || obj->town_cn == UINT16_MAX);

	obj->town = ClosestTownFromTile(obj->xy, UINT_MAX);

	/* Find first unused number belonging to this town. This can never fail,
	 * as long as there can be at most 65535 waypoints/depots in total.
	 *
	 * This does 'n * m' search, but with 32bit 'used' bitmap, it needs at
	 * most 'n * (1 + ceil(m / 32))' steps (n - number of waypoints in pool,
	 * m - number of waypoints near this town).
	 * Usually, it needs only 'n' steps.
	 *
	 * If it wasn't using 'used' and 'idx', it would just search for increasing 'next',
	 * but this way it is faster */

	uint32 used = 0; // bitmap of used waypoint numbers, sliding window with 'next' as base
	uint32 next = 0; // first number in the bitmap
	uint32 idx  = 0; // index where we will stop
	uint32 cid  = 0; // current index, goes to T::GetPoolSize()-1, then wraps to 0

	do {
		T *lobj = T::GetIfValid(cid);

		/* check only valid waypoints... */
		if (lobj != NULL && obj != lobj) {
			/* only objects within the same city and with the same type */
			if (lobj->town == obj->town && lobj->IsOfType(obj)) {
				/* if lobj->town_cn < next, uint will overflow to '+inf' */
				uint i = (uint)lobj->town_cn - next;

				if (i < 32) {
					SetBit(used, i); // update bitmap
					if (i == 0) {
						/* shift bitmap while the lowest bit is '1';
						 * increase the base of the bitmap too */
						do {
							used >>= 1;
							next++;
						} while (HasBit(used, 0));
						/* when we are at 'idx' again at end of the loop and
						 * 'next' hasn't changed, then no object had town_cn == next,
						 * so we can safely use it */
						idx = cid;
					}
				}
			}
		}

		cid++;
		if (cid == T::GetPoolSize()) cid = 0; // wrap to zero...
	} while (cid != idx);

	obj->town_cn = (uint16)next; // set index...
}

extern uint32 _town_cargoes_accepted;

#endif /* TOWN_H */