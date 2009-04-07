/* $Id$ */
/*
   Copyright (C) 2003 - 2009 by Jörg Hinrichs, refactored from various
   places formerly created by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "savegame.hpp"

#include "dialogs.hpp" //FIXME: move illegal file character function here and get rid of this include
#include "game_end_exceptions.hpp"
#include "game_events.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "map.hpp"
#include "map_label.hpp"
#include "preferences_display.hpp"
#include "replay.hpp"
#include "serialization/binary_or_text.hpp"
#include "sound.hpp"

#define LOG_SAVE LOG_STREAM(info, engine)

savegame::savegame(game_state& gamestate, const std::string title) 
	: gamestate_(gamestate), 
	title_(title), 	snapshot_(),
	error_message_(_("The game could not be saved")),
	interactive_(false)
{}

play_controller* save_blocker::controller_ = NULL;
void (play_controller::*save_blocker::callback_)() = NULL;
SDL_sem* save_blocker::sem_ = SDL_CreateSemaphore(1);

save_blocker::save_blocker() {
	block();
}

save_blocker::~save_blocker() {
	unblock();
	if(controller_ && callback_) {
		(controller_->*callback_)();
		controller_ = NULL;
		callback_ = NULL;
	}
}

void save_blocker::on_unblock(play_controller* controller, void (play_controller::*callback)()) {
	if(try_block()) {
		unblock();
		(controller->*callback)();
	} else {
		controller_ = controller;
		callback_ = callback;
	}
}

bool save_blocker::saves_are_blocked() {
	return SDL_SemValue(sem_) == 0;
}

void save_blocker::block() {
	SDL_SemWait(sem_);
}

bool save_blocker::try_block() {
	return SDL_SemTryWait(sem_) == 0;
}

void save_blocker::unblock() {
	assert(SDL_SemValue(sem_) == 0);
	SDL_SemPost(sem_);
}

// Prototypes

void savegame::save_game_interactive(display& gui, const std::string& message, 
									 gui::DIALOG_TYPE dialog_type, const bool has_exit_button, 
									 const bool ask_for_filename)
{
	interactive_ = true;
	create_filename();
	const int res = dialogs::get_save_name(gui, message, _("Name: "), &filename_, dialog_type, title_, has_exit_button, ask_for_filename);

	if (res == 2)
		throw end_level_exception(QUIT);

	if (res != 0)
		return;

	save_game(&gui);
}

void savegame::before_save()
{
	gamestate_.replay_data = recorder.get_replay_data();
}

void savegame::save_game(const std::string& filename)
{
	filename_ = filename;
	if (!interactive_)
		create_filename();
	
	save_game();
}

void savegame::save_game(display* gui)
{
	try {
		before_save();
		save_game_internal(filename_);

		if (gui != NULL && interactive_)
			gui::message_dialog(*gui,_("Saved"),_("The game has been saved")).show();
	} catch(game::save_game_failed&) {
		if (gui != NULL){
			gui::message_dialog to_show(*gui,_("Error"), error_message_);
			to_show.show();
			//do not bother retrying, since the user can just try to save the game again
			//maybe show a yes-no dialog for "disable autosaves now"?
		}
	};
}

void savegame::save_game_internal(const std::string& filename)
{
	LOG_SAVE << "savegame::save_game";

	filename_ = filename;

	if(preferences::compress_saves()) {
		filename_ += ".gz";
	}

	std::stringstream ss;
	{
		config_writer out(ss, preferences::compress_saves());
		::write_game(out, snapshot_, gamestate_);
		finish_save_game(out, gamestate_, gamestate_.label);
	}
	scoped_ostream os(open_save_game(filename_));
	(*os) << ss.str();

	if (!os->good()) {
		throw game::save_game_failed(_("Could not write to file"));
	}
}

void savegame::set_filename(std::string filename)
{
	filename.erase(std::remove_if(filename.begin(), filename.end(),
	            dialogs::is_illegal_file_char), filename.end());
	filename_ = filename;
}

replay_savegame::replay_savegame(game_state &gamestate) 
	: savegame(gamestate, _("Save Replay")) 
{}

void replay_savegame::create_filename()
{
	std::stringstream stream;

	const std::string ellipsed_name = font::make_text_ellipsis(gamestate().label,
			font::SIZE_NORMAL, 200);
	stream << ellipsed_name << " " << _("replay");

	set_filename(stream.str());
}

autosave_savegame::autosave_savegame(game_state &gamestate, const config& level_cfg, 
							 const game_display& gui, const std::vector<team>& teams, 
							 const unit_map& units, const gamestatus& gamestatus,
							 const gamemap& map)
	: game_savegame(gamestate, level_cfg, gui, teams, units, gamestatus, map)
{
	set_error_message(_("Could not auto save the game. Please save the game manually."));
	create_filename();
}

void autosave_savegame::create_filename()
{
	std::string filename;
	if (gamestate().label.empty())
		filename = _("Auto-Save");
	else
		filename = gamestate().label + "-" + _("Auto-Save") + lexical_cast<std::string>(gamestatus_.turn());

	set_filename(filename);
}

game_savegame::game_savegame(game_state &gamestate, const config& level_cfg, 
							 const game_display& gui, const std::vector<team>& teams, 
							 const unit_map& units, const gamestatus& gamestatus,
							 const gamemap& map) 
	: savegame(gamestate, _("Save Game")),
	level_cfg_(level_cfg), gui_(gui),
	teams_(teams), units_(units),
	gamestatus_(gamestatus), map_(map)
{}

void game_savegame::create_filename()
{
	std::stringstream stream;

	const std::string ellipsed_name = font::make_text_ellipsis(gamestate().label,
			font::SIZE_NORMAL, 200);
	stream << ellipsed_name << " " << _("Turn") << " " << gamestatus_.turn();
	set_filename(stream.str());
}

void game_savegame::before_save()
{
	savegame::before_save();
	write_game_snapshot();
}

void game_savegame::write_game_snapshot()
{
	snapshot().merge_attributes(level_cfg_);

	snapshot()["snapshot"] = "yes";

	std::stringstream buf;
	buf << gui_.playing_team();
	snapshot()["playing_team"] = buf.str();

	for(std::vector<team>::const_iterator t = teams_.begin(); t != teams_.end(); ++t) {
		const unsigned int side_num = t - teams_.begin() + 1;

		config& side = snapshot().add_child("side");
		t->write(side);
		side["no_leader"] = "yes";
		buf.str(std::string());
		buf << side_num;
		side["side"] = buf.str();

		//current visible units
		for(unit_map::const_iterator i = units_.begin(); i != units_.end(); ++i) {
			if(i->second.side() == side_num) {
				config& u = side.add_child("unit");
				i->first.write(u);
				i->second.write(u);
			}
		}
		//recall list
		{
			for(std::map<std::string, player_info>::const_iterator i=gamestate().players.begin();
			i!=gamestate().players.end(); ++i) {
				for(std::vector<unit>::const_iterator j = i->second.available_units.begin();
					j != i->second.available_units.end(); ++j) {
					if (j->side() == side_num){
						config& u = side.add_child("unit");
						j->write(u);
					}
				}
			}
		}
	}

	gamestatus_.write(snapshot());
	game_events::write_events(snapshot());

	// Write terrain_graphics data in snapshot, too
	const config::child_list& terrains = level_cfg_.get_children("terrain_graphics");
	for(config::child_list::const_iterator tg = terrains.begin();
			tg != terrains.end(); ++tg) {

		snapshot().add_child("terrain_graphics", **tg);
	}

	sound::write_music_play_list(snapshot());

	gamestate().write_snapshot(snapshot());

	//write out the current state of the map
	snapshot()["map_data"] = map_.write();

	gui_.labels().write(snapshot());
}

//// Interactive replay save
//void save_replay(std::string filename, game_state& gamestate)
//{
//	// Create an empty snapshot
//	config snapshot;
//
//	// FIXME
//	// Doesn't make too much sense at the moment but will, once
//	// including the savegame dialog here is done
//	//save_game(filename, snapshot, gamestate);
//}
//
//// Automatic replay save at the end of the scenario
//void save_replay(game_state& gamestate)
//{
//	// FIXME:
//	// We create a copy here in order to avoid having to change the original
//	// gamestate. Once we made sure the player information is within
//	// [replay_start] only, this whole stuff is no longer needed.
//	game_state saveInfo = gamestate;
//
//	// If the starting position contains player information, use this for
//	// the replay savegame (this originally comes from the gamestate constructor,
//	// where the player stuff is added to the starting position to be used here.
//	config::child_itors player_list = gamestate.starting_pos.child_range("player");
//	if (player_list.first != player_list.second) {
//		saveInfo.players.clear();
//		saveInfo.load_recall_list(player_list);
//	}
//
//	// Create an empty snapshot
//	config snapshot;
//
//	std::string filename = gamestate.label + _(" replay");
//	//save_game(filename, snapshot, saveInfo);
//
//	// clear replay data and snapshot for the next scenario
//	gamestate.replay_data = config();
//	gamestate.snapshot = config();
//}

///** Autosave */
//std::string save_autosave(unsigned turn, const config& snapshot, game_state& gamestate)
//{
//	std::string filename;
//	if (gamestate.label.empty())
//		filename = _("Auto-Save");
//	else
//		filename = gamestate.label + "-" + _("Auto-Save") + lexical_cast<std::string>(turn);
//
//	//save_game(filename, snapshot, gamestate);
//
//	return filename;
//}

//void save_game(std::string& filename, const config& snapshot, game_state& gamestate)
//{
//	LOG_SAVE << "savegame::save_game";
//
//	if(preferences::compress_saves()) {
//		filename += ".gz";
//	}
//
//	std::stringstream ss;
//	{
//		config_writer out(ss, preferences::compress_saves());
//		::write_game(out, snapshot, gamestate);
//		finish_save_game(out, gamestate, gamestate.label);
//	}
//	scoped_ostream os(open_save_game(filename));
//	(*os) << ss.str();
//
//	if (!os->good()) {
//		throw game::save_game_failed(_("Could not write to file"));
//	}
//}
//
//std::string create_filename(const std::string& filename_base, const unsigned turn)
//{
//	std::stringstream stream;
//
//	const std::string ellipsed_name = font::make_text_ellipsis(filename_base,
//			font::SIZE_NORMAL, 200);
//	stream << ellipsed_name << " " << _("Turn") << " " << turn;
//	std::string filename = stream.str();
//
//	filename.erase(std::remove_if(filename.begin(), filename.end(),
//	            dialogs::is_illegal_file_char), filename.end());
//
//	return filename;
//}
//
//std::string create_replay_filename(const std::string& filename_base)
//{
//	std::stringstream stream;
//
//	const std::string ellipsed_name = font::make_text_ellipsis(filename_base,
//			font::SIZE_NORMAL, 200);
//	stream << ellipsed_name << " " << _("replay");
//	std::string filename = stream.str();
//
//	filename.erase(std::remove_if(filename.begin(), filename.end(),
//	            dialogs::is_illegal_file_char), filename.end());
//
//	return filename;
//}

