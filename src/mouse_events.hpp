#ifndef MOUSE_EVENTS_H_INCLUDED
#define MOUSE_EVENTS_H_INCLUDED

#include "global.hpp"

#include "display.hpp"
#include "gamestatus.hpp"
#include "pathfind.hpp"
#include "unit.hpp"

#include "SDL.h"

namespace events{

class mouse_handler{
public:
	mouse_handler(display* gui, std::vector<team>& teams, unit_map& units, gamemap& map, gamestatus& status, const game_data& gameinfo);
	void mouse_motion(const SDL_MouseMotionEvent& event, const int player_number);
	void set_gui(display* gui) { gui_ = gui; }
private:
	team& viewing_team(const int player_number) { return teams_[(*gui_).viewing_team()]; }
	
	void mouse_motion(int x, int y, const int player_number);
	gamemap::location current_unit_attacks_from(const gamemap::location& loc, const gamemap::location::DIRECTION preferred, const gamemap::location::DIRECTION second_preferred, const int player_number);
	unit_map::const_iterator find_unit(const gamemap::location& hex, const int player_number);
	unit_map& visible_units(const int player_number);

	display* gui_;
	std::vector<team>& teams_;
	unit_map& units_;
	gamemap& map_;
	gamestatus& status_;
	const game_data& gameinfo_;

	bool minimap_scrolling_;
	gamemap::location last_hex_;
	gamemap::location selected_hex_;
	gamemap::location::DIRECTION last_nearest_, last_second_nearest_;
	paths::route current_route_;
	paths current_paths_;
	bool enemy_paths_;
	bool browse_;
	mutable unit_map visible_units_;
	int path_turns_;
};

}

#endif
