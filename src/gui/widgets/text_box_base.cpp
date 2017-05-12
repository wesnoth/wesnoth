/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/text_box_base.hpp"

#include "desktop/clipboard.hpp"
#include "gui/core/log.hpp"
#include "gui/core/timer.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"

#include "utils/functional.hpp"

#include <limits>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

text_box_base::text_box_base()
	: styled_widget(COUNT)
	, state_(ENABLED)
	, text_()
	, selection_start_(0)
	, selection_length_(0)
	, cursor_timer_(0)
	, cursor_alpha_(0)
	, cursor_blink_rate_ms_(750)
	, text_changed_callback_()
{
#ifdef __unix__
	// pastes on UNIX systems.
	connect_signal<event::MIDDLE_BUTTON_CLICK>(std::bind(
			&text_box_base::signal_handler_middle_button_click, this, _2, _3));

#endif

	connect_signal<event::SDL_KEY_DOWN>(std::bind(
			&text_box_base::signal_handler_sdl_key_down, this, _2, _3, _5, _6, _7));

	connect_signal<event::RECEIVE_KEYBOARD_FOCUS>(std::bind(
			&text_box_base::signal_handler_receive_keyboard_focus, this, _2));
	connect_signal<event::LOSE_KEYBOARD_FOCUS>(
			std::bind(&text_box_base::signal_handler_lose_keyboard_focus, this, _2));

	toggle_cursor_timer(true);
}

text_box_base::~text_box_base()
{
	toggle_cursor_timer(false);
}

void text_box_base::set_active(const bool active)
{
	if(get_active() != active) {
		set_state(active ? ENABLED : DISABLED);
	}
}

bool text_box_base::get_active() const
{
	return state_ != DISABLED;
}

unsigned text_box_base::get_state() const
{
	return state_;
}

void text_box_base::set_maximum_length(const size_t maximum_length)
{
	if(maximum_length == 0) {
		return;
	}

	const bool need_update = text_.get_length() > maximum_length;

	text_.set_maximum_length(maximum_length);

	if(need_update) {
		if(selection_start_ > maximum_length) {
			selection_start_ = maximum_length;
			selection_length_ = 0;
		} else if(selection_start_ + selection_length_ > maximum_length) {
			selection_length_ = maximum_length - selection_start_;
		}
		update_canvas();
		set_is_dirty(true);
	}
}

void text_box_base::set_value(const std::string& text)
{
	if(text != text_.text()) {
		text_.set_text(text, false);

		// default to put the cursor at the end of the buffer.
		selection_start_ = text_.get_length();
		selection_length_ = 0;
		update_canvas();
		set_is_dirty(true);
	}
}

void text_box_base::set_cursor(const size_t offset, const bool select)
{
	reset_cursor_state();

	if(select) {

		if(selection_start_ == offset) {
			selection_length_ = 0;
		} else {
			selection_length_ = -static_cast<int>(selection_start_ - offset);
		}

#ifdef __unix__
		// selecting copies on UNIX systems.
		copy_selection(true);
#endif
		update_canvas();
		set_is_dirty(true);

	} else {
		assert(offset <= text_.get_length());
		selection_start_ = offset;
		selection_length_ = 0;

		update_canvas();
		set_is_dirty(true);
	}
}

void text_box_base::insert_char(const utf8::string& unicode)
{
	delete_selection();

	if(text_.insert_text(selection_start_, unicode)) {

		// Update status
		set_cursor(selection_start_ + utf8::size(unicode), false);
		update_canvas();
		set_is_dirty(true);
	}
}

void text_box_base::copy_selection(const bool mouse)
{
	if(selection_length_ == 0) {
		return;
	}

	unsigned end, start = selection_start_;
	const utf8::string txt = text_.text();

	if(selection_length_ > 0) {
		end = utf8::index(txt, start + selection_length_);
		start = utf8::index(txt, start);
	} else {
		// inverse selection: selection_start_ is in fact the end
		end = utf8::index(txt, start);
		start = utf8::index(txt, start + selection_length_);
	}
	desktop::clipboard::copy_to_clipboard(txt.substr(start, end - start), mouse);
}

void text_box_base::paste_selection(const bool mouse)
{
	const std::string& text = desktop::clipboard::copy_from_clipboard(mouse);
	if(text.empty()) {
		return;
	}

	delete_selection();

	selection_start_ += text_.insert_text(selection_start_, text);

	update_canvas();
	set_is_dirty(true);
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void text_box_base::set_selection_start(const size_t selection_start)
{
	if(selection_start != selection_start_) {
		selection_start_ = selection_start;
		set_is_dirty(true);
	}
}

void text_box_base::set_selection_length(const int selection_length)
{
	if(selection_length != selection_length_) {
		selection_length_ = selection_length;
		set_is_dirty(true);
	}
}

void text_box_base::set_selection(size_t start, int length)
{
	const size_t text_size = text_.get_length();

	if(start >= text_size) {
		start = text_size;
	}

	if(length == 0) {
		set_cursor(start, false);
		return;
	}

	// The text pos/size type differs in both signedness and size with the
	// selection length. Such is life.
	const int sel_start = std::min<size_t>(start, std::numeric_limits<int>::max());
	const int sel_max_length = std::min<size_t>(text_size - start, std::numeric_limits<int>::max());

	const bool backwards = length < 0;

	if(backwards && -length > sel_start) {
		length = -sel_start;
	} else if(!backwards && length > sel_max_length) {
		length = sel_max_length;
	}

	set_selection_start(start);
	set_selection_length(length);

	update_canvas();
}

void text_box_base::set_state(const state_t state)
{
	if(state != state_) {
		state_ = state;
		set_is_dirty(true);
	}
}

void text_box_base::toggle_cursor_timer(bool enable)
{
	if(!cursor_blink_rate_ms_) {
		return;
	}

	if(cursor_timer_) {
		remove_timer(cursor_timer_);
	}

	cursor_timer_ = enable
			? add_timer(cursor_blink_rate_ms_, std::bind(&text_box_base::cursor_timer_callback, this), true)
			: 0;
}

void text_box_base::cursor_timer_callback()
{
	switch(state_) {
		case DISABLED:
			cursor_alpha_ = 0;
			return;
		case ENABLED:
			cursor_alpha_ = 255;
			return;
		default:
			if(get_window() != open_window_stack.back()) {
				cursor_alpha_ = 0;
			} else {
				cursor_alpha_ = (~cursor_alpha_) & 0xFF;
			}
	}

	for(auto& tmp : get_canvases()) {
		tmp.set_variable("cursor_alpha", wfl::variant(cursor_alpha_));
	}

	set_is_dirty(true);
}

void text_box_base::reset_cursor_state()
{
	if(!cursor_blink_rate_ms_) {
		return;
	}

	cursor_alpha_ = 255;

	for(auto& tmp : get_canvases()) {
		tmp.set_variable("cursor_alpha", wfl::variant(cursor_alpha_));
	}

	// Restart the blink timer.
	toggle_cursor_timer(true);
}

void text_box_base::handle_key_left_arrow(SDL_Keymod modifier, bool& handled)
{
	/** @todo implement the ctrl key. */
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	handled = true;
	const int offset = selection_start_ - 1 + selection_length_;
	if(offset >= 0) {
		set_cursor(offset, (modifier & KMOD_SHIFT) != 0);
	}
}

void text_box_base::handle_key_right_arrow(SDL_Keymod modifier, bool& handled)
{
	/** @todo implement the ctrl key. */
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	handled = true;
	const size_t offset = selection_start_ + 1 + selection_length_;
	if(offset <= text_.get_length()) {
		set_cursor(offset, (modifier & KMOD_SHIFT) != 0);
	}
}

void text_box_base::handle_key_home(SDL_Keymod modifier, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	handled = true;
	if(modifier & KMOD_CTRL) {
		goto_start_of_data((modifier & KMOD_SHIFT) != 0);
	} else {
		goto_start_of_line((modifier & KMOD_SHIFT) != 0);
	}
}

void text_box_base::handle_key_end(SDL_Keymod modifier, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	handled = true;
	if(modifier & KMOD_CTRL) {
		goto_end_of_data((modifier & KMOD_SHIFT) != 0);
	} else {
		goto_end_of_line((modifier & KMOD_SHIFT) != 0);
	}
}

void text_box_base::handle_key_backspace(SDL_Keymod /*modifier*/, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	handled = true;
	if(selection_length_ != 0) {
		delete_selection();
	} else if(selection_start_) {
		delete_char(true);
	}
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void text_box_base::handle_key_delete(SDL_Keymod /*modifier*/, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	handled = true;
	if(selection_length_ != 0) {
		delete_selection();
	} else if(selection_start_ < text_.get_length()) {
		delete_char(false);
	}
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void text_box_base::handle_key_default(bool& handled,
								SDL_Keycode /*key*/,
								SDL_Keymod /*modifier*/,
								const utf8::string& unicode)
{
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	if(unicode.size() > 1 || unicode[0] != 0) {
		handled = true;
		insert_char(unicode);
		fire(event::NOTIFY_MODIFIED, *this, nullptr);
	}
}

void text_box_base::signal_handler_middle_button_click(const event::ui_event event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	paste_selection(true);

	handled = true;
}

void text_box_base::signal_handler_sdl_key_down(const event::ui_event event,
										 bool& handled,
										 const SDL_Keycode key,
										 SDL_Keymod modifier,
										 const utf8::string& unicode)
{

	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

// For copy/paste we use a different key on the MAC. Other ctrl modifiers won't
// be modifed seems not to be required when I read the comment in
// widgets/textbox.cpp:516. Would be nice if somebody on a MAC would test it.
#ifdef __APPLE__
	const unsigned copypaste_modifier = KMOD_LGUI | KMOD_RGUI;
#else
	const unsigned copypaste_modifier = KMOD_CTRL;
#endif

	switch(key) {

		case SDLK_LEFT:
			handle_key_left_arrow(modifier, handled);
			break;

		case SDLK_RIGHT:
			handle_key_right_arrow(modifier, handled);
			break;

		case SDLK_UP:
			handle_key_up_arrow(modifier, handled);
			break;

		case SDLK_DOWN:
			handle_key_down_arrow(modifier, handled);
			break;

		case SDLK_PAGEUP:
			handle_key_page_up(modifier, handled);
			break;

		case SDLK_PAGEDOWN:
			handle_key_page_down(modifier, handled);
			break;

		case SDLK_a:
			if(!(modifier & KMOD_CTRL)) {
				handle_key_default(handled, key, modifier, unicode);
				break;
			}

			// If ctrl-a is used for home drop the styled_widget modifier
			modifier = static_cast<SDL_Keymod>(modifier & ~KMOD_CTRL);
			FALLTHROUGH;

		case SDLK_HOME:
			handle_key_home(modifier, handled);
			break;

		case SDLK_e:
			if(!(modifier & KMOD_CTRL)) {
				handle_key_default(handled, key, modifier, unicode);
				break;
			}

			// If ctrl-e is used for end drop the styled_widget modifier
			modifier = static_cast<SDL_Keymod>(modifier & ~KMOD_CTRL);
			FALLTHROUGH;

		case SDLK_END:
			handle_key_end(modifier, handled);
			break;

		case SDLK_BACKSPACE:
			handle_key_backspace(modifier, handled);
			break;

		case SDLK_u:
			if(modifier & KMOD_CTRL) {
				handle_key_clear_line(modifier, handled);
			} else {
				handle_key_default(handled, key, modifier, unicode);
			}
			break;

		case SDLK_DELETE:
			handle_key_delete(modifier, handled);
			break;

		case SDLK_c:
			if(!(modifier & copypaste_modifier)) {
				handle_key_default(handled, key, modifier, unicode);
				break;
			}

			// atm we don't care whether there is something to copy or paste
			// if nothing is there we still don't want to be chained.
			copy_selection(false);
			handled = true;
			break;

		case SDLK_x:
			if(!(modifier & copypaste_modifier)) {
				handle_key_default(handled, key, modifier, unicode);
				break;
			}

			copy_selection(false);
			delete_selection();
			handled = true;
			break;

		case SDLK_v:
			if(!(modifier & copypaste_modifier)) {
				handle_key_default(handled, key, modifier, unicode);
				break;
			}

			paste_selection(false);
			handled = true;
			break;

		default:
			handle_key_default(handled, key, modifier, unicode);
	}

	if(text_changed_callback_) {
		text_changed_callback_(this, this->text());
	}
}

void text_box_base::signal_handler_receive_keyboard_focus(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(FOCUSED);
}

void text_box_base::signal_handler_lose_keyboard_focus(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	set_state(ENABLED);
}

} // namespace gui2
