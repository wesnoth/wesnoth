/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "cursor.hpp"
#include "desktop/clipboard.hpp"
#include "gui/core/gui_definition.hpp"
#include "gui/core/log.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/unicode.hpp"

#include <functional>
#include <limits>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

using namespace std::chrono_literals;

namespace gui2
{

text_box_base::text_box_base(const implementation::builder_styled_widget& builder, const std::string& control_type)
	: styled_widget(builder, control_type)
	, state_(ENABLED)
	, text_()
	, selection_start_(0)
	, selection_length_(0)
	, editable_(true)
	, ime_composing_(false)
	, ime_start_point_(0)
	, cursor_timer_(0)
	, cursor_alpha_(0)
	, cursor_blink_rate_(750ms)
{
	auto cfg = get_control(control_type, builder.definition);
	set_font_family(cfg->text_font_family);

#ifdef __unix__
	// pastes on UNIX systems.
	connect_signal<event::MIDDLE_BUTTON_CLICK>(std::bind(
			&text_box_base::signal_handler_middle_button_click, this, std::placeholders::_2, std::placeholders::_3));

#endif

	connect_signal<event::SDL_KEY_DOWN>(std::bind(
			&text_box_base::signal_handler_sdl_key_down, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5, std::placeholders::_6));
	connect_signal<event::SDL_TEXT_INPUT>(std::bind(&text_box_base::handle_commit, this, std::placeholders::_3, std::placeholders::_5));
	connect_signal<event::SDL_TEXT_EDITING>(std::bind(&text_box_base::handle_editing, this, std::placeholders::_3, std::placeholders::_5, std::placeholders::_6, std::placeholders::_7));

	connect_signal<event::RECEIVE_KEYBOARD_FOCUS>(std::bind(
			&text_box_base::signal_handler_receive_keyboard_focus, this, std::placeholders::_2));
	connect_signal<event::LOSE_KEYBOARD_FOCUS>(
			std::bind(&text_box_base::signal_handler_lose_keyboard_focus, this, std::placeholders::_2));

	connect_signal<event::MOUSE_ENTER>(
			std::bind(&text_box_base::signal_handler_mouse_enter, this, std::placeholders::_2, std::placeholders::_3));
	connect_signal<event::MOUSE_LEAVE>(
			std::bind(&text_box_base::signal_handler_mouse_leave, this, std::placeholders::_2, std::placeholders::_3));

	toggle_cursor_timer(true);
}

text_box_base::~text_box_base()
{
	toggle_cursor_timer(false);
	update_mouse_cursor(false);
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

void text_box_base::set_maximum_length(const std::size_t maximum_length)
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
		queue_redraw();
	}
}

void text_box_base::set_value(const std::string& text)
{
	if(text != text_.text()) {
		text_.set_text(text, get_use_markup());

		// default to put the cursor at the end of the buffer.
		selection_start_ = text_.get_length();
		selection_length_ = 0;
		update_canvas();
		queue_redraw();
	}
}

void text_box_base::set_cursor(const std::size_t offset, const bool select)
{
	reset_cursor_state();

	if(select) {
		selection_length_ = (selection_start_ == offset) ? 0 : -static_cast<int>(selection_start_ - offset);
	} else {
		selection_start_ = (offset <= text_.get_length()) ? offset : 0;
		selection_length_ = 0;
	}

	update_canvas();
	queue_redraw();
}

void text_box_base::insert_char(const std::string& unicode)
{
	if(!editable_)
	{
		return;
	}

	delete_selection();

	if(text_.insert_text(selection_start_, unicode, get_use_markup())) {
		// Update status
		size_t plain_text_len = utf8::size(plain_text());
		size_t cursor_pos = selection_start_ + utf8::size(unicode);
		if (get_use_markup() && (selection_start_ + utf8::size(unicode) > plain_text_len + 1)) {
			cursor_pos = plain_text_len;
		}
		set_cursor(cursor_pos, false);
		update_canvas();
		queue_redraw();
	}
}

size_t text_box_base::get_composition_length() const
{
	if(!is_composing()) {
		return 0;
	}

	size_t text_length = utf8::size(text_.text());
	size_t text_cached_length = utf8::size(text_cached_);
	if(text_length < text_cached_length) {
		return 0;
	}

	return utf8::size(text_.text()) - utf8::size(text_cached_);
}

void text_box_base::interrupt_composition()
{
	ime_composing_ = false;
	// We need to inform the IME that text input is no longer in progress.
	SDL_StopTextInput();
	SDL_StartTextInput();
}

void text_box_base::copy_selection()
{
	if(selection_length_ == 0) {
		return;
	}

	unsigned end, start = selection_start_;
	const std::string txt = get_use_markup() ? plain_text() : text_.text();

	if(selection_length_ > 0) {
		end = utf8::index(txt, start + selection_length_);
		start = utf8::index(txt, start);
	} else {
		// inverse selection: selection_start_ is in fact the end
		end = utf8::index(txt, start);
		start = utf8::index(txt, start + selection_length_);
	}
	desktop::clipboard::copy_to_clipboard(txt.substr(start, end - start));
}

void text_box_base::paste_selection()
{
	if(!editable_)
	{
		return;
	}

	const std::string& text = desktop::clipboard::copy_from_clipboard();
	if(text.empty()) {
		return;
	}

	delete_selection();

	selection_start_ += text_.insert_text(selection_start_, text, get_use_markup());

	update_canvas();
	queue_redraw();
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void text_box_base::set_selection_start(const std::size_t selection_start)
{
	if(selection_start != selection_start_) {
		selection_start_ = selection_start;
		queue_redraw();
	}
}

void text_box_base::set_selection_length(const int selection_length)
{
	if(selection_length != selection_length_) {
		selection_length_ = selection_length;
		queue_redraw();
	}
}

void text_box_base::set_selection(std::size_t start, int length)
{
	const std::size_t text_size = text_.get_length();

	if(start >= text_size) {
		start = text_size;
	}

	if(length == 0) {
		set_cursor(start, false);
		return;
	}

	// The text pos/size type differs in both signedness and size with the
	// selection length. Such is life.
	const int sel_start = std::min<std::size_t>(start, std::numeric_limits<int>::max());
	const int sel_max_length = std::min<std::size_t>(text_size - start, std::numeric_limits<int>::max());

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
		queue_redraw();
	}
}

void text_box_base::toggle_cursor_timer(bool enable)
{
	if(cursor_blink_rate_ == 0ms) {
		return;
	}

	if(cursor_timer_) {
		remove_timer(cursor_timer_);
	}

	cursor_timer_ = enable
			? add_timer(cursor_blink_rate_, std::bind(&text_box_base::cursor_timer_callback, this), true)
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
			// FIXME: very hacky way to check if the widget's owner is the top window
			// back() on an empty vector is UB and was causing a crash when run on Wayland (see #7104 on github)
			const auto& dispatchers = event::get_all_dispatchers();
			if(!dispatchers.empty() && static_cast<event::dispatcher*>(get_window()) != dispatchers.back()) {
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

	queue_redraw();
}

void text_box_base::reset_cursor_state()
{
	if(cursor_blink_rate_ == 0ms) {
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
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;
	const int offset = selection_start_ - 1 + selection_length_;
	if(offset >= 0) {
		set_cursor(offset, (modifier & KMOD_SHIFT) != 0);
	}
}

void text_box_base::handle_key_right_arrow(SDL_Keymod modifier, bool& handled)
{
	/** @todo implement the ctrl key. */
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;
	const std::size_t offset = selection_start_ + 1 + selection_length_;
	if(offset <= (get_use_markup() ? utf8::size(plain_text()) : text_.get_length())) {
		set_cursor(offset, (modifier & KMOD_SHIFT) != 0);
	}
}

void text_box_base::handle_key_home(SDL_Keymod modifier, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;
	if(modifier & KMOD_CTRL) {
		goto_start_of_data((modifier & KMOD_SHIFT) != 0);
	} else {
		goto_start_of_line((modifier & KMOD_SHIFT) != 0);
	}
}

void text_box_base::handle_key_end(SDL_Keymod modifier, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;
	if(modifier & KMOD_CTRL) {
		goto_end_of_data((modifier & KMOD_SHIFT) != 0);
	} else {
		goto_end_of_line((modifier & KMOD_SHIFT) != 0);
	}
}

void text_box_base::handle_key_backspace(SDL_Keymod /*modifier*/, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;
	if(selection_length_ != 0) {
		delete_selection();
	} else if(selection_start_) {
		delete_char(true);
		if(is_composing()) {
			if(get_composition_length() == 0) {
				ime_composing_ = false;
			}
		}
	}
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void text_box_base::handle_key_delete(SDL_Keymod /*modifier*/, bool& handled)
{
	DBG_GUI_E << LOG_SCOPE_HEADER;

	handled = true;
	if(selection_length_ != 0) {
		delete_selection();
	} else if(selection_start_ < text_.get_length()) {
		delete_char(false);
		if(is_composing()) {
			if(get_composition_length() == 0) {
				ime_composing_ = false;
			}
		}
	}
	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void text_box_base::handle_commit(bool& handled, const std::string& unicode)
{
	DBG_GUI_E << LOG_SCOPE_HEADER;

	if(unicode.size() > 1 || unicode[0] != 0) {
		handled = true;
		if(is_composing()) {
			set_selection(ime_start_point_, get_composition_length());
			ime_composing_ = false;
		}
		insert_char(unicode);
		fire(event::NOTIFY_MODIFIED, *this, nullptr);
	}
}

/**
 * SDL_TEXTEDITING handler. See example at https://wiki.libsdl.org/Tutorials/TextInput
 */
void text_box_base::handle_editing(bool& handled, const std::string& unicode, int32_t start, int32_t len)
{
	if(unicode.size() > 1 || unicode[0] != 0) {
		handled = true;
		std::size_t new_len = utf8::size(unicode);
		if(!is_composing()) {
			ime_composing_ = true;
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

#ifdef __unix__
		// In SDL_TextEditingEvent, size of editing_text is limited
		// If length of composition text is more than the limit,
		// Linux (ibus) implementation of SDL separates it into multiple
		// SDL_TextEditingEvent.
		// start is start position of the separated event in entire composition text
		if(start == 0) {
			text_.set_text(text_cached_, get_use_markup());
		}
		text_.insert_text(ime_start_point_ + start, unicode, get_use_markup());
#else
		std::string new_text(text_cached_);
		utf8::insert(new_text, ime_start_point_, unicode);
		text_.set_text(new_text, get_use_markup());

#endif
		int maximum_length = text_.get_length();

		// Update status
		set_cursor(std::min(maximum_length, ime_start_point_ + start), false);
		if(len > 0) {
			set_cursor(std::min(maximum_length, ime_start_point_ + start + len), true);
		}
		update_canvas();
		queue_redraw();
	}
}

void text_box_base::signal_handler_middle_button_click(const event::ui_event event,
												bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	paste_selection();

	handled = true;
}

void text_box_base::signal_handler_sdl_key_down(const event::ui_event event,
										 bool& handled,
										 const SDL_Keycode key,
										 SDL_Keymod modifier)
{

	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

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
			if (!is_editable())
			{
				return;
			}

			handle_key_backspace(modifier, handled);
			break;

		case SDLK_u:
			if( !(modifier & KMOD_CTRL) || !is_editable() ) {
				return;
			}

			handle_key_clear_line(modifier, handled);
			break;

		case SDLK_DELETE:
			if (!is_editable())
			{
				return;
			}

			handle_key_delete(modifier, handled);
			break;

		case SDLK_c:
			if(!(modifier & modifier_key)) {
				return;
			}

			// atm we don't care whether there is something to copy or paste
			// if nothing is there we still don't want to be chained.
			copy_selection();
			handled = true;
			break;

		case SDLK_x:
			if( !(modifier & modifier_key) ) {
				return;
			}

			copy_selection();

			if ( is_editable() ) {
				delete_selection();
			}
			handled = true;
			break;

		case SDLK_v:
			if( !(modifier & modifier_key) || !is_editable() ) {
				return;
			}

			paste_selection();
			handled = true;
			break;

		case SDLK_RETURN:
		case SDLK_KP_ENTER:

//	TODO: check if removing the following check causes any side effects
//	To be removed if there aren't any text rendering problems.
//			if(!is_composing()) {
//				return;
//			}

			handle_key_enter(modifier, handled);
			break;

		case SDLK_ESCAPE:
			if(!is_composing() || (modifier & (KMOD_CTRL | KMOD_ALT | KMOD_GUI | KMOD_SHIFT))) {
				return;
			}
			interrupt_composition();
			handled = true;
			break;

		case SDLK_TAB:
			handle_key_tab(modifier, handled);
			break;

		default:
			return;
	}
}

void text_box_base::signal_handler_receive_keyboard_focus(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(FOCUSED);
}

void text_box_base::signal_handler_lose_keyboard_focus(const event::ui_event event)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	set_state(ENABLED);
}

void text_box_base::signal_handler_mouse_enter(const event::ui_event event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	if(state_ != FOCUSED) {
		set_state(HOVERED);
	}

	update_mouse_cursor(true);

	handled = true;
}

void text_box_base::signal_handler_mouse_leave(const event::ui_event event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".";

	if(state_ != FOCUSED) {
		set_state(ENABLED);
	}

	update_mouse_cursor(false);

	handled = true;
}

void text_box_base::update_mouse_cursor(bool enable)
{
	// Someone else may set the mouse cursor for us to something unusual (e.g.
	// the WAIT cursor) so we ought to mess with that only if it's set to
	// NORMAL or IBEAM.

	if(enable && cursor::get() == cursor::NORMAL) {
		cursor::set(cursor::IBEAM);
	} else if(!enable && cursor::get() == cursor::IBEAM) {
		cursor::set(cursor::NORMAL);
	}
}


} // namespace gui2
