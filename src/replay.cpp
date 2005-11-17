/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
#include "filesystem.hpp"
#include "game_config.hpp"
#include "game_events.hpp"
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

#define LOG_NW LOG_STREAM(info, network)
#define ERR_NW LOG_STREAM(err, network)

//functions to verify that the unit structure on both machines is identical
namespace {
	void verify(const unit_map& units, const config& cfg)
	{
		LOG_NW << "verifying unit structure...\n";

		const size_t nunits = atoi(cfg["num_units"].c_str());
		if(nunits != units.size()) {
			ERR_NW << "SYNC VERIFICATION FAILED: number of units from data source differ: "
			       << nunits << " according to data source. " << units.size() << " locally\n";

			std::set<gamemap::location> locs;
			const config::child_list& items = cfg.get_children("unit");
			for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
				const gamemap::location loc(**i);
				locs.insert(loc);

				if(units.count(loc) == 0) {
					ERR_NW << "data source says there is a unit at "
					       << loc << " but none found locally\n";
				}
			}

			for(unit_map::const_iterator j = units.begin(); j != units.end(); ++j) {
				if(locs.count(j->first) == 0) {
					ERR_NW << "local unit at " << j->first
					       << " but none in data source\n";
				}
			}

			if (!game_config::ignore_replay_errors) throw replay::error();
		}

		const config::child_list& items = cfg.get_children("unit");
		for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
			const gamemap::location loc(**i);
			const unit_map::const_iterator u = units.find(loc);
			if(u == units.end()) {
				ERR_NW << "SYNC VERIFICATION FAILED: data source says there is a '"
				       << (**i)["type"] << "' (side " << (**i)["side"] << ") at "
				       << loc << " but there is no local record of it\n";
				if (!game_config::ignore_replay_errors) throw replay::error();
			}

			config cfg;
			u->second.write(cfg);

			bool is_ok = true;
			static const std::string fields[] = {"type","hitpoints","experience","side",""};
			for(const std::string* str = fields; str->empty() == false; ++str) {
				if(cfg[*str] != (**i)[*str]) {
					ERR_NW << "ERROR IN FIELD '" << *str << "' for unit at "
					       << loc << " data source: '" << (**i)[*str]
					       << "' local: '" << cfg[*str] << "'\n";
					is_ok = false;
				}
			}

			if(!is_ok) {
				ERR_NW << "(SYNC VERIFICATION FAILED)\n";
				if (!game_config::ignore_replay_errors) throw replay::error();
			}
		}

		LOG_NW << "verification passed\n";
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

// FIXME: this one now has to be assigned with set_random_generator
// from play_level or similar.  We should surely hunt direct
// references to it from this very file and move it out of here.
replay recorder;

replay::replay() : pos_(0), current_(NULL), skip_(0)
{}

replay::replay(const config& cfg) : cfg_(cfg), pos_(0), current_(NULL), skip_(0)
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

void replay::set_skip(bool skip)
{
	skip_ = skip;
}

bool replay::is_skipping() const
{
	return at_end() == false && skip_;
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

void replay::add_start()
{
	config* const cmd = add_command();
	cmd->add_child("start");
}

void replay::add_recruit(int value, const gamemap::location& loc)
{
	config* const cmd = add_command();

	config val;

	char buf[100];
	snprintf(buf,sizeof(buf),"%d",value);
	val["value"] = buf;

	loc.write(val);

	cmd->add_child("recruit",val);
}

void replay::add_recall(int value, const gamemap::location& loc)
{
	config* const cmd = add_command();

	config val;

	char buf[100];
	snprintf(buf,sizeof(buf),"%d",value);
	val["value"] = buf;

	loc.write(val);

	cmd->add_child("recall",val);
}

void replay::add_disband(int value)
{
	config* const cmd = add_command();

	config val;

	char buf[100];
	snprintf(buf,sizeof(buf),"%d",value);
	val["value"] = buf;

	cmd->add_child("disband",val);
}

void replay::add_movement(const gamemap::location& a,const gamemap::location& b)
{
	add_pos("move",a,b);
}

void replay::add_attack(const gamemap::location& a, const gamemap::location& b, int weapon)
{
	add_pos("attack",a,b);
	char buf[100];
	snprintf(buf,sizeof(buf),"%d",weapon);
	current_->child("attack")->values["weapon"] = buf;
}

void replay::add_pos(const std::string& type,
                     const gamemap::location& a, const gamemap::location& b)
{
	config* const cmd = add_command();

	config move, src, dst;
	a.write(src);
	b.write(dst);

	move.add_child("source",src);
	move.add_child("destination",dst);
	cmd->add_child(type,move);
}

void replay::add_value(const std::string& type, int value)
{
	config* const cmd = add_command();

	config val;

	char buf[100];
	snprintf(buf,sizeof(buf),"%d",value);
	val["value"] = buf;

	cmd->add_child(type,val);
}

void replay::choose_option(int index)
{
	add_value("choose",index);
}

void replay::add_label(const std::string& text, const gamemap::location& loc)
{
	config* const cmd = add_command(false);

	(*cmd)["undo"] = "no";

	config val;

	loc.write(val);
	val["text"] = text;

	cmd->add_child("label",val);
}

void replay::add_rename(const std::string& name, const gamemap::location& loc)
{
	config* const cmd = add_command(false);
	(*cmd)["undo"] = "no";
	config val;
	loc.write(val);
	val["name"] = name;
	cmd->add_child("rename", val);
}

void replay::end_turn()
{
	config* const cmd = add_command();
	cmd->add_child("end_turn");
}

void replay::speak(const config& cfg)
{
	config* const cmd = add_command(false);
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
		current_ = NULL;
		set_random(NULL);
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

config* replay::add_command(bool update_random_context)
{
	pos_ = ncommands()+1;
	current_ = &cfg_.add_child("command");
	if(update_random_context)
		set_random(current_);

	return current_;
}

void replay::start_replay()
{
	pos_ = 0;
}

config* replay::get_next_action()
{
	if(pos_ >= commands().size())
		return NULL;

	LOG_NW << "up to replay action " << pos_ << "/" << commands().size() << "\n";

	current_ = commands()[pos_];
	set_random(current_);
	++pos_;
	return current_;
}

void replay::pre_replay()
{
	while(pos_ < commands().size() && commands()[pos_]->child("start") != NULL) {
		if(get_next_action() == NULL)
			return;
	}
}

bool replay::at_end() const
{
	return pos_ >= commands().size();
}

void replay::set_to_end()
{
	pos_ = commands().size();
	current_ = NULL;
	set_random(NULL);
}

void replay::clear()
{
	cfg_ = config();
	pos_ = 0;
	current_ = NULL;
	set_random(NULL);
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

	if (!replayer.is_skipping()){
		clear_shroud(disp,state,map,gameinfo,units,teams,team_num-1);
		disp.recalculate_minimap();
	}

	const set_random_generator generator_setter(&replayer);

	update_locker lock_update(disp.video(),replayer.is_skipping());

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
				ERR_NW << "promotion expected, but none found\n";
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
			replayer.set_skip(false);
			return false;
		}

		else if(cfg->child("start") != NULL) {
			//do nothing

		} else if((child = cfg->child("speak")) != NULL) {
			const std::string& team_name = (*child)["team_name"];
			if(team_name == "" || teams[disp.viewing_team()].team_name() == team_name) {
				if(preferences::message_bell()) {
					if(!replayer.is_skipping())
						sound::play_sound(game_config::sounds::receive_message);
				}

				const int side = lexical_cast_default<int>((*child)["side"].c_str(),1);
				if (!replayer.is_skipping()){
					disp.add_chat_message((*child)["description"],side,(*child)["message"],
										  team_name == "" ? display::MESSAGE_PUBLIC : display::MESSAGE_PRIVATE);
				}
			}
		} else if((child = cfg->child("label")) != NULL) {
			const gamemap::location loc(*child);
			const std::string& text = (*child)["text"];

			if (!replayer.is_skipping()){
				disp.labels().set_label(loc,text);
			}
		}

		else if((child = cfg->child("rename")) != NULL) {
			const gamemap::location loc(*child);
			const std::string& name = (*child)["name"];

			std::map<gamemap::location,unit>::iterator u = units.find(loc);

			if(u->second.unrenamable()) {
				ERR_NW << "renaming unrenamable unit " << u->second.name() << "\n";
				if (!game_config::ignore_replay_errors) throw replay::error();
			}

			u->second.rename(name);
		}

		//if there is an end turn directive
		else if(cfg->child("end_turn") != NULL) {
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
				ERR_NW << "recruitment index is illegal: " << val
				       << " while this side only has " << recruits.size()
				       << " units available for recruitment\n";
				throw replay::error();
			}

			std::set<std::string>::const_iterator itor = recruits.begin();
			std::advance(itor,val);
			const std::map<std::string,unit_type>::const_iterator u_type = gameinfo.unit_types.find(*itor);
			if(u_type == gameinfo.unit_types.end()) {
				ERR_NW << "recruiting illegal unit: '" << *itor << "'\n";
				throw replay::error();
			}

			unit new_unit(&(u_type->second),team_num,true);
			const std::string& res = recruit_unit(map,team_num,units,new_unit,loc);
			if(!res.empty()) {
				ERR_NW << "cannot recruit unit: " << res << "\n";
				if (!game_config::ignore_replay_errors) throw replay::error();
			}

			if(u_type->second.cost() > current_team.gold()) {
				ERR_NW << "unit '" << u_type->second.id() << "' is too expensive to recruit: "
				       << u_type->second.cost() << "/" << current_team.gold() << "\n";
				if (!game_config::ignore_replay_errors) throw replay::error();
			}
			LOG_NW << "recruit: team=" << team_num << " '" << u_type->second.id() << "' at (" << loc
			       << ") cost=" << u_type->second.cost() << " from gold=" << current_team.gold() << ' ';


			statistics::recruit_unit(new_unit);

			current_team.spend_gold(u_type->second.cost());
			LOG_NW << "-> " << (current_team.gold()) << "\n";
			fix_shroud = !replayer.is_skipping() && true;
}

		else if((child = cfg->child("recall")) != NULL) {
			player_info* player = state_of_game.get_player(current_team.save_id());
			if(player == NULL) {
				ERR_NW << "illegal recall\n";
				throw replay::error();
			}

			sort_units(player->available_units);

			const std::string& recall_num = (*child)["value"];
			const int val = atoi(recall_num.c_str());

			gamemap::location loc(*child);

			if(val >= 0 && val < int(player->available_units.size())) {
				statistics::recall_unit(player->available_units[val]);
				recruit_unit(map,team_num,units,player->available_units[val],loc);
				player->available_units.erase(player->available_units.begin()+val);
				current_team.spend_gold(game_config::recall_cost);
			} else {
				ERR_NW << "illegal recall\n";
				if (!game_config::ignore_replay_errors) throw replay::error();
			}
			fix_shroud = !replayer.is_skipping() && true;
		}

		else if((child = cfg->child("disband")) != NULL) {
			player_info* const player = state_of_game.get_player(current_team.save_id());
			if(player == NULL) {
				ERR_NW << "illegal disband\n";
				throw replay::error();
			}

			sort_units(player->available_units);
			const std::string& unit_num = (*child)["value"];
			const int val = atoi(unit_num.c_str());

			if(val >= 0 && val < int(player->available_units.size())) {
				player->available_units.erase(player->available_units.begin()+val);
			} else {
				ERR_NW << "illegal disband\n";
				if (!game_config::ignore_replay_errors) throw replay::error();
			}
		}

		else if((child = cfg->child("move")) != NULL) {

			const config* const destination = child->child("destination");
			const config* const source = child->child("source");

			if(destination == NULL || source == NULL) {
				ERR_NW << "no destination/source found in movement\n";
				throw replay::error();
			}

			const gamemap::location src(*source);
			const gamemap::location dst(*destination);

			std::map<gamemap::location,unit>::iterator u = units.find(dst);
			if(u != units.end()) {
				ERR_NW << "destination already occupied: "
				       << dst << '\n';
				if (!game_config::ignore_replay_errors) throw replay::error();
			}
			u = units.find(src);
			if(u == units.end()) {
				ERR_NW << "unfound location for source of movement: "
				       << src << " -> " << dst << '\n';
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
					ERR_NW << "can get to: " << rt->first << '\n';
				}

				ERR_NW << "src cannot get to dst: " << current_unit.movement_left() << ' '
				       << paths_list.routes.size() << ' ' << src << " -> " << dst << '\n';
				if (!game_config::ignore_replay_errors) throw replay::error();
			}

			rt->second.steps.push_back(dst);

			if(!replayer.is_skipping() && unit_display::unit_visible_on_path(disp,map,rt->second.steps,current_unit,state.get_time_of_day(),units,teams)) {
				disp.set_paths(&paths_list);

				disp.scroll_to_tiles(src.x,src.y,dst.x,dst.y);
			}

			units.erase(u);

			if(!replayer.is_skipping()) {
				unit_display::move_unit(disp,map,rt->second.steps,current_unit,state.get_time_of_day(),units,teams);
			}
			else{
				//unit location needs to be updated
				current_unit.set_goto(*(rt->second.steps.end() - 1));
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

			if(!replayer.is_skipping()) {
				disp.draw_tile(dst.x,dst.y);
				disp.update_display();
			}

			game_events::fire("moveto",dst);
			//FIXME: what's special about team 1?
			if(team_num != 1 && (teams.front().uses_shroud() || teams.front().uses_fog()) && !teams.front().fogged(dst.x,dst.y)) {
				game_events::fire("sighted",dst);
			}

			fix_shroud = !replayer.is_skipping() && true;
		}

		else if((child = cfg->child("attack")) != NULL) {
			const config* const destination = child->child("destination");
			const config* const source = child->child("source");

			if(destination == NULL || source == NULL) {
				ERR_NW << "no destination/source found in attack\n";
				throw replay::error();
			}

			const gamemap::location src(*source);
			const gamemap::location dst(*destination);

			const std::string& weapon = (*child)["weapon"];
			const int weapon_num = atoi(weapon.c_str());

			std::map<gamemap::location,unit>::iterator u = units.find(src);
			if(u == units.end()) {
				ERR_NW << "unfound location for source of attack\n";
				throw replay::error();
			}

			if(size_t(weapon_num) >= u->second.attacks().size()) {
				ERR_NW << "illegal weapon type in attack\n";
				if (!game_config::ignore_replay_errors) throw replay::error();
			}

			std::map<gamemap::location,unit>::const_iterator tgt = units.find(dst);

			if(tgt == units.end()) {
				ERR_NW << "unfound defender for attack: " << src << " -> " << dst << '\n';
				if (!game_config::ignore_replay_errors) throw replay::error();
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
			fix_shroud = !replayer.is_skipping() && true;
		} else {
			ERR_NW << "unrecognized action\n";
			if (!game_config::ignore_replay_errors) throw replay::error();
		}

		//Check if we should refresh the shroud, and redraw the minimap/map tiles.
		//This is needed for shared vision to work properly.
		if(fix_shroud && clear_shroud(disp,state,map,gameinfo,units,teams,team_num-1) && !recorder.is_skipping()) {
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
