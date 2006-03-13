#include "play_controller.hpp"

#include "config_adapter.hpp"
#include "log.hpp"
#include "preferences.hpp"
#include "replay.hpp"
#include "sound.hpp"

#define LOG_NG LOG_STREAM(info, engine)

play_controller::play_controller(const config& level, const game_data& gameinfo, game_state& state_of_game, 
								 int ticks, int num_turns, const config& game_config, CVideo& video) : 
	level_(level), ticks_(ticks), first_human_team_(-1), gameinfo_(gameinfo),
	gamestate_(state_of_game), status_(level, num_turns), statistics_context_(level_["name"]),
	game_config_(game_config), map_(game_config, level["map_data"]), verify_manager_(units_),
	labels_manager_(), help_manager_(&game_config, &gameinfo, &map_), 
	team_manager_(teams_), xp_modifier_(atoi(level["experience_modifier"].c_str()))
{
	init(video);
}

play_controller::~play_controller(){
	delete halo_manager_;
	delete prefs_disp_manager_;
	delete tooltips_manager_;
	delete events_manager_;
	delete gui_;
}

void play_controller::init(CVideo& video){
	//if the recorder has no event, adds an "game start" event to the
	//recorder, whose only goal is to initialize the RNG
	if(recorder.empty()) {
		recorder.add_start();
	} else {
		recorder.pre_replay();
	}
	recorder.set_skip(false);

	const config::child_list& unit_cfg = level_.get_children("side");

	if(level_["modify_placing"] == "true") {
		LOG_NG << "modifying placing...\n";
		play::place_sides_in_preferred_locations(map_,unit_cfg);
	}

	LOG_NG << "initializing teams..." << unit_cfg.size() << "\n";;
	LOG_NG << (SDL_GetTicks() - ticks_) << "\n";

	std::set<std::string> seen_save_ids;

	for(config::child_list::const_iterator ui = unit_cfg.begin(); ui != unit_cfg.end(); ++ui) {
		std::string save_id = get_unique_saveid(**ui, seen_save_ids);
		seen_save_ids.insert(save_id);
		if (first_human_team_ == -1){
			first_human_team_ = get_first_human_team(ui, unit_cfg);
		}
		get_player_info(**ui, gamestate_, save_id, teams_, level_, gameinfo_, map_, units_);
	}

	preferences::encounter_recruitable_units(teams_);
	preferences::encounter_start_units(units_);
	preferences::encounter_recallable_units(gamestate_);
	preferences::encounter_map_terrain(map_);

	LOG_NG << "initialized teams... " << (SDL_GetTicks() - ticks_) << "\n";
	LOG_NG << "initializing display... " << (SDL_GetTicks() - ticks_) << "\n";

	const config* theme_cfg = get_theme(game_config_, level_["theme"]);
	gui_ = new display(units_,video,map_,status_,teams_,*theme_cfg, game_config_, level_);
	theme::set_known_themes(&game_config_);

	LOG_NG << "done initializing display... " << (SDL_GetTicks() - ticks_) << "\n";

	if(first_human_team_ != -1) {
		gui_->set_team(first_human_team_);
	}

	init_managers();

	gui_->labels().read(level_);

	LOG_NG << "start music... " << (SDL_GetTicks() - ticks_) << "\n";

	const std::string& music = level_["music"];
	if(music != "") {
		sound::play_music_repeatedly(music);
	}
}

void play_controller::init_managers(){
	LOG_NG << "initializing managers... " << (SDL_GetTicks() - ticks_) << "\n";
	prefs_disp_manager_ = new preferences::display_manager(gui_);
	tooltips_manager_ = new tooltips::manager(gui_->video());

	//this *needs* to be created before the show_intro and show_map_scene
	//as that functions use the manager state_of_game
	events_manager_ = new game_events::manager(level_,*gui_,map_,units_,teams_,
	                                    gamestate_,status_,gameinfo_);

	halo_manager_ = new halo::manager(*gui_);
	LOG_NG << "done initializing managers... " << (SDL_GetTicks() - ticks_) << "\n";
}

const int play_controller::get_xp_modifier(){
	return xp_modifier_;
}

