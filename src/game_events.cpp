/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "game_events.hpp"
#include "image.hpp"
#include "language.hpp"
#include "playlevel.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"
#include "util.hpp"

#include <cstdlib>
#include <deque>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

namespace game_events {

bool conditional_passed(game_state& state_of_game,
                        const std::map<gamemap::location,unit>* units,
                        const config& cond)
{
	//if the if statement requires we have a certain unit, then
	//check for that.
	const config::child_list& have_unit = cond.get_children("have_unit");

	for(config::child_list::const_iterator u = have_unit.begin();
	    u != have_unit.end(); ++u) {

		if(units == NULL)
			return false;

		std::map<gamemap::location,unit>::const_iterator itor;
		for(itor = units->begin(); itor != units->end(); ++itor) {
			if(itor->second.matches_filter(**u)) {
				break;
			}
		}

		if(itor == units->end()) {
			return false;
		}
	}

	//check against each variable statement to see if the variable
	//matches the conditions or not
	const config::child_list& variables = cond.get_children("variable");
	for(config::child_list::const_iterator var = variables.begin();
	    var != variables.end(); ++var) {
		const string_map& values = (*var)->values;
		string_map& vars = state_of_game.variables;

		string_map::const_iterator itor = values.find("name");
		if(itor == values.end())
			return false;

		const std::string& name = itor->second;

		//if we don't have a record of the variable, then the statement
		//is not true, unless it's a not equals statement, in which it's
		//automatically true
		if(vars.find(name) == vars.end()) {
			if(values.count("not_equals") == 0 && values.count("numerical_not_equals") == 0) {
				return false;
			}

			continue;
		}

		const std::string& value = vars[name];
		const double num_value = atof(value.c_str());

		itor = values.find("equals");
		if(itor != values.end() && itor->second != value) {
			return false;
		}

		itor = values.find("numerical_equals");
		if(itor != values.end() && atof(itor->second.c_str()) != num_value){
			return false;
		}

		itor = values.find("not_equals");
		if(itor != values.end() && itor->second == value) {
			return false;
		}

		itor = values.find("numerical_not_equals");
		if(itor != values.end() && atof(itor->second.c_str()) == num_value){
			return false;
		}

		itor = values.find("greater_than");
		if(itor != values.end() && atof(itor->second.c_str()) >= num_value){
			return false;
		}

		itor = values.find("less_than");
		if(itor != values.end() && atof(itor->second.c_str()) <= num_value){
			return false;
		}

		itor = values.find("greater_than_equal_to");
		if(itor != values.end() && atof(itor->second.c_str()) > num_value){
			return false;
		}

		itor = values.find("less_than_equal_to");
		if(itor != values.end() && atof(itor->second.c_str()) < num_value){
			return false;
		}
	}

	return true;
}

} //end namespace game_events

namespace {

display* screen = NULL;
gamemap* game_map = NULL;
std::map<gamemap::location,unit>* units = NULL;
std::vector<team>* teams = NULL;
game_state* state_of_game = NULL;
game_data* game_data_ptr = NULL;
std::set<std::string> used_items;

bool events_init() { return screen != NULL; }

struct queued_event {
	queued_event(const std::string& name, const gamemap::location& loc1,
	                                      const gamemap::location& loc2)
			: name(name), loc1(loc1), loc2(loc2) {}

	std::string name;
	gamemap::location loc1;
	gamemap::location loc2;
};

std::deque<queued_event> events_queue;

class event_handler
{
public:
	event_handler(const config* cfg) : name_((*cfg)["name"]),
	                 first_time_only_((*cfg)["first_time_only"] != "no"),
					 disabled_(false),
	                 cfg_(cfg)
	{}

	void write(config& cfg) const {
		if(cfg_ == NULL || disabled_)
			return;

		cfg = *cfg_;
	}

	const std::string& name() const { return name_; }

	bool first_time_only() const { return first_time_only_; }

	void disable() { disabled_ = true; }
	bool disabled() const { return disabled_; }

	const config::child_list& first_arg_filters() {
		return cfg_->get_children("filter");
	}

	const config::child_list& second_arg_filters() {
		return cfg_->get_children("filter_second");
	}

	void handle_event(const queued_event& event_info, const config* cfg=NULL);

private:
	void handle_event_command(const queued_event& event_info, const std::string& cmd, const config& cfg);

	std::string name_;
	bool first_time_only_;
	bool disabled_;
	const config* cfg_;
};

std::pair<int,int> parse_range(const std::string& str)
{
	const std::string::const_iterator dash = std::find(str.begin(),str.end(),'-');
	const std::string a(str.begin(),dash);
	const std::string b = dash != str.end() ? std::string(dash+1,str.end()) : a;
	std::pair<int,int> res(atoi(a.c_str()),atoi(b.c_str()));
	if(res.second < res.first)
		res.second = res.first;

	return res;
}

std::vector<gamemap::location> multiple_locs(const config& cfg)
{
	std::vector<gamemap::location> res;
	const std::vector<std::string> xvals = config::split(cfg["x"]);
	const std::vector<std::string> yvals = config::split(cfg["y"]);

	for(unsigned int i = 0; i != minimum(xvals.size(),yvals.size()); ++i) {
		const std::pair<int,int> xrange = parse_range(xvals[i]);
		const std::pair<int,int> yrange = parse_range(yvals[i]);
		std::cerr << "range: " << xrange.first << "-" << xrange.second << "\n";
		std::cerr << "range: " << yrange.first << "-" << yrange.second << "\n";

		for(int x = xrange.first; x <= xrange.second; ++x) {
			for(int y = yrange.first; y <= yrange.second; ++y) {
				res.push_back(gamemap::location(x-1,y-1));
			}
		}
	}

	return res;
}

std::multimap<std::string,event_handler> events_map;

//this function handles all the different types of actions that can be triggered
//by an event.
void event_handler::handle_event_command(const queued_event& event_info, const std::string& cmd, const config& cfg)
{
	//sub commands that need to be handled in a guaranteed ordering
	if(cmd == "command") {
		handle_event(event_info,&cfg);
	}

	//reveal sections of the map that would otherwise be under shroud
	else if(cmd == "remove_shroud") {
		const size_t index = maximum<int>(1,atoi(cfg["side"].c_str())) - 1;
		if(index < teams->size()) {
			const std::vector<gamemap::location>& locs = multiple_locs(cfg);
			for(std::vector<gamemap::location>::const_iterator j = locs.begin(); j != locs.end(); ++j) {
				(*teams)[index].clear_shroud(j->x,j->y);
			}
		}
		
		screen->invalidate_all();
	}


	//teleport a unit from one location to another
	else if(cmd == "teleport") {
		
		unit_map::iterator u = units->find(event_info.loc1);

		//search for a valid unit filter, and if we have one, look for the matching unit
		const config* const filter = cfg.child("filter");
		if(filter != NULL) {	
			for(u = units->begin(); u != units->end(); ++u){
				if(game_events::unit_matches_filter(u,*filter))
					break;
			}
		}

		//we have found a unit that matches the filter
		if(u != units->end()) {
			const gamemap::location dst(cfg);
			if(game_map->on_board(dst)) {
				const gamemap::location vacant_dst = find_vacant_tile(*game_map,*units,dst,(*game_map)[dst.x][dst.y]);
				if(game_map->on_board(vacant_dst)) {
					//note that inserting into a map does NOT invalidate iterators
					//into the map, so this sequence is fine.
					units->insert(std::pair<gamemap::location,unit>(vacant_dst,u->second));
					units->erase(u);
				}
			}
		}
	}

	//allow a side to recruit a new type of unit
	else if(cmd == "allow_recruit") {
		const int side = maximum<int>(1,atoi(cfg["side"].c_str()));
		const size_t index = side-1;
		if(index > teams->size())
			return;

		const std::string& type = cfg["type"];
		(*teams)[index].recruits().insert(type);
		if(index == 0) {
			state_of_game->can_recruit.insert(type);
		}
	}

	//remove the ability to recruit a unit from a certain side
	else if(cmd == "disallow_recruit") {
		const int side = maximum<int>(1,atoi(cfg["side"].c_str()));
		const size_t index = side-1;
		if(index > teams->size())
			return;

		const std::string& type = cfg["type"];
		(*teams)[index].recruits().erase(type);
		if(index == 0) {
			state_of_game->can_recruit.erase(type);
		}
	}

	else if(cmd == "set_recruit") {
		const int side = maximum<int>(1,atoi(cfg["side"].c_str()));
		const size_t index = side-1;
		if(index > teams->size())
			return;

		std::vector<std::string> recruit = config::split(cfg["recruit"]);
		if(recruit.size() == 1 && recruit.back() == "")
			recruit.clear();

		std::set<std::string>& can_recruit = (*teams)[index].recruits();
		can_recruit.clear();
		std::copy(recruit.begin(),recruit.end(),std::inserter(can_recruit,can_recruit.end()));
		if(index == 0) {
			state_of_game->can_recruit = can_recruit;
		}
	}

	else if(cmd == "sound") {
		sound::play_sound(cfg["name"]);
	}

	else if(cmd == "colour_adjust") {
		const int r = atoi(cfg["red"].c_str());
		const int g = atoi(cfg["green"].c_str());
		const int b = atoi(cfg["blue"].c_str());
		screen->adjust_colours(r,g,b);
		screen->invalidate_all();
		screen->draw(true,true);
	}

	else if(cmd == "delay") {
		const int delay_time = atoi(cfg["time"].c_str());
		::SDL_Delay(delay_time);
	}

	else if(cmd == "scroll") {
		const int xoff = atoi(cfg["x"].c_str());
		const int yoff = atoi(cfg["y"].c_str());
		screen->scroll(xoff,yoff);
		screen->draw(true,true);
	}

	else if(cmd == "scroll_to_unit") {
		unit_map::const_iterator u;
		for(u = units->begin(); u != units->end(); ++u){
			if(game_events::unit_matches_filter(u,cfg))
				break;
		}

		if(u != units->end()) {
			screen->scroll_to_tile(u->first.x,u->first.y);
		}
	}

	//an award of gold to a particular side
	else if(cmd == "gold") {
		const std::string& side = cfg["side"];
		const std::string& amount = cfg["amount"];
		const int side_num = side.empty() ? 1 : atoi(side.c_str());
		const int amount_num = atoi(amount.c_str());
		const size_t team_index = side_num-1;
		if(team_index < teams->size()) {
			(*teams)[team_index].spend_gold(-amount_num);
		}
	}

	//moving a 'unit' - i.e. a dummy unit that is just moving for
	//the visual effect
	else if(cmd == "move_unit_fake") {
		const std::string& type = cfg["type"];
		const game_data::unit_type_map::const_iterator itor = game_data_ptr->unit_types.find(type);
		if(itor != game_data_ptr->unit_types.end()) {
			unit dummy_unit(&itor->second,0);
			const std::vector<std::string> xvals = config::split(cfg["x"]);
			const std::vector<std::string> yvals = config::split(cfg["y"]);
			std::vector<gamemap::location> path;
			for(size_t i = 0; i != minimum(xvals.size(),yvals.size()); ++i) {
				path.push_back(gamemap::location(atoi(xvals[i].c_str())-1,
				                                 atoi(yvals[i].c_str())-1));
			}

			screen->move_unit(path,dummy_unit);
		}
	}

	//setting a variable
	else if(cmd == "set_variable") {
		const std::string& name = cfg["name"];
		const std::string& value = cfg["value"];
		if(value.empty() == false) {
			state_of_game->variables[name] = value;
		}

		const std::string& add = cfg["add"];
		if(add.empty() == false) {
			double value = atof(state_of_game->variables[name].c_str());
			value += atof(add.c_str());
			char buf[50];
			sprintf(buf,"%f",value);
			state_of_game->variables[name] = buf;
		}

		const std::string& multiply = cfg["multiply"];
		if(multiply.empty() == false) {
			double value = atof(state_of_game->variables[name].c_str());
			value *= atof(multiply.c_str());
			char buf[50];
			sprintf(buf,"%f",value);
			state_of_game->variables[name] = buf;
		}
	}

	//conditional statements
	else if(cmd == "if") {
		const std::string type = game_events::conditional_passed(
		                     *state_of_game,units,cfg) ? "then":"else";

		//if the if statement passed, then execute all 'then' statements,
		//otherwise execute 'else' statements
		const config::child_list& commands = cfg.get_children(type);
		for(config::child_list::const_iterator cmd = commands.begin();
		    cmd != commands.end(); ++cmd) {
			handle_event(event_info,*cmd);
		}
	}

	//if we are assigning a role to a unit from the available units list
	else if(cmd == "role") {

		//get a list of the types this unit can be
		std::vector<std::string> types = config::split(cfg["type"]);

		//iterate over all the types, and for each type, try to find
		//a unit that matches
		std::vector<std::string>::iterator ti;
		for(ti = types.begin(); ti != types.end(); ++ti) {
			config item;
			item["type"] = *ti;

			std::map<gamemap::location,unit>::iterator itor;
			for(itor = units->begin(); itor != units->end(); ++itor) {
				if(game_events::unit_matches_filter(itor,item)) {
					itor->second.assign_role(cfg["role"]);
					break;
				}
			}

			if(itor != units->end())
				break;

			std::vector<unit>::iterator ui;
			//iterate over the units, and try to find one that matches
			for(ui = state_of_game->available_units.begin();
			    ui != state_of_game->available_units.end(); ++ui) {
				if(ui->matches_filter(item)) {
					ui->assign_role(cfg["role"]);
					break;
				}
			}

			//if we found a unit, we don't have to keep going.
			if(ui != state_of_game->available_units.end())
				break;
		}
	}

	else if(cmd == "removeitem") {
		gamemap::location loc(cfg);
		if(!loc.valid()) {
			loc = event_info.loc1;
		}

		screen->remove_overlay(loc);
	}

	//hiding units
	else if(cmd == "hide_unit") {
		const gamemap::location loc(cfg);
		screen->hide_unit(loc);
		screen->draw_tile(loc.x,loc.y);
	}

	else if(cmd == "unhide_unit") {
		const gamemap::location loc = screen->hide_unit(gamemap::location());
		screen->draw_tile(loc.x,loc.y);
	}

	//adding new items
	else if(cmd == "item") {
		gamemap::location loc(cfg);
		const std::string& img = cfg["image"];
		if(!img.empty()) {
			screen->add_overlay(loc,img);
			screen->draw_tile(loc.x,loc.y);
		}
	}

	//changing the terrain
	else if(cmd == "terrain") {
		const std::vector<gamemap::location> locs = multiple_locs(cfg);

		for(std::vector<gamemap::location>::const_iterator loc = locs.begin(); loc != locs.end(); ++loc) {
			const std::string& terrain_type = cfg["letter"];
			if(terrain_type.size() > 0) {
				game_map->set_terrain(*loc,terrain_type[0]);
				screen->recalculate_minimap();
				screen->invalidate_all();
			}
		}
	}

	//if we should spawn a new unit on the map somewhere
	else if(cmd == "unit") {
		std::cerr << "spawning unit...\n";
		unit new_unit(*game_data_ptr,cfg);
		gamemap::location loc(cfg);

		std::cerr << "location: " << loc.x << "," << loc.y << "\n";

		if(game_map->on_board(loc)) {
			loc = find_vacant_tile(*game_map,*units,loc);
			std::cerr << "found vacant tile: " << loc.x << "," << loc.y << "\n";
			units->insert(std::pair<gamemap::location,unit>(loc,new_unit));
			screen->invalidate(loc);
		} else {
			state_of_game->available_units.push_back(new_unit);
		}
	}

	//if we should recall units that match a certain description
	else if(cmd == "recall") {
		std::vector<unit>& avail = state_of_game->available_units;
		for(std::vector<unit>::iterator u = avail.begin(); u != avail.end(); ++u) {
			if(u->matches_filter(cfg)) {
				recruit_unit(*game_map,1,*units,*u,gamemap::location(),screen,false,true);
				avail.erase(u);
				break;
			}
		}
	}

	else if(cmd == "object") {
		const std::string& id = cfg["id"];

		//if this item has already been used
		if(used_items.count(id))
			return;

		const std::string image = cfg["image"];
		std::string caption = cfg["name"];

		const std::string& caption_lang = string_table[id + "_name"];
		if(caption_lang.empty() == false)
			caption = caption_lang;

		const std::map<gamemap::location,unit>::iterator u = units->find(event_info.loc1);

		if(u == units->end())
			return;

		std::string text;

		const config::child_list& filters = cfg.get_children("filter");

		if(filters.empty() || u->second.matches_filter(*filters.front())) {
			const std::string& lang = string_table[id];
			if(!lang.empty())
				text = lang;
			else
				text = cfg["description"];

			u->second.add_modification("object",cfg);
			screen->remove_overlay(event_info.loc1);
			screen->select_hex(event_info.loc1);
			screen->invalidate_unit();

			//mark that this item won't be used again
			used_items.insert(id);
		} else {
			const std::string& lang = string_table[id + "_cannot_use"];
			if(!lang.empty())
				text = lang;
			else
				text = cfg["cannot_use_message"];
		}

		scoped_sdl_surface surface(NULL);

		if(image.empty() == false) {
			surface.assign(image::get_image(image,image::UNSCALED));
		}

		gui::show_dialog(*screen,surface,caption,text);

		//this will redraw the unit, with its new stats
		screen->draw();
	}

	//displaying a message dialog
	else if(cmd == "message") {
		std::map<gamemap::location,unit>::iterator speaker = units->end();
		if(cfg["speaker"] == "unit") {
			speaker = units->find(event_info.loc1);
		} else if(cfg["speaker"] == "second_unit") {
			speaker = units->find(event_info.loc2);
		} else if(cfg["speaker"] != "narrator") {
			for(speaker = units->begin(); speaker != units->end(); ++speaker){
				if(game_events::unit_matches_filter(speaker,cfg))
					break;
			}

			if(speaker == units->end()) {
				//no matching unit found, so the dialog can't come up
				//continue onto the next message
				return;
			}
		}

		const std::string& sfx = cfg["sound"];
		if(sfx != "") {
			sound::play_sound(sfx);
		}

		const std::string& id = cfg["id"];

		std::string image = cfg["image"];
		std::string caption;

		const std::string& lang_caption = string_table[id + "_caption"];
		if(!lang_caption.empty())
			caption = lang_caption;
		else
			caption = cfg["caption"];

		if(speaker != units->end()) {
			screen->highlight_hex(speaker->first);
			screen->scroll_to_tile(speaker->first.x,speaker->first.y);

			if(image.empty()) {
				image = speaker->second.type().image_profile();
			}

			if(caption.empty()) {
				caption = speaker->second.description();
				if(caption.empty()) {
					caption = speaker->second.type().language_name();
				}
			}
		}

		std::vector<std::string> options;
		std::vector<config::const_child_itors> option_events;

		std::cerr << "building menu items...\n";

		const config::child_list& menu_items = cfg.get_children("option");
		for(config::child_list::const_iterator mi = menu_items.begin();
		    mi != menu_items.end(); ++mi) {
			const std::string& msg = translate_string_default((**mi)["id"],(**mi)["message"]);
			options.push_back(msg);
			option_events.push_back((*mi)->child_range("command"));
		}

		scoped_sdl_surface surface(NULL);
		if(image.empty() == false) {
			surface.assign(image::get_image(image,image::UNSCALED));
		}

		const std::string& lang_message = string_table[id];
		int option_chosen = -1;
		
		//if we're not replaying, or if there is no choice to be made, show
		//the dialog.
		if(recorder.at_end() || options.empty()) {
			option_chosen = gui::show_dialog(*screen,surface,caption,
		                        lang_message.empty() ? cfg["message"] : lang_message,
		                        options.empty() ? gui::MESSAGE : gui::OK_ONLY,
		                        options.empty() ? NULL : &options);

			if(options.empty() == false) {
				recorder.choose_option(option_chosen);
			}
		}

		//otherwise if a choice has to be made, get it from the replay data
		else {
			const config* const action = recorder.get_next_action();
			if(action == NULL || action->get_children("choose").empty()) {
				std::cerr << "choice expected but none found\n";
				throw replay::error();
			}

			if(size_t(option_chosen) >= options.size()) {
				option_chosen = 0;
			}

			const std::string& val = (*(action->get_children("choose").front()))["value"];
			option_chosen = atol(val.c_str());
		}

		//implement the consequences of the choice
		if(options.empty() == false) {
			assert(size_t(option_chosen) < menu_items.size());
			
			for(config::const_child_itors events = option_events[option_chosen];
			    events.first != events.second; ++events.first) {
				handle_event(event_info,*events.first);
			}
		}
	}

	else if(cmd == "kill") {

		for(std::map<gamemap::location,unit>::iterator un = units->begin();
		    un != units->end(); ++un) {
			while(un != units->end() && game_events::unit_matches_filter(un,cfg)) {
				units->erase(un);
				un = units->begin();
			}
		}

		std::vector<unit>& avail_units = state_of_game->available_units;
		for(std::vector<unit>::iterator j = avail_units.begin();
		    j != avail_units.end(); ++j) {
			while(j != avail_units.end() && j->matches_filter(cfg)) {
				j = avail_units.erase(j);
			}
		}
	}

	//adding of new events
	else if(cmd == "event") {
		event_handler new_handler(&cfg);
		events_map.insert(std::pair<std::string,event_handler>(new_handler.name(),new_handler));
	}

	else if(cmd == "endlevel") {
		const std::string& next_scenario = cfg["next_scenario"];
		if(next_scenario.empty() == false) {
			state_of_game->scenario = next_scenario;
		}

		const std::string& result = cfg["result"];
		if(result.empty() || result == "victory") {
			const bool bonus = cfg["bonus"] == "yes";
			throw end_level_exception(VICTORY,bonus);
		} else {
			throw end_level_exception(DEFEAT);
		}
	}
}

void event_handler::handle_event(const queued_event& event_info, const config* cfg)
{
	if(cfg == NULL)
		cfg = cfg_;

	for(config::all_children_iterator i = cfg->ordered_begin(); i != cfg->ordered_end(); ++i) {
		const std::pair<const std::string*,const config*> item = *i;

		handle_event_command(event_info,*item.first,*item.second);
	}
}

bool filter_loc_impl(const gamemap::location& loc, const std::string& xloc,
                                                   const std::string& yloc)
{
	if(std::find(xloc.begin(),xloc.end(),',') != xloc.end()) {
		std::vector<std::string> xlocs = config::split(xloc);
		std::vector<std::string> ylocs = config::split(yloc);

		const int size = xlocs.size() < ylocs.size()?xlocs.size():ylocs.size();
		for(int i = 0; i != size; ++i) {
			if(filter_loc_impl(loc,xlocs[i],ylocs[i]))
				return true;
		}

		return false;
	}

	if(!xloc.empty()) {
		const std::string::const_iterator dash =
		             std::find(xloc.begin(),xloc.end(),'-');

		if(dash != xloc.end()) {
			const std::string beg(xloc.begin(),dash);
			const std::string end(dash+1,xloc.end());

			const int bot = atoi(beg.c_str()) - 1;
			const int top = atoi(end.c_str()) - 1;

			if(loc.x < bot || loc.x > top)
				return false;
		} else {
			const int xval = atoi(xloc.c_str()) - 1;
			if(xval != loc.x)
				return false;
		}
	}

	if(!yloc.empty()) {
		const std::string::const_iterator dash =
		             std::find(yloc.begin(),yloc.end(),'-');

		if(dash != yloc.end()) {
			const std::string beg(yloc.begin(),dash);
			const std::string end(dash+1,yloc.end());

			const int bot = atoi(beg.c_str()) - 1;
			const int top = atoi(end.c_str()) - 1;

			if(loc.y < bot || loc.y > top)
				return false;
		} else {
			const int yval = atoi(yloc.c_str()) - 1;
			if(yval != loc.y)
				return false;
		}
	}

	return true;
}

bool filter_loc(const gamemap::location& loc, const config& cfg)
{
	const std::string& xloc = cfg["x"];
	const std::string& yloc = cfg["y"];

	return filter_loc_impl(loc,xloc,yloc);
}

bool process_event(event_handler& handler, const queued_event& ev)
{
	if(handler.disabled())
		return false;

	std::map<gamemap::location,unit>::iterator unit1 = units->find(ev.loc1);
	std::map<gamemap::location,unit>::iterator unit2 = units->find(ev.loc2);

	const config::child_list& first_filters = handler.first_arg_filters();
	for(config::child_list::const_iterator ffi = first_filters.begin();
	    ffi != first_filters.end(); ++ffi) {

			if(!game_events::unit_matches_filter(unit1,**ffi))
			return false;
	}

	const config::child_list& second_filters = handler.second_arg_filters();
	for(config::child_list::const_iterator sfi = second_filters.begin();
	    sfi != second_filters.end(); ++sfi) {
		if(!game_events::unit_matches_filter(unit2,**sfi))
			return false;
	}

	//the event hasn't been filtered out, so execute the handler
	handler.handle_event(ev);

	if(handler.first_time_only())
		handler.disable();

	return true;
}

} //end anonymous namespace

namespace game_events {

bool unit_matches_filter(unit_map::const_iterator itor, const config& filter)
{
	return filter_loc(itor->first,filter) && itor->second.matches_filter(filter);
}

const std::string& get_variable(const std::string& key)
{
	static const std::string empty_string;
	if(state_of_game != NULL) {
		const string_map::const_iterator i = state_of_game->variables.find(key);
		if(i != state_of_game->variables.end())
			return i->second;
		else
			return empty_string;
	} else {
		return empty_string;
	}
}

manager::manager(config& cfg, display& gui_, gamemap& map_,
                 std::map<gamemap::location,unit>& units_,
                 std::vector<team>& teams_,
                 game_state& state_of_game_, game_data& game_data_)
{
	const config::child_list& events_list = cfg.get_children("event");
	for(config::child_list::const_iterator i = events_list.begin();
	    i != events_list.end(); ++i) {
		event_handler new_handler(*i);
		events_map.insert(std::pair<std::string,event_handler>(
		                                   new_handler.name(), new_handler));
	}

	teams = &teams_;
	screen = &gui_;
	game_map = &map_;
	units = &units_;
	state_of_game = &state_of_game_;
	game_data_ptr = &game_data_;

	used_items.clear();
	const std::string& used = cfg["used_items"];
	if(!used.empty()) {
		const std::vector<std::string>& v = config::split(used);
		for(std::vector<std::string>::const_iterator i = v.begin(); i != v.end(); ++i) {
			used_items.insert(*i);
		}
	}
}

void write_events(config& cfg)
{
	for(std::multimap<std::string,event_handler>::const_iterator i = events_map.begin(); i != events_map.end(); ++i) {
		if(!i->second.disabled()) {
			i->second.write(cfg.add_child("event"));
		}
	}

	std::stringstream used;
	for(std::set<std::string>::const_iterator u = used_items.begin(); u != used_items.end(); ++u) {
		if(u != used_items.begin())
			used << ",";

		used << *u;
	}

	cfg["used_items"] = used.str();

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
}

void raise(const std::string& event,
           const gamemap::location& loc1,
           const gamemap::location& loc2)
{
	if(!events_init())
		return;

	events_queue.push_back(queued_event(event,loc1,loc2));
}

bool fire(const std::string& event,
          const gamemap::location& loc1,
          const gamemap::location& loc2)
{
	raise(event,loc1,loc2);
	return pump();
}

bool pump()
{
	if(!events_init())
		return false;

	bool result = false;

	while(events_queue.empty() == false) {
		queued_event ev = events_queue.front();
		events_queue.pop_front(); //pop now for exception safety
		const std::string& event_name = ev.name;
		typedef std::multimap<std::string,event_handler>::iterator itor;

		//find all handlers for this event in the map
		std::pair<itor,itor> i = events_map.equal_range(event_name);

		while(i.first != i.second) {
			event_handler& handler = i.first->second;
			if(process_event(handler, ev))
				result = true;
			++i.first;
		}
	}

	return result;
}

} //end namespace game_events
