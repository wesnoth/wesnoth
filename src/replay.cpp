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
#include "rng.hpp"
#include "statistics.hpp"
#include "whiteboard/manager.hpp"

#include <boost/foreach.hpp>

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
	current_(NULL),
	skip_(false),
	message_locations(),
	expected_advancements_()
{}

replay::replay(const config& cfg) :
	cfg_(cfg),
	pos_(0),
	current_(NULL),
	skip_(false),
	message_locations(),
	expected_advancements_()
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

void replay::add_start()
{
	config* const cmd = add_command(true);
	cmd->add_child("start");
}


void replay::add_disband(const std::string& unit_id)
{
	config* const cmd = add_command();

	config val;

	val["value"] = unit_id;

	cmd->add_child("disband",val);
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


/**
 * Records that the player has toggled automatic shroud updates.
 */
void replay::add_auto_shroud(bool turned_on)
{
	config* cmd = add_command(false);
	config& child = cmd->add_child("auto_shroud");
	child["active"] = turned_on;
}

/**
 * Records that the player has manually updated fog/shroud.
 */
void replay::update_shroud()
{
	config* cmd = add_command(false);
	cmd->add_child("update_shroud");
}

void replay::add_seed(const char* child_name, int seed)
{
	LOG_REPLAY << "Setting seed for child type " << child_name << ": " << seed << "\n";
	random()->child(child_name)["seed"] = seed;
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
	config* const cmd = add_command(false);

	(*cmd)["undo"] = false;

	config val;

	label->write(val);

	cmd->add_child("label",val);
}

void replay::clear_labels(const std::string& team_name, bool force)
{
	config* const cmd = add_command(false);

	(*cmd)["undo"] = false;
	config val;
	val["team_name"] = team_name;
	val["force"] = force;
	cmd->add_child("clear_labels",val);
}

void replay::add_rename(const std::string& name, const map_location& loc)
{
	config* const cmd = add_command(false);
	(*cmd)["async"] = true; // Not undoable, but depends on moves/recruits that are
	config val;
	loc.write(val);
	val["name"] = name;
	cmd->add_child("rename", val);
}

void replay::init_side()
{
	config* const cmd = add_command();
	config init_side;
	if(!lg::debug.dont_log("network")) init_side["side_number"] = resources::controller->current_side();
	cmd->add_child("init_side", init_side);
}

void replay::end_turn()
{
	config* const cmd = add_command();
	cmd->add_child("end_turn");
}

void replay::add_event(const std::string& name, const map_location& loc)
{
	config* const cmd = add_command();
	config& ev = cmd->add_child("fire_event");
	ev["raise"] = name;
	if(loc.valid()) {
		config& source = ev.add_child("source");
		loc.write(source);
	}
	(*cmd)["undo"] = false;
}

void replay::add_lua_ai(const std::string& lua_code)
{
	config* const cmd = add_command();
	config& child = cmd->add_child("lua_ai");
	child["code"] = lua_code;
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
	config* const cmd = add_command(false);
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

config& replay::get_last_real_command()
{
	for (int cmd_num = pos_ - 1; cmd_num >= 0; --cmd_num)
	{
		config &c = command(cmd_num);
		if (c["dependent"].to_bool(false) || !c["undo"].to_bool(true) || c["async"].to_bool(false))
		{
			continue;
		}
		return c;
	}
	ERR_REPLAY << "replay::get_last_real_command called with not existant command.\n";
	assert(false && "replay::get_last_real_command called with not existant command.");
	//this code can never be reached because of the assert above so no need to return something.
	throw "assert didnt work :o";
}

void replay::undo()
{
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
		config &c = command(cmd);
		if (c["dependent"].to_bool(false))
		{
			continue;
		}
		if (c["undo"] != "no" && c["async"] != "yes" && c["sent"] != "yes") break;
		if (c["async"] == "yes") {
			async_cmd ac = { &c, cmd };
			async_cmds.push_back(ac);
		}
	}

	if (cmd < 0) return;
	
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
	current_ = NULL;
	set_random(NULL);
}

config &replay::command(int n)
{
	return cfg_.child("command", n);
}

int replay::ncommands() const
{
	return cfg_.child_count("command");
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

	current_ = &command(pos_);
	set_random(current_);
	++pos_;
	return current_;
}

void replay::pre_replay()
{
	if (rng::random() == NULL && ncommands() > 0) {
		if (at_end())
		{
			add_command(true);
		}
		else
		{
			set_random(&command(pos_));
		}
	}
}

bool replay::at_end() const
{
	return pos_ >= ncommands();
}

void replay::set_to_end()
{
	pos_ = ncommands();
	current_ = NULL;
	set_random(NULL);
}

void replay::clear()
{
	message_locations.clear();
	message_log.clear();
	cfg_ = config();
	pos_ = 0;
	current_ = NULL;
	set_random(NULL);
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

bool do_replay(int side_num)
{
	log_scope("do replay");

	if (!get_replay_source().is_skipping()){
		resources::screen->recalculate_minimap();
	}

	const rand_rng::set_random_generator generator_setter(&get_replay_source());

	update_locker lock_update(resources::screen->video(),get_replay_source().is_skipping());
	return do_replay_handle(side_num, "");
}

bool do_replay_handle(int side_num, const std::string &do_untill)
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
			return false;
		}

		// We return if caller wants it for this tag
		if (!do_untill.empty() && cfg->child(do_untill))
		{
			get_replay_source().revert_action();
			return false;
		}

		config::all_children_itors ch_itors = cfg->all_children_range();
		//if there is an empty command tag, create by pre_replay() or a start tag
		if (ch_itors.first == ch_itors.second || cfg->child("start"))
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

			return true;
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
			// the onyl other otoion for "dependent" command is checksum wich is already checked.
			assert(cfg->all_children_count() == 1);
			std::string child_name = cfg->all_children_range().first->key;
			DBG_REPLAY << "got an dependent action name = " << child_name <<"\n";
			get_replay_source().revert_action();
			// simply returning here is dangerous becaus we don't know wether the caller was waiting for a dependen command or not, 
			// in case not, it's an oos error we don't report.
			return false;
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


static config get_user_choice_internal(const std::string &name, const mp_sync::user_choice &uch, int side)
{
	//this should never change during the execution of this function.
	int current_side = resources::controller->current_side();
	
	if(current_side != side)
	{
		//if side != current_side we send the data over the network, that means undoing is impossible
		//maybe it would be better to do this in replayturn.cpp or similar. or maybe not.
		resources::undo_stack->clear();
	}
	/*
		if we have to wait for network data, the controlling side might change due to 
		players leaving/ getting reassigned during raise_sync_network.
	*/
	while(true)
	{
		/*
			there might be speak or simiar comands in the replay before the user input.
		*/
		do_replay_handle(current_side, name);

		/*
			these value might change due to player left/reassign during pull_remote_user_input
		*/
		bool is_local_side = (*resources::teams)[side-1].is_local();
		bool is_replay_end = get_replay_source().at_end();
		
		if (is_replay_end && is_local_side)
		{
			set_scontext_local_choice sync;
			/* The decision is ours, and it will be inserted
			   into the replay. */
			DBG_REPLAY << "MP synchronization: local choice\n";
			config cfg = uch.query_user();
			
			recorder.user_input(name, cfg, side);
			return cfg;

		}
		else if(is_replay_end && !is_local_side)
		{
			//we are in a mp game, and the data has not been recieved yet.
			DBG_REPLAY << "MP synchronization: waiting for remote choice\n";
			
			synced_context::pull_remote_user_input();

			SDL_Delay(10);
			continue;
		}
		else if(!is_replay_end)
		{

			DBG_REPLAY << "MP synchronization: extracting choice from replay with is_local_side=" << is_local_side << "\n";
			
			const config *action = get_replay_source().get_next_action();
			if (!action) 
			{
				replay::process_error("[" + name + "] expected but none found\n");
				return config();
			} 
			else if( !action->has_child(name))
			{
				replay::process_error("[" + name + "] expected but none found\n. found instead:\n" + action->debug());
				return config();
			}
			if ((*action)["side_invalid"].to_bool(false) == true)
			{
				WRN_REPLAY << "MP synchronization: side_invalid in replay data, this could mean someone wants to cheat.\n";
			}
			if ((*action)["from_side"].to_int(0) != side)
			{
				WRN_REPLAY << "MP synchronization: wrong from_side in replay data, this could mean someone wants to cheat.\n";
			}
			return action->child(name);
		}
	}//while
}
/*
	fixes some rare cases and calls get_user_choice_internal if we are in a synced context.
*/
config mp_sync::get_user_choice(const std::string &name, const mp_sync::user_choice &uch,
	int side)
{	
	bool is_synced = synced_context::get_syced_state() == synced_context::SYNCED;
	bool is_mp_game = network::nconnections() != 0;
	bool is_side_null_controlled;
	const int max_side  = static_cast<int>(resources::teams->size());

	if(!is_synced)
	{
		//we got called from inside luas wesnoth.synchronize_choice or from a select event.
		replay::process_error("MP synchronization only works in a synced context (for example Select events are no synced context).\n");
		return uch.query_user();
	}
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
		//most likeley we are in a start event with an empty side 1 
		//but calling [set_global_variable] to an empty side might also cause this.
		//i think in that case we should better use uch.random_choice(), 
		//which could return something like config_of("invalid", true);
		side = 1;
		while ( side <= max_side  &&  (*resources::teams)[side-1].is_empty() )
			side++;
		assert(side <= max_side);
	}


	assert(1 <= side && side <= max_side);
	return get_user_choice_internal(name, uch, side);
}
