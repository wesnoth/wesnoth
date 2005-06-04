/* $Id$ */
/*
   Copyright (C)
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "display.hpp"
#include "gettext.hpp"
#include "image.hpp"
#include "show_dialog.hpp"
#include "map_create.hpp"
#include "multiplayer_create.hpp"
#include "filesystem.hpp"
#include "preferences.hpp"
#include "video.hpp"
#include "serialization/string_utils.hpp"

namespace {
const SDL_Rect null_rect = {0, 0, 0, 0};
}

namespace mp {

create::create(display& disp, const config &cfg, chat& c, config& gamelist) :
	ui(disp, cfg, c, gamelist),

	tooltip_manager_(disp.video()),
	map_selection_(-1),

	maps_menu_(disp.video(), std::vector<std::string>()),
	turns_slider_(disp.video()),
	turns_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOUR),
	village_gold_slider_(disp.video()),
	village_gold_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOUR),
	xp_modifier_slider_(disp.video()),
	xp_modifier_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOUR),
	name_entry_label_(disp.video(), _("Name of game:"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	num_players_label_(disp.video(), "", font::SIZE_SMALL, font::LOBBY_COLOUR),
	era_label_(disp.video(), _("Era:"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	map_label_(disp.video(), _("Map to play:"), font::SIZE_SMALL, font::LOBBY_COLOUR),
	fog_game_(disp.video(), _("Fog Of War"), gui::button::TYPE_CHECK),
	shroud_game_(disp.video(), _("Shroud"), gui::button::TYPE_CHECK),
	observers_game_(disp.video(), _("Observers"), gui::button::TYPE_CHECK),
	cancel_game_(disp.video(), _("Cancel")),
	launch_game_(disp.video(), _("OK")),
	regenerate_map_(disp.video(), _("Regenerate")),
	generator_settings_(disp.video(), _("Settings...")),
	era_combo_(disp, std::vector<std::string>()),
	vision_combo_(disp, std::vector<std::string>()),
	name_entry_(disp.video(), 32),
	minimap_restorer_(NULL),
	minimap_rect_(null_rect),
	generator_(NULL)
{
	//build the list of scenarios to play
	get_files_in_dir(get_user_data_dir() + "/editor/maps",&user_maps_,NULL,FILE_NAME_ONLY);

	map_options_ = user_maps_;

	const config::child_list& levels = cfg.get_children("multiplayer");
	for(config::child_list::const_iterator j = levels.begin(); j != levels.end(); ++j){
		map_options_.push_back((**j)["name"]);
	}
	//add the 'load game' option
	map_options_.push_back(_("Load Game") + std::string("..."));

	//create the scenarios menu
	maps_menu_.set_items(map_options_);
	if (size_t(preferences::map()) < map_options_.size())
		maps_menu_.move_selection(preferences::map());
	maps_menu_.set_numeric_keypress_selection(false);

	turns_slider_.set_min(20);
	turns_slider_.set_max(100);
	turns_slider_.set_value(preferences::turns());
	turns_slider_.set_help_string(_("The maximum turns the game will go for"));

	village_gold_slider_.set_min(1);
	village_gold_slider_.set_max(5);
	village_gold_slider_.set_value(preferences::village_gold());
	village_gold_slider_.set_help_string(_("The amount of income each village yields per turn"));
	xp_modifier_slider_.set_min(30);
	xp_modifier_slider_.set_max(200);
	xp_modifier_slider_.set_value(preferences::xp_modifier());
	xp_modifier_slider_.set_increment(10);
	xp_modifier_slider_.set_help_string(_("The amount of experience a unit needs to advance"));

	fog_game_.set_check(preferences::fog());
	fog_game_.set_help_string(_("Enemy units cannot be seen unless they are in range of your units"));

	shroud_game_.set_check(preferences::shroud());
	shroud_game_.set_help_string(_("The map is unknown until your units explore it"));

	observers_game_.set_check(preferences::allow_observers());
	observers_game_.set_help_string(_("Allow users who are not playing to watch the game"));

	// The possible vision settings
	std::vector<std::string> vision_types;
	vision_types.push_back(_("Share View"));
	vision_types.push_back(_("Share Maps"));
	vision_types.push_back(_("Share None"));
	vision_combo_.set_items(vision_types);
	vision_combo_.set_selected(0);

	// The possible eras to play
	const config::child_list& era_list = cfg.get_children("era");
	std::vector<std::string> eras;
	for(config::child_list::const_iterator er = era_list.begin(); er != era_list.end(); ++er) {
		eras.push_back((**er)["name"]);
	}
	if(eras.empty()) {
		gui::show_dialog(disp, NULL, "", _("No multiplayer sides."), gui::OK_ONLY);
		std::cerr << "ERROR: no eras found\n";
		throw config::error(_("No eras found"));
	}
	era_combo_.set_items(eras);

	if (size_t(preferences::era()) < eras.size()) {
		era_combo_.set_selected(preferences::era());
	} else {
		era_combo_.set_selected(0);
	}


	utils::string_map i18n_symbols;
	i18n_symbols["login"] = preferences::login();
	name_entry_.set_text(vgettext("$login's game", i18n_symbols));

	gamelist_updated();
}

create::~create()
{
	get_parameters();

	//Save values for next game
	preferences::set_allow_observers(parameters_.allow_observers);
	preferences::set_fog(parameters_.fog_game);
	preferences::set_shroud(parameters_.shroud_game);
	preferences::set_turns(parameters_.num_turns);
	preferences::set_village_gold(parameters_.village_gold);
	preferences::set_xp_modifier(parameters_.xp_modifier);
	preferences::set_era(era_combo_.selected()); // FIXME: may be broken if new eras are added
	preferences::set_map(map_selection_);
	preferences::set_turns(turns_slider_.value());
}

create::parameters& create::get_parameters()
{
	const config::child_list& era_list = game_config().get_children("era");
	const int turns = turns_slider_.value() < turns_slider_.max_value() ?
		turns_slider_.value() : -1;

	// Updates the values in the "parameters_" member to match the values
	// selected by the user with the widgets:
	parameters_.name = name_entry_.text();
	if (size_t(era_combo_.selected()) >= era_list.size()) {
		throw config::error(_("Invalid era selected"));
	}
	parameters_.era = (*era_list[era_combo_.selected()])["id"];
	parameters_.num_turns = turns;
	parameters_.village_gold = village_gold_slider_.value();
	parameters_.xp_modifier = xp_modifier_slider_.value();
	parameters_.fog_game = fog_game_.checked();
	parameters_.shroud_game = shroud_game_.checked();
	parameters_.allow_observers = observers_game_.checked();
	parameters_.share_view = vision_combo_.selected() == 0;
	parameters_.share_maps = vision_combo_.selected() == 1;

	return parameters_;
}

void create::process_event()
{
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex, mousey);

	if(cancel_game_.pressed()) {
		set_result(QUIT);
		return;
	}

	if(launch_game_.pressed() || maps_menu_.double_clicked()) {
		if(name_entry_.text() != "") {
			set_result(CREATE);
			return;
		} else {
			gui::show_dialog(disp_, NULL, "", _("You must enter a name."), gui::OK_ONLY);
		}
	}

	// Turns per game
	const int cur_turns = turns_slider_.value();

	std::stringstream buf;
	if(cur_turns < 100) {
		buf << _("Turns: ") << cur_turns;
	} else {
		buf << _("Unlimited Turns");
	}
	turns_label_.set_text(buf.str());

	//Villages can produce between 1 and 10 gold a turn
	const int village_gold = village_gold_slider_.value();
	buf.str("");
	buf << _("Village Gold: ") << village_gold;
	village_gold_label_.set_text(buf.str());

	//experience modifier
	const int xpmod = xp_modifier_slider_.value();
	buf.str("");
	buf << _("Experience Modifier: ") << xpmod << "%";
	xp_modifier_label_.set_text(buf.str());

	bool map_changed = map_selection_ != maps_menu_.selection();
	map_selection_ = maps_menu_.selection();

	if(map_changed) {
		generator_.assign(NULL);

		tooltips::clear_tooltips(minimap_rect_);

		const size_t select = size_t(maps_menu_.selection());

		if(select < user_maps_.size()) {
			parameters_.saved_game = false;
			const config* const generic_multiplayer = game_config().child("generic_multiplayer");
			if(generic_multiplayer != NULL) {
				parameters_.scenario_data = *generic_multiplayer;
				parameters_.scenario_data["map_data"] = read_map(user_maps_[select]);
			}

		} else if(select != maps_menu_.nitems()-1) {
			parameters_.saved_game = false;
			const size_t index = select - user_maps_.size();
			const config::child_list& levels = game_config().get_children("multiplayer");

			if(index < levels.size()) {

				parameters_.scenario_data = *levels[index];

				t_string& map_data = parameters_.scenario_data["map_data"];
				if(map_data == "" && parameters_.scenario_data["map"] != "") {
					map_data = read_map(parameters_.scenario_data["map"]);
				}

				//if the map should be randomly generated
				if(parameters_.scenario_data["map_generation"] != "") {
					generator_.assign(create_map_generator(parameters_.scenario_data["map_generation"],parameters_.scenario_data.child("generator")));
				}

				if(!parameters_.scenario_data["description"].empty()) {
					tooltips::add_tooltip(minimap_rect_,parameters_.scenario_data["description"]);
				}
			}
		} else {
			parameters_.scenario_data.clear();
			parameters_.saved_game = true;

			if (minimap_restorer_ != NULL)
				minimap_restorer_->restore();
		}
	}

	if(generator_ != NULL && generator_->allow_user_config() && generator_settings_.pressed()) {
		generator_->user_config(disp_);
		map_changed = true;
	}

	if(generator_ != NULL && (map_changed || regenerate_map_.pressed())) {
		const cursor::setter cursor_setter(cursor::WAIT);

		//generate the random map
		parameters_.scenario_data = generator_->create_scenario(std::vector<std::string>());
		map_changed = true;

		//set the scenario to have placing of sides based on the terrain they prefer
		parameters_.scenario_data["modify_placing"] = "true";
	}

	if(map_changed) {
		generator_settings_.hide(generator_ == NULL);
		regenerate_map_.hide(generator_ == NULL);

		const std::string& map_data = parameters_.scenario_data["map_data"];
		gamemap map(game_config(), map_data);

		//if there are less sides in the configuration than there are starting
		//positions, then generate the additional sides
		const int map_positions = map.num_valid_starting_positions();

		for(int pos =  parameters_.scenario_data.get_children("side").size(); pos < map_positions; ++pos) {
			config& side = parameters_.scenario_data.add_child("side");
			side["enemy"] = "1";
			side["side"] = lexical_cast<std::string>(pos+1);
			side["team_name"] = lexical_cast<std::string>(pos+1);
			side["canrecruit"] = "1";
			side["controller"] = "human";
		}

		//if there are too many sides, remove some
		while(int(parameters_.scenario_data.get_children("side").size()) > map_positions) {
			 parameters_.scenario_data.remove_child("side",
					  parameters_.scenario_data.get_children("side").size()-1);
		}

		const surface mini(image::getMinimap(minimap_rect_.w,minimap_rect_.h,map,0));
		if(mini != NULL) {
			SDL_Rect rect = minimap_rect_;
			SDL_BlitSurface(mini, NULL, video().getSurface(), &rect);
			update_rect(rect);
		}
		const int nsides = parameters_.scenario_data.get_children("side").size();
		std::stringstream players;
		players << _("Players: ") << nsides;
		num_players_label_.set_text(players.str());
	}
}

void create::hide_children(bool hide)
{
	ui::hide_children(hide);

	maps_menu_.hide(hide);
	turns_slider_.hide(hide);
	turns_label_.hide(hide);
	village_gold_slider_.hide(hide);
	village_gold_label_.hide(hide);
	xp_modifier_slider_.hide(hide);
	xp_modifier_label_.hide(hide);

	name_entry_label_.hide(hide);
	num_players_label_.hide(hide);
	era_label_.hide(hide);
	map_label_.hide(hide);

	fog_game_.hide(hide);
	shroud_game_.hide(hide);
	observers_game_.hide(hide);
	cancel_game_.hide(hide);
	launch_game_.hide(hide);
	regenerate_map_.hide(hide || generator_ == NULL);
	generator_settings_.hide(hide || generator_ == NULL);

	era_combo_.hide(hide);
	vision_combo_.hide(hide);
	name_entry_.hide(hide);

	if (hide) {
		minimap_restorer_.assign(NULL);
	} else {
		minimap_restorer_.assign(new surface_restorer(&video(), minimap_rect_));

		const std::string& map_data = parameters_.scenario_data["map_data"];
		gamemap map(game_config(), map_data);
		const surface mini(image::getMinimap(minimap_rect_.w,minimap_rect_.h,map,0));
		if(mini != NULL) {
			SDL_Rect rect = minimap_rect_;
			SDL_BlitSurface(mini, NULL, video().getSurface(), &rect);
			update_rect(rect);
		}
	}
}

void create::layout_children(const SDL_Rect& rect)
{
	ui::layout_children(rect);
	SDL_Rect ca = client_area();

	const int border_size = 6;
	const int column_border_size = 10;
	int xpos = ca.x;
	int ypos = ca.y;

	// Dialog title
	ypos += gui::draw_dialog_title(xpos, ypos, &video(), _("Create Game")).h + border_size;

	// Name Entry
	name_entry_label_.set_location(xpos, ypos);
	ypos += name_entry_label_.height() + border_size;
	name_entry_.set_location(xpos, ypos);
	name_entry_.set_width(ca.w);
	ypos += name_entry_.height() + border_size;

	// Save ypos here (column top)
	int ypos_columntop = ypos;

	// First column: minimap & random map options
	const int minimap_width = 200;
	SDL_Rect mmrect = { xpos, ypos, minimap_width, minimap_width };
	minimap_rect_ = mmrect;
	ypos += minimap_width + border_size;

	num_players_label_.set_location(xpos, ypos);
	ypos += num_players_label_.height() + border_size;

	regenerate_map_.set_location(xpos, ypos);
	ypos += regenerate_map_.height() + border_size;
	generator_settings_.set_location(xpos, ypos);

	// Second column: map menu
	ypos = ypos_columntop;
	xpos += minimap_width + column_border_size;
	map_label_.set_location(xpos, ypos);
	ypos += map_label_.height() + border_size;
	maps_menu_.set_max_width(200);
	maps_menu_.set_max_height(ca.h + ca.x - ypos - border_size);
	maps_menu_.set_location(xpos, ypos);
	// Menu dimensions are only updated when items are set. So do this now.
	int mapsel_save = maps_menu_.selection();
	maps_menu_.set_items(map_options_);
	maps_menu_.move_selection(mapsel_save);

	// Third column: big buch of options
	ypos = ypos_columntop;
	xpos += 200 + column_border_size;

	turns_label_.set_location(xpos, ypos);
	ypos += turns_label_.height() + border_size;
	turns_slider_.set_width(ca.w - xpos);
	turns_slider_.set_location(xpos, ypos);
	ypos += turns_slider_.height() + border_size;

	village_gold_label_.set_location(xpos, ypos);
	ypos += village_gold_label_.height() + border_size;
	village_gold_slider_.set_width(ca.w - xpos);
	village_gold_slider_.set_location(xpos, ypos);
	ypos += village_gold_slider_.height() + border_size;

	xp_modifier_label_.set_location(xpos, ypos);
	ypos += xp_modifier_label_.height() + border_size;
	xp_modifier_slider_.set_width(ca.w - xpos);
	xp_modifier_slider_.set_location(xpos, ypos);
	ypos += xp_modifier_slider_.height() + border_size;

	era_label_.set_location(xpos, ypos + (era_combo_.height() - era_label_.height()) / 2);
	era_combo_.set_location(xpos + era_label_.width() + border_size, ypos);
	ypos += era_combo_.height() + border_size;

	fog_game_.set_location(xpos, ypos);
	ypos += fog_game_.height() + border_size;

	shroud_game_.set_location(xpos, ypos);
	ypos += shroud_game_.height() + border_size;

	observers_game_.set_location(xpos, ypos);
	ypos += observers_game_.height() + border_size;

	vision_combo_.set_location(xpos, ypos);
	ypos += vision_combo_.height() + border_size;

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
}

}
