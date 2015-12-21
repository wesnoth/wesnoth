#pragma once

#include "vision.hpp"
#include "../map_location.hpp"
#include "../unit_ptr.hpp"

#include <boost/noncopyable.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/optional.hpp>
namespace actions
{
/// base class for classes that clear srhoud (move/recruit/recall)
struct shroud_clearing_action
{

	shroud_clearing_action(const config& cfg)
		: route()
		, view_info(cfg.child_or_empty("unit"))
	{
		read_locations(cfg, route);
	}

	shroud_clearing_action(const unit_const_ptr u, const map_location& loc)
		: route(1, loc)
		, view_info(*u)
	{

	}

	typedef std::vector<map_location> t_route;

	shroud_clearing_action(const unit_const_ptr u, const t_route::const_iterator& begin, const t_route::const_iterator& end)
		: route(begin, end)
		, view_info(*u)
	{

	}

	/// The hexes occupied by the affected unit during this action.
	/// For recruits and recalls this only contains one hex.
	t_route route;
	/// A record of the affected unit's ability to see.
	clearer_info view_info;

	void write(config & cfg) const
	{
		write_locations(route, cfg);
		view_info.write(cfg.add_child("unit"));
	}

	virtual ~shroud_clearing_action() {}
};
}