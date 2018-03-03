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
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "gui/dialogs/editor/generate_map.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "generators/map_generator.hpp"
#include "lexical_cast.hpp"

#include "utils/functional.hpp"

#define ERR_ED LOG_STREAM_INDENT(err, editor)

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_editor_generate_map
 *
 * == Editor generate map ==
 *
 * This shows the dialog in the editor to select which random generator
 * should be used to generate a map.
 *
 * @begin{table}{dialog_widgets}
 *
 * generators_list & & listbox & m &
 *         Listbox displaying known map generators. $
 *
 * settings & & button & m &
 *         When clicked this button opens the generator settings dialog. $
 *
 * seed_textbox & & text_box & m &
 *         Allows entering a seed for the map generator. $
 *
 * @end{table}
 */

REGISTER_DIALOG(editor_generate_map)

editor_generate_map::editor_generate_map(std::vector<std::unique_ptr<map_generator>>& mg)
	: map_generators_(mg)
	, last_map_generator_(nullptr)
	, current_map_generator_(0)
	, random_seed_()
{
}

void editor_generate_map::do_generator_selected(window& window)
{
	listbox& list = find_widget<listbox>(&window, "generators_list", false);
	const int current = list.get_selected_row();

	if(current == -1 || unsigned(current) > map_generators_.size()) {
		return; // shouldn't happen!
	}

	button& settings = find_widget<button>(&window, "settings", false);
	settings.set_active(map_generators_[current]->allow_user_config());

	current_map_generator_ = current;
}

void editor_generate_map::do_settings()
{
	get_selected_map_generator()->user_config();
}

map_generator* editor_generate_map::get_selected_map_generator()
{
	assert(static_cast<size_t>(current_map_generator_)
		   < map_generators_.size());
	return map_generators_[current_map_generator_].get();
}

void editor_generate_map::select_map_generator(map_generator* mg)
{
	last_map_generator_ = mg;
}

void editor_generate_map::pre_show(window& window)
{
	assert(!map_generators_.empty());

	register_text("seed_textbox", false, random_seed_, false);

	listbox& list = find_widget<listbox>(&window, "generators_list", false);
	window.keyboard_capture(&list);

	std::map<std::string, string_map> lrow;
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
		this->do_generator_selected(window);
	}

	connect_signal_notify_modified(list,
		std::bind(&editor_generate_map::do_generator_selected, this, std::ref(window)));

	button& settings_button = find_widget<button>(&window, "settings", false);
	connect_signal_mouse_left_click(
			settings_button,
			std::bind(&editor_generate_map::do_settings,this));
}

boost::optional<uint32_t> editor_generate_map::get_seed()
{
	try {
		return lexical_cast<uint32_t>(random_seed_);
	}
	catch(const bad_lexical_cast& ) {
		return boost::none;
	}
}

} // namespace dialogs
} // namespace gui2
