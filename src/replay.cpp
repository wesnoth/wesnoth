/*
	Copyright (C) 2003 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 *  See https://www.wesnoth.org/wiki/ReplayWML for more info.
 */

#include "replay.hpp"

#include "display_chat_manager.hpp"
#include "game_display.hpp"
#include "game_data.hpp"
#include "gettext.hpp"
#include "lexical_cast.hpp"
#include "log.hpp"
#include "map/label.hpp"
#include "map/location.hpp"
#include "play_controller.hpp"
#include "preferences/preferences.hpp"
#include "replay_recorder_base.hpp"
#include "resources.hpp"
#include "serialization/chrono.hpp"
#include "synced_context.hpp"
#include "units/unit.hpp"
#include "whiteboard/manager.hpp"
#include "wml_exception.hpp"

#include <array>
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
	LOG_REPLAY << "verifying unit structure...";

	const std::size_t nunits = cfg["num_units"].to_size_t();
	if(nunits != units.size()) {
		errbuf << "SYNC VERIFICATION FAILED: number of units from data source differ: "
			   << nunits << " according to data source. " << units.size() << " locally\n";

		std::set<map_location> locs;
		for (const config &u : cfg.child_range("unit"))
		{
			const map_location loc(u);
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

	for (const config &un : cfg.child_range("unit"))
	{
		const map_location loc(un);
		const unit_map::const_iterator u = units.find(loc);
		if(u == units.end()) {
			errbuf << "SYNC VERIFICATION FAILED: data source says there is a '"
				   << un["type"] << "' (side " << un["side"] << ") at "
				   << loc << " but there is no local record of it\n";
			replay::process_error(errbuf.str());
			errbuf.clear();
		}

		config u_cfg;
		u->write(u_cfg);

		bool is_ok = true;

		using namespace std::literals::string_literals;
		static const std::array fields{"type"s, "hitpoints"s, "experience"s, "side"s};

		for(const std::string& field : fields) {
			if (u_cfg[field] != un[field]) {
				errbuf << "ERROR IN FIELD '" << field << "' for unit at "
					   << loc << " data source: '" << un[field]
					   << "' local: '" << u_cfg[field] << "'\n";
				is_ok = false;
			}
		}

		if(!is_ok) {
			errbuf << "(SYNC VERIFICATION FAILED)\n";
			replay::process_error(errbuf.str());
			errbuf.clear();
		}
	}

	LOG_REPLAY << "verification passed";
}

static std::chrono::system_clock::time_point get_time(const config& speak)
{
	if(!speak["time"].empty()) {
		return chrono::parse_timestamp(speak["time"]);
	} else {
		// fallback in case sender uses wesnoth that doesn't send timestamps
		return std::chrono::system_clock::now();
	}
}

chat_msg::chat_msg(const config &cfg)
	: color_()
	, nick_()
	, text_(cfg["message"].str())
	, time_(get_time(cfg))
{
	if(cfg["team_name"].empty() && cfg["to_sides"].empty())
	{
		nick_ = cfg["id"].str();
	} else {
		nick_ = "*"+cfg["id"].str()+"*";
	}
	int side = cfg["side"].to_int(0);
	LOG_REPLAY << "side in message: " << side;
	if (side==0) {
		color_ = "white";//observers
	} else {
		color_ = team::get_side_highlight_pango(side);
	}
}

chat_msg::~chat_msg()
{
}

replay::replay(replay_recorder_base& base)
	: base_(&base)
	, sent_upto_(base.size())
	, message_locations()
{}

void replay::delete_upcoming_commands()
{
	base_->delete_upcoming_commands();
}
/*
	TODO: there should be different types of OOS messages:
		1)the normal OOS message
		2) the 'is guaranteed you'll get an assertion error after this and therefore you cannot continue' OOS message
		3) the 'do you want to overwrite calculated data with the data stored in replay' OOS error message.

*/
void replay::process_error(const std::string& msg)
{
	ERR_REPLAY << msg;

	resources::controller->process_oos(msg); // might throw quit_game_exception()
}

void replay::add_unit_checksum(const map_location& loc,config& cfg)
{
	if(! game_config::mp_debug) {
		return;
	}
	config& cc = cfg.add_child("checksum");
	loc.write(cc);
	unit_map::const_iterator u = resources::gameboard->units().find(loc);
	assert(u.valid());
	cc["value"] = get_checksum(*u);
}


void replay::init_side()
{
	config& cmd = add_command();
	config init_side;
	init_side["side_number"] = resources::controller->current_side();
	cmd.add_child("init_side", init_side);
}

void replay::add_start()
{
	config& cmd = add_command();
	cmd["sent"] = true;
	cmd.add_child("start");
}

void replay::add_surrender(int side_number)
{
	config& cmd = add_nonundoable_command();
	cmd.add_child("surrender")["side_number"] = side_number;
}

void replay::add_countdown_update(int value, int team)
{
	config& cmd = add_command();
	config val;
	val["value"] = value;
	val["team"] = team;
	cmd.add_child("countdown_update", std::move(val));
}
void replay::add_synced_command(const std::string& name, const config& command)
{
	config& cmd = add_command();
	cmd.add_child(name,command);
	cmd["from_side"] = resources::controller->current_side();
	LOG_REPLAY << "add_synced_command: \n" << cmd.debug();
}



void replay::user_input(const std::string &name, const config &input, int from_side)
{
	config& cmd = add_command();
	cmd["dependent"] = true;
	if(from_side == -1)
	{
		cmd["from_side"] = "server";
	}
	else
	{
		cmd["from_side"] = from_side;
	}
	cmd.add_child(name, input);
}

void replay::add_label(const terrain_label* label)
{
	assert(label);
	config& cmd = add_nonundoable_command();
	config val;

	label->write(val);

	cmd.add_child("label",val);
}

void replay::clear_labels(const std::string& team_name, bool force)
{
	config& cmd = add_nonundoable_command();

	config val;
	val["team_name"] = team_name;
	val["force"] = force;
	cmd.add_child("clear_labels", std::move(val));
}

void replay::add_rename(const std::string& name, const map_location& loc)
{
	config& cmd = add_command();
	cmd["async"] = true; // Not undoable, but depends on moves/recruits that are
	config val;
	loc.write(val);
	val["name"] = name;
	cmd.add_child("rename", std::move(val));
}


void replay::end_turn(int next_player_number)
{
	config& cmd = add_command();
	config& end_turn = cmd.add_child("end_turn");

	end_turn["next_player_number"] = next_player_number;
}


void replay::add_log_data(const std::string &key, const std::string &var)
{
	config& ulog = base_->get_upload_log();
	ulog[key] = var;
}

void replay::add_log_data(const std::string &category, const std::string &key, const std::string &var)
{
	config& ulog = base_->get_upload_log();
	config& cat = ulog.child_or_add(category);
	cat[key] = var;
}

void replay::add_log_data(const std::string &category, const std::string &key, const config &c)
{
	config& ulog = base_->get_upload_log();
	config& cat = ulog.child_or_add(category);
	cat.add_child(key,c);
}

bool replay::add_chat_message_location()
{
	return add_chat_message_location(base_->get_pos() - 1);
}

bool replay::add_chat_message_location(int pos)
{
	assert(base_->get_command_at(pos).has_child("speak"));
	if(std::find(message_locations.begin(), message_locations.end(), pos) == message_locations.end()) {
		message_locations.push_back(pos);
		return true;
	}
	else {
		return false;
	}
}

void replay::speak(const config& cfg)
{
	config& cmd = add_nonundoable_command();
	cmd.add_child("speak",cfg);
	add_chat_message_location(base_->size() - 1);
}

void replay::add_chat_log_entry(const config &cfg, std::back_insert_iterator<std::vector<chat_msg>> &i) const
{

	if (!prefs::get().parse_should_show_lobby_join(cfg["id"], cfg["message"])) return;
	if (prefs::get().is_ignored(cfg["id"])) return;
	*i = chat_msg(cfg);
}

void replay::remove_command(int index)
{
	base_->remove_command(index);
	std::vector<int>::reverse_iterator loc_it;
	for (loc_it = message_locations.rbegin(); loc_it != message_locations.rend() && index < *loc_it;++loc_it)
	{
		--(*loc_it);
	}
}

// cached message log
static std::vector< chat_msg > message_log;


const std::vector<chat_msg>& replay::build_chat_log() const
{
	message_log.clear();
	std::vector<int>::const_iterator loc_it;
	int last_location = 0;
	std::back_insert_iterator<std::vector < chat_msg >> chat_log_appender( back_inserter(message_log));
	for (loc_it = message_locations.begin(); loc_it != message_locations.end(); ++loc_it)
	{
		last_location = *loc_it;

		const config &speak = command(last_location).mandatory_child("speak");
		add_chat_log_entry(speak, chat_log_appender);

	}
	return message_log;
}

config replay::get_unsent_commands(DATA_TYPE data_type)
{
	config res;
	for (int cmd = sent_upto_; cmd < ncommands(); ++cmd)
	{
		config &c = command(cmd);
		//prevent creating 'blank' attribute values during checks
		const config &cc = c;
		if ((data_type == ALL_DATA || !cc["undo"].to_bool(true)) && !cc["sent"].to_bool(false))
		{
			res.add_child("command", c);
			c["sent"] = true;
		}
	}
	if(data_type == ALL_DATA) {
		sent_upto_ = ncommands();
	}
	return res;
}

void replay::redo(const config& cfg, bool set_to_end)
{
	assert(base_->get_pos() == ncommands());
	int old_pos = base_->get_pos();
	for (const config &cmd : cfg.child_range("command"))
	{
		base_->add_child() = cmd;
	}
	if(set_to_end) {
		//The engine does not execute related wml events so mark ad dpendent actions as handled
		base_->set_to_end();
	}
	else {
		//The engine does execute related wml events so it needs to reprocess depndent choices
		base_->set_pos(old_pos + 1);
	}

}



config& replay::get_last_real_command()
{
	for (int cmd_num = base_->get_pos() - 1; cmd_num >= 0; --cmd_num)
	{
		config &c = command(cmd_num);
		const config &cc = c;
		if (cc["dependent"].to_bool(false) || !cc["undo"].to_bool(true) || cc["async"].to_bool(false))
		{
			continue;
		}
		return c;
	}
	ERR_REPLAY << "replay::get_last_real_command called with no existent command.";
	assert(false && "replay::get_last_real_command called with no existent command.");
	throw "replay::get_last_real_command called with no existent command.";
}
/**
 * fixes a rename command when undoing a earlier command.
 * @return: true if the command should be removed.
 */
static bool fix_rename_command(const config& c, config& async_child)
{
	if (const auto child = c.optional_child("move"))
	{
		// A unit's move is being undone.
		// Repair unsynced cmds whose locations depend on that unit's location.
		std::vector<map_location> steps;

		try {
			read_locations(child.value(), steps);
		} catch(const bad_lexical_cast &) {
			WRN_REPLAY << "Warning: Path data contained something which could not be parsed to a sequence of locations:" << "\n config = " << child->debug();
		}

		if (steps.empty()) {
			ERR_REPLAY << "trying to undo a move using an empty path";
		}
		else {
			const map_location &src = steps.front();
			const map_location &dst = steps.back();
			map_location aloc(async_child);
			if (dst == aloc) src.write(async_child);
		}
	}
	else
	{
		auto loc = c.optional_child("recruit");
		if(!loc) {
			loc = c.optional_child("recall");
		}

		if(loc) {
			// A unit is being un-recruited or un-recalled.
			// Remove unsynced commands that would act on that unit.
			map_location src(loc.value());
			map_location aloc(async_child);
			if (src == aloc) {
				return true;
			}
		}
	}
	return false;
}

void replay::undo_cut(config& dst)
{
	assert(dst.empty());
	//assert that we are not undoing a command which we didn't execute yet.
	assert(at_end());

	//calculate the index of the last synced user action (which we want to undo).
	int cmd_index = ncommands() - 1;
	for (; cmd_index >= 0; --cmd_index)
	{
		//"undo"=no means speak/label/remove_label, especially attack, recruits etc. have "undo"=yes
		//"async"=yes means rename_unit
		//"dependent"=true means user input
		const config &c = command(cmd_index);

		if(c["undo"].to_bool(true) && !c["async"].to_bool(false) && !c["dependent"].to_bool(false))
		{
			if(c["sent"].to_bool(false))
			{
				ERR_REPLAY << "trying to undo a command that was already sent.";
				return;
			}
			else
			{
				break;
			}
		}
	}

	if (cmd_index < 0)
	{
		ERR_REPLAY << "trying to undo a command but no command was found.";
		return;
	}
	//Fix the [command]s after the undone action. This includes dependent commands for that user actions and async user action.
	for(int i = ncommands() - 1; i >= cmd_index; --i)
	{
		config &c = command(i);
		const config &cc = c;
		if(!cc["undo"].to_bool(true))
		{
			//Leave these commands on the replay.
		}
		else if(cc["async"].to_bool(false))
		{
			if(auto rename = c.optional_child("rename"))
			{
				if(fix_rename_command(command(cmd_index), rename.value()))
				{
					//remove the command from the replay if fix_rename_command requested it.
					remove_command(i);
				}
			}
		}
		else if(cc["dependent"].to_bool(false) || i == cmd_index)
		{
			//we loop backwars so we must insert new insert at beginning to preserve order.
			dst.add_child_at("command", config(), 0).swap(c);
			remove_command(i);
		}
		else
		{
			ERR_REPLAY << "Couldn't handle command:\n" << cc << "\nwhen undoing.";
		}
	}
	set_to_end();
}

void replay::undo()
{
	config dummy;
	undo_cut(dummy);
}

config &replay::command(int n) const
{
	config & retv = base_->get_command_at(n);
	return retv;
}

int replay::ncommands() const
{
	return base_->size();
}

config& replay::add_command()
{
	// If we weren't at the end of the replay we should skip one or more
	// commands.
	assert(at_end());
	config& retv = base_->add_child();
	set_to_end();
	return retv;
}

config& replay::add_nonundoable_command()
{
	const bool was_at_end = at_end();
	config& r = base_->insert_command(base_->size());
	r["undo"] = false;
	if(was_at_end) {
		base_->set_pos(base_->get_pos() + 1);
	}
	assert(was_at_end == at_end());
	return r;
}

void replay::start_replay()
{
	base_->set_pos(0);
}

void replay::revert_action()
{

	if (base_->get_pos() > 0)
		base_->set_pos(base_->get_pos() - 1);
}

config* replay::get_next_action()
{
	if (at_end())
		return nullptr;

	LOG_REPLAY << "up to replay action " << base_->get_pos() + 1 << '/' << ncommands();

	config* retv = &command(base_->get_pos());
	base_->set_pos(base_->get_pos() + 1);
	return retv;
}

config* replay::peek_next_action()
{
	if (at_end())
		return nullptr;

	LOG_REPLAY << "up to replay action " << base_->get_pos() + 1 << '/' << ncommands();

	config* retv = &command(base_->get_pos());
	return retv;
}


bool replay::at_end() const
{
	assert(base_->get_pos() <= ncommands());
	return base_->get_pos() == ncommands();
}

void replay::set_to_end()
{
	base_->set_to_end();
}

bool replay::empty() const
{
	return ncommands() == 0;
}

void replay::add_config(const config& cfg, MARK_SENT mark)
{
	for (const config &cmd : cfg.child_range("command"))
	{
		config &cmd_cfg = base_->insert_command(base_->size());
		cmd_cfg = cmd;
		if(mark == MARK_AS_SENT) {
			cmd_cfg["sent"] = true;
		}
		if(cmd_cfg.has_child("speak")) {
			cmd_cfg["undo"] = false;
		}
	}
}
bool replay::add_start_if_not_there_yet()
{
	//this method would confuse the value of 'pos' otherwise
	VALIDATE(base_->get_pos() == 0, _("The file you have tried to load is corrupt"));
	//since pos is 0, at_end() is equivalent to empty()
	if(at_end() || !base_->get_command_at(0).has_child("start"))
	{
		base_->insert_command(0) = config {"start", config(), "sent", true};
		return true;
	}
	else
	{
		return false;
	}
}

REPLAY_ACTION_TYPE get_replay_action_type(const config& command)
{
	if(command.all_children_count() != 1) {
		return REPLAY_ACTION_TYPE::INVALID;
	}
	auto [key, _] = command.all_children_view().front();
	if(key == "speak" || key == "label" || key == "surrender" || key == "clear_labels" || key == "rename" || key == "countdown_update") {
		return REPLAY_ACTION_TYPE::UNSYNCED;
	}
	if(command["dependent"].to_bool(false)) {
		return REPLAY_ACTION_TYPE::DEPENDENT;
	}
	return REPLAY_ACTION_TYPE::SYNCED;
}

REPLAY_RETURN do_replay(bool one_move)
{
	log_scope("do replay");

	if (!resources::controller->is_skipping_replay()) {
		display::get_singleton()->recalculate_minimap();
	}

	return do_replay_handle(one_move);
}
/**
	@returns:
		if we expect a user choice and found something that prevents us from moving on we return REPLAY_FOUND_DEPENDENT (even if it is not a dependent command)
		else if we found an [end_turn] we return REPLAY_FOUND_END_TURN
		else if we found a player action and one_move=true we return REPLAY_FOUND_END_MOVE
		else (<=> we reached the end of the replay) we return REPLAY_RETURN_AT_END
*/
REPLAY_RETURN do_replay_handle(bool one_move)
{

	//team &current_team = resources::gameboard->get_team(side_num);

	const int side_num = resources::controller->current_side();
	while(true)
	{
		const config *cfg = resources::recorder->get_next_action();
		const bool is_synced = synced_context::is_synced();
		const bool is_unsynced = synced_context::get_synced_state() == synced_context::UNSYNCED;

		DBG_REPLAY << "in do replay with is_synced=" << is_synced << "is_unsynced=" << is_unsynced;

		if (cfg != nullptr)
		{
			DBG_REPLAY << "Replay data:\n" << *cfg;
		}
		else
		{
			DBG_REPLAY << "Replay data at end";
			return REPLAY_RETURN_AT_END;
		}


		const auto ch_itors = cfg->all_children_view();
		//if there is an empty command tag or a start tag
		if (ch_itors.empty() || cfg->has_child("start"))
		{
			//this shouldn't happen anymore because replaycontroller now moves over the [start] with get_next_action
			//also we removed the the "add empty replay entry at scenario reload" behavior.
			ERR_REPLAY << "found "<<  cfg->debug() <<" in replay";
			//do nothing
		}
		else if (auto speak = cfg->optional_child("speak"))
		{
			const std::string &team_name = speak["to_sides"];
			const std::string &speaker_name = speak["id"];
			const std::string &message = speak["message"];

			bool is_whisper = (speaker_name.find("whisper: ") == 0);
			if(resources::recorder->add_chat_message_location()) {
				DBG_REPLAY << "tried to add a chat message twice.";
				if (!resources::controller->is_skipping_replay() || is_whisper) {
					int side = speak["side"].to_int();
					auto as_time_t = std::chrono::system_clock::to_time_t(get_time(*speak)); // FIXME: remove
					game_display::get_singleton()->get_chat_manager().add_chat_message(as_time_t, speaker_name, side, message,
						(team_name.empty() ? events::chat_handler::MESSAGE_PUBLIC
						: events::chat_handler::MESSAGE_PRIVATE),
						prefs::get().message_bell());
				}
			}
		}
		else if (cfg->has_child("surrender"))
		{
			//prevent sending of a synced command for surrender
		}
		else if (auto label_config = cfg->optional_child("label"))
		{
			terrain_label label(display::get_singleton()->labels(), *label_config);

			display::get_singleton()->labels().set_label(label.location(),
						label.text(),
						label.creator(),
						label.team_name(),
						label.color());
		}
		else if (auto clear_labels = cfg->optional_child("clear_labels"))
		{
			display::get_singleton()->labels().clear(std::string(clear_labels["team_name"]), clear_labels["force"].to_bool());
		}
		else if (auto rename = cfg->optional_child("rename"))
		{
			const map_location loc(*rename);
			const std::string &name = rename["name"];

			unit_map::iterator u = resources::gameboard->units().find(loc);
			if (u.valid() && !u->unrenamable()) {
				u->rename(name);
			} else {
				// Users can rename units while it's being killed or at another machine.
				// This since the player can rename units when it's not his/her turn.
				// There's not a simple way to prevent that so in that case ignore the
				// rename instead of throwing an OOS.
				// The same way it is possible that an unrenamable unit moves to a
				// hex where previously a renamable unit was.
				WRN_REPLAY << "attempt to rename unit at location: "
				   << loc << (u.valid() ? ", which is unrenamable" : ", where none exists (anymore)");
			}
		}

		else if (cfg->has_child("init_side"))
		{

			if(!is_unsynced)
			{
				replay::process_error("found side initialization in replay expecting a user choice\n" );
				resources::recorder->revert_action();
				return REPLAY_FOUND_DEPENDENT;
			}
			else
			{
				resources::controller->do_init_side();
				if (one_move) {
					return REPLAY_FOUND_INIT_TURN;
				}
			}
		}

		//if there is an end turn directive
		else if (auto end_turn = cfg->optional_child("end_turn"))
		{
			if(!is_unsynced)
			{
				replay::process_error("found turn end in replay while expecting a user choice\n" );
				resources::recorder->revert_action();
				return REPLAY_FOUND_DEPENDENT;
			}
			else
			{
				if (auto cfg_verify = cfg->optional_child("verify")) {
					verify(resources::gameboard->units(), *cfg_verify);
				}
				if(int npn = end_turn["next_player_number"].to_int(0); npn > 0) {
					resources::controller->gamestate().next_player_number_ = npn;
				}
				resources::controller->gamestate().gamedata_.set_phase(game_data::TURN_ENDED);
				return REPLAY_FOUND_END_TURN;
			}
		}
		else if (auto countdown_update = cfg->optional_child("countdown_update"))
		{
			auto val = chrono::parse_duration<std::chrono::milliseconds>(countdown_update["value"]);
			int tval = countdown_update["team"].to_int();
			if (tval <= 0  || tval > static_cast<int>(resources::gameboard->teams().size())) {
				std::stringstream errbuf;
				errbuf << "Illegal countdown update \n"
					<< "Received update for :" << tval << " Current user :"
					<< side_num << "\n" << " Updated value :" << val.count();

				replay::process_error(errbuf.str());
			} else {
				resources::gameboard->get_team(tval).set_countdown_time(val);
			}
		}
		else if ((*cfg)["dependent"].to_bool(false))
		{
			if(is_unsynced)
			{
				replay::process_error("found dependent command in replay while is_synced=false\n" );
				//ignore this command
				continue;
			}
			//this means user choice.
			// it never makes sense to try to execute a user choice.
			// but we are called from
			// the only other option for "dependent" command is checksum which is already checked.
			assert(cfg->all_children_count() == 1);
			auto [child_name, _] = cfg->all_children_view().front();
			DBG_REPLAY << "got an dependent action name = " << child_name;
			resources::recorder->revert_action();
			return REPLAY_FOUND_DEPENDENT;
		}
		else
		{
			//we checked for empty commands at the beginning.
			const auto [commandname, data] = cfg->all_children_view().front();

			if(!is_unsynced)
			{
				replay::process_error("found [" + commandname + "] command in replay expecting a user choice\n" );
				resources::recorder->revert_action();
				return REPLAY_FOUND_DEPENDENT;
			}
			else
			{
				LOG_REPLAY << "found commandname " << commandname << "in replay";

				if((*cfg)["from_side"].to_int(0) != resources::controller->current_side()) {
					ERR_REPLAY << "received a synced [command] from side " << (*cfg)["from_side"].to_int(0) << ". Expacted was a [command] from side " << resources::controller->current_side();
				}
				else if((*cfg)["side_invalid"].to_bool(false)) {
					ERR_REPLAY << "received a synced [command] from side " << (*cfg)["from_side"].to_int(0) << ". Sent from wrong client.";
				}
				/*
					we need to use the undo stack during replays in order to make delayed shroud updated work.
				*/
				auto spectator = action_spectator([](const std::string& message) { replay::process_error(message); });
				synced_context::run(commandname, data, spectator);
				if(resources::controller->is_regular_game_end()) {
					return REPLAY_FOUND_END_LEVEL;
				}
				if (one_move) {
					return REPLAY_FOUND_END_MOVE;
				}
			}
		}

		if (auto child = cfg->optional_child("verify")) {
			verify(resources::gameboard->units(), *child);
		}
	}
}
