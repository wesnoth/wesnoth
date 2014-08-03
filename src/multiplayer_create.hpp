/*
   Copyright (C) 2007 - 2014
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#ifndef MULTIPLAYER_CREATE_HPP_INCLUDED
#define MULTIPLAYER_CREATE_HPP_INCLUDED

#include "depcheck.hpp"
#include "create_engine.hpp"
#include "multiplayer_ui.hpp"
#include "widgets/slider.hpp"
#include "widgets/combo.hpp"
#include "tooltips.hpp"

namespace mp {

class create : public mp::ui
{
public:
	create(game_display& disp, const config& game_config, saved_game& state,
		chat& c, config& gamelist);
	~create();

	const mp_game_settings& get_parameters();

protected:
	virtual void layout_children(const SDL_Rect& rect);
	virtual void process_event();
	virtual void hide_children(bool hide=true);

private:
	void init_level_type_changed(size_t index);
	void init_level_changed(size_t index);

	void synchronize_selections();

	void draw_level_image();

	void set_description(const std::string& description);

	void update_mod_menu_images();

	std::string select_campaign_difficulty();

	tooltips::manager tooltip_manager_;
	int era_selection_;
	int mod_selection_;
	int level_selection_;

	gui::menu eras_menu_;
	gui::menu levels_menu_;
	gui::menu mods_menu_;

	gui::label filter_name_label_;
	gui::label filter_num_players_label_;
	gui::label map_generator_label_;
	gui::label era_label_;
	gui::label no_era_label_;
	gui::label mod_label_;
	gui::label map_size_label_;
	gui::label num_players_label_;
	gui::label level_type_label_;

	gui::button launch_game_;
	gui::button cancel_game_;
	gui::button regenerate_map_;
	gui::button generator_settings_;
	gui::button load_game_;
	gui::button select_mod_;

	gui::combo level_type_combo_;

	gui::slider filter_num_players_slider_;

	gui::textbox description_;
	gui::textbox filter_name_;

	util::scoped_ptr<surface_restorer> image_restorer_;
	SDL_Rect image_rect_;

	std::vector<ng::level::TYPE> available_level_types_;

	ng::create_engine engine_;
};

} // end namespace mp

#endif

