/* $Id: cargopacket.cpp 23640 2011-12-20 17:57:56Z truebrain $ */

/*
 * This file is part of OpenTTD.
 * OpenTTD is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * OpenTTD is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with OpenTTD. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file cargopacket.cpp Implementation of the cargo packets. */

#include "stdafx.h"
#include "core/pool_func.hpp"
#include "economy_base.h"
#include "station_base.h"
#include "cargodest_func.h"
#include "cargodest_base.h"
#include "settings_type.h"

/* Initialize the cargopacket-pool */
CargoPacketPool _cargopacket_pool("CargoPacket");
INSTANTIATE_POOL_METHODS(CargoPacket)

/**
 * Create a new packet for savegame loading.
 */
CargoPacket::CargoPacket()
{
	this->source_type = ST_INDUSTRY;
	this->source_id   = INVALID_SOURCE;
	this->dest_xy     = INVALID_TILE;
	this->dest_id     = INVALID_SOURCE;
	this->dest_type   = ST_INDUSTRY;
	this->flags       = 0;
	this->next_order  = INVALID_ORDER;
	this->next_station = INVALID_STATION;
}

/**
 * Creates a new cargo packet.
 * @param source      Source station of the packet.
 * @param source_xy   Source location of the packet.
 * @param count       Number of cargo entities to put in this packet.
 * @param source_type 'Type' of source the packet comes from (for subsidies).
 * @param source_id   Actual source of the packet (for subsidies).
 * @param dest_xy     Destination location of the packet.
 * @param dest_type   'Type' of the destination.
 * @param dest_id     Actual destination of the packet.
 * @param next_order  Desired next hop of the packet.
 * @param next_station Station to unload the packet next.
 * @param flags       Routing flags of the packet.
 * @pre count != 0
 * @note We have to zero memory ourselves here because we are using a 'new'
 * that, in contrary to all other pools, does not memset to 0.
 */
CargoPacket::CargoPacket(StationID source, TileIndex source_xy, uint16 count, SourceType source_type, SourceID source_id, TileIndex dest_xy, SourceType dest_type, SourceID dest_id, OrderID next_order, StationID next_station, byte flags) :
	feeder_share(0),
	count(count),
	days_in_transit(0),
	source_id(source_id),
	source(source),
	source_xy(source_xy),
	loaded_at_xy(0),
	dest_xy(dest_xy),
	dest_id(dest_id),
	flags(flags),
	next_order(next_order),
	next_station(next_station)
{
	assert(count != 0);
	this->source_type  = source_type;
	this->dest_type    = dest_type;
}

/**
 * Creates a new cargo packet. Initializes the fields that cannot be changed later.
 * Used when loading or splitting packets.
 * @param count           Number of cargo entities to put in this packet.
 * @param days_in_transit Number of days the cargo has been in transit.
 * @param source          Station the cargo was initially loaded.
 * @param source_xy       Station location the cargo was initially loaded.
 * @param loaded_at_xy    Location the cargo was loaded last.
 * @param feeder_share    Feeder share the packet has already accumulated.
 * @param source_type     'Type' of source the packet comes from (for subsidies).
 * @param source_id       Actual source of the packet (for subsidies).
 * @param dest_xy         Destination location of the packet.
 * @param dest_type       'Type' of the destination.
 * @param dest_id         Actual destination of the packet.
 * @param next_order      Desired next hop of the packet.
 * @param next_station Station to unload the packet next.
 * @param flags           Routing flags of the packet.
 * @note We have to zero memory ourselves here because we are using a 'new'
 * that, in contrary to all other pools, does not memset to 0.
 */
CargoPacket::CargoPacket(uint16 count, byte days_in_transit, StationID source, TileIndex source_xy, TileIndex loaded_at_xy, Money feeder_share, SourceType source_type, SourceID source_id, TileIndex dest_xy, SourceType dest_type, SourceID dest_id, OrderID next_order, StationID next_station, byte flags) :
		feeder_share(feeder_share),
		count(count),
		days_in_transit(days_in_transit),
		source_id(source_id),
		source(source),
		source_xy(source_xy),
		loaded_at_xy(loaded_at_xy),
		dest_xy(dest_xy),
		dest_id(dest_id),
		flags(flags),
		next_order(next_order),
		next_station(next_station)
{
	assert(count != 0);
	this->source_type = source_type;
	this->dest_type   = dest_type;
}

/**
 * Split this packet in two and return the split off part.
 * @param new_size Size of the remaining part.
 * @return Split off part, or NULL if no packet could be allocated!
 */
inline CargoPacket *CargoPacket::Split(uint new_size)
{
	if (!CargoPacket::CanAllocateItem()) return NULL;

	Money fs = this->feeder_share * new_size / static_cast<uint>(this->count);
	CargoPacket *cp_new = new CargoPacket(new_size, this->days_in_transit, this->source, this->source_xy, this->loaded_at_xy, fs, this->source_type, this->source_id, this->dest_xy, this->dest_type, this->dest_id, this->next_order, this->next_station, this->flags);
	this->feeder_share -= fs;
	this->count -= new_size;
	return cp_new;
}

/**
 * Merge another packet into this one.
 * @param cp Packet to be merged in.
 */
inline void CargoPacket::Merge(CargoPacket *cp)
{
	this->count += cp->count;
	this->feeder_share += cp->feeder_share;
	delete cp;
}

/**
 * Invalidates (sets source_id to INVALID_SOURCE) all cargo packets from given source.
 * @param src_type Type of source.
 * @param src Index of source.
 */
/* static */ void CargoPacket::InvalidateAllFrom(SourceType src_type, SourceID src)
{
	/* Clear next hop of those packets that loose their destination. */
	StationCargoList::InvalidateAllTo(src_type, src);

	CargoPacket *cp;
	FOR_ALL_CARGOPACKETS(cp) {
		if (cp->source_type == src_type && cp->source_id == src) cp->source_id = INVALID_SOURCE;
		if (cp->dest_type == src_type && cp->dest_id == src) {
			cp->dest_id = INVALID_SOURCE;
			cp->dest_xy = INVALID_TILE;
		}
	}
}

/**
 * Invalidates (sets source to INVALID_STATION) all cargo packets from given station.
 * @param sid Station that gets removed.
 */
/* static */ void CargoPacket::InvalidateAllFrom(StationID sid)
{
	CargoPacket *cp;
	FOR_ALL_CARGOPACKETS(cp) {
		if (cp->source == sid) cp->source = INVALID_STATION;
		if (cp->next_station == sid) cp->next_station = INVALID_STATION;
	}
}

/*
 *
 * Cargo list implementation
 *
 */

/**
 * Destroy the cargolist ("frees" all cargo packets).
 */
template <class Tinst>
CargoList<Tinst>::~CargoList()
{
	for (Iterator it(this->packets.begin()); it != this->packets.end(); ++it) {
		delete *it;
	}
}

/**
 * Empty the cargo list, but don't free the cargo packets;
 * the cargo packets are cleaned by CargoPacket's CleanPool.
 */
template <class Tinst>
void CargoList<Tinst>::OnCleanPool()
{
	this->packets.clear();
}

/**
 * Update the cached values to reflect the removal of this packet.
 * Decreases count and days_in_transit.
 * @param cp Packet to be removed from cache.
 */
template <class Tinst>
void CargoList<Tinst>::RemoveFromCache(const CargoPacket *cp)
{
	this->count                 -= cp->count;
	this->cargo_days_in_transit -= cp->days_in_transit * cp->count;
}

/**
 * Update the cache to reflect adding of this packet.
 * Increases count and days_in_transit.
 * @param cp New packet to be inserted.
 */
template <class Tinst>
void CargoList<Tinst>::AddToCache(const CargoPacket *cp)
{
	this->count                 += cp->count;
	this->cargo_days_in_transit += cp->days_in_transit * cp->count;
}

/**
 * Appends the given cargo packet. Tries to merge it with another one in the
 * packets list. If no fitting packet is found, appends it.
 * @warning After appending this packet may not exist anymore!
 * @note Do not use the cargo packet anymore after it has been appended to this CargoList!
 * @param cp Cargo packet to add.
 * @pre cp != NULL
 */
template <class Tinst>
void CargoList<Tinst>::Append(CargoPacket *cp)
{
	assert(cp != NULL);
	static_cast<Tinst *>(this)->AddToCache(cp);

	for (List::reverse_iterator it(this->packets.rbegin()); it != this->packets.rend(); it++) {
		CargoPacket *icp = *it;
		if (Tinst::AreMergable(icp, cp) && icp->count + cp->count <= CargoPacket::MAX_COUNT) {
			icp->Merge(cp);
			return;
		}
	}

	/* The packet could not be merged with another one */
	this->packets.push_back(cp);
}

/**
 * Truncates the cargo in this list to the given amount. It leaves the
 * first count cargo entities and removes the rest.
 * @param max_remaining Maximum amount of entities to be in the list after the command.
 */
template <class Tinst>
void CargoList<Tinst>::Truncate(uint max_remaining)
{
	for (Iterator it(packets.begin()); it != packets.end(); /* done during loop*/) {
		CargoPacket *cp = *it;
		if (max_remaining == 0) {
			/* Nothing should remain, just remove the packets. */
			it = this->packets.erase(it);
			static_cast<Tinst *>(this)->RemoveFromCache(cp);
			delete cp;
			continue;
		}

		uint local_count = cp->count;
		if (local_count > max_remaining) {
			uint diff = local_count - max_remaining;
			this->count -= diff;
			this->cargo_days_in_transit -= cp->days_in_transit * diff;
			static_cast<Tinst *>(this)->RemoveFromCacheLocal(cp, diff);
			cp->count = max_remaining;
			max_remaining = 0;
		} else {
			max_remaining -= local_count;
		}
		++it;
	}
}

/**
 * Moves the given amount of cargo to another list.
 * Depending on the value of mta the side effects of this function differ:
 *  - MTA_FINAL_DELIVERY: Destroys the packets that do not originate from a specific station.
 *  - MTA_CARGO_LOAD:     Sets the loaded_at_xy value of the moved packets.
 *  - MTA_TRANSFER:       Just move without side effects.
 *  - MTA_UNLOAD:         Just move without side effects.
 *  - MTA_NO_ACTION:      Does nothing for packets without destination, otherwise either like MTA_TRANSFER or MTA_FINAL_DELIVERY.
 * @param dest  Destination to move the cargo to.
 * @param max_move Amount of cargo entities to move.
 * @param mta   How to handle the moving (side effects).
 * @param st    Station ID where we are loading/unloading or STATION_INVALID for move from vehicle to vehicle.
 * @param payment The payment helper.
 * @param cur_order The current order of the loading vehicle.
 * @param did_transfer Set to true if some cargo was transfered.
 *
 * @pre mta == MTA_FINAL_DELIVERY || dest != NULL
 * @pre mta == MTA_UNLOAD || mta == MTA_CARGO_LOAD || payment != NULL
 * @pre st != INVALID_STATION || (mta != MTA_CARGO_LOAD && payment == NULL)
 * @return True if there are still packets that might be moved from this cargo list.
 */
template <class Tinst>
template <class Tother_inst>
bool CargoList<Tinst>::MoveTo(Tother_inst *dest, uint max_move, MoveToAction mta, CargoPayment *payment, StationID st, OrderID cur_order, CargoID cid, bool *did_transfer)
{
	assert(mta == MTA_FINAL_DELIVERY || dest != NULL);
	assert(mta == MTA_UNLOAD || mta == MTA_CARGO_LOAD || payment != NULL);
	assert(st != INVALID_STATION || (mta != MTA_CARGO_LOAD && payment == NULL));

restart:;
	Iterator it(this->packets.begin());
	while (it != this->packets.end() && max_move > 0) {
		CargoPacket *cp = *it;
		MoveToAction cp_mta = mta;
		OrderID current_next_order = cp->NextHop();
		StationID current_next_unload = cp->NextStation();

		if (cp_mta == MTA_CARGO_LOAD) {
			/* Invalid next hop but valid destination? Recompute next hop. */
			if (current_next_order == INVALID_ORDER && cp->DestinationID() != INVALID_SOURCE) {
				if (!this->UpdateCargoNextHop(cp, Station::Get(st), cid)) {
					/* Failed to find destination, drop packet. */
					it = this->packets.erase(it);
					continue;
				}
				current_next_order = cp->NextHop();
				current_next_unload = cp->NextStation();
			}

			/* Loading and not for the current vehicle? Skip. */
			if (current_next_order != cur_order) {
				++it;
				continue;
			}
		}

		/* Has this packet a destination and are we unloading to a station (not autoreplace)? */
		if (cp->DestinationID() != INVALID_SOURCE && cp_mta != MTA_CARGO_LOAD && payment != NULL) {
			/* Not forced unload and not for unloading at this station? Skip the packet. */
			if (cp_mta != MTA_UNLOAD && cp->NextStation() != INVALID_STATION && cp->NextStation() != st) {
				++it;
				continue;
			}

			Station *station = Station::Get(st);

			bool found;
			StationID next_unload;
			RouteLink *link = FindRouteLinkForCargo(station, cid, cp, &next_unload, cur_order, &found);
			if (!found) {
				/* Sorry, link to destination vanished, make cargo disappear. */
				static_cast<Tinst *>(this)->RemoveFromCache(cp);
				delete cp;
				it = this->packets.erase(it);
				continue;
			}

			if (link != NULL) {
				/* Not final destination. */
				if (link->GetOriginOrderId() == cur_order && cp_mta != MTA_UNLOAD) {
					/* Cargo should stay on the vehicle and not forced unloading? Skip. */
					++it;
					continue;
				}
				/* Force transfer and update next hop. */
				cp_mta = MTA_TRANSFER;
				current_next_order = link->GetOriginOrderId();
				current_next_unload = next_unload;
			} else {
				/* Final destination, deliver. */
				cp_mta = MTA_FINAL_DELIVERY;
			}
		} else if (cp_mta == MTA_NO_ACTION || (cp->source == st && cp_mta == MTA_FINAL_DELIVERY)) {
			/* Skip cargo that is not accepted or originated from this station. */
			++it;
			continue;
		}

		if (did_transfer != NULL && cp_mta == MTA_TRANSFER) *did_transfer = true;

		if (cp->count <= max_move) {
			/* Can move the complete packet */
			max_move -= cp->count;
			it = this->packets.erase(it);
			static_cast<Tinst *>(this)->RemoveFromCache(cp);
			cp->next_order = current_next_order;
			cp->next_station = current_next_unload;
			switch (cp_mta) {
				case MTA_FINAL_DELIVERY:
					payment->PayFinalDelivery(cp, cp->count);
					delete cp;
					continue; // of the loop

				case MTA_CARGO_LOAD:
					cp->loaded_at_xy = Station::Get(st)->xy;
					break;

				case MTA_TRANSFER:
					cp->feeder_share += payment->PayTransfer(cp, cp->count);
					break;

				default:
					break;
			}
			dest->Append(cp);
			continue;
		}

		/* Can move only part of the packet */
		if (cp_mta == MTA_FINAL_DELIVERY) {
			/* Final delivery doesn't need package splitting. */
			payment->PayFinalDelivery(cp, max_move);

			/* Remove the delivered data from the cache */
			uint left = cp->count - max_move;
			cp->count = max_move;
			static_cast<Tinst *>(this)->RemoveFromCache(cp);

			/* Final delivery payment pays the feeder share, so we have to
			 * reset that so it is not 'shown' twice for partial unloads. */
			cp->feeder_share = 0;
			cp->count = left;
		} else {
			/* But... the rest needs package splitting. */
			CargoPacket *cp_new = cp->Split(max_move);

			/* We could not allocate a CargoPacket? Is the map that full? */
			if (cp_new == NULL) return false;

			static_cast<Tinst *>(this)->RemoveFromCache(cp_new); // this reflects the changes in cp.
			cp_new->next_order = current_next_order;
			cp_new->next_station = current_next_unload;

			if (cp_mta == MTA_TRANSFER) {
				/* Add the feeder share before inserting in dest. */
				cp_new->feeder_share += payment->PayTransfer(cp_new, max_move);
			} else if (cp_mta == MTA_CARGO_LOAD) {
				cp_new->loaded_at_xy = Station::Get(st)->xy;
			}

			dest->Append(cp_new);
		}

		max_move = 0;
	}

	if (max_move > 0 && mta == MTA_CARGO_LOAD && cur_order != INVALID_ORDER && Station::Get(st)->goods[cid].cargo.CountForNextHop(INVALID_ORDER) > 0) {
		/* We loaded all packets for the next hop, now load all packets without destination. */
		cur_order = INVALID_ORDER;
		goto restart;
	}

	return it != packets.end();
}

/** Invalidates the cached data and rebuilds it. */
template <class Tinst>
void CargoList<Tinst>::InvalidateCache()
{
	this->count = 0;
	this->cargo_days_in_transit = 0;

	for (ConstIterator it(this->packets.begin()); it != this->packets.end(); it++) {
		static_cast<Tinst *>(this)->AddToCache(*it);
	}
}

/**
 * Update the cached values to reflect the removal of this packet.
 * Decreases count, feeder share and days_in_transit.
 * @param cp Packet to be removed from cache.
 */
void VehicleCargoList::RemoveFromCache(const CargoPacket *cp)
{
	this->feeder_share -= cp->feeder_share;
	this->Parent::RemoveFromCache(cp);
}

/**
 * Update the cache to reflect adding of this packet.
 * Increases count, feeder share and days_in_transit.
 * @param cp New packet to be inserted.
 */
void VehicleCargoList::AddToCache(const CargoPacket *cp)
{
	this->feeder_share += cp->feeder_share;
	this->Parent::AddToCache(cp);
}

/**
 * Ages the all cargo in this list.
 */
void VehicleCargoList::AgeCargo()
{
	for (ConstIterator it(this->packets.begin()); it != this->packets.end(); it++) {
		CargoPacket *cp = *it;
		/* If we're at the maximum, then we can't increase no more. */
		if (cp->days_in_transit == 0xFF) continue;

		cp->days_in_transit++;
		this->cargo_days_in_transit += cp->count;
	}
}

/** Invalidates the cached data and rebuild it. */
void VehicleCargoList::InvalidateCache()
{
	this->feeder_share = 0;
	this->Parent::InvalidateCache();
}

/** Invalidate next unload station of all cargo packets. */
void VehicleCargoList::InvalidateNextStation()
{
	for (VehicleCargoList::ConstIterator it = this->packets.begin(); it != this->packets.end(); ++it) {
		(*it)->next_station = INVALID_STATION;
	}
}

/**
 * Update the local next-hop count cache.
 * @param cp Packet the be removed.
 * @param amount Cargo amount to be removed.
 */
void StationCargoList::RemoveFromCacheLocal(const CargoPacket *cp, uint amount)
{
	this->order_cache[cp->next_order] -= amount;
	if (this->order_cache[cp->next_order] == 0) this->order_cache.erase(cp->next_order);
}

/**
 * Update the cached values to reflect the removal of this packet.
 * Decreases count and days_in_transit.
 * @param cp Packet to be removed from cache.
 */
void StationCargoList::RemoveFromCache(const CargoPacket *cp)
{
	this->RemoveFromCacheLocal(cp, cp->count);
	this->Parent::RemoveFromCache(cp);
}

/**
 * Update the cache to reflect adding of this packet.
 * Increases count and days_in_transit.
 * @param cp New packet to be inserted.
 */
void StationCargoList::AddToCache(const CargoPacket *cp)
{
	this->order_cache[cp->next_order] += cp->count;
	this->Parent::AddToCache(cp);
}

/** Invalidates the cached data and rebuild it. */
void StationCargoList::InvalidateCache()
{
	this->order_cache.clear();
	this->Parent::InvalidateCache();
}

/**
 * Recompute the desired next hop of a cargo packet.
 * @param cp  Cargo packet to update.
 * @param st  Station of  this list.
 * @param cid Cargo type of this list.
 * @return False if the packet was deleted, true otherwise.
 */
bool StationCargoList::UpdateCargoNextHop(CargoPacket *cp, Station *st, CargoID cid)
{
	StationID next_unload;
	RouteLink *l = FindRouteLinkForCargo(st, cid, cp, &next_unload);

	if (l == NULL) {
		/* No link to destination, drop packet. */
		this->RemoveFromCache(cp);
		delete cp;
		return false;
	}

	/* Update next hop info. */
	this->RemoveFromCache(cp);
	cp->next_station = next_unload;
	cp->next_order = l->GetOriginOrderId();
	this->AddToCache(cp);

	return true;
}

/**
 * Recompute the desired next hop of all cargo packets.
 * @param st  Station of this list.
 * @param cid Cargo type of this list.
 */
void StationCargoList::UpdateCargoNextHop(Station *st, CargoID cid)
{
	uint count = 0;
	StationCargoList::Iterator iter;
	for (iter = this->packets.begin(); count < this->next_start + _settings_game.economy.cargodest.route_recalc_chunk && iter != this->packets.end(); count++) {
		if (count < this->next_start) continue;
		if ((*iter)->DestinationID() != INVALID_SOURCE) {
			if (this->UpdateCargoNextHop(*iter, st, cid)) {
				++iter;
			} else {
				iter = this->packets.erase(iter);
			}
		} else {
			++iter;
		}
	}

	/* Update start counter for next loop. */
	this->next_start = count;
	if (this->next_start >= this->packets.size()) this->next_start = 0;
}

/**
 * Invalidates the next hop info of all cargo packets with a given next order or unload station.
 * @param order Next order to invalidate.
 * @param st_unload Unload station to invalidate.
 */
/* static */ void StationCargoList::InvalidateAllTo(OrderID order, StationID st_unload)
{
	Station *st;
	FOR_ALL_STATIONS(st) {
		for (CargoID cid = 0; cid < NUM_CARGO; cid++) {
			for (StationCargoList::Iterator it = st->goods[cid].cargo.packets.begin(); it != st->goods[cid].cargo.packets.end(); ++it) {
				CargoPacket *cp = *it;
				if (cp->next_order == order || cp->next_station == st_unload) {
					/* Invalidate both order and unload station as both likely
					 * don't make sense anymore. */
					st->goods[cid].cargo.RemoveFromCache(cp);
					cp->next_order = INVALID_ORDER;
					cp->next_station = INVALID_STATION;
					st->goods[cid].cargo.AddToCache(cp);
				}
			}
		}
	}
}

/**
 * Invalidates the next hop info of all cargo packets for a given destination.
 * @param order Next order to invalidate.
 */
/* static */ void StationCargoList::InvalidateAllTo(SourceType type, SourceID dest)
{
	Station *st;
	FOR_ALL_STATIONS(st) {
		for (CargoID cid = 0; cid < NUM_CARGO; cid++) {
			for (StationCargoList::Iterator it = st->goods[cid].cargo.packets.begin(); it != st->goods[cid].cargo.packets.end(); ++it) {
				CargoPacket *cp = *it;
				if (cp->dest_id == dest && cp->dest_type == type) {
					/* Invalidate both next order and unload station as we
					 * want the packets to be not routed anymore. */
					st->goods[cid].cargo.RemoveFromCache(cp);
					cp->next_order = INVALID_ORDER;
					cp->next_station = INVALID_STATION;
					st->goods[cid].cargo.AddToCache(cp);
				}
			}
		}
	}
}

/*
 * We have to instantiate everything we want to be usable.
 */
template class CargoList<VehicleCargoList>;
template class CargoList<StationCargoList>;

/** Autoreplace Vehicle -> Vehicle 'transfer'. */
template bool CargoList<VehicleCargoList>::MoveTo(VehicleCargoList *, uint max_move, MoveToAction mta, CargoPayment *payment, StationID st, OrderID cur_order, CargoID cid, bool *did_transfer);
/** Cargo unloading at a station. */
template bool CargoList<VehicleCargoList>::MoveTo(StationCargoList *, uint max_move, MoveToAction mta, CargoPayment *payment, StationID st, OrderID cur_order, CargoID cid, bool *did_transfer);
/** Cargo loading at a station. */
template bool CargoList<StationCargoList>::MoveTo(VehicleCargoList *, uint max_move, MoveToAction mta, CargoPayment *payment, StationID st, OrderID cur_order, CargoID cid, bool *did_transfer);
