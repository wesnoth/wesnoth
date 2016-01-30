/*
   Copyright (C) 2003 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "global.hpp"

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "desktop/notifications.hpp" //needed to check if notifications are not available
#include "display.hpp"
#include "filesystem.hpp"
#include "filechooser.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/advanced_graphics_options.hpp"
#include "gui/dialogs/game_cache_options.hpp"
#include "gui/dialogs/mp_alerts_options.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/theme_list.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "lobby_preferences.hpp"
#include "marked-up_text.hpp"
#include "preferences_display.hpp"
#include "wml_separators.hpp"
#include "widgets/combo.hpp"
#include "widgets/slider.hpp"
#include "formula_string_utils.hpp"

#include <boost/foreach.hpp>

namespace preferences {

static void set_lobby_joins(int ison)
{
	_set_lobby_joins(ison);
}

static void set_sort_list(bool ison)
{
	_set_sort_list(ison);
}

static void set_iconize_list(bool ison)
{
	_set_iconize_list(ison);
}

namespace {

struct advanced_preferences_sorter
{
	bool operator()(const config& lhs, const config& rhs) const
	{
		return lhs["name"].t_str().str() < rhs["name"].t_str().str();
	}
};

class preferences_parent_dialog : public gui::dialog
{
public:
	preferences_parent_dialog(CVideo &video) : dialog(video, _("Preferences"),"",gui::CLOSE_ONLY),
		clear_buttons_(false) {}
	~preferences_parent_dialog() {write_preferences();}
	void action(gui::dialog_process_info &info)
	{
		if(clear_buttons_) {
			info.clear_buttons();
			clear_buttons_ = false;
		}
	}
	void clear_buttons() { clear_buttons_ = true; }
private:
	bool clear_buttons_;
};

class preferences_dialog : public gui::preview_pane
{
public:
	preferences_dialog(CVideo& video, const config& game_cfg);

	virtual sdl_handler_vector handler_members();
private:

	void process_event();
	bool left_side() const { return false; }
	void set_selection(int index);
	void update_location(SDL_Rect const &rect);
	const config* get_advanced_pref() const;
	void set_advanced_menu();
	void sort_advanced_preferences();
	void set_friends_menu();
	std::vector<std::string> friends_names_;
	std::vector<std::string> color_ids_;

	gui::slider music_slider_, sound_slider_, UI_sound_slider_, bell_slider_,
	            scroll_slider_, chat_lines_slider_, buffer_size_slider_,
	            idle_anim_slider_, autosavemax_slider_, advanced_slider_;

	gui::list_slider<double> turbo_slider_;
	gui::button fullscreen_button_, turbo_button_, show_ai_moves_button_,
			interrupt_when_ally_sighted_button_,
			show_grid_button_, save_replays_button_, delete_saves_button_,
			show_lobby_joins_button1_,
			show_lobby_joins_button2_,
			show_lobby_joins_button3_,
			sort_list_by_group_button_, iconize_list_button_,
			remember_pw_button_, mp_server_search_button_,
			friends_list_button_, friends_back_button_,
			friends_add_friend_button_, friends_add_ignore_button_,
			friends_remove_button_, show_floating_labels_button_,
			turn_dialog_button_, whiteboard_on_start_button_,
			hide_whiteboard_button_, turn_bell_button_, show_team_colors_button_,
			show_haloing_button_, video_mode_button_,
			theme_button_,
			hotkeys_button_, colors_button_,
			cache_button_,
			advanced_button_, sound_button_,
			music_button_, chat_timestamp_button_,
			advanced_sound_button_, normal_sound_button_,
			display_button_,
			UI_sound_button_, sample_rate_button1_,
			sample_rate_button2_, sample_rate_button3_,
			confirm_sound_button_, idle_anim_button_,
			standing_anim_button_,
			animate_map_button_,
			disable_auto_move_button_,
			mp_alerts_options_button_,
			advanced_graphics_options_button_,

			// color tab
			orb_colors_defaults_,
			orb_colors_enemy_toggle_,
			orb_colors_ally_toggle_,
			orb_colors_unmoved_toggle_,
			orb_colors_partial_toggle_,
			orb_colors_moved_toggle_;

	std::vector<gui::button> orb_colors_ally_buttons_;
	std::vector<gui::button> orb_colors_enemy_buttons_;
	std::vector<gui::button> orb_colors_moved_buttons_;
	std::vector<gui::button> orb_colors_partial_buttons_;
	std::vector<gui::button> orb_colors_unmoved_buttons_;

	gui::label music_label_, sound_label_, UI_sound_label_, bell_label_,
	           scroll_label_, chat_lines_label_,
	           turbo_slider_label_, sample_rate_label_, buffer_size_label_,
			   idle_anim_slider_label_, autosavemax_slider_label_,
			   advanced_option_label_;
	gui::textbox sample_rate_input_, friends_input_;
	gui::combo advanced_combo_;

	unsigned slider_label_width_;

	gui::menu advanced_, friends_;
	int advanced_selection_, friends_selection_;

	enum TAB {	GENERAL_TAB, DISPLAY_TAB, SOUND_TAB, MULTIPLAYER_TAB, ADVANCED_TAB,
				/*extra tab*/
				COLOR_TAB, ADVANCED_SOUND_TAB, FRIENDS_TAB};
	TAB tab_;
	CVideo &video_;
	const config& game_cfg_;
	std::vector<config> adv_preferences_cfg_;
public:
	util::scoped_ptr<preferences_parent_dialog> parent;
};

//change
preferences_dialog::preferences_dialog(CVideo& video, const config& game_cfg)
	: gui::preview_pane(video),
	  friends_names_(),
	  color_ids_(),
	  music_slider_(video), sound_slider_(video),
	  UI_sound_slider_(video), bell_slider_(video),
	  scroll_slider_(video),
	  chat_lines_slider_(video), buffer_size_slider_(video),
	  idle_anim_slider_(video), autosavemax_slider_(video),
	  advanced_slider_(video),
	  turbo_slider_(video),


	  fullscreen_button_(video, _("Full screen"), gui::button::TYPE_CHECK),
	  turbo_button_(video, _("Accelerated speed"), gui::button::TYPE_CHECK),
	  show_ai_moves_button_(video, _("Skip AI moves"), gui::button::TYPE_CHECK),
	  interrupt_when_ally_sighted_button_(video, _("Interrupt move when an ally is sighted"), gui::button::TYPE_CHECK),
	  show_grid_button_(video, _("Show grid"), gui::button::TYPE_CHECK),
	  save_replays_button_(video, _("Save replays at the end of scenarios"), gui::button::TYPE_CHECK),
	  delete_saves_button_(video, _("Delete auto-saves at the end of scenarios"), gui::button::TYPE_CHECK),
	  show_lobby_joins_button1_(video, _("Do not show lobby joins"), gui::button::TYPE_RADIO),
	  show_lobby_joins_button2_(video, _("Show lobby joins of friends only"), gui::button::TYPE_RADIO),
	  show_lobby_joins_button3_(video, _("Show all lobby joins"), gui::button::TYPE_RADIO),
	  sort_list_by_group_button_(video, _("Sort lobby list"), gui::button::TYPE_CHECK),
	  iconize_list_button_(video, _("Iconize lobby list"), gui::button::TYPE_CHECK),
	  remember_pw_button_(video, _("Save password to preferences (plain text)"), gui::button::TYPE_CHECK),
	  mp_server_search_button_(video, _("Set Path to wesnothd")),
	  friends_list_button_(video, _("Friends List")),
	  friends_back_button_(video, _("Multiplayer Options")),
	  friends_add_friend_button_(video, _("Add As Friend")),
	  friends_add_ignore_button_(video, _("Add As Ignore")),
	  friends_remove_button_(video, _("Remove")),
	  show_floating_labels_button_(video, _("Show floating labels"), gui::button::TYPE_CHECK),
	  turn_dialog_button_(video, _("Turn dialog"), gui::button::TYPE_CHECK),
	  whiteboard_on_start_button_(video, _("Enable planning mode on start"), gui::button::TYPE_CHECK),
	  hide_whiteboard_button_(video, _("Hide allies’ plans by default"), gui::button::TYPE_CHECK),
	  turn_bell_button_(video, _("Turn bell"), gui::button::TYPE_CHECK),
	  show_team_colors_button_(video, _("Show team colors"), gui::button::TYPE_CHECK),
	  show_haloing_button_(video, _("Show haloing effects"), gui::button::TYPE_CHECK),
	  video_mode_button_(video, _("Change Resolution")),
	  theme_button_(video, _("Theme")),
	  hotkeys_button_(video, _("Hotkeys")),
	  colors_button_(video, _("Colors")),
	  cache_button_(video, _("Cache")),
	  advanced_button_(video, "", gui::button::TYPE_CHECK),
	  sound_button_(video, _("Sound effects"), gui::button::TYPE_CHECK),
	  music_button_(video, _("Music"), gui::button::TYPE_CHECK),
	  chat_timestamp_button_(video, _("Chat timestamping"), gui::button::TYPE_CHECK),
	  advanced_sound_button_(video, _("sound^Advanced Options")),
	  normal_sound_button_(video, _("sound^Standard Options")),
	  display_button_(video, _("colors^Display")),
	  UI_sound_button_(video, _("User interface sounds"), gui::button::TYPE_CHECK),
	  sample_rate_button1_(video, "22050", gui::button::TYPE_RADIO),
	  sample_rate_button2_(video, "44100", gui::button::TYPE_RADIO),
	  sample_rate_button3_(video, _("Custom"), gui::button::TYPE_RADIO),
	  confirm_sound_button_(video, _("Apply")),
	  idle_anim_button_(video, _("Show unit idle animations"), gui::button::TYPE_CHECK),
	  standing_anim_button_(video, _("Show unit standing animations"), gui::button::TYPE_CHECK),
	  animate_map_button_(video, _("Animate map"), gui::button::TYPE_CHECK),
	  disable_auto_move_button_(video, _("Disable automatic moves"), gui::button::TYPE_CHECK),
	  mp_alerts_options_button_(video, _("Alerts")),
	  advanced_graphics_options_button_(video, _("Advanced")),

	  // Colors tab buttons
	  orb_colors_defaults_(video, _("Defaults")),
	  orb_colors_enemy_toggle_(video, _("Show enemy orb"), gui::button::TYPE_CHECK),
	  orb_colors_ally_toggle_(video, _("Show ally orb"), gui::button::TYPE_CHECK),
	  orb_colors_unmoved_toggle_(video, _("Show unmoved orb"), gui::button::TYPE_CHECK),
	  orb_colors_partial_toggle_(video, _("Show partial moved orb"), gui::button::TYPE_CHECK),
	  orb_colors_moved_toggle_(video, _("Show moved orb"), gui::button::TYPE_CHECK),

	  //colors tab buttons
	  orb_colors_ally_buttons_(),
	  orb_colors_enemy_buttons_(),
	  orb_colors_moved_buttons_(),
	  orb_colors_partial_buttons_(),
	  orb_colors_unmoved_buttons_(),

	  // Sound tab labels
	  music_label_(video, _("Volume:"), font::SIZE_SMALL),
	  sound_label_(video, _("Volume:"), font::SIZE_SMALL),
	  UI_sound_label_(video, _("Volume:"), font::SIZE_SMALL),
	  bell_label_(video, _("Volume:"), font::SIZE_SMALL),

	  scroll_label_(video, _("Scroll speed:")),
	  chat_lines_label_(video, ""),
	  turbo_slider_label_(video, "", font::SIZE_SMALL ),
	  sample_rate_label_(video, _("Sample rate (Hz):")), buffer_size_label_(video, ""),
	  idle_anim_slider_label_(video, _("Frequency:"), font::SIZE_SMALL ),
	  autosavemax_slider_label_(video, "", font::SIZE_SMALL),
	  advanced_option_label_(video, "", font::SIZE_SMALL),

	  sample_rate_input_(video, 70),
	  friends_input_(video, 170),

	  advanced_combo_(video, std::vector<std::string>()),

	  slider_label_width_(0),
	  advanced_(video,std::vector<std::string>(),false,-1,-1,NULL,&gui::menu::bluebg_style),
	  friends_(video,std::vector<std::string>(),false,-1,-1,NULL,&gui::menu::bluebg_style),

	  advanced_selection_(-1),
	  friends_selection_(-1),

	  tab_(GENERAL_TAB), video_(video), game_cfg_(game_cfg), adv_preferences_cfg_(), parent(NULL)
{
	sort_advanced_preferences();

	set_measurements(preferences::width, preferences::height);


	sound_button_.set_check(sound_on());
	sound_button_.set_help_string(_("Sound effects on/off"));
	sound_slider_.set_min(0);
	sound_slider_.set_max(128);
	sound_slider_.set_value(sound_volume());
	sound_slider_.set_help_string(_("Change the sound effects volume"));

	music_button_.set_check(music_on());
	music_button_.set_help_string(_("Music on/off"));
	music_slider_.set_min(0);
	music_slider_.set_max(128);
	music_slider_.set_value(music_volume());
	music_slider_.set_help_string(_("Change the music volume"));

	orb_colors_ally_toggle_.set_check(preferences::show_allied_orb());
	orb_colors_enemy_toggle_.set_check(preferences::show_enemy_orb());
	orb_colors_moved_toggle_.set_check(preferences::show_moved_orb());
	orb_colors_partial_toggle_.set_check(preferences::show_partial_orb());
	orb_colors_unmoved_toggle_.set_check(preferences::show_unmoved_orb());

	const std::map<std::string, t_string>& colors = game_config::team_rgb_name;
	std::map<std::string, t_string>::const_iterator colors_it;
	for (colors_it = colors.begin(); colors_it != colors.end(); ++colors_it) {

		const std::string& color_id = colors_it->first;
		const t_string& color_name  = colors_it->second;

		if (color_id.substr(0,4) != "orb_")
			continue;

		color_ids_.push_back(color_id);

		std::string image_path = "misc/orb"; //game_config::images::orb;
		std::string image_path_suffix = "~RC(magenta>" + color_id + ")";

		gui::button color_radio_button(video, "", gui::button::TYPE_IMAGE, image_path, gui::button::MINIMUM_SPACE);
		color_radio_button.set_tooltip_string(color_name);
		color_radio_button.set_help_string(color_name);
		color_radio_button.set_image_path_suffix(image_path_suffix);

		color_radio_button.set_check(color_id == preferences::allied_color());
		orb_colors_ally_buttons_.push_back(color_radio_button);

		color_radio_button.set_check(color_id == preferences::enemy_color());
		orb_colors_enemy_buttons_.push_back(color_radio_button);

		color_radio_button.set_check(color_id == preferences::moved_color());
		orb_colors_moved_buttons_.push_back(color_radio_button);

		color_radio_button.set_check(color_id == preferences::partial_color());
		orb_colors_partial_buttons_.push_back(color_radio_button);

		color_radio_button.set_check(color_id == preferences::unmoved_color());
		orb_colors_unmoved_buttons_.push_back(color_radio_button);
	}

	// bell volume slider
	bell_slider_.set_min(0);
	bell_slider_.set_max(128);
	bell_slider_.set_value(bell_volume());
	bell_slider_.set_help_string(_("Change the bell volume"));

	UI_sound_button_.set_check(UI_sound_on());
	UI_sound_button_.set_help_string(_("Turn menu and button sounds on/off"));
	UI_sound_slider_.set_min(0);
	UI_sound_slider_.set_max(128);
	UI_sound_slider_.set_value(UI_volume());
	UI_sound_slider_.set_help_string(_("Change the sound volume for button clicks, etc."));

	sample_rate_label_.set_help_string(_("Change the sample rate"));
	std::string rate = lexical_cast<std::string>(sample_rate());
	if (rate == "22050")
		sample_rate_button1_.set_check(true);
	else if (rate == "44100")
		sample_rate_button2_.set_check(true);
	else
		sample_rate_button3_.set_check(true);
	sample_rate_input_.set_text(rate);
	sample_rate_input_.set_help_string(_("User defined sample rate"));
	sample_rate_input_.enable(sample_rate_button3_.checked());
	confirm_sound_button_.enable(sample_rate_button3_.checked());

	buffer_size_slider_.set_min(0);
	buffer_size_slider_.set_max(3);
	int v = sound_buffer_size()/512 - 1;
	buffer_size_slider_.set_value(v);
	//avoid sound reset the first time we load advanced sound
	buffer_size_slider_.value_change();
	buffer_size_slider_.set_help_string(_("Change the buffer size"));
	std::stringstream buf;
	buf << _("Buffer size: ") << sound_buffer_size();
	buffer_size_label_.set_text(buf.str());
	buffer_size_label_.set_help_string(_("Change the buffer size"));

	scroll_slider_.set_min(1);
	scroll_slider_.set_max(100);
	scroll_slider_.set_value(scroll_speed());
	scroll_slider_.set_help_string(_("Change the speed of scrolling around the map"));

	chat_lines_slider_.set_min(1);
	chat_lines_slider_.set_max(20);
	chat_lines_slider_.set_value(chat_lines());
	chat_lines_slider_.set_help_string(_("Set the amount of chat lines shown"));
	// Have the tooltip appear over the static "Chat lines" label, too.
	chat_lines_label_.set_help_string(_("Set the amount of chat lines shown"));

	chat_timestamp_button_.set_check(chat_timestamping());
	chat_timestamp_button_.set_help_string(_("Add a timestamp to chat messages"));

	fullscreen_button_.set_check(fullscreen());
	fullscreen_button_.set_help_string(_("Choose whether the game should run full screen or in a window"));

	turbo_button_.set_check(turbo());
	turbo_button_.set_help_string(_("Make units move and fight faster"));

	std::vector< double > turbo_items;
	turbo_items.push_back(0.25);
	turbo_items.push_back(0.5);
	turbo_items.push_back(0.75);
	turbo_items.push_back(1);
	turbo_items.push_back(1.25);
	turbo_items.push_back(1.5);
	turbo_items.push_back(1.75);
	turbo_items.push_back(2);
	turbo_items.push_back(3);
	turbo_items.push_back(4);
	turbo_items.push_back(8);
	turbo_items.push_back(16);
	turbo_slider_.set_items(turbo_items);
	if(!turbo_slider_.select_item(turbo_speed())) {
		turbo_slider_.select_item(1);
	}
	turbo_slider_.set_help_string(_("Units move and fight speed"));

	idle_anim_button_.set_check(idle_anim());
	idle_anim_button_.set_help_string(_("Play short random animations for idle units"));

	standing_anim_button_.set_check(show_standing_animations());
	standing_anim_button_.set_help_string(_("Continuously animate standing units in the battlefield"));

	animate_map_button_.set_check(animate_map());
	animate_map_button_.set_help_string(_("Display animated terrain graphics"));

	// exponential scale (2^(n/10))
	idle_anim_slider_.set_min(-40);
	idle_anim_slider_.set_max(30);
	idle_anim_slider_.set_value(idle_anim_rate());
	idle_anim_slider_.set_help_string(_("Set the frequency of unit idle animations"));

	autosavemax_slider_.set_min(0);
	autosavemax_slider_.set_max(preferences::INFINITE_AUTO_SAVES);
	autosavemax_slider_.set_value(autosavemax());
	autosavemax_slider_.set_help_string(_("Set maximum number of automatic saves to be retained"));


	show_ai_moves_button_.set_check(!show_ai_moves());
	show_ai_moves_button_.set_help_string(_("Do not animate AI units moving"));

	interrupt_when_ally_sighted_button_.set_check(interrupt_when_ally_sighted());
	interrupt_when_ally_sighted_button_.set_help_string(_("Sighting an allied unit interrupts your unit’s movement"));

	save_replays_button_.set_check(save_replays());
	save_replays_button_.set_help_string(_("Saves replays of games on victory in all modes and defeat in multiplayer"));

	delete_saves_button_.set_check(delete_saves());
	delete_saves_button_.set_help_string(_("Deletes previous auto-saves on victory in all modes and defeat in multiplayer"));
	show_grid_button_.set_check(grid());
	show_grid_button_.set_help_string(_("Overlay a grid onto the map"));

	sort_list_by_group_button_.set_check(sort_list());
	sort_list_by_group_button_.set_help_string(_("Sort the player list in the lobby by player groups"));

	iconize_list_button_.set_check(iconize_list());
	iconize_list_button_.set_help_string(_("Show icons in front of the player names in the lobby"));

	remember_pw_button_.set_check(remember_password());
	remember_pw_button_.set_help_string(_("Uncheck to delete the saved password (on exit)"));

	int lj = lobby_joins();
	show_lobby_joins_button1_.set_check(lj == SHOW_NONE);
	show_lobby_joins_button1_.set_help_string(_("Do not show messages about players joining the multiplayer lobby"));
	show_lobby_joins_button2_.set_check(lj == SHOW_FRIENDS);
	show_lobby_joins_button2_.set_help_string(_("Show messages about your friends joining the multiplayer lobby"));
	show_lobby_joins_button3_.set_check(lj == SHOW_ALL);
	show_lobby_joins_button3_.set_help_string(_("Show messages about all players joining the multiplayer lobby"));

	mp_server_search_button_.set_help_string(_("Find and set path to MP server to host LAN games"));
	friends_list_button_.set_help_string(_("View and edit your friends and ignores list"));
	friends_back_button_.set_help_string(_("Back to the multiplayer options"));
	friends_add_friend_button_.set_help_string(_("Add this username to your friends list (add optional notes, e.g., 'player_name notes on friend')"));
	friends_add_ignore_button_.set_help_string(_("Add this username to your ignores list (add optional reason, e.g., 'player_name reason ignored')"));
	friends_remove_button_.set_help_string(_("Remove this username from your list"));

	friends_input_.set_text("");
	friends_input_.set_help_string(_("Insert a username"));

	show_floating_labels_button_.set_check(show_floating_labels());
	show_floating_labels_button_.set_help_string(_("Show text above a unit when it is hit to display damage inflicted"));

	video_mode_button_.set_help_string(_("Change the resolution the game runs at"));
	theme_button_.set_help_string(_("Change the theme the game runs with"));

	turn_dialog_button_.set_check(turn_dialog());
	turn_dialog_button_.set_help_string(_("Display a dialog at the beginning of your turn"));

	whiteboard_on_start_button_.set_check(enable_whiteboard_mode_on_start());
	whiteboard_on_start_button_.set_help_string(_("Activates Planning Mode on game start"));

	hide_whiteboard_button_.set_check(hide_whiteboard());
	hide_whiteboard_button_.set_help_string(_("Hides allies’ Planning Mode plans in multiplayer games"));

	turn_bell_button_.set_check(turn_bell());
	turn_bell_button_.set_help_string(_("Play a bell sound at the beginning of your turn"));

	show_team_colors_button_.set_check(show_side_colors());
	show_team_colors_button_.set_help_string(_("Show a colored circle around the base of each unit to show which side it is on"));

	show_haloing_button_.set_check(show_haloes());
	show_haloing_button_.set_help_string(_("Use graphical special effects (may be slower)"));

	hotkeys_button_.set_help_string(_("View and configure keyboard shortcuts"));
	colors_button_.set_help_string(_("Adjust orb colors"));
	cache_button_.set_help_string(_("Manage the game WML cache"));

	disable_auto_move_button_.set_check(disable_auto_moves());
	disable_auto_move_button_.set_help_string(_("Do not allow automatic movements at the begining of a turn"));

	//mp_alerts_options_button_.set_help_string(_("Configure the sounds played in the mp lobby and setup screens"));
	//^ The gui 1 tooltips don't play nice with gui 2 tool tips so we have to comment this out.

	set_advanced_menu();
	set_friends_menu();
}

sdl_handler_vector preferences_dialog::handler_members()
{
	sdl_handler_vector h;
	h.push_back(&music_slider_);
	h.push_back(&sound_slider_);
	h.push_back(&bell_slider_);
	h.push_back(&UI_sound_slider_);
	h.push_back(&scroll_slider_);
	h.push_back(&chat_lines_slider_);
	h.push_back(&turbo_slider_);
	h.push_back(&idle_anim_slider_);
	h.push_back(&autosavemax_slider_);
	h.push_back(&buffer_size_slider_);
	h.push_back(&advanced_slider_);
	h.push_back(&fullscreen_button_);
	h.push_back(&turbo_button_);
	h.push_back(&idle_anim_button_);
	h.push_back(&standing_anim_button_);
	h.push_back(&animate_map_button_);
	h.push_back(&show_ai_moves_button_);
	h.push_back(&interrupt_when_ally_sighted_button_);
	h.push_back(&save_replays_button_);
	h.push_back(&delete_saves_button_);
	h.push_back(&show_grid_button_);
	h.push_back(&sort_list_by_group_button_);
	h.push_back(&iconize_list_button_);
	h.push_back(&remember_pw_button_);
	h.push_back(&show_lobby_joins_button1_);
	h.push_back(&show_lobby_joins_button2_);
	h.push_back(&show_lobby_joins_button3_);
	h.push_back(&mp_server_search_button_);
	h.push_back(&friends_list_button_);
	h.push_back(&friends_back_button_);
	h.push_back(&friends_add_friend_button_);
	h.push_back(&friends_add_ignore_button_);
	h.push_back(&friends_remove_button_);
	h.push_back(&friends_input_);
	h.push_back(&advanced_combo_);
	h.push_back(&show_floating_labels_button_);
	h.push_back(&turn_dialog_button_);
	h.push_back(&whiteboard_on_start_button_);
	h.push_back(&hide_whiteboard_button_);
	h.push_back(&turn_bell_button_);
	h.push_back(&UI_sound_button_);
	h.push_back(&show_team_colors_button_);
	h.push_back(&show_haloing_button_);
	h.push_back(&video_mode_button_);
	h.push_back(&theme_button_);
	h.push_back(&hotkeys_button_);
	h.push_back(&colors_button_);
	h.push_back(&cache_button_);
	h.push_back(&advanced_button_);
	h.push_back(&sound_button_);
	h.push_back(&music_button_);
	h.push_back(&chat_timestamp_button_);
	h.push_back(&advanced_sound_button_);
	h.push_back(&normal_sound_button_);
	h.push_back(&sample_rate_button1_);
	h.push_back(&sample_rate_button2_);
	h.push_back(&sample_rate_button3_);
	h.push_back(&confirm_sound_button_);
	h.push_back(&music_label_);
	h.push_back(&sound_label_);
	h.push_back(&bell_label_);
	h.push_back(&UI_sound_label_);
	h.push_back(&scroll_label_);
	h.push_back(&turbo_slider_label_);
	h.push_back(&idle_anim_slider_label_);
	h.push_back(&autosavemax_slider_label_);
	h.push_back(&advanced_option_label_);
	h.push_back(&chat_lines_label_);
	h.push_back(&sample_rate_label_);
	h.push_back(&buffer_size_label_);
	h.push_back(&sample_rate_input_);
	h.push_back(&advanced_);
	h.push_back(&friends_);
	h.push_back(&disable_auto_move_button_);
	h.push_back(&mp_alerts_options_button_);
	h.push_back(&advanced_graphics_options_button_);

	// Colors tab
	for (unsigned i = 0; i < color_ids_.size(); i++) {
		h.push_back(&orb_colors_ally_buttons_[i]);
		h.push_back(&orb_colors_enemy_buttons_[i]);
		h.push_back(&orb_colors_moved_buttons_[i]);
		h.push_back(&orb_colors_partial_buttons_[i]);
		h.push_back(&orb_colors_unmoved_buttons_[i]);
	}

	h.push_back(&orb_colors_ally_toggle_);
	h.push_back(&orb_colors_enemy_toggle_);
	h.push_back(&orb_colors_moved_toggle_);
	h.push_back(&orb_colors_partial_toggle_);
	h.push_back(&orb_colors_unmoved_toggle_);
	h.push_back(&orb_colors_defaults_);
	h.push_back(&display_button_);

	return h;
}

void preferences_dialog::update_location(SDL_Rect const &rect)
{
	bg_register(rect);


	const int right_border = font::relative_size(10);
	const int horizontal_padding = 25;
	// please also check 800x480 resolution if you change these spacings
	const int top_border = 10;
	const int bottom_border = 10;
	const int short_interline = 21;
	const int item_interline = 40;
	const int bottom_row_y = rect.y + rect.h - bottom_border;

	// General tab
	int ypos = rect.y + top_border;
	scroll_label_.set_location(rect.x, ypos);
	SDL_Rect scroll_rect = sdl::create_rect(rect.x + scroll_label_.width()
			, ypos
			, rect.w - scroll_label_.width() - right_border
			, 0);

	scroll_slider_.set_location(scroll_rect);
	ypos += item_interline; turbo_button_.set_location(rect.x, ypos);
	ypos += short_interline; turbo_slider_label_.set_location(rect.x + horizontal_padding, ypos);
	ypos += short_interline;
	SDL_Rect turbo_rect = sdl::create_rect(rect.x + horizontal_padding
			, ypos
			, rect.w - horizontal_padding - right_border
			, 0);

	turbo_slider_.set_location(turbo_rect);
	ypos += item_interline; show_ai_moves_button_.set_location(rect.x, ypos);
	ypos += short_interline; disable_auto_move_button_.set_location(rect.x, ypos);
	ypos += short_interline; turn_dialog_button_.set_location(rect.x, ypos);
	ypos += short_interline; whiteboard_on_start_button_.set_location(rect.x, ypos);
	ypos += short_interline; hide_whiteboard_button_.set_location(rect.x, ypos);
	ypos += short_interline; interrupt_when_ally_sighted_button_.set_location(rect.x, ypos);
	ypos += item_interline; save_replays_button_.set_location(rect.x, ypos);
	ypos += short_interline; delete_saves_button_.set_location(rect.x, ypos);
	ypos += short_interline; autosavemax_slider_label_.set_location(rect.x + horizontal_padding, ypos);
	SDL_Rect autosavemax_rect = sdl::create_rect(rect.x + horizontal_padding
			, ypos + short_interline
			, rect.w - horizontal_padding - right_border
			, 0);

	autosavemax_slider_.set_location(autosavemax_rect);
	hotkeys_button_.set_location(rect.x, bottom_row_y - hotkeys_button_.height());
	cache_button_.set_location(rect.x + hotkeys_button_.width() + 10, bottom_row_y - cache_button_.height());

	// Display tab
	ypos = rect.y + top_border;
	fullscreen_button_.set_location(rect.x, ypos);

	ypos += item_interline; show_floating_labels_button_.set_location(rect.x, ypos);
	ypos += item_interline; show_haloing_button_.set_location(rect.x, ypos);
	ypos += item_interline; show_team_colors_button_.set_location(rect.x, ypos);
	ypos += item_interline; show_grid_button_.set_location(rect.x, ypos);

	ypos += item_interline; animate_map_button_.set_location(rect.x, ypos);
	ypos += short_interline; standing_anim_button_.set_location(rect.x, ypos);
	ypos += short_interline; idle_anim_button_.set_location(rect.x, ypos);
	ypos += short_interline;
	idle_anim_slider_label_.set_location(rect.x + horizontal_padding, ypos);
	SDL_Rect idle_anim_rect = sdl::create_rect(rect.x + horizontal_padding + idle_anim_slider_label_.width()
			, ypos
			, rect.w - horizontal_padding - idle_anim_slider_label_.width() - right_border
			, 0);
	idle_anim_slider_.set_location(idle_anim_rect);

	video_mode_button_.set_location(rect.x, bottom_row_y - video_mode_button_.height());
	theme_button_.set_location(rect.x + video_mode_button_.width() + 10,
	                           bottom_row_y - theme_button_.height());
	colors_button_.set_location(rect.x + video_mode_button_.width() + theme_button_.width() + 20, bottom_row_y - colors_button_.height());

	advanced_graphics_options_button_.set_location(rect.x, bottom_row_y - video_mode_button_.height() - advanced_graphics_options_button_.height() - 10);

	// Color tab
	const int width = 28; // orb_colors_ally_buttons_[0].width();
	const unsigned color_number = color_ids_.size();
	const unsigned number = std::max(color_number, 2u);

	const int orb_x_offset = ( (rect.w - width*number) / number ) * 2;

	int xpos = rect.x;
	ypos = rect.y + top_border;
	orb_colors_unmoved_toggle_.set_location(xpos, ypos);
	ypos += item_interline - 10;
	xpos -= orb_x_offset;
	xpos += horizontal_padding;
	BOOST_FOREACH(gui::button& button, orb_colors_unmoved_buttons_) {
		xpos += orb_x_offset;
		button.set_location(xpos, ypos);
	}

	xpos = rect.x;
	ypos += item_interline;
	orb_colors_partial_toggle_.set_location(xpos, ypos);
	ypos += item_interline - 10;
	xpos -= orb_x_offset;
	xpos += horizontal_padding;
	BOOST_FOREACH(gui::button& button, orb_colors_partial_buttons_) {
		xpos += orb_x_offset;
		button.set_location(xpos, ypos);
	}

	xpos = rect.x;
	ypos += item_interline;
	orb_colors_moved_toggle_.set_location(xpos, ypos);
	ypos += item_interline - 10;
	xpos -= orb_x_offset;
	xpos += horizontal_padding;
	BOOST_FOREACH(gui::button& button, orb_colors_moved_buttons_) {
		xpos += orb_x_offset;
		button.set_location(xpos, ypos);
	}

	xpos = rect.x;
	ypos += item_interline;
	orb_colors_ally_toggle_.set_location(xpos, ypos);
	ypos += item_interline - 10;
	xpos -= orb_x_offset;
	xpos += horizontal_padding;
	BOOST_FOREACH(gui::button& button, orb_colors_ally_buttons_) {
		xpos += orb_x_offset;
		button.set_location(xpos, ypos);
	}

	xpos = rect.x;
	ypos += item_interline;
	orb_colors_enemy_toggle_.set_location(xpos, ypos);
	ypos += item_interline - 10;
	xpos -= orb_x_offset;
	xpos += horizontal_padding;
	BOOST_FOREACH(gui::button& button, orb_colors_enemy_buttons_) {
		xpos += orb_x_offset;
		button.set_location(xpos, ypos);
	}

	display_button_.set_location(rect.x, bottom_row_y - display_button_.height());
	orb_colors_defaults_.set_location(rect.x + display_button_.width() + 10, bottom_row_y - orb_colors_defaults_.height());

	// Sound tab
	slider_label_width_ = std::max<unsigned>(music_label_.width(), sound_label_.width());
	slider_label_width_ = std::max<unsigned>(slider_label_width_, bell_label_.width());
	slider_label_width_ = std::max<unsigned>(slider_label_width_, UI_sound_label_.width());
	ypos = rect.y + top_border;
	sound_button_.set_location(rect.x, ypos);

	ypos += short_interline;
	sound_label_.set_location(rect.x + horizontal_padding, ypos);
	const SDL_Rect sound_rect = sdl::create_rect(rect.x + horizontal_padding + slider_label_width_
			, ypos
			, rect.w - slider_label_width_ - horizontal_padding - right_border
			, 0);
	sound_slider_.set_location(sound_rect);

	ypos += item_interline;
	music_button_.set_location(rect.x, ypos);

	ypos += short_interline;
	music_label_.set_location(rect.x + horizontal_padding, ypos);
	const SDL_Rect music_rect = sdl::create_rect(rect.x + horizontal_padding + slider_label_width_
			, ypos
			, rect.w - slider_label_width_ - horizontal_padding - right_border
			, 0);
	music_slider_.set_location(music_rect);

	ypos += item_interline; //Bell slider
	turn_bell_button_.set_location(rect.x, ypos);
	ypos += short_interline;
	bell_label_.set_location(rect.x + horizontal_padding, ypos);
	const SDL_Rect bell_rect = sdl::create_rect(rect.x + horizontal_padding + slider_label_width_
			, ypos
			, rect.w - slider_label_width_ - horizontal_padding - right_border
			, 0);
	bell_slider_.set_location(bell_rect);

	ypos += item_interline; //UI sound slider
	UI_sound_button_.set_location(rect.x, ypos);
	ypos += short_interline;
	UI_sound_label_.set_location(rect.x + horizontal_padding, ypos);
	const SDL_Rect UI_sound_rect = sdl::create_rect(rect.x + horizontal_padding + slider_label_width_
			, ypos
			, rect.w - slider_label_width_ - horizontal_padding - right_border
			, 0);
	UI_sound_slider_.set_location(UI_sound_rect);
	advanced_sound_button_.set_location(rect.x, bottom_row_y - advanced_sound_button_.height());


	//Advanced Sound tab
	ypos = rect.y + top_border;
	sample_rate_label_.set_location(rect.x, ypos);
	ypos += short_interline;
	int h_offset = rect.x + horizontal_padding;
	sample_rate_button1_.set_location(h_offset, ypos);
	ypos += short_interline;
	sample_rate_button2_.set_location(h_offset, ypos);
	ypos += short_interline;
	sample_rate_button3_.set_location(h_offset, ypos);
	h_offset += sample_rate_button3_.width() + 5;
	sample_rate_input_.set_location(h_offset, ypos);
	h_offset += sample_rate_input_.width() + 5;
	confirm_sound_button_.set_location(h_offset, ypos);

	ypos += item_interline;
	buffer_size_label_.set_location(rect.x, ypos);
	ypos += short_interline;
	SDL_Rect buffer_rect = sdl::create_rect(rect.x + horizontal_padding
			, ypos
			, rect.w - horizontal_padding - right_border
			, 0);
	buffer_size_slider_.set_location(buffer_rect);
	ypos += item_interline;
	normal_sound_button_.set_location(rect.x, bottom_row_y - normal_sound_button_.height());


	// Multiplayer tab
	ypos = rect.y + top_border;
	chat_lines_label_.set_location(rect.x, ypos);
	ypos += short_interline;
	SDL_Rect chat_lines_rect = sdl::create_rect(rect.x + horizontal_padding
			, ypos
			, rect.w - horizontal_padding - right_border
			, 0);
	chat_lines_slider_.set_location(chat_lines_rect);
	ypos += item_interline; chat_timestamp_button_.set_location(rect.x, ypos);
	ypos += item_interline; remember_pw_button_.set_location(rect.x, ypos);
	ypos += item_interline; sort_list_by_group_button_.set_location(rect.x, ypos);
	ypos += item_interline; iconize_list_button_.set_location(rect.x, ypos);

	ypos += item_interline; show_lobby_joins_button1_.set_location(rect.x, ypos);
	ypos += short_interline; show_lobby_joins_button2_.set_location(rect.x, ypos);
	ypos += short_interline; show_lobby_joins_button3_.set_location(rect.x, ypos);

	friends_list_button_.set_location(rect.x, bottom_row_y - friends_list_button_.height());

	mp_server_search_button_.set_location(rect.x + 10 + friends_list_button_.width(), bottom_row_y - mp_server_search_button_.height());
	mp_alerts_options_button_.set_location(mp_server_search_button_.location().x + mp_server_search_button_.width() + 10, bottom_row_y - mp_alerts_options_button_.height());

	//Friends tab
	ypos = rect.y + top_border;
	friends_input_.set_location(rect.x,ypos);

	friends_.set_location(rect.x,ypos + item_interline);
	friends_.set_max_height(height() - 100 - friends_back_button_.height());

	int friends_xpos;

	if (friends_.width() > friends_input_.width()) {
		friends_xpos = rect.x+  friends_.width() + 20;
	} else {
		friends_xpos = rect.x+  friends_input_.width() + 20;
	}
	friends_.set_max_width(friends_xpos - rect.x - 1);

	friends_add_friend_button_.set_location(friends_xpos,ypos);
	ypos += short_interline+3; friends_add_ignore_button_.set_location(friends_xpos,ypos);
	ypos += short_interline+3; friends_remove_button_.set_location(friends_xpos,ypos);
	friends_back_button_.set_location(rect.x, bottom_row_y - friends_back_button_.height());

	//Advanced tab
	ypos = rect.y + top_border;
	advanced_.set_location(rect.x,ypos);
	advanced_.set_max_height(height()-100);

	ypos += advanced_.height() + font::relative_size(14);

	advanced_button_.set_location(rect.x,ypos);
	advanced_option_label_.set_location(rect.x,ypos);
	const SDL_Rect advanced_slider_rect = sdl::create_rect(rect.x
			, ypos + short_interline
			, rect.w - right_border
			, 0);
	advanced_slider_.set_location(advanced_slider_rect);
	advanced_combo_.set_location(rect.x,ypos + short_interline);

	set_selection(tab_);
}

void preferences_dialog::process_event()
{
	if (tab_ == GENERAL_TAB) {
		if (turbo_button_.pressed()) {
			set_turbo(turbo_button_.checked());
			turbo_slider_.enable(turbo());
			turbo_slider_label_.enable(turbo());
		}
		if (show_ai_moves_button_.pressed())
			set_show_ai_moves(!show_ai_moves_button_.checked());
		if (disable_auto_move_button_.pressed())
			set_disable_auto_moves(disable_auto_move_button_.checked());
		if (interrupt_when_ally_sighted_button_.pressed())
			set_interrupt_when_ally_sighted(interrupt_when_ally_sighted_button_.checked());
		if (save_replays_button_.pressed())
			set_save_replays(save_replays_button_.checked());
		if (delete_saves_button_.pressed())
			set_delete_saves(delete_saves_button_.checked());
		if (turn_dialog_button_.pressed())
			set_turn_dialog(turn_dialog_button_.checked());
		if (whiteboard_on_start_button_.pressed())
			set_enable_whiteboard_mode_on_start(whiteboard_on_start_button_.checked());
		if (hide_whiteboard_button_.pressed())
			set_hide_whiteboard(hide_whiteboard_button_.checked());
		if (hotkeys_button_.pressed()) {
			show_hotkeys_preferences_dialog(video_);
			parent->clear_buttons();
		}
		if (cache_button_.pressed()) {
			gui2::tgame_cache_options::display(video_);
			parent->clear_buttons();
		}

		set_scroll_speed(scroll_slider_.value());
		set_autosavemax(autosavemax_slider_.value());
		set_turbo_speed(turbo_slider_.item_selected());

		std::stringstream buf;
		buf << _("Speed: ") << turbo_slider_.item_selected();
		turbo_slider_label_.set_text(buf.str());

		std::stringstream buf2;
		if (autosavemax_slider_.value() == preferences::INFINITE_AUTO_SAVES)
			buf2 << _("Maximum auto-saves: ") << _("infinite");
		else
			buf2 << _("Maximum auto-saves: ") << autosavemax_slider_.value();
		autosavemax_slider_label_.set_text(buf2.str());

		return;
	}

	if (tab_ == DISPLAY_TAB) {
		if (show_floating_labels_button_.pressed())
			set_show_floating_labels(show_floating_labels_button_.checked());
		if (video_mode_button_.pressed())
			show_video_mode_dialog(video_);
		if (theme_button_.pressed()) {
			show_theme_dialog(video_);
			parent->clear_buttons();
		}
		if (fullscreen_button_.pressed())
			video_.set_fullscreen(fullscreen_button_.checked());
		if (show_haloing_button_.pressed())
			set_show_haloes(show_haloing_button_.checked());
		if (show_team_colors_button_.pressed())
			set_show_side_colors(show_team_colors_button_.checked());
		if (show_grid_button_.pressed())
			set_grid(show_grid_button_.checked());
		if (animate_map_button_.pressed())
			set_animate_map(animate_map_button_.checked());
		if (idle_anim_button_.pressed()) {
			const bool enable_idle_anim = idle_anim_button_.checked();
			idle_anim_slider_label_.enable(enable_idle_anim);
			idle_anim_slider_.enable(enable_idle_anim);
			set_idle_anim(enable_idle_anim);
			if (!enable_idle_anim)
				idle_anim_slider_.set_value(0);
		}
		if (standing_anim_button_.pressed())
			set_show_standing_animations(standing_anim_button_.checked());

		set_idle_anim_rate(idle_anim_slider_.value());

		if (colors_button_.pressed())
				set_selection(COLOR_TAB);

		if (advanced_graphics_options_button_.pressed())
			show_advanced_graphics_dialog(video_);

		return;
	}

	if (tab_ == COLOR_TAB) {

		if (display_button_.pressed()) {
			set_selection(DISPLAY_TAB);
			return;
		}

		if (orb_colors_defaults_.pressed()) {
			preferences::set_show_allied_orb(game_config::show_ally_orb);
			preferences::set_show_enemy_orb(game_config::show_enemy_orb);
			preferences::set_show_moved_orb(game_config::show_moved_orb);
			preferences::set_show_partial_orb(game_config::show_partial_orb);
			preferences::set_show_unmoved_orb(game_config::show_unmoved_orb);
			orb_colors_ally_toggle_.set_check(preferences::show_allied_orb());
			orb_colors_enemy_toggle_.set_check(preferences::show_enemy_orb());
			orb_colors_moved_toggle_.set_check(preferences::show_moved_orb());
			orb_colors_partial_toggle_.set_check(preferences::show_partial_orb());
			orb_colors_unmoved_toggle_.set_check(preferences::show_unmoved_orb());
			preferences::set_allied_color(game_config::colors::ally_orb_color);
			preferences::set_enemy_color(game_config::colors::enemy_orb_color);
			preferences::set_moved_color(game_config::colors::moved_orb_color);
			preferences::set_unmoved_color(game_config::colors::unmoved_orb_color);
			preferences::set_partial_color(game_config::colors::partial_orb_color);
		}

		if (orb_colors_ally_toggle_.pressed()) {
			preferences::set_show_allied_orb(orb_colors_ally_toggle_.checked());
			return;
		}
		if (orb_colors_enemy_toggle_.pressed()) {
			preferences::set_show_enemy_orb(orb_colors_enemy_toggle_.checked());
			return;
		}
		if (orb_colors_moved_toggle_.pressed()) {
			preferences::set_show_moved_orb(orb_colors_moved_toggle_.checked());
			return;
		}
		if (orb_colors_partial_toggle_.pressed()) {
			preferences::set_show_partial_orb(orb_colors_partial_toggle_.checked());
			return;
		}
		if (orb_colors_unmoved_toggle_.pressed()) {
			preferences::set_show_unmoved_orb(orb_colors_unmoved_toggle_.checked());
			return;
		}

		for (unsigned i = 0; i < color_ids_.size(); i++) {
			if (orb_colors_ally_buttons_[i].pressed())
				preferences::set_allied_color(color_ids_[i]);
			if (orb_colors_enemy_buttons_[i].pressed())
				preferences::set_enemy_color(color_ids_[i]);
			if (orb_colors_moved_buttons_[i].pressed())
				preferences::set_moved_color(color_ids_[i]);
			if (orb_colors_unmoved_buttons_[i].pressed())
				preferences::set_unmoved_color(color_ids_[i]);
			if (orb_colors_partial_buttons_[i].pressed())
				preferences::set_partial_color(color_ids_[i]);

			orb_colors_ally_buttons_[i].set_check(
					preferences::allied_color() == color_ids_[i]);
			orb_colors_enemy_buttons_[i].set_check(
					preferences::enemy_color() == color_ids_[i]);
			orb_colors_moved_buttons_[i].set_check(
					preferences::moved_color() == color_ids_[i]);
			orb_colors_partial_buttons_[i].set_check(
					preferences::partial_color() == color_ids_[i]);
			orb_colors_unmoved_buttons_[i].set_check(
					preferences::unmoved_color() == color_ids_[i]);
		}

		return;
	}

	if (tab_ == SOUND_TAB) {
		if (turn_bell_button_.pressed()) {
			if(!set_turn_bell(turn_bell_button_.checked()))
				turn_bell_button_.set_check(false);
			bell_slider_.enable(turn_bell());
			bell_label_.enable(turn_bell());
		}
		if (sound_button_.pressed()) {
			if(!set_sound(sound_button_.checked()))
				sound_button_.set_check(false);
			sound_slider_.enable(sound_on());
			sound_label_.enable(sound_on());
		}
		if (UI_sound_button_.pressed()) {
			if(!set_UI_sound(UI_sound_button_.checked()))
				UI_sound_button_.set_check(false);
			UI_sound_slider_.enable(UI_sound_on());
			UI_sound_label_.enable(UI_sound_on());
		}
		set_sound_volume(sound_slider_.value());
		set_UI_volume(UI_sound_slider_.value());
		set_bell_volume(bell_slider_.value());

		if (music_button_.pressed()) {
			if(!set_music(music_button_.checked()))
				music_button_.set_check(false);
			music_slider_.enable(music_on());
			music_label_.enable(music_on());
		}
		set_music_volume(music_slider_.value());

		if (advanced_sound_button_.pressed())
			set_selection(ADVANCED_SOUND_TAB);

		return;
	}

	if (tab_ == ADVANCED_SOUND_TAB) {
		bool apply = false;
		std::string rate;

		if (sample_rate_button1_.pressed()) {
			if (sample_rate_button1_.checked()) {
				sample_rate_button2_.set_check(false);
				sample_rate_button3_.set_check(false);
				sample_rate_input_.enable(false);
				confirm_sound_button_.enable(false);
				apply = true;
				rate = "22050";
			} else
				sample_rate_button1_.set_check(true);
		}
		if (sample_rate_button2_.pressed()) {
			if (sample_rate_button2_.checked()) {
				sample_rate_button1_.set_check(false);
				sample_rate_button3_.set_check(false);
				sample_rate_input_.enable(false);
				confirm_sound_button_.enable(false);
				apply = true;
				rate = "44100";
			} else
				sample_rate_button2_.set_check(true);
		}
		if (sample_rate_button3_.pressed()) {
			if (sample_rate_button3_.checked()) {
				sample_rate_button1_.set_check(false);
				sample_rate_button2_.set_check(false);
				sample_rate_input_.enable(true);
				confirm_sound_button_.enable(true);
			} else
				sample_rate_button3_.set_check(true);
		}
		if (confirm_sound_button_.pressed()) {
			apply = true;
			rate = sample_rate_input_.text();
		}

		if (apply)
			try {
			save_sample_rate(lexical_cast<unsigned int>(rate));
			} catch (bad_lexical_cast&) {
			}

		if (buffer_size_slider_.value_change()) {
			const size_t buffer_size = 512 << buffer_size_slider_.value();
			save_sound_buffer_size(buffer_size);
			std::stringstream buf;
			buf << _("Buffer size: ") << buffer_size;
			buffer_size_label_.set_text(buf.str());
		}

		if (normal_sound_button_.pressed())
			set_selection(SOUND_TAB);

		return;
	}

	if (tab_ == MULTIPLAYER_TAB) {
		if (show_lobby_joins_button1_.pressed()) {
			set_lobby_joins(SHOW_NONE);
			show_lobby_joins_button1_.set_check(true);
			show_lobby_joins_button2_.set_check(false);
			show_lobby_joins_button3_.set_check(false);
		} else if (show_lobby_joins_button2_.pressed()) {
			set_lobby_joins(SHOW_FRIENDS);
			show_lobby_joins_button1_.set_check(false);
			show_lobby_joins_button2_.set_check(true);
			show_lobby_joins_button3_.set_check(false);
		} else if (show_lobby_joins_button3_.pressed()) {
			set_lobby_joins(SHOW_ALL);
			show_lobby_joins_button1_.set_check(false);
			show_lobby_joins_button2_.set_check(false);
			show_lobby_joins_button3_.set_check(true);
		}
		if (sort_list_by_group_button_.pressed())
			set_sort_list(sort_list_by_group_button_.checked());
		if (iconize_list_button_.pressed())
			set_iconize_list(iconize_list_button_.checked());
		if (remember_pw_button_.pressed())
			set_remember_password(remember_pw_button_.checked());
		if (chat_timestamp_button_.pressed())
			set_chat_timestamping(chat_timestamp_button_.checked());
		if (friends_list_button_.pressed())
			set_selection(FRIENDS_TAB);

		if (mp_server_search_button_.pressed())
		{
			std::string path = show_wesnothd_server_search(video_);
			if (!path.empty())
			{
				preferences::set_mp_server_program_name(path);
			}
			parent->clear_buttons();
		}

		if (mp_alerts_options_button_.pressed()) {
			show_mp_alerts_dialog(video_);
		}

		set_chat_lines(chat_lines_slider_.value());

		//display currently select amount of chat lines
		std::stringstream buf;
		buf << _("Chat lines: ") << chat_lines_slider_.value();
		chat_lines_label_.set_text(buf.str());

		return;
	}

	if (tab_ == FRIENDS_TAB) {
		if(friends_.double_clicked() || friends_.selection() != friends_selection_) {
			friends_selection_ = friends_.selection();
			std::stringstream ss;
			ss << friends_names_[friends_.selection()];
			if (ss.str() != "(empty list)") friends_input_.set_text(ss.str());
			else friends_input_.set_text("");
		}
		if (friends_back_button_.pressed())
			set_selection(MULTIPLAYER_TAB);

		if (friends_add_friend_button_.pressed()) {
			std::string notes;
			std::string username = friends_input_.text();
			size_t pos = username.find_first_of(' ');

			if(pos != std::string::npos) {
				notes = username.substr(pos + 1);
				username = username.substr(0, pos);
			}

			if (preferences::add_friend(username, notes)) {
				friends_input_.clear();
				set_friends_menu();
			} else {
				gui2::show_transient_error_message(video_, _("Invalid username"));
			}
		}
		if (friends_add_ignore_button_.pressed()) {
			std::string reason;
			std::string username = friends_input_.text();
			size_t pos = username.find_first_of(' ');

			if(pos != std::string::npos) {
				reason = username.substr(pos + 1);
				username = username.substr(0, pos);
			}

			if (preferences::add_ignore(username, reason)) {
				friends_input_.clear();
				set_friends_menu();
			} else {
				gui2::show_transient_error_message(video_, _("Invalid username"));
            }
        }
		if (friends_remove_button_.pressed()) {
			std::string to_remove = friends_input_.text();
			if(to_remove.empty() && friends_.selection() >= 0 && friends_names_[friends_.selection()] != "(empty list)") {
				to_remove = friends_names_[friends_.selection()];
			}
			if(!to_remove.empty()) {
				/** @todo Better to remove from a specific relation. */
				preferences::remove_acquaintance(to_remove);
				friends_input_.clear();
				set_friends_menu();
            }
        }
		return;
	}

	if (tab_ == ADVANCED_TAB) {
		if(advanced_.selection() != advanced_selection_) {
			advanced_selection_ = advanced_.selection();
			const config* const adv = get_advanced_pref();
			if(adv != NULL) {
				const config& pref = *adv;

				std::string description = pref["description"];
				if(description.empty()) {
					description = pref["name"].str();
				}

				std::string value = preferences::get(pref["field"]);

				// Hide all advanced preference controls before unhiding the
				// ones needed for the selection, otherwise we may end up with
				// a "ghost" of a previously visible control glitching through
				// the current one when traversing the list item by item (e.g.
				// with the keyboard.)
				advanced_combo_.hide(true);
				advanced_button_.hide(true);
				advanced_slider_.hide(true);
				advanced_option_label_.hide(true);

				if(pref["type"] == "boolean") {
					advanced_button_.hide(false);
				}

				if(pref["type"] == "int") {
					advanced_option_label_.hide(false);
					advanced_slider_.hide(false);
				}

				if(pref["type"] == "combo") {
					advanced_option_label_.hide(false);
					advanced_combo_.hide(false);
				}

				if(value.empty()) {
					value = pref["default"].str();
				}
				if (pref["type"] == "boolean") {
					advanced_button_.set_width(0);
					advanced_button_.set_label(pref["name"]);
					advanced_button_.set_check(value == "yes");
					advanced_button_.set_help_string(description);
				} else if (pref["type"] == "int") {
					std::stringstream ss;
					ss << pref["name"] << ": " << value;
					advanced_option_label_.set_text(ss.str());
					advanced_option_label_.set_help_string(description);
					advanced_slider_.set_min(pref["min"].to_int());
					advanced_slider_.set_max(pref["max"].to_int());
					advanced_slider_.set_increment(pref["step"].to_int(1));
					advanced_slider_.set_value(lexical_cast<int>(value));
					advanced_slider_.set_help_string(description);
				} else if (pref["type"] == "combo") {
					std::vector<std::string> adv_combo_items;
					int adv_combo_choice = 0;
					BOOST_FOREACH(const config& adv_combo_option, pref.child_range("option"))
					{
						if(adv_combo_option.has_attribute("description")) {
							// The longer description is supposed to be used in the combo
							// box only as a workaround for the main listbox's layout
							// limitations.
							std::ostringstream ss;
							ss << adv_combo_option["name"] << COLUMN_SEPARATOR
							   << adv_combo_option["description"];
							adv_combo_items.push_back(ss.str());
						} else {
							adv_combo_items.push_back(adv_combo_option["name"]);
						}

						if(value == adv_combo_option["id"]) {
							adv_combo_choice = adv_combo_items.size() - 1;
						}
					}
					advanced_combo_.set_items(adv_combo_items);
					advanced_combo_.set_selected(adv_combo_choice);
					advanced_combo_.set_help_string(description);
					advanced_option_label_.set_text(pref["name"].str() + ":");
					advanced_option_label_.set_help_string(description);
				}
			}
		}

		const config* const adv = get_advanced_pref();
		const bool double_click_toggle_boolean = adv
				? advanced_.double_clicked() && (*adv)["type"] == "boolean"
				: false;

		if(advanced_button_.pressed() || double_click_toggle_boolean) {
			bool advanced_button_check = advanced_button_.checked();
			if (double_click_toggle_boolean) {
				if (advanced_button_.checked()) {
					advanced_button_check = false;
				}
				else {
					advanced_button_check = true;
				}
				advanced_button_.set_check(advanced_button_check);
			}
			if(adv != NULL) {
				const config& pref = *adv;
				preferences::set(pref["field"], advanced_button_check);
				set_advanced_menu();

				if(pref["field"] == "color_cursors") {
					set_color_cursors(advanced_button_.checked());
				}

				if(pref["field"] == "disable_notifications" && !advanced_button_check && !desktop::notifications::available()) {
					gui2::show_transient_message(video_,
												 _("Desktop Notifications Unavailable"),
												 _("This build of Wesnoth does not include support for desktop notifications. Check with the packager for your platform, or if compiling yourself, make sure that you have the appropriate dependencies installed."));
				}
			}
		}

		if(advanced_slider_.value_change()) {
			if(adv != NULL) {
				const config& pref = *adv;
				preferences::set(pref["field"],
						str_cast(advanced_slider_.value()));
				set_advanced_menu();
				std::stringstream ss;
				ss << pref["name"] << ": " << advanced_slider_.value();
				advanced_option_label_.set_text(ss.str());
			}
		}

		if(advanced_combo_.changed()) {
			if(adv != NULL) {
				const config& pref = *adv;
				config::const_child_itors options = pref.child_range("option");

				const config* option = NULL;
				int k = 0;

				BOOST_FOREACH(const config& o, options)
				{
					if(advanced_combo_.selected() == k) {
						option = &o;
						break;
					}
					++k;
				}

				if(option) {
					preferences::set(pref["field"], (*option)["id"].str());
					set_advanced_menu();
				}
			}
		}

		return;
	}
}

const config* preferences_dialog::get_advanced_pref() const
{
	if(advanced_selection_ >= 0) {
		const size_t n = static_cast<size_t>(advanced_selection_);
		if (n < adv_preferences_cfg_.size()) {
			return &adv_preferences_cfg_[n];
		}
	}

	return NULL;
}

void preferences_dialog::set_advanced_menu()
{
	std::vector<std::string> advanced_items;
	BOOST_FOREACH(const config &adv, adv_preferences_cfg_)
	{
		std::ostringstream str;
		std::string field = preferences::get(adv["field"]);
		if(field.empty()) {
			field = adv["default"].str();
		}

		if(adv["type"] == "combo") {
			BOOST_FOREACH(const config& optdef, adv.child_range("option"))
			{
				if(field == optdef["id"]) {
					if(optdef.has_attribute("name_short")) {
						field = optdef["name_short"].str();
					} else {
						field = optdef["name"].str();
					}
					break;
				}
			}
		} else if(field == "yes") {
			field = _("yes");
		} else if(field == "no") {
			field = _("no");
		}

		const std::string& label = adv["description"];

		std::string display_label = adv["name"];
		std::string display_field = field;

		// NOTE:
		// The character count limits below only really work with the basic
		// ASCII character set. Some Unicode characters may be rendered wider.
		// Furthermore, the Preferences page selection list may container
		// longer entries that push this listbox further to the right, closer
		// to the dialog's right edge.
		utils::ellipsis_truncate(display_label, 46);
		utils::ellipsis_truncate(display_field, 8);

		// We need the tooltip twice because individual columns have
		// individual tooltips.

		str << display_label << HELP_STRING_SEPARATOR << label
		    << COLUMN_SEPARATOR
		    << display_field << HELP_STRING_SEPARATOR << label;
		advanced_items.push_back(str.str());
	}

	advanced_.set_items(advanced_items,true,true);
}

void preferences_dialog::sort_advanced_preferences()
{
	adv_preferences_cfg_.clear();

	BOOST_FOREACH(const config& adv, game_cfg_.child_range("advanced_preference")) {
		adv_preferences_cfg_.push_back(adv);
	}

	std::sort(adv_preferences_cfg_.begin(), adv_preferences_cfg_.end(), advanced_preferences_sorter());
}

void preferences_dialog::set_friends_menu()
{
	const std::map<std::string, acquaintance>& acquaintances = preferences::get_acquaintances();

	std::vector<std::string> friends_items;
	std::vector<std::string> friends_names;
	std::string const imgpre = IMAGE_PREFIX + std::string("misc/status-");

	for (std::map<std::string, acquaintance>::const_iterator i = acquaintances.begin(); i != acquaintances.end(); ++i)
	{
		std::string image = "friend.png";
		std::string descriptor = _("friend");

		if(i->second.get_status() == "ignore") {
			image = "ignore.png";
			descriptor = _("ignored");
		}

		std::string notes;

		if(!i->second.get_notes().empty()) {
			notes = " (" + i->second.get_notes() + ")";
		}

		friends_items.push_back(imgpre + image + COLUMN_SEPARATOR
				+ i->second.get_nick() + notes + COLUMN_SEPARATOR + descriptor);
		friends_names.push_back(i->first);
	}
	if (friends_items.empty()) {
		friends_items.push_back(_("(empty list)"));
		friends_names.push_back("(empty list)");
	}
	friends_names_ = friends_names;
	friends_.set_items(friends_items,true,true);
}

void preferences_dialog::set_selection(int index)
{
	tab_ = TAB(index);
	set_dirty();
	bg_restore();

	const bool hide_general = tab_ != GENERAL_TAB;
	scroll_label_.hide(hide_general);
	scroll_slider_.hide(hide_general);
	turbo_button_.hide(hide_general);
	turbo_slider_label_.hide(hide_general);
	turbo_slider_.hide(hide_general);
	turbo_slider_label_.enable(turbo());
	turbo_slider_.enable(turbo());
	show_ai_moves_button_.hide(hide_general);
	disable_auto_move_button_.hide(hide_general);
	turn_dialog_button_.hide(hide_general);
	whiteboard_on_start_button_.hide(hide_general);
	hide_whiteboard_button_.hide(hide_general);
	interrupt_when_ally_sighted_button_.hide(hide_general);
	hotkeys_button_.hide(hide_general);
	cache_button_.hide(hide_general);
	save_replays_button_.hide(hide_general);
	delete_saves_button_.hide(hide_general);
	autosavemax_slider_label_.hide(hide_general);
	autosavemax_slider_label_.enable(!hide_general);
	autosavemax_slider_.hide(hide_general);
	autosavemax_slider_.enable(!hide_general);

	const bool hide_display = tab_ != DISPLAY_TAB;
	show_floating_labels_button_.hide(hide_display);
	show_haloing_button_.hide(hide_display);
	fullscreen_button_.hide(hide_display);
	idle_anim_button_.hide(hide_display);
	idle_anim_slider_label_.hide(hide_display);
	idle_anim_slider_label_.enable(idle_anim());
	idle_anim_slider_.hide(hide_display);
	idle_anim_slider_.enable(idle_anim());
	standing_anim_button_.hide(hide_display);
	animate_map_button_.hide(hide_display);
	video_mode_button_.hide(hide_display);
	theme_button_.hide(hide_display);
	show_team_colors_button_.hide(hide_display);
	show_grid_button_.hide(hide_display);
	colors_button_.hide(hide_display);
	advanced_graphics_options_button_.hide(hide_display);

	const bool hide_colors = tab_ != COLOR_TAB;
	orb_colors_ally_toggle_.hide(hide_colors);
	orb_colors_enemy_toggle_.hide(hide_colors);
	orb_colors_moved_toggle_.hide(hide_colors);
	orb_colors_partial_toggle_.hide(hide_colors);
	orb_colors_unmoved_toggle_.hide(hide_colors);
	for (unsigned i = 0; i < color_ids_.size(); i++) {
		orb_colors_ally_buttons_[i].hide(hide_colors);
		orb_colors_enemy_buttons_[i].hide(hide_colors);
		orb_colors_moved_buttons_[i].hide(hide_colors);
		orb_colors_partial_buttons_[i].hide(hide_colors);
		orb_colors_unmoved_buttons_[i].hide(hide_colors);
	}
	display_button_.hide(hide_colors);
	display_button_.enable(!hide_colors);
	orb_colors_defaults_.hide(hide_colors);
	orb_colors_defaults_.enable(!hide_colors);

	const bool hide_sound = tab_ != SOUND_TAB;
	music_button_.hide(hide_sound);
	music_label_.hide(hide_sound);
	music_slider_.hide(hide_sound);
	sound_button_.hide(hide_sound);
	sound_label_.hide(hide_sound);
	sound_slider_.hide(hide_sound);
	UI_sound_button_.hide(hide_sound);
	UI_sound_label_.hide(hide_sound);
	UI_sound_slider_.hide(hide_sound);
	turn_bell_button_.hide(hide_sound);
	bell_label_.hide(hide_sound);
	bell_slider_.hide(hide_sound);
	music_slider_.enable(music_on());
	bell_slider_.enable(turn_bell());
	sound_slider_.enable(sound_on());
	UI_sound_slider_.enable(UI_sound_on());
	music_label_.enable(music_on());
	bell_label_.enable(turn_bell());
	sound_label_.enable(sound_on());
	UI_sound_label_.enable(UI_sound_on());
	advanced_sound_button_.hide(hide_sound);

	const bool hide_advanced_sound = tab_ != ADVANCED_SOUND_TAB;
	sample_rate_label_.hide(hide_advanced_sound);
	sample_rate_button1_.hide(hide_advanced_sound);
	sample_rate_button2_.hide(hide_advanced_sound);
	sample_rate_button3_.hide(hide_advanced_sound);
	sample_rate_input_.hide(hide_advanced_sound);
	confirm_sound_button_.hide(hide_advanced_sound);
	buffer_size_label_.hide(hide_advanced_sound);
	buffer_size_slider_.hide(hide_advanced_sound);
	normal_sound_button_.hide(hide_advanced_sound);

	const bool hide_multiplayer = tab_ != MULTIPLAYER_TAB;
	chat_lines_label_.hide(hide_multiplayer);
	chat_lines_slider_.hide(hide_multiplayer);
	chat_timestamp_button_.hide(hide_multiplayer);
	sort_list_by_group_button_.hide(hide_multiplayer);
	iconize_list_button_.hide(hide_multiplayer);
	remember_pw_button_.hide(hide_multiplayer);
	show_lobby_joins_button1_.hide(hide_multiplayer);
	show_lobby_joins_button2_.hide(hide_multiplayer);
	show_lobby_joins_button3_.hide(hide_multiplayer);
	friends_list_button_.hide(hide_multiplayer);
	mp_server_search_button_.hide(hide_multiplayer);
	mp_alerts_options_button_.hide(hide_multiplayer);

	const bool hide_friends = tab_ != FRIENDS_TAB;
	friends_.hide(hide_friends);
	friends_back_button_.hide(hide_friends);
	friends_add_friend_button_.hide(hide_friends);
	friends_add_ignore_button_.hide(hide_friends);
	friends_remove_button_.hide(hide_friends);
	friends_input_.hide(hide_friends);

	const bool hide_advanced = tab_ != ADVANCED_TAB;
	advanced_.hide(hide_advanced);
	std::string adv_type = get_advanced_pref() != NULL ? (*get_advanced_pref())["type"].str() : "";
	const bool hide_advanced_bool = hide_advanced || adv_type != "boolean";
	const bool hide_advanced_int = hide_advanced || adv_type != "int";
	const bool hide_advanced_combo = hide_advanced || adv_type != "combo";
	advanced_button_.hide(hide_advanced_bool);
	advanced_slider_.hide(hide_advanced_int);
	advanced_combo_.hide(hide_advanced_combo);
	advanced_option_label_.hide(hide_advanced_int && hide_advanced_combo);
}

} //end anonymous namespace

void show_preferences_dialog(CVideo& video, const config& game_cfg)
{
	std::vector<std::string> items;

	std::string const pre = IMAGE_PREFIX + std::string("icons/icon-");
	char const sep = COLUMN_SEPARATOR;
	items.push_back(pre + "general.png" + sep + translation::sgettext("Prefs section^General"));
	items.push_back(pre + "display.png" + sep + translation::sgettext("Prefs section^Display"));
	items.push_back(pre + "music.png" + sep + translation::sgettext("Prefs section^Sound"));
	items.push_back(pre + "multiplayer.png" + sep + translation::sgettext("Prefs section^Multiplayer"));
	items.push_back(pre + "advanced.png" + sep + translation::sgettext("Advanced section^Advanced"));

	if(items[1].empty() || items[1][0] != '*') {
		items[0] = "*" + items[0];
	}

	preferences_dialog dialog(video,game_cfg);
	dialog.parent.assign(new preferences_parent_dialog(video));
	dialog.parent->set_menu(items);
	dialog.parent->add_pane(&dialog);
	dialog.parent->show();
}

bool show_theme_dialog(CVideo& video)
{
	std::vector<theme_info> themes = theme::get_known_themes();

	if(!themes.empty()){
		gui2::ttheme_list dlg(themes);

		for(size_t k = 0; k < themes.size(); ++k) {
			if(themes[k].id == preferences::theme()) {
				dlg.set_selected_index(static_cast<int>(k));
			}
		}

		dlg.show(video);
		const int action = dlg.selected_index();

		if(action >= 0){
			preferences::set_theme(themes[action].id);
			// FIXME: it would be preferable for the new theme to take effect
			//        immediately.
			return 1;
		}
	} else {
		gui2::show_transient_message(video,"",_("No known themes. Try changing from within an existing game."));
	}

	return 0;
}

void show_mp_alerts_dialog(CVideo& video)
{
	gui2::tmp_alerts_options::display(video);
}

void show_advanced_graphics_dialog(CVideo& video)
{
	gui2::tadvanced_graphics_options::display(video);
}

std::string show_wesnothd_server_search(CVideo& video)
{
	// Showing file_chooser so user can search the wesnothd
	std::string old_path = preferences::get_mp_server_program_name();
	size_t offset = old_path.rfind("/");
	if (offset != std::string::npos)
	{
		old_path = old_path.substr(0, offset);
	}
	else
	{
		old_path.clear();
	}
#ifndef _WIN32

#ifndef WESNOTH_PREFIX
#define WESNOTH_PREFIX "/usr"
#endif
	const std::string filename = "wesnothd";
	std::string path = WESNOTH_PREFIX + std::string("/bin");
	if (!filesystem::is_directory(path))
		path = filesystem::get_cwd();

#else
	const std::string filename = "wesnothd.exe";
	std::string path = filesystem::get_cwd();
#endif
	if (!old_path.empty()
			&& filesystem::is_directory(old_path))
	{
		path = old_path;
	}

	utils::string_map symbols;

	symbols["filename"] = filename;

	const std::string title = utils::interpolate_variables_into_string(
			  _("Find $filename server binary to host networked games")
			, &symbols);

	int res = dialogs::show_file_chooser_dialog(video, path, title, false, filename);
	if (res == 0)
		return path;
	else
		return "";
}


} // end namespace preferences
