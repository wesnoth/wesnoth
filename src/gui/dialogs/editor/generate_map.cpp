/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
#include "utils/foreach.hpp"

#include <boost/bind.hpp>

#define ERR_ED LOG_STREAM_INDENT(err, editor)

namespace gui2
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

teditor_generate_map::teditor_generate_map()
	: map_generators_()
	, last_map_generator_(NULL)
	, current_map_generator_(0)
	, random_seed_()
{
}

void teditor_generate_map::do_generator_selected(twindow& window)
{
	tlistbox& list = find_widget<tlistbox>(&window, "generators_list", false);
	const int current = list.get_selected_row();

	if(current == -1 || unsigned(current) > map_generators_.size()) {
		return; // shouldn't happen!
	}

	tbutton& settings = find_widget<tbutton>(&window, "settings", false);
	settings.set_active(map_generators_[current]->allow_user_config());

	current_map_generator_ = current;
}

void teditor_generate_map::do_settings(twindow& window)
{
	get_selected_map_generator()->user_config(window.video());
}

map_generator* teditor_generate_map::get_selected_map_generator()
{
	assert(static_cast<size_t>(current_map_generator_)
		   < map_generators_.size());
	return map_generators_[current_map_generator_];
}

void teditor_generate_map::select_map_generator(map_generator* mg)
{
	last_map_generator_ = mg;
}

void teditor_generate_map::pre_show(twindow& window)
{
	assert(!map_generators_.empty());

	register_text("seed_textbox", false, random_seed_, false);

	tlistbox& list = find_widget<tlistbox>(&window, "generators_list", false);
	window.keyboard_capture(&list);

	std::map<std::string, string_map> lrow;
	FOREACH(const AUTO & gen, map_generators_)
	{
		assert(gen);
		lrow["generator_name"]["label"] = gen->config_name();
		// lrow["generator_id"]["label"] = gen->name();

		list.add_row(lrow);

		if(gen == last_map_generator_) {
			list.select_row(list.get_item_count() - 1);
		}
	}

	if (last_map_generator_ != NULL) {
		// We need to call this manually because it won't be called by
		// list.select_row() even if we set the callback before
		// calling it
		this->do_generator_selected(window);
	}

	list.set_callback_item_change(
			boost::bind(&teditor_generate_map::do_generator_selected, this, boost::ref(window)));

	tbutton& settings_button = find_widget<tbutton>(&window, "settings", false);
	connect_signal_mouse_left_click(
			settings_button,
			boost::bind(&teditor_generate_map::do_settings,
						this,
						boost::ref(window)));
}

boost::optional<boost::uint32_t> teditor_generate_map::get_seed()
{
	try {
		return lexical_cast<boost::uint32_t>(random_seed_);
	}
	catch(const bad_lexical_cast& ) {
		return boost::none;
	}
}

} // namespace gui2
