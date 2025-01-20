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

#include "gui/dialogs/modal_dialog.hpp"

#include "cursor.hpp"
#include "events.hpp"
#include "gui/auxiliary/field.hpp"
#include "gui/core/gui_definition.hpp" // get_window_builder
#include "gui/widgets/integer_selector.hpp"
#include "scripting/plugins/context.hpp"
#include "scripting/plugins/manager.hpp"
#include "video.hpp"

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)
#define WRN_DP LOG_STREAM(warn, log_display)

namespace gui2::dialogs
{

modal_dialog::modal_dialog(const std::string& window_id)
	: window(get_window_builder(window_id))
	, always_save_fields_(false)
	, fields_()
	, focus_()
	, allow_plugin_skip_(true)
	, show_even_without_video_(false)
{
	widget::set_id(window_id);
}

modal_dialog::~modal_dialog()
{
}

bool modal_dialog::show(const unsigned auto_close_time)
{
	if(video::headless() && !show_even_without_video_) {
		DBG_DP << "modal_dialog::show denied";
		return false;
	}
	if(allow_plugin_skip_) {
		bool skipped = false;

		plugins_manager* pm = plugins_manager::get();
		if (pm && pm->any_running())
		{
			plugins_context pc("Dialog");
			pc.set_callback("skip_dialog", [this, &skipped](const config&) { set_retval(retval::OK); skipped = true; }, false);
			pc.set_callback("quit", [this, &skipped](const config&) { set_retval(retval::CANCEL); skipped = true; }, false);
			pc.set_callback("select", [this, &skipped](const config& c) { set_retval(c["retval"].to_int()); skipped = true; }, false);
			pc.set_accessor_string("id", [this](const config&) { return window_id(); });
			pc.play_slice();
		}

		if(skipped) {
			return false;
		}
	}

	init_fields();

	pre_show();

	{
		cursor::setter cur{cursor::NORMAL};
		window::show(auto_close_time);
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

	finalize_fields(get_retval() == retval::OK || always_save_fields_);

	post_show();

	// post_show may update the window retval
	return get_retval() == retval::OK;
}

template<typename T, typename... Args>
T* modal_dialog::register_field(Args&&... args)
{
	static_assert(std::is_base_of_v<field_base, T>, "Type is not a field type");
	auto field = std::make_unique<T>(std::forward<Args>(args)...);
	T* res = field.get();
	fields_.push_back(std::move(field));
	return res;
}

field_bool* modal_dialog::register_bool(
		const std::string& id,
		const bool mandatory,
		const std::function<bool()>& callback_load_value,
		const std::function<void(bool)>& callback_save_value,
		const std::function<void(widget&)>& callback_change,
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
					   const std::function<void(widget&)>& callback_change,
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
		const std::function<int()>& callback_load_value,
		const std::function<void(int)>& callback_save_value)
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
		const std::function<std::string()>& callback_load_value,
		const std::function<void(const std::string&)>& callback_save_value,
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

void modal_dialog::pre_show()
{
	/* DO NOTHING */
}

void modal_dialog::post_show()
{
	/* DO NOTHING */
}

void modal_dialog::init_fields()
{
	for(auto& field : fields_)
	{
		field->attach_to_window(*this);
		field->widget_init();
	}

	if(!focus_.empty()) {
		if(widget* widget = window::find(focus_, false)) {
			window::keyboard_capture(widget);
		}
	}
}

void modal_dialog::finalize_fields(const bool save_fields)
{
	for(auto& field : fields_)
	{
		if(save_fields) {
			field->widget_finalize();
		}
		field->detach_from_window();
	}
}

} // namespace dialogs
