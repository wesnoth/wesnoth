#ifndef MOUSE_EVENTS_H_INCLUDED
#define MOUSE_EVENTS_H_INCLUDED

#include "global.hpp"

#include "actions.hpp"
#include "display.hpp"
#include "gamestatus.hpp"
#include "pathfind.hpp"
#include "unit.hpp"

#include "SDL.h"

namespace events{

class mouse_handler{
public:
	mouse_handler(display* gui, std::vector<team>& teams, const unit_map& units, gamemap& map, gamestatus& status, const game_data& gameinfo);
	bool browse();
	void mouse_motion(const SDL_MouseMotionEvent& event, const int player_number);
	void mouse_press(const SDL_MouseButtonEvent& event, const int player_number);
	void set_gui(display* gui) { gui_ = gui; }
private:
	team& viewing_team() { return teams_[(*gui_).viewing_team()]; }
	team& current_team() { return teams_[team_num_-1]; }

	void mouse_motion(int x, int y);
	bool is_left_click(const SDL_MouseButtonEvent& event);
	bool is_middle_click(const SDL_MouseButtonEvent& event);
	bool is_right_click(const SDL_MouseButtonEvent& event);
	void left_click(const SDL_MouseButtonEvent& event);
	void show_attack_options(unit_map::const_iterator u);
	gamemap::location current_unit_attacks_from(const gamemap::location& loc, const gamemap::location::DIRECTION preferred, const gamemap::location::DIRECTION second_preferred);
	unit_map::const_iterator find_unit(const gamemap::location& hex);
	const unit_map& visible_units();

	display* gui_;
	std::vector<team>& teams_;
	const unit_map& units_;
	gamemap& map_;
	gamestatus& status_;
	const game_data& gameinfo_;

	bool minimap_scrolling_;
	gamemap::location last_hex_;
	gamemap::location selected_hex_;
	gamemap::location::DIRECTION last_nearest_, last_second_nearest_;
	gamemap::location next_unit_;
	paths::route current_route_;
	paths current_paths_;
	bool enemy_paths_;
	bool browse_;
	mutable unit_map visible_units_;
	int path_turns_;
	int team_num_;
	undo_list undo_stack_;
	undo_list redo_stack_;

	//cached value indicating whether any enemy units are visible.
	//computed with enemies_visible()
	bool enemies_visible_;
};

}

#endif
