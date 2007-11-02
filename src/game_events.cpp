/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file game_events.cpp
//! Processing of WML-events.

#include "global.hpp"
#include "actions.hpp"
#include "construct_dialog.hpp"
#include "game_display.hpp"
#include "dialogs.hpp"
#include "game_errors.hpp"
#include "game_events.hpp"
#include "image.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "menu_events.hpp"
#include "game_preferences.hpp"
#include "replay.hpp"
#include "SDL_timer.h"
#include "sound.hpp"
#include "soundsource.hpp"
#include "terrain_filter.hpp"
#include "unit_display.hpp"
#include "util.hpp"
#include "wassert.hpp"
#include "gettext.hpp"
#include "variable.hpp"
#include "serialization/string_utils.hpp"
#include "wesconfig.h"

#include <cstdlib>
#include <deque>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

#define LOG_NG LOG_STREAM(info, engine)
#define WRN_NG LOG_STREAM(warn, engine)
#define ERR_NG LOG_STREAM(err, engine)
#define LOG_DP LOG_STREAM(info, display)
#define ERR_CF LOG_STREAM(err, config)

#define LOG_NO LOG_STREAM(info, notifs)
#define WRN_NO LOG_STREAM(warn, notifs)
#define ERR_NO LOG_STREAM(err, notifs)

namespace {

game_display* screen = NULL;
soundsource::manager* soundsources = NULL;
gamemap* game_map = NULL;
unit_map* units = NULL;
std::vector<team>* teams = NULL;
game_state* state_of_game = NULL;
const game_data* game_data_ptr = NULL;
gamestatus* status_ptr = NULL;
int floating_label = 0;
Uint32 unit_mutations = 0;

class event_handler;
std::vector< event_handler > new_handlers;
typedef std::pair< std::string, config* > wmi_command_change;
std::vector< wmi_command_change > wmi_command_changes;

const gui::msecs prevent_misclick_duration = 10;
const gui::msecs average_frame_time = 30;

class wml_event_dialog : public gui::message_dialog {
public:
	wml_event_dialog(game_display &disp, const std::string& title="", const std::string& message="", const gui::DIALOG_TYPE type=gui::MESSAGE)
		: message_dialog(disp, title, message, type)
	{}
	void action(gui::dialog_process_info &info) {
		if(result() == gui::CLOSE_DIALOG && !info.key_down && info.key[SDLK_ESCAPE]) {
			set_result(gui::ESCAPE_DIALOG);
		}
	}
};

} // end anonymous namespace (1)

#ifdef _MSC_VER
// std::getline might be broken in Visual Studio so show a warning
 #if _MSC_VER < 1300
  #ifndef GETLINE_PATCHED
   #pragma message("warning: the std::getline implementation in your compiler might be broken.")
   #pragma message(" http://support.microsoft.com/default.aspx?scid=kb;EN-US;q240015")
  #endif
 #endif
#endif

/**
 * Shows a summary of the errors encountered in WML thusfar,
 * to avoid a lot of the same messages to be shown.
 * Identical messages are shown once, with (between braces)
 * the number of times that message was encountered.
 * The order in which the messages are shown does not need
 * to be the order in which these messages are encountered.
 * Messages are always written to std::cerr.
 */
static void show_wml_errors()
{
	// Get all unique messages in messages,
	// with the number of encounters for these messages
	std::map<std::string, int> messages;
	while(true) {
		std::string msg;
		std::getline(lg::wml_error, msg);

		if(lg::wml_error.eof()) {
			break;
		}

		if(msg == "") {
			continue;
		}

		if(messages.find(msg) == messages.end()) {
			messages[msg] = 1;
		} else {
			messages[msg]++;
		}
	}
	// Make sure the eof flag is cleared otherwise no new messages are shown
	lg::wml_error.clear();

	// Show the messages collected
	std::string caption = "Deprecated WML found";
	for(std::map<std::string, int>::const_iterator itor = messages.begin();
			itor != messages.end(); ++itor) {

		std::stringstream msg;
		msg << itor->first;
		if(itor->second > 1) {
			msg << " (" << itor->second << ")";
		}

		screen->add_chat_message(caption, 0, msg.str(), game_display::MESSAGE_PUBLIC, false);
		std::cerr << caption << ": " << msg.str() << '\n';
	}
}

namespace game_events {

static bool unit_matches_filter(const unit& u, const vconfig filter,const gamemap::location& loc);
static bool matches_special_filter(const config* cfg, const vconfig filter);

game_state* get_state_of_game()
{
	return state_of_game;
}

bool internal_conditional_passed(const unit_map* units,
                        const vconfig cond, bool& backwards_compat)
{

	// If the if statement requires we have a certain unit,
	// then check for that.
	const vconfig::child_list& have_unit = cond.get_children("have_unit");
	backwards_compat = backwards_compat && have_unit.empty();
	for(vconfig::child_list::const_iterator u = have_unit.begin(); u != have_unit.end(); ++u) {

		if(units == NULL)
			return false;

		unit_map::const_iterator itor;
		for(itor = units->begin(); itor != units->end(); ++itor) {
			if(itor->second.hitpoints() > 0 && game_events::unit_matches_filter(itor, *u)) {
				break;
			}
		}

		if(itor == units->end()) {
			return false;
		}
	}

	// If the if statement requires we have a certain location,
	// then check for that.
	const vconfig::child_list& have_location = cond.get_children("have_location");
	backwards_compat = backwards_compat && have_location.empty();
	for(vconfig::child_list::const_iterator v = have_location.begin(); v != have_location.end(); ++v) {
		std::set<gamemap::location> res;
		wassert(game_map != NULL && units != NULL && status_ptr != NULL);
		get_locations(*game_map, res, *v, *status_ptr, *units);
		if(res.empty()) {
			return false;
		}
	}

	// Check against each variable statement,
	// to see if the variable matches the conditions or not.
	const vconfig::child_list& variables = cond.get_children("variable");
	backwards_compat = backwards_compat && variables.empty();
	for(vconfig::child_list::const_iterator var = variables.begin(); var != variables.end(); ++var) {
		const vconfig& values = *var;

		const std::string name = values["name"];
		wassert(state_of_game != NULL);
		const std::string& value = state_of_game->get_variable_const(name);

		const double num_value = atof(value.c_str());

		const std::string equals = values["equals"];
		if(values.get_attribute("equals") != "" && value != equals) {
			return false;
		}

		const std::string numerical_equals = values["numerical_equals"];
		if(values.get_attribute("numerical_equals") != "" && atof(numerical_equals.c_str()) != num_value){
			return false;
		}

		const std::string not_equals = values["not_equals"];
		if(values.get_attribute("not_equals") != "" && not_equals == value) {
			return false;
		}

		const std::string numerical_not_equals = values["numerical_not_equals"];
		if(values.get_attribute("numerical_not_equals") != "" && atof(numerical_not_equals.c_str()) == num_value){
			return false;
		}

		const std::string greater_than = values["greater_than"];
		if(values.get_attribute("greater_than") != "" && atof(greater_than.c_str()) >= num_value){
			return false;
		}

		const std::string less_than = values["less_than"];
		if(values.get_attribute("less_than") != "" && atof(less_than.c_str()) <= num_value){
			return false;
		}

		const std::string greater_than_equal_to = values["greater_than_equal_to"];
		if(values.get_attribute("greater_than_equal_to") != "" && atof(greater_than_equal_to.c_str()) > num_value){
			return false;
		}

		const std::string less_than_equal_to = values["less_than_equal_to"];
		if(values.get_attribute("less_than_equal_to") != "" && atof(less_than_equal_to.c_str()) < num_value) {
			return false;
		}
		const std::string boolean_equals = values["boolean_equals"];
		if(values.get_attribute("boolean_equals") != ""
		&& (utils::string_bool(value) != utils::string_bool(boolean_equals))) {
			return false;
		}
		const std::string contains = values["contains"];
		if(values.get_attribute("contains") != "" && value.find(contains) == std::string::npos) {
			return false;
		}
	}
	return true;
}

bool conditional_passed(const unit_map* units,
                        const vconfig cond, bool backwards_compat)
{
	bool allow_backwards_compat = backwards_compat = backwards_compat &&
		utils::string_bool(cond["backwards_compat"],true);
	bool matches = internal_conditional_passed(units, cond, allow_backwards_compat);

	// Handle [and], [or], and [not] with in-order precedence
	int or_count = 0;
	config::all_children_iterator cond_i = cond.get_config().ordered_begin();
	config::all_children_iterator cond_end = cond.get_config().ordered_end();
	while(cond_i != cond_end)
	{
		const std::string& cond_name = *((*cond_i).first);
		const vconfig cond_filter(&(*((*cond_i).second)));

		// Handle [and]
		if(cond_name == "and")
		{
			matches = matches && conditional_passed(units, cond_filter, backwards_compat);
			backwards_compat = false;
		}
		// Handle [or]
		else if(cond_name == "or")
		{
			matches = matches || conditional_passed(units, cond_filter, backwards_compat);
			++or_count;
		}
		// Handle [not]
		else if(cond_name == "not")
		{
			matches = matches && !conditional_passed(units, cond_filter, backwards_compat);
			backwards_compat = false;
		}
		++cond_i;
	}
	// Check for deprecated [or] syntax
	if(matches && or_count > 1 && allow_backwards_compat)
	{
		lg::wml_error << "possible deprecated [or] syntax: now forcing re-interpretation\n";
		//! @todo For now we will re-interpret it according to the old rules,
		// but this should be later to prevent re-interpretation errors.
		const vconfig::child_list& orcfgs = cond.get_children("or");
		for(unsigned int i=0; i < orcfgs.size(); ++i) {
			if(conditional_passed(units, orcfgs[i])) {
				return true;
			}
		}
		return false;
	}
	return matches;
}

} // end namespace game_events (1)

namespace {

std::set<std::string> used_items;
/*
jhinrichs, 12.11.2006:
This variable controls the maximum number of hexes in a map, that can be parsed by an event.
It was set to 1024 before and for larger maps this could become a problem. So it is raised to
65536 now, which can feature a map of size 256*256 (maps really shouldn't be bigger than that :-).
This constant also controls the maximum number of loops for a WML while loop (hence its name).
If this is felt to be too high now, we must split it into two constants,
but i don't feel the need at the moment.
*/
const size_t MaxLoop = 65536;

} // end anonymous namespace (2)

static bool events_init() { return screen != NULL; }

namespace {

struct queued_event {
	queued_event(const std::string& name, const game_events::entity_location& loc1,
	                                      const game_events::entity_location& loc2,
										  const config& data)
			: name(name), loc1(loc1), loc2(loc2),data(data) {}

	std::string name;
	game_events::entity_location loc1;
	game_events::entity_location loc2;
	config data;
};

std::deque<queued_event> events_queue;

class event_handler
{
public:
	event_handler(const config& cfg) :
		name_(cfg["name"]),
		first_time_only_(utils::string_bool(cfg["first_time_only"],true)),
		disabled_(false),rebuild_screen_(false),
		cfg_(&cfg)
	{}

	void write(config& cfg) const
	{
		if(disabled_)
			return;

		cfg = cfg_.get_config();
	}

	const std::string& name() const { return name_; }

	bool first_time_only() const { return first_time_only_; }

	void disable() { disabled_ = true; }
	bool disabled() const { return disabled_; }

	bool is_menu_item() const {
		wassert(state_of_game != NULL);
		std::map<std::string, wml_menu_item *>::iterator itor = state_of_game->wml_menu_items.begin();
		while(itor != state_of_game->wml_menu_items.end()) {
			if(&cfg_.get_config() == &itor->second->command) {
				return true;
			}
			++itor;
		}
		return false;
	}

	const vconfig::child_list first_arg_filters()
	{
		return cfg_.get_children("filter");
	}
	const vconfig::child_list first_special_filters()
	{
		return cfg_.get_children("special_filter");
	}

	const vconfig::child_list second_arg_filters()
	{
		return cfg_.get_children("filter_second");
	}
	const vconfig::child_list second_special_filters()
	{
		return cfg_.get_children("special_filter_second");
	}

	bool handle_event(const queued_event& event_info,
			const vconfig cfg = vconfig());

	bool& rebuild_screen() {return rebuild_screen_;}

private:
	bool handle_event_command(const queued_event& event_info, const std::string& cmd, const vconfig cfg, bool& mutated);

	std::string name_;
	bool first_time_only_;
	bool disabled_;
	bool rebuild_screen_;
	vconfig cfg_;
};

} // end anonymous namespace (3)

static gamemap::location cfg_to_loc(const vconfig cfg,int defaultx = 0, int defaulty = 0)
{
	int x = lexical_cast_default(cfg["x"], defaultx) - 1;
	int y = lexical_cast_default(cfg["y"], defaulty) - 1;

	return gamemap::location(x, y);
}

static std::vector<gamemap::location> multiple_locs(const vconfig cfg)
{
	return parse_location_range(cfg["x"],cfg["y"]);
}

namespace {

std::multimap<std::string,event_handler> events_map;

//! Handles all the different types of actions that can be triggered by an event.
bool event_handler::handle_event_command(const queued_event& event_info,
		const std::string& cmd, const vconfig cfg, bool& mutated)
{
	log_scope2(engine, "handle_event_command");
	LOG_NG << "handling command: '" << cmd << "'\n";

	bool rval = true;
	// Sub commands that need to be handled in a guaranteed ordering
	if(cmd == "command") {
		if(!handle_event(event_info, cfg)) {
			mutated = false;
		}
	}

	// Allow undo sets the flag saying whether the event has mutated the game to false
	else if(cmd == "allow_undo") {
		mutated = false;
	}
	// Change shroud settings for portions of the map
	else if(cmd == "remove_shroud" || cmd == "place_shroud") {
		const bool remove = cmd == "remove_shroud";

		std::string side = cfg["side"];
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side,1);
		const size_t index = side_num-1;

		if(index < teams->size()) {
			const std::vector<gamemap::location>& locs = multiple_locs(cfg);
			for(std::vector<gamemap::location>::const_iterator j = locs.begin(); j != locs.end(); ++j) {
				if(remove) {
					(*teams)[index].clear_shroud(j->x,j->y);
				} else {
					(*teams)[index].place_shroud(j->x,j->y);
				}
			}
		}

		screen->labels().recalculate_shroud();
		screen->invalidate_all();
	}


	// Teleport a unit from one location to another
	else if(cmd == "teleport") {

		unit_map::iterator u = units->find(event_info.loc1);

		// Search for a valid unit filter, and if we have one, look for the matching unit
		const vconfig filter = cfg.child("filter");
		if(!filter.null()) {
			for(u = units->begin(); u != units->end(); ++u){
				if(game_events::unit_matches_filter(u, filter))
					break;
			}
		}

		// We have found a unit that matches the filter
		if(u != units->end()) {
			const gamemap::location dst = cfg_to_loc(cfg);
			if(dst != u->first && game_map->on_board(dst)) {
				const gamemap::location vacant_dst = find_vacant_tile(*game_map,*units,dst);
				if(game_map->on_board(vacant_dst)) {
					const int side = u->second.side();

					screen->invalidate(u->first);
					std::pair<gamemap::location,unit> *up = units->extract(u->first);
					up->first = vacant_dst;
					units->add(up);
					unit_mutations++;
					if(game_map->is_village(vacant_dst)) {
						get_village(vacant_dst, *screen,*teams,side,*units);
					}

					if(utils::string_bool(cfg["clear_shroud"],true)) {
						clear_shroud(*screen,*status_ptr,*game_map,*game_data_ptr,*units,*teams,side-1);
					}

					screen->invalidate(dst);
					screen->draw();
				}
			}
		}
	}

	// Remove units from being turned to stone
	else if(cmd == "unstone") {
		const vconfig filter = cfg.child("filter");
		// Store which side will need a shroud/fog update
		std::vector<bool> clear_fog_side(teams->size(),false);

		for(unit_map::iterator i = units->begin(); i != units->end(); ++i) {
			if(utils::string_bool(i->second.get_state("stoned"))) {
				if(filter.null() || game_events::unit_matches_filter(i, filter)) {
					i->second.set_state("stoned","");
					clear_fog_side[i->second.side()-1] = true;
				}
			}
		}

		for (size_t side = 0; side != teams->size(); side++) {
			if (clear_fog_side[side] && (*teams)[side].auto_shroud_updates()) {
				recalculate_fog(*game_map,*status_ptr,*game_data_ptr,*units,*teams, side);
			}
		}
	}

	// Allow a side to recruit a new type of unit
	else if(cmd == "allow_recruit") {
		std::string side = cfg["side"];
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side,1);
		const size_t index = side_num-1;

		if(index >= teams->size())
			return rval;

		const std::string type = cfg["type"];

		const std::vector<std::string>& types = utils::split(type);
		for(std::vector<std::string>::const_iterator i = types.begin(); i != types.end(); ++i) {
			(*teams)[index].recruits().insert(*i);
			preferences::encountered_units().insert(*i);

                        player_info *player=state_of_game->get_player((*teams)[index].save_id());
                        if(player) {
                                player->can_recruit.insert(*i);
			}
		}
	}

	// Remove the ability to recruit a unit from a certain side
	else if(cmd == "disallow_recruit") {
		std::string side = cfg["side"];
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side,1);
		const size_t index = side_num-1;

		if(index >= teams->size())
			return rval;

		const std::string type = cfg["type"];
		const std::vector<std::string>& types = utils::split(type);
		for(std::vector<std::string>::const_iterator i = types.begin(); i != types.end(); ++i) {
			(*teams)[index].recruits().erase(*i);

                        player_info *player=state_of_game->get_player((*teams)[index].save_id());
                        if(player) {
                                player->can_recruit.erase(*i);
			}
		}
	}

	else if(cmd == "set_recruit") {
		std::string side = cfg["side"];
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side,1);
		const size_t index = side_num-1;

		if(index >= teams->size())
			return rval;

		std::vector<std::string> recruit = utils::split(cfg["recruit"]);
		if(recruit.size() == 1 && recruit.back() == "")
			recruit.clear();

		std::set<std::string>& can_recruit = (*teams)[index].recruits();
		can_recruit.clear();
		std::copy(recruit.begin(),recruit.end(),std::inserter(can_recruit,can_recruit.end()));

                player_info *player=state_of_game->get_player((*teams)[index].save_id());
                if(player) {
                        player->can_recruit = can_recruit;
		}
	}

	else if(cmd == "music") {
		sound::play_music_config(cfg.get_parsed_config());
	}

	else if(cmd == "sound") {
		std::string sound = cfg["name"];
		wassert(state_of_game != NULL);
		sound::play_sound(sound);
	}

	else if(cmd == "colour_adjust") {
		std::string red = cfg["red"];
		std::string green = cfg["green"];
		std::string blue = cfg["blue"];
		wassert(state_of_game != NULL);
		const int r = atoi(red.c_str());
		const int g = atoi(green.c_str());
		const int b = atoi(blue.c_str());
		screen->adjust_colours(r,g,b);
		screen->invalidate_all();
		screen->draw(true,true);
	}

	else if(cmd == "delay") {
		std::string delay_string = cfg["time"];
		wassert(state_of_game != NULL);
		const int delay_time = atoi(delay_string.c_str());
		screen->delay(delay_time);
	}

	else if(cmd == "scroll") {
		std::string x = cfg["x"];
		std::string y = cfg["y"];
		wassert(state_of_game != NULL);
		const int xoff = atoi(x.c_str());
		const int yoff = atoi(y.c_str());
		screen->scroll(xoff,yoff);
		screen->draw(true,true);
	}

	else if(cmd == "scroll_to") {
		std::string x = cfg["x"];
		std::string y = cfg["y"];
		std::string check_fogged = cfg["check_fogged"];
		wassert(state_of_game != NULL);
		const gamemap::location loc(atoi(x.c_str()), atoi(y.c_str()));
		screen->scroll_to_tile(loc,game_display::SCROLL,utils::string_bool(check_fogged,false));
	}

	else if(cmd == "scroll_to_unit") {
		unit_map::const_iterator u;
		for(u = units->begin(); u != units->end(); ++u){
			if(game_events::unit_matches_filter(u,cfg))
				break;
		}
		std::string check_fogged = cfg["check_fogged"];
		if(u != units->end()) {
			screen->scroll_to_tile(u->first,game_display::SCROLL,utils::string_bool(check_fogged,false));
		}
	}

	// An award of gold to a particular side
	else if(cmd == "gold") {
		std::string side = cfg["side"];
		std::string amount = cfg["amount"];
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side,1);
		const int amount_num = atoi(amount.c_str());
		const size_t team_index = side_num-1;
		if(team_index < teams->size()) {
			(*teams)[team_index].spend_gold(-amount_num);
		}
	}

	// Modifications of some attributes of a side: gold, income, team name
	else if(cmd == "modify_side") {
		LOG_NG << "modifying side...\n";
		std::string side = cfg["side"];
		std::string income = cfg["income"];
		std::string team_name = cfg["team_name"];
		std::string user_team_name = cfg["user_team_name"];
		std::string gold = cfg["gold"];
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side,1);
		const size_t team_index = side_num-1;

		if(team_index < teams->size()) {
			LOG_NG << "modifying team: " << side_num << "\n";
			if(!team_name.empty()) {
				LOG_NG << "change team to team_name '" << team_name << "'\n";
				(*teams)[team_index].change_team(team_name,
												 user_team_name);
			}

			if(!income.empty()) {
				(*teams)[team_index].set_income(lexical_cast_default<int>(income));
			}

			if(!gold.empty()) {
				(*teams)[team_index].spend_gold((*teams)[team_index].gold()-lexical_cast_default<int>(gold));
			}
		}
	}
	// Stores of some attributes of a side: gold, income, team name
	else if(cmd == "store_side" || cmd == "store_gold") {
		t_string *gold_store;
		std::string side = cfg["side"];
		std::string var_name = cfg["variable"];
		if(var_name.empty()) {
			var_name = cmd.substr(cmd.find_first_of('_') + 1);
		}
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side,1);
		const size_t team_index = side_num-1;
		if(team_index < teams->size()) {
			if(cmd == "store_side") {
				config side_data;
				(*teams)[team_index].write(side_data);
				state_of_game->get_variable(var_name+".controller") = side_data["controller"];
				state_of_game->get_variable(var_name+".recruit") = side_data["recruit"];
				state_of_game->get_variable(var_name+".fog") = side_data["fog"];
				state_of_game->get_variable(var_name+".shroud") = side_data["shroud"];
				
				state_of_game->get_variable(var_name+".income") = lexical_cast_default<std::string>((*teams)[team_index].income(),"");
				state_of_game->get_variable(var_name+".village_gold") = lexical_cast_default<std::string>((*teams)[team_index].village_gold(),"");
				state_of_game->get_variable(var_name+".name") = (*teams)[team_index].name();
				state_of_game->get_variable(var_name+".team_name") = (*teams)[team_index].team_name();
				state_of_game->get_variable(var_name+".user_team_name") = (*teams)[team_index].user_team_name();
				//state_of_game->get_variable(var_name+".recruit") = lexical_cast_default<std::string>((*teams)[team_index]
				state_of_game->get_variable(var_name+".colour") = (*teams)[team_index].map_colour_to();

				gold_store = &state_of_game->get_variable(var_name+".gold");
			} else {
				gold_store = &state_of_game->get_variable(var_name);
			}
			*gold_store = lexical_cast_default<std::string>((*teams)[team_index].gold(),"");
		}
	}
	else if(cmd == "modify_turns") {
		std::string value = cfg["value"];
		std::string add = cfg["add"];
		wassert(state_of_game != NULL);
		wassert(status_ptr != NULL);
		if(add != "") {
			status_ptr->modify_turns(add);
		} else {
			status_ptr->add_turns(-status_ptr->number_of_turns());
			status_ptr->add_turns(lexical_cast_default<int>(value,50));
		}
	}
	else if(cmd == "store_turns") {
		std::string var_name = cfg["variable"];
		if(var_name.empty()) {
			var_name = cmd.substr(cmd.find_first_of('_') + 1);
		}
		wassert(state_of_game != NULL);
		wassert(status_ptr != NULL);
		int turns = status_ptr->number_of_turns();
		state_of_game->get_variable(var_name) = lexical_cast_default<std::string>(turns,"");
	}

	// Moving a 'unit' - i.e. a dummy unit
	// that is just moving for the visual effect
	else if(cmd == "move_unit_fake") {
		std::string type = cfg["type"];
		std::string side = cfg["side"];
		std::string x = cfg["x"];
		std::string y = cfg["y"];
		std::string variation = cfg["variation"];
		wassert(state_of_game != NULL);

		size_t side_num = lexical_cast_default<int>(side,1)-1;
		if (side_num >= teams->size()) side_num = 0;

		const unit_race::GENDER gender = string_gender(cfg["gender"]);
		const game_data::unit_type_map::const_iterator itor = game_data_ptr->unit_types.find(type);
		if(itor != game_data_ptr->unit_types.end()) {
			wassert(game_data_ptr != NULL);
			wassert(units != NULL);
			wassert(game_map != NULL);
			wassert(status_ptr != NULL);
			unit dummy_unit(game_data_ptr,units,game_map,status_ptr,teams,&itor->second,side_num+1,false,true,gender,variation);
			const std::vector<std::string> xvals = utils::split(x);
			const std::vector<std::string> yvals = utils::split(y);
			std::vector<gamemap::location> path;
			gamemap::location src;
			gamemap::location dst;
			for(size_t i = 0; i != minimum(xvals.size(),yvals.size()); ++i) {
				if(i==0){
					src.x = atoi(xvals[i].c_str())-1;
					src.y = atoi(yvals[i].c_str())-1;
					if (!game_map->on_board(src)) {
						ERR_CF << "invalid move_unit_fake source: " << src << '\n';
						break;
					}
					continue;
				}
				shortest_path_calculator calc(dummy_unit,
						(*teams)[side_num],
						*units,
						*teams,
						*game_map);

				dst.x = atoi(xvals[i].c_str())-1;
				dst.y = atoi(yvals[i].c_str())-1;
				if (!game_map->on_board(dst)) {
					ERR_CF << "invalid move_unit_fake destination: " << dst << '\n';
					break;
				}

				paths::route route = a_star_search(src, dst, 10000, &calc,
				                                   game_map->w(), game_map->h());

				if (route.steps.size() == 0) {
					WRN_NG << "Could not find move_unit_fake route from " << src << " to " << dst << ": ignoring complexities\n";
					emergency_path_calculator calc(dummy_unit, *game_map);

					route = a_star_search(src, dst, 10000, &calc,
										  game_map->w(), game_map->h());
					wassert(route.steps.size() > 0);
				}
				unit_display::move_unit( *game_map, route.steps, dummy_unit, *teams);

				src = dst;
			}
		}
	}

	// Provide a means of specifying win/loss conditions:
	// [event]
	// name=prestart
	// [objectives]
	//   side=1
	//   summary="Escape the forest alive"
	//   victory_string="Victory:"
	//   defeat_string="Defeat:"
	//   [objective]
	//     condition=win
	//     description="Defeat all enemies"
	//   [/objective]
	//   [objective]
	//     description="Death of Konrad"
	//     condition=lose
	//   [/objective]
	// [/objectives]
	// [/event]
	//instead of the current (but still supported):
	// objectives= _ "
	// Victory:
	// @Move Konrad to the signpost in the north-west
	// Defeat:
	// #Death of Konrad
	// #Death of Delfador
	// #Turns run out"
	//
	// If side is set to 0, the new objectives are added to each player.
	//
	// The new objectives will be automatically displayed,
	// but only to the player whose objectives did change,
	// and only when it's this player's turn.
	else if(cmd == "objectives") {
		const std::string win_str = "@";
		const std::string lose_str = "#";

		wassert(state_of_game != NULL);
		const t_string summary = cfg["summary"];
		const t_string note = cfg["note"];
		std::string side = cfg["side"];
		bool silent = utils::string_bool(cfg["silent"]);
		const size_t side_num = lexical_cast_default<size_t>(side,0);

		if(side_num != 0 && (side_num - 1) >= teams->size()) {
			ERR_NG << "Invalid side: " << cfg["side"] << " in objectives event\n";
			return rval;
		}

		t_string win_string = cfg["victory_string"];
		if(win_string.empty())
			win_string = t_string(N_("Victory:"), "wesnoth");
		t_string lose_string = cfg["defeat_string"];
		if(lose_string.empty())
			lose_string = t_string(N_("Defeat:"), "wesnoth");

		t_string win_objectives;
		t_string lose_objectives;

		const vconfig::child_list objectives = cfg.get_children("objective");
		for(vconfig::child_list::const_iterator obj_it = objectives.begin();
				obj_it != objectives.end(); ++obj_it) {

			t_string description = (*obj_it)["description"];
			std::string condition = (*obj_it)["condition"];
			LOG_NG << condition << " objective: " << description << "\n";
			if(condition == "win") {
				win_objectives += "\n";
				win_objectives += win_str;
				win_objectives += description;
			} else if(condition == "lose") {
				lose_objectives += "\n";
				lose_objectives += lose_str;
				lose_objectives += description;
			} else {
				ERR_NG << "unknown condition '" << condition << "', ignoring\n";
			}
		}

		t_string objs;
		if(!summary.empty())
			objs += "*" + summary + "\n";
		if(!win_objectives.empty()) {
			objs += "*" + win_string + "\n";
			objs += win_objectives + "\n";
		}
		if(!lose_objectives.empty()) {
			objs += "*" + lose_string + "\n";
			objs += lose_objectives + "\n";
		}
		if(!note.empty())
			objs += note + "\n";

		if(side_num == 0) {
			for(std::vector<team>::iterator itor = teams->begin();
					itor != teams->end(); ++itor) {

				itor->set_objectives(objs, silent);
			}
		} else {
			(*teams)[side_num - 1].set_objectives(objs, silent);
		}
	}


	// Setting a variable
	else if(cmd == "set_variable") {
		wassert(state_of_game != NULL);

		const std::string name = cfg["name"];
		t_string& var = state_of_game->get_variable(name);

		const t_string& literal = cfg.get_attribute("literal");	// no $var substitution
		if(literal.empty() == false) {
			var = literal;
		}

		const t_string value = cfg["value"];
		if(value.empty() == false) {
			var = value;
		}

		const t_string format = cfg["format"];	// Deprecated, use value
		if(format.empty() == false) {
			var = format;
		}

		const std::string to_variable = cfg["to_variable"];
		if(to_variable.empty() == false) {
			var = state_of_game->get_variable(to_variable);
		}

		const std::string add = cfg["add"];
		if(add.empty() == false) {
			int value = int(atof(var.c_str()));
			value += atoi(add.c_str());
			char buf[50];
			snprintf(buf,sizeof(buf),"%d",value);
			var = buf;
		}

		const std::string multiply = cfg["multiply"];
		if(multiply.empty() == false) {
			int value = int(atof(var.c_str()));
			value = int(double(value) * atof(multiply.c_str()));
			char buf[50];
			snprintf(buf,sizeof(buf),"%d",value);
			var = buf;
		}

		const std::string divide = cfg["divide"];
		if(divide.empty() == false) {
			int value = int(atof(var.c_str()));
			double divider = atof(divide.c_str());
			if (divider == 0) {
				ERR_NG << "division by zero on variable " << name << "\n";
				return rval;
			} else {
				value = int(double(value) / divider);
				char buf[50];
				snprintf(buf,sizeof(buf),"%d",value);
				var = buf;
			}
		}

		const std::string modulo = cfg["modulo"];
		if(modulo.empty() == false) {
			int value = atoi(var.c_str());
			int divider = atoi(modulo.c_str());
			if (divider == 0) {
				ERR_NG << "division by zero on variable " << name << "\n";
				return rval;
			} else {
				value %= divider;
				var = str_cast(value);
			}
		}

		// Note: maybe we add more options later, eg. strftime formatting.
		// For now make the stamp mandatory.
		const std::string time = cfg["time"];
		if(time == "stamp") {
			char buf[50];
			snprintf(buf,sizeof(buf),"%d",SDL_GetTicks());
			var = buf;
		}

		// Random generation works as follows:
		// random=[comma delimited list]
		// Each element in the list will be considered a separate choice,
		// unless it contains "..". In this case, it must be a numerical
		// range (i.e. -1..-10, 0..100, -10..10, etc).
		const std::string random = cfg["random"];
		if(random.empty() == false) {
			std::string random_value;
			// If we're not replaying, create a random number
			if(get_replay_source().at_end()) {
				std::string word;
				std::vector<std::string> words;
				std::vector<std::pair<long,long> > ranges;
				int num_choices = 0;
				std::string::size_type pos = 0, pos2 = std::string::npos;
				std::stringstream ss(std::stringstream::in|std::stringstream::out);
				while (pos2 != random.length()) {
					pos = pos2+1;
					pos2 = random.find(",", pos);

					if (pos2 == std::string::npos)
						pos2 = random.length();

					word = random.substr(pos, pos2-pos);
					words.push_back(word);
					std::string::size_type tmp = word.find("..");


					if (tmp == std::string::npos) {
						// Treat this element as a string
						ranges.push_back(std::pair<int, int>(0,0));
						num_choices += 1;
					}
					else {
						// Treat as a numerical range
						const std::string first = word.substr(0, tmp);
						const std::string second = word.substr(tmp+2,
								random.length());

						long low, high;
						ss << first + " " + second;
						ss >> low;
						ss >> high;
						ss.clear();

						if (low > high) {
							tmp = low;
							low = high;
							high = tmp;
						}
						ranges.push_back(std::pair<long, long>(low,high));
						num_choices += (high - low) + 1;
					}
				}

				int choice = get_random() % num_choices;
				int tmp = 0;
				for(size_t i = 0; i < ranges.size(); i++) {
					tmp += (ranges[i].second - ranges[i].first) + 1;
					if (tmp > choice) {
						if (ranges[i].first == 0 && ranges[i].second == 0) {
							random_value = words[i];
						}
						else {
							tmp = (ranges[i].second - (tmp - choice)) + 1;
							ss << tmp;
							ss >> random_value;
						}
						break;
					}
				}

				recorder.set_random_value(random_value.c_str());
			}

			// Otherwise get the random value from the replay data
			else {
				const config* const action = get_replay_source().get_next_action();
				if(action == NULL || action->get_children("random_number").empty()) {
					replay::throw_error("choice expected but none found\n");
				}

				const std::string& val = (*(action->get_children("random_number").front()))["value"];
				random_value = val;
			}
			var = random_value;
		}
	}

	// Conditional statements
	else if(cmd == "if" || cmd == "while") {
		log_scope(cmd);
		const size_t max_iterations = (cmd == "if" ? 1 : MaxLoop);
		const std::string pass = (cmd == "if" ? "then" : "do");
		const std::string fail = (cmd == "if" ? "else" : "");
		for(size_t i = 0; i != max_iterations; ++i) {
			const std::string type = game_events::conditional_passed(
			                              units,cfg) ? pass : fail;

			if(type == "") {
				break;
			}

			// If the if statement passed, then execute all 'then' statements,
			// otherwise execute 'else' statements
			const vconfig::child_list commands = cfg.get_children(type);
			for(vconfig::child_list::const_iterator cmd = commands.begin();
			cmd != commands.end(); ++cmd) {
				if(!handle_event(event_info, *cmd)) {
					mutated = false;
				}
			}
		}
	}

	else if(cmd == "role") {

		// Get a list of the types this unit can be
		std::vector<std::string> types = utils::split(cfg["type"]);
		if (types.size() == 0) types.push_back("");

                std::vector<std::string> sides = utils::split(cfg["side"]);

		// Iterate over all the types, and for each type,
		// try to find a unit that matches
		std::vector<std::string>::iterator ti;
		for(ti = types.begin(); ti != types.end(); ++ti) {
			config item = cfg.get_config();
			item["type"] = *ti;
			item["role"] = "";
			vconfig filter(&item);

			unit_map::iterator itor;
			for(itor = units->begin(); itor != units->end(); ++itor) {
				if(game_events::unit_matches_filter(itor, filter)) {
					itor->second.assign_role(cfg["role"]);
					break;
				}
			}

			if(itor != units->end())
				break;

                        bool found = false;

			if(sides.empty() == false) {
				std::vector<std::string>::const_iterator si;
				for(si = sides.begin(); si != sides.end(); ++si) {
					int side_num = lexical_cast_default<int>(*si,1);
					const std::string player_id = (*teams)[side_num-1].save_id();
					player_info* player=state_of_game->get_player(player_id);

					if(!player)
						continue;

					// Iterate over the units, and try to find one that matches
					std::vector<unit>::iterator ui;
					for(ui = player->available_units.begin();
							ui != player->available_units.end(); ++ui) {
						wassert(game_data_ptr != NULL);
						ui->set_game_context(game_data_ptr,units,game_map,status_ptr,teams);
						scoped_recall_unit auto_store("this_unit", player_id,
							(ui - player->available_units.begin()));
						if(game_events::unit_matches_filter(*ui, filter,gamemap::location())) {
							ui->assign_role(cfg["role"]);
							found=true;
							break;
						}
					}
				}
			} else {
				std::map<std::string, player_info>::iterator pi;
				for(pi=state_of_game->players.begin();
						pi!=state_of_game->players.end(); ++pi) {
					std::vector<unit>::iterator ui;
					// Iterate over the units, and try to find one that matches
					for(ui = pi->second.available_units.begin();
							ui != pi->second.available_units.end(); ++ui) {
						wassert(game_data_ptr != NULL);
						ui->set_game_context(game_data_ptr,units,game_map,status_ptr,teams);
						scoped_recall_unit auto_store("this_unit", pi->first,
							(ui - pi->second.available_units.begin()));
						if(game_events::unit_matches_filter(*ui, filter,gamemap::location())) {
							ui->assign_role(cfg["role"]);
							found=true;
							break;
						}
					}
				}
			}

			// Stop searching if we found a unit:
                        if(found)
				break;
		}
	}

	else if(cmd == "removeitem") {
		gamemap::location loc = cfg_to_loc(cfg);
		if(!loc.valid()) {
			loc = event_info.loc1;
		}

		screen->remove_overlay(loc);
	}

	else if(cmd == "unit_overlay") {
		std::string img = cfg["image"];
		wassert(state_of_game != NULL);
		for(unit_map::iterator itor = units->begin(); itor != units->end(); ++itor) {
			if(game_events::unit_matches_filter(itor,cfg)) {
				itor->second.add_overlay(img);
				break;
			}
		}
	}

	else if(cmd == "remove_unit_overlay") {
		std::string img = cfg["image"];
		wassert(state_of_game != NULL);
		for(unit_map::iterator itor = units->begin(); itor != units->end(); ++itor) {
			if(game_events::unit_matches_filter(itor,cfg)) {
				itor->second.remove_overlay(img);
				break;
			}
		}
	}

	// Hiding units
	else if(cmd == "hide_unit") {
		const gamemap::location loc = cfg_to_loc(cfg);
		unit_map::iterator u = units->find(loc);
		if(u != units->end()) {
			u->second.set_hidden(true);
			screen->invalidate(loc);
			screen->draw();
		}
	}

	else if(cmd == "unhide_unit") {
		const gamemap::location loc = cfg_to_loc(cfg);
		unit_map::iterator u;
		// Unhide all for backward compatibility
		for(u =  units->begin(); u != units->end() ; u++) {
			u->second.set_hidden(false);
			screen->invalidate(loc);
			screen->draw();
		}
	}

	// Adding new items
	else if(cmd == "item") {
		gamemap::location loc = cfg_to_loc(cfg);
		std::string img = cfg["image"];
		std::string halo = cfg["halo"];
		wassert(state_of_game != NULL);
		if(!img.empty() || !halo.empty()) {
			screen->add_overlay(loc,img,halo);
			screen->invalidate(loc);
			screen->draw();
		}
	}

	else if(cmd == "sound_source") {
		std::string sounds = cfg["sounds"];
		std::string id = cfg["id"];
		std::string delay = cfg["delay"];
		std::string chance = cfg["chance"];
		std::string play_fogged = cfg["check_fogged"];
		std::string x = cfg["x"];
		std::string y = cfg["y"];

		wassert(state_of_game != NULL);

		if(!sounds.empty() && !delay.empty() && !chance.empty()) {
			const std::vector<std::string>& vx = utils::split(x);
			const std::vector<std::string>& vy = utils::split(y);

			if(play_fogged.empty())
				soundsources->add(id, sounds, lexical_cast<int>(delay), lexical_cast<int>(chance));
			else
				soundsources->add(id, sounds, lexical_cast<int>(delay),
						lexical_cast<int>(chance), utils::string_bool(play_fogged));

			for(unsigned int i = 0; i < minimum(vx.size(), vy.size()); ++i) {
				gamemap::location loc(lexical_cast<int>(vx[i]), lexical_cast<int>(vy[i]));
				soundsources->add_location(id, loc);
			}
		}
	}

	else if(cmd == "remove_sound_source") {
		soundsources->remove(cfg["id"]);
	}

	// Changing the terrain
	else if(cmd == "terrain") {
		const std::vector<gamemap::location> locs = multiple_locs(cfg);

		std::string terrain_type = cfg["letter"];
		wassert(state_of_game != NULL);

		t_translation::t_letter terrain = t_translation::read_letter(terrain_type);

		if(terrain != t_translation::NONE_TERRAIN) {

			for(std::vector<gamemap::location>::const_iterator loc = locs.begin(); loc != locs.end(); ++loc) {
				preferences::encountered_terrains().insert(terrain);
				const bool old_village = game_map->is_village(*loc);
				const bool new_village = game_map->is_village(terrain);

				if(old_village && !new_village) {
					int owner = village_owner(*loc, *teams);
					if(owner != -1) {
						(*teams)[owner].lose_village(*loc);
					}
				}

				game_map->set_terrain(*loc,terrain);
			}
			rebuild_screen_ = true;
		}
	}

	// Creating a mask of the terrain
	else if(cmd == "terrain_mask") {
		gamemap::location loc = cfg_to_loc(cfg, 1, 1);

		gamemap mask(*game_map);

		try {
			mask.read(cfg["mask"], gamemap::NO_BORDER, gamemap::IS_MASK);
		} catch(gamemap::incorrect_format_exception&) {
			ERR_NG << "terrain mask is in the incorrect format, and couldn't be applied\n";
			return rval;
		}

		game_map->overlay(mask, cfg.get_parsed_config(), loc.x, loc.y);
		rebuild_screen_ = true;
	}

	// If we should spawn a new unit on the map somewhere
	else if(cmd == "unit") {
		wassert(game_data_ptr != NULL);
		wassert(units != NULL);
		wassert(game_map != NULL);
		wassert(status_ptr != NULL);
		unit new_unit(game_data_ptr,units,game_map,status_ptr,teams,cfg.get_parsed_config(),true);
		preferences::encountered_units().insert(new_unit.id());
		gamemap::location loc = cfg_to_loc(cfg);

		if(game_map->on_board(loc)) {
			loc = find_vacant_tile(*game_map,*units,loc);
			const bool show = screen != NULL && !screen->fogged(loc);
			const bool animate = show && cfg["animate"] != "";

			units->erase(loc);
			units->add(new std::pair<gamemap::location,unit>(loc,new_unit));
			unit_mutations++;
			if(game_map->is_village(loc)) {
				get_village(loc,*screen,*teams,new_unit.side()-1,*units);
			}

			screen->invalidate(loc);

			unit_map::iterator un = units->find(loc);

			if(animate) {
				unit_display::unit_recruited(loc);
			}
			else if(show) {
				screen->draw();
			}
		} else {
			player_info* const player = state_of_game->get_player((*teams)[new_unit.side()-1].save_id());

			if(player != NULL) {
				player->available_units.push_back(new_unit);
			} else {
				ERR_NG << "Cannot create unit: location is not on the map, and player "
					<< new_unit.side() << " has no recall list.\n";
			}
		}
	}

	// If we should recall units that match a certain description
	else if(cmd == "recall") {
		LOG_NG << "recalling unit...\n";
		bool unit_recalled = false;
		config unit_filter(cfg.get_config());
		// Prevent the recall unit filter from using the location as a criterion
		//! @todo FIXME: we should design the WML to avoid these types of collisions;
		// filters should be named consistently and always have a distinct scope.
		unit_filter["x"] = "";
		unit_filter["y"] = "";
		for(int index = 0; !unit_recalled && index < int(teams->size()); ++index) {
			LOG_NG << "for side " << index << "...\n";
			const std::string player_id = (*teams)[index].save_id();
			player_info* const player = state_of_game->get_player(player_id);

			if(player == NULL) {
				ERR_NG << "player not found!\n";
				continue;
			}

			std::vector<unit>& avail = player->available_units;

			for(std::vector<unit>::iterator u = avail.begin(); u != avail.end(); ++u) {
				LOG_NG << "checking unit against filter...\n";
				wassert(game_data_ptr != NULL);
				u->set_game_context(game_data_ptr,units,game_map,status_ptr,teams);
				scoped_recall_unit auto_store("this_unit", player_id, u - avail.begin());
				if(game_events::unit_matches_filter(*u, &unit_filter, gamemap::location())) {
					gamemap::location loc = cfg_to_loc(cfg);
					unit to_recruit(*u);
					avail.erase(u);	// Erase before recruiting, since recruiting can fire more events
					unit_mutations++;
					recruit_unit(*game_map,index+1,*units,to_recruit,loc,utils::string_bool(cfg["show"],true),false,true);
					unit_recalled = true;
					break;
				}
			}
		}
	} else if(cmd == "object") {
		const vconfig filter = cfg.child("filter");

		std::string id = cfg["id"];
		wassert(state_of_game != NULL);

		// If this item has already been used
		if(id != "" && used_items.count(id))
			return rval;

		std::string image = cfg["image"];
		std::string caption = cfg["name"];
		std::string text;

		gamemap::location loc;
		if(!filter.null()) {
			for(unit_map::const_iterator u = units->begin(); u != units->end(); ++u) {
				if(game_events::unit_matches_filter(u, filter)) {
					loc = u->first;
					break;
				}
			}
		}

		if(loc.valid() == false) {
			loc = event_info.loc1;
		}

		const unit_map::iterator u = units->find(loc);

		std::string command_type = "then";

		if(u != units->end() && (filter.null() || game_events::unit_matches_filter(u, filter))) {
			text = cfg["description"];

			u->second.add_modification("object", cfg.get_parsed_config());

			screen->select_hex(event_info.loc1);
			screen->invalidate_unit();

			// Mark this item as used up.
			used_items.insert(id);
		} else {
			text = cfg["cannot_use_message"];
			command_type = "else";
		}

		if(!utils::string_bool(cfg["silent"])) {
			surface surface(NULL);

			if(image.empty() == false) {
				surface.assign(image::get_image(image));
			}

			// Redraw the unit, with its new stats
			screen->draw();

			const std::string duration_str = cfg["duration"];
			const unsigned int lifetime = average_frame_time * lexical_cast_default<unsigned int>(duration_str, prevent_misclick_duration);
			wml_event_dialog to_show(*screen,((surface.null())? caption : ""),text);
			if(!surface.null()) {
				to_show.set_image(surface, caption);
			}
			to_show.layout();
			to_show.show(lifetime);
		}

		const vconfig::child_list commands = cfg.get_children(command_type);
		for(vconfig::child_list::const_iterator cmd = commands.begin();
		cmd != commands.end(); ++cmd) {
			if(!handle_event(event_info, *cmd)) {
				mutated = false;
			}
		}
	}

	// Display a message on-screen
	else if(cmd == "print") {
		std::string text = cfg["text"];
		std::string size_str = cfg["size"];
		std::string duration_str = cfg["duration"];
		std::string red_str = cfg["red"];
		std::string green_str = cfg["green"];
		std::string blue_str = cfg["blue"];

		wassert(state_of_game != NULL);
		const int size = lexical_cast_default<int>(size_str,font::SIZE_SMALL);
		const int lifetime = lexical_cast_default<int>(duration_str,50);
		const int red = lexical_cast_default<int>(red_str,0);
		const int green = lexical_cast_default<int>(green_str,0);
		const int blue = lexical_cast_default<int>(blue_str,0);

		SDL_Color colour = {red,green,blue,255};

		// Remove any old message.
		if (floating_label)
			font::remove_floating_label(floating_label);

		const std::string& msg = text;
		if(msg != "") {
			const SDL_Rect rect = screen->map_outside_area();
			floating_label = font::add_floating_label(msg,size,colour,
									 rect.w/2,rect.h/2,
			                         0.0,0.0,lifetime,rect,font::CENTER_ALIGN);
		}
	}

	else if(cmd == "deprecated_message") {
		// Note: no need to translate the string, since only used for deprecated things.
		const std::string message = cfg["message"];
		lg::wml_error << message << '\n';
	}
	
	else if(cmd == "debug_message") {
		const std::string log_level = cfg["logger"];
		const std::string log_message = cfg["message"];
		if (log_level == "err")
			ERR_NO << log_message << "'\n";
		else if (log_level == "warn" || log_level == "wrn")
			WRN_NO << log_message << "'\n";
		else
			LOG_NO << log_message << "'\n";
	}

	// Display a message dialog
	else if(cmd == "message") {
		unit_map::iterator speaker = units->end();

		std::string speaker_str = cfg["speaker"];
		wassert(state_of_game != NULL);
		if(speaker_str == "unit") {
			speaker = units->find(event_info.loc1);
		} else if(speaker_str == "second_unit") {
			speaker = units->find(event_info.loc2);
		} else if(speaker_str != "narrator") {
			for(speaker = units->begin(); speaker != units->end(); ++speaker){
				if(game_events::unit_matches_filter(speaker,cfg))
					break;
			}
		}

		if(speaker == units->end() && speaker_str != "narrator") {
			// No matching unit found, so the dialog can't come up.
			// Continue onto the next message.
			WRN_NG << "cannot show message\n";
			return rval;
		}

		if(speaker != units->end()) {
			LOG_NG << "set speaker to '" << speaker->second.description() << "'\n";
		} else {
			LOG_NG << "no speaker\n";
		}

		std::string sfx = cfg["sound"];
		if(sfx != "") {
			sound::play_sound(sfx);
		}

		std::string image = cfg["image"];
		std::string caption = cfg["caption"];

		if(speaker != units->end()) {
			LOG_DP << "scrolling to speaker..\n";
			screen->highlight_hex(speaker->first);
			const int offset_from_center = maximum<int>(0, speaker->first.y - 1);
			screen->scroll_to_tile(gamemap::location(speaker->first.x,offset_from_center));

			if(image.empty()) {
				image = speaker->second.profile();
				if(image == speaker->second.absolute_image()) {
					std::stringstream ss;

#ifdef LOW_MEM
					ss	<< image;
#else
					ss	<< image << speaker->second.image_mods();
#endif

					image = ss.str();
				}
			}

			if(caption.empty()) {
				caption = speaker->second.description();
				if(caption.empty()) {
					caption = speaker->second.language_name();
				}
			}
			LOG_DP << "done scrolling to speaker...\n";
		}

		std::vector<std::string> options;
		std::vector<vconfig::child_list> option_events;

		const vconfig::child_list menu_items = cfg.get_children("option");
		for(vconfig::child_list::const_iterator mi = menu_items.begin();
				mi != menu_items.end(); ++mi) {
			std::string msg_str = (*mi)["message"];
			if(!(*mi).has_child("show_if")
				|| game_events::conditional_passed(units,(*mi).child("show_if"))) {
				options.push_back(msg_str);
				option_events.push_back((*mi).get_children("command"));
            }
		}

		surface surface(NULL);
		if(image.empty() == false) {
			surface.assign(image::get_image(image));
		}

		int option_chosen = -1;

		LOG_DP << "showing dialog...\n";

		// If we're not replaying, or if we are replaying
		// and there is no choice to be made, show the dialog.
		if(get_replay_source().at_end() || options.empty()) {
			const t_string msg = cfg["message"];
			const std::string duration_str = cfg["duration"];
			const unsigned int lifetime = average_frame_time * lexical_cast_default<unsigned int>(duration_str, prevent_misclick_duration);
			const SDL_Rect& map_area = screen->map_outside_area();

			wml_event_dialog to_show(*screen, ((surface.null())? caption : ""),
				msg, ((options.empty())? gui::MESSAGE : gui::OK_ONLY));
			if(!surface.null()) {
				to_show.set_image(surface, caption);
			}
			if(!options.empty()) {
				to_show.set_menu(options);
			}
			gui::dialog::dimension_measurements dim = to_show.layout();
			to_show.get_menu().set_width( dim.menu_width );
			to_show.get_menu().set_max_width( dim.menu_width );
			to_show.get_menu().wrap_words();
			static const int dialog_top_offset = 26;
			to_show.layout(-1, map_area.y + dialog_top_offset);
			option_chosen = to_show.show(lifetime);
			LOG_DP << "showed dialog...\n";

			if (option_chosen == gui::ESCAPE_DIALOG){
				rval = false;
			}

			if(options.empty() == false) {
				recorder.choose_option(option_chosen);
			}
		}

		// Otherwise if a choice has to be made, get it from the replay data
		else {
			const config* action = get_replay_source().get_next_action();
			if (action != NULL && !action->get_children("start").empty()){
				action = get_replay_source().get_next_action();
			}
			if(action == NULL || action->get_children("choose").empty()) {
				replay::throw_error("choice expected but none found\n");
			}

			const std::string& val = (*(action->get_children("choose").front()))["value"];
			option_chosen = atol(val.c_str());
		}

		// Implement the consequences of the choice
		if(options.empty() == false) {
			if(size_t(option_chosen) >= menu_items.size()) {
				std::stringstream errbuf;
				errbuf << "invalid choice (" << option_chosen
				       << ") was specified, choice 0 to " << (menu_items.size() - 1)
				       << " was expected.\n";
				replay::throw_error(errbuf.str());
			}

			vconfig::child_list events = option_events[option_chosen];
			for(vconfig::child_list::const_iterator itor = events.begin();
			itor != events.end(); ++itor) {
				if(!handle_event(event_info, *itor)) {
					mutated = false;
				}
			}
		}
	}

	else if(cmd == "kill") {
		// Use (x,y) iteration, because firing events ruins unit_map iteration
		for(gamemap::location loc(0,0); loc.x < game_map->w(); ++loc.x) {
			for(loc.y = 0; loc.y < game_map->h(); ++loc.y) {
				unit_map::iterator un = units->find(loc);
				if(un != units->end() && game_events::unit_matches_filter(un,cfg)) {
					if(utils::string_bool(cfg["animate"])) {
						screen->scroll_to_tile(loc);
						unit_display::unit_die(loc, un->second);
					}
					if(utils::string_bool(cfg["fire_event"])) {
						game_events::entity_location death_loc(un);
						game_events::fire("die", death_loc, death_loc);
						un = units->find(death_loc);
						if(un != units->end() && death_loc.matches_unit(un->second)) {
							units->erase(un);
							unit_mutations++;
						}
					} else {
						units->erase(un);
						unit_mutations++;
					}
				}
			}
		}

		// If the filter doesn't contain positional information,
		// then it may match units on all recall lists.
		if(cfg["x"].empty() && cfg["y"].empty()) {
                  std::map<std::string, player_info>& players=state_of_game->players;

                  for(std::map<std::string, player_info>::iterator pi = players.begin();
                      pi!=players.end(); ++pi) {
                        std::vector<unit>& avail_units = pi->second.available_units;
			for(std::vector<unit>::iterator j = avail_units.begin(); j != avail_units.end();) {
				wassert(game_data_ptr != NULL);
				j->set_game_context(game_data_ptr,units,game_map,status_ptr,teams);
				scoped_recall_unit auto_store("this_unit", pi->first, j - avail_units.begin());
				if(game_events::unit_matches_filter(*j, cfg,gamemap::location())) {
                            j = avail_units.erase(j);
                          } else {
                            ++j;
                          }
			}
                  }
		}
	}

	// Adding of new events
	else if(cmd == "event") {
		new_handlers.push_back(event_handler(cfg.get_config()));
	}

	// Setting of menu items
	else if(cmd == "set_menu_item") {
		/*
		[set_menu_item]
			id=test1
			image="buttons/group_all.png"
			description="Summon Troll"
			[show_if]
				[not]
					[have_unit]
						x,y=$x1,$y1
					[/have_unit]
				[/not]
			[/show_if]
			[filter_location]
			[/filter_location]
			[command]
				{UNIT (Troll) (Myname) ( _ "Myname") $side_number $x1 $y1}
			[/command]
		[/set_menu_item]
		*/
		std::string id = cfg["id"];
		wml_menu_item*& mref = state_of_game->wml_menu_items[id];
		if(mref == NULL) {
			mref = new wml_menu_item(id);
		}
		if(cfg.has_attribute("image")) {
			mref->image = cfg["image"];
		}
		if(cfg.has_attribute("description")) {
			mref->description = cfg["description"];
		}
		if(cfg.has_attribute("needs_select")) {
			mref->needs_select = utils::string_bool(cfg["needs_select"], false);
		}
		if(cfg.has_child("show_if")) {
			mref->show_if = cfg.child("show_if").get_config();
		}
		if(cfg.has_child("filter_location")) {
			mref->filter_location = cfg.child("filter_location").get_config();
		}
		if(cfg.has_child("command")) {
			config* new_command = new config(cfg.child("command").get_config());
			wmi_command_changes.push_back(wmi_command_change(id, new_command));
		}
	}
	// Unit serialization to and from variables
	//! @todo FIXME: Check that store is automove bug safe
	else if(cmd == "store_unit") {
		const config empty_filter;
		vconfig filter = cfg.child("filter");
		if(filter.null())
			filter = &empty_filter;

		std::string variable = cfg["variable"];
		if(variable.empty()) {
			variable="unit";
		}
		const std::string mode = cfg["mode"];
		config to_store;
		variable_info varinfo(variable, true, variable_info::TYPE_ARRAY);

		const bool kill_units = utils::string_bool(cfg["kill"]);

		for(unit_map::iterator i = units->begin(); i != units->end();) {
			if(game_events::unit_matches_filter(i,filter) == false) {
				++i;
				continue;
			}

			config& data = to_store.add_child(varinfo.key);
			i->first.write(data);
			i->second.write(data);

			if(kill_units) {
				units->erase(i++);
				unit_mutations++;
			} else {
				++i;
			}
		}

		if(filter["x"].empty() && filter["y"].empty()) {
			std::map<std::string, player_info>& players = state_of_game->players;

			for(std::map<std::string, player_info>::iterator pi = players.begin();
					pi!=players.end(); ++pi) {
				std::vector<unit>& avail_units = pi->second.available_units;
				for(std::vector<unit>::iterator j = avail_units.begin(); j != avail_units.end();) {
				wassert(game_data_ptr != NULL);
				j->set_game_context(game_data_ptr,units,game_map,status_ptr,teams);
				scoped_recall_unit auto_store("this_unit", pi->first, j - avail_units.begin());
				if(game_events::unit_matches_filter(*j, filter,gamemap::location()) == false) {
						++j;
						continue;
					}
					config& data = to_store.add_child(varinfo.key);
					j->write(data);
					data["x"] = "recall";
					data["y"] = "recall";

					if(kill_units) {
						j = avail_units.erase(j);
					} else {
						++j;
					}
				}
			}
		}
		if(mode != "append") {
			varinfo.vars->clear_children(varinfo.key);
		}
		varinfo.vars->append(to_store);
	}

	else if(cmd == "unstore_unit") {
		wassert(state_of_game != NULL);
		const config& var = state_of_game->get_variable_cfg(cfg["variable"]);

		try {
			wassert(game_data_ptr != NULL);
			wassert(units != NULL);
			wassert(game_map != NULL);
			wassert(status_ptr != NULL);
			const unit u(game_data_ptr,units,game_map,status_ptr,teams,var, false);

			preferences::encountered_units().insert(u.id());
			gamemap::location loc(var, game_events::get_state_of_game());
			if(loc.valid()) {
				if(utils::string_bool(cfg["find_vacant"])) {
					loc = find_vacant_tile(*game_map,*units,loc);
				}

				units->erase(loc);
				units->add(new std::pair<gamemap::location,unit>(loc,u));
				unit_mutations++;

				std::string text = cfg["text"];
				if(!text.empty())
				{
					// Print floating label
					std::string red_str = cfg["red"];
					std::string green_str = cfg["green"];
					std::string blue_str = cfg["blue"];
					const int red = lexical_cast_default<int>(red_str,0);
					const int green = lexical_cast_default<int>(green_str,0);
					const int blue = lexical_cast_default<int>(blue_str,0);
					{
						screen->float_label(loc,text,red,green,blue);
					}
				}

				if(utils::string_bool(cfg["advance"], true) && get_replay_source().at_end()) {
					// Try to advance the unit

					//! @todo FIXME: get player_number_ from the play_controller, not from the WML vars.
					const t_string& side_str = state_of_game->get_variable("side_number");
					const int side = lexical_cast_default<int>(side_str.base_str(), -1);

					// Select advancement if it is on the playing side and the player is a human
					const bool sel = (side == static_cast<int>(u.side())
					                 && (*teams)[side-1].is_human());

					// The code in dialogs::advance_unit tests whether the unit can advance
					dialogs::advance_unit(*game_data_ptr, *game_map, *units, loc, *screen, !sel, true);
				}

			} else {
				player_info *player=state_of_game->get_player((*teams)[u.side()-1].save_id());

				if(player) {
					player->available_units.push_back(u);
				} else {
					ERR_NG << "Cannot unstore unit: no recall list for player " << u.side()
						<< " and the map location is invalid.\n";
				}
			}
		} catch(game::load_game_failed& e) {
			ERR_NG << "could not de-serialize unit: '" << e.message << "'\n";
		}
	}

	else if(cmd == "store_starting_location") {
		std::string side = cfg["side"];
		std::string variable = cfg["variable"];
		if (variable.empty()) {
			variable="location";
		}
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side,1);

		const gamemap::location& loc = game_map->starting_position(side_num);
		wassert(state_of_game != NULL);
		config &loc_store = state_of_game->get_variable_cfg(variable);
		loc_store.clear();
		loc.write(loc_store);
		game_map->write_terrain(loc, loc_store);
	}

	/* [store_villages] : store villages into an array
	 * Keys:
	 * - variable (mandatory): variable to store in
	 * - side: if present, the village should be owned by this side (0=unowned villages)
	 * - terrain: if present, filter the village types against this list of terrain types
	 */
	else if(cmd == "store_villages" ) {
		log_scope("store_villages");
		std::string variable = cfg["variable"];
		if (variable.empty()) {
			variable="location";
		}
		config to_store;
		variable_info varinfo(variable, true, variable_info::TYPE_ARRAY);

		std::vector<gamemap::location> locs = game_map->villages();

		for(std::vector<gamemap::location>::const_iterator j = locs.begin(); j != locs.end(); ++j) {
			bool matches = false;
			if(cfg.has_attribute("side")) { 	//! @deprecated, use owner_side instead
				lg::wml_error << "side key is no longer accepted in [store_villages],"
					<< " use owner_side instead.\n";
				config temp_cfg(cfg.get_config());
				temp_cfg["owner_side"] = temp_cfg["side"];
				temp_cfg["side"] = "";
				matches = terrain_matches_filter(*game_map, *j, &temp_cfg, *status_ptr, *units);
			} else {
				matches = terrain_matches_filter(*game_map, *j, cfg, *status_ptr, *units);
			}
			if(matches) {
				config &loc_store = to_store.add_child(varinfo.key);
				j->write(loc_store);
				game_map->write_terrain(*j, loc_store);
			}
		}
		varinfo.vars->clear_children(varinfo.key);
		varinfo.vars->append(to_store);
	}

	else if(cmd == "store_locations" ) {
		log_scope("store_locations");
		std::string variable = cfg["variable"];
		if (variable.empty()) {
			variable="location";
		}

		std::set<gamemap::location> res;
		get_locations(*game_map, res, cfg, *status_ptr, *units, false, MaxLoop);

		state_of_game->clear_variable_cfg(variable);
		for(std::set<gamemap::location>::const_iterator j = res.begin(); j != res.end(); ++j) {
			config &loc_store = state_of_game->add_variable_cfg(variable);
			j->write(loc_store);
			game_map->write_terrain(*j, loc_store);
		}
	}

	// Command to take control of a village for a certain side
	else if(cmd == "capture_village") {
		std::string side = cfg["side"];
		wassert(state_of_game != NULL);
		const int side_num = lexical_cast_default<int>(side);
		// If 'side' is 0, then it will become an invalid index,
		// and so the village will become neutral.
		const size_t team_num = size_t(side_num-1);

		const std::vector<gamemap::location> locs(multiple_locs(cfg));

		for(std::vector<gamemap::location>::const_iterator i = locs.begin(); i != locs.end(); ++i) {
			if(game_map->is_village(*i)) {
				get_village(*i,*screen,*teams,team_num,*units);
			}
		}
	}

	// Command to remove a variable
	else if(cmd == "clear_variable") {
		const std::string name = cfg["name"];
		state_of_game->clear_variable(name);
	}

	else if(cmd == "endlevel") {
		const std::string next_scenario = cfg["next_scenario"];
		if(next_scenario.empty() == false) {
			state_of_game->scenario = next_scenario;
		}

		const std::string result = cfg["result"].base_str(); //do not translate
		if(result.empty() || result == "victory") {
			const bool bonus = utils::string_bool(cfg["bonus"],true);
			throw end_level_exception(VICTORY,bonus);
		} else if(result == "continue") {
			throw end_level_exception(LEVEL_CONTINUE);
		} else if(result == "continue_no_save") {
			throw end_level_exception(LEVEL_CONTINUE_NO_SAVE);
		} else {
			LOG_NG << "throwing event defeat...\n";
			throw end_level_exception(DEFEAT);
		}
	}

	else if(cmd == "redraw") {
		std::string side = cfg["side"];
		wassert(state_of_game != NULL);
		if(side != "") {
			const int side_num = lexical_cast_default<int>(side);
			recalculate_fog(*game_map,*status_ptr,*game_data_ptr,*units,*teams,side_num-1);
			screen->recalculate_minimap();
		}
		if(rebuild_screen_) {
			rebuild_screen_ = false;
			screen->recalculate_minimap();
			screen->rebuild_all();
		}
		screen->invalidate_all();
		screen->draw(true,true);
	}

	else if(cmd == "animate_unit") {

		unit_map::iterator u = units->find(event_info.loc1);

		// Search for a valid unit filter,
		// and if we have one, look for the matching unit
		const vconfig filter = cfg.child("filter");
		if(!filter.null()) {
			for(u = units->begin(); u != units->end(); ++u){
				if(game_events::unit_matches_filter(u, filter))
					break;
			}
		}

		// We have found a unit that matches the filter
		if(u != units->end() && ! screen->fogged(u->first)) {
			screen->highlight_hex(u->first);
			screen->scroll_to_tile(u->first);

			u->second.set_extra_anim(*screen,u->first,cfg["flag"]);
			while(!u->second.get_animation()->animation_would_finish()) {
				screen->invalidate(u->first);
				screen->draw();
				events::pump();
				screen->delay(10);
			}
			u->second.set_standing(*screen,u->first);
			screen->invalidate(u->first);
			screen->draw();
			events::pump();
		}
	} else if(cmd == "label") {

		terrain_label label(screen->labels(),
				    cfg.get_config(),
				    game_events::get_state_of_game());

		screen->labels().set_label(label.location(),
					   label.text(),
					   label.team_name(),
					   label.colour());
	}

	LOG_NG << "done handling command...\n";

	return rval;
}

static void commit_new_handlers() {
	// Commit any spawned events-within-events
	while(new_handlers.size() > 0) {
		event_handler& new_handler = new_handlers.back();
		events_map.insert(std::pair<std::string,event_handler>(new_handler.name(),new_handler));
		LOG_NG << "spawning new handler for event " << new_handler.name() << "\n";
		//new_handler.cfg_->debug(lg::info(lg::engine));
		new_handlers.pop_back();
	}
}

static void commit_wmi_commands() {
	// Commit WML Menu Item command changes
	while(wmi_command_changes.size() > 0) {
		wmi_command_change wcc = wmi_command_changes.back();
		wml_menu_item*& mref = state_of_game->wml_menu_items[wcc.first];
		const bool no_current_handler = mref->command.empty();
		mref->command = *(wcc.second);
		if(no_current_handler) {
			if(!mref->command.empty()) {
				mref->command["name"] = mref->name;
				mref->command["first_time_only"] = "no";
				event_handler new_handler(mref->command);
				events_map.insert(std::pair<std::string,event_handler>(
					new_handler.name(), new_handler));
			}
		} else if(mref->command.empty()) {
			mref->command["name"] = mref->name;
			mref->command["first_time_only"] = "no";
			mref->command.add_child("allow_undo");
		}
		LOG_NG << "setting command for " << mref->name << "\n";
		LOG_NG << wcc.second->debug();
		delete wcc.second;
		wmi_command_changes.pop_back();
	}
}

bool event_handler::handle_event(const queued_event& event_info, const vconfig conf)
{
	bool mutated = true;
	bool skip_messages = false;

	vconfig cfg = conf;
	if(cfg.null()) {
		cfg = cfg_;
	}
	for(config::all_children_iterator i = cfg.get_config().ordered_begin();
			i != cfg.get_config().ordered_end(); ++i) {

		const std::pair<const std::string*,const config*> item = *i;

		// If the user pressed escape, we skip any message
		// that doesn't require them to make a choice.
		if ((skip_messages) && (*item.first == "message")) {
			if ((item.second)->get_children("option").size() == 0) {
				continue;
			}
		}

		if (!handle_event_command(event_info, *item.first, vconfig(item.second), mutated)) {
			skip_messages = true;
		}
		else {
			skip_messages = false;
		}
	}

	// We do this once the event has completed any music alterations
	sound::commit_music_changes();

	return mutated;
}

} // end anonymous namespace (4)

static bool process_event(event_handler& handler, const queued_event& ev)
{
	if(handler.disabled())
		return false;

	unit_map::iterator unit1 = units->find(ev.loc1);
	unit_map::iterator unit2 = units->find(ev.loc2);
	bool filtered_unit1 = false, filtered_unit2 = false;
	scoped_xy_unit first_unit("unit", ev.loc1.x, ev.loc1.y, *units);
	scoped_xy_unit second_unit("second_unit", ev.loc2.x, ev.loc2.y, *units);

	const vconfig::child_list first_filters = handler.first_arg_filters();
	vconfig::child_list::const_iterator ffi;
	for(ffi = first_filters.begin();
			ffi != first_filters.end(); ++ffi) {

		if(unit1 == units->end() || !game_events::unit_matches_filter(unit1,*ffi)) {
			return false;
		}
		if(!ffi->empty()) {
			filtered_unit1 = true;
		}
	}
	bool special_matches = false;
	const vconfig::child_list first_special_filters = handler.first_special_filters();
	special_matches = first_special_filters.size() ? false : true;
	for(ffi = first_special_filters.begin();
			ffi != first_special_filters.end(); ++ffi) {

		if(unit1 != units->end() && game_events::matches_special_filter(ev.data.child("first"),*ffi)) {
			special_matches = true;
		}
		if(!ffi->empty()) {
			filtered_unit1 = true;
		}
	}
	if(!special_matches) {
		return false;
	}

	const vconfig::child_list second_filters = handler.second_arg_filters();
	for(vconfig::child_list::const_iterator sfi = second_filters.begin();
			sfi != second_filters.end(); ++sfi) {
		if(unit2 == units->end() || !game_events::unit_matches_filter(unit2,*sfi)) {
			return false;
		}
		if(!sfi->empty()) {
			filtered_unit2 = true;
		}
	}
	const vconfig::child_list second_special_filters = handler.second_special_filters();
	special_matches = second_special_filters.size() ? false : true;
	for(ffi = second_special_filters.begin();
			ffi != second_special_filters.end(); ++ffi) {

		if(unit2 != units->end() && game_events::matches_special_filter(ev.data.child("second"),*ffi)) {
			special_matches = true;
		}
		if(!ffi->empty()) {
			filtered_unit2 = true;
		}
	}
	if(!special_matches) {
		return false;
	}
	if(ev.loc1.requires_unit() && filtered_unit1
	&& (unit1 == units->end() || !ev.loc1.matches_unit(unit1->second))) {
		// Wrong or missing entity at src location
		return false;
	}
	if(ev.loc2.requires_unit()  && filtered_unit2
	&& (unit2 == units->end() || !ev.loc2.matches_unit(unit2->second))) {
		// Wrong or missing entity at dst location
		return false;
	}

	// The event hasn't been filtered out, so execute the handler
	const bool res = handler.handle_event(ev);
	if(ev.name == "select") {
		state_of_game->last_selected = ev.loc1;
	}

	if(handler.rebuild_screen()) {
		handler.rebuild_screen() = false;
		screen->recalculate_minimap();
		screen->invalidate_all();
		screen->rebuild_all();
	}

	if(handler.first_time_only()) {
		handler.disable();
	}

	return res;
}

namespace game_events {

bool matches_special_filter(const config* cfg, const vconfig filter)
{
	//! @todo FIXME: This filter should be deprecated and removed,
	// instead we should just auto-store $attacker_weapon and check it in a conditional

	if(!cfg) {
		return false;
	}
	bool matches = true;
	if(filter["weapon"] != "") {
		if(filter["weapon"] != (*cfg)["weapon"]) {
			matches = false;
		}
	}

	// Handle [and], [or], and [not] with in-order precedence
	config::all_children_iterator cond_i = filter.get_config().ordered_begin();
	config::all_children_iterator cond_end = filter.get_config().ordered_end();
	while(cond_i != cond_end)
	{
		const std::string& cond_name = *((*cond_i).first);
		const vconfig cond_filter(&(*((*cond_i).second)));

		// Handle [and]
		if(cond_name == "and")
		{
			matches = matches && matches_special_filter(cfg, cond_filter);
		}
		// Handle [or]
		else if(cond_name == "or")
		{
			matches = matches || matches_special_filter(cfg, cond_filter);
		}
		// Handle [not]
		else if(cond_name == "not")
		{
			matches = matches && !matches_special_filter(cfg, cond_filter);
		}
		++cond_i;
	}
	return matches;
}

bool unit_matches_filter(const unit& u, const vconfig filter,const gamemap::location& loc)
{
	return u.matches_filter(filter,loc);
}

bool unit_matches_filter(unit_map::const_iterator itor, const vconfig filter)
{
	return itor->second.matches_filter(filter,itor->first);
}

static config::child_list unit_wml_configs;
static std::set<std::string> unit_wml_ids;

manager::manager(const config& cfg, game_display& gui_, gamemap& map_,
		 soundsource::manager& sndsources_,
                 unit_map& units_,
                 std::vector<team>& teams_,
                 game_state& state_of_game_, gamestatus& status,
		 const game_data& game_data_) :
	variable_manager(&state_of_game_)
{
	const config::child_list& events_list = cfg.get_children("event");
	for(config::child_list::const_iterator i = events_list.begin();
	    i != events_list.end(); ++i) {
		event_handler new_handler(**i);
		events_map.insert(std::pair<std::string,event_handler>(
					new_handler.name(), new_handler));
	}
	std::vector<std::string> unit_ids = utils::split(cfg["unit_wml_ids"]);
	for(std::vector<std::string>::const_iterator id_it = unit_ids.begin(); id_it != unit_ids.end(); ++id_it) {
		unit_wml_ids.insert(*id_it);
	}

	teams = &teams_;
	screen = &gui_;
	soundsources = &sndsources_;
	game_map = &map_;
	units = &units_;
	state_of_game = &state_of_game_;
	game_data_ptr = &game_data_;
	status_ptr = &status;

	used_items.clear();

	const std::string used = cfg["used_items"];
	if(!used.empty()) {
		const std::vector<std::string>& v = utils::split(used);
		for(std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
			used_items.insert(*i);
		}
	}
	int wmi_count = 0;
	std::map<std::string, wml_menu_item *>::iterator itor = state_of_game->wml_menu_items.begin();
	while(itor != state_of_game->wml_menu_items.end()) {
		if(!itor->second->command.empty()) {
			event_handler new_handler(itor->second->command);
			events_map.insert(std::pair<std::string,event_handler>(
				new_handler.name(), new_handler));
		}
		++itor;
		++wmi_count;
	}
	if(wmi_count > 0) {
		LOG_NG << wmi_count << " WML menu items found, loaded." << std::endl;
	}
}

void write_events(config& cfg)
{
	for(std::multimap<std::string,event_handler>::const_iterator i = events_map.begin(); i != events_map.end(); ++i) {
		if(!i->second.disabled() && !i->second.is_menu_item()) {
			i->second.write(cfg.add_child("event"));
		}
	}

	std::stringstream used;
	std::set<std::string>::const_iterator u;
	for(u = used_items.begin(); u != used_items.end(); ++u) {
		if(u != used_items.begin())
			used << ",";

		used << *u;
	}

	cfg["used_items"] = used.str();
	std::stringstream ids;
	for(u = unit_wml_ids.begin(); u != unit_wml_ids.end(); ++u) {
		if(u != unit_wml_ids.begin())
			ids << ",";

		ids << *u;
	}

	cfg["unit_wml_ids"] = ids.str();

	if(screen != NULL)
		screen->write_overlays(cfg);
}

manager::~manager() {
	events_queue.clear();
	events_map.clear();
	screen = NULL;
	game_map = NULL;
	units = NULL;
	state_of_game = NULL;
	game_data_ptr = NULL;
	status_ptr = NULL;
	for(config::child_list::iterator d = unit_wml_configs.begin(); d != unit_wml_configs.end(); ++d) {
		delete *d;
	}
	unit_wml_configs.clear();
	unit_wml_ids.clear();
}

void raise(const std::string& event,
           const entity_location& loc1,
           const entity_location& loc2,
		   const config& data)
{
	if(!events_init())
		return;

	events_queue.push_back(queued_event(event,loc1,loc2,data));
}

bool fire(const std::string& event,
          const entity_location& loc1,
          const entity_location& loc2,
		  const config& data)
{
	raise(event,loc1,loc2,data);
	return pump();
}

void add_events(const config::child_list& cfgs,const std::string& id)
{
	if(std::find(unit_wml_ids.begin(),unit_wml_ids.end(),id) == unit_wml_ids.end()) {
		unit_wml_ids.insert(id);
		for(config::child_list::const_iterator new_ev = cfgs.begin(); new_ev != cfgs.end(); ++ new_ev) {
			unit_wml_configs.push_back(new config(**new_ev));
			event_handler new_handler(*unit_wml_configs.back());
			events_map.insert(std::pair<std::string,event_handler>(new_handler.name(), new_handler));
		}
	}
}

bool pump()
{
	if(!events_init())
		return false;

	bool result = false;

	while(events_queue.empty() == false) {
		queued_event ev = events_queue.front();
		events_queue.pop_front();	// pop now for exception safety
		const std::string& event_name = ev.name;
		typedef std::multimap<std::string,event_handler>::iterator itor;

		// Clear the unit cache, since the best clearing time is hard to figure out
		// due to status changes by WML. Every event will flush the cache.
		unit::clear_status_caches();

		// Find all handlers for this event in the map
		std::pair<itor,itor> i = events_map.equal_range(event_name);

		// Set the variables for the event
		if(i.first != i.second && state_of_game != NULL) {
			char buf[50];
			snprintf(buf,sizeof(buf),"%d",ev.loc1.x+1);
			state_of_game->set_variable("x1", buf);

			snprintf(buf,sizeof(buf),"%d",ev.loc1.y+1);
			state_of_game->set_variable("y1", buf);

			snprintf(buf,sizeof(buf),"%d",ev.loc2.x+1);
			state_of_game->set_variable("x2", buf);

			snprintf(buf,sizeof(buf),"%d",ev.loc2.y+1);
			state_of_game->set_variable("y2", buf);
		}

		while(i.first != i.second) {
			LOG_NG << "processing event '" << event_name << "'\n";
			event_handler& handler = i.first->second;
			if(process_event(handler, ev))
				result = true;
			++i.first;
		}

		commit_wmi_commands();
		commit_new_handlers();

		// Dialogs can only be shown if the display is not locked
		if(! screen->video().update_locked()) {
			show_wml_errors();
		}
	}

	return result;
}

Uint32 mutations() {
	return unit_mutations;
}

entity_location::entity_location(gamemap::location loc, const std::string& id)
	: location(loc), id_(id)
{}

entity_location::entity_location(unit_map::iterator itor)
	: location(itor->first), id_(itor->second.underlying_description())
{}

bool entity_location::matches_unit(const unit& u) const
{
	return id_ == u.underlying_description();
}

bool entity_location::requires_unit() const
{
	return !id_.empty();
}

} // end namespace game_events (2)

