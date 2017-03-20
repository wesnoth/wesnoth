#include "actions/undo_update_shroud_action.hpp"
#include "replay.hpp"

namespace actions
{
namespace undo
{


/**
 * Writes this into the provided config.
 */
void auto_shroud_action::write(config & cfg) const
{
	undo_action_base::write(cfg);
	cfg["active"] = active;
}

/**
 * Writes this into the provided config.
 */
void update_shroud_action::write(config & cfg) const
{
	undo_action_base::write(cfg);
}


}
}
