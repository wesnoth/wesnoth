#include "undo_recruit_action.hpp"
#include "create.hpp"

#include "../construct_dialog.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../replay.hpp"
#include "../unit_map.hpp"
#include "../statistics.hpp"
#include "../log.hpp"
#include "../game_display.hpp"

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
void recruit_action::write(config & cfg) const
{
	undo_action::write(cfg);
	shroud_clearing_action::write(cfg);

	recruit_from.write(cfg.add_child("leader"));
	config & child = cfg.child("unit");
	child["type"] = u_type.base_id();
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool recruit_action::undo(int side)
{
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side-1];

	const map_location & recruit_loc = route.front();
	unit_map::iterator un_it = units.find(recruit_loc);
	if ( un_it == units.end() ) {
		return false;
	}

	const unit &un = *un_it;
	statistics::un_recruit_unit(un);
	current_team.spend_gold(-un.type().cost());

	//MP_COUNTDOWN take away recruit bonus
	current_team.set_action_bonus_count(current_team.action_bonus_count() - 1);

	// invalidate before erasing allow us
	// to also do the overlapped hexes
	gui.invalidate(recruit_loc);
	units.erase(recruit_loc);
	this->return_village();
	execute_undo_umc_wml();
	return true;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool recruit_action::redo(int side)
{
	game_display & gui = *resources::screen;
	team &current_team = (*resources::teams)[side-1];

	map_location loc = route.front();
	map_location from = recruit_from;
	const std::string & name = u_type.base_id();

	//search for the unit to be recruited in recruits
	if ( !util::contains(get_recruits(side, loc), name) ) {
		ERR_NG << "Trying to redo a recruit for side " << side
			<< ", which does not recruit type \"" << name << "\"\n";
		assert(false);
		return false;
	}

	current_team.last_recruit(name);
	const std::string &msg = find_recruit_location(side, loc, from, name);
	if ( msg.empty() ) {
		//MP_COUNTDOWN: restore recruitment bonus
		current_team.set_action_bonus_count(1 + current_team.action_bonus_count());
		resources::recorder->redo(replay_data);
		replay_data.clear();
		set_scontext_synced sync;
		recruit_unit(u_type, side, loc, from, true, false);

		// Quick error check. (Abuse of [allow_undo]?)
		if ( loc != route.front() ) {
			ERR_NG << "When redoing a recruit at " << route.front()
			       << ", the location was moved to " << loc << ".\n";
			// Not really fatal, I suppose. Just update the action so
			// undoing this works.
			route.front() = loc;
		}
		sync.do_final_checkup();
	} else {
		gui::dialog(gui.video(), "", msg, gui::OK_ONLY).show();
		return false;
	}

	return true;
}
}
}
