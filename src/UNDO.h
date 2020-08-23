#pragma once

#include <vector>
#include <cstdint>

#include "COLOR.h"
#include "QUAD.h"

struct UNDO_ENTRY
{
	QUAD affected_region;
	uint16_t affected_layer;
	uint16_t affected_frame;

	std::vector<COLOR> undo_pixels;
	std::vector<COLOR> redo_pixels;

	UNDO_ENTRY(QUAD _rect, uint16_t _layer, uint16_t _frame)
		: affected_region{ _rect }
		, affected_layer{ _layer }
		, affected_frame{ _frame }
		, undo_pixels(_rect.width()*_rect.height())
		, redo_pixels(_rect.width()*_rect.height())
	{
	}

	void set(uint16_t xx, uint16_t yy, COLOR prev_col, COLOR new_col)
	{
		int index = xx + yy * this->affected_region.width();
		this->undo_pixels[index] = prev_col;
		this->redo_pixels[index] = new_col;
	}
};


void push_undo_entry(UNDO_ENTRY undo_entry);
void clear_undo_stack();

// NOTE: these names are way too short but I couldn't think of anything better
void undo();
void redo();


extern uint16_t UNDO_UPDATE_LAYER;
extern QUAD UNDO_UPDATE_REGION;
