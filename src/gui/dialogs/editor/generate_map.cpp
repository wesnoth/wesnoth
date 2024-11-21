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

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "gui/dialogs/editor/generate_map.hpp"


#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/window.hpp"
#include "generators/map_generator.hpp"
#include "lexical_cast.hpp"

#include <functional>

#define ERR_ED LOG_STREAM_INDENT(err, editor)

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_generate_map)

editor_generate_map::editor_generate_map(std::vector<std::unique_ptr<map_generator>>& mg)
	: modal_dialog(window_id())
	, map_generators_(mg)
	, last_map_generator_(nullptr)
	, current_map_generator_(0)
	, random_seed_()
{
}

void editor_generate_map::do_generator_selected()
{
	listbox& list = find_widget<listbox>("generators_list");
	const int current = list.get_selected_row();

	if(current == -1 || static_cast<unsigned>(current) > map_generators_.size()) {
		return; // shouldn't happen!
	}

	button& settings = find_widget<button>("settings");
	settings.set_active(map_generators_[current]->allow_user_config());

	current_map_generator_ = current;
}

void editor_generate_map::do_settings()
{
	get_selected_map_generator()->user_config();
}

map_generator* editor_generate_map::get_selected_map_generator()
{
	assert(static_cast<std::size_t>(current_map_generator_)
		   < map_generators_.size());
	return map_generators_[current_map_generator_].get();
}

void editor_generate_map::select_map_generator(map_generator* mg)
{
	last_map_generator_ = mg;
}

void editor_generate_map::pre_show()
{
	assert(!map_generators_.empty());

	register_text("seed_textbox", false, random_seed_, false);

	listbox& list = find_widget<listbox>("generators_list");
	keyboard_capture(&list);

	widget_data lrow;
	for(const auto & gen : map_generators_)
	{
		assert(gen);
		lrow["generator_name"]["label"] = gen->config_name();
		// lrow["generator_id"]["label"] = gen->name();

		list.add_row(lrow);

		if(gen.get() == last_map_generator_) {
			list.select_last_row();
		}
	}

	if (last_map_generator_ != nullptr) {
		// We need to call this manually because it won't be called by
		// list.select_row() even if we set the callback before
		// calling it
		this->do_generator_selected();
	}

	connect_signal_notify_modified(list,
		std::bind(&editor_generate_map::do_generator_selected, this));

	button& settings_button = find_widget<button>("settings");
	connect_signal_mouse_left_click(
			settings_button,
			std::bind(&editor_generate_map::do_settings,this));
}

utils::optional<uint32_t> editor_generate_map::get_seed()
{
	try {
		return lexical_cast<uint32_t>(random_seed_);
	}
	catch(const bad_lexical_cast& ) {
		return utils::nullopt;
	}
}

} // namespace dialogs
