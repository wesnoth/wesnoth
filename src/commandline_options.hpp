/*
   Copyright (C) 2011 - 2014 by Lukasz Dobrogowski <lukasz.dobrogowski@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef COMMANDLINE_OPTIONS_HPP_INCLUDED
#define COMMANDLINE_OPTIONS_HPP_INCLUDED

#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <boost/tuple/tuple.hpp>

#include <string>
#include <vector>

class commandline_options
{
/// To be used for printing help to the commandline.
friend std::ostream& operator<<(std::ostream &os, const commandline_options& cmdline_opts);

public:
	commandline_options(int argc, char **argv);

	/// BitsPerPixel specified by --bpp option.
	boost::optional<int> bpp;
	/// Non-empty if --bunzip2 was given on the command line. Uncompresses a .bz2 file and exits.
	boost::optional<std::string> bunzip2;
	/// Non-empty if --bzip2 was given on the command line. Compresses a file to .bz2 and exits.
	boost::optional<std::string> bzip2;
	/// Non-empty if --campaign was given on the command line. ID of the campaign we want to start.
	boost::optional<std::string> campaign;
	/// Non-empty if --campaign-difficulty was given on the command line. Numerical difficulty of the campaign to be played. Dependent on --campaign.
	boost::optional<int> campaign_difficulty;
	/// Non-empty if --campaign-scenario was given on the command line. Chooses starting scenario in the campaign to be played. Dependent on --campaign.
	boost::optional<std::string> campaign_scenario;
	/// True if --clock was given on the command line. Enables
	bool clock;
	/// True if --data-path was given on the command line. Prints path to data directory and exits.
	bool data_path;
	/// Non-empty if --data-dir was given on the command line. Sets the config dir to the specified one.
	boost::optional<std::string> data_dir;
	/// True if --debug was given on the command line. Enables debug mode.
	bool debug;
	/// True if --debug-lua was given in the commandline. Enables some Lua debugging mechanisms.
	bool debug_lua;
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	/// Non-empty if --debug-dot-domain was given on the command line.
	boost::optional<std::string> debug_dot_domain;
	/// Non-empty if --debug-dot-level was given on the command line.
	boost::optional<std::string> debug_dot_level;
#endif
	/// Non-empty if --editor was given on the command line. Goes directly into editor. If string is longer than 0, it contains path to the file to edit.
	boost::optional<std::string> editor;
	/// True if --fps was given on the command line. Shows number of fps.
	bool fps;
	/// True if --fullscreen was given on the command line. Starts Wesnoth in fullscreen mode.
	bool fullscreen;
	/// Non-empty if --gunzip was given on the command line. Uncompresses a .gz file and exits.
	boost::optional<std::string> gunzip;
	/// Non-empty if --gzip was given on the command line. Compresses a file to .gz and exits.
	boost::optional<std::string> gzip;
	/// True if --help was given on the command line. Prints help and exits.
	bool help;
	/// Non-empty if --language was given on the command line. Sets the language for this session.
	boost::optional<std::string> language;
	/// Contains parsed arguments of --log-* (e.g. --log-debug).
	/// Vector of pairs (severity, log domain).
	boost::optional<std::vector<boost::tuple<int, std::string> > > log;
	/// Non-empty if --load was given on the command line. Savegame specified to load after start.
	boost::optional<std::string> load;
	/// Non-empty if --logdomains was given on the command line. Prints possible logdomains filtered by given string and exits.
	boost::optional<std::string> logdomains;
	/// True if --log-precise was given on the command line. Shows timestamps in log with more precision.
	bool log_precise_timestamps;
	/// True if --multiplayer was given on the command line. Goes directly into multiplayer mode.
	bool multiplayer;
	/// Non-empty if --ai-config was given on the command line. Vector of pairs (side number, value). Dependent on --multiplayer.
	boost::optional<std::vector<boost::tuple<unsigned int, std::string> > > multiplayer_ai_config;
	/// Non-empty if --algorithm was given on the command line. Vector of pairs (side number, value). Dependent on --multiplayer.
	boost::optional<std::vector<boost::tuple<unsigned int, std::string> > > multiplayer_algorithm;
	/// Non-empty if --controller was given on the command line. Vector of pairs (side number, controller). Dependent on --multiplayer.
	boost::optional<std::vector<boost::tuple<unsigned int, std::string> > > multiplayer_controller;
	/// Non-empty if --era was given on the command line. Dependent on --multiplayer.
	boost::optional<std::string> multiplayer_era;
	/// True if --exit-at-and was given on the command line. Dependent on --multiplayer.
	bool multiplayer_exit_at_end;
	/// True if --ignore-map-settings was given at the command line.  Do not use map settings.
	bool multiplayer_ignore_map_settings;
	/// Non-empty if --label was given on the command line. Dependent on --multiplayer.
	boost::optional<std::string> multiplayer_label;
	/// Non-empty if --parm was given on the command line. Vector of pairs (side number, parm name, parm value). Dependent on --multiplayer.
	boost::optional<std::vector<boost::tuple<unsigned int, std::string, std::string> > > multiplayer_parm;
	/// Repeats specified by --multiplayer-repeat option. Repeats a multiplayer game after it is finished. Dependent on --multiplayer.
	boost::optional<unsigned int> multiplayer_repeat;
	/// Non-empty if --scenario was given on the command line. Dependent on --multiplayer.
	boost::optional<std::string> multiplayer_scenario;
	/// Non-empty if --side was given on the command line. Vector of pairs (side number, faction id). Dependent on --multiplayer.
	boost::optional<std::vector<boost::tuple<unsigned int, std::string> > > multiplayer_side;
	/// Non-empty if --turns was given on the command line. Dependent on --multiplayer.
	boost::optional<std::string> multiplayer_turns;
	/// Max FPS specified by --max-fps option.
	boost::optional<int> max_fps;
	/// True if --nocache was given on the command line. Disables cache usage.
	bool nocache;
	/// True if --nodelay was given on the command line.
	bool nodelay;
	/// True if --nogui was given on the command line. Disables GUI.
	bool nogui;
	/// True if --nomusic was given on the command line. Disables music.
	bool nomusic;
	/// True if --nosound was given on the command line. Disables sound.
	bool nosound;
	/// True if --new-widgets was given on the command line. Hidden option to enable the new widget toolkit.
	bool new_widgets;
	/// True if --path was given on the command line. Prints the path to data directory and exits.
	bool path;
	/// True if --preprocess was given on the command line. Starts Wesnoth in preprocessor-only mode.
	bool preprocess;
	/// Defines that were given to the --preprocess option.
	boost::optional<std::vector<std::string> > preprocess_defines;
	/// Non-empty if --preprocess-input-macros was given on the command line. Specifies a file that contains [preproc_define]s to be included before preprocessing. Dependant on --preprocess.
	boost::optional<std::string> preprocess_input_macros;
	/// Non-empty if --preprocess-output-macros was given on the command line. Outputs all preprocessed macros to the specified file. Dependant on --preprocess.
	boost::optional<std::string> preprocess_output_macros;
	/// Path to parse that was given to the --preprocess option.
	boost::optional<std::string> preprocess_path;
	/// Target (output) path that was given to the --preprocess option.
	boost::optional<std::string> preprocess_target;
	/// True if --proxy was given on the command line. Enables proxy mode.
	bool proxy;
	/// Non-empty if --proxy-address was given on the command line.
	boost::optional<std::string> proxy_address;
	/// Non-empty if --proxy-password was given on the command line.
	boost::optional<std::string> proxy_password;
	/// Non-empty if --proxy-port was given on the command line.
	boost::optional<std::string> proxy_port;
	/// Non-empty if --proxy-user was given on the command line.
	boost::optional<std::string> proxy_user;
	/// Pair of AxB values specified after --resolution. Changes Wesnoth resolution.
	boost::optional<boost::tuple<int,int> > resolution;
	/// RNG seed specified by --rng-seed option. Initializes RNG with given seed.
	boost::optional<unsigned int> rng_seed;
	/// Non-empty if --server was given on the command line.  Connects Wesnoth to specified server. If no server was specified afterwards, contains an empty string.
	boost::optional<std::string> server;
	/// Non-empty if --username was given on the command line. Forces Wesnoth to use this network username.
	boost::optional<std::string> username;
	/// Non-empty if --password was given on the command line. Forces Wesnoth to use this network password.
	boost::optional<std::string> password;
	/// True if --screenshot was given on the command line. Starts Wesnoth in screenshot mode.
	bool screenshot;
	/// Map file to make a screenshot of. First parameter given after --screenshot.
	boost::optional<std::string> screenshot_map_file;
	/// Output file to put screenshot in. Second parameter given after --screenshot.
	boost::optional<std::string> screenshot_output_file;
	/// True if --strict-validation was given on the command line. Makes Wesnoth trust validation errors as fatal WML errors and create WML exception, if so.
	bool strict_validation;
	/// Non-empty if --test was given on the command line. Goes directly into test mode, into a scenario, if specified.
	boost::optional<std::string> test;
	/// True if --userconfig-path was given on the command line. Prints path to user config directory and exits.
	bool userconfig_path;
	/// Non-empty if --userconfig-dir was given on the command line. Sets the user config dir to the specified one.
	boost::optional<std::string> userconfig_dir;
	/// True if --userdata-path was given on the command line. Prints path to user data directory and exits.
	bool userdata_path;
	/// Non-empty if --userdata-dir was given on the command line. Sets the user data dir to the specified one.
	boost::optional<std::string> userdata_dir;
	/// True if --validcache was given on the command line. Makes Wesnoth assume the cache is valid.
	bool validcache;
	/// True if --version was given on the command line. Prints version and exits.
	bool version;
	/// True if --windowed was given on the command line. Starts Wesnoth in windowed mode.
	bool windowed;
	/// True if --with-replay was given on the command line. Shows replay of the loaded file.
	bool with_replay;
private:
	void parse_log_domains_(const std::string &domains_string, const int severity);
	void parse_resolution_ (const std::string &resolution_string);
	/// A helper function splitting vector of strings of format unsigned int:string to vector of tuples (unsigned int,string)
	std::vector<boost::tuple<unsigned int,std::string> > parse_to_uint_string_tuples_(const std::vector<std::string> &strings, char separator = ':');
	/// A helper function splitting vector of strings of format unsigned int:string:string to vector of tuples (unsigned int,string,string)
	std::vector<boost::tuple<unsigned int,std::string,std::string> > parse_to_uint_string_string_tuples_(const std::vector<std::string> &strings, char separator = ':');
	int argc_;
	char **argv_;
	boost::program_options::options_description all_;
	boost::program_options::options_description visible_;
	boost::program_options::options_description hidden_;
};

#endif
