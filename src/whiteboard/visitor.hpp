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

#ifndef WB_VISITOR_HPP_
#define WB_VISITOR_HPP_

#include "action.hpp"
#include "side_actions.hpp"
#include "typedefs.hpp"

#include "play_controller.hpp"
#include "resources.hpp"
#include "team.hpp"

#include <boost/noncopyable.hpp>

namespace wb
{

/**
 * visitor_base: A base class template, using the so-called CRTP
 * If you want visit_all() and reverse_visit_all() in your class,
 *   you should derive from this class.
 * Derived classes can "override" the following:
 *   visit, pre_visit_team, post_visit_team
 * Derived classes should declare visitor_base<Derived> as a friend, or
 *   else make the overridden functions public.
 * I recommend making the inheritance private or protected.
 */
template<typename Derived>
class visitor_base
{
public:
	void visit_all() {visit_all_helper<false>();}
	void reverse_visit_all() {visit_all_helper<true>();}

protected:
	/**
	 * visit():
	 *   @return Whether or not to continue any visitation after this action.
	 * The fcn is commented out because derived classes are required to implement it.
	 */
	//bool visit(size_t team_index, team&, side_actions&, side_actions::iterator);

	///@return Whether or not to visit any of the contents of sa.
	bool pre_visit_team(size_t team_index, team& t, side_actions& sa) {return true;}
	///@return Whether or not to visit any more teams after this one.
	bool post_visit_team(size_t team_index, team& t, side_actions& sa) {return true;}

private:
	template<bool reverse>
	void visit_all_helper()
	{
		Derived* const new_this = static_cast<Derived*>(this);

		size_t const current_team = resources::controller->current_side() - 1;
		size_t const num_teams = resources::teams->size();
		for(size_t iteration = 0; iteration < num_teams; ++iteration)
		{
			size_t const team_index
					= (current_team+num_teams+(reverse? -1-iteration: iteration)) % num_teams;
			team& t = resources::teams->at(team_index);
			side_actions& sa = *t.get_side_actions();
			if(!new_this->pre_visit_team(team_index,t,sa))
				continue; //< Skip this team's actions

			if(reverse)
			{
				side_actions::reverse_iterator itor = sa.rbegin();
				side_actions::reverse_iterator end  = sa.rend();
				while(itor!=end) {
					++itor;
					if(!new_this->visit(team_index,t,sa,itor.base()))
						return; //< Early abort
				}
			}
			else //forward
			{
				side_actions::iterator itor = sa.begin();
				side_actions::iterator end  = sa.end();
				for(; itor!=end; ++itor)
					if(!new_this->visit(team_index,t,sa,itor))
						return; //< Early abort
			}
			if(!new_this->post_visit_team(team_index,t,sa))
				break; //< Early abort
		}
	}
};

/**
 * Abstract base class for all the visitors (cf GoF Visitor Design Pattern)
 * the whiteboard uses.
 */
class visitor : private boost::noncopyable
{
	friend class move;
	friend class attack;
	friend class recruit;
	friend class recall;
	friend class suppose_dead;

protected:
	visitor() {}
	virtual ~visitor() {} //Not intended for polymorphic deletion

	void visit_all_actions(); //< weird utility function for derived classes

	virtual void visit_move(move_ptr move) = 0;
	virtual void visit_attack(attack_ptr attack) = 0;
	virtual void visit_recruit(recruit_ptr recruit) = 0;
	virtual void visit_recall(recall_ptr recall) = 0;
	virtual void visit_suppose_dead(suppose_dead_ptr sup_d) = 0;
};

}

#endif /* WB_VISITOR_HPP_ */
