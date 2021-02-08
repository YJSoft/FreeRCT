/*
 * This file is part of FreeRCT.
 * FreeRCT is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 2.
 * FreeRCT is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details. You should have received a copy of the GNU General Public License along with FreeRCT. If not, see <http://www.gnu.org/licenses/>.
 */

/** @file gentle_thrill_type.cpp Implementation of gentle rides and thrill rides. */

#include "stdafx.h"
#include "map.h"
#include "shop_type.h"
#include "person.h"
#include "people.h"
#include "fileio.h"
#include "gentle_thrill_ride_type.h"
#include "generated/gentle_thrill_rides_strings.cpp"

GentleThrillRideType::GentleThrillRideType() : FixedRideType(RTK_GENTLE /* Kind will be set later in Load(). */)
{
	capacity.number_of_batches = 0;
	capacity.guests_per_batch = 0;
}

GentleThrillRideType::~GentleThrillRideType()
{
	/* Images and texts are handled by the sprite collector, no need to release its memory here. */
}

RideInstance *GentleThrillRideType::CreateInstance() const
{
	return new GentleThrillRideInstance(this);
}

/**
 * Load a type of gentle or thrill ride from the RCD file.
 * @param rcd_file Rcd file being loaded.
 * @param sprites Already loaded sprites.
 * @param texts Already loaded texts.
 * @return Loading was successful.
 * @todo #GentleThrillRideType::Load and #ShopType::Load share a lot of similar code. Pull it out into a common function in #FixedRideType.
 */
bool GentleThrillRideType::Load(RcdFileReader *rcd_file, const ImageMap &sprites, const TextMap &texts)
{
	if (rcd_file->version != 2 || rcd_file->size < 3) return false;
	this->kind = rcd_file->GetUInt8() ? RTK_THRILL : RTK_GENTLE;
	this->width_x = rcd_file->GetUInt8();
	this->width_y = rcd_file->GetUInt8();
	if (this->width_x < 1 || this->width_y < 1) return false;
	if (static_cast<int>(rcd_file->size) != 79 + (this->width_x * this->width_y)) return false;
	
	this->heights.reset(new int8[this->width_x * this->width_y]);
	for (int8 x = 0; x < this->width_x; ++x) {
		for (int8 y = 0; y < this->width_y; ++y) {
			this->heights[x * this->width_y + y] = rcd_file->GetUInt8();
		}
	}

	animation_idle = _sprite_manager.GetFrameSet(rcd_file->GetUInt32());
	animation_starting = _sprite_manager.GetTimedAnimation(rcd_file->GetUInt32());
	animation_working = _sprite_manager.GetTimedAnimation(rcd_file->GetUInt32());
	animation_stopping = _sprite_manager.GetTimedAnimation(rcd_file->GetUInt32());
	for (int i = 0; i < 4; i++) {
		ImageData *view;
		if (!LoadSpriteFromFile(rcd_file, sprites, &view)) return false;
		this->previews[i] = view;
	}

	for (uint i = 0; i < 3; i++) {
		uint32 recolour = rcd_file->GetUInt32();
		this->recolours.Set(i, RecolourEntry(recolour));
	}
	this->item_cost[0] = rcd_file->GetInt32(); // Entrance fee.
	this->item_cost[1] = 0;                    // Unused.
	this->monthly_cost = rcd_file->GetInt32();
	this->monthly_open_cost = rcd_file->GetInt32();
	this->capacity.number_of_batches = rcd_file->GetUInt32();
	this->capacity.guests_per_batch = rcd_file->GetUInt32();
	this->idle_duration = rcd_file->GetUInt32();
	this->working_duration = rcd_file->GetUInt32();

	/* Check that all animations fit to the ride. */
	if (animation_idle == nullptr || animation_starting == nullptr ||
			animation_working == nullptr || animation_stopping == nullptr ||
			animation_idle->width_x != this->width_x || animation_idle->width_y != this->width_y) {
		return false;
	}
	int working_animation_min_length = 0;
	for (int i = 0; i < animation_starting->frames; ++i) {
		if (animation_starting->views[i]->width_x != this->width_x || animation_starting->views[i]->width_y != this->width_y) return false;
		working_animation_min_length += animation_starting->durations[i];
	}
	for (int i = 0; i < animation_working->frames; ++i) {
		if (animation_working->views[i]->width_x != this->width_x || animation_working->views[i]->width_y != this->width_y) return false;
		working_animation_min_length += animation_working->durations[i];
	}
	for (int i = 0; i < animation_stopping->frames; ++i) {
		if (animation_stopping->views[i]->width_x != this->width_x || animation_stopping->views[i]->width_y != this->width_y) return false;
		working_animation_min_length += animation_stopping->durations[i];
	}
	if (working_animation_min_length > this->working_duration) return false;
	if (capacity.number_of_batches < 1 || capacity.guests_per_batch < 1) return false;
	if (capacity.number_of_batches > 1 && working_animation_min_length != 0) return false;

	TextData *text_data;
	if (!LoadTextFromFile(rcd_file, texts, &text_data)) return false;
	StringID base = _language.RegisterStrings(*text_data, _gentle_thrill_rides_strings_table);
	this->SetupStrings(text_data, base,
			STR_GENERIC_GENTLE_THRILL_RIDES_START, GENTLE_THRILL_RIDES_STRING_TABLE_END,
			GENTLE_THRILL_RIDES_NAME_TYPE, GENTLE_THRILL_RIDES_DESCRIPTION_TYPE);
	return true;
}

FixedRideType::RideCapacity GentleThrillRideType::GetRideCapacity() const
{
	return capacity;
}

const StringID *GentleThrillRideType::GetInstanceNames() const
{
	static const StringID names[] = {GENTLE_THRILL_RIDES_NAME_INSTANCE1, GENTLE_THRILL_RIDES_NAME_INSTANCE2, STR_INVALID};
	return names;
}

/**
 * Constructor of a gentle or thrill ride.
 * @param type Kind of ride.
 */
GentleThrillRideInstance::GentleThrillRideInstance(const GentleThrillRideType *type) : FixedRideInstance(type)
{
	/* Nothing to do currently. */
}

GentleThrillRideInstance::~GentleThrillRideInstance()
{
	/* Nothing to do currently. */
}

/**
 * Get the ride type of the ride.
 * @return The gentle or thrill ride type of the ride.
 */
const GentleThrillRideType *GentleThrillRideInstance::GetGentleThrillRideType() const
{
	assert(this->type->kind == RTK_GENTLE || this->type->kind == RTK_THRILL);
	return static_cast<const GentleThrillRideType *>(this->type);
}

uint8 GentleThrillRideInstance::GetEntranceDirections(const XYZPoint16 &vox) const
{
	return SHF_ENTRANCE_BITS; // \todo Ride entrances are not implemented yet.
}

RideEntryResult GentleThrillRideInstance::EnterRide(int guest, TileEdge entry)
{
	return RER_REFUSED; // \todo Ride entrances are not implemented yet.
}

XYZPoint32 GentleThrillRideInstance::GetExit(int guest, TileEdge entry_edge)
{
	NOT_REACHED(); // \todo Ride exits are not implemented yet.
}

void GentleThrillRideInstance::Load(Loader &ldr)
{
	this->FixedRideInstance::Load(ldr);
	/* Nothing to do here currently. */
}

void GentleThrillRideInstance::Save(Saver &svr)
{
	this->FixedRideInstance::Save(svr);
	/* Nothing to do here currently. */
}