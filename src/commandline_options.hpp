/*
   Copyright (C) 2011 - 2018 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
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

#include <optional>

#include <boost/program_options/options_description.hpp>
#include <iosfwd>                       // for ostream
#include <string>                       // for string
#include <tuple>
#include <vector>                       // for vector

class bad_commandline_resolution : public boost::program_options::error
{
public:
    bad_commandline_resolution(const std::string& resolution);
};

class bad_commandline_tuple : public boost::program_options::error
{
public:
	bad_commandline_tuple(const std::string& str,
						  const std::string& expected_format);
};

class config;

class commandline_options
{
	/** To be used for printing help to the commandline. */
	friend std::ostream& operator<<(std::ostream &os, const commandline_options& cmdline_opts);

public:
	commandline_options(const std::vector<std::string>& args);

	config to_config() const; /* Used by lua scrips. Not all of the options need to be exposed here, just those exposed to lua */

	/** True if the --validate or any of the --validate-*  options are given. */
	bool any_validation_option() const;

	/** Non-empty if --bunzip2 was given on the command line. Uncompresses a .bz2 file and exits. */
	std::optional<std::string> bunzip2;
	/** Non-empty if --bzip2 was given on the command line. Compresses a file to .bz2 and exits. */
	std::optional<std::string> bzip2;
	/** Non-empty if --campaign was given on the command line. ID of the campaign we want to start. */
	std::optional<std::string> campaign;
	/** Non-empty if --campaign-difficulty was given on the command line. Numerical difficulty of the campaign to be played. Dependent on --campaign. */
	std::optional<int> campaign_difficulty;
	/** Non-empty if --campaign-scenario was given on the command line. Chooses starting scenario in the campaign to be played. Dependent on --campaign. */
	std::optional<std::string> campaign_scenario;
	/** True if --skip-story was given on the command line. Skips [story] and [message]s through the end of the "start" event. Dependent on --campaign. */
	bool campaign_skip_story;
	/** True if --clock was given on the command line. Enables */
	bool clock;
	/** Non-empty if --core was given on the command line. Chooses the core to be loaded. */
	std::optional<std::string> core_id;
	/** True if --data-path was given on the command line. Prints path to data directory and exits. */
	bool data_path;
	/** Non-empty if --data-dir was given on the command line. Sets the config dir to the specified one. */
	std::optional<std::string> data_dir;
	/** True if --debug was given on the command line. Enables debug mode. */
	bool debug;
	/** True if --debug-lua was given in the commandline. Enables some Lua debugging mechanisms. */
	bool debug_lua;
	/** True if --strict-lua was given in the commandline. Disallows use of deprecated APIs. */
	bool strict_lua;
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	/** Non-empty if --debug-dot-domain was given on the command line. */
	std::optional<std::string> debug_dot_domain;
	/** Non-empty if --debug-dot-level was given on the command line. */
	std::optional<std::string> debug_dot_level;
#endif
	/** Non-empty if --editor was given on the command line. Goes directly into editor. If string is longer than 0, it contains path to the file to edit. */
	std::optional<std::string> editor;
	/** True if --fps was given on the command line. Shows number of fps. */
	bool fps;
	/** True if --fullscreen was given on the command line. Starts Wesnoth in fullscreen mode. */
	bool fullscreen;
	/** Non-empty if --gunzip was given on the command line. Uncompresses a .gz file and exits. */
	std::optional<std::string> gunzip;
	/** Non-empty if --gzip was given on the command line. Compresses a file to .gz and exits. */
	std::optional<std::string> gzip;
	/** True if --help was given on the command line. Prints help and exits. */
	bool help;
	/** Non-empty if --language was given on the command line. Sets the language for this session. */
	std::optional<std::string> language;
	/**
	 * Contains parsed arguments of --log-* (e.g. --log-debug).
	 * Vector of pairs (severity, log domain).
	 */
	std::optional<std::vector<std::pair<int, std::string>>> log;
	/** Non-empty if --log-strict was given */
	std::optional<int> log_strict_level;
	/** Non-empty if --load was given on the command line. Savegame specified to load after start. */
	std::optional<std::string> load;
	/** Non-empty if --logdomains was given on the command line. Prints possible logdomains filtered by given string and exits. */
	std::optional<std::string> logdomains;
	/** True if --log-precise was given on the command line. Shows timestamps in log with more precision. */
	bool log_precise_timestamps;
	/** True if --multiplayer was given on the command line. Goes directly into multiplayer mode. */
	bool multiplayer;
	/** Non-empty if --ai-config was given on the command line. Vector of pairs (side number, value). Dependent on --multiplayer. */
	std::optional<std::vector<std::pair<unsigned int, std::string>>> multiplayer_ai_config;
	/** Non-empty if --algorithm was given on the command line. Vector of pairs (side number, value). Dependent on --multiplayer. */
	std::optional<std::vector<std::pair<unsigned int, std::string>>> multiplayer_algorithm;
	/** Non-empty if --controller was given on the command line. Vector of pairs (side number, controller). Dependent on --multiplayer. */
	std::optional<std::vector<std::pair<unsigned int, std::string>>> multiplayer_controller;
	/** Non-empty if --era was given on the command line. Dependent on --multiplayer. */
	std::optional<std::string> multiplayer_era;
	/** True if --exit-at-and was given on the command line. Dependent on --multiplayer. */
	bool multiplayer_exit_at_end;
	/** True if --ignore-map-settings was given at the command line.  Do not use map settings. */
	bool multiplayer_ignore_map_settings;
	/** Non-empty if --label was given on the command line. Dependent on --multiplayer. */
	std::optional<std::string> multiplayer_label;
	/** Non-empty if --parm was given on the command line. Vector of pairs (side number, parm name, parm value). Dependent on --multiplayer. */
	std::optional<std::vector<std::tuple<unsigned int, std::string, std::string>>> multiplayer_parm;
	/** Repeats specified by --multiplayer-repeat option. Repeats a multiplayer game after it is finished. Dependent on --multiplayer. */
	std::optional<unsigned int> multiplayer_repeat;
	/** Non-empty if --scenario was given on the command line. Dependent on --multiplayer. */
	std::optional<std::string> multiplayer_scenario;
	/** Non-empty if --side was given on the command line. Vector of pairs (side number, faction id). Dependent on --multiplayer. */
	std::optional<std::vector<std::pair<unsigned int, std::string>>> multiplayer_side;
	/** Non-empty if --turns was given on the command line. Dependent on --multiplayer. */
	std::optional<std::string> multiplayer_turns;
	/** Max FPS specified by --max-fps option. */
	std::optional<int> max_fps;
	/** True if --noaddons was given on the command line. Disables the loading of all add-ons. */
	bool noaddons;
	/** True if --nocache was given on the command line. Disables cache usage. */
	bool nocache;
	/** True if --nodelay was given on the command line. */
	bool nodelay;
	/** True if --nogui was given on the command line. Disables GUI. */
	bool nogui;
	/** True if --nobanner was given on the command line. Disables startup banner. */
	bool nobanner;
	/** True if --nomusic was given on the command line. Disables music. */
	bool nomusic;
	/** True if --nosound was given on the command line. Disables sound. */
	bool nosound;
	/** True if --new-widgets was given on the command line. Hidden option to enable the new widget toolkit. */
	bool new_widgets;
	/** True if --preprocess was given on the command line. Starts Wesnoth in preprocessor-only mode. */
	bool preprocess;
	/** Defines that were given to the --preprocess option. */
	std::optional<std::vector<std::string>> preprocess_defines;
	/** Non-empty if --preprocess-input-macros was given on the command line. Specifies a file that contains [preproc_define]s to be included before preprocessing. Dependent on --preprocess. */
	std::optional<std::string> preprocess_input_macros;
	/** Non-empty if --preprocess-output-macros was given on the command line. Outputs all preprocessed macros to the specified file. Dependent on --preprocess. */
	std::optional<std::string> preprocess_output_macros;
	/** Path to parse that was given to the --preprocess option. */
	std::optional<std::string> preprocess_path;
	/** Target (output) path that was given to the --preprocess option. */
	std::optional<std::string> preprocess_target;
	/** Pair of AxB values specified after --resolution. Changes Wesnoth resolution. */
	std::optional<std::pair<int,int>> resolution;
	/** RNG seed specified by --rng-seed option. Initializes RNG with given seed. */
	std::optional<unsigned int> rng_seed;
	/** Non-empty if --server was given on the command line.  Connects Wesnoth to specified server. If no server was specified afterwards, contains an empty string. */
	std::optional<std::string> server;
	/** Non-empty if --username was given on the command line. Forces Wesnoth to use this network username. */
	std::optional<std::string> username;
	/** Non-empty if --password was given on the command line. Forces Wesnoth to use this network password. */
	std::optional<std::string> password;
	/** Image path to render. First parameter after --render-image */
	std::optional<std::string> render_image;
	/** Output file to put rendered image path in. Optional second parameter after --render-image */
	std::optional<std::string> render_image_dst;
	/** True if --screenshot was given on the command line. Starts Wesnoth in screenshot mode. */
	bool screenshot;
	/** Map file to make a screenshot of. First parameter given after --screenshot. */
	std::optional<std::string> screenshot_map_file;
	/** Output file to put screenshot in. Second parameter given after --screenshot. */
	std::optional<std::string> screenshot_output_file;
	/** File to load lua script from. */
	std::optional<std::string> script_file;
	/** File to load a lua plugin (similar to a script) from. Experimental / may replace script. */
	std::optional<std::string> plugin_file;
	/** Whether to load the "package" package for the scripting environment. (This allows to load arbitrary lua packages, and gives untrusted lua the same permissions as wesnoth executable) */
	bool script_unsafe_mode;
	/** True if --strict-validation was given on the command line. Makes Wesnoth trust validation errors as fatal WML errors and create WML exception, if so. */
	bool strict_validation;
	/** Non-empty if --test was given on the command line. Goes directly into test mode, into a scenario, if specified. */
	std::optional<std::string> test;
	/** Non-empty if --unit was given on the command line. Goes directly into unit test mode, into a scenario, if specified. */
	std::vector<std::string> unit_test;
	/** True if --unit is used and --showgui is not present. */
	bool headless_unit_test;
	/** True if --noreplaycheck was given on the command line. Dependent on --unit. */
	bool noreplaycheck;
	/** True if --mp-test was given on the command line. */
	bool mptest;
	/** True if --userconfig-path was given on the command line. Prints path to user config directory and exits. */
	bool userconfig_path;
	/** Non-empty if --userconfig-dir was given on the command line. Sets the user config dir to the specified one. */
	std::optional<std::string> userconfig_dir;
	/** True if --userdata-path was given on the command line. Prints path to user data directory and exits. */
	bool userdata_path;
	/** Non-empty if --userdata-dir was given on the command line. Sets the user data dir to the specified one. */
	std::optional<std::string> userdata_dir;
	/** True if --validcache was given on the command line. Makes Wesnoth assume the cache is valid. */
	bool validcache;
	/** True if --validate-core was given on the command line. Makes Wesnoth validate the core WML. */
	bool validate_core;
	/** Non-empty if --validate-addon was given on the command line. Makes Wesnoth validate an addon's WML. */
	std::optional<std::string> validate_addon;
	/** Non-empty if --validate-schema was given on the command line. Makes Wesnoth validate a WML schema. */
	std::optional<std::string> validate_schema;
	/** Non-empty if --validate was given on the command line. Makes Wesnoth validate a WML file against a schema. */
	std::optional<std::string> validate_wml;
	/** Non-empty if --use-schema was given on the command line. Specifies the schema for use with --validate. */
	std::optional<std::string> validate_with;
	/** Output filename for WML diff or preprocessing */
	std::optional<std::string> output_file;
	bool do_diff, do_patch;
	/** Files for diffing or patching */
	std::string diff_left, diff_right;
	/** True if --version was given on the command line. Prints version and exits. */
	bool version;
	/** True if --report was given on the command line. Prints a bug report-style info dump and exits. */
	bool report;
	/** True if --windowed was given on the command line. Starts Wesnoth in windowed mode. */
	bool windowed;
	/** True if --with-replay was given on the command line. Shows replay of the loaded file. */
	bool with_replay;
	/** Non-empty if --all-translations or --translations-over is given on the command line. */
	std::optional<unsigned int> translation_percent;
private:
	void parse_log_domains_(const std::string &domains_string, const int severity);
	void parse_log_strictness (const std::string &severity);
	void parse_resolution_ (const std::string &resolution_string);
	/** A helper function splitting vector of strings of format unsigned int:string to vector of tuples (unsigned int,string) */
	std::vector<std::pair<unsigned int,std::string>> parse_to_uint_string_tuples_(const std::vector<std::string> &strings, char separator = ':');
	/** A helper function splitting vector of strings of format unsigned int:string:string to vector of tuples (unsigned int,string,string) */
	std::vector<std::tuple<unsigned int,std::string,std::string>> parse_to_uint_string_string_tuples_(const std::vector<std::string> &strings, char separator = ':');
	std::vector<std::string> args_;
	std::string args0_;
	boost::program_options::options_description all_;
	boost::program_options::options_description visible_;
	boost::program_options::options_description hidden_;
};
