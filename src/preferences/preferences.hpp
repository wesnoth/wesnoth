/*
	Copyright (C) 2024 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "filesystem.hpp"
#include "game_config.hpp"
#include "gui/sort_order.hpp"
#include "preferences/preferences_list.hpp"
#include "serialization/compression.hpp"
#include "team.hpp"
#include "terrain/translation.hpp"
#include "lua/wrapper_lua.h" // for lua_State arguments to friend functions
#include "log.hpp"
#include <set>

struct point;
class unit_map;
class game_board;

namespace pref_constants
{
const int min_window_width  = 800;
const int min_window_height = 540;

const int def_window_width  = 1280;
const int def_window_height = 720;

const int max_window_width = 1920;
const int max_window_height = 1080;

const int min_font_scaling  = 80;
const int max_font_scaling  = 150;

const int min_pixel_scale = 1;
const int max_pixel_scale = 4;

const int TRANSITION_UPDATE_OFF = 0;
const int TRANSITION_UPDATE_ON = 1;
const int TRANSITION_UPDATE_PARTIAL = 2;
const int TRANSITION_UPDATE_COUNT = 3;

const unsigned char CREDENTIAL_SEPARATOR = '\f';
const std::string EMPTY_LOGIN = "@@";

const int INFINITE_AUTO_SAVES = 61;

const std::string default_addons_server = "add-ons.wesnoth.org";

// preferences for MP alerts
// all listed here have three variants with the suffixes: _lobby, _sound, _notif
const std::string player_joins = "player_joins";
const std::string player_leaves = "player_leaves";
const std::string private_message = "private_message";
const std::string friend_message = "friend_message";
const std::string public_message = "public_message";
const std::string server_message = "server_message";
const std::string ready_for_start = "ready_for_start";
const std::string game_has_begun = "game_has_begun";
const std::string turn_changed = "turn_changed";
const std::string game_created = "game_created";

enum class lobby_joins { show_none, show_friends, show_all };

enum PREFERENCE_VIEW { VIEW_DEFAULT, VIEW_FRIENDS };
};

namespace preferences
{
class acquaintance
{
public:
	acquaintance()
	{
	}

	explicit acquaintance(const config& cfg)
	{
		nick_ = cfg["nick"].str();
		status_ = cfg["status"].str();
		notes_ = cfg["notes"].str();
	}

	acquaintance(const std::string& nick, const std::string& status, const std::string& notes)
		: nick_(nick)
		, status_(status)
		, notes_(notes)
	{
	}

	const std::string& get_nick() const { return nick_; }
	const std::string& get_status() const { return status_; }
	const std::string& get_notes() const { return notes_; }

	void save(config& item)
	{
		item["nick"] = nick_;
		item["status"] = status_;
		item["notes"] = notes_;
	}

private:
	/** acquaintance's MP nick */
	std::string nick_;

	/**status (e.g., "friend", "ignore") */
	std::string status_;

	/** notes on the acquaintance */
	std::string notes_;
};

class secure_buffer : public std::vector<unsigned char>
{
public:
	template<typename... T> secure_buffer(T&&... args)
		: vector<unsigned char>(std::forward<T>(args)...)
	{}
	~secure_buffer()
	{
		std::fill(begin(), end(), '\0');
	}
};

struct login_info
{
	std::string username, server;
	secure_buffer key;
	login_info(const std::string& username, const std::string& server, const secure_buffer& key)
		: username(username), server(server), key(key)
	{}
	login_info(const std::string& username, const std::string& server)
		: username(username), server(server), key()
	{}
	std::size_t size() const
	{
		return 3 + username.size() + server.size() + key.size();
	}
};

struct option
{
	option(const config& pref)
	: type()
	, name(pref["name"].t_str())
	, description(pref["description"].t_str())
	, field(pref["field"].str())
	, cfg(pref)
	{
		const std::string p_type = cfg["type"];

		if(p_type == "boolean") {
			type = avd_type::TOGGLE;
		} else if(p_type == "int") {
			type = avd_type::SLIDER;
		} else if(p_type == "combo") {
			type = avd_type::COMBO;
		} else if(p_type == "custom") {
			type = avd_type::SPECIAL;
		} else {
			throw std::invalid_argument("Unknown type '" + p_type + "' for advanced preference " + name);
		}
	}

	enum class avd_type { TOGGLE, SLIDER, COMBO, SPECIAL };

	/** The preference type. */
	avd_type type;

	/** Displayed name. */
	t_string name;

	/** Displayed description. */
	t_string description;

	/** The actual field saved in the prefs file and by which this preference may be accessed. */
	std::string field;

	/** The full config, including type-specific options. */
	config cfg;
};
};

class prefs
{
	friend struct preferences_dialog_friend;
	friend int impl_preferences_set(lua_State* L);
	friend int impl_preferences_get(lua_State* L);

	public:
		static prefs& get()
		{
			// for wesnoth: checks that the userdata folders have been initialized/found since if it hasn't been then it's too soon to be getting any values from it
			// for boost: the userdata folders don't get initialized and the preferences aren't used for anything, so skip the check here
			//            macos -  called "unit_tests"
			//            others - called "boost_unit_tests"
			static bool called_before_init = !(filesystem::base_name(filesystem::get_exe_path()).find("unit_tests") == std::string::npos && !filesystem::is_userdata_initialized());
			assert(called_before_init && "Attempt to use preferences before userdata initialization");

			static prefs prefs_manager;
			return prefs_manager;
		}

		~prefs();

		void write_preferences();
		void load_advanced_prefs(const game_config_view& gc);
		void migrate_preferences(const std::string& prefs_dir);
		void reload_preferences();
		std::set<std::string> all_attributes();

		std::string core_id();
		void set_core_id(const std::string& root);

		bool scroll_to_action();
		void set_scroll_to_action(bool ison);

		point resolution();
		void set_resolution(const point& res);

		int pixel_scale();
		void set_pixel_scale(const int scale);

		bool auto_pixel_scale();
		void set_auto_pixel_scale(bool choice);

		bool maximized();
		void set_maximized(bool ison);

		bool fullscreen();
		void set_fullscreen(bool ison);

		bool vsync();
		void set_vsync(bool ison);

		bool turbo();
		void set_turbo(bool ison);

		double turbo_speed();
		void set_turbo_speed(const double speed);

		int font_scaling();
		void set_font_scaling(int scale);
		int font_scaled(int size);

		int keepalive_timeout();
		void keepalive_timeout(int seconds);

		bool idle_anim();
		void set_idle_anim(const bool ison);

		int idle_anim_rate();
		void set_idle_anim_rate(int rate);

		std::string language();
		void set_language(const std::string& s);

		std::string gui_theme();
		void set_gui_theme(const std::string& s);

		// Don't rename it to sound() because of a gcc-3.3 branch bug which will cause it to conflict with the sound namespace.
		bool sound_on();
		bool set_sound(bool ison);

		unsigned int sample_rate();
		void save_sample_rate(const unsigned int rate);

		std::size_t sound_buffer_size();
		void save_sound_buffer_size(const std::size_t size);

		int sound_volume();
		void set_sound_volume(int vol);

		int bell_volume();
		void set_bell_volume(int vol);

		int ui_volume();
		void set_ui_volume(int vol);

		bool music_on();
		bool set_music(bool ison);

		int music_volume();
		void set_music_volume(int vol);

		bool stop_music_in_background();
		void set_stop_music_in_background(bool ison);

		unsigned int tile_size();
		void set_tile_size(const unsigned int size);

		bool turn_bell();
		bool set_turn_bell(bool ison);

		bool ui_sound_on();
		bool set_ui_sound(bool ison);

		bool message_bell();

		// Proxies for preferences_dialog
		void load_hotkeys();
		void save_hotkeys();
		void clear_hotkeys();

		void add_alias(const std::string& alias, const std::string& command);
		optional_const_config get_alias();

		std::string allied_color();
		void set_allied_color(const std::string& color_id);

		std::string enemy_color();
		void set_enemy_color(const std::string& color_id);

		std::string unmoved_color();
		void set_unmoved_color(const std::string& color_id);

		std::string partial_color();
		void set_partial_color(const std::string& color_id);

		std::string moved_color();
		void set_moved_color(const std::string& color_id);

		bool show_ally_orb();
		void set_show_ally_orb(bool show_orb);

		bool show_status_on_ally_orb();
		void set_show_status_on_ally_orb(bool show_orb);

		bool show_enemy_orb();
		void set_show_enemy_orb(bool show_orb);

		bool show_moved_orb();
		void set_show_moved_orb(bool show_orb);

		bool show_unmoved_orb();
		void set_show_unmoved_orb(bool show_orb);

		bool show_partial_orb();
		void set_show_partial_orb(bool show_orb);

		bool show_disengaged_orb();
		void set_show_disengaged_orb(bool show_orb);

		int scroll_speed();
		void set_scroll_speed(const int scroll);

		bool middle_click_scrolls();
		bool mouse_scroll_enabled();
		void enable_mouse_scroll(bool value);

		/**
		 * Gets the threshold for when to scroll.
		 *
		 * This scrolling happens when the mouse is in the application and near the border.
		 */
		int mouse_scroll_threshold();

		int draw_delay();
		void set_draw_delay(int value);

		bool animate_map();
		void set_animate_map(bool value);

		bool animate_water();
		void set_animate_water(bool value);

		bool minimap_movement_coding();
		void toggle_minimap_movement_coding();

		bool minimap_terrain_coding();
		void toggle_minimap_terrain_coding();

		bool minimap_draw_units();
		void toggle_minimap_draw_units();

		bool minimap_draw_villages();
		void toggle_minimap_draw_villages();

		bool minimap_draw_terrain();
		void toggle_minimap_draw_terrain();

		bool show_fps();
		void set_show_fps(bool value);

		bool ellipses();
		void set_ellipses(bool ison);

		bool grid();
		void set_grid(bool ison);

		bool confirm_load_save_from_different_version();

		bool use_twelve_hour_clock_format();

		bool disable_auto_moves();
		void set_disable_auto_moves(bool value);

		bool damage_prediction_allow_monte_carlo_simulation();
		void set_damage_prediction_allow_monte_carlo_simulation(bool value);

		std::string addon_manager_saved_order_name();
		void set_addon_manager_saved_order_name(const std::string& value);

		sort_order::type addon_manager_saved_order_direction();
		void set_addon_manager_saved_order_direction(sort_order::type value);

		std::string selected_achievement_group();
		void set_selected_achievement_group(const std::string& content_for);

		/**
		 * @param content_for The achievement group the achievement is part of.
		 * @param id The ID of the achievement within the achievement group.
		 * @return True if the achievement exists and is completed, false otherwise.
		 */
		bool achievement(const std::string& content_for, const std::string& id);
		/**
		 * Marks the specified achievement as completed.
		 *
		 * @param content_for The achievement group the achievement is part of.
		 * @param id The ID of the achievement within the achievement group.
		 */
		void set_achievement(const std::string& content_for, const std::string& id);

		/**
		 * Increments the achievement's current progress by @a amount if it hasn't already been completed.
		 * If you only want to check the achievement's current progress, then omit the last three arguments.
		 * @a amount defaults to 0, which will result in the current progress value being returned without being changed (x + 0 == x).
		 *
		 * Note that this uses the same [in_progress] as is used for set_sub_achievement().
		 *
		 * @param content_for The id of the achievement group this achievement is in.
		 * @param id The id for the specific achievement in the achievement group.
		 * @param limit The maximum value that a specific call to this function can increase the achievement progress value.
		 * @param max_progress The value when the achievement is considered completed.
		 * @param amount The amount to progress the achievement.
		 * @return The achievement's current progress, or -1 if it has already been completed.
		 */
		int progress_achievement(const std::string& content_for, const std::string& id, int limit = 999999, int max_progress = 999999, int amount = 0);

		/**
		 * @param content_for The achievement group the achievement is part of.
		 * @param id The ID of the achievement within the achievement group.
		 * @param sub_id The ID of the sub-achievement within the achievement.
		 * @return True if the sub-achievement exists and is completed, false otherwise.
		 */
		bool sub_achievement(const std::string& content_for, const std::string& id, const std::string& sub_id);

		/**
		 * Marks the specified sub-achievement as completed.
		 *
		 * Note that this uses the same [in_progress] as is used for progress_achievement().
		 *
		 * @param content_for The achievement group the achievement is part of.
		 * @param id The ID of the achievement within the achievement group.
		 * @param sub_id The ID of the sub-achievement within the achievement.
		 */
		void set_sub_achievement(const std::string& content_for, const std::string& id, const std::string& sub_id);

		/**
		 * @param addon_id The chosen addon id from the editor to store in the preferences.
		 */
		void set_editor_chosen_addon(const std::string& addon_id);

		/**
		 * @return The most recently selected add-on id from the editor. May be an empty string.
		 */
		std::string editor_chosen_addon();

		void set_mp_alert_option(const std::string& id, const std::string& type, bool value);
		bool mp_alert_option(const std::string& id, const std::string& type, bool def = false);
		bool has_mp_alert_option(const std::string& id, const std::string& type);

		void set_last_cache_cleared_version(const std::string& version);
		std::string last_cache_cleared_version();

		bool get_show_deprecation(bool def);

		bool get_scroll_when_mouse_outside(bool def);

		void set_dir_bookmarks(const config& cfg);
		optional_const_config dir_bookmarks();

		bool whisper_friends_only();
		void set_whisper_friends_only(bool v);

		bool auto_open_whisper_windows();

		bool fi_invert();
		void set_fi_invert(bool value);

		bool fi_vacant_slots();
		void set_fi_vacant_slots(bool value);

		bool fi_friends_in_game();
		void set_fi_friends_in_game(bool value);

		bool fi_blocked_in_game();
		void set_fi_blocked_in_game(bool value);

		int editor_auto_update_transitions();
		void set_editor_auto_update_transitions(int value);

		std::string default_dir();

		bool editor_draw_terrain_codes();
		void set_editor_draw_terrain_codes(bool value);

		bool editor_draw_hex_coordinates();
		void set_editor_draw_hex_coordinates(bool value);

		bool editor_draw_num_of_bitmaps();
		void set_editor_draw_num_of_bitmaps(bool value);

		std::size_t editor_mru_limit();
		std::vector<std::string> do_read_editor_mru();
		void do_commit_editor_mru(const std::vector<std::string>& mru);
		/** Retrieves the list of recently opened files. */
		std::vector<std::string> recent_files();
		/** Adds an entry to the recent files list. */
		void add_recent_files_entry(const std::string& path);

		bool use_color_cursors();
		void set_color_cursors(bool value);

		bool show_standing_animations();
		void set_show_standing_animations(bool value);

		void show_wesnothd_server_search();
		bool show_theme_dialog();

		void set_theme(const std::string& theme);
		std::string theme();

		void set_mp_server_program_name(const std::string&);
		std::string get_mp_server_program_name();

		bool parse_should_show_lobby_join(const std::string& sender, const std::string& message);
		pref_constants::lobby_joins get_lobby_joins();
		void set_lobby_joins(pref_constants::lobby_joins show);

		const std::map<std::string, preferences::acquaintance>& get_acquaintances();
		const std::string get_ignored_delim();
		std::map<std::string, std::string> get_acquaintances_nice(const std::string& filter);
		std::pair<preferences::acquaintance*, bool> add_acquaintance(const std::string& nick, const std::string& mode, const std::string& notes);
		void add_completed_campaign(const std::string&campaign_id, const std::string& difficulty_level);
		bool remove_acquaintance(const std::string& nick);
		bool is_friend(const std::string& nick);
		bool is_ignored(const std::string& nick);
		bool is_campaign_completed(const std::string& campaign_id);
		bool is_campaign_completed(const std::string& campaign_id, const std::string& difficulty_level);

		const std::vector<game_config::server_info>& builtin_servers_list();
		std::vector<game_config::server_info> user_servers_list();
		void set_user_servers_list(const std::vector<game_config::server_info>& value);

		std::string network_host();
		void set_network_host(const std::string& host);

		std::string campaign_server();
		void set_campaign_server(const std::string& host);

		bool turn_dialog();
		void set_turn_dialog(bool ison);

		bool enable_whiteboard_mode_on_start();
		void set_enable_whiteboard_mode_on_start(bool value);

		bool hide_whiteboard();
		void set_hide_whiteboard(bool value);

		bool show_combat();

		bool allow_observers();
		void set_allow_observers(bool value);

		bool shuffle_sides();
		void set_shuffle_sides(bool value);

		std::string random_faction_mode();
		void set_random_faction_mode(const std::string& value);

		bool use_map_settings();
		void set_use_map_settings(bool value);

		int mp_server_warning_disabled();
		void set_mp_server_warning_disabled(int value);

		bool random_start_time();
		void set_random_start_time(bool value);

		bool fog();
		void set_fog(bool value);

		bool shroud();
		void set_shroud(bool value);

		int turns();
		void set_turns(int value);

		const config& options();
		void set_options(const config& values);

		bool skip_mp_replay();
		void set_skip_mp_replay(bool value);

		bool blindfold_replay();
		void set_blindfold_replay(bool value);

		bool countdown();
		void set_countdown(bool value);
		int countdown_init_time();
		void set_countdown_init_time(int value);
		void clear_countdown_init_time();

		int countdown_turn_bonus();
		void set_countdown_turn_bonus(int value);
		void clear_countdown_turn_bonus();

		int countdown_reservoir_time();
		void set_countdown_reservoir_time(int value);
		void clear_countdown_reservoir_time();

		int countdown_action_bonus();
		void set_countdown_action_bonus(int value);
		void clear_countdown_action_bonus();

		int village_gold();
		void set_village_gold(int value);

		int village_support();
		void set_village_support(int value);

		int xp_modifier();
		void set_xp_modifier(int value);

		std::string era();
		void set_era(const std::string& value);

		std::string level();
		void set_level(const std::string& value);
		int level_type();
		void set_level_type(int value);

		const std::vector<std::string>& modifications(bool mp = true);
		void set_modifications(const std::vector<std::string>& value, bool mp = true);

		bool skip_ai_moves();
		void set_skip_ai_moves(bool value);

		void set_show_side_colors(bool value);
		bool show_side_colors();

		bool save_replays();
		void set_save_replays(bool value);

		bool delete_saves();
		void set_delete_saves(bool value);

		void set_ask_delete_saves(bool value);
		bool ask_delete_saves();

		void set_interrupt_when_ally_sighted(bool value);
		bool interrupt_when_ally_sighted();

		void set_autosavemax(int value);
		int autosavemax();

		bool show_floating_labels();
		void set_show_floating_labels(bool value);

		bool message_private();
		void set_message_private(bool value);

		// Multiplayer functions
		std::string get_chat_timestamp(const std::time_t& t);
		bool chat_timestamping();
		void set_chat_timestamping(bool value);

		int chat_lines();
		void set_chat_lines(int lines);

		int chat_message_aging();
		void set_chat_message_aging(const int aging);

		bool show_all_units_in_help();
		void set_show_all_units_in_help(bool value);

		compression::format save_compression_format();

		std::set<std::string>& encountered_units();
		std::set<t_translation::terrain_code>& encountered_terrains();

		std::string custom_command();
		void set_custom_command(const std::string& command);

		std::vector<std::string>* get_history(const std::string& id);

		// Ask for end turn confirmation
		bool yellow_confirm();
		bool green_confirm();
		bool confirm_no_moves();

		// Add all recruitable units as encountered so that information
		// about them are displayed to the user in the help system.
		void encounter_recruitable_units(const std::vector<team>& teams);

		// Add all units that exist at the start to the encountered units so
		// that information about them are displayed to the user in the help
		// system.
		void encounter_start_units(const unit_map& units);

		// Add all units that are recallable as encountered units.
		void encounter_recallable_units(const std::vector<team>& teams);

		// Add all terrains on the map as encountered terrains.
		void encounter_map_terrain(const gamemap& map);

		// Calls all of the above functions on the current game board
		void encounter_all_content(const game_board& gb);

		bool remember_password();
		void set_remember_password(bool remember);

		std::string login();
		void set_login(const std::string& login);

		std::string password(const std::string& server, const std::string& login);
		void set_password(const std::string& server, const std::string& login, const std::string& key);

		std::vector<preferences::option>& get_advanced_preferences() {return advanced_prefs_;}

		static void disable_preferences_save() {
			no_preferences_save = true;
		}

		static bool preferences_save() {
			return no_preferences_save;
		}

	private:
		prefs();
		// don't move, assign, or copy a singleton
		prefs(const prefs& p) = delete;
		prefs& operator=(const prefs& p) = delete;
		prefs(const prefs&& p) = delete;
		prefs& operator=(const prefs&& p) = delete;

		inline static bool no_preferences_save = false;

		config preferences_;
		bool fps_;
		std::map<std::string, std::set<std::string>> completed_campaigns_;
		std::set<std::string> encountered_units_set_;
		std::set<t_translation::terrain_code> encountered_terrains_set_;
		std::map<std::string, std::vector<std::string>> history_map_;
		std::map<std::string, preferences::acquaintance> acquaintances_;
		config option_values_;
		bool options_initialized_;
		std::vector<std::string> mp_modifications_;
		bool mp_modifications_initialized_;
		std::vector<std::string> sp_modifications_;
		bool sp_modifications_initialized_;
		bool message_private_on_;
		std::vector<preferences::login_info> credentials_;
		std::vector<preferences::option> advanced_prefs_;

		void load_preferences();
		void clear_preferences() {preferences_.clear();}
		void load_credentials();
		void save_credentials();
		void clear_credentials();

		void clear(const std::string& key);
		void set_child(const std::string& key, const config& val);
		optional_const_config get_child(const std::string &key);
		std::string get(const std::string& key, const std::string& def);
		config::attribute_value get_as_attribute(const std::string& key);
		void erase(const std::string& key);

		std::string get_system_username();
		/**
		 * Encrypts the value of @a text using @a key and a hard coded IV using AES.
		 * Max size of @a text must not be larger than 1008 bytes.
		 *
		 * NOTE: This is not meant to provide strong protections against a determined attacker.
		 * This is meant to hide the passwords from malware scanning files for passwords, family/friends poking around, etc.
		 *
		 * @param text The original unencrypted data.
		 * @param key The value to use to encrypt the data. See build_key() for key generation.
		 * @return secure_buffer The encrypted data.
		 */
		preferences::secure_buffer aes_encrypt(const preferences::secure_buffer& text, const preferences::secure_buffer& key);
		/**
		 * Same as aes_encrypt(), except of course it takes encrypted data as an argument and returns decrypted data.
		 */
		preferences::secure_buffer aes_decrypt(const preferences::secure_buffer& text, const preferences::secure_buffer& key);
		/**
		 * Fills a secure_buffer with 32 bytes of deterministically generated bytes, then overwrites it with the system login name, server login name, and server name.
		 * If this is more than 32 bytes, then it's truncated. If it's less than 32 bytes, then the pre-generated bytes are used to pad it.
		 *
		 * @param server The server being logged into.
		 * @param login The username being used to login.
		 * @return secure_buffer The data to be used as the encryption key.
		 */
		preferences::secure_buffer build_key(const std::string& server, const std::string& login);
		preferences::secure_buffer escape(const preferences::secure_buffer& text);
		preferences::secure_buffer unescape(const preferences::secure_buffer& text);
};

//
// might be overkill, but it does best limit the private functionality being exposed
//

struct preferences_dialog_friend
{
	friend class preferences_dialog;

	static void set(const std::string& pref, bool value)
	{
		prefs::get().preferences_[pref] = value;
	}
	static void set(const std::string& pref, int value)
	{
		prefs::get().preferences_[pref] = value;
	}
	static void set(const std::string& pref, const std::string& value)
	{
		prefs::get().preferences_[pref] = value;
	}

	static bool get(const std::string& pref, bool def)
	{
		return prefs::get().preferences_[pref].to_bool(def);
	}
	static int get(const std::string& pref, int def)
	{
		return prefs::get().preferences_[pref].to_int(def);
	}
	static std::string get(const std::string& pref, const std::string& def)
	{
		return prefs::get().get(pref, def);
	}
};
