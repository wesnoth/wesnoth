#pragma once

#include "undo_action.hpp"
#include "units/ptr.hpp"
#include "units/unit.hpp"

namespace actions
{
namespace undo
{

struct dismiss_action : undo_action
{
	unit_ptr dismissed_unit;


	explicit dismiss_action(const unit_const_ptr dismissed)
		: undo_action()
		, dismissed_unit(new unit(*dismissed))
	{
	}
	explicit dismiss_action(const config & cfg, const config & unit_cfg)
		: undo_action(cfg)
		, dismissed_unit(new unit(unit_cfg))
	{
	}
	virtual const char* get_type() const { return "dismiss"; }
	virtual ~dismiss_action() {};

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;

	/// Undoes this action.
	virtual bool undo(int side);
	/// Redoes this action.
	virtual bool redo(int side);
};

}
}
