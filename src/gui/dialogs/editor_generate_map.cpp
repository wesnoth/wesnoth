/* $Id$ */
/*
   copyright (c) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "gui/dialogs/editor_generate_map.hpp"

#include "gui/dialogs/helper.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "mapgen.hpp"

#define ERR_ED LOG_STREAM_INDENT(err, editor)

namespace gui2 {

teditor_generate_map::teditor_generate_map()
: map_generators_(), current_map_generator_(0),
current_generator_label_(NULL), gui_(NULL)
{
}

void teditor_generate_map::do_settings(twindow& /*window*/)
{
	map_generator* mg = get_selected_map_generator();
	if (mg->allow_user_config()) {
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
	assert(static_cast<size_t>(current_map_generator_) < map_generators_.size());
	return map_generators_[current_map_generator_];
}

void teditor_generate_map::update_current_generator_label(twindow& window)
{
	std::stringstream ss;
	ss << lexical_cast<std::string>(current_map_generator_ + 1);
	ss << "/" << lexical_cast<std::string>(map_generators_.size());
	ss << ": " << get_selected_map_generator()->name() << ", " << get_selected_map_generator()->config_name();
	current_generator_label_->set_label(ss.str());

	window.invalidate_layout();
}

twindow* teditor_generate_map::build_window(CVideo& video)
{
	return build(video, get_id(EDITOR_GENERATE_MAP));
}

void teditor_generate_map::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(!map_generators_.empty());
	assert(gui_);
	current_generator_label_ = &window.get_widget<tlabel>("current_generator", false);
	tbutton& settings_button = window.get_widget<tbutton>("settings", false);
	settings_button.set_callback_mouse_left_click(
		dialog_callback<teditor_generate_map, &teditor_generate_map::do_settings>);
	tbutton& next_generator_button = window.get_widget<tbutton>("next_generator", false);
	next_generator_button.set_callback_mouse_left_click(
		dialog_callback<teditor_generate_map, &teditor_generate_map::do_next_generator>);
	update_current_generator_label(window);
}

void teditor_generate_map::post_show(twindow& /*window*/)
{
}

} // namespace gui2
