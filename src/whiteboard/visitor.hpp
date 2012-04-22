/* $Id$ */
/*
 Copyright (C) 2010 - 2012 by Gabriel Morin <gabrielmorin (at) gmail (dot) com>
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
 * Two classes are presented in this file: enable_visit_all and visitor.
 * enable_visit_all is a class template that provides the all-too-common iteration
 *   code that iterates over every planned action from every team, starting with the
 *   current-turn team. Using "static polymorphism," derived classes can customize
 *   the iteration by overriding some functions.
 * visitor is an abstract interface that simply provides derived classes with the
 *   mechanism for the double dispatch used in the Visitor Pattern:
 *       action.accept(visitor)   calls    visitor.visit_***(action)
 * Derived classes will usually derive from both of these.
 * Example usage is seen is highlight_visitor and mapbuilder.
 */

#ifndef WB_VISITOR_HPP_
#define WB_VISITOR_HPP_

#include "action.hpp"
#include "side_actions.hpp"

#include "foreach.hpp"
#include "play_controller.hpp"
#include "resources.hpp"

#include <boost/noncopyable.hpp>

namespace wb
{

/**
 * enable_visit_all: A base class template, using the so-called CRTP. This is the
 *   "static polymorphism" part.
 * If you want visit_all() and reverse_visit_all() in your class,
 *   you should derive from this class template.
 * Derived classes can "override" the following:
 *   visit (required), pre_visit_team, post_visit_team
 * Derived classes should declare enable_visit_all<Derived> as a friend, or
 *   else make the overridden functions public.
 */
template<typename Derived>
class enable_visit_all
{
public:
	void visit_all() { visit_all_helper(false); }
	void reverse_visit_all() { visit_all_helper(true); }

protected:
	/**
	 * visit():
	 *   @return Whether or not to continue any visitation after this action.
	 * The fcn is commented out because derived classes are required to implement it.
	 */
	//bool visit(size_t team_index, team&, side_actions&, side_actions::iterator);

	///@return Whether or not to visit any of the contents of sa.
	bool pre_visit_team(size_t /*turn*/, size_t /*team_index*/, team&, side_actions& sa) { return !sa.hidden(); }
	///@return Whether or not to visit any more teams after this one.
	bool post_visit_team(size_t /*turn*/, size_t /*team_index*/, team&, side_actions&) { return true; }

private:
	void visit_all_helper( const bool reverse)
	{
		if( resources::teams == NULL ){
			return; //< Early abort
		}

		Derived* const new_this = static_cast<Derived*>(this);

		//Determine how many turns' worth of plans there are
		size_t max_turns = 0;
		foreach(team& t, *resources::teams)
			max_turns = std::max(max_turns,t.get_side_actions()->num_turns());

		size_t const current_team = resources::controller->current_side() - 1;
		size_t const num_teams = resources::teams->size();
		//for each turn with any planned actions
		for(size_t turn_iter=0; turn_iter<max_turns; ++turn_iter)
		{
			size_t const turn = (reverse? max_turns-1-turn_iter: turn_iter);

			//for each team
			for(size_t team_iter = 0; team_iter < num_teams; ++team_iter)
			{
				size_t const team_index
						= (current_team+num_teams+(reverse? -1-team_iter: team_iter)) % num_teams;
				team& t = resources::teams->at(team_index);
				side_actions& sa = *t.get_side_actions();
				if(!new_this->pre_visit_team(turn_iter, team_index, t, sa))
					continue; //< Skip this team's actions

				if(reverse)
				{
					side_actions::rrange_t acts = sa.riter_turn(turn);
					side_actions::reverse_iterator itor = acts.first;
					side_actions::reverse_iterator end  = acts.second;
					while(itor!=end) {
						++itor;
						if(!new_this->process(team_index,t,sa,itor.base()))
							return; //< Early abort
					}
				}
				else //forward
				{
					side_actions::range_t  acts = sa.iter_turn(turn);
					side_actions::iterator itor = acts.first;
					side_actions::iterator end  = acts.second;
					for(; itor!=end; ++itor)
						if(!new_this->process(team_index,t,sa,itor))
							return; //< Early abort
				}
				if(!new_this->post_visit_team(turn_iter, team_index, t, sa))
					break; //< Early abort
			}
		}
	}
};

/**
 * visitor: This is the "dynamic polymorphism" part.
 * Abstract base class for all the visitors (cf GoF Visitor Design Pattern)
 *   the whiteboard uses.
 * visitor does not inherit from enable_visit_all because it tends to make it more
 *   "difficult" for derived classes to inherit from both visitor and enable_visit_all.
 */
class visitor
	: private boost::noncopyable
{

public:

	virtual void visit(move_ptr move) = 0;
	virtual void visit(attack_ptr attack) = 0;
	virtual void visit(recruit_ptr recruit) = 0;
	virtual void visit(recall_ptr recall) = 0;
	virtual void visit(suppose_dead_ptr sup_d) = 0;

protected:
	visitor() {}
	virtual ~visitor() {} //Not intended for polymorphic deletion

	/**
	 * This function is available for derived classes so they can inherit from enable_visit_all
	 * without having to override visit(); i.e., the below implementation can be used as a
	 * default.
	 */
	bool process(size_t /*team_index*/, team&, side_actions&, side_actions::iterator itor)
		{ (*itor)->accept(*this);   return true; }

	//Utility fcn for derived classes that don't need to customize the iteration.
	void visit_all_actions() {visitor_helper::visit_all_actions_helper(this);}

private:
	struct visitor_helper
		: enable_visit_all<visitor_helper>
	{
		bool process(size_t /*team_index*/, team&, side_actions&, side_actions::iterator itor)
			{ (*itor)->accept(*v_);   return true; }

		static void visit_all_actions_helper(visitor* v)
		{
			static visitor_helper vh;
			vh.v_ = v;
			vh.visit_all();
		}

		visitor* v_;
	};
};

}

#endif /* WB_VISITOR_HPP_ */
