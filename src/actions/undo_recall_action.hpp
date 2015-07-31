#pragma once

#include "undo_action.hpp"
#include "shroud_clearing_action.hpp"
#include "../unit_ptr.hpp"
#include "../unit.hpp"

namespace actions
{
namespace undo
{
	

struct recall_action : undo_action, shroud_clearing_action
{
	std::string id;
	map_location recall_from;


	recall_action(const unit_const_ptr recalled, const map_location& loc,
	              const map_location& from)
		: undo_action()
		, shroud_clearing_action(recalled, loc)
		, id(recalled->id())
		, recall_from(from)
	{
	}
	recall_action(const config & cfg, const map_location & from)
		: undo_action(cfg)
		, shroud_clearing_action(cfg)
		, id(cfg["id"])
		, recall_from(from)
	{}
	virtual const char* get_type() const { return "recall"; }
	virtual ~recall_action() {}

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side);
	/// Redoes this action.
	virtual bool redo(int side);
};

}
}
