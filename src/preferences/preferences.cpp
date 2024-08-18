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

/**
 *  @file
 *  Get and set user-preferences.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "preferences/preferences.hpp"

#include "cursor.hpp"
#include "game_board.hpp"
#include "game_display.hpp"
#include "formula/string_utils.hpp"
#include "game_config.hpp"
#include "game_data.hpp"
#include "gettext.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/theme_list.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "hotkey/hotkey_item.hpp"
#include "log.hpp"
#include "map_settings.hpp"
#include "map/map.hpp"
#include "resources.hpp"
#include "serialization/parser.hpp"
#include "sound.hpp"
#include "units/unit.hpp"
#include "video.hpp"

#include <sys/stat.h> // for setting the permissions of the preferences file
#include <boost/algorithm/string.hpp>

#ifdef _WIN32
#include "serialization/unicode_cast.hpp"
#include <boost/range/iterator_range.hpp>
#include <windows.h>
#endif

#ifndef __APPLE__
#include <openssl/evp.h>
#include <openssl/err.h>
#else
#include <CommonCrypto/CommonCryptor.h>
#endif

static lg::log_domain log_config("config");
#define ERR_CFG LOG_STREAM(err , log_config)
#define DBG_CFG LOG_STREAM(debug , log_config)

static lg::log_domain log_filesystem("filesystem");
#define ERR_FS LOG_STREAM(err, log_filesystem)

static lg::log_domain advanced_preferences("advanced_preferences");
#define ERR_ADV LOG_STREAM(err, advanced_preferences)

prefs::prefs()
: preferences_()
, fps_(false)
, completed_campaigns_()
, encountered_units_set_()
, encountered_terrains_set_()
, history_map_()
, acquaintances_()
, option_values_()
, options_initialized_(false)
, mp_modifications_()
, mp_modifications_initialized_(false)
, sp_modifications_()
, sp_modifications_initialized_(false)
, message_private_on_(false)
, credentials_()
, advanced_prefs_()
{
	load_preferences();
	load_credentials();

	// make sure this has a default set
	if(!preferences_.has_attribute("scroll_threshold")) {
		preferences_[prefs_list::scroll_threshold] = 10;
	}

	for(const config& acfg : preferences_.child_range("acquaintance")) {
		preferences::acquaintance ac = preferences::acquaintance(acfg);
		acquaintances_[ac.get_nick()] = ac;
	}
}

prefs::~prefs()
{
	config campaigns;
	for(const auto& elem : completed_campaigns_) {
		config cmp;
		cmp["name"] = elem.first;
		cmp["difficulty_levels"] = utils::join(elem.second);
		campaigns.add_child("campaign", cmp);
	}

	set_child(prefs_list::completed_campaigns, campaigns);

	preferences_[prefs_list::encountered_units] = utils::join(encountered_units_set_);
	t_translation::ter_list terrain(encountered_terrains_set_.begin(), encountered_terrains_set_.end());
	preferences_[prefs_list::encountered_terrain_list] = t_translation::write_list(terrain);

	/* Structure of the history
		[history]
			[history_id]
				[line]
					message = foobar
				[/line]
	*/
	config history;
	for(const auto& history_id : history_map_) {
		config history_id_cfg; // [history_id]
		for(const std::string& line : history_id.second) {
			config cfg; // [line]

			cfg["message"] = line;
			history_id_cfg.add_child("line", std::move(cfg));
		}

		history.add_child(history_id.first, history_id_cfg);
	}
	set_child(prefs_list::history, history);

	preferences_.clear_children("acquaintance");

	for(auto& a : acquaintances_) {
		config& item = preferences_.add_child("acquaintance");
		a.second.save(item);
	}

	history_map_.clear();
	encountered_units_set_.clear();
	encountered_terrains_set_.clear();

	try {
		if(!no_preferences_save) {
			write_preferences();
		}
	} catch (...) {
		ERR_FS << "Failed to write preferences due to exception: " << utils::get_unknown_exception_type();
	}
}

void prefs::load_advanced_prefs(const game_config_view& gc)
{
	for(const config& pref : gc.child_range("advanced_preference")) {
		try {
			advanced_prefs_.emplace_back(pref);
		} catch(const std::invalid_argument& e) {
			ERR_ADV << e.what();
			continue;
		}
	}

	// show_deprecation has a different default on the dev branch
	if(game_config::wesnoth_version.is_dev_version()) {
		for(preferences::option& op : advanced_prefs_) {
			if(op.field == prefs_list::show_deprecation) {
				op.cfg["default"] = true;
			}
		}
	}

	std::sort(advanced_prefs_.begin(), advanced_prefs_.end(), [](const auto& lhs, const auto& rhs) { return translation::icompare(lhs.name, rhs.name) < 0; });
}

void prefs::migrate_preferences(const std::string& migrate_prefs_file)
{
	if(migrate_prefs_file != filesystem::get_synced_prefs_file() && filesystem::file_exists(migrate_prefs_file)) {
		// if the file doesn't exist, just copy the file over
		// else need to merge the preferences file
		if(!filesystem::file_exists(filesystem::get_synced_prefs_file())) {
			filesystem::copy_file(migrate_prefs_file, filesystem::get_synced_prefs_file());
		} else {
			config current_cfg;
			filesystem::scoped_istream current_stream = filesystem::istream_file(filesystem::get_synced_prefs_file(), false);
			read(current_cfg, *current_stream);
			config old_cfg;
			filesystem::scoped_istream old_stream = filesystem::istream_file(migrate_prefs_file, false);
			read(old_cfg, *old_stream);

			// when both files have the same attribute, use the one from whichever was most recently modified
			bool current_prefs_are_older = filesystem::file_modified_time(filesystem::get_synced_prefs_file()) < filesystem::file_modified_time(migrate_prefs_file);
			for(const config::attribute& val : old_cfg.attribute_range()) {
				if(current_prefs_are_older || !current_cfg.has_attribute(val.first)) {
					preferences_[val.first] = val.second;
				}
			}

			// don't touch child tags

			prefs::get().write_preferences();
		}
	}
}
void prefs::reload_preferences()
{
	clear_preferences();
	load_preferences();
	load_credentials();
}

std::set<std::string> prefs::all_attributes()
{
	std::set<std::string> attrs;

	// attributes that exist in the preferences file
	for(const auto& attr : preferences_.attribute_range()) {
		attrs.emplace(attr.first);
	}
	// all mainline preference attributes, whether they're set or not
	for(const auto attr : prefs_list::values) {
		attrs.emplace(attr);
	}

	return attrs;
}

void prefs::load_preferences()
{
	preferences_.clear();
	try{
		config default_prefs;
		config unsynced_prefs;
		config synced_prefs;
#ifdef DEFAULT_PREFS_PATH
		// NOTE: the system preferences file is only ever relevant for the first time wesnoth starts
		//	   any default values will subsequently be written to the normal preferences files, which takes precedence over any values in the system preferences file
		{
			filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_default_prefs_file(), false);
			read(default_prefs, *stream);
		}
#endif
		{
			filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_unsynced_prefs_file(), false);
			read(unsynced_prefs, *stream);
		}

		{
			filesystem::scoped_istream stream = filesystem::istream_file(filesystem::get_synced_prefs_file(), false);
			read(synced_prefs, *stream);
		}

		preferences_.merge_with(default_prefs);
		preferences_.merge_with(unsynced_prefs);
		preferences_.merge_with(synced_prefs);

		// check for any unknown preferences
		for(const auto& attr : synced_prefs.attribute_range()) {
			if(std::find(synced_attributes_.begin(), synced_attributes_.end(), attr.first) == synced_attributes_.end()) {
				unknown_synced_attributes_.insert(attr.first);
			}
		}
		for(const auto& attr : unsynced_prefs.attribute_range()) {
			if(std::find(unsynced_attributes_.begin(), unsynced_attributes_.end(), attr.first) == unsynced_attributes_.end()) {
				unknown_unsynced_attributes_.insert(attr.first);
			}
		}

		for(const auto child : synced_prefs.all_children_range()) {
			if(std::find(synced_children_.begin(), synced_children_.end(), child.key) == synced_children_.end()) {
				unknown_synced_children_.insert(child.key);
			}
		}
		for(const auto child : unsynced_prefs.all_children_range()) {
			if(std::find(unsynced_children_.begin(), unsynced_children_.end(), child.key) == unsynced_children_.end()) {
				unknown_unsynced_children_.insert(child.key);
			}
		}
	} catch(const config::error& e) {
		ERR_CFG << "Error loading preference, message: " << e.what();
	}

	set_music_volume(music_volume());
	set_sound_volume(sound_volume());

	/*
	completed_campaigns = "A,B,C"
	[completed_campaigns]
		[campaign]
			name = "A"
			difficulty_levels = "EASY,MEDIUM"
		[/campaign]
	[/completed_campaigns]
	*/
	// presumably for backwards compatibility?
	// nothing actually sets the attribute, only the child tags
	for(const std::string& c : utils::split(preferences_[prefs_list::completed_campaigns])) {
		completed_campaigns_[c]; // create the elements
	}

	if(auto ccc = get_child(prefs_list::completed_campaigns)) {
		for(const config& cc : ccc->child_range("campaign")) {
			std::set<std::string>& d = completed_campaigns_[cc["name"]];
			std::vector<std::string> nd = utils::split(cc["difficulty_levels"]);
			std::copy(nd.begin(), nd.end(), std::inserter(d, d.begin()));
		}
	}

	encountered_units_set_ = utils::split_set(preferences_[prefs_list::encountered_units].str());

	const t_translation::ter_list terrain(t_translation::read_list(preferences_[prefs_list::encountered_terrain_list].str()));
	encountered_terrains_set_.insert(terrain.begin(), terrain.end());

	if(auto history = get_child(prefs_list::history)) {
		/* Structure of the history
			[history]
				[history_id]
					[line]
						message = foobar
					[/line]
		*/
		for(const config::any_child h : history->all_children_range()) {
			for(const config& l : h.cfg.child_range("line")) {
				history_map_[h.key].push_back(l["message"]);
			}
		}
	}
}

void prefs::write_preferences()
{
#ifndef _WIN32
	bool synced_prefs_file_existed = filesystem::file_exists(filesystem::get_synced_prefs_file());
	bool unsynced_prefs_file_existed = filesystem::file_exists(filesystem::get_unsynced_prefs_file());
#endif

	config synced;
	config unsynced;

	for(const char* attr : synced_attributes_) {
		if(preferences_.has_attribute(attr)) {
			synced[attr] = preferences_[attr];
		}
	}
	for(const char* attr : synced_children_) {
		for(const auto& child : preferences_.child_range(attr)) {
			synced.add_child(attr, child);
		}
	}

	for(const char* attr : unsynced_attributes_) {
		if(preferences_.has_attribute(attr)) {
			unsynced[attr] = preferences_[attr];
		}
	}
	for(const char* attr : unsynced_children_) {
		for(const auto& child : preferences_.child_range(attr)) {
			unsynced.add_child(attr, child);
		}
	}

	// write any unknown preferences back out
	for(const std::string& attr : unknown_synced_attributes_) {
		synced[attr] = preferences_[attr];
	}
	for(const std::string& attr : unknown_synced_children_) {
		for(const auto& child : preferences_.child_range(attr)) {
			synced.add_child(attr, child);
		}
	}

	for(const std::string& attr : unknown_unsynced_attributes_) {
		unsynced[attr] = preferences_[attr];
	}
	for(const std::string& attr : unknown_unsynced_children_) {
		for(const auto& child : preferences_.child_range(attr)) {
			unsynced.add_child(attr, child);
		}
	}

	try {
		filesystem::scoped_ostream synced_prefs_file = filesystem::ostream_file(filesystem::get_synced_prefs_file());
		write(*synced_prefs_file, synced);
	} catch(const filesystem::io_exception&) {
		ERR_FS << "error writing to synced preferences file '" << filesystem::get_synced_prefs_file() << "'";
	}

	try {
		filesystem::scoped_ostream unsynced_prefs_file = filesystem::ostream_file(filesystem::get_unsynced_prefs_file());
		write(*unsynced_prefs_file, unsynced);
	} catch(const filesystem::io_exception&) {
		ERR_FS << "error writing to unsynced preferences file '" << filesystem::get_unsynced_prefs_file() << "'";
	}

	save_credentials();

#ifndef _WIN32
	if(!synced_prefs_file_existed) {
		if(chmod(filesystem::get_synced_prefs_file().c_str(), 0600) == -1) {
			ERR_FS << "error setting permissions of preferences file '" << filesystem::get_synced_prefs_file() << "'";
		}
	}
	if(!unsynced_prefs_file_existed) {
		if(chmod(filesystem::get_unsynced_prefs_file().c_str(), 0600) == -1) {
			ERR_FS << "error setting permissions of unsynced preferences file '" << filesystem::get_unsynced_prefs_file() << "'";
		}
	}
#endif
}

void prefs::clear_credentials()
{
	// Zero them before clearing.
	// Probably overly paranoid, but doesn't hurt?
	for(auto& cred : credentials_) {
		std::fill(cred.username.begin(), cred.username.end(), '\0');
		std::fill(cred.server.begin(), cred.server.end(), '\0');
	}
	credentials_.clear();
}

void prefs::load_credentials()
{
	if(!remember_password()) {
		return;
	}
	clear_credentials();
	std::string cred_file = filesystem::get_credentials_file();
	if(!filesystem::file_exists(cred_file)) {
		return;
	}
	filesystem::scoped_istream stream = filesystem::istream_file(cred_file, false);
	// Credentials file is a binary blob, so use streambuf iterator
	preferences::secure_buffer data((std::istreambuf_iterator<char>(*stream)), (std::istreambuf_iterator<char>()));
	data = aes_decrypt(data, build_key("global", get_system_username()));
	if(data.empty() || data[0] != pref_constants::CREDENTIAL_SEPARATOR) {
		ERR_CFG << "Invalid data in credentials file";
		return;
	}
	for(const std::string& elem : utils::split(std::string(data.begin(), data.end()), pref_constants::CREDENTIAL_SEPARATOR, utils::REMOVE_EMPTY)) {
		std::size_t at = elem.find_last_of('@');
		std::size_t eq = elem.find_first_of('=', at + 1);
		if(at != std::string::npos && eq != std::string::npos) {
			preferences::secure_buffer key(elem.begin() + eq + 1, elem.end());
			credentials_.emplace_back(elem.substr(0, at), elem.substr(at + 1, eq - at - 1), unescape(key));
		}
	}
}

void prefs::save_credentials()
{
	if(!remember_password()) {
		filesystem::delete_file(filesystem::get_credentials_file());
		return;
	}

#ifndef _WIN32
	bool creds_file_existed = filesystem::file_exists(filesystem::get_credentials_file());
#endif

	preferences::secure_buffer credentials_data;
	for(const auto& cred : credentials_) {
		credentials_data.push_back(pref_constants::CREDENTIAL_SEPARATOR);
		credentials_data.insert(credentials_data.end(), cred.username.begin(), cred.username.end());
		credentials_data.push_back('@');
		credentials_data.insert(credentials_data.end(), cred.server.begin(), cred.server.end());
		credentials_data.push_back('=');
		preferences::secure_buffer key_escaped = escape(cred.key);
		credentials_data.insert(credentials_data.end(), key_escaped.begin(), key_escaped.end());
	}
	try {
		filesystem::scoped_ostream credentials_file = filesystem::ostream_file(filesystem::get_credentials_file());
		preferences::secure_buffer encrypted = aes_encrypt(credentials_data, build_key("global", get_system_username()));
		credentials_file->write(reinterpret_cast<const char*>(encrypted.data()), encrypted.size());
	} catch(const filesystem::io_exception&) {
		ERR_CFG << "error writing to credentials file '" << filesystem::get_credentials_file() << "'";
	}

#ifndef _WIN32
	if(!creds_file_existed) {
		if(chmod(filesystem::get_credentials_file().c_str(), 0600) == -1) {
			ERR_FS << "error setting permissions of credentials file '" << filesystem::get_credentials_file() << "'";
		}
	}
#endif
}

//
// helpers
//
void prefs::set_child(const std::string& key, const config& val) {
	preferences_.clear_children(key);
	preferences_.add_child(key, val);
}

optional_const_config prefs::get_child(const std::string& key)
{
	return preferences_.optional_child(key);
}

std::string prefs::get(const std::string& key, const std::string& def) {
	return preferences_[key].empty() ? def : preferences_[key];
}

config::attribute_value prefs::get_as_attribute(const std::string &key)
{
	return preferences_[key];
}

//
// accessors
//
static std::string fix_orb_color_name(const std::string& color) {
	if (color.substr(0,4) == "orb_") {
		if(color[4] >= '0' && color[4] <= '9') {
			return color.substr(5);
		} else {
			return color.substr(4);
		}
	}
	return color;
}

std::string prefs::allied_color() {
	std::string ally_color = preferences_[prefs_list::ally_orb_color].str();
	if (ally_color.empty())
		return game_config::colors::ally_orb_color;
	return fix_orb_color_name(ally_color);
}
void prefs::set_allied_color(const std::string& color_id) {
	preferences_[prefs_list::ally_orb_color] = color_id;
}

std::string prefs::enemy_color() {
	std::string enemy_color = preferences_[prefs_list::enemy_orb_color].str();
	if (enemy_color.empty())
		return game_config::colors::enemy_orb_color;
	return fix_orb_color_name(enemy_color);
}
void prefs::set_enemy_color(const std::string& color_id) {
	preferences_[prefs_list::enemy_orb_color] = color_id;
}

std::string prefs::moved_color() {
	std::string moved_color = preferences_[prefs_list::moved_orb_color].str();
	if (moved_color.empty())
		return game_config::colors::moved_orb_color;
	return fix_orb_color_name(moved_color);
}
void prefs::set_moved_color(const std::string& color_id) {
	preferences_[prefs_list::moved_orb_color] = color_id;
}

std::string prefs::unmoved_color() {
	std::string unmoved_color = preferences_[prefs_list::unmoved_orb_color].str();
	if (unmoved_color.empty())
		return game_config::colors::unmoved_orb_color;
	return fix_orb_color_name(unmoved_color);
}
void prefs::set_unmoved_color(const std::string& color_id) {
	preferences_[prefs_list::unmoved_orb_color] = color_id;
}

std::string prefs::partial_color() {
	std::string partmoved_color = preferences_[prefs_list::partial_orb_color].str();
	if (partmoved_color.empty())
		return game_config::colors::partial_orb_color;
	return fix_orb_color_name(partmoved_color);
}
void prefs::set_partial_color(const std::string& color_id) {
	preferences_[prefs_list::partial_orb_color] = color_id;
}

point prefs::resolution()
{
	const unsigned x_res = preferences_[prefs_list::xresolution].to_unsigned();
	const unsigned y_res = preferences_[prefs_list::yresolution].to_unsigned();

	// Either resolution was unspecified, return default.
	if(x_res == 0 || y_res == 0) {
		return point(pref_constants::def_window_width, pref_constants::def_window_height);
	}

	return point(
		std::max<unsigned>(x_res, pref_constants::min_window_width),
		std::max<unsigned>(y_res, pref_constants::min_window_height)
	);
}

void prefs::set_resolution(const point& res)
{
	preferences_[prefs_list::xresolution] = std::to_string(res.x);
	preferences_[prefs_list::yresolution] = std::to_string(res.y);
}

int prefs::pixel_scale()
{
	// For now this has a minimum value of 1 and a maximum of 4.
	return std::max<int>(std::min<int>(preferences_[prefs_list::pixel_scale].to_int(1), pref_constants::max_pixel_scale), pref_constants::min_pixel_scale);
}

void prefs::set_pixel_scale(const int scale)
{
	preferences_[prefs_list::pixel_scale] = std::clamp(scale, pref_constants::min_pixel_scale, pref_constants::max_pixel_scale);
}

bool prefs::turbo()
{
	if(video::headless()) {
		return true;
	}

	return preferences_[prefs_list::turbo].to_bool();
}

void prefs::set_turbo(bool ison)
{
	preferences_[prefs_list::turbo] = ison;
}

int prefs::font_scaling()
{
	// Clip at 80 because if it's too low it'll cause crashes
	return std::max<int>(std::min<int>(preferences_[prefs_list::font_scale].to_int(100), pref_constants::max_font_scaling), pref_constants::min_font_scaling);
}

void prefs::set_font_scaling(int scale)
{
	preferences_[prefs_list::font_scale] = std::clamp(scale, pref_constants::min_font_scaling, pref_constants::max_font_scaling);
}

int prefs::font_scaled(int size)
{
	return (size * font_scaling()) / 100;
}

int prefs::keepalive_timeout()
{
	return preferences_[prefs_list::keepalive_timeout].to_int(20);
}

void prefs::keepalive_timeout(int seconds)
{
	preferences_[prefs_list::keepalive_timeout] = std::abs(seconds);
}

std::size_t prefs::sound_buffer_size()
{
	// Sounds don't sound good on Windows unless the buffer size is 4k,
	// but this seems to cause crashes on other systems...
	#ifdef _WIN32
		const std::size_t buf_size = 4096;
	#else
		const std::size_t buf_size = 1024;
	#endif

	return preferences_[prefs_list::sound_buffer_size].to_int(buf_size);
}

void prefs::save_sound_buffer_size(const std::size_t size)
{
	const std::string new_size = std::to_string(size);
	if (preferences_[prefs_list::sound_buffer_size] == new_size)
		return;

	preferences_[prefs_list::sound_buffer_size] = new_size;

	sound::reset_sound();
}

int prefs::music_volume()
{
	return preferences_[prefs_list::music_volume].to_int(100);
}

void prefs::set_music_volume(int vol)
{
	if(music_volume() == vol) {
		return;
	}

	preferences_[prefs_list::music_volume] = vol;
	sound::set_music_volume(music_volume());
}

int prefs::sound_volume()
{
	return preferences_[prefs_list::sound_volume].to_int(100);
}

void prefs::set_sound_volume(int vol)
{
	if(sound_volume() == vol) {
		return;
	}

	preferences_[prefs_list::sound_volume] = vol;
	sound::set_sound_volume(sound_volume());
}

int prefs::bell_volume()
{
	return preferences_[prefs_list::bell_volume].to_int(100);
}

void prefs::set_bell_volume(int vol)
{
	if(bell_volume() == vol) {
		return;
	}

	preferences_[prefs_list::bell_volume] = vol;
	sound::set_bell_volume(bell_volume());
}

// old pref name had uppercase UI
int prefs::ui_volume()
{
	if(preferences_.has_attribute(prefs_list::ui_volume)) {
		return preferences_[prefs_list::ui_volume].to_int(100);
	} else {
		return preferences_["UI_volume"].to_int(100);
	}
}

void prefs::set_ui_volume(int vol)
{
	if(ui_volume() == vol) {
		return;
	}

	preferences_[prefs_list::ui_volume] = vol;
	sound::set_UI_volume(ui_volume());
}

bool prefs::turn_bell()
{
	return preferences_[prefs_list::turn_bell].to_bool(true);
}

bool prefs::set_turn_bell(bool ison)
{
	if(!turn_bell() && ison) {
		preferences_[prefs_list::turn_bell] = true;
		if(!music_on() && !sound() && !ui_sound_on()) {
			if(!sound::init_sound()) {
				preferences_[prefs_list::turn_bell] = false;
				return false;
			}
		}
	} else if(turn_bell() && !ison) {
		preferences_[prefs_list::turn_bell] = false;
		sound::stop_bell();
		if(!music_on() && !sound() && !ui_sound_on())
			sound::close_sound();
	}
	return true;
}

// old pref name had uppercase UI
bool prefs::ui_sound_on()
{
	if(preferences_.has_attribute(prefs_list::ui_sound)) {
		return preferences_[prefs_list::ui_sound].to_bool(true);
	} else {
		return preferences_["UI_sound"].to_bool(true);
	}
}

bool prefs::set_ui_sound(bool ison)
{
	if(!ui_sound_on() && ison) {
		preferences_[prefs_list::ui_sound] = true;
		if(!music_on() && !sound() && !turn_bell()) {
			if(!sound::init_sound()) {
				preferences_[prefs_list::ui_sound] = false;
				return false;
			}
		}
	} else if(ui_sound_on() && !ison) {
		preferences_[prefs_list::ui_sound] = false;
		sound::stop_UI_sound();
		if(!music_on() && !sound() && !turn_bell())
			sound::close_sound();
	}
	return true;
}

bool prefs::message_bell()
{
	return preferences_[prefs_list::message_bell].to_bool(true);
}

bool prefs::sound()
{
	return preferences_[prefs_list::sound].to_bool(true);
}

bool prefs::set_sound(bool ison) {
	if(!sound() && ison) {
		preferences_[prefs_list::sound] = true;
		if(!music_on() && !turn_bell() && !ui_sound_on()) {
			if(!sound::init_sound()) {
				preferences_[prefs_list::sound] = false;
				return false;
			}
		}
	} else if(sound() && !ison) {
		preferences_[prefs_list::sound] = false;
		sound::stop_sound();
		if(!music_on() && !turn_bell() && !ui_sound_on())
			sound::close_sound();
	}
	return true;
}

bool prefs::music_on()
{
	return preferences_[prefs_list::music].to_bool(true);
}

bool prefs::set_music(bool ison) {
	if(!music_on() && ison) {
		preferences_[prefs_list::music] = true;
		if(!sound() && !turn_bell() && !ui_sound_on()) {
			if(!sound::init_sound()) {
				preferences_[prefs_list::music] = false;
				return false;
			}
		}
		else
			sound::play_music();
	} else if(music_on() && !ison) {
		preferences_[prefs_list::music] = false;
		if(!sound() && !turn_bell() && !ui_sound_on())
			sound::close_sound();
		else
			sound::stop_music();
	}
	return true;
}

int prefs::scroll_speed()
{
	return std::clamp<int>(preferences_[prefs_list::scroll].to_int(50), 1, 100);
}

void prefs::set_scroll_speed(const int new_speed)
{
	preferences_[prefs_list::scroll] = new_speed;
}

bool prefs::middle_click_scrolls()
{
	return preferences_[prefs_list::middle_click_scrolls].to_bool(true);
}

int prefs::mouse_scroll_threshold()
{
	return preferences_[prefs_list::scroll_threshold].to_int(10);
}

bool prefs::show_fps()
{
	return fps_;
}

void prefs::set_show_fps(bool value)
{
	fps_ = value;
}

void prefs::load_hotkeys()
{
	hotkey::load_custom_hotkeys(game_config_view::wrap(preferences_));
}

void prefs::save_hotkeys()
{
	hotkey::save_hotkeys(preferences_);
}

void prefs::clear_hotkeys()
{
	hotkey::reset_default_hotkeys();
	preferences_.clear_children("hotkey");
}

void prefs::add_alias(const std::string &alias, const std::string &command)
{
	config &alias_list = preferences_.child_or_add("alias");
	alias_list[alias] = command;
}


optional_const_config prefs::get_alias()
{
	return get_child(prefs_list::alias);
}

unsigned int prefs::sample_rate()
{
	return preferences_[prefs_list::sample_rate].to_int(44100);
}

void prefs::save_sample_rate(const unsigned int rate)
{
	if (sample_rate() == rate)
		return;

	preferences_[prefs_list::sample_rate] = rate;

	// If audio is open, we have to re set sample rate
	sound::reset_sound();
}

bool prefs::confirm_load_save_from_different_version()
{
	return preferences_[prefs_list::confirm_load_save_from_different_version].to_bool(true);
}

bool prefs::use_twelve_hour_clock_format()
{
	return preferences_[prefs_list::use_twelve_hour_clock_format].to_bool();
}

sort_order::type prefs::addon_manager_saved_order_direction()
{
	return sort_order::get_enum(preferences_[prefs_list::addon_manager_saved_order_direction]).value_or(sort_order::type::none);
}

void prefs::set_addon_manager_saved_order_direction(sort_order::type value)
{
	preferences_[prefs_list::addon_manager_saved_order_direction] = sort_order::get_string(value);
}

bool prefs::achievement(const std::string& content_for, const std::string& id)
{
	for(config& ach : preferences_.child_range(prefs_list::achievements))
	{
		if(ach["content_for"].str() == content_for)
		{
			std::vector<std::string> ids = utils::split(ach["ids"]);
			return std::find(ids.begin(), ids.end(), id) != ids.end();
		}
	}
	return false;
}

void prefs::set_achievement(const std::string& content_for, const std::string& id)
{
	for(config& ach : preferences_.child_range(prefs_list::achievements))
	{
		// if achievements already exist for this content and the achievement has not already been set, add it
		if(ach["content_for"].str() == content_for)
		{
			std::vector<std::string> ids = utils::split(ach["ids"]);

			if(ids.empty())
			{
				ach["ids"] = id;
			}
			else if(std::find(ids.begin(), ids.end(), id) == ids.end())
			{
				ach["ids"] = ach["ids"].str() + "," + id;
			}
			ach.remove_children("in_progress", [&id](config cfg){return cfg["id"].str() == id;});
			return;
		}
	}

	// else no achievements have been set for this content yet
	config ach;
	ach["content_for"] = content_for;
	ach["ids"] = id;
	preferences_.add_child(prefs_list::achievements, ach);
}

int prefs::progress_achievement(const std::string& content_for, const std::string& id, int limit, int max_progress, int amount)
{
	if(achievement(content_for, id))
	{
		return -1;
	}

	for(config& ach : preferences_.child_range(prefs_list::achievements))
	{
		// if achievements already exist for this content and the achievement has not already been set, add it
		if(ach["content_for"].str() == content_for)
		{
			// check if this achievement has progressed before - if so then increment it
			for(config& in_progress : ach.child_range("in_progress"))
			{
				if(in_progress["id"].str() == id)
				{
					// don't let using 'limit' decrease the achievement's current progress
					int starting_progress = in_progress["progress_at"].to_int();
					if(starting_progress >= limit) {
						return starting_progress;
					}

					in_progress["progress_at"] = std::clamp(starting_progress + amount, 0, std::min(limit, max_progress));
					return in_progress["progress_at"].to_int();
				}
			}

			// else this is the first time this achievement is progressing
			if(amount != 0)
			{
				config set_progress;
				set_progress["id"] = id;
				set_progress["progress_at"] = std::clamp(amount, 0, std::min(limit, max_progress));

				config& child = ach.add_child("in_progress", set_progress);
				return child["progress_at"].to_int();
			}
			return 0;
		}
	}

	// else not only has this achievement not progressed before, this is the first achievement for this achievement group to be added
	if(amount != 0)
	{
		config ach;
		config set_progress;

		set_progress["id"] = id;
		set_progress["progress_at"] = std::clamp(amount, 0, std::min(limit, max_progress));

		ach["content_for"] = content_for;
		ach["ids"] = "";

		config& child = ach.add_child("in_progress", set_progress);
		preferences_.add_child(prefs_list::achievements, ach);
		return child["progress_at"].to_int();
	}
	return 0;
}

bool prefs::sub_achievement(const std::string& content_for, const std::string& id, const std::string& sub_id)
{
	// this achievement is already completed
	if(achievement(content_for, id))
	{
		return true;
	}

	for(config& ach : preferences_.child_range(prefs_list::achievements))
	{
		if(ach["content_for"].str() == content_for)
		{
			// check if the specific sub-achievement has been completed but the overall achievement is not completed
			for(const auto& in_progress : ach.child_range("in_progress"))
			{
				if(in_progress["id"] == id)
				{
					std::vector<std::string> sub_ids = utils::split(in_progress["sub_ids"]);
					return std::find(sub_ids.begin(), sub_ids.end(), sub_id) != sub_ids.end();
				}
			}
		}
	}
	return false;
}

void prefs::set_sub_achievement(const std::string& content_for, const std::string& id, const std::string& sub_id)
{
	// this achievement is already completed
	if(achievement(content_for, id))
	{
		return;
	}

	for(config& ach : preferences_.child_range(prefs_list::achievements))
	{
		// if achievements already exist for this content and the achievement has not already been set, add it
		if(ach["content_for"].str() == content_for)
		{
			// check if this achievement has had sub-achievements set before
			for(config& in_progress : ach.child_range("in_progress"))
			{
				if(in_progress["id"].str() == id)
				{
					std::vector<std::string> sub_ids = utils::split(ach["ids"]);

					if(std::find(sub_ids.begin(), sub_ids.end(), sub_id) == sub_ids.end())
					{
						in_progress["sub_ids"] = in_progress["sub_ids"].str() + "," + sub_id;
					}

					in_progress["progress_at"] = sub_ids.size()+1;
					return;
				}
			}

			// else if this is the first sub-achievement being set
			config set_progress;
			set_progress["id"] = id;
			set_progress["sub_ids"] = sub_id;
			set_progress["progress_at"] = 1;
			ach.add_child("in_progress", set_progress);
			return;
		}
	}

	// else not only has this achievement not had a sub-achievement completed before, this is the first achievement for this achievement group to be added
	config ach;
	config set_progress;

	set_progress["id"] = id;
	set_progress["sub_ids"] = sub_id;
	set_progress["progress_at"] = 1;

	ach["content_for"] = content_for;
	ach["ids"] = "";

	ach.add_child("in_progress", set_progress);
	preferences_.add_child(prefs_list::achievements, ach);
}

bool prefs::get_show_deprecation(bool def)
{
	return preferences_[prefs_list::show_deprecation].to_bool(def);
}

bool prefs::get_scroll_when_mouse_outside(bool def)
{
	return preferences_[prefs_list::scroll_when_mouse_outside].to_bool(def);
}

void prefs::set_dir_bookmarks(const config& cfg)
{
	set_child(prefs_list::dir_bookmarks, cfg);
}
optional_const_config prefs::dir_bookmarks()
{
	return get_child(prefs_list::dir_bookmarks);
}

bool prefs::auto_open_whisper_windows()
{
	return preferences_[prefs_list::lobby_auto_open_whisper_windows].to_bool(true);
}

std::size_t prefs::editor_mru_limit()
{
	return std::max(std::size_t(1), preferences_[prefs_list::editor_max_recent_files].to_size_t(10));
}

//
// NOTE: The MRU read/save functions enforce the entry count limit in
// order to ensure the list on disk doesn't grow forever. Otherwise,
// normally this would be the UI's responsibility instead.
//

std::vector<std::string> prefs::do_read_editor_mru()
{
	auto cfg = get_child(prefs_list::editor_recent_files);

	std::vector<std::string> mru;
	if(!cfg) {
		return mru;
	}

	for(const config& child : cfg->child_range("entry"))
	{
		const std::string& entry = child["path"].str();
		if(!entry.empty()) {
			mru.push_back(entry);
		}
	}

	mru.resize(std::min(editor_mru_limit(), mru.size()));

	return mru;
}

void prefs::do_commit_editor_mru(const std::vector<std::string>& mru)
{
	config cfg;
	unsigned n = 0;

	for(const std::string& entry : mru)
	{
		if(entry.empty()) {
			continue;
		}

		config& child = cfg.add_child("entry");
		child["path"] = entry;

		if(++n >= editor_mru_limit()) {
			break;
		}
	}

	set_child(prefs_list::editor_recent_files, cfg);
}

std::vector<std::string> prefs::recent_files()
{
	return do_read_editor_mru();
}

void prefs::add_recent_files_entry(const std::string& path)
{
	if(path.empty()) {
		return;
	}

	std::vector<std::string> mru = do_read_editor_mru();

	// Enforce uniqueness. Normally shouldn't do a thing unless somebody
	// has been tampering with the preferences file.
	mru.erase(std::remove(mru.begin(), mru.end(), path), mru.end());

	mru.insert(mru.begin(), path);
	mru.resize(std::min(editor_mru_limit(), mru.size()));

	do_commit_editor_mru(mru);
}

bool prefs::use_color_cursors()
{
	return preferences_[prefs_list::color_cursors].to_bool(true);
}

void prefs::set_color_cursors(bool value)
{
	preferences_[prefs_list::color_cursors] = value;

	cursor::set();
}

bool prefs::show_standing_animations()
{
	return preferences_[prefs_list::unit_standing_animations].to_bool(true);
}

void prefs::set_show_standing_animations(bool value)
{
	preferences_[prefs_list::unit_standing_animations] = value;

	if(display* d = display::get_singleton()) {
		d->reset_standing_animations();
	}
}

bool prefs::show_theme_dialog()
{
	std::vector<theme_info> themes = theme::get_basic_theme_info();

	if (themes.empty()) {
		gui2::show_transient_message("",
			_("No known themes. Try changing from within an existing game."));

		return false;
	}

	gui2::dialogs::theme_list dlg(themes);

	for (std::size_t k = 0; k < themes.size(); ++k) {
		if(themes[k].id == theme()) {
			dlg.set_selected_index(static_cast<int>(k));
		}
	}

	dlg.show();
	const int action = dlg.selected_index();

	if (action >= 0) {
		set_theme(themes[action].id);
		if(display::get_singleton() && resources::gamedata && resources::gamedata->get_theme().empty()) {
			display::get_singleton()->set_theme(themes[action].id);
		}

		return true;
	}

	return false;
}

void prefs::show_wesnothd_server_search()
{
	const std::string filename = filesystem::get_wesnothd_name();

	const std::string& old_path = filesystem::directory_name(get_mp_server_program_name());
	std::string path =
		!old_path.empty() && filesystem::is_directory(old_path)
		? old_path : filesystem::get_exe_dir();

	const std::string msg = VGETTEXT("The <b>$filename</b> server application provides multiplayer server functionality and is required for hosting local network games. It will normally be found in the same folder as the game executable.", {{"filename", filename}});

	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Find Server Application"))
	.set_message(msg)
	.set_ok_label(_("Select"))
	.set_read_only(true)
	.set_filename(filename)
	.set_path(path);

	if(dlg.show()) {
		path = dlg.path();
		set_mp_server_program_name(path);
	}
}

std::string prefs::theme()
{
	if(video::headless()) {
		static const std::string null_theme = "null";
		return null_theme;
	}

	std::string res = preferences_[prefs_list::theme];
	if(res.empty()) {
		return "Default";
	}

	return res;
}

void prefs::set_theme(const std::string& theme)
{
	if(theme != "null") {
		preferences_[prefs_list::theme] = theme;
	}
}

void prefs::set_mp_server_program_name(const std::string& path)
{
	if(path.empty()) {
		preferences_.remove_attribute(prefs_list::mp_server_program_name);
	} else {
		preferences_[prefs_list::mp_server_program_name] = path;
	}
}

std::string prefs::get_mp_server_program_name()
{
	return preferences_[prefs_list::mp_server_program_name].str();
}

const std::map<std::string, preferences::acquaintance>& prefs::get_acquaintances()
{
	return acquaintances_;
}

const std::string prefs::get_ignored_delim()
{
	std::vector<std::string> ignored;

	for(const auto& person : acquaintances_) {
		if(person.second.get_status() == "ignore") {
			ignored.push_back(person.second.get_nick());
		}
	}

	return utils::join(ignored);
}

// returns acquaintances in the form nick => notes where the status = filter
std::map<std::string, std::string> prefs::get_acquaintances_nice(const std::string& filter)
{
	std::map<std::string, std::string> ac_nice;

	for(const auto& a : acquaintances_) {
		if(a.second.get_status() == filter) {
			ac_nice[a.second.get_nick()] = a.second.get_notes();
		}
	}

	return ac_nice;
}

std::pair<preferences::acquaintance*, bool> prefs::add_acquaintance(const std::string& nick, const std::string& mode, const std::string& notes)
{
	if(!utils::isvalid_wildcard(nick)) {
		return std::pair(nullptr, false);
	}

	preferences::acquaintance new_entry(nick, mode, notes);
	auto [iter, added_new] = acquaintances_.insert_or_assign(nick, new_entry);

	return std::pair(&iter->second, added_new);
}

bool prefs::remove_acquaintance(const std::string& nick)
{
	std::map<std::string, preferences::acquaintance>::iterator i = acquaintances_.find(nick);

	// nick might include the notes, depending on how we're removing
	if(i == acquaintances_.end()) {
		std::size_t pos = nick.find_first_of(' ');

		if(pos != std::string::npos) {
			i = acquaintances_.find(nick.substr(0, pos));
		}
	}

	if(i == acquaintances_.end()) {
		return false;
	}

	acquaintances_.erase(i);

	return true;
}

bool prefs::is_friend(const std::string& nick)
{
	const auto it = acquaintances_.find(nick);

	if(it == acquaintances_.end()) {
		return false;
	} else {
		return it->second.get_status() == "friend";
	}
}

bool prefs::is_ignored(const std::string& nick)
{
	const auto it = acquaintances_.find(nick);

	if(it == acquaintances_.end()) {
		return false;
	} else {
		return it->second.get_status() == "ignore";
	}
}

void prefs::add_completed_campaign(const std::string& campaign_id, const std::string& difficulty_level)
{
	completed_campaigns_[campaign_id].insert(difficulty_level);
}

bool prefs::is_campaign_completed(const std::string& campaign_id)
{
	return completed_campaigns_.count(campaign_id) != 0;
}

bool prefs::is_campaign_completed(const std::string& campaign_id, const std::string& difficulty_level)
{
	const auto it = completed_campaigns_.find(campaign_id);
	return it == completed_campaigns_.end() ? false : it->second.count(difficulty_level) != 0;
}

bool prefs::parse_should_show_lobby_join(const std::string& sender, const std::string& message)
{
	// If it's actually not a lobby join or leave message return true (show it).
	if(sender != "server") {
		return true;
	}

	std::string::size_type pos = message.find(" has logged into the lobby");
	if(pos == std::string::npos) {
		pos = message.find(" has disconnected");
		if(pos == std::string::npos) {
			return true;
		}
	}

	pref_constants::lobby_joins lj = get_lobby_joins();
	if(lj == pref_constants::lobby_joins::show_none) {
		return false;
	}

	if(lj == pref_constants::lobby_joins::show_all) {
		return true;
	}

	return is_friend(message.substr(0, pos));
}

pref_constants::lobby_joins prefs::get_lobby_joins()
{
	std::string pref = preferences_[prefs_list::lobby_joins];
	if(pref == "friends") {
		return pref_constants::lobby_joins::show_friends;
	} else if(pref == "all") {
		return pref_constants::lobby_joins::show_all;
	} else if(pref == "none") {
		return pref_constants::lobby_joins::show_none;
	} else {
		return pref_constants::lobby_joins::show_friends;
	}
}

void prefs::set_lobby_joins(pref_constants::lobby_joins show)
{
	if(show == pref_constants::lobby_joins::show_friends) {
		preferences_[prefs_list::lobby_joins] = "friends";
	} else if(show == pref_constants::lobby_joins::show_all) {
		preferences_[prefs_list::lobby_joins] = "all";
	} else if(show == pref_constants::lobby_joins::show_none) {
		preferences_[prefs_list::lobby_joins] = "none";
	}
}

const std::vector<game_config::server_info>& prefs::builtin_servers_list()
{
	static std::vector<game_config::server_info> pref_servers = game_config::server_list;
	return pref_servers;
}

std::vector<game_config::server_info> prefs::user_servers_list()
{
	std::vector<game_config::server_info> pref_servers;

	for(const config& server : preferences_.child_range("server")) {
		pref_servers.emplace_back();
		pref_servers.back().name = server["name"].str();
		pref_servers.back().address = server["address"].str();
	}

	return pref_servers;
}

void prefs::set_user_servers_list(const std::vector<game_config::server_info>& value)
{
	preferences_.clear_children("server");

	for(const auto& svinfo : value) {
		config& sv_cfg = preferences_.add_child("server");
		sv_cfg["name"] = svinfo.name;
		sv_cfg["address"] = svinfo.address;
	}
}

std::string prefs::network_host()
{
	const std::string res = preferences_[prefs_list::host];
	if(res.empty()) {
		return builtin_servers_list().front().address;
	} else {
		return res;
	}
}

void prefs::set_network_host(const std::string& host)
{
	preferences_[prefs_list::host] = host;
}

std::string prefs::campaign_server()
{
	if(!preferences_[prefs_list::campaign_server].empty()) {
		return preferences_[prefs_list::campaign_server].str();
	} else {
		return pref_constants::default_addons_server;
	}
}

void prefs::set_campaign_server(const std::string& host)
{
	preferences_[prefs_list::campaign_server] = host;
}

bool prefs::show_combat()
{
	return preferences_[prefs_list::show_combat].to_bool(true);
}

const config& prefs::options()
{
	if(options_initialized_) {
		return option_values_;
	}

	if(!get_child(prefs_list::options)) {
		// It may be an invalid config, which would cause problems in
		// multiplayer_create, so let's replace it with an empty but valid
		// config
		option_values_.clear();
	} else {
		option_values_ = *get_child(prefs_list::options);
	}

	options_initialized_ = true;

	return option_values_;
}

void prefs::set_options(const config& values)
{
	set_child(prefs_list::options, values);
	options_initialized_ = false;
}

int prefs::countdown_init_time()
{
	return std::clamp<int>(preferences_[prefs_list::mp_countdown_init_time].to_int(240), 0, 1500);
}

void prefs::set_countdown_init_time(int value)
{
	preferences_[prefs_list::mp_countdown_init_time] = value;
}

void prefs::clear_countdown_init_time()
{
	preferences_.remove_attribute(prefs_list::mp_countdown_init_time);
}

int prefs::countdown_reservoir_time()
{
	return std::clamp<int>(preferences_[prefs_list::mp_countdown_reservoir_time].to_int(360), 30, 1500);
}

void prefs::set_countdown_reservoir_time(int value)
{
	preferences_[prefs_list::mp_countdown_reservoir_time] = value;
}

void prefs::clear_countdown_reservoir_time()
{
	preferences_.remove_attribute(prefs_list::mp_countdown_reservoir_time);
}

int prefs::countdown_turn_bonus()
{
	return std::clamp<int>(preferences_[prefs_list::mp_countdown_turn_bonus].to_int(240), 0, 300);
}

void prefs::set_countdown_turn_bonus(int value)
{
	preferences_[prefs_list::mp_countdown_turn_bonus] = value;
}

void prefs::clear_countdown_turn_bonus()
{
	preferences_.remove_attribute(prefs_list::mp_countdown_turn_bonus);
}

int prefs::countdown_action_bonus()
{
	return std::clamp<int>(preferences_[prefs_list::mp_countdown_action_bonus], 0, 30);
}

void prefs::set_countdown_action_bonus(int value)
{
	preferences_[prefs_list::mp_countdown_action_bonus] = value;
}

void prefs::clear_countdown_action_bonus()
{
	preferences_.remove_attribute(prefs_list::mp_countdown_action_bonus);
}

int prefs::village_gold()
{
	return settings::get_village_gold(preferences_[prefs_list::mp_village_gold]);
}

void prefs::set_village_gold(int value)
{
	preferences_[prefs_list::mp_village_gold] = value;
}

int prefs::village_support()
{
	return settings::get_village_support(preferences_[prefs_list::mp_village_support]);
}

void prefs::set_village_support(int value)
{
	preferences_[prefs_list::mp_village_support] = std::to_string(value);
}

int prefs::xp_modifier()
{
	return settings::get_xp_modifier(preferences_[prefs_list::mp_xp_modifier]);
}

void prefs::set_xp_modifier(int value)
{
	preferences_[prefs_list::mp_xp_modifier] = value;
}

const std::vector<std::string>& prefs::modifications(bool mp)
{
	if((!mp_modifications_initialized_ && mp) || (!sp_modifications_initialized_ && !mp)) {
		if(mp) {
			mp_modifications_ = utils::split(preferences_[prefs_list::mp_modifications].str(), ',');
			mp_modifications_initialized_ = true;
		} else {
			sp_modifications_ = utils::split(preferences_[prefs_list::sp_modifications].str(), ',');
			sp_modifications_initialized_ = true;
		}
	}

	return mp ? mp_modifications_ : sp_modifications_;
}

void prefs::set_modifications(const std::vector<std::string>& value, bool mp)
{
	if(mp) {
		preferences_[prefs_list::mp_modifications] = utils::join(value, ",");
		mp_modifications_initialized_ = false;
	} else {
		preferences_[prefs_list::sp_modifications] = utils::join(value, ",");
		sp_modifications_initialized_ = false;
	}
}

bool prefs::message_private()
{
	return message_private_on_;
}

void prefs::set_message_private(bool value)
{
	message_private_on_ = value;
}

compression::format prefs::save_compression_format()
{
	const std::string& choice = preferences_[prefs_list::compress_saves];

	// "yes" was used in 1.11.7 and earlier; the compress_saves
	// option used to be a toggle for gzip in those versions.
	if(choice.empty() || choice == "gzip" || choice == "yes") {
		return compression::format::gzip;
	} else if(choice == "bzip2") {
		return compression::format::bzip2;
	} else if(choice == "none" || choice == "no") { // see above
		return compression::format::none;
	} /*else*/

	// In case the preferences file was created by a later version
	// supporting some algorithm we don't; although why would anyone
	// playing a game need more algorithms, really...
	return compression::format::gzip;
}

std::string prefs::get_chat_timestamp(const std::time_t& t)
{
	if(chat_timestamp()) {
		if(use_twelve_hour_clock_format() == false) {
			return lg::get_timestamp(t, _("[%H:%M]")) + " ";
		} else {
			return lg::get_timestamp(t, _("[%I:%M %p]")) + " ";
		}
	}

	return "";
}

std::set<std::string>& prefs::encountered_units()
{
	return encountered_units_set_;
}

std::set<t_translation::terrain_code>& prefs::encountered_terrains()
{
	return encountered_terrains_set_;
}

/**
 * Returns a pointer to the history vector associated with given id
 * making a new one if it doesn't exist.
 *
 * @todo FIXME only used for gui2. Could be used for the above histories.
 */
std::vector<std::string>* prefs::get_history(const std::string& id)
{
	return &history_map_[id];
}

bool prefs::green_confirm()
{
	const std::string confirmation = preferences_[prefs_list::confirm_end_turn];
	return confirmation == "green" || confirmation == "yes";
}

bool prefs::yellow_confirm()
{
	return preferences_[prefs_list::confirm_end_turn] == "yellow";
}

bool prefs::confirm_no_moves()
{
	// This is very non-intrusive so it is on by default
	const std::string confirmation = preferences_[prefs_list::confirm_end_turn];
	return confirmation == "no_moves" || confirmation.empty();
}

void prefs::encounter_recruitable_units(const std::vector<team>& teams)
{
	for(const team& help_team : teams) {
		help_team.log_recruitable();
		encountered_units_set_.insert(help_team.recruits().begin(), help_team.recruits().end());
	}
}

void prefs::encounter_start_units(const unit_map& units)
{
	for(const auto& help_unit : units) {
		encountered_units_set_.insert(help_unit.type_id());
	}
}

void prefs::encounter_recallable_units(const std::vector<team>& teams)
{
	for(const team& t : teams) {
		for(const unit_const_ptr u : t.recall_list()) {
			encountered_units_set_.insert(u->type_id());
		}
	}
}

void prefs::encounter_map_terrain(const gamemap& map)
{
	map.for_each_loc([&](const map_location& loc) {
		const t_translation::terrain_code terrain = map.get_terrain(loc);
		encountered_terrains().insert(terrain);
		for(t_translation::terrain_code t : map.underlying_union_terrain(loc)) {
			encountered_terrains().insert(t);
		}
	});
}

void prefs::encounter_all_content(const game_board& gameboard_)
{
	encounter_recruitable_units(gameboard_.teams());
	encounter_start_units(gameboard_.units());
	encounter_recallable_units(gameboard_.teams());
	encounter_map_terrain(gameboard_.map());
}

void prefs::clear_mp_alert_prefs()
{
	preferences_.remove_attribute(prefs_list::player_joins_sound);
	preferences_.remove_attribute(prefs_list::player_joins_notif);
	preferences_.remove_attribute(prefs_list::player_joins_lobby);
	preferences_.remove_attribute(prefs_list::player_leaves_sound);
	preferences_.remove_attribute(prefs_list::player_leaves_notif);
	preferences_.remove_attribute(prefs_list::player_leaves_lobby);
	preferences_.remove_attribute(prefs_list::private_message_sound);
	preferences_.remove_attribute(prefs_list::private_message_notif);
	preferences_.remove_attribute(prefs_list::private_message_lobby);
	preferences_.remove_attribute(prefs_list::friend_message_sound);
	preferences_.remove_attribute(prefs_list::friend_message_notif);
	preferences_.remove_attribute(prefs_list::friend_message_lobby);
	preferences_.remove_attribute(prefs_list::public_message_sound);
	preferences_.remove_attribute(prefs_list::public_message_notif);
	preferences_.remove_attribute(prefs_list::public_message_lobby);
	preferences_.remove_attribute(prefs_list::server_message_sound);
	preferences_.remove_attribute(prefs_list::server_message_notif);
	preferences_.remove_attribute(prefs_list::server_message_lobby);
	preferences_.remove_attribute(prefs_list::ready_for_start_sound);
	preferences_.remove_attribute(prefs_list::ready_for_start_notif);
	preferences_.remove_attribute(prefs_list::ready_for_start_lobby);
	preferences_.remove_attribute(prefs_list::game_has_begun_sound);
	preferences_.remove_attribute(prefs_list::game_has_begun_notif);
	preferences_.remove_attribute(prefs_list::game_has_begun_lobby);
	preferences_.remove_attribute(prefs_list::turn_changed_sound);
	preferences_.remove_attribute(prefs_list::turn_changed_notif);
	preferences_.remove_attribute(prefs_list::turn_changed_lobby);
	preferences_.remove_attribute(prefs_list::game_created_sound);
	preferences_.remove_attribute(prefs_list::game_created_notif);
	preferences_.remove_attribute(prefs_list::game_created_lobby);
}

std::string prefs::get_system_username()
{
	std::string res;
#ifdef _WIN32
	wchar_t buffer[300];
	DWORD size = 300;
	if(GetUserNameW(buffer, &size)) {
		//size includes a terminating null character.
		assert(size > 0);
		res = unicode_cast<std::string>(boost::iterator_range<wchar_t*>(buffer, buffer + size - 1));
	}
#else
	if(char* const login = getenv("USER")) {
		res = login;
	}
#endif
	return res;
}

preferences::secure_buffer prefs::build_key(const std::string& server, const std::string& login)
{
	std::string sysname = get_system_username();
	preferences::secure_buffer result(std::max<std::size_t>(server.size() + login.size() + sysname.size(), 32));
	unsigned char i = 0;
	std::generate(result.begin(), result.end(), [&i]() {return 'x' ^ i++;});
	std::copy(login.begin(), login.end(), result.begin());
	std::copy(sysname.begin(), sysname.end(), result.begin() + login.size());
	std::copy(server.begin(), server.end(), result.begin() + login.size() + sysname.size());
	return result;
}

preferences::secure_buffer prefs::aes_encrypt(const preferences::secure_buffer& plaintext, const preferences::secure_buffer& key)
{
#ifndef __APPLE__
	int update_length;
	int extra_length;
	int total_length;
	// AES IV is generally 128 bits
	const unsigned char iv[] = {1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8};
	unsigned char encrypted_buffer[1024];

	if(plaintext.size() > 1008)
	{
		ERR_CFG << "Cannot encrypt data larger than 1008 bytes.";
		return preferences::secure_buffer();
	}
	DBG_CFG << "Encrypting data with length: " << plaintext.size();

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if(!ctx)
	{
		ERR_CFG << "AES EVP_CIPHER_CTX_new failed with error:";
		ERR_CFG << ERR_error_string(ERR_get_error(), NULL);
		return preferences::secure_buffer();
	}

	// TODO: use EVP_EncryptInit_ex2 once openssl 3.0 is more widespread
	if(EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv) != 1)
	{
		ERR_CFG << "AES EVP_EncryptInit_ex failed with error:";
		ERR_CFG << ERR_error_string(ERR_get_error(), NULL);
		EVP_CIPHER_CTX_free(ctx);
		return preferences::secure_buffer();
	}

	if(EVP_EncryptUpdate(ctx, encrypted_buffer, &update_length, plaintext.data(), plaintext.size()) != 1)
	{
		ERR_CFG << "AES EVP_EncryptUpdate failed with error:";
		ERR_CFG << ERR_error_string(ERR_get_error(), NULL);
		EVP_CIPHER_CTX_free(ctx);
		return preferences::secure_buffer();
	}
	DBG_CFG << "Update length: " << update_length;

	if(EVP_EncryptFinal_ex(ctx, encrypted_buffer + update_length, &extra_length) != 1)
	{
		ERR_CFG << "AES EVP_EncryptFinal failed with error:";
		ERR_CFG << ERR_error_string(ERR_get_error(), NULL);
		EVP_CIPHER_CTX_free(ctx);
		return preferences::secure_buffer();
	}
	DBG_CFG << "Extra length: " << extra_length;

	EVP_CIPHER_CTX_free(ctx);

	total_length = update_length+extra_length;
	preferences::secure_buffer result;
	for(int i = 0; i < total_length; i++)
	{
		result.push_back(encrypted_buffer[i]);
	}

	DBG_CFG << "Successfully encrypted plaintext value of '" << utils::join(plaintext, "") << "' having length " << plaintext.size();
	DBG_CFG << "For a total encrypted length of: " << total_length;

	return result;
#else
	size_t outWritten = 0;
	preferences::secure_buffer result(plaintext.size(), '\0');

	CCCryptorStatus ccStatus = CCCrypt(kCCDecrypt,
		kCCAlgorithmRC4,
		kCCOptionPKCS7Padding,
		key.data(),
		key.size(),
		nullptr,
		plaintext.data(),
		plaintext.size(),
		result.data(),
		result.size(),
		&outWritten);

	assert(ccStatus == kCCSuccess);
	assert(outWritten == plaintext.size());

	return result;
#endif
}

preferences::secure_buffer prefs::aes_decrypt(const preferences::secure_buffer& encrypted, const preferences::secure_buffer& key)
{
#ifndef __APPLE__
	int update_length;
	int extra_length;
	int total_length;
	// AES IV is generally 128 bits
	const unsigned char iv[] = {1,2,3,4,5,6,7,8,1,2,3,4,5,6,7,8};
	unsigned char plaintext_buffer[1024];

	if(encrypted.size() > 1024)
	{
		ERR_CFG << "Cannot decrypt data larger than 1024 bytes.";
		return preferences::secure_buffer();
	}
	DBG_CFG << "Decrypting data with length: " << encrypted.size();

	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if(!ctx)
	{
		ERR_CFG << "AES EVP_CIPHER_CTX_new failed with error:";
		ERR_CFG << ERR_error_string(ERR_get_error(), NULL);
		return preferences::secure_buffer();
	}

	// TODO: use EVP_DecryptInit_ex2 once openssl 3.0 is more widespread
	if(EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key.data(), iv) != 1)
	{
		ERR_CFG << "AES EVP_DecryptInit_ex failed with error:";
		ERR_CFG << ERR_error_string(ERR_get_error(), NULL);
		EVP_CIPHER_CTX_free(ctx);
		return preferences::secure_buffer();
	}

	if(EVP_DecryptUpdate(ctx, plaintext_buffer, &update_length, encrypted.data(), encrypted.size()) != 1)
	{
		ERR_CFG << "AES EVP_DecryptUpdate failed with error:";
		ERR_CFG << ERR_error_string(ERR_get_error(), NULL);
		EVP_CIPHER_CTX_free(ctx);
		return preferences::secure_buffer();
	}
	DBG_CFG << "Update length: " << update_length;

	if(EVP_DecryptFinal_ex(ctx, plaintext_buffer + update_length, &extra_length) != 1)
	{
		ERR_CFG << "AES EVP_DecryptFinal failed with error:";
		ERR_CFG << ERR_error_string(ERR_get_error(), NULL);
		EVP_CIPHER_CTX_free(ctx);
		return preferences::secure_buffer();
	}
	DBG_CFG << "Extra length: " << extra_length;

	EVP_CIPHER_CTX_free(ctx);

	total_length = update_length+extra_length;
	preferences::secure_buffer result;
	for(int i = 0; i < total_length; i++)
	{
		result.push_back(plaintext_buffer[i]);
	}

	DBG_CFG << "Successfully decrypted data to the value: " << utils::join(result, "");
	DBG_CFG << "For a total decrypted length of: " << total_length;

	return result;
#else
	size_t outWritten = 0;
	preferences::secure_buffer result(encrypted.size(), '\0');

	CCCryptorStatus ccStatus = CCCrypt(kCCDecrypt,
		kCCAlgorithmRC4,
		kCCOptionPKCS7Padding,
		key.data(),
		key.size(),
		nullptr,
		encrypted.data(),
		encrypted.size(),
		result.data(),
		result.size(),
		&outWritten);

	assert(ccStatus == kCCSuccess);
	assert(outWritten == encrypted.size());

	// the decrypted result is likely shorter than the encrypted data, so the extra padding needs to be removed.
	while(!result.empty() && result.back() == 0) {
		result.pop_back();
	}

	return result;
#endif
}

preferences::secure_buffer prefs::unescape(const preferences::secure_buffer& text)
{
	preferences::secure_buffer unescaped;
	unescaped.reserve(text.size());
	bool escaping = false;
	for(char c : text) {
		if(escaping) {
			if(c == '\xa') {
				unescaped.push_back('\xc');
			} else if(c == '.') {
				unescaped.push_back('@');
			} else {
				unescaped.push_back(c);
			}
			escaping = false;
		} else if(c == '\x1') {
			escaping = true;
		} else {
			unescaped.push_back(c);
		}
	}
	assert(!escaping);
	return unescaped;
}

preferences::secure_buffer prefs::escape(const preferences::secure_buffer& text)
{
	preferences::secure_buffer escaped;
	escaped.reserve(text.size());
	for(char c : text) {
		if(c == '\x1') {
			escaped.push_back('\x1');
			escaped.push_back('\x1');
		} else if(c == '\xc') {
			escaped.push_back('\x1');
			escaped.push_back('\xa');
		} else if(c == '@') {
			escaped.push_back('\x1');
			escaped.push_back('.');
		} else {
			escaped.push_back(c);
		}
	}
	return escaped;
}

bool prefs::remember_password()
{
	return preferences_[prefs_list::remember_password].to_bool();
}

void prefs::set_remember_password(bool remember)
{
	preferences_[prefs_list::remember_password] = remember;

	if(remember) {
		load_credentials();
	} else {
		clear_credentials();
	}
}

std::string prefs::login()
{
	std::string name = get("login", pref_constants::EMPTY_LOGIN);
	if(name == pref_constants::EMPTY_LOGIN) {
		name = get_system_username();
	} else if(name.size() > 2 && name.front() == '@' && name.back() == '@') {
		name = name.substr(1, name.size() - 2);
	} else {
		ERR_CFG << "malformed user credentials (did you manually edit the preferences file?)";
	}
	if(name.empty()) {
		return "player";
	}
	return name;
}

void prefs::set_login(const std::string& login)
{
	auto login_clean = login;
	boost::trim(login_clean);

	preferences_[prefs_list::login] = '@' + login_clean + '@';
}

std::string prefs::password(const std::string& server, const std::string& login)
{
	DBG_CFG << "Retrieving password for server: '" << server << "', login: '" << login << "'";
	auto login_clean = login;
	boost::trim(login_clean);

	if(!remember_password()) {
		if(!credentials_.empty() && credentials_[0].username == login_clean && credentials_[0].server == server) {
			auto temp = aes_decrypt(credentials_[0].key, build_key(server, login_clean));
			return std::string(temp.begin(), temp.end());
		} else {
			return "";
		}
	}
	auto cred = std::find_if(credentials_.begin(), credentials_.end(), [&](const preferences::login_info& cred) {
		return cred.server == server && cred.username == login_clean;
	});
	if(cred == credentials_.end()) {
		return "";
	}
	auto temp = aes_decrypt(cred->key, build_key(server, login_clean));
	return std::string(temp.begin(), temp.end());
}

void prefs::set_password(const std::string& server, const std::string& login, const std::string& key)
{
	DBG_CFG << "Setting password for server: '" << server << "', login: '" << login << "'";
	auto login_clean = login;
	boost::trim(login_clean);

	preferences::secure_buffer temp(key.begin(), key.end());
	if(!remember_password()) {
		clear_credentials();
		credentials_.emplace_back(login_clean, server, aes_encrypt(temp, build_key(server, login_clean)));
		return;
	}
	auto cred = std::find_if(credentials_.begin(), credentials_.end(), [&](const preferences::login_info& cred) {
		return cred.server == server && cred.username == login_clean;
	});
	if(cred == credentials_.end()) {
		// This is equivalent to emplace_back, but also returns the iterator to the new element
		cred = credentials_.emplace(credentials_.end(), login_clean, server);
	}
	cred->key = aes_encrypt(temp, build_key(server, login_clean));
}
