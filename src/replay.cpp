/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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
 *  Replay control code.
 *
 *  See http://www.wesnoth.org/wiki/ReplayWML for more info.
 */

#include "global.hpp"

#include "ai/manager.hpp"
#include "actions/attack.hpp"
#include "actions/create.hpp"
#include "actions/move.hpp"
#include "actions/undo.hpp"
#include "dialogs.hpp"
#include "game_display.hpp"
#include "game_end_exceptions.hpp"
#include "game_events/pump.hpp"
#include "game_preferences.hpp"
#include "gamestatus.hpp"
#include "log.hpp"
#include "map_label.hpp"
#include "map_location.hpp"
#include "play_controller.hpp"
#include "synced_context.hpp"
#include "replay.hpp"
#include "resources.hpp"
#include "statistics.hpp"
#include "whiteboard/manager.hpp"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <set>
#include <map>

static lg::log_domain log_replay("replay");
#define DBG_REPLAY LOG_STREAM(debug, log_replay)
#define LOG_REPLAY LOG_STREAM(info, log_replay)
#define WRN_REPLAY LOG_STREAM(warn, log_replay)
#define ERR_REPLAY LOG_STREAM(err, log_replay)

static lg::log_domain log_random("random");
#define DBG_RND LOG_STREAM(debug, log_random)
#define LOG_RND LOG_STREAM(info, log_random)
#define WRN_RND LOG_STREAM(warn, log_random)
#define ERR_RND LOG_STREAM(err, log_random)


//functions to verify that the unit structure on both machines is identical

static void verify(const unit_map& units, const config& cfg) {
	std::stringstream errbuf;
	LOG_REPLAY << "verifying unit structure...\n";

	const size_t nunits = cfg["num_units"].to_size_t();
	if(nunits != units.size()) {
		errbuf << "SYNC VERIFICATION FAILED: number of units from data source differ: "
			   << nunits << " according to data source. " << units.size() << " locally\n";

		std::set<map_location> locs;
		BOOST_FOREACH(const config &u, cfg.child_range("unit"))
		{
			const map_location loc(u, resources::gamedata);
			locs.insert(loc);

			if(units.count(loc) == 0) {
				errbuf << "data source says there is a unit at "
					   << loc << " but none found locally\n";
			}
		}

		for(unit_map::const_iterator j = units.begin(); j != units.end(); ++j) {
			if (locs.count(j->get_location()) == 0) {
				errbuf << "local unit at " << j->get_location()
					   << " but none in data source\n";
			}
		}
		replay::process_error(errbuf.str());
		errbuf.clear();
	}

	BOOST_FOREACH(const config &un, cfg.child_range("unit"))
	{
		const map_location loc(un, resources::gamedata);
		const unit_map::const_iterator u = units.find(loc);
		if(u == units.end()) {
			errbuf << "SYNC VERIFICATION FAILED: data source says there is a '"
				   << un["type"] << "' (side " << un["side"] << ") at "
				   << loc << " but there is no local record of it\n";
			replay::process_error(errbuf.str());
			errbuf.clear();
		}

		config cfg;
		u->write(cfg);

		bool is_ok = true;
		static const std::string fields[] = {"type","hitpoints","experience","side",""};
		for(const std::string* str = fields; str->empty() == false; ++str) {
			if (cfg[*str] != un[*str]) {
				errbuf << "ERROR IN FIELD '" << *str << "' for unit at "
					   << loc << " data source: '" << un[*str]
					   << "' local: '" << cfg[*str] << "'\n";
				is_ok = false;
			}
		}

		if(!is_ok) {
			errbuf << "(SYNC VERIFICATION FAILED)\n";
			replay::process_error(errbuf.str());
			errbuf.clear();
		}
	}

	LOG_REPLAY << "verification passed\n";
}

static time_t get_time(const config &speak)
{
	time_t time;
	if (!speak["time"].empty())
	{
		std::stringstream ss(speak["time"].str());
		ss >> time;
	}
	else
	{
		//fallback in case sender uses wesnoth that doesn't send timestamps
		time = ::time(NULL);
	}
	return time;
}

// FIXME: this one now has to be assigned with set_random_generator
// from play_level or similar.  We should surely hunt direct
// references to it from this very file and move it out of here.
replay recorder;

chat_msg::chat_msg(const config &cfg)
	: color_()
	, nick_()
	, text_(cfg["message"].str())
{
	const std::string& team_name = cfg["team_name"];
	if(team_name == "")
	{
		nick_ = cfg["id"].str();
	} else {
		nick_ = str_cast("*")+cfg["id"].str()+"*";
	}
	int side = cfg["side"].to_int(0);
	LOG_REPLAY << "side in message: " << side << std::endl;
	if (side==0) {
		color_ = "white";//observers
	} else {
		color_ = team::get_side_highlight_pango(side-1);
	}
	time_ = get_time(cfg);
	/*
	} else if (side==1) {
		color_ = "red";
	} else if (side==2) {
		color_ = "blue";
	} else if (side==3) {
		color_ = "green";
	} else if (side==4) {
		color_ = "purple";
		}*/
}

chat_msg::~chat_msg()
{
}

replay::replay() :
	cfg_(),
	pos_(0),
	skip_(false),
	message_locations()
{}

replay::replay(const config& cfg) :
	cfg_(cfg),
	pos_(0),
	skip_(false),
	message_locations()
{}

void replay::append(const config& cfg)
{
	cfg_.append(cfg);
}

void replay::process_error(const std::string& msg)
{
	ERR_REPLAY << msg;

	resources::controller->process_oos(msg); // might throw end_level_exception(QUIT)
}

void replay::set_skip(bool skip)
{
	skip_ = skip;
}

bool replay::is_skipping() const
{
	return skip_;
}

void replay::add_unit_checksum(const map_location& loc,config* const cfg)
{
	if(! game_config::mp_debug) {
		return;
	}
	config& cc = cfg->add_child("checksum");
	loc.write(cc);
	unit_map::const_iterator u = resources::units->find(loc);
	assert(u.valid());
	cc["value"] = get_checksum(*u);
}


void replay::init_side()
{
	config* const cmd = add_command();
	config init_side;
	init_side["side_number"] = resources::controller->current_side();
	cmd->add_child("init_side", init_side);
}

void replay::add_start()
{
	config* const cmd = add_command();
	(*cmd)["sent"] = true;
	cmd->add_child("start");
}

void replay::add_countdown_update(int value, int team)
{
	config* const cmd = add_command();
	config val;
	val["value"] = value;
	val["team"] = team;
	cmd->add_child("countdown_update",val);
}
void replay::add_synced_command(const std::string& name, const config& command)
{
	config* const cmd = add_command();
	cmd->add_child(name,command);
	LOG_REPLAY << "add_synced_command: \n" << cmd->debug() << "\n";
}



void replay::user_input(const std::string &name, const config &input, int from_side)
{
	config* const cmd = add_command();
	(*cmd)["dependent"] = true;
	if(from_side == -1)
	{
		(*cmd)["from_side"] = "server";
	}
	else
	{
		(*cmd)["from_side"] = from_side;
	}
	cmd->add_child(name, input);
}

void replay::add_label(const terrain_label* label)
{
	assert(label);
	config* const cmd = add_command();

	(*cmd)["undo"] = false;

	config val;

	label->write(val);

	cmd->add_child("label",val);
}

void replay::clear_labels(const std::string& team_name, bool force)
{
	config* const cmd = add_command();

	(*cmd)["undo"] = false;
	config val;
	val["team_name"] = team_name;
	val["force"] = force;
	cmd->add_child("clear_labels",val);
}

void replay::add_rename(const std::string& name, const map_location& loc)
{
	config* const cmd = add_command();
	(*cmd)["async"] = true; // Not undoable, but depends on moves/recruits that are
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


void replay::add_log_data(const std::string &key, const std::string &var)
{
	config& ulog = cfg_.child_or_add("upload_log");
	ulog[key] = var;
}

void replay::add_log_data(const std::string &category, const std::string &key, const std::string &var)
{
	config& ulog = cfg_.child_or_add("upload_log");
	config& cat = ulog.child_or_add(category);
	cat[key] = var;
}

void replay::add_log_data(const std::string &category, const std::string &key, const config &c)
{
	config& ulog = cfg_.child_or_add("upload_log");
	config& cat = ulog.child_or_add(category);
	cat.add_child(key,c);
}

void replay::add_checksum_check(const map_location& loc)
{
	if(! game_config::mp_debug || ! (resources::units->find(loc).valid()) ) {
		return;
	}
	config* const cmd = add_command();
	(*cmd)["dependent"] = true;
	add_unit_checksum(loc,cmd);
}

void replay::add_chat_message_location()
{
	message_locations.push_back(pos_-1);
}

void replay::speak(const config& cfg)
{
	config* const cmd = add_command();
	if(cmd != NULL) {
		cmd->add_child("speak",cfg);
		(*cmd)["undo"] = false;
		add_chat_message_location();
	}
}

void replay::add_chat_log_entry(const config &cfg, std::back_insert_iterator<std::vector<chat_msg> > &i) const
{
	if (!cfg) return;

	if (!preferences::parse_should_show_lobby_join(cfg["id"], cfg["message"])) return;
	if (preferences::is_ignored(cfg["id"])) return;
	*i = chat_msg(cfg);
}

void replay::remove_command(int index)
{
	cfg_.remove_child("command", index);
	std::vector<int>::reverse_iterator loc_it;
	for (loc_it = message_locations.rbegin(); loc_it != message_locations.rend() && index < *loc_it;++loc_it)
	{
		--(*loc_it);
	}
}

// cached message log
std::vector< chat_msg > message_log;


const std::vector<chat_msg>& replay::build_chat_log()
{
	std::vector<int>::iterator loc_it;
	int last_location = 0;
	std::back_insert_iterator<std::vector < chat_msg > > chat_log_appender( back_inserter(message_log));
	for (loc_it = message_locations.begin(); loc_it != message_locations.end(); ++loc_it)
	{
		last_location = *loc_it;
		
		const config &speak = command(last_location).child("speak");
		assert(speak);
		add_chat_log_entry(speak, chat_log_appender);

	}
	message_locations.clear();
	return message_log;
}

config replay::get_data_range(int cmd_start, int cmd_end, DATA_TYPE data_type)
{
	config res;

	for (int cmd = cmd_start; cmd < cmd_end; ++cmd)
	{
		config &c = command(cmd);
		if ((data_type == ALL_DATA || c["undo"] == "no") && c["sent"] != "yes")
		{
			res.add_child("command", c);
			if (data_type == NON_UNDO_DATA) c["sent"] = true;
		}
	}

	return res;
}

struct async_cmd
{
	config *cfg;
	int num;
};

void replay::redo(const config& cfg)
{
	//we set pos_ = ncommands(), if we recorded something else in the meantime it doesn't make sense to redo an action.
	assert(pos_ == ncommands());
	BOOST_FOREACH(const config &cmd, cfg.child_range("command"))
	{
		/*config &cfg = */cfg_.add_child("command", cmd);
	}
	pos_ = ncommands();
	
}



config& replay::get_last_real_command()
{
	for (int cmd_num = pos_ - 1; cmd_num >= 0; --cmd_num)
	{
		config &c = command(cmd_num);
		const config &cc = c;
		if (cc["dependent"].to_bool(false) || !cc["undo"].to_bool(true) || cc["async"].to_bool(false))
		{
			continue;
		}
		return c;
	}
	ERR_REPLAY << "replay::get_last_real_command called with no existent command.\n";
	assert(false && "replay::get_last_real_command called with no existent command.");
	throw "replay::get_last_real_command called with no existent command.";
}


void replay::undo_cut(config& dst)
{
	assert(dst.empty());
	//pos_ < ncommands() could mean that we try to undo commands that haven't been executed yet.
	assert(pos_ == ncommands());
	std::vector<async_cmd> async_cmds;
	// Remember commands not yet synced and skip over them.
	// We assume that all already sent (sent=yes) data isn't undoable
	// even if not marked explicitly with undo=no.

	/**
	 * @todo Change undo= to default to "no" and explicitly mark all
	 * undoable commands with yes.
	 */

	int cmd;
	for (cmd = ncommands() - 1; cmd >= 0; --cmd)
	{
		//"undo"=no means speak/label/remove_label
		//"async"=yes means rename_unit
		//"dependent"=true means user input or unit_checksum_check
		config &c = command(cmd);
		const config &cc = c;
		if (cc["dependent"].to_bool(false))
		{
			continue;
		}
		if (cc["undo"] != "no" && cc["async"] != "yes" && cc["sent"] != "yes") break;
		if (cc["async"] == "yes") {
			async_cmd ac = { &c, cmd };
			async_cmds.push_back(ac);
		}
	}

	if (cmd < 0) return;
	//we add the commands that we want to remove later to the passed cfg first.
	dst.add_child("command", cfg_.child("command", cmd));
	//we do this in a seperate loop because we don't want to loop forward in the loop while when we remove the elements to keepo the indexes simple.
	for(int cmd_2 = cmd + 1; cmd_2 < ncommands(); ++cmd_2)
	{
		if(command(cmd_2)["dependent"].to_bool(false))
		{
			dst.add_child("command", cfg_.child("command", cmd_2));
		}
	}

	//we remove dependent commands after the actual removed command that don't make sense if they stand alone especialy user choices and checksum data.
	for(int cmd_2 = ncommands() - 1; cmd_2 > cmd; --cmd_2)
	{
		if(command(cmd_2)["dependent"].to_bool(false))
		{
			remove_command(cmd_2);
		}
	}
	
	
	config &c = command(cmd);

	if (const config &child = c.child("move"))
	{
		// A unit's move is being undone.
		// Repair unsynced cmds whose locations depend on that unit's location.
		const std::vector<map_location> steps =
			parse_location_range(child["x"], child["y"]);

		if (steps.empty()) {
			ERR_REPLAY << "trying to undo a move using an empty path";
		}
		else {
			const map_location early_stop(child["stop_x"].to_int(-999) - 1,
			                              child["stop_y"].to_int(-999) - 1);
			const map_location &src = steps.front();
			const map_location &dst = early_stop.valid() ? early_stop : steps.back();

			BOOST_FOREACH(const async_cmd &ac, async_cmds)
			{
				if (config &async_child = ac.cfg->child("rename")) {
					map_location aloc(async_child, resources::gamedata);
					if (dst == aloc) src.write(async_child);
				}
			}
		}
	}
	else
	{
		const config *chld = &c.child("recruit");
		if (!*chld) chld = &c.child("recall");
		if (*chld) {
			// A unit is being un-recruited or un-recalled.
			// Remove unsynced commands that would act on that unit.
			map_location src(*chld, resources::gamedata);
			BOOST_FOREACH(const async_cmd &ac, async_cmds)
			{
				if (config &async_child = ac.cfg->child("rename"))
				{
					map_location aloc(async_child, resources::gamedata);
					if (src == aloc) remove_command(ac.num);
				}
			}
		}
	}
	remove_command(cmd);
	pos_ = ncommands();
}

void replay::undo()
{
	config dummy;
	undo_cut(dummy);
}

config &replay::command(int n)
{
	config & retv = cfg_.child("command", n);
	assert(retv);
	return retv;
}

int replay::ncommands() const
{
	return cfg_.child_count("command");
}

config* replay::add_command()
{
	//pos_ != ncommands() means that there is a command on the replay which would be skipped.
	assert(pos_ == ncommands());
	pos_ = ncommands()+1;
	return &cfg_.add_child("command");
}

void replay::start_replay()
{
	pos_ = 0;
}

void replay::revert_action()
{
	if (pos_ > 0)
		--pos_;
}

config* replay::get_next_action()
{
	if (pos_ >= ncommands())
		return NULL;

	LOG_REPLAY << "up to replay action " << pos_ + 1 << '/' << ncommands() << '\n';
	
	config* retv =  &command(pos_);
	++pos_;
	return retv;
}


bool replay::at_end() const
{
	return pos_ >= ncommands();
}

void replay::set_to_end()
{
	pos_ = ncommands();
}

void replay::clear()
{
	message_locations.clear();
	message_log.clear();
	cfg_ = config();
	pos_ = 0;
	skip_ = false;
}

bool replay::empty()
{
	return ncommands() == 0;
}

void replay::add_config(const config& cfg, MARK_SENT mark)
{
	BOOST_FOREACH(const config &cmd, cfg.child_range("command"))
	{
		config &cfg = cfg_.add_child("command", cmd);
		if(mark == MARK_AS_SENT) {
			cfg["sent"] = true;
		}
	}
}

replay& get_replay_source()
{
	return recorder;
}

static void check_checksums(const config &cfg)
{
	if(! game_config::mp_debug) {
		return;
	}
	BOOST_FOREACH(const config &ch, cfg.child_range("checksum"))
	{
		map_location loc(ch, resources::gamedata);
		unit_map::const_iterator u = resources::units->find(loc);
		if (!u.valid()) {
			std::stringstream message;
			message << "non existent unit to checksum at " << loc.x+1 << "," << loc.y+1 << "!";
			resources::screen->add_chat_message(time(NULL), "verification", 1, message.str(),
					events::chat_handler::MESSAGE_PRIVATE, false);
			continue;
		}
		if (get_checksum(*u) != ch["value"]) {
			std::stringstream message;
			message << "checksum mismatch at " << loc.x+1 << "," << loc.y+1 << "!";
			resources::screen->add_chat_message(time(NULL), "verification", 1, message.str(),
					events::chat_handler::MESSAGE_PRIVATE, false);
		}
	}
}



static void show_oos_error_error_function(const std::string& message, bool /*heavy*/)
{
	replay::process_error(message);
}

REPLAY_RETURN do_replay(int side_num)
{
	log_scope("do replay");

	if (!get_replay_source().is_skipping()){
		resources::screen->recalculate_minimap();
	}

	update_locker lock_update(resources::screen->video(),get_replay_source().is_skipping());
	return do_replay_handle(side_num);
}

REPLAY_RETURN do_replay_handle(int side_num)
{
	
	//team &current_team = (*resources::teams)[side_num - 1];


	for(;;) {
		const config *cfg = get_replay_source().get_next_action();
		bool is_synced = (synced_context::get_syced_state() == synced_context::SYNCED);

		if (cfg)
		{
			DBG_REPLAY << "Replay data:\n" << *cfg << "\n";
		}
		else
		{
			DBG_REPLAY << "Replay data at end\n";
		}

		LOG_REPLAY << "in do replay with is_synced=" << is_synced << "\n";

		//if there is nothing more in the records
		if(cfg == NULL) {
			//replayer.set_skip(false);
			return REPLAY_RETURN_AT_END;
		}

		config::all_children_itors ch_itors = cfg->all_children_range();
		//if there is an empty command tag or a start tag
		if (ch_itors.first == ch_itors.second || cfg->has_child("start"))
		{
			//do nothing
		}
		else if (const config &child = cfg->child("speak"))
		{
			const std::string &team_name = child["team_name"];
			const std::string &speaker_name = child["id"];
			const std::string &message = child["message"];
			//if (!preferences::parse_should_show_lobby_join(speaker_name, message)) return;
			bool is_whisper = (speaker_name.find("whisper: ") == 0);
			get_replay_source().add_chat_message_location();
			if (!get_replay_source().is_skipping() || is_whisper) {
				int side = child["side"];
				resources::screen->add_chat_message(get_time(child), speaker_name, side, message,
						(team_name.empty() ? events::chat_handler::MESSAGE_PUBLIC
						: events::chat_handler::MESSAGE_PRIVATE),
						preferences::message_bell());
			}
		}
		else if (const config &child = cfg->child("label"))
		{
			terrain_label label(resources::screen->labels(), child);

			resources::screen->labels().set_label(label.location(),
						label.text(),
						label.team_name(),
						label.color());
		}
		else if (const config &child = cfg->child("clear_labels"))
		{
			resources::screen->labels().clear(std::string(child["team_name"]), child["force"].to_bool());
		}
		else if (const config &child = cfg->child("rename"))
		{
			const map_location loc(child, resources::gamedata);
			const std::string &name = child["name"];

			unit_map::iterator u = resources::units->find(loc);
			if (u.valid()) {
				if (u->unrenamable()) {
					std::stringstream errbuf;
					errbuf << "renaming unrenamable unit " << u->id() << '\n';
					replay::process_error(errbuf.str());
					continue;
				}
				u->rename(name);
			} else {
				// Users can rename units while it's being killed at another machine.
				// This since the player can rename units when it's not his/her turn.
				// There's not a simple way to prevent that so in that case ignore the
				// rename instead of throwing an OOS.
				WRN_REPLAY << "attempt to rename unit at location: "
				   << loc << ", where none exists (anymore).\n";
			}
		}

		else if (cfg->child("init_side"))
		{
			if(is_synced)
			{
				replay::process_error("found init_side in replay while is_synced=true\n" );
			}
			set_scontext_synced sync;
			resources::controller->do_init_side(side_num - 1, true);
		}

		//if there is an end turn directive
		else if (cfg->child("end_turn"))
		{
			if(is_synced)
			{
				replay::process_error("found end_turn in replay while is_synced=true\n" );
			}
			// During the original game, the undo stack would have been
			// committed at this point.
			resources::undo_stack->clear();

			if (const config &child = cfg->child("verify")) {
				verify(*resources::units, child);
			}

			return REPLAY_FOUND_END_TURN;
		}
		else if (const config &change = cfg->child("record_change_controller"))
		{
			//During the original game, a control change occurred. We want to note the change but
			//not restart the turn in replay, so this tag is recorded instead of [change_controller].
			//This code adapted from [change_controller] handler in playturn.cpp

			//don't use lexical_cast_default it's "safer" to end on error
			const int side = lexical_cast<int>(change["side"]);
			const size_t index = static_cast<size_t>(side-1);

			const std::string &controller = change["controller"];
			const std::string &player = change["player"];

			DBG_REPLAY << "Record change controller:" << std::endl << cfg->debug() << std::endl;

			if(index < resources::teams->size()) {
				team &tm = (*resources::teams)[index];
				if (!player.empty()) {
					tm.set_current_player(player);
				}
				unit_map::iterator leader = resources::units->find_leader(side);
				if (!player.empty() && leader.valid()) {
					leader->rename(player);
				}

				tm.change_controller(controller);
			} else {
				WRN_REPLAY << "Recorded nonsensical controller change... side = " << side << " > #teams = " << resources::teams->size() << std::endl;
			}

		}
		else if (const config &child = cfg->child("countdown_update"))
		{
			int val = child["value"];
			int tval = child["team"];
			if (tval <= 0  || tval > int(resources::teams->size())) {
				std::stringstream errbuf;
				errbuf << "Illegal countdown update \n"
					<< "Received update for :" << tval << " Current user :"
					<< side_num << "\n" << " Updated value :" << val;

				replay::process_error(errbuf.str());
			} else {
				(*resources::teams)[tval - 1].set_countdown_time(val);
			}
		}
		else  if ( cfg->child("checksum") )
		{
			check_checksums(*cfg);
		}
		else if ((*cfg)["dependent"].to_bool(false))
		{
			if(!is_synced)
			{
				replay::process_error("found dependent command in replay while is_synced=false\n" );
			}
			//this means user choice.
			// it never makes sense to try to execute a user choice.
			// but we are called from 
			// the only other option for "dependent" command is checksum wich is already checked.
			assert(cfg->all_children_count() == 1);
			std::string child_name = cfg->all_children_range().first->key;
			DBG_REPLAY << "got an dependent action name = " << child_name <<"\n";
			get_replay_source().revert_action();
			return REPLAY_FOUND_DEPENDENT;
		}
		else
		{
			const std::string & commandname = cfg->ordered_begin()->key;
			config data = cfg->ordered_begin()->cfg;
			
			if(is_synced)
			{
				replay::process_error("found " + commandname + " command in replay while is_synced=true\n" );
			}
			LOG_REPLAY << "found commandname " << commandname << "in replay";
			synced_context::run_in_synced_context(commandname, data, false, !get_replay_source().is_skipping(), false,show_oos_error_error_function);
		}

		if (const config &child = cfg->child("verify")) {
			verify(*resources::units, child);
		}
	}
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
		resources::whiteboard->send_network_data();

		config cfg;
		const config& data = cfg.add_child("turn",obj_.get_data_range(upto_,obj_.ncommands(),replay::NON_UNDO_DATA));
		if(data.empty() == false) {
			network::send_data(cfg, 0);
		}
	}
}

void replay_network_sender::commit_and_sync()
{
	if(network::nconnections() > 0) {
		resources::whiteboard->send_network_data();
		
		config cfg;
		const config& data = cfg.add_child("turn",obj_.get_data_range(upto_,obj_.ncommands()));
		
		if(data.empty() == false) {
			network::send_data(cfg, 0);
		}

		upto_ = obj_.ncommands();
	}
}


static std::map<int, config> get_user_choice_internal(const std::string &name, const mp_sync::user_choice &uch, const std::set<int>& sides)
{
	const int max_side  = static_cast<int>(resources::teams->size());
	
	BOOST_FOREACH(int side, sides)
	{
		//the caller has to ensure this.
		assert(1 <= side && side <= max_side);
		assert(!(*resources::teams)[side-1].is_empty());
	}


	//this should never change during the execution of this function.
	int current_side = resources::controller->current_side();
	bool is_mp_game = network::nconnections() != 0;
	
	std::map<int,config> retv;
	/*
		when we got all our answers we stop.
	*/
	while(retv.size() != sides.size())
	{
		/*
			there might be speak or similar commands in the replay before the user input.
		*/
		do_replay_handle(current_side);

		/*
			these value might change due to player left/reassign during pull_remote_user_input
		*/
		//equals to any side in sides that is local, 0 if no such side exists.
		int local_side = 0;
		//if for any side from which we need an answer
		BOOST_FOREACH(int side, sides)
		{
			//and we havent already received our answer from that side
			if(retv.find(side) == retv.end())
			{
				//and it is local
				if((*resources::teams)[side-1].is_local())
				{
					//then we have to make a local choice.
					local_side = side;
					break;
				}
			}
		}

		bool has_local_side = local_side != 0;
		bool is_replay_end = get_replay_source().at_end();
		
		if (is_replay_end && has_local_side)
		{
			set_scontext_local_choice sync;
			/* At least one of the decisions is ours, and it will be inserted
			   into the replay. */
			DBG_REPLAY << "MP synchronization: local choice\n";
			config cfg = uch.query_user(local_side);
			
			recorder.user_input(name, cfg, local_side);
			retv[local_side]= cfg;

			//send data to others.
			//but if there wasn't any data sended during this turn, we don't want to bein wth that now.
			if(synced_context::is_simultaneously() || current_side != local_side)
			{
				synced_context::pull_remote_user_input();
			}
			continue;

		}
		else if(is_replay_end && !has_local_side)
		{
			//we are in a mp game, and the data has not been recieved yet.
			DBG_REPLAY << "MP synchronization: waiting for remote choice\n";
			
			assert(is_mp_game);
			synced_context::pull_remote_user_input();

			SDL_Delay(10);
			continue;
		}
		else if(!is_replay_end)
		{
			DBG_REPLAY << "MP synchronization: extracting choice from replay with has_local_side=" << has_local_side << "\n";
			
			const config *action = get_replay_source().get_next_action();
			if (!action) 
			{
				replay::process_error("[" + name + "] expected but none found\n");
				//this is weird, because it means that there is data on the replay but get_next_action returned an invalid config which should be impossible.
				assert(false);
			} 
			else if( !action->has_child(name))
			{
				replay::process_error("[" + name + "] expected but none found\n. found instead:\n" + action->debug());
				return retv;
			}
			int from_side = (*action)["from_side"].to_int(0);
			if ((*action)["side_invalid"].to_bool(false) == true)
			{
				//since this 'cheat' can have a quite heavy effect especialy in umc content we give an oos error .
				replay::process_error("MP synchronization: side_invalid in replay data, this could mean someone wants to cheat.\n");
			}
			if (sides.find(from_side) == sides.end())
			{
				replay::process_error("MP synchronization: we got an answer from side " + boost::lexical_cast<std::string>(from_side) + "for [" + name + "] which is not was we expected\n");
				continue;
			}
			if(retv.find(from_side) != retv.end())
			{
				replay::process_error("MP synchronization: we got already our answer from side " + boost::lexical_cast<std::string>(from_side) + "for [" + name + "] now we have it twice.\n");
			}
			retv[from_side] = action->child(name);
			continue;
		}
	}//while
	return retv;
}

std::map<int,config> mp_sync::get_user_choice_multiple_sides(const std::string &name, const mp_sync::user_choice &uch,
	std::set<int> sides)
{	
	//pass sides by copy because we need a copy.
	const bool is_synced = synced_context::get_syced_state() == synced_context::SYNCED;
	const int max_side  = static_cast<int>(resources::teams->size());
	//we currently don't check for too early because luas sync choice doesn't necessarily show screen dialogs.
	//It (currently) in the responsibility of the user of sync choice to not use dialogs during prestart events..
	if(!is_synced)
	{
		//we got called from inside luas wesnoth.synchronize_choice or from a select event.
		replay::process_error("MP synchronization only works in a synced context (for example Select or preload events are no synced context).\n");
		return std::map<int,config>();
	}

	/*
		for empty sides we want to use random choice instead.
	*/
	std::set<int> empty_sides;
	BOOST_FOREACH(int side, sides)
	{
		assert(1 <= side && side <= max_side);
		if( (*resources::teams)[side-1].is_empty())
		{
			empty_sides.insert(side);
		}
	}

	BOOST_FOREACH(int side, empty_sides)
	{
		sides.erase(side);
	}

	std::map<int,config> retv =  get_user_choice_internal(name, uch, sides);
	
	BOOST_FOREACH(int side, empty_sides)
	{
		retv[side] = uch.random_choice(side);
	}
	return retv;

}

/*
	fixes some rare cases and calls get_user_choice_internal if we are in a synced context.
*/
config mp_sync::get_user_choice(const std::string &name, const mp_sync::user_choice &uch,
	int side)
{
	const bool is_too_early = resources::gamedata->phase() != game_data::START && resources::gamedata->phase() != game_data::PLAY;
	const bool is_synced = synced_context::get_syced_state() == synced_context::SYNCED;
	const bool is_mp_game = network::nconnections() != 0;//Only used in debugging output below
	const int max_side  = static_cast<int>(resources::teams->size());
	const int current_side = resources::controller->current_side();
	bool is_side_null_controlled;
	
	if(!is_synced)
	{
		//we got called from inside luas wesnoth.synchronize_choice or from a select event (or maybe a preload event?).
		//This doesn't cause problems and someone could use it for example to use a [message][option] inside a wesnoth.synchronize_choice which could be useful, 
		//so just give a warning.
		WRN_REPLAY << "MP synchronization called during an unsynced context.";; 
		return uch.query_user(side);
	}
	if(is_too_early && uch.is_visible())
	{
		//We are in a prestart event or even earlier.
		//Although we are able to sync them, we cannot use query_user,
		//because we cannot (or shouldn't) put things on the screen inside a prestart event, this is true for SP and MP games.
		//Quotation form event wiki: "For things displayed on-screen such as character dialog, use start instead"
		return uch.random_choice(side);
	}
	//in start events it's unclear to decide on which side the function should be executed (default= side1 still). 
	//But for advancements we can just decide on the side that owns the unit and that's in the responsibility of advance_unit_at.
	//For [message][option] and luas sync_choice the scenario designer is responsible for that.
	//For [get_global_variable] side is never null.
	
	/*
		side = 0 should default to the currently active side per definition.
	*/
	if ( side < 1  ||  max_side < side )
	{
		if(side != 0)
		{
			ERR_REPLAY << "Invalid parameter for side in get_user_choice.\n";
		}
		side = resources::controller->current_side();
		LOG_REPLAY << " side changed to " << side << "\n";
	}
	is_side_null_controlled = (*resources::teams)[side-1].is_empty();
	
	LOG_REPLAY << "get_user_choice_called with"
			<< " name=" << name
			<< " is_synced=" << is_synced
			<< " is_mp_game=" << is_mp_game
			<< " is_side_null_controlled=" << is_side_null_controlled << "\n";

	if (is_side_null_controlled)
	{
		DBG_REPLAY << "MP synchronization: side 1 being null-controlled in get_user_choice.\n";
		//most likely we are in a start event with an empty side 1 
		//but calling [set_global_variable] to an empty side might also cause this.
		//i think in that case we should better use uch.random_choice(), 
		//which could return something like config_of("invalid", true);
		side = 1;
		while ( side <= max_side  &&  (*resources::teams)[side-1].is_empty() )
			side++;
		assert(side <= max_side);
	}


	assert(1 <= side && side <= max_side);
	
	if(current_side != side)
	{
		//if side != current_side we send the data over the network, that means undoing is impossible
		//maybe it would be better to do this in replayturn.cpp or similar. or maybe not.
		resources::undo_stack->clear();
	}
	std::set<int> sides;
	sides.insert(side);
	std::map<int, config> retv = get_user_choice_internal(name, uch, sides);
	if(retv.find(side) == retv.end())
	{
		//An error occured, get_user_choice_internal should have given an oos error message
		return config();
	}
	return retv[side];
}
