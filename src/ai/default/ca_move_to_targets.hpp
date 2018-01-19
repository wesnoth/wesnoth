/*
   Copyright (C) 2009 - 2018 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Strategic movement routine, for experimentation
 */

#pragma once

#include "ai/composite/rca.hpp"

#include "units/map.hpp"

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace pathfind {

struct plain_route;

} //of namespace pathfind

namespace ai {

namespace ai_default_rca {

class move_to_targets_phase : public candidate_action {
public:

	move_to_targets_phase( rca_context &context, const config &cfg );

	virtual ~move_to_targets_phase();

	virtual double evaluate();

	virtual void execute();

protected:
	void access_points(const move_map& srcdst, const map_location& u, const map_location& dst, std::vector<map_location>& out);

	std::pair<map_location,map_location> choose_move(std::vector<target>& targets);

	double compare_groups(const std::set<map_location>& our_group, const std::set<map_location>& their_group, const std::vector<map_location>& battlefield) const;

	void enemies_along_path(const std::vector<map_location>& route, const move_map& dstsrc, std::set<map_location>& res);

	map_location form_group(const std::vector<map_location>& route, const move_map& dstsrc, std::set<map_location>& res);

	bool move_group(const map_location& dst, const std::vector<map_location>& route, const std::set<map_location>& units);

	double rate_group(const std::set<map_location>& group, const std::vector<map_location>& battlefield) const;

	/**
	 * rate a target, but can also return the maximal possible rating
	 * by passing a dummy route
	*/
	double rate_target(const target& tg, const unit_map::iterator& u,
			const move_map& dstsrc, const move_map& enemy_dstsrc,
			const pathfind::plain_route& rt);

	bool should_retreat(const map_location& loc, const unit_map::const_iterator& un,
			    const move_map& srcdst, const move_map& dstsrc, const move_map& enemy_dstsrc,
			    double caution);
};

} // of namespace testing_ai_default

} // of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif
