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

#include "gui/dialogs/modal_dialog.hpp"

#include "gui/auxiliary/field.hpp"
#include "gui/widgets/integer_selector.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "video.hpp"

namespace gui2
{
namespace dialogs
{
modal_dialog::modal_dialog()
	: window_(nullptr)
	, retval_(0)
	, always_save_fields_(false)
	, fields_()
	, focus_()
	, restore_(false)
	, allow_plugin_skip_(true)
	, show_even_without_video_(false)
{
}

modal_dialog::~modal_dialog()
{
}

namespace {
	struct window_stack_handler {
		window_stack_handler(std::unique_ptr<window>& win) : local_window(win) {
			open_window_stack.push_back(local_window.get());
		}
		~window_stack_handler() {
			remove_from_window_stack(local_window.get());
		}
		std::unique_ptr<window>& local_window;
	};
}

bool modal_dialog::show(const unsigned auto_close_time)
{
	if(CVideo::get_singleton().faked() && !show_even_without_video_) {
		if(!allow_plugin_skip_) {
			return false;
		}

		plugins_manager* pm = plugins_manager::get();
		if (pm && pm->any_running())
		{
			plugins_context pc("Dialog");
			pc.set_callback("skip_dialog", [this](config) { retval_ = window::OK; }, false);
			pc.set_callback("quit", [](config) {}, false);
			pc.play_slice();
		}

		return false;
	}

	window_.reset(build_window());
	assert(window_.get());

	post_build(*window_);

	window_->set_owner(this);

	init_fields(*window_);

	pre_show(*window_);

	{ // Scope the window stack
		window_stack_handler push_window_stack(window_);
		retval_ = window_->show(auto_close_time);
	}

	/*
	 * It can happen that when two clicks follow each other fast that the event
	 * handling code in events.cpp generates a DOUBLE_CLICK_EVENT. For some
	 * reason it can happen that this event gets pushed in the queue when the
	 * window is shown, but processed after the window is closed. This causes
	 * the next window to get this pending event.
	 *
	 * This caused a bug where double clicking in the campaign selection dialog
	 * directly selected a difficulty level and started the campaign. In order
	 * to avoid that problem, filter all pending DOUBLE_CLICK_EVENT events after
	 * the window is closed.
	 */
	SDL_FlushEvent(DOUBLE_CLICK_EVENT);

	finalize_fields(*window_, (retval_ == window::OK || always_save_fields_));

	post_show(*window_);

	// post_show may have updated the window retval. Update it here.
	retval_ = window_->get_retval();

	// Reset window object.
	window_.reset(nullptr);

	return retval_ == window::OK;
}

field_bool* modal_dialog::register_bool(
		const std::string& id,
		const bool mandatory,
		const std::function<bool()> callback_load_value,
		const std::function<void(bool)> callback_save_value,
		const std::function<void(widget&)> callback_change,
		const bool initial_fire)
{
	field_bool* field = new field_bool(id,
										 mandatory,
										 callback_load_value,
										 callback_save_value,
										 callback_change,
										 initial_fire);

	fields_.emplace_back(field);
	return field;
}

field_bool*
modal_dialog::register_bool(const std::string& id,
					   const bool mandatory,
					   bool& linked_variable,
					   const std::function<void(widget&)> callback_change,
					   const bool initial_fire)
{
	field_bool* field
			= new field_bool(id, mandatory, linked_variable, callback_change, initial_fire);

	fields_.emplace_back(field);
	return field;
}

field_integer* modal_dialog::register_integer(
		const std::string& id,
		const bool mandatory,
		const std::function<int()> callback_load_value,
		const std::function<void(const int)> callback_save_value)
{
	field_integer* field = new field_integer(
			id, mandatory, callback_load_value, callback_save_value);

	fields_.emplace_back(field);
	return field;
}

field_integer* modal_dialog::register_integer(const std::string& id,
										  const bool mandatory,
										  int& linked_variable)
{
	field_integer* field = new field_integer(id, mandatory, linked_variable);

	fields_.emplace_back(field);
	return field;
}

field_text* modal_dialog::register_text(
		const std::string& id,
		const bool mandatory,
		const std::function<std::string()> callback_load_value,
		const std::function<void(const std::string&)> callback_save_value,
		const bool capture_focus)
{
	field_text* field = new field_text(
			id, mandatory, callback_load_value, callback_save_value);

	if(capture_focus) {
		focus_ = id;
	}

	fields_.emplace_back(field);
	return field;
}

field_text* modal_dialog::register_text(const std::string& id,
									const bool mandatory,
									std::string& linked_variable,
									const bool capture_focus)
{
	field_text* field = new field_text(id, mandatory, linked_variable);

	if(capture_focus) {
		focus_ = id;
	}

	fields_.emplace_back(field);
	return field;
}

field_label* modal_dialog::register_label(const std::string& id,
									  const bool mandatory,
									  const std::string& text,
									  const bool use_markup)
{
	field_label* field = new field_label(id, mandatory, text, use_markup);

	fields_.emplace_back(field);
	return field;
}

window* modal_dialog::build_window() const
{
	return build(window_id());
}

void modal_dialog::post_build(window& /*window*/)
{
	/* DO NOTHING */
}

void modal_dialog::pre_show(window& /*window*/)
{
	/* DO NOTHING */
}

void modal_dialog::post_show(window& /*window*/)
{
	/* DO NOTHING */
}

void modal_dialog::init_fields(window& window)
{
	for(auto& field : fields_)
	{
		field->attach_to_window(window);
		field->widget_init(window);
	}

	if(!focus_.empty()) {
		if(widget* widget = window.find(focus_, false)) {
			window.keyboard_capture(widget);
		}
	}
}

void modal_dialog::finalize_fields(window& window, const bool save_fields)
{
	for(auto& field : fields_)
	{
		if(save_fields) {
			field->widget_finalize(window);
		}
		field->detach_from_window();
	}
}

} // namespace dialogs
} // namespace gui2


/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 1
 *
 * {{Autogenerated}}
 *
 * = Window definition =
 *
 * The window definition define how the windows shown in the dialog look.
 */

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = ZZZZZZ_footer
 *
 * [[Category: WML Reference]]
 * [[Category: GUI WML Reference]]
 */
