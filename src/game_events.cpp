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
                        config& cond)
{
	//if the if statement requires we have a certain unit, then
	//check for that.
	std::vector<config*>& have_unit = cond.children["have_unit"];

	for(std::vector<config*>::iterator u = have_unit.begin();
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
	std::vector<config*>& variables = cond.children["variable"];
	for(std::vector<config*>::iterator var = variables.begin();
	    var != variables.end(); ++var) {
		string_map& values = (*var)->values;
		string_map& vars = state_of_game.variables;
		const std::string& name = values["name"];

		//if we don't have a record of the variable, then the statement
		//is not true, unless it's a not equals statement, in which it's
		//automatically true
		if(vars.find(name) == vars.end()) {
			if(values.find("not_equals") == values.end() &&
			   values.find("numerical_not_equals") == values.end()) {
				return false;
			}

			continue;
		}

		const std::string& value = vars[name];
		const double num_value = atof(value.c_str());

		string_map::iterator itor;

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
	event_handler(config* cfg) : name_(cfg->values["name"]),
	                 first_time_only_(cfg->values["first_time_only"] != "no"),
					 disabled_(false),
	                 cfg_(cfg)
	{}

	const std::string& name() const { return name_; }

	bool first_time_only() const { return first_time_only_; }

	void disable() { disabled_ = true; }
	bool disabled() const { return disabled_; }

	std::vector<config*>& first_arg_filters() {
		return cfg_->children["filter"];
	}

	std::vector<config*>& second_arg_filters() {
		return cfg_->children["filter_second"];
	}

	void handle_event(const queued_event& event_info, const config* cfg=NULL);

private:
	std::string name_;
	bool first_time_only_;
	bool disabled_;
	config* cfg_;
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

void event_handler::handle_event(const queued_event& event_info, const config* cfg)
{
	if(cfg == NULL)
		cfg = cfg_;

	std::vector<config*>::const_iterator i;

	//sub commands that need to be handled in a guaranteed ordering
	const config::child_list& commands = cfg->get_children("command");
	for(i = commands.begin(); i != commands.end(); ++i) {
		handle_event(event_info,*i);
	}

	//allow a side to recruit a new type of unit
	const config::child_list& allow_recruit = cfg->get_children("allow_recruit");
	for(i = allow_recruit.begin(); i != allow_recruit.end(); ++i) {
		const int side = maximum<int>(1,atoi((**i)["side"].c_str()));
		const size_t index = side-1;
		if(index > teams->size())
			continue;

		const std::string& type = (**i)["type"];
		(*teams)[index].recruits().insert(type);
		if(index == 0) {
			state_of_game->can_recruit.insert(type);
		}
	}

	//remove the ability to recruit a unit from a certain side
	const config::child_list& disallow_recruit = cfg->get_children("disallow_recruit");
	for(i = disallow_recruit.begin(); i != disallow_recruit.end(); ++i) {
		const int side = maximum<int>(1,atoi((**i)["side"].c_str()));
		const size_t index = side-1;
		if(index > teams->size())
			continue;

		const std::string& type = (**i)["type"];
		(*teams)[index].recruits().erase(type);
		if(index == 0) {
			state_of_game->can_recruit.erase(type);
		}
	}

	const config::child_list& set_recruit = cfg->get_children("set_recruit");
	for(i = set_recruit.begin(); i != set_recruit.end(); ++i) {
		const int side = maximum<int>(1,atoi((**i)["side"].c_str()));
		const size_t index = side-1;
		if(index > teams->size())
			continue;

		std::vector<std::string> recruit = config::split((**i)["recruit"]);
		if(recruit.size() == 1 && recruit.back() == "")
			recruit.clear();

		std::set<std::string>& can_recruit = (*teams)[index].recruits();
		can_recruit.clear();
		std::copy(recruit.begin(),recruit.end(),std::inserter(can_recruit,can_recruit.end()));
		if(index == 0) {
			state_of_game->can_recruit = can_recruit;
		}
	}

	//sounds
	const config::child_list& sounds = cfg->get_children("sound");
	for(i = sounds.begin(); i != sounds.end(); ++i) {
		sound::play_sound((**i)["name"]);
	}

	const config::child_list& colour_adjust = cfg->get_children("colour_adjust");
	for(i = colour_adjust.begin(); i != colour_adjust.end(); ++i) {
		const int r = atoi((**i)["red"].c_str());
		const int g = atoi((**i)["green"].c_str());
		const int b = atoi((**i)["blue"].c_str());
		screen->adjust_colours(r,g,b);
		screen->invalidate_all();
		screen->draw(true,true);
	}

	const config* delay = cfg->child("delay");
	if(delay != NULL) {
		const int delay_time = atoi((*delay)["time"].c_str());
		::SDL_Delay(delay_time);
	}

	const config* scroll = cfg->child("scroll");
	if(scroll != NULL) {
		const int xoff = atoi((*scroll)["x"].c_str());
		const int yoff = atoi((*scroll)["y"].c_str());
		screen->scroll(xoff,yoff);
		screen->draw(true,true);
	}

	const config* scroll_to = cfg->child("scroll_to_unit");
	if(scroll_to != NULL) {
		unit_map::const_iterator u;
		for(u = units->begin(); u != units->end(); ++u){
			if(u->second.matches_filter(*scroll_to))
				break;
		}

		if(u != units->end()) {
			screen->scroll_to_tile(u->first.x,u->first.y);
		}
	}

	//an award of gold to a particular side
	const config::child_list& gold = cfg->get_children("gold");
	for(i = gold.begin(); i != gold.end(); ++i) {
		const std::string& side = (**i)["side"];
		const std::string& amount = (**i)["amount"];
		const int side_num = side.empty() ? 1 : atoi(side.c_str());
		const int amount_num = atoi(amount.c_str());
		const size_t team_index = side_num-1;
		if(team_index < teams->size()) {
			(*teams)[team_index].spend_gold(-amount_num);
		}
	}

	//moving a 'unit' - i.e. a dummy unit that is just moving for
	//the visual effect
	const config::child_list& move_unit_fake = cfg->get_children("move_unit_fake");
	for(i = move_unit_fake.begin(); i != move_unit_fake.end(); ++i) {
		const std::string& type = (**i)["type"];
		const game_data::unit_type_map::const_iterator itor = game_data_ptr->unit_types.find(type);
		if(itor != game_data_ptr->unit_types.end()) {
			unit dummy_unit(&itor->second,0);
			const std::vector<std::string> xvals = config::split((**i)["x"]);
			const std::vector<std::string> yvals = config::split((**i)["y"]);
			std::vector<gamemap::location> path;
			for(size_t i = 0; i != minimum(xvals.size(),yvals.size()); ++i) {
				path.push_back(gamemap::location(atoi(xvals[i].c_str())-1,
				                                 atoi(yvals[i].c_str())-1));
			}

			screen->move_unit(path,dummy_unit);
		}
	}

	//setting a variable
	const config::child_list& set_vars = cfg->get_children("set_variable");
	for(i = set_vars.begin(); i != set_vars.end(); ++i) {

		const std::string& name = (**i)["name"];
		const std::string& value = (**i)["value"];
		if(value.empty() == false) {
			state_of_game->variables[name] = value;
		}

		const std::string& add = (**i)["add"];
		if(add.empty() == false) {
			double value = atof(state_of_game->variables[name].c_str());
			value += atof(add.c_str());
			char buf[50];
			sprintf(buf,"%f",value);
			state_of_game->variables[name] = buf;
		}

		const std::string& multiply = (**i)["multiply"];
		if(multiply.empty() == false) {
			double value = atof(state_of_game->variables[name].c_str());
			value *= atof(multiply.c_str());
			char buf[50];
			sprintf(buf,"%f",value);
			state_of_game->variables[name] = buf;
		}
	}

	//conditional statements
	const config::child_list& conditionals = cfg->get_children("if");
	for(i = conditionals.begin(); i != conditionals.end(); ++i) {
		const std::string type = game_events::conditional_passed(
		                     *state_of_game,units,**i) ? "then":"else";

		//if the if statement passed, then execute all 'then' statements,
		//otherwise execute 'else' statements
		const std::vector<config*>& commands = (*i)->get_children(type);
		for(std::vector<config*>::const_iterator cmd = commands.begin();
		    cmd != commands.end(); ++cmd) {
			handle_event(event_info,*cmd);
		}
	}

	//if we are assigning a role to a unit from the available units list
	const config::child_list& assign_role = cfg->get_children("role");
	for(i = assign_role.begin(); i != assign_role.end(); ++i) {

		//get a list of the types this unit can be
		std::vector<std::string> types = config::split((**i)["type"]);

		//iterate over all the types, and for each type, try to find
		//a unit that matches
		std::vector<std::string>::iterator ti;
		for(ti = types.begin(); ti != types.end(); ++ti) {
			config cfg;
			cfg.values["type"] = *ti;

			std::map<gamemap::location,unit>::iterator itor;
			for(itor = units->begin(); itor != units->end(); ++itor) {
				if(itor->second.matches_filter(cfg)) {
					itor->second.assign_role((**i)["role"]);
					break;
				}
			}

			if(itor != units->end())
				break;

			std::vector<unit>::iterator ui;
			//iterate over the units, and try to find one that matches
			for(ui = state_of_game->available_units.begin();
			    ui != state_of_game->available_units.end(); ++ui) {
				if(ui->matches_filter(cfg)) {
					ui->assign_role((**i)["role"]);
					break;
				}
			}

			//if we found a unit, we don't have to keep going.
			if(ui != state_of_game->available_units.end())
				break;
		}

		//if we found a unit in the current type, we don't have to
		//look through any more types
		if(ti != types.end())
			break;
	}

	const config::child_list& remove_overlays = cfg->get_children("removeitem");
	for(i = remove_overlays.begin(); i != remove_overlays.end(); ++i) {
		gamemap::location loc(**i);
		if(!loc.valid()) {
			loc = event_info.loc1;
		}

		screen->remove_overlay(loc);
	}

	//hiding units
	const config::child_list& hide = cfg->get_children("hide_unit");
	for(i = hide.begin(); i != hide.end(); ++i) {
		const gamemap::location loc(**i);
		screen->hide_unit(loc);
		screen->draw_tile(loc.x,loc.y);
	}

	if(cfg->child("unhide_unit") != NULL) {
		const gamemap::location loc = screen->hide_unit(gamemap::location());
		screen->draw_tile(loc.x,loc.y);
	}

	//adding new items
	const config::child_list& add_overlays = cfg->get_children("item");
	for(i = add_overlays.begin(); i != add_overlays.end(); ++i) {
		gamemap::location loc(**i);
		const std::string& img = (**i)["image"];
		if(!img.empty()) {
			screen->add_overlay(loc,img);
			screen->draw_tile(loc.x,loc.y);
		}
	}

	//changing the terrain
	const config::child_list& terrain_changes = cfg->get_children("terrain");
	for(i = terrain_changes.begin(); i != terrain_changes.end(); ++i) {
		const std::vector<gamemap::location> locs = multiple_locs(**i);

		for(std::vector<gamemap::location>::const_iterator loc = locs.begin(); loc != locs.end(); ++loc) {
			const std::string& terrain_type = (**i)["letter"];
			if(terrain_type.size() > 0) {
				game_map->set_terrain(*loc,terrain_type[0]);
				screen->recalculate_minimap();
				screen->invalidate_all();
			}
		}
	}

	//if we should spawn a new unit on the map somewhere
	const config::child_list& new_units = cfg->get_children("unit");
	for(i = new_units.begin(); i != new_units.end(); ++i) {
		unit new_unit(*game_data_ptr,**i);
		gamemap::location loc(**i);

		if(game_map->on_board(loc)) {
			loc = find_vacant_tile(*game_map,*units,loc);
			units->insert(std::pair<gamemap::location,unit>(loc,new_unit));
			screen->invalidate(loc);
		} else {
			state_of_game->available_units.push_back(new_unit);
		}
	}

	//if we should recall units that match a certain description
	const config::child_list& recalls = cfg->get_children("recall");
	for(i = recalls.begin(); i != recalls.end(); ++i) {
		std::vector<unit>& avail = state_of_game->available_units;
		for(std::vector<unit>::iterator u = avail.begin(); u != avail.end(); ++u) {
			if(u->matches_filter(**i)) {
				recruit_unit(*game_map,1,*units,*u,gamemap::location(),screen,false);
				avail.erase(u);
				break;
			}
		}
	}

	const config::child_list& objects = cfg->get_children("object");
	for(i = objects.begin(); i != objects.end(); ++i) {
		const config& values = **i;

		const std::string& id = values["id"];

		//if this item has already been used
		if(used_items.count(id))
			continue;

		const std::string image = values["image"];
		std::string caption = values["name"];

		const std::string& caption_lang = string_table[id + "_name"];
		if(caption_lang.empty() == false)
			caption = caption_lang;

		const std::map<gamemap::location,unit>::iterator u = units->find(event_info.loc1);

		if(u == units->end())
			continue;

		std::string text;

		const config::child_list& filters = (*i)->get_children("filter");

		if(filters.empty() || u->second.matches_filter(*filters[0])) {
			const std::string& lang = string_table[id];
			if(!lang.empty())
				text = lang;
			else
				text = values["description"];

			u->second.add_modification("object",**i);
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
				text = values["cannot_use_message"];
		}


		SDL_Surface* surface = NULL;
		if(image.empty() == false) {
			surface = image::get_image(image,image::UNSCALED);
		}

		gui::show_dialog(*screen,surface,caption,text);

		//this will redraw the unit, with its new stats
		screen->draw();
	}

	const config::child_list& messages = cfg->get_children("message");
	for(i = messages.begin(); i != messages.end(); ++i) {
		const config& values = **i;

		std::map<gamemap::location,unit>::iterator speaker = units->end();
		if(values["speaker"] == "unit") {
			speaker = units->find(event_info.loc1);
		} else if(values["speaker"] == "second_unit") {
			speaker = units->find(event_info.loc2);
		} else if(values["speaker"] != "narrator") {
			for(speaker = units->begin(); speaker != units->end();
			    ++speaker){
				if(speaker->second.matches_filter(**i))
					break;
			}

			if(speaker == units->end()) {
				//no matching unit found, so the dialog can't come up
				//continue onto the next message
				continue;
			}
		}

		const std::string& sfx = values["sound"];
		if(sfx != "") {
			sound::play_sound(sfx);
		}

		const std::string& id = values["id"];

		std::string image = values["image"];
		std::string caption;

		const std::string& lang_caption = string_table[id + "_caption"];
		if(!lang_caption.empty())
			caption = lang_caption;
		else
			caption = values["caption"];

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
		std::vector<std::vector<config*>*> option_events;

		std::cerr << "building menu items...\n";

		const std::vector<config*>& menu_items = (*i)->get_children("option");
		for(std::vector<config*>::const_iterator mi = menu_items.begin();
		    mi != menu_items.end(); ++mi) {
			const std::string& msg = translate_string_default((**mi)["id"],(**mi)["message"]);
			options.push_back(msg);
			option_events.push_back(&(*mi)->children["command"]);
		}

		SDL_Surface* surface = NULL;
		if(image.empty() == false) {
			surface = image::get_image(image,image::UNSCALED);
		}

		const std::string& lang_message = string_table[id];
		int option_chosen = gui::show_dialog(*screen,surface,caption,
		           lang_message.empty() ? values["message"] : lang_message,
		           options.empty() ? gui::MESSAGE : gui::OK_ONLY,
		           options.empty() ? NULL : &options);

		if(screen->update_locked() && options.empty() == false) {
			config* const cfg = recorder.get_next_action();
			if(cfg == NULL || cfg->children["choose"].empty()) {
				std::cerr << "choice expected but none found\n";
				throw replay::error();
			}

			if(size_t(option_chosen) >= options.size()) {
				option_chosen = 0;
			}

			const std::string& val = cfg->children["choose"].front()->values["value"];
			option_chosen = atol(val.c_str());

		} else if(options.empty() == false) {
			recorder.choose_option(option_chosen);
		}

		if(options.empty() == false) {
			assert(size_t(option_chosen) < menu_items.size());
			const std::vector<config*>& events = *option_events[option_chosen];
			for(std::vector<config*>::const_iterator ev = events.begin();
			    ev != events.end(); ++ev) {
				handle_event(event_info,*ev);
			}
		}
	}

	const config::child_list& dead_units = cfg->get_children("kill");
	for(i = dead_units.begin(); i != dead_units.end(); ++i) {

		for(std::map<gamemap::location,unit>::iterator un = units->begin();
		    un != units->end(); ++un) {
			while(un != units->end() && un->second.matches_filter(**i)) {
				units->erase(un);
				un = units->begin();
			}
		}

		std::vector<unit>& avail_units = state_of_game->available_units;
		for(std::vector<unit>::iterator j = avail_units.begin();
		    j != avail_units.end(); ++j) {
			while(j != avail_units.end() && j->matches_filter(**i)) {
				j = avail_units.erase(j);
			}
		}
	}

	//adding of new events
	const config::child_list& new_events = cfg->get_children("event");
	for(i = new_events.begin(); i != new_events.end(); ++i) {
		event_handler new_handler(*i);
		events_map.insert(std::pair<std::string,event_handler>(
		                             new_handler.name(),new_handler));
	}

	const config* const end_info = cfg->child("endlevel");
	if(end_info != NULL) {
		const std::string& next_scenario = (*end_info)["next_scenario"];
		if(next_scenario.empty() == false) {
			state_of_game->scenario = next_scenario;
		}

		const std::string& result = (*end_info)["result"];
		if(result.empty() || result == "victory") {
			const bool bonus = (*end_info)["bonus"] == "yes";
			throw end_level_exception(VICTORY,bonus);
		} else {
			throw end_level_exception(DEFEAT);
		}
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

bool filter_loc(const gamemap::location& loc, config& cfg)
{
	const std::string& xloc = cfg.values["x"];
	const std::string& yloc = cfg.values["y"];

	return filter_loc_impl(loc,xloc,yloc);
}

bool process_event(event_handler& handler, const queued_event& ev)
{
	if(handler.disabled())
		return false;

	std::map<gamemap::location,unit>::iterator unit1 = units->find(ev.loc1);
	std::map<gamemap::location,unit>::iterator unit2 = units->find(ev.loc2);

	std::vector<config*>& first_filters = handler.first_arg_filters();
	for(std::vector<config*>::iterator ffi = first_filters.begin();
	    ffi != first_filters.end(); ++ffi) {
		if(!filter_loc(ev.loc1,**ffi))
			return false;

		if(unit1 != units->end() && !unit1->second.matches_filter(**ffi)) {
			return false;
		}
	}

	std::vector<config*>& second_filters = handler.second_arg_filters();
	for(std::vector<config*>::iterator sfi = second_filters.begin();
	    sfi != second_filters.end(); ++sfi) {
		if(!filter_loc(ev.loc2,**sfi))
			return false;

		if(unit2 != units->end() && !unit2->second.matches_filter(**sfi)) {
			return false;
		}
	}

	//the event hasn't been filtered out, so execute the handler
	handler.handle_event(ev);

	if(handler.first_time_only())
		handler.disable();

	return true;
}

} //end anonymous namespace

namespace game_events {

manager::manager(config& cfg, display& gui_, gamemap& map_,
                 std::map<gamemap::location,unit>& units_,
                 std::vector<team>& teams_,
                 game_state& state_of_game_, game_data& game_data_)
{
	std::vector<config*>& events_list = cfg.children["event"];
	for(std::vector<config*>::iterator i = events_list.begin();
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
