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

/**
 * @file
 * Create a multiplayer-game: select map, players, options etc.
 */

#include "global.hpp"

#include "gettext.hpp"
#include "game_config_manager.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "construct_dialog.hpp"
#include "settings.hpp"
#include "map.hpp"
#include "map_exception.hpp"
#include "generators/map_create.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/dialogs/mp_create_game_set_password.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "minimap.hpp"
#include "multiplayer_create.hpp"
#include "filesystem.hpp"
#include "log.hpp"
#include "wml_exception.hpp"
#include "wml_separators.hpp"
#include "formula_string_utils.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)

static lg::log_domain log_mp_create("mp/create");
#define DBG_MP LOG_STREAM(debug, log_mp_create)

namespace {
const SDL_Rect null_rect = {0, 0, 0, 0};
}

namespace mp {

create::create(game_display& disp, const config& cfg, game_state& state,
	chat& c, config& gamelist) :
	ui(disp, _("Create Game"), cfg, c, gamelist),
	tooltip_manager_(disp.video()),
	era_selection_(-1),
	mod_selection_(-1),
	level_selection_(-1),
	eras_menu_(disp.video(), std::vector<std::string>()),
	levels_menu_(disp.video(), std::vector<std::string>()),
	mods_menu_(disp.video(), std::vector<std::string>()),
	filter_name_label_(disp.video(), _("Filter:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	filter_num_players_label_(disp.video(), _("Number of players: any"), font::SIZE_SMALL, font::LOBBY_COLOR),
	map_generator_label_(disp.video(), _("Random map options:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	era_label_(disp.video(), _("Era:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	no_era_label_(disp.video(), _("No eras available\nfor this game."),
		font::SIZE_SMALL, font::LOBBY_COLOR),
	mod_label_(disp.video(), _("Modifications:"), font::SIZE_SMALL, font::LOBBY_COLOR),
	map_size_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	num_players_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOR),
	level_type_label_(disp.video(), "Game type:", font::SIZE_SMALL, font::LOBBY_COLOR),
	launch_game_(disp.video(), _("Next")),
	cancel_game_(disp.video(), _("Cancel")),
	regenerate_map_(disp.video(), _("Regenerate")),
	generator_settings_(disp.video(), _("Settings...")),
	load_game_(disp.video(), _("Load Game...")),
	select_mod_(disp.video(), _("Activate")),
	level_type_combo_(disp, std::vector<std::string>()),
	filter_num_players_slider_(disp.video()),
	description_(disp.video(), 100, "", false),
	filter_name_(disp.video(), 100, "", true, 256, font::SIZE_SMALL),
	image_restorer_(NULL),
	image_rect_(null_rect),
	available_level_types_(),
	engine_(disp, state)
{
	filter_num_players_slider_.set_min(1);
	filter_num_players_slider_.set_max(9);
	filter_num_players_slider_.set_increment(1);

	DBG_MP << "constructing multiplayer create dialog" << std::endl;

	levels_menu_.set_numeric_keypress_selection(false);

	typedef std::pair<level::TYPE, std::string> level_type_info;
	std::vector<level_type_info> all_level_types;
	all_level_types.push_back(std::make_pair(level::SCENARIO, _("Scenarios")));
	all_level_types.push_back(std::make_pair(level::CAMPAIGN, _("Campaigns")));
	all_level_types.push_back(std::make_pair(level::USER_MAP, _("User Maps")));
	all_level_types.push_back(std::make_pair(level::USER_SCENARIO, _("User Scenarios")));
	all_level_types.push_back(std::make_pair(level::RANDOM_MAP, _("Random Maps")));

	if (game_config::debug) {
		all_level_types.push_back(std::make_pair(level::SP_CAMPAIGN,
			"SP Campaigns"));
	}

	std::vector<std::string> combo_level_names;

	BOOST_FOREACH(level_type_info type_info, all_level_types) {
		if (!engine_.get_levels_by_type_unfiltered(type_info.first).empty()) {
			available_level_types_.push_back(type_info.first);
			combo_level_names.push_back(type_info.second);
		}
	}

	if (available_level_types_.empty()) {
		gui2::show_transient_message(disp.video(), "", _("No games found."));
		throw game::error(_("No games found."));
	}

	level_type_combo_.set_items(combo_level_names);

	size_t combo_new_selection = 0;
	size_t level_new_selection = 0;

	// TODO: this is needed to get the levels menu stretched to its max
	// width, otherwise there might be problems with gui widgets alignment.
	// Ideally, there could be a gui::menu::set_min_width() method,
	// so this would no longer be necessary.
	init_level_type_changed(0);

	// Set level selection according to the preferences, if possible.
	size_t type_index = 0;
	BOOST_FOREACH(level::TYPE type, available_level_types_) {
		if (preferences::level_type() == type) {
			break;
		}
		type_index++;
	}
	if (type_index < available_level_types_.size()) {
		combo_new_selection = type_index;

		int level_index = engine_.find_level_by_id(preferences::level());
		if (level_index != -1) {
			level_new_selection = level_index;
		}
	}

	level_type_combo_.set_selected(combo_new_selection);
	init_level_type_changed(level_new_selection);

	const std::vector<std::string>& era_names =
		engine_.extras_menu_item_names(create_engine::ERA);
	if(era_names.empty()) {
		gui2::show_transient_message(disp.video(), "", _("No eras found."));
		throw config::error(_("No eras found"));
	}
	eras_menu_.set_items(era_names);

	// Set era selection according to the preferences, if possible.
	int era_new_selection = engine_.find_extra_by_id(create_engine::ERA,
		preferences::era());
	eras_menu_.move_selection((era_new_selection != -1) ? era_new_selection : 0);

	std::vector<std::string> mods = engine_.extras_menu_item_names(create_engine::MOD);
	BOOST_FOREACH(std::string& mod, mods) {
		std::stringstream newval;
		newval << IMAGE_PREFIX << "buttons/checkbox.png" << COLUMN_SEPARATOR << mod;
		mod = newval.str();
	}
	mods_menu_.set_items(mods);
	mods_menu_.move_selection(0);
	// don't set 0 explicitly, because move_selection(0) may fail if there's
	// no modifications at all
	mod_selection_ = mods_menu_.selection();

	if (mod_selection_ == -1) {
		mod_label_.set_text(_("Modifications:\nNone found."));
	} else if (engine_.dependency_manager().is_modification_active(mod_selection_)) {
		select_mod_.set_label(_("Deactivate"));
	}

	gamelist_updated();
}

create::~create()
{
	// Only save the settings if the dialog was 'accepted'
	if(get_result() != CREATE) {
		DBG_MP << "destructing multiplayer create dialog - aborted game creation" << std::endl;
		return;
	}
	DBG_MP << "destructing multiplayer create dialog - a game will be created" << std::endl;

	// Save values for next game
	DBG_MP << "storing parameter values in preferences" << std::endl;
	preferences::set_era(engine_.current_extra(create_engine::ERA).id);
	preferences::set_level(engine_.current_level().id());
	preferences::set_level_type(engine_.current_level_type());
	preferences::set_modifications(engine_.active_mods());
}

const mp_game_settings& create::get_parameters()
{
	return engine_.get_parameters();
}

void create::process_event()
{
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex, mousey);

	if (cancel_game_.pressed()) {
		set_result(QUIT);
		return;
	}

	if (launch_game_.pressed() || levels_menu_.double_clicked()) {
		if (engine_.current_level().can_launch_game()) {
			if (engine_.current_level_type() == level::CAMPAIGN ||
				engine_.current_level_type() == level::SP_CAMPAIGN) {

				std::string difficulty = select_campaign_difficulty();
				if (difficulty == "CANCEL") {
					return;
				}

				engine_.prepare_for_campaign(difficulty);
			}

			engine_.prepare_for_new_level();

			set_result(CREATE);
			return;
		} else {
			gui2::show_transient_message(disp_.video(), "",
				_("The level is invalid."));
		}
	}

	if (level_type_combo_.changed()) {
		init_level_type_changed(0);
	}

	if (load_game_.pressed()) {
		engine_.prepare_for_saved_game();

		set_result(LOAD_GAME);

		return;
	}

	bool update_mod_button_label = mod_selection_ != mods_menu_.selection();
	if (select_mod_.pressed() || mods_menu_.double_clicked()) {
		int index = mods_menu_.selection();
		engine_.set_current_mod_index(index);
		engine_.toggle_current_mod();

		update_mod_button_label = true;
		synchronize_selections();
	}

	if (update_mod_button_label) {
		mod_selection_ = mods_menu_.selection();
		engine_.set_current_mod_index(mod_selection_);
		set_description(engine_.current_extra(create_engine::MOD).description);
		if (engine_.dependency_manager().is_modification_active(mod_selection_)) {
			select_mod_.set_label(_("Deactivate"));
		} else {
			select_mod_.set_label(_("Activate"));
		}
	}

	bool era_changed = era_selection_ != eras_menu_.selection();
	era_selection_ = eras_menu_.selection();

	if (era_changed) {
		engine_.set_current_era_index(era_selection_);

		set_description(engine_.current_extra(create_engine::ERA).description);
		synchronize_selections();
	}

	if (filter_name_.text() != engine_.level_name_filter()) {
		engine_.apply_level_filter(filter_name_.text());
		init_level_type_changed(0);
	}

	bool level_changed = level_selection_ != levels_menu_.selection();
	level_selection_ = levels_menu_.selection();

	if (level_changed && level_selection_ >= 0) {
		init_level_changed(level_selection_);

		synchronize_selections();
	}

	if (engine_.generator_assigned() && generator_settings_.pressed()) {
		engine_.generator_user_config(disp_);

		level_changed = true;
	}

	if(engine_.generator_assigned() &&
		(level_changed || regenerate_map_.pressed())) {
		const cursor::setter cursor_setter(cursor::WAIT);
		cursor::setter cur(cursor::WAIT);

		engine_.init_generated_level_data();

		if (!engine_.current_level().data()["error_message"].empty())
			gui2::show_message(disp().video(), "map generation error",
				engine_.current_level().data()["error_message"]);

		level_changed = true;
	}

	if(level_changed) {
		std::stringstream players;
		std::stringstream map_size;

		players << _("Players: ");

		engine_.current_level().set_metadata();

		draw_level_image();

		set_description(engine_.current_level().description());

		switch (engine_.current_level_type()) {
		case level::SCENARIO:
		case level::USER_MAP:
		case level::USER_SCENARIO:
		case level::RANDOM_MAP: {

			scenario* current_scenario =
				dynamic_cast<scenario*>(&engine_.current_level());

			players << current_scenario->num_players();
			map_size << _("Size: ") << current_scenario->map_size();

			break;
		}
		case level::CAMPAIGN:
		case level::SP_CAMPAIGN: {
			campaign* current_campaign =
				dynamic_cast<campaign*>(&engine_.current_level());

			players << current_campaign->min_players();
			if (current_campaign->max_players() !=
				current_campaign->min_players()) {

				players << " to " << current_campaign->max_players();
			}

			break;
		}
		} // end switch

		map_size_label_.set_text(map_size.str());
		num_players_label_.set_text(players.str());

		launch_game_.enable(engine_.current_level().can_launch_game());
		generator_settings_.enable(engine_.generator_assigned());
		regenerate_map_.enable(engine_.generator_assigned());
	}

	if (filter_num_players_slider_.value() != engine_.player_num_filter()) {
		const int val = filter_num_players_slider_.value();
		engine_.apply_level_filter(val);
		std::stringstream ss;
		if (val == 1) {
			ss << _("Number of players: any");
		} else {
			ss << _("Number of players: ") << val;
		}
		filter_num_players_label_.set_text(ss.str());
		init_level_type_changed(0);
	}
}

void create::init_level_type_changed(size_t index)
{
	int selected = level_type_combo_.selected();
	if (selected < 0) {
		selected = 0;
	}

	engine_.set_current_level_type(available_level_types_[selected]);
	const std::vector<std::string>& menu_item_names =
		engine_.levels_menu_item_names();

	init_level_changed((index < menu_item_names.size()) ? index : 0);

	levels_menu_.set_items(menu_item_names);
	levels_menu_.move_selection(index);

	level_selection_ = -1;
}

void create::init_level_changed(size_t index)
{
	engine_.set_current_level(index);

	// N.B. the order of hide() calls here is important
	// to avoid redrawing glitches.
	if (engine_.current_level().allow_era_choice()) {
		no_era_label_.hide(true);
		eras_menu_.hide(false);
	} else {
		eras_menu_.hide(true);
		no_era_label_.hide(false);
	}
}

void create::synchronize_selections()
{
	DBG_MP << "Synchronizing with the dependency manager" << std::endl;
	if (era_selection_ != engine_.dependency_manager().get_era_index()) {
		eras_menu_.move_selection(engine_.dependency_manager().get_era_index());
		process_event();
	}

	if (engine_.current_level_type() != level::CAMPAIGN &&
		engine_.current_level_type() != level::SP_CAMPAIGN) {
		if (engine_.current_level().id() !=
			engine_.dependency_manager().get_scenario()) {

			// Match scenario and scenario type
			level::TYPE level_type_at_index;
			int index = engine_.find_level_by_id(
				engine_.dependency_manager().get_scenario());
			size_t type_index;

			if (index == -1) {
				return;
			}
			level_type_at_index = engine_.find_level_type_by_id(
				engine_.dependency_manager().get_scenario());
			engine_.set_current_level_type(level_type_at_index);

			init_level_changed(index);
			levels_menu_.set_items(engine_.levels_menu_item_names());
			levels_menu_.move_selection(index);
			type_index = 0;
			BOOST_FOREACH(level::TYPE type, available_level_types_) {
				if (level_type_at_index == type) {
					level_type_combo_.set_selected(type_index);
					break;
				}
				type_index++;
			}
		}

		process_event();
	}

	engine_.init_active_mods();
	update_mod_menu_images();
}

void create::draw_level_image()
{
	boost::scoped_ptr<surface> image(
		engine_.current_level().create_image_surface(image_rect_));

	if (image.get() != NULL) {
		SDL_Color back_color = {0,0,0,255};
		draw_centered_on_background(*image, image_rect_, back_color,
			video().getSurface());
	} else {
		surface display(disp_.get_screen_surface());
		sdl_fill_rect(display, &image_rect_,
			SDL_MapRGB(display->format, 0, 0, 0));
		update_rect(image_rect_);

	}
}

void create::set_description(const std::string& description)
{
	description_.set_text(description.empty() ? _("No description available.") :
												description);
}

void create::update_mod_menu_images()
{
	for (size_t i = 0; i<mods_menu_.number_of_items(); i++) {
		std::stringstream val;
		if (engine_.dependency_manager().is_modification_active(i)) {
			val << IMAGE_PREFIX << "buttons/checkbox-pressed.png";
		} else {
			val << IMAGE_PREFIX << "buttons/checkbox.png";
		}
		mods_menu_.change_item(i, 0, val.str());
	}
}

std::string create::select_campaign_difficulty()
{
	const std::string difficulty_descriptions =
		engine_.current_level().data()["difficulty_descriptions"];
	std::vector<std::string> difficulty_options =
		utils::split(difficulty_descriptions, ';');
	const std::vector<std::string> difficulties =
		utils::split(engine_.current_level().data()["difficulties"]);

	if(!difficulties.empty()) {
		int difficulty = 0;
		if(difficulty_options.size() != difficulties.size()) {
			difficulty_options = difficulties;
		}

		gui2::tcampaign_difficulty dlg(difficulty_options);
		dlg.show(disp().video());

		if(dlg.selected_index() == -1) {
			return "CANCEL";
		}
		difficulty = dlg.selected_index();

		return difficulties[difficulty];
	}

	return "";
}

void create::hide_children(bool hide)
{
	DBG_MP << (hide ? "hiding" : "showing" ) << " children widgets" << std::endl;

	ui::hide_children(hide);

	eras_menu_.hide(hide || !engine_.current_level().allow_era_choice());
	no_era_label_.hide(hide || engine_.current_level().allow_era_choice());
	levels_menu_.hide(hide);
	mods_menu_.hide(hide);

	filter_name_.hide(hide);
	filter_num_players_label_.hide(hide);
	map_generator_label_.hide(hide);
	map_size_label_.hide(hide);
	era_label_.hide(hide);
	mod_label_.hide(hide);
	num_players_label_.hide(hide);
	level_type_label_.hide(hide);

	level_type_combo_.hide(hide);

	cancel_game_.hide(hide);
	launch_game_.hide(hide);

	load_game_.hide(hide);

	select_mod_.hide(hide);

	regenerate_map_.hide(hide);
	generator_settings_.hide(hide);

	filter_num_players_slider_.hide(hide);

	description_.hide(hide);
	filter_name_.hide(hide);

	if (hide) {
		image_restorer_.assign(NULL);
	} else {
		image_restorer_.assign(new surface_restorer(&video(), image_rect_));

		engine_.current_level().set_metadata();
		draw_level_image();
	}
}

void create::layout_children(const SDL_Rect& rect)
{
	DBG_MP << "laying out the children" << std::endl;

	ui::layout_children(rect);

	const int border_size =  6;
	const int column_border_size = 10;

	SDL_Rect ca = client_area();
	int xpos = ca.x;
	int ypos = ca.y;

	// 222 is two times a button's minimal width plus one time border_size.
	// Instead of binding this value to the actual button widths, I chose this
	// because it makes no difference for most languages, and where it does, I
	// guess we'd prefer having the buttons less neatly aligned to having a
	// potentially giant image.
	const int image_width = ca.h < 500 ? 111 : 222;
	const int menu_width = (ca.w - 3 * column_border_size - image_width) / 3;
	const int eras_menu_height = (ca.h / 2 - era_label_.height() -
		2 * border_size - cancel_game_.height());
	const int mods_menu_height = (ca.h / 2 - mod_label_.height() -
		3 * border_size - cancel_game_.height() - select_mod_.height());

	// Dialog title
	ypos += title().height() + border_size;

	// Save ypos here (column top)
	int ypos_columntop = ypos;

	// First column: image & random map options
	image_rect_ = create_rect(xpos, ypos, image_width, image_width);
	ypos += image_width + border_size;

	num_players_label_.set_location(xpos, ypos);
	ypos += num_players_label_.height() + border_size;

	map_size_label_.set_location(xpos, ypos);
	ypos += map_size_label_.height() + 2 * border_size;

	const int ypos1 = ypos;
	const int xpos1 = xpos;
	// The description box is set later

	// Second column: filtering options
	ypos = ypos_columntop;
	xpos += image_width + column_border_size;
	filter_name_label_.set_location(xpos, ypos);
	filter_name_.set_location(xpos + filter_name_label_.width() + border_size, ypos);
	filter_name_.set_measurements(menu_width - border_size - filter_name_label_.width(), filter_name_label_.height());
	ypos += filter_name_.height() + border_size;
	filter_num_players_label_.set_location(xpos, ypos);
	ypos += filter_num_players_label_.height() + border_size;
	filter_num_players_slider_.set_location(xpos, ypos);
	filter_num_players_slider_.set_width(menu_width);
	ypos += filter_num_players_slider_.height() + border_size;
	map_generator_label_.set_location(xpos, ypos);
	ypos += map_generator_label_.height() + border_size;
	regenerate_map_.set_location(xpos, ypos);
	ypos += regenerate_map_.height() + border_size;
	generator_settings_.set_location(xpos, ypos);
	ypos += generator_settings_.height() + border_size;
	load_game_.set_location(xpos, ypos + 4 * border_size);

	// And now the description box
	description_.set_location(xpos1, std::max(ypos,ypos1));
	description_.set_measurements(image_width + border_size + menu_width, ca.h + ca.y - std::max(ypos,ypos1) - border_size);
	description_.set_wrap(true);
	ypos += description_.height() + border_size;

	//Third column: levels menu
	ypos = ypos_columntop;
	xpos += menu_width + column_border_size;
	level_type_label_.set_location(xpos, ypos);
	ypos += level_type_label_.height() + border_size;
	level_type_combo_.set_location(xpos, ypos);
	ypos += level_type_combo_.height() + border_size;

	const int levels_menu_y_offset = (ca.w < 900 || ca.h < 500) ?
		((cancel_game_.height() + border_size) * -1) : 0;
	levels_menu_.set_max_width(menu_width);
	levels_menu_.set_max_height(ca.h + ca.y - ypos + levels_menu_y_offset);
	levels_menu_.set_location(xpos, ypos);
	// Menu dimensions are only updated when items are set. So do this now.
	int levelsel = levels_menu_.selection();
	levels_menu_.set_items(engine_.levels_menu_item_names());
	levels_menu_.move_selection(levelsel);

	// Place game type combo and label in the middle of levels menu
	// by x axis.
	const int level_type_combo_x_offset = (levels_menu_.width() -
		level_type_combo_.width()) / 2;
	level_type_combo_.set_location(
		level_type_combo_.location().x + level_type_combo_x_offset,
		level_type_combo_.location().y);
	const int level_type_label_x_offset = (levels_menu_.width() -
		level_type_label_.width()) / 2;
	level_type_label_.set_location(
		level_type_label_.location().x + level_type_label_x_offset,
		level_type_label_.location().y);

	//Fourth column: eras & mods menu
	ypos = ypos_columntop;
	xpos += menu_width + column_border_size;
	era_label_.set_location(xpos, ypos);
	ypos += era_label_.height() + border_size;
	no_era_label_.set_location(xpos, ypos);
	eras_menu_.set_max_width(menu_width);
	eras_menu_.set_max_height(eras_menu_height);
	eras_menu_.set_location(xpos, ypos);
	// Menu dimensions are only updated when items are set. So do this now.
	int erasel_save = eras_menu_.selection();
	eras_menu_.set_items(engine_.extras_menu_item_names(create_engine::ERA));
	eras_menu_.move_selection(erasel_save);
	ypos += eras_menu_height;

	//TODO: use when mods_menu_ would be functional.
	mod_label_.set_location(xpos, ypos);
	ypos += mod_label_.height() + border_size;
	mods_menu_.set_max_width(menu_width);
	mods_menu_.set_max_height(mods_menu_height);
	mods_menu_.set_location(xpos, ypos);
	if (mods_menu_.number_of_items() > 0) {
		ypos += mods_menu_.height() + border_size;
		select_mod_.set_location(xpos, ypos);
	}

	// OK / Cancel buttons
	gui::button* left_button = &launch_game_;
	gui::button* right_button = &cancel_game_;

#ifdef OK_BUTTON_ON_RIGHT
	std::swap(left_button,right_button);
#endif

	// Buttons
	right_button->set_location(ca.x + ca.w - right_button->width(),
		ca.y + ca.h - right_button->height());
	left_button->set_location(right_button->location().x - left_button->width() -
		gui::ButtonHPadding, ca.y + ca.h - left_button->height());

	if (ca.h < 500) {
		load_game_.set_location(left_button->location().x - load_game_.width() -
			gui::ButtonHPadding, ca.y + ca.h - load_game_.height());
	}
}

} // namespace mp


