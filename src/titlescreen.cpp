/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Shows the titlescreen, with main-menu and tip-of-the-day.
 *
 *  The menu consists of buttons, such als Start-Tutorial, Start-Campaign,
 *  Load-Game, etc.  As decoration, the wesnoth-logo and a landmap in the
 *  background are shown.
 */

#include "titlescreen.hpp"

#include "config.hpp"
#include "filesystem.hpp"
#include "foreach.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"

#include <algorithm>
#include <vector>

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)

static lg::log_domain log_config("config");
#define LOG_CF LOG_STREAM(info, log_config)
#define ERR_CF LOG_STREAM(err, log_config)

/** Read the file with the tips-of-the-day. */
void read_tips_of_day(config& tips_of_day)
{
	tips_of_day.clear();
	LOG_CF << "Loading tips of day\n";
	try {
		scoped_istream stream = preprocess_file(get_wml_location("hardwired/tips.cfg"));
		read(tips_of_day, *stream);
	} catch(config::error&) {
		ERR_CF << "Could not read data/hardwired/tips.cfg\n";
	}

	//we shuffle the tips after each initial loading.
	config::const_child_itors itors = tips_of_day.child_range("tip");
	if (itors.first != itors.second) {
		std::vector<config> tips(itors.first, itors.second);
		std::random_shuffle(tips.begin(), tips.end());
		tips_of_day.clear();
		foreach (const config &tip, tips) {
			tips_of_day.add_child("tip", tip);
		}
	}
}

/** Go to the next tips-of-the-day */
void next_tip_of_day(config& tips_of_day, bool reverse)
{
	// we just rotate the tip list, to avoid the need to keep track
	// of the current one, and keep it valid, cycle it, etc...
	config::const_child_itors itors = tips_of_day.child_range("tip");
	if (itors.first != itors.second) {
		std::vector<config> tips(itors.first, itors.second);
		std::vector<config>::iterator direction =
			reverse ? tips.begin() + 1 : tips.end() - 1;
		std::rotate(tips.begin(), direction, tips.end());
		tips_of_day.clear();
		foreach (const config &tip, tips) {
			tips_of_day.add_child("tip", tip);
		}
	}
}

const config* get_tip_of_day(config& tips_of_day)
{
	if (tips_of_day.empty()) {
		read_tips_of_day(tips_of_day);
	}

	// next_tip_of_day rotate tips, so better stay iterator-safe
	for (int nb_tips = tips_of_day.child_count("tip"); nb_tips > 0;
	     --nb_tips, next_tip_of_day(tips_of_day))
	{
		const config &tip = tips_of_day.child("tip");
		assert(tip);

		const std::vector<std::string> needed_units = utils::split(tip["encountered_units"], ',');
		if (needed_units.empty()) {
			return &tip;
		}
		const std::set<std::string>& seen_units = preferences::encountered_units();

		// test if one of the listed unit types is already encountered
		// if if's a number, test if we have encountered more than this
		for (std::vector<std::string>::const_iterator i = needed_units.begin();
				i != needed_units.end(); ++i) {
			int needed_units_nb = lexical_cast_default<int>(*i,-1);
			if (needed_units_nb !=-1) {
				if (needed_units_nb <= static_cast<int>(seen_units.size())) {
					return &tip;
				}
			} else if (seen_units.find(*i) != seen_units.end()) {
				return &tip;
			}
		}
	}
	// not tip match, someone forget to put an always-match one
	return NULL;
}

