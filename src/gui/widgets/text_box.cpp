/* $Id$ */
/*
   copyright (C) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/widgets/text_box.hpp"

#include "font.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "game_preferences.hpp"

#include <numeric>

#define DBG_G LOG_STREAM_INDENT(debug, gui)
#define LOG_G LOG_STREAM_INDENT(info, gui)
#define WRN_G LOG_STREAM_INDENT(warn, gui)
#define ERR_G LOG_STREAM_INDENT(err, gui)

#define DBG_G_D LOG_STREAM_INDENT(debug, gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, gui_draw)

#define DBG_G_E LOG_STREAM_INDENT(debug, gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, gui_event)

#define DBG_G_P LOG_STREAM_INDENT(debug, gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, gui_parse)
#define WRN_G_P LOG_STREAM_INDENT(warn, gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, gui_parse)


namespace gui2 {

static surface render_text(const std::string& text, unsigned font_size)
{
	static SDL_Color col = {0, 0, 0, 0};
	return font::get_rendered_text(text, font_size, col, TTF_STYLE_NORMAL);
}

//! Helper function for text more efficient as set_text.
//! Inserts a character at the cursor.
void ttext_box::insert_char(Uint16 unicode)
{
	if(sel_len() > 0) {
		delete_selection();
	}

	// Determine the width of the new character.
	std::string tmp_text;
	tmp_text.insert(tmp_text.begin(), unicode);

	surface surf = render_text(tmp_text, config()->text_font_size);
	assert(surf);
	const unsigned width = surf->w;

	// Insert the char in the buffer, we need to assume it's a wide string.
	wide_string tmp = utils::string_to_wstring(text());
	tmp.insert(tmp.begin() + sel_start(), unicode);
	text() = utils::wstring_to_string(tmp);

	// Update the widths.
	character_offset_.insert(character_offset_.begin() + sel_start(), width);
	if(sel_start() != 0) {
		character_offset_[sel_start()] += character_offset_[sel_start() - 1]; 
	}

	++sel_start();
	for(size_t i = sel_start(); i < character_offset_.size(); ++i) {
		character_offset_[i] += width;
	}

	set_cursor(sel_start(), false);
	set_canvas_text();
	set_dirty();
}

//! Deletes the character.
//!
//! @param before_cursor     If true it deletes the character before the cursor
//!                          (backspace) else the character after the cursor
//!                          (delete). 
void ttext_box::delete_char(const bool before_cursor)
{
	if(before_cursor) {
		--sel_start();
		set_cursor(sel_start(), false);
	}

	sel_len() = 1;

	delete_selection();
}

//! Deletes the current selection.
void ttext_box::delete_selection()
{
	assert(sel_len() != 0);

	// If we have a negative range change it to a positive range.
	// This makes the rest of the algoritms easier.
	if(sel_len() < 0) {
		sel_len() = - sel_len();
		sel_start() -= sel_len();
		set_cursor(sel_start(), false);
	}

	// Update the text, we need to assume it's a wide string.
	wide_string tmp = utils::string_to_wstring(text());
	tmp.erase(tmp.begin() + sel_start(), tmp.begin() + sel_start() + sel_len());
	text() = utils::wstring_to_string(tmp);

	// Update the offsets
	const unsigned width = character_offset_[sel_start() + sel_len() - 1] - 
		(sel_start() ? character_offset_[sel_start() - 1] : 0);

	character_offset_.erase(character_offset_.begin() + sel_start(), character_offset_.begin() + sel_start() + sel_len());
	for(size_t i = sel_start(); i < character_offset_.size(); ++i) {
		character_offset_[i] -= width;
	}

	sel_len() = 0;
	set_canvas_text();
	set_dirty();
}

//! Inherited from tcontrol.
void ttext_box::set_canvas_text()
{
	foreach(tcanvas& tmp, canvas()) {
		tmp.set_variable("text", variant(text()));
		//FIXME add selection info.
		if(text().empty() || sel_start() == 0) {
			tmp.set_variable("cursor_offset", variant(0));
		} else {
			tmp.set_variable("cursor_offset", variant(character_offset_[sel_start() -1] + 0));
		}
	}
}

//! Calculates the offsets of all chars.
void ttext_box::calculate_char_offset()
{
	// If the text is set before the config is loaded do it ourselves.
	// This isn't really clean solution, maybe fix it later.
	if(!config()) {
		load_config();
	}
	assert(config());
	character_offset_.clear();

	std::string rendered_text;
	const unsigned font_size = config()->text_font_size;

	// FIXME we assume the text start at offset 0!!!
	foreach(const wchar_t& unicode, utils::string_to_wstring(text())) {
		rendered_text.insert(rendered_text.end(), unicode);
		surface surf = render_text(rendered_text, font_size);
		assert(surf);
		character_offset_.push_back(surf->w);

	}
}

void ttext_box::handle_key_clear_line(SDLMod /*modifier*/, bool& handled)
{
	handled = true;

	set_text("");
}

void ttext_box::handle_key_up_arrow(SDLMod /*modifier*/, bool& handled)
{
	if (history_.get_enabled()) {
		std::string s = history_.up(text());
		if (!s.empty()) {
			set_text(s);
		}
				
		handled = true;
	}
			
}

void ttext_box::handle_key_down_arrow(SDLMod /*modifier*/, bool& handled)
{
	if (history_.get_enabled()) {
		set_text(history_.down(text()));
		handled = true;
	}
}

ttext_history ttext_history::get_history(const std::string& id, const bool enabled) 
{
	std::vector<std::string>* vec = preferences::get_history(id);
	return ttext_history(vec, enabled);
}

void ttext_history::push(const std::string& text) 
{
	if (!enabled_) {
		return; 
	} else {		
		if (!text.empty() && (history_->empty() || text != history_->back())) {
			history_->push_back(text); 
		}
		
		pos_ = history_->size();
	}
}

std::string ttext_history::up(const std::string& text)
{
	
	if (!enabled_) {
		return "";
	} else if (pos_ == history_->size()) {
		unsigned curr = pos_;
		push(text);
		pos_ = curr;
	}	

	if (pos_ != 0) {
		--pos_;
	}
	
	return get_value();
}

// Will push text to history if it is pointing at the end of the vector.
std::string ttext_history::down(const std::string& text)
{
	if (!enabled_) {
		return "";
	} else if (pos_ == history_->size()) {
		push(text);
	} else {
		pos_++;
	}
		
	return get_value();
}

std::string ttext_history::get_value() const 
{
	if (!enabled_ || pos_ == history_->size()) {
		return "";
	} else { 
		return history_->at(pos_);
	}
}

} //namespace gui2


