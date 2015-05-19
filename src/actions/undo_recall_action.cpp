#include "undo_recall_action.hpp"
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
void recall_action::write(config & cfg) const
{
	undo_action::write(cfg);
	shroud_clearing_action::write(cfg);

	recall_from.write(cfg.add_child("leader"));
	cfg["id"] = id;
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool recall_action::undo(int side)
{
	game_display & gui = *resources::screen;
	unit_map &   units = *resources::units;
	team &current_team = (*resources::teams)[side-1];

	const map_location & recall_loc = route.front();
	unit_map::iterator un_it = units.find(recall_loc);
	if ( un_it == units.end() ) {
		return false;
	}

	unit_ptr un = un_it.get_shared_ptr();
	if (!un) {
		return false;
	}

	statistics::un_recall_unit(*un);
	int cost = statistics::un_recall_unit_cost(*un);
	if (cost < 0) {
		current_team.spend_gold(-current_team.recall_cost());
	}
	else {
		current_team.spend_gold(-cost);
	}

	current_team.recall_list().add(un);
	// invalidate before erasing allow us
	// to also do the overlapped hexes
	gui.invalidate(recall_loc);
	units.erase(recall_loc);
	return true;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool recall_action::redo(int side)
{
	game_display & gui = *resources::screen;
	team &current_team = (*resources::teams)[side-1];

	map_location loc = route.front();
	map_location from = recall_from;

	unit_ptr un = current_team.recall_list().find_if_matches_id(id);
	if ( !un ) {
		ERR_NG << "Trying to redo a recall of '" << id
		       << "', but that unit is not in the recall list.";
		return false;
	}

	const std::string &msg = find_recall_location(side, loc, from, *un);
	if ( msg.empty() ) {
		resources::recorder->redo(replay_data);
		replay_data.clear();
		set_scontext_synced sync;
		recall_unit(id, current_team, loc, from, true, false);

		// Quick error check. (Abuse of [allow_undo]?)
		if ( loc != route.front() ) {
			ERR_NG << "When redoing a recall at " << route.front()
			       << ", the location was moved to " << loc << ".\n";
			// Not really fatal, I suppose. Just update the action so
			// undoing this works.
			route.front() = loc;
		}
		sync.do_final_checkup();
	} else {
		gui::dialog(gui, "", msg, gui::OK_ONLY).show();
		return false;
	}

	return true;
}
}
}
