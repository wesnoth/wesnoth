/* $Id$ */
/*
 Copyright (C) 2010 - 2011 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org

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
 */

#ifndef WB_VALIDATE_VISITOR_HPP_
#define WB_VALIDATE_VISITOR_HPP_

#include "mapbuilder_visitor.hpp"
#include "side_actions.hpp"

#include <set>

namespace wb
{

/**
 * Works like mapbuilder_visitor, but with these differences:
 *   * Instead of stopping at viewer_team, it visits all actions of every team.
 *   * actions are evaluated for validity along the way.
 *   * Some invalid actions are deleted.
 */
class validate_visitor
	: private mapbuilder_visitor
	, private visitor_base<validate_visitor>
{
	friend class visitor_base<validate_visitor>;

public:
	explicit validate_visitor(unit_map& unit_map);
	virtual ~validate_visitor();

	/// @return false some actions had to be deleted during validation,
	/// which may warrant a second validation
	bool validate_actions();

private:
	virtual void visit_move(move_ptr move);
	virtual void visit_attack(attack_ptr attack);
	virtual void visit_recruit(recruit_ptr recruit);
	virtual void visit_recall(recall_ptr recall);
	virtual void visit_suppose_dead(suppose_dead_ptr sup_d);

	enum VALIDITY {VALID, OBSTRUCTED, WORTHLESS};
	VALIDITY evaluate_move_validity(move_ptr);
	bool no_previous_invalids(side_actions::iterator const&);

	//"Inherited" from visitor_base
	bool visit(size_t team_index, team& t, side_actions& sa, side_actions::iterator itor)
		{ arg_itor_=itor;   return visitor::visit(team_index,t,sa,itor); }
	using mapbuilder_visitor::pre_visit_team;
	using visitor_base<validate_visitor>::post_visit_team;
	using visitor_base<validate_visitor>::visit_all;

	side_actions& viewer_actions_;

	std::set<action_ptr> actions_to_erase_;

	//Parameter for the visit_***() fcns -- see validate_actions()
	side_actions::iterator arg_itor_;
};

}

#endif /* WB_VALIDATE_VISITOR_HPP_ */
