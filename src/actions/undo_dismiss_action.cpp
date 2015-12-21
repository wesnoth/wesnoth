#include "undo_dismiss_action.hpp"
#include "../resources.hpp"
#include "../team.hpp"
#include "../replay.hpp"

namespace actions
{
namespace undo
{

/**
 * Writes this into the provided config.
 */
void dismiss_action::write(config & cfg) const
{
	undo_action::write(cfg);
	dismissed_unit->write(cfg.add_child("unit"));
}

/**
 * Undoes this action.
 * @return true on success; false on an error.
 */
bool dismiss_action::undo(int side)
{
	team &current_team = (*resources::teams)[side-1];

	current_team.recall_list().add(dismissed_unit);
	execute_undo_umc_wml();
	return true;
}

/**
 * Redoes this action.
 * @return true on success; false on an error.
 */
bool dismiss_action::redo(int side)
{
	team &current_team = (*resources::teams)[side-1];

	resources::recorder->redo(replay_data);
	replay_data.clear();
	current_team.recall_list().erase_if_matches_id(dismissed_unit->id());
	execute_redo_umc_wml();
	return true;
}

}
}
