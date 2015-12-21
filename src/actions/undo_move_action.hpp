#pragma once

#include "undo_action.hpp"
#include "shroud_clearing_action.hpp"
#include "../unit_ptr.hpp"
#include "../unit.hpp"

namespace actions
{
namespace undo
{

struct move_action : undo_action, shroud_clearing_action
{
	int starting_moves;
	int original_village_owner;
	int countdown_time_bonus;
	map_location::DIRECTION starting_dir;
	map_location goto_hex;


	move_action(const unit_const_ptr moved,
	            const std::vector<map_location>::const_iterator & begin,
	            const std::vector<map_location>::const_iterator & end,
	            int sm, int timebonus, int orig, const map_location::DIRECTION dir)
		: undo_action()
		, shroud_clearing_action(moved, begin, end)
		, starting_moves(sm)
		, original_village_owner(orig)
		, countdown_time_bonus(timebonus)
		, starting_dir(dir == map_location::NDIRECTIONS ? moved->facing() : dir)
		, goto_hex(moved->get_goto())
	{
	}
	move_action(const config & cfg, const config & unit_cfg,
	            int sm, int timebonus, int orig, const map_location::DIRECTION dir)
		: undo_action(cfg)
		, shroud_clearing_action(cfg)
		, starting_moves(sm)
		, original_village_owner(orig)
		, countdown_time_bonus(timebonus)
		, starting_dir(dir)
		, goto_hex(unit_cfg["goto_x"].to_int(-999) - 1,
		         unit_cfg["goto_y"].to_int(-999) - 1)
	{
	}
	virtual const char* get_type() const { return "move"; }
	virtual ~move_action() {}

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side);
	/// Redoes this action.
	virtual bool redo(int side);
};

}
}
