#include "undo_move_action.hpp"
#include "move.hpp"

#include "../construct_dialog.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../replay.hpp"
#include "../unit_map.hpp"
#include "../unit_animation_component.hpp"
#include "../log.hpp"
#include "../game_display.hpp"
#include "../unit_display.hpp"
#include "../game_board.hpp"
#include "../map.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define LOG_NG LOG_STREAM(info, log_engine)

namespace actions
{
namespace undo
{

	
/**
 * Writes this into the provided config.
 */
void move_action::write(config & cfg) const
{
	undo_action::write(cfg);
	shroud_clearing_action::write(cfg);
	cfg["starting_direction"] = map_location::write_direction(starting_dir);
	cfg["starting_moves"] = starting_moves;
	cfg["time_bonus"] = countdown_time_bonus;
	cfg["village_owner"] = original_village_owner;
	config & child = cfg.child("unit");
	child["goto_x"] = goto_hex.x + 1;
	child["goto_y"] = goto_hex.y + 1;
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool move_action::undo(int side)
{
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side-1];

	// Copy some of our stored data.
	const int saved_moves = starting_moves;
	std::vector<map_location> rev_route = route;
	std::reverse(rev_route.begin(), rev_route.end());

	// Check units.
	unit_map::iterator u = units.find(rev_route.front());
	const unit_map::iterator u_end = units.find(rev_route.back());
	if ( u == units.end()  ||  u_end != units.end() ) {
		//this can actually happen if the scenario designer has abused the [allow_undo] command
		ERR_NG << "Illegal 'undo' found. Possible abuse of [allow_undo]?" << std::endl;
		return false;
	}

	if ( resources::gameboard->map().is_village(rev_route.front()) ) {
		get_village(rev_route.front(), original_village_owner + 1);
		//MP_COUNTDOWN take away capture bonus
		if ( countdown_time_bonus )
		{
			current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);
		}
	}

	// Record the unit's current state so it can be redone.
	starting_moves = u->movement_left();
	goto_hex = u->get_goto();

	// Move the unit.
	unit_display::move_unit(rev_route, u.get_shared_ptr(), true, starting_dir);
	units.move(u->get_location(), rev_route.back());
	unit::clear_status_caches();

	// Restore the unit's old state.
	u = units.find(rev_route.back());
	u->set_goto(map_location());
	u->set_movement(saved_moves, true);
	u->anim_comp().set_standing();

	gui.invalidate_unit_after_move(rev_route.front(), rev_route.back());
	return true;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool move_action::redo(int side)
{
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side-1];

	// Check units.
	unit_map::iterator u = units.find(route.front());
	if ( u == units.end() ) {
		ERR_NG << "Illegal movement 'redo'." << std::endl;
		assert(false);
		return false;
	}

	// Adjust starting moves.
	const int saved_moves = starting_moves;
	starting_moves = u->movement_left();

	// Move the unit.
	unit_display::move_unit(route, u.get_shared_ptr());
	units.move(u->get_location(), route.back());
	u = units.find(route.back());
	unit::clear_status_caches();

	// Set the unit's state.
	u->set_goto(goto_hex);
	u->set_movement(saved_moves, true);
	u->anim_comp().set_standing();

	if ( resources::gameboard->map().is_village(route.back()) ) {
		get_village(route.back(), u->side());
		//MP_COUNTDOWN restore capture bonus
		if ( countdown_time_bonus )
		{
			current_team.set_action_bonus_count(1 + current_team.action_bonus_count());
		}
	}

	gui.invalidate_unit_after_move(route.front(), route.back());
	resources::recorder->redo(replay_data);
	replay_data.clear();
	return true;
}

}
}
