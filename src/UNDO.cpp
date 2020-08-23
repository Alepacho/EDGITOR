#include "UNDO.h"
#include "CANVAS.h"
#include "COLOR.h"
#include "FUNCTIONS.h"
#include "VARIABLES.h"
#include "BRUSH.h"
#include "QUAD.h"
#include <memory>

uint16_t UNDO_UPDATE_LAYER = 0;
QUAD UNDO_UPDATE_REGION = QUAD::empty();

void push_undo_entry(UNDO_ENTRY undo_entry)
{
	// if we're back a few steps in the undo reel, we clear all the above undo steps.
	if (CURRENT_FILE_PTR->undo_pos > 0) {
		auto const old_end = CURRENT_FILE_PTR->undo_stack.end();
		auto const new_end = std::prev(old_end, CURRENT_FILE_PTR->undo_pos);
		CURRENT_FILE_PTR->undo_stack.erase(new_end, old_end);
	}

	// add the new undo
	CURRENT_FILE_PTR->undo_stack.push_back(std::move(undo_entry));
	CURRENT_FILE_PTR->undo_pos = 0;
}


void clear_undo_stack()
{
	CURRENT_FILE_PTR->undo_stack.clear();
	CURRENT_FILE_PTR->undo_pos = 0;
}


static void apply_undo_data(UNDO_ENTRY const* undo_entry, bool is_undo)
{
	auto const region = undo_entry->affected_region;
	auto const& data = is_undo? undo_entry->undo_pixels : undo_entry->redo_pixels;

	for (auto [x, y] : region)
	{
		const int index = (x - region.left) + (y - region.top) * region.width();
		set_pixel_layer(x, y, data[index], CURRENT_FILE_PTR->frames[undo_entry->affected_frame]->layers[(undo_entry->affected_layer)]);
	}

	CANVAS_UPDATE = true;
	//CURRENT_LAYER = undo_entry->affected_layer;
	//UNDO_UPDATE_LAYER = undo_entry->affected_layer;
	UNDO_UPDATE_REGION = region;
}


void undo()
{
	if (CURRENT_FILE_PTR->undo_pos >= CURRENT_FILE_PTR->undo_stack.size()) {
		return;
	}

	auto const undo_entry = &CURRENT_FILE_PTR->undo_stack[CURRENT_FILE_PTR->undo_stack.size() - CURRENT_FILE_PTR->undo_pos - 1];
	apply_undo_data(undo_entry, true);

	CURRENT_FILE_PTR->undo_pos++;
}


void redo()
{
	if (CURRENT_FILE_PTR->undo_pos == 0) {
		return;
	}

	CURRENT_FILE_PTR->undo_pos--;

	auto const undo_entry = &CURRENT_FILE_PTR->undo_stack[CURRENT_FILE_PTR->undo_stack.size() - CURRENT_FILE_PTR->undo_pos - 1];
	apply_undo_data(undo_entry, false);
}
