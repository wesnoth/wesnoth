/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
#include "serialization/unicode.hpp"

#include "utils/functional.hpp"

#include <limits>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

text_box_base::text_box_base(const implementation::builder_styled_widget& builder, const std::string& control_type)
	: styled_widget(builder, control_type)
	, state_(ENABLED)
	, text_()
	, selection_start_(0)
	, selection_length_(0)
	, ime_in_progress_(false)
	, ime_start_point_(0)
	, ime_cursor_(0)
	, ime_length_(0)
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
			&text_box_base::signal_handler_sdl_key_down, this, _2, _3, _5, _6));
	connect_signal<event::SDL_TEXT_INPUT>(std::bind(&text_box_base::handle_commit, this, _3, _5));
	connect_signal<event::SDL_TEXT_EDITING>(std::bind(&text_box_base::handle_editing, this, _3, _5, _6, _7));

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

void text_box_base::interrupt_composition()
{
	ime_in_progress_ = false;
	ime_length_ = 0;
	// We need to inform the IME that text input is no longer in progress.
	SDL_StopTextInput();
	SDL_StartTextInput();
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
	unsigned was_alpha = cursor_alpha_;
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

	if(was_alpha == cursor_alpha_) {
		return;
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

void text_box_base::handle_commit(bool& handled, const utf8::string& unicode)
{
	DBG_GUI_E << LOG_SCOPE_HEADER << '\n';

	if(unicode.size() > 1 || unicode[0] != 0) {
		handled = true;
		if(ime_in_progress_) {
			selection_start_ = ime_start_point_;
			selection_length_ = ime_length_;
			ime_in_progress_ = false;
			ime_length_ = 0;
		}
		insert_char(unicode);
		fire(event::NOTIFY_MODIFIED, *this, nullptr);

		if(text_changed_callback_) {
			text_changed_callback_(this, this->text());
		}
	}
}

void text_box_base::handle_editing(bool& handled, const utf8::string& unicode, int32_t start, int32_t len)
{
	if(unicode.size() > 1 || unicode[0] != 0) {
		handled = true;
		size_t new_len = utf8::size(unicode);
		if(!ime_in_progress_) {
			ime_in_progress_ = true;
			delete_selection();
			ime_start_point_ = selection_start_;
			text_cached_ = text_.text();
			SDL_Rect rect = get_rectangle();
			if(new_len > 0) {
				rect.x += get_cursor_position(ime_start_point_).x;
				rect.w = get_cursor_position(ime_start_point_ + new_len).x - rect.x;
			} else {
				rect.x += get_cursor_position(ime_start_point_ + new_len).x;
				rect.w = get_cursor_position(ime_start_point_).x - rect.x;
			}
			SDL_SetTextInputRect(&rect);
		}
		ime_cursor_ = start;
		ime_length_ = new_len;
		std::string new_text(text_cached_);
		utf8::insert(new_text, ime_start_point_, unicode);
		text_.set_text(new_text, false);

		// Update status
		set_cursor(ime_start_point_ + ime_cursor_, false);
		if(len > 0) {
			set_cursor(ime_start_point_ + ime_cursor_ + len, true);
		}
		update_canvas();
		set_is_dirty(true);
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
										 SDL_Keymod modifier)
{

	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

/*
 * For copy, cut and paste we use a different key on the MAC. Even for 'select
 * all', contradicting the comment in widgets/textbox.cpp:495.
 *
 * The reason for that is, by coupling 'select all' to the behavior for copy,
 * cut and paste, the text box behavior as a whole gets consistent with default
 * macOS hotkey idioms.
 */
#ifdef __APPLE__
	// Idiomatic modifier key in macOS computers.
	const SDL_Keycode modifier_key = KMOD_GUI;
#else
	// Idiomatic modifier key in Microsoft desktop environments. Common in
	// GNU/Linux as well, to some extent.
	const SDL_Keycode modifier_key = KMOD_CTRL;
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
			if(!(modifier & modifier_key)) {
				return;
			}

			select_all();
			break;

		case SDLK_HOME:
			handle_key_home(modifier, handled);
			break;

		case SDLK_END:
			handle_key_end(modifier, handled);
			break;

		case SDLK_BACKSPACE:
			handle_key_backspace(modifier, handled);
			break;

		case SDLK_u:
			if(!(modifier & KMOD_CTRL)) {
				return;
			}
			handle_key_clear_line(modifier, handled);
			break;

		case SDLK_DELETE:
			handle_key_delete(modifier, handled);
			break;

		case SDLK_c:
			if(!(modifier & modifier_key)) {
				return;
			}

			// atm we don't care whether there is something to copy or paste
			// if nothing is there we still don't want to be chained.
			copy_selection(false);
			handled = true;
			break;

		case SDLK_x:
			if(!(modifier & modifier_key)) {
				return;
			}

			copy_selection(false);
			delete_selection();
			handled = true;
			break;

		case SDLK_v:
			if(!(modifier & modifier_key)) {
				return;
			}

			paste_selection(false);
			handled = true;
			break;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:
			if(!ime_in_progress_ || (modifier & (KMOD_CTRL | KMOD_ALT | KMOD_GUI | KMOD_SHIFT))) {
				return;
			}
			// The IME will handle it, we just need to make sure nothing else handles it too.
			handled = true;
			break;

		case SDLK_ESCAPE:
			if(!ime_in_progress_ || (modifier & (KMOD_CTRL | KMOD_ALT | KMOD_GUI | KMOD_SHIFT))) {
				return;
			}
			interrupt_composition();
			handled = true;
			break;

		default:
			// Don't call the text changed callback if nothing happened.
			return;
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
