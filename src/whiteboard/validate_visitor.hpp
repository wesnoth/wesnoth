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

#include "mapbuilder.hpp"
#include "side_actions.hpp"

#include <set>

namespace wb
{

/**
 * Works like mapbuilder, but with these differences:
 *   * Instead of stopping at viewer_team, it visits all actions of every team.
 *   * actions are evaluated for validity along the way.
 *   * Some invalid actions are deleted.
 */
class validate_visitor
	: private visitor
{
public:
	explicit validate_visitor(unit_map& unit_map);
	virtual ~validate_visitor();

	/// @return false some actions had to be deleted during validation,
	/// which may warrant a second validation
	bool validate_actions();

private:
	virtual void visit(move_ptr move);
	virtual void visit(attack_ptr attack);
	virtual void visit(recruit_ptr recruit);
	virtual void visit(recall_ptr recall);
	virtual void visit(suppose_dead_ptr sup_d);

	enum VALIDITY {VALID, OBSTRUCTED, WORTHLESS};
	VALIDITY evaluate_move_validity(move_ptr);

	bool no_previous_invalids(side_actions::iterator const&);

	struct helper: public mapbuilder
	{
		helper(unit_map& umap, validate_visitor& parent)
				: mapbuilder(umap)
				, parent_(parent)
			{}
		virtual void validate(side_actions::iterator const& itor);
		validate_visitor& parent_;
	};
	friend struct helper;

	helper builder_;

	side_actions& viewer_actions_;

	std::set<action_ptr> actions_to_erase_;

	//Parameter for the visit_***() fcns -- see helper::validate()
	side_actions::iterator arg_itor_;
};

}

#endif /* WB_VALIDATE_VISITOR_HPP_ */
