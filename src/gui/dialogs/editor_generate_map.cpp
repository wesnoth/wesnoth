/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/editor_generate_map.hpp"

#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/helper.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "generators/map_generator.hpp"

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
 * current_generator & & label & m &
 *         The label displaying the name of the currently selected generator. $
 *
 * settings & & button & m &
 *         When clicked this button opens the generator settings dialog. $
 *
 * next_generator & & button & m &
 *         Selects the next generator in the list, this list wraps at the
 *         end. $
 *
 * @end{table}
 */

REGISTER_DIALOG(editor_generate_map)

teditor_generate_map::teditor_generate_map()
	: map_generators_()
	, current_map_generator_(0)
	, random_seed_()
	, current_generator_label_(NULL)
	, gui_(NULL)
{
}

void teditor_generate_map::do_settings(twindow& /*window*/)
{
	map_generator* mg = get_selected_map_generator();
	if(mg->allow_user_config()) {
		mg->user_config(*gui_);
	}
}

void teditor_generate_map::do_next_generator(twindow& window)
{
	current_map_generator_++;
	current_map_generator_ %= map_generators_.size();
	update_current_generator_label(window);
}

map_generator* teditor_generate_map::get_selected_map_generator()
{
	assert(static_cast<size_t>(current_map_generator_)
		   < map_generators_.size());
	return map_generators_[current_map_generator_];
}

void teditor_generate_map::update_current_generator_label(twindow& window)
{
	std::stringstream ss;
	ss << lexical_cast<std::string>(current_map_generator_ + 1) << "/"
	   << lexical_cast<std::string>(map_generators_.size()) << ": "
	   << get_selected_map_generator()->name() << ", "
	   << get_selected_map_generator()->config_name();

	current_generator_label_->set_label(ss.str());

	window.invalidate_layout();
}

void teditor_generate_map::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(!map_generators_.empty());
	assert(gui_);

	current_generator_label_
			= &find_widget<tlabel>(&window, "current_generator", false);
	
	register_text("seed_textbox", false, random_seed_, false);

	tbutton& settings_button = find_widget<tbutton>(&window, "settings", false);
	connect_signal_mouse_left_click(
			settings_button,
			boost::bind(&teditor_generate_map::do_settings,
						this,
						boost::ref(window)));

	tbutton& next_generator_button
			= find_widget<tbutton>(&window, "next_generator", false);
	connect_signal_mouse_left_click(
			next_generator_button,
			boost::bind(&teditor_generate_map::do_next_generator,
						this,
						boost::ref(window)));

	update_current_generator_label(window);
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
