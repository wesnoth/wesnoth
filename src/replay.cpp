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

#include "global.hpp"

#include "actions.hpp"
#include "ai_interface.hpp"
#include "dialogs.hpp"
#include "game_config.hpp"
#include "log.hpp"
#include "network.hpp"
#include "pathfind.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "show_dialog.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "unit_display.hpp"
#include "util.hpp"
#include "wassert.hpp"

#include <cstdio>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <set>
#include <sstream>

//functions to verify that the unit structure on both machines is identical
namespace {
	void verify(const unit_map& units, const config& cfg)
	{
		std::cerr << "verifying unit structure...\n";

		const size_t nunits = atoi(cfg["num_units"].c_str());
		if(nunits != units.size()) {
			std::cerr << "SYNC VERIFICATION FAILED: number of units from data source differ: "
			          << nunits << " according to data source. " << units.size() << " locally\n";

			std::set<gamemap::location> locs;
			const config::child_list& items = cfg.get_children("unit");
			for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
				const gamemap::location loc(**i);
				locs.insert(loc);

				if(units.count(loc) == 0) {
					std::cerr << "data source says there is a unit at " << (loc.x+1) << "," << (loc.y+1) << " but none found locally\n";
				}
			}

			for(unit_map::const_iterator j = units.begin(); j != units.end(); ++j) {
				if(locs.count(j->first) == 0) {
					std::cerr << "local unit at " << (j->first.x+1) << "," << (j->first.y+1) << " but none in data source\n";
				}
			}

			throw replay::error();
		}

		const config::child_list& items = cfg.get_children("unit");
		for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
			const gamemap::location loc(**i);
			const unit_map::const_iterator u = units.find(loc);
			if(u == units.end()) {
				std::cerr << "SYNC VERIFICATION FAILED: data source says there is a '"
				          << (**i)["type"] << "' (side " << (**i)["side"] << ") at " << (**i)["x"] << "," << (**i)["y"]
						  << " but there is no local record of it\n";
				throw replay::error();
			}

			config cfg;
			u->second.write(cfg);

			bool is_ok = true;
			static const std::string fields[] = {"type","hitpoints","experience","side",""};
			for(const std::string* str = fields; str->empty() == false; ++str) {
				if(cfg[*str] != (**i)[*str]) {
					std::cerr << "ERROR IN FIELD '" << *str << "' for unit at " << (**i)["x"] << "," << (**i)["y"]
						      << " data source: '" << (**i)[*str] << "' local: '" << cfg[*str] << "'\n";
					is_ok = false;
				}
			}

			if(!is_ok) {
				std::cerr << "(SYNC VERIFICATION FAILED)\n";
				throw replay::error();
			}
		}

		std::cerr << "verification passed\n";
	}

	config create_verification(const unit_map& units)
	{
		config res;
		std::stringstream buf;
		buf << units.size();
		res["num_units"] = buf.str();

		for(unit_map::const_iterator i = units.begin(); i != units.end(); ++i) {
			config u;
			i->first.write(u);

			static const std::string fields[] = {"type","hitpoints","experience","side",""};
			config tmp;
			i->second.write(tmp);
			for(const std::string* f = fields; f->empty() == false; ++f) {
				u[*f] = tmp[*f];
			}

			res.add_child("unit",u);
		}

		return res;
	}

	const unit_map* unit_map_ref = NULL;

	void verify_units(const config& cfg)
	{
		if(unit_map_ref != NULL) {
			verify(*unit_map_ref,cfg);
		}
	}

	config make_verify_units()
	{
		return create_verification(*unit_map_ref);
	}
}

verification_manager::verification_manager(const unit_map& units)
{
	unit_map_ref = &units;
}

verification_manager::~verification_manager()
{
	unit_map_ref = NULL;
}

replay recorder;

namespace {

replay* random_generator = &recorder;

struct set_random_generator {

	set_random_generator(replay* r) : old_(random_generator)
	{
		random_generator = r;
	}

	~set_random_generator()
	{
		random_generator = old_;
	}

private:
	replay* old_;
};

}

int get_random()
{
	return random_generator->get_random();
}

const config* get_random_results()
{
	return random_generator->get_random_results();
}

void set_random_results(const config& cfg)
{
	random_generator->set_random_results(cfg);
}

replay::replay() : pos_(0), current_(NULL), random_(NULL), skip_(0)
{}

replay::replay(const config& cfg) : cfg_(cfg), pos_(0), current_(NULL), random_(NULL), skip_(0)
{}

config& replay::get_config()
{
	return cfg_;
}

void replay::set_save_info(const game_state& save)
{
	saveInfo_ = save;
}

const game_state& replay::get_save_info() const
{
	return saveInfo_;
}

void replay::set_skip(int turns_to_skip)
{
	skip_ = turns_to_skip;
}

void replay::next_skip()
{
	if(skip_ > 0)
		--skip_;
}

bool replay::skipping() const
{
	return at_end() == false && skip_ != 0;
}

void replay::save_game(const std::string& label, const config& snapshot,
                       const config& starting_pos, bool include_replay)
{
	log_scope("replay::save_game");
	saveInfo_.snapshot = snapshot;
	saveInfo_.starting_pos = starting_pos;

	if(include_replay) {
		saveInfo_.replay_data = cfg_;
	} else {
		saveInfo_.replay_data = config();
	}

	saveInfo_.label = label;

	::save_game(saveInfo_);

	saveInfo_.replay_data = config();
	saveInfo_.snapshot = config();
}

void replay::add_recruit(int value, const gamemap::location& loc)
{
	config* const cmd = add_command();

	config val;

	char buf[100];
	sprintf(buf,"%d",value);
	val["value"] = buf;

	sprintf(buf,"%d",loc.x+1);
	val["x"] = buf;

	sprintf(buf,"%d",loc.y+1);
	val["y"] = buf;

	cmd->add_child("recruit",val);

	random_ = cmd;
}

void replay::add_recall(int value, const gamemap::location& loc)
{
	config* const cmd = add_command();

	config val;

	char buf[100];
	sprintf(buf,"%d",value);
	val["value"] = buf;

	sprintf(buf,"%d",loc.x+1);
	val["x"] = buf;

	sprintf(buf,"%d",loc.y+1);
	val["y"] = buf;

	cmd->add_child("recall",val);
}

void replay::add_disband(int value)
{
	config* const cmd = add_command();

	config val;

	char buf[100];
	sprintf(buf,"%d",value);
	val["value"] = buf;

	cmd->add_child("disband",val);
}

void replay::add_movement(const gamemap::location& a,const gamemap::location& b)
{
	add_pos("move",a,b);
	//current_->add_child("verify",make_verify_units());
	current_ = NULL;
	random_ = NULL;
}

void replay::add_attack(const gamemap::location& a, const gamemap::location& b, int weapon)
{
	add_pos("attack",a,b);
	char buf[100];
	sprintf(buf,"%d",weapon);
	current_->child("attack")->values["weapon"] = buf;
	random_ = current_;
}

void replay::add_pos(const std::string& type,
                     const gamemap::location& a, const gamemap::location& b)
{
	config* const cmd = add_command();

	config move, src, dst;

	char buf[100];
	sprintf(buf,"%d",a.x+1);
	src["x"] = buf;
	sprintf(buf,"%d",a.y+1);
	src["y"] = buf;
	sprintf(buf,"%d",b.x+1);
	dst["x"] = buf;
	sprintf(buf,"%d",b.y+1);
	dst["y"] = buf;

	move.add_child("source",src);
	move.add_child("destination",dst);
	cmd->add_child(type,move);

	current_ = cmd;
}

void replay::add_value(const std::string& type, int value)
{
	config* const cmd = add_command();

	config val;

	char buf[100];
	sprintf(buf,"%d",value);
	val["value"] = buf;

	cmd->add_child(type,val);
}

void replay::choose_option(int index)
{
	add_value("choose",index);
}

void replay::add_label(const std::string& text, const gamemap::location& loc)
{
	config* const cmd = add_command();

	(*cmd)["undo"] = "no";

	config val;

	loc.write(val);
	val["text"] = text;

	cmd->add_child("label",val);
}

void replay::end_turn()
{
	config* const cmd = add_command();
	cmd->add_child("end_turn");
	//cmd->add_child("verify",make_verify_units());
}

void replay::speak(const config& cfg)
{
	config* const cmd = add_command();
	if(cmd != NULL) {
		cmd->add_child("speak",cfg);
		(*cmd)["undo"] = "no";
	}
}

std::string replay::build_chat_log(const std::string& team) const
{
	std::stringstream str;
	const config::child_list& cmd = commands();
	for(config::child_list::const_iterator i = cmd.begin(); i != cmd.end(); ++i) {
		const config* speak = (**i).child("speak");
		if(speak != NULL) {
			const config& cfg = *speak;
			const std::string& team_name = cfg["team_name"];
			if(team_name == "" || team_name == team) {
				if(team_name == "") {
					str << "<" << cfg["description"] << "> ";
				} else {
					str << "*" << cfg["description"] << "* ";
				}

				str << cfg["message"] << "\n";
			}
		}
	}

	return str.str();
}

config replay::get_data_range(int cmd_start, int cmd_end, DATA_TYPE data_type)
{
	config res;

	const config::child_list& cmd = commands();
	while(cmd_start < cmd_end) {
		if((data_type == ALL_DATA || (*cmd[cmd_start])["undo"] == "no") && (*cmd[cmd_start])["sent"] != "yes") {
			res.add_child("command",*cmd[cmd_start]);

			if(data_type == NON_UNDO_DATA) {
				(*cmd[cmd_start])["sent"] = "yes";
			}
		}

		++cmd_start;
	}

	return res;
}

void replay::undo()
{
	config::child_itors cmd = cfg_.child_range("command");
	while(cmd.first != cmd.second && (**(cmd.second-1))["undo"] == "no") {
		--cmd.second;
	}

	if(cmd.first != cmd.second) {
		cfg_.remove_child("command",cmd.second - cmd.first - 1);
		current_ = random_ = NULL;
	}
}

const config::child_list& replay::commands() const
{
	return cfg_.get_children("command");
}

int replay::ncommands()
{
	return commands().size();
}

void replay::mark_current()
{
	if(current_ != NULL) {
		(*current_)["mark"] = "yes";
	}
}

config* replay::add_command()
{
	pos_ = ncommands()+1;
	return current_ = &cfg_.add_child("command");
}

int replay::get_random()
{
	if(random_ == NULL) {
		return rand();
	}

	//random numbers are in a 'list' meaning that each random
	//number contains another random numbers unless it's at
	//the end of the list. Generating a new random number means
	//nesting a new node inside the current node, and making
	//the current node the new node
	config* const random = random_->child("random");
	if(random == NULL) {
		const int res = rand();
		random_ = &random_->add_child("random");

		char buf[100];
		sprintf(buf,"%d",res);
		(*random_)["value"] = buf;

		return res;
	} else {
		const int res = atol((*random)["value"].c_str());
		random_ = random;
		return res;
	}
}

const config* replay::get_random_results() const
{
	wassert(random_ != NULL);
	return random_->child("results");
}

void replay::set_random_results(const config& cfg)
{
	wassert(random_ != NULL);
	random_->clear_children("results");
	random_->add_child("results",cfg);
}

void replay::start_replay()
{
	pos_ = 0;
}

config* replay::get_next_action()
{
	if(pos_ >= commands().size())
		return NULL;

	std::cerr << "up to replay action " << pos_ << "/" << commands().size() << "\n";

	random_ = current_ = commands()[pos_];
	++pos_;
	return current_;
}

bool replay::at_end() const
{
	return pos_ >= commands().size();
}

void replay::set_to_end()
{
	pos_ = commands().size();
	current_ = NULL;
	random_ = NULL;
}

void replay::clear()
{
	cfg_ = config();
	pos_ = 0;
	current_ = NULL;
	random_ = NULL;
	skip_ = 0;
}

bool replay::empty()
{
	return commands().empty();
}

void replay::add_config(const config& cfg, MARK_SENT mark)
{
	for(config::const_child_itors i = cfg.child_range("command"); i.first != i.second; ++i.first) {
		config& cfg = cfg_.add_child("command",**i.first);
		if(mark == MARK_AS_SENT) {
			cfg["sent"] = "yes";
		}
	}
}

namespace {
replay* replay_src = NULL;

struct replay_source_manager
{
	replay_source_manager(replay* o) : old_(replay_src)
	{
		replay_src = o;
	}

	~replay_source_manager()
	{
		replay_src = old_;
	}

private:
	replay* const old_;
};

}

replay& get_replay_source()
{
	if(replay_src != NULL) {
		return *replay_src;
	} else {
		return recorder;
	}
}

bool do_replay(display& disp, const gamemap& map, const game_data& gameinfo,
               unit_map& units,
			   std::vector<team>& teams, int team_num, const gamestatus& state,
			   game_state& state_of_game, replay* obj)
{
	log_scope("do replay");

	const replay_source_manager replay_manager(obj);

	replay& replayer = (obj != NULL) ? *obj : recorder;

	clear_shroud(disp,state,map,gameinfo,units,teams,team_num-1);
	disp.recalculate_minimap();

	const set_random_generator generator_setter(&replayer);

	update_locker lock_update(disp,replayer.skipping());

	//a list of units that have promoted from the last attack
	std::deque<gamemap::location> advancing_units;

	team& current_team = teams[team_num-1];

	for(;;) {
		config* const cfg = replayer.get_next_action();
		config* child;
		
		//do we need to recalculate shroud after this action is processed?
		bool fix_shroud = false;

		//if we are expecting promotions here
		if(advancing_units.empty() == false) {
			if(cfg == NULL) {
				std::cerr << "promotion expected, but none found\n";
				throw replay::error();
			}

			//if there is a promotion, we process it and go onto the next command
			//but if this isn't a promotion, we just keep waiting for the promotion
			//command -- it may have been mixed up with other commands such as messages
			if((child = cfg->child("choose")) != NULL) {

				const int val = lexical_cast_default<int>((*child)["value"]);

				dialogs::animate_unit_advancement(gameinfo,units,advancing_units.front(),disp,val);

				advancing_units.pop_front();

				//if there are no more advancing units, then we check for victory,
				//in case the battle that led to advancement caused the end of scenario
				if(advancing_units.empty()) {
					check_victory(units,teams);
				}

				continue;
			}
		}

		//if there is nothing more in the records
		if(cfg == NULL) {
			replayer.set_skip(0);
			return false;
		}

		else if((child = cfg->child("speak")) != NULL) {
			const std::string& team_name = (*child)["team_name"];
			if(team_name == "" || teams[disp.viewing_team()].team_name() == team_name) {
				if(preferences::message_bell()) {
					if(!replayer.skipping())
						sound::play_sound(game_config::sounds::receive_message);
				}

				const int side = lexical_cast_default<int>((*child)["side"].c_str());
				disp.add_chat_message((*child)["description"],side,(*child)["message"],
				                      team_name == "" ? display::MESSAGE_PUBLIC : display::MESSAGE_PRIVATE);
			}
		} else if((child = cfg->child("label")) != NULL) {
			const gamemap::location loc(*child);
			const std::string& text = (*child)["text"];

			disp.labels().set_label(loc,text);
		} 

		//if there is an end turn directive
		else if(cfg->child("end_turn") != NULL) {
			replayer.next_skip();

			child = cfg->child("verify");
			if(child != NULL) {
				verify_units(*child);
			}

			return true;
		}

		else if((child = cfg->child("recruit")) != NULL) {
			const std::string& recruit_num = (*child)["value"];
			const int val = atoi(recruit_num.c_str());

			gamemap::location loc(*child);

			const std::set<std::string>& recruits = current_team.recruits();

			if(val < 0 || val >= recruits.size()) {
				std::cerr << "recruitment index is illegal: " << val
				          << " while this side only has " << recruits.size() << " units available for recruitment\n";
				throw replay::error();
			}

			std::set<std::string>::const_iterator itor = recruits.begin();
			std::advance(itor,val);
			const std::map<std::string,unit_type>::const_iterator u_type = gameinfo.unit_types.find(*itor);
			if(u_type == gameinfo.unit_types.end()) {
				std::cerr << "recruiting illegal unit: '" << *itor << "'\n";
				throw replay::error();
			}

			unit new_unit(&(u_type->second),team_num,true);
			const std::string& res = recruit_unit(map,team_num,units,new_unit,loc);
			if(!res.empty()) {
				std::cerr << "cannot recruit unit: " << res << "\n";
				throw replay::error();
			}

			if(u_type->second.cost() > current_team.gold()) {
				std::cerr << "unit '" << u_type->second.name() << "' is too expensive to recruit: "
				          << u_type->second.cost() << "/" << current_team.gold() << "\n";
				throw replay::error();
			}

			statistics::recruit_unit(new_unit);

			current_team.spend_gold(u_type->second.cost());
			fix_shroud = true;
		}

		else if((child = cfg->child("recall")) != NULL) {
			player_info* player = state_of_game.get_player(current_team.save_id());
			if(player == NULL) {
				std::cerr << "illegal recall\n";
				throw replay::error();
			}

			std::sort(player->available_units.begin(),player->available_units.end(),compare_unit_values());

			const std::string& recall_num = (*child)["value"];
			const int val = atoi(recall_num.c_str());

			gamemap::location loc(*child);

			if(val >= 0 && val < int(player->available_units.size())) {
				statistics::recall_unit(player->available_units[val]);
				recruit_unit(map,team_num,units,player->available_units[val],loc);
				player->available_units.erase(player->available_units.begin()+val);
				current_team.spend_gold(game_config::recall_cost);
			} else {
				std::cerr << "illegal recall\n";
				throw replay::error();
			}
			fix_shroud = true;
		}

		else if((child = cfg->child("disband")) != NULL) {
			player_info* const player = state_of_game.get_player(current_team.save_id());
			if(player == NULL) {
				std::cerr << "illegal disband\n";
				throw replay::error();
			}

			std::sort(player->available_units.begin(),player->available_units.end(),compare_unit_values());
			const std::string& unit_num = (*child)["value"];
			const int val = atoi(unit_num.c_str());

			if(val >= 0 && val < int(player->available_units.size())) {
				player->available_units.erase(player->available_units.begin()+val);
			} else {
				std::cerr << "illegal disband\n";
				throw replay::error();
			}
		}

		else if((child = cfg->child("move")) != NULL) {

			const config* const destination = child->child("destination");
			const config* const source = child->child("source");

			if(destination == NULL || source == NULL) {
				std::cerr << "no destination/source found in movement\n";
				throw replay::error();
			}

			const gamemap::location src(*source);
			const gamemap::location dst(*destination);

			std::map<gamemap::location,unit>::iterator u = units.find(dst);
			if(u != units.end()) {
				std::cerr << "destination already occupied: "
						  << (dst.x+1) << "," << (dst.y+1) << "\n";
				throw replay::error();
			}
			u = units.find(src);
			if(u == units.end()) {
				std::cerr << "unfound location for source of movement: "
				          << (src.x+1) << "," << (src.y+1) << "-"
						  << (dst.x+1) << "," << (dst.y+1) << "\n";
				throw replay::error();
			}

			const bool ignore_zocs = u->second.type().is_skirmisher();
			const bool teleport = u->second.type().teleports();

			paths paths_list(map,state,gameinfo,units,src,teams,ignore_zocs,teleport);
			paths_wiper wiper(disp);

			unit current_unit = u->second;

			std::map<gamemap::location,paths::route>::iterator rt = paths_list.routes.find(dst);
			if(rt == paths_list.routes.end()) {

				for(rt = paths_list.routes.begin(); rt != paths_list.routes.end(); ++rt) {
					std::cerr << "can get to: " << (rt->first.x+1) << "," << (rt->first.y+1) << "\n";
				}

				std::cerr << "src cannot get to dst: " << current_unit.movement_left() << " "
				          << paths_list.routes.size() << " " << (src.x+1)
				          << "," << (src.y+1) << "-" << (dst.x+1) << ","
				          << (dst.y+1) << "\n";
				throw replay::error();
			}

			rt->second.steps.push_back(dst);

			if(!replayer.skipping() && unit_display::unit_visible_on_path(disp,map,rt->second.steps,current_unit,state.get_time_of_day(),units,teams)) {
				disp.set_paths(&paths_list);

				disp.scroll_to_tiles(src.x,src.y,dst.x,dst.y);
			}

			units.erase(u);

			if(!replayer.skipping()) {
				unit_display::move_unit(disp,map,rt->second.steps,current_unit,state.get_time_of_day(),units,teams);
			}

			current_unit.set_movement(rt->second.move_left);
			u = units.insert(std::pair<gamemap::location,unit>(dst,current_unit)).first;
			if(map.is_village(dst)) {
				const int orig_owner = village_owner(dst,teams) + 1;
				if(orig_owner != team_num) {
					u->second.set_movement(0);
					get_village(dst,teams,team_num-1,units);
				}
			}

			if(!replayer.skipping()) {
				disp.draw_tile(dst.x,dst.y);
				disp.update_display();
			}

			game_events::fire("moveto",dst);
			if(team_num != 1 && (teams.front().uses_shroud() || teams.front().uses_fog()) && !teams.front().fogged(dst.x,dst.y)) {
				game_events::fire("sighted",dst);
			}

			fix_shroud = true;
		}

		else if((child = cfg->child("attack")) != NULL) {

			const config* const destination = child->child("destination");
			const config* const source = child->child("source");

			if(destination == NULL || source == NULL) {
				std::cerr << "no destination/source found in attack\n";
				throw replay::error();
			}

			const gamemap::location src(*source);
			const gamemap::location dst(*destination);

			const std::string& weapon = (*child)["weapon"];
			const int weapon_num = atoi(weapon.c_str());

			std::map<gamemap::location,unit>::iterator u = units.find(src);
			if(u == units.end()) {
				std::cerr << "unfound location for source of attack\n";
				throw replay::error();
			}

			if(size_t(weapon_num) >= u->second.attacks().size()) {
				std::cerr << "illegal weapon type in attack\n";
				throw replay::error();
			}

			std::map<gamemap::location,unit>::const_iterator tgt = units.find(dst);

			if(tgt == units.end()) {
				std::cerr << "unfound defender for attack: "
				          << (src.x+1) << "," << (src.y+1) << " -> "
						  << (dst.x+1) << "," << (dst.y+1) << "\n";
				throw replay::error();
			}

			game_events::fire("attack",src,dst);

			u = units.find(src);
			tgt = units.find(dst);

			if(u != units.end() && tgt != units.end()) {
				attack(disp, map, teams, src, dst, weapon_num, units, state, gameinfo);
			}

			u = units.find(src);
			tgt = units.find(dst);

			if(u != units.end() && u->second.advances()) {
				advancing_units.push_back(u->first);
			}

			if(tgt != units.end() && tgt->second.advances()) {
				advancing_units.push_back(tgt->first);
			}

			//check victory now if we don't have any advancements. If we do have advancements,
			//we don't check until the advancements are processed.
			if(advancing_units.empty()) {
				check_victory(units,teams);
			}
			fix_shroud = true;
		} else {
			std::cerr << "unrecognized action: '" << cfg->write() << "'\n";
			throw replay::error();
		}

		//Check if we should refresh the shroud, and redraw the minimap/map tiles.
		//This is needed for shared vision to work properly.
		if(fix_shroud && clear_shroud(disp,state,map,gameinfo,units,teams,team_num-1)) {
			disp.recalculate_minimap();
			disp.invalidate_all();
		}

		child = cfg->child("verify");
		if(child != NULL) {
			verify_units(*child);
		}
	}

	return false; /* Never attained, but silent a gcc warning. --Zas */
}

replay_network_sender::replay_network_sender(replay& obj) : obj_(obj), upto_(obj_.ncommands())
{
}

replay_network_sender::~replay_network_sender()
{
	commit_and_sync();
}

void replay_network_sender::sync_non_undoable()
{
	if(network::nconnections() > 0) {
		config cfg;
		const config& data = cfg.add_child("turn",obj_.get_data_range(upto_,obj_.ncommands(),replay::NON_UNDO_DATA));
		if(data.empty() == false) {
			network::send_data(cfg);
		}
	}
}

void replay_network_sender::commit_and_sync()
{
	if(network::nconnections() > 0) {
		config cfg;
		const config& data = cfg.add_child("turn",obj_.get_data_range(upto_,obj_.ncommands()));
		if(data.empty() == false) {
			network::send_data(cfg);
		}

		upto_ = obj_.ncommands();
	}
}
