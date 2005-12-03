#include "global.hpp"

#include "ai_interface.hpp"
#include "config_adapter.hpp"
#include "cursor.hpp"
#include "dialogs.hpp"
#include "events.hpp"
#include "filesystem.hpp"
#include "game_errors.hpp"
#include "gamestatus.hpp"
#include "gettext.hpp"
#include "game_events.hpp"
#include "halo.hpp"
#include "help.hpp"
#include "hotkeys.hpp"
#include "intro.hpp"
#include "log.hpp"
#include "mapgen.hpp"
#include "map_create.hpp"
#include "network.hpp"
#include "playlevel.hpp"
#include "playturn.hpp"
#include "preferences.hpp"
#include "preferences_display.hpp"
#include "random.hpp"
#include "replay.hpp"
#include "replay_controller.hpp"
#include "scoped_resource.hpp"
#include "sound.hpp"
#include "statistics.hpp"
#include "tooltips.hpp"
#include "unit_display.hpp"
#include "util.hpp"
#include "video.hpp"

#include <iostream>
#include <iterator>

#define LOG_NG LOG_STREAM(info, engine)

LEVEL_RESULT play_replay_level(const game_data& gameinfo, const config& game_config,
		const config* level, CVideo& video, game_state& state_of_game,
		const std::vector<config*>& story)
{
	try{
		const int ticks = SDL_GetTicks();
		const int num_turns = atoi((*level)["turns"].c_str());
		replay_controller replaycontroller(*level, gameinfo, state_of_game, ticks, num_turns, game_config, video, story);

		//replay event-loop
		for (;;){
			replaycontroller.replay_slice();
		}
	}
	catch(end_level_exception&){
	}

	return LEVEL_CONTINUE;
}

replay_controller::replay_controller(const config& level, const game_data& gameinfo, game_state& state_of_game,
						   const int ticks, const int num_turns, const config& game_config,
						   CVideo& video, const std::vector<config*>& story) :
	level_(level), gameinfo_(gameinfo), gamestate_(state_of_game), gamestate_start_(state_of_game), ticks_(ticks),
	status_(level, num_turns), status_start_(level, num_turns), map_(game_config, level["map_data"]),
	game_config_(game_config), team_manager_(teams_), verify_manager_(units_), help_manager_(&game_config, &gameinfo, &map_),
	labels_manager_(), xp_modifier_(atoi(level["experience_modifier"].c_str())),
	mouse_handler_(gui_, teams_, units_, map_, status_, gameinfo)
{
	player_number_ = 1;
	delay_ = 0;
	is_playing_ = false;
	current_turn_ = 1;
	loading_game_ = level["playing_team"].empty() == false;
	first_player_ = atoi(level_["playing_team"].c_str());
	if(first_player_ < 0 || first_player_ >= int(teams_.size())) {
		first_player_ = 0;
	}
	init(video, story);
}

replay_controller::~replay_controller(){
	delete gui_;
	delete halo_manager_;
	delete prefs_disp_manager_;
	delete tooltips_manager_;
	delete events_manager_;
}

void replay_controller::init(CVideo& video, const std::vector<config*>& story){
	//if the recorder has no event, adds an "game start" event to the
	//recorder, whose only goal is to initialize the RNG
	if(recorder.empty()) {
		recorder.add_start();
	} else {
		recorder.pre_replay();
	}
	recorder.set_skip(false);
	replay_network_sender replay_sender(recorder);

	const set_random_generator generator_setter(&recorder);

	//guarantee the cursor goes back to 'normal' at the end of the level
	const cursor::setter cursor_setter(cursor::NORMAL);

	const int ticks = SDL_GetTicks();
	LOG_NG << "in replay_controller::init()...\n";

	const statistics::scenario_context statistics_context(level_["name"]);

	LOG_NG << "created objects... " << (SDL_GetTicks() - ticks) << "\n";

	const unit_type::experience_accelerator xp_mod(xp_modifier_ > 0 ? xp_modifier_ : 100);

	const config::child_list& unit_cfg = level_.get_children("side");

	if(level_["modify_placing"] == "true") {
		LOG_NG << "modifying placing...\n";
		play::place_sides_in_preferred_locations(map_,unit_cfg);
	}

	LOG_NG << "initializing teams..." << unit_cfg.size() << "\n";;
	LOG_NG << (SDL_GetTicks() - ticks) << "\n";
	int first_human_team = -1;
	std::set<std::string> seen_save_ids;

	for(config::child_list::const_iterator ui = unit_cfg.begin(); ui != unit_cfg.end(); ++ui) {
		std::string save_id = get_unique_saveid(**ui, seen_save_ids);
		seen_save_ids.insert(save_id);
		if (first_human_team == -1){
			first_human_team = get_first_human_team(ui, unit_cfg);
		}
		get_player_info(**ui, gamestate_, save_id, teams_, level_, gameinfo_, map_, units_);
	}

	preferences::encounter_recruitable_units(teams_);
	preferences::encounter_start_units(units_);
	preferences::encounter_recallable_units(gamestate_);
	preferences::encounter_map_terrain(map_);
	LOG_NG << "initialized teams... " << (SDL_GetTicks() - ticks) << "\n";

	LOG_NG << "initializing display... " << (SDL_GetTicks() - ticks) << "\n";
	const config* theme_cfg = get_theme(game_config_, level_["theme"]);
	gui_ = new display(units_,video,map_,status_,teams_,*theme_cfg, game_config_, level_);
	mouse_handler_.set_gui(gui_);
	theme::set_known_themes(&game_config_);
	LOG_NG << "done initializing display... " << (SDL_GetTicks() - ticks) << "\n";

	if(first_human_team != -1) {
		gui_->set_team(first_human_team);
	}

	init_managers();

	/*
	if(recorder.is_skipping() == false) {
		for(std::vector<config*>::const_iterator story_i = story.begin(); story_i != story.end(); ++story_i) {

			show_intro(*gui_,**story_i, level_);
		}
	}
	*/

	gui_->labels().read(level_);
	LOG_NG << "c... " << (SDL_GetTicks() - ticks) << "\n";

	const std::string& music = level_["music"];
	if(music != "") {
		sound::play_music(music);
	}

	LOG_NG << "d... " << (SDL_GetTicks() - ticks) << "\n";

	//find a list of 'items' (i.e. overlays) on the level, and add them
	const config::child_list& overlays = level_.get_children("item");
	for(config::child_list::const_iterator overlay = overlays.begin(); overlay != overlays.end(); ++overlay) {
		gui_->add_overlay(gamemap::location(**overlay),(**overlay)["image"], (**overlay)["halo"]);
	}

	game_events::fire("prestart");
	gui_->begin_game();
	gui_->adjust_colours(0,0,0);

	std::deque<config> data_backlog;

	const hotkey::basic_handler key_events_handler(gui_);

	LOG_NG << "first_time..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";

	update_locker lock_display((*gui_).video(),false);
	for(std::vector<team>::iterator t = teams_.begin(); t != teams_.end(); ++t) {
		clear_shroud(*gui_,status_,map_,gameinfo_,units_,teams_,(t-teams_.begin()));
		t->set_fog(false);
		t->set_shroud(false);
	}

	LOG_NG << "scrolling... " << (SDL_GetTicks() - ticks) << "\n";
	gui_->scroll_to_leader(units_, player_number_);
	LOG_NG << "done scrolling... " << (SDL_GetTicks() - ticks) << "\n";

	if(!loading_game_) {
		game_events::fire("start");
		gamestate_.set_variable("turn_number", "1");
	}

	update_gui();

	units_start_ = units_;
	teams_start_ = team_manager_.clone(teams_);
}

void replay_controller::init_managers(){
	prefs_disp_manager_ = new preferences::display_manager(gui_);
	tooltips_manager_ = new tooltips::manager(gui_->video());

	//this *needs* to be created before the show_intro and show_map_scene
	//as that functions use the manager state_of_game
	events_manager_ = new game_events::manager(level_,*gui_,map_,units_,teams_,
	                                    gamestate_,status_,gameinfo_);

	halo_manager_ = new halo::manager(*gui_);
}

std::vector<team>& replay_controller::get_teams(){
	return teams_;
}

unit_map replay_controller::get_units(){
	return units_;
}

display& replay_controller::get_gui(){
	return *gui_;
}

gamemap& replay_controller::get_map(){
	return map_;
}

const gamestatus& replay_controller::get_status(){
	return status_;
}

const int replay_controller::get_player_number(){
	return player_number_;
}

const bool replay_controller::is_loading_game(){
	return loading_game_;
}

void replay_controller::objectives(){
	menu_handler_.objectives(*gui_, level_, teams_[player_number_]);
}

void replay_controller::show_statistics(){
	menu_handler_.show_statistics(*gui_, gameinfo_);
}

void replay_controller::unit_list(){
	menu_handler_.unit_list(units_, *gui_, map_);
}

void replay_controller::status_table(){
	menu_handler_.status_table(teams_, *gui_, units_);
}

void replay_controller::save_game(){
	menu_handler_.save_game("",gui::OK_CANCEL, gamestate_, status_, *gui_, level_, teams_, units_, map_);
}

void replay_controller::load_game(){
	menu_handler_.load_game(*gui_, game_config_, gameinfo_);
}

void replay_controller::preferences(){
	menu_handler_.preferences(*gui_, game_config_);
}

void replay_controller::show_chat_log(){
	menu_handler_.show_chat_log(teams_, *gui_);
}

void replay_controller::show_help(){
	menu_handler_.show_help(*gui_);
}

void replay_controller::reset_replay(){
	is_playing_ = false;
	current_turn_ = 1;
	recorder.start_replay();
	units_ = *(new unit_map(units_start_));
	status_ = *(new gamestatus(status_start_));
	gamestate_ = *(new game_state(gamestate_start_));
	teams_ = team_manager_.clone(teams_start_);
	(*gui_).invalidate_all();
	(*gui_).draw();
}

void replay_controller::stop_replay(){
	is_playing_ = false;
}

void replay_controller::replay_next_turn(){
	is_playing_ = true;
	play_turn();
	is_playing_ = false;
}

void replay_controller::replay_next_side(){
	is_playing_ = true;
	play_side(player_number_ - 1);
	if (player_number_ > teams_.size()){
		player_number_ = 1;
		current_turn_++;
	}
	is_playing_ = false;
}

void replay_controller::replay_switch_fog(){
	for(std::vector<team>::iterator t = teams_.begin(); t != teams_.end(); ++t) {
		t->set_fog(!t->uses_fog());
	}
	update_teams();
	update_gui();
}

void replay_controller::replay_switch_shroud(){
	for(std::vector<team>::iterator t = teams_.begin(); t != teams_.end(); ++t) {
		t->set_shroud(!t->uses_shroud());
	}
	update_teams();
	update_gui();
}

void replay_controller::replay_skip_animation(){
	recorder.set_skip(!recorder.is_skipping());
}

void replay_controller::play_replay(){
	if (recorder.at_end()){
		return;
	}

	try{
		is_playing_ = true;

		LOG_NG << "starting main loop\n" << (SDL_GetTicks() - ticks_) << "\n";
		for(; !recorder.at_end() && is_playing_; first_player_ = 0) {
			play_turn();
			replay_slice();
		} //end for loop
		is_playing_ = false;
	}
	catch(end_level_exception&){
	}
}

void replay_controller::play_turn(){
	if (recorder.at_end()){
		return;
	}

	//FixMe
	//This is a little bit ugly at the moment, since it really should happen only once in a game
	//Probably need to fix the unit xp_max calculation
	const unit_type::experience_accelerator xp_mod(xp_modifier_ > 0 ? xp_modifier_ : 100);

	LOG_NG << "turn: " << current_turn_ << "\n";

	while (player_number_ <= teams_.size()) {
		play_side(player_number_ - 1);
	}

	std::stringstream event_stream;
	event_stream << status_.turn();

	{
		LOG_NG << "turn event..." << (recorder.is_skipping() ? "skipping" : "no skip") << "\n";
		update_locker lock_display(gui_->video(),recorder.is_skipping());
		const std::string turn_num = event_stream.str();
		gamestate_.set_variable("turn_number",turn_num);
		game_events::fire("turn " + turn_num);
		game_events::fire("new turn");
	}

	player_number_ = 1;
	current_turn_++;
}

void replay_controller::play_side(const int team_index){
	if (recorder.at_end()){
		return;
	}

	team& current_team = teams_[team_index];
	log_scope("player turn");

	//FixMe
	//This is a little bit ugly at the moment, since it really should happen only once in a game
	//Probably need to fix the unit xp_max calculation
	const unit_type::experience_accelerator xp_mod(xp_modifier_ > 0 ? xp_modifier_ : 100);

	//if a side is dead, don't do their turn
	if(!current_team.is_empty() && team_units(units_,player_number_) > 0) {
		if(team_manager_.is_observer()) {
			(*gui_).set_team(size_t(player_number_-1));
		}

		std::stringstream player_number_str;
		player_number_str << player_number_;
		gamestate_.set_variable("side_number",player_number_str.str());

		//fire side turn event only if real side change occurs not counting changes from void to a side
		if (team_index != first_player_ || current_turn_ > 1) {
			game_events::fire("side turn");
		}

		//we want to work out if units for this player should get healed, and the
		//player should get income now. healing/income happen if it's not the first
		//turn of processing, or if we are loading a game, and this is not the
		//player it started with.
		const bool turn_refresh = current_turn_ > 1 || loading_game_ && team_index != first_player_;

		if(turn_refresh) {
			for(unit_map::iterator i = units_.begin(); i != units_.end(); ++i) {
				if(i->second.side() == player_number_) {
					i->second.new_turn();
				}
			}

			current_team.new_turn();

			//if the expense is less than the number of villages owned,
			//then we don't have to pay anything at all
			const int expense = team_upkeep(units_,player_number_) -
								current_team.villages().size();
			if(expense > 0) {
				current_team.spend_gold(expense);
			}

			calculate_healing((*gui_),status_,map_,units_,player_number_,teams_);
		}

		current_team.set_time_of_day(int(status_.turn()),status_.get_time_of_day());

		gui_->set_playing_team(size_t(player_number_-1));

		if (!recorder.is_skipping()){
			clear_shroud(*gui_,status_,map_,gameinfo_,units_,teams_,player_number_-1);
		}

		const hotkey::basic_handler key_events_handler(gui_);
		LOG_NG << "doing replay " << player_number_ << "\n";
		bool replaying;
		try {
			replaying = do_replay(*gui_,map_,gameinfo_,units_,teams_,
								  player_number_,status_,gamestate_);
		} catch(replay::error&) {
			gui::show_dialog(*gui_,NULL,"",_("The file you have tried to load is corrupt"),gui::OK_ONLY);

			replaying = false;
		}
		LOG_NG << "result of replay: " << (replaying?"true":"false") << "\n";

		for(unit_map::iterator uit = units_.begin(); uit != units_.end(); ++uit) {
			if(uit->second.side() == player_number_){
				uit->second.end_turn();
			}
			else{
				//this is necessary for replays in order to show possible movements
				uit->second.new_turn();
			}
		}

		//This implements "delayed map sharing." It's meant as an alternative to shared vision.
		if(current_team.copy_ally_shroud()) {
			gui_->recalculate_minimap();
			gui_->invalidate_all();
		}

		game_events::pump();

		//check_victory(units_,teams_);
	}

	player_number_++;

	if (player_number_ > teams_.size()) {
		status_.next_turn();
	}
	update_teams();
	update_gui();
}

void replay_controller::update_teams(){
	int next_team = player_number_;
	if (next_team > teams_.size()) { next_team = 1; }
	if (teams_[next_team - 1].uses_fog()){
		gui_->set_team(next_team - 1);
		clear_shroud(*gui_, status_, map_, gameinfo_, units_, teams_, next_team - 1);
	}
	if (teams_[next_team - 1].uses_shroud()){
		gui_->set_team(next_team - 1);
		recalculate_fog(map_, status_, gameinfo_, units_, teams_, next_team - 1);
	}
	gui_->set_playing_team(next_team - 1);
	(*gui_).scroll_to_leader(units_, next_team);
}

void replay_controller::update_gui(){
	(*gui_).recalculate_minimap();
	(*gui_).redraw_minimap();
	(*gui_).invalidate_all();
	events::raise_draw_event();
	(*gui_).draw();
}

void replay_controller::replay_slice()
{
	CKey key;

	events::pump();
	events::raise_process_event();

	events::raise_draw_event();

	const theme::menu* const m = gui_->menu_pressed();
	if(m != NULL) {
		const SDL_Rect& menu_loc = m->location(gui_->screen_area());
		show_menu(m->items(),menu_loc.x+1,menu_loc.y + menu_loc.h + 1,false,*gui_);
		return;
	}

	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	tooltips::process(mousex, mousey);

	const int scroll_threshold = 5;

	if(key[SDLK_UP] || mousey < scroll_threshold)
		gui_->scroll(0,-preferences::scroll_speed());

	if(key[SDLK_DOWN] || mousey > gui_->y()-scroll_threshold)
		gui_->scroll(0,preferences::scroll_speed());

	if(key[SDLK_LEFT] || mousex < scroll_threshold)
		gui_->scroll(-preferences::scroll_speed(),0);

	if(key[SDLK_RIGHT] || mousex > gui_->x()-scroll_threshold)
		gui_->scroll(preferences::scroll_speed(),0);

	gui_->draw();

	if(!mouse_handler_.browse() && teams_[player_number_].objectives_changed()) {
		dialogs::show_objectives(*gui_, level_, teams_[player_number_].objectives());
		teams_[player_number_].reset_objectives_changed();
	}
}

bool replay_controller::can_execute_command(hotkey::HOTKEY_COMMAND command) const
{
	switch(command) {

	//commands we can always do
	case hotkey::HOTKEY_LEADER:
	case hotkey::HOTKEY_CYCLE_UNITS:
	case hotkey::HOTKEY_CYCLE_BACK_UNITS:
	case hotkey::HOTKEY_ZOOM_IN:
	case hotkey::HOTKEY_ZOOM_OUT:
	case hotkey::HOTKEY_ZOOM_DEFAULT:
	case hotkey::HOTKEY_FULLSCREEN:
	case hotkey::HOTKEY_SCREENSHOT:
	case hotkey::HOTKEY_ACCELERATED:
	case hotkey::HOTKEY_TOGGLE_GRID:
	case hotkey::HOTKEY_STATUS_TABLE:
	case hotkey::HOTKEY_MUTE:
	case hotkey::HOTKEY_PREFERENCES:
	case hotkey::HOTKEY_OBJECTIVES:
	case hotkey::HOTKEY_UNIT_LIST:
	case hotkey::HOTKEY_STATISTICS:
	case hotkey::HOTKEY_QUIT_GAME:
	case hotkey::HOTKEY_SEARCH:
	case hotkey::HOTKEY_HELP:
	case hotkey::HOTKEY_USER_CMD:
	case hotkey::HOTKEY_SAVE_GAME:
	case hotkey::HOTKEY_LOAD_GAME:
	case hotkey::HOTKEY_PLAY_REPLAY:
	case hotkey::HOTKEY_RESET_REPLAY:
	case hotkey::HOTKEY_STOP_REPLAY:
	case hotkey::HOTKEY_REPLAY_NEXT_TURN:
	case hotkey::HOTKEY_REPLAY_NEXT_SIDE:
	case hotkey::HOTKEY_REPLAY_FOG:
	case hotkey::HOTKEY_REPLAY_SHROUD:
	case hotkey::HOTKEY_REPLAY_SKIP_ANIMATION:
		return true;

	case hotkey::HOTKEY_CHAT_LOG:
		return network::nconnections() > 0;

	default:
		return false;
	}
}

void replay_controller::handle_event(const SDL_Event& event)
{
	if(gui::in_dialog()) {
		return;
	}

	switch(event.type) {
	case SDL_KEYDOWN:
		hotkey::key_event(*gui_,event.key,this);
		//FixMe
		//add textbox support
		/*
		//detect key press events, unless there is a textbox present on-screen
		//in which case the key press events should go only to it.
		if(textbox_.active() == false) {
			hotkey::key_event(gui_,event.key,this);
		} else if(event.key.keysym.sym == SDLK_ESCAPE) {
			close_textbox();
		} else if(event.key.keysym.sym == SDLK_TAB) {
			tab_textbox();
		} else if(event.key.keysym.sym == SDLK_RETURN) {
			enter_textbox();
		}
		*/
		//intentionally fall-through
	case SDL_KEYUP:
		//FixMe
		//add unit-turn support
		/*
		//if the user has pressed 1 through 9, we want to show how far
		//the unit can move in that many turns
		if(event.key.keysym.sym >= '1' && event.key.keysym.sym <= '7') {
			const int new_path_turns = (event.type == SDL_KEYDOWN) ?
			                           event.key.keysym.sym - '1' : 0;

			if(new_path_turns != path_turns_) {
				path_turns_ = new_path_turns;

				const unit_map::iterator u = selected_unit();

				if(u != units_.end() && u->second.side() == team_num_) {
					const bool ignore_zocs = u->second.type().is_skirmisher();
					const bool teleport = u->second.type().teleports();
					current_paths_ = paths(map_,status_,gameinfo_,units_,u->first,
					                       teams_,ignore_zocs,teleport,
					                       path_turns_);
					gui_->set_paths(&current_paths_);
				}
			}
		}
		*/
		break;
	case SDL_MOUSEMOTION:
		// ignore old mouse motion events in the event queue
		SDL_Event new_event;

		if(SDL_PeepEvents(&new_event,1,SDL_GETEVENT,
					SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0) {
			while(SDL_PeepEvents(&new_event,1,SDL_GETEVENT,
						SDL_EVENTMASK(SDL_MOUSEMOTION)) > 0);
			mouse_handler_.mouse_motion(new_event.motion, player_number_);
		} else {
			mouse_handler_.mouse_motion(event.motion, player_number_);
		}
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
		mouse_handler_.mouse_press(event.button, player_number_);
		break;
	default:
		break;
	}

}

