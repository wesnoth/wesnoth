#pragma once

#include "undo_action.hpp"
#include "units/ptr.hpp"
#include "units/unit.hpp"

namespace actions
{
namespace undo
{

struct auto_shroud_action : undo_action_base {
	bool active;

	explicit auto_shroud_action(bool turned_on)
		: undo_action_base()
		, active(turned_on)
	{}
	virtual const char* get_type() const { return "auto_shroud"; }
	virtual ~auto_shroud_action() {}

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;
};

struct update_shroud_action : undo_action_base {
	// No additional data.

	update_shroud_action()
		: undo_action_base()
	{}
	virtual const char* get_type() const { return "update_shroud"; }
	virtual ~update_shroud_action() {}

	/// Writes this into the provided config.
	virtual void write(config & cfg) const;
};

}
}
