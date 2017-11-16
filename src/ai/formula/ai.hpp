/*
   Copyright (C) 2008 - 2017 by David White <dave@whitevine.net>
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
 * Defines formula ai
 * */

#pragma once

#include "ai/contexts.hpp"
#include "ai/formula/function_table.hpp"           // for ai_function_symbol_table
#include "ai/formula/callable_objects.hpp"         // for position_callable, etc
#include "ai/formula/candidates.hpp"               // for candidate_action_ptr, etc
#include "config.hpp"                   // for config
#include "formula/callable.hpp"         // for formula_callable, etc
#include "formula/formula_fwd.hpp"              // for const_formula_ptr, etc
#include "generic_event.hpp"  // for observer
#include "pathfind/teleport.hpp"  // for teleport_map
#include "units/map.hpp"
#include <set>                          // for multiset
#include <string>                       // for string
#include <utility>                      // for pair
#include <vector>                       // for vector

namespace ai { class ai_context; }
namespace pathfind { struct plain_route; }  // lines 57-57
struct map_location;

namespace wfl {
	struct formula_error;
	class variant;
}

#ifdef _MSC_VER
#pragma warning(push)
//silence "inherits via dominance" warnings
#pragma warning(disable:4250)
#endif

namespace wfl {

typedef std::pair< unit_map::unit_iterator, int> unit_formula_pair;

struct unit_formula_compare {
        bool operator() (const unit_formula_pair& left,
                        const unit_formula_pair& right) const
        {
                return left.second > right.second;
        }
};

typedef std::multiset< unit_formula_pair, unit_formula_compare > unit_formula_set;

}

namespace pathfind {

struct plain_route;

} // of namespace pathfind

namespace ai {

class formula_ai : public readonly_context_proxy, public wfl::formula_callable {
public:
	formula_ai(const formula_ai&) = delete;
	formula_ai& operator=(const formula_ai&) = delete;

	explicit formula_ai(readonly_context &context, const config &cfg);
	virtual ~formula_ai() {}

	virtual config to_config() const;

	std::string evaluate(const std::string& formula_str);

	virtual void add_formula_function(const std::string& name, wfl::const_formula_ptr formula, wfl::const_formula_ptr precondition, const std::vector<std::string>& args);

#if 0
	//class responsible for looking for possible infinite loops when calling set_var or set_unit_var
	class gamestate_change_observer : public events::observer
	{
		static const int MAX_CALLS = 1000;
		int set_var_counter_;
		int set_unit_var_counter_;
		int continue_counter_;
	public:
		gamestate_change_observer();
		~gamestate_change_observer();

		void handle_generic_event(const std::string& /*event_name*/);

		bool set_var_check();

		bool set_unit_var_check();

		bool continue_check();
	};
#endif

	typedef wfl::position_callable::move_map_backup move_map_backup;

	void swap_move_map(move_map_backup& backup);

	wfl::variant get_keeps() const;

	void on_create();

	int get_recursion_count() const override;

	const wfl::variant& get_keeps_cache() const { return keeps_cache_; }

	// Check if given unit can reach another unit
	bool can_reach_unit(map_location unit_A, map_location unit_B) const;

	void handle_exception(wfl::formula_error& e) const;
	void handle_exception(wfl::formula_error& e, const std::string& failed_operation) const;

	pathfind::teleport_map get_allowed_teleports(unit_map::iterator& unit_it) const;
	pathfind::plain_route shortest_path_calculator(const map_location& src, const map_location& dst, unit_map::iterator& unit_it, pathfind::teleport_map& allowed_teleports) const;

	/** Create a new formula from the string, using the symbol table which is stored in the AI.
	*
	*   @param formula_string the string from which a formula should be created
	*   @return pointer to created function or
	*   @retval game_logic::formula_ptr() if there were any problems
	*/
	wfl::formula_ptr create_optional_formula(const std::string& formula_string) const;

	wfl::candidate_action_ptr load_candidate_action_from_config(const config& cfg);

	/** Evaluate the fai candidate action */
	void evaluate_candidate_action(wfl::candidate_action_ptr fai_ca);

	/**
	 * Execute the fai candidate action
	 * @return true if game state was changed
	 * @return false if game state was not changed
	 */
	bool execute_candidate_action(wfl::candidate_action_ptr fai_ca);

	void set_ai_context(ai_context *context);

	wfl::variant make_action(wfl::const_formula_ptr formula_, const wfl::formula_callable& variables);

private:
	ai_context *ai_ptr_;
	const config cfg_;
	recursion_counter recursion_counter_;
	void display_message(const std::string& msg) const;
	virtual wfl::variant get_value(const std::string& key) const override;
	void set_value(const std::string& key, const wfl::variant& value) override;
	virtual void get_inputs(wfl::formula_input_vector& inputs) const override;

	mutable wfl::variant keeps_cache_;
	wfl::attack_map_callable attacks_callable;

//	gamestate_change_observer infinite_loop_guardian_;
	wfl::map_formula_callable vars_;
	// Making this mutable is a bit of a hack, but I don't have a better idea.
	// (It's needed because a formula could define new functions.)
	mutable wfl::ai_function_symbol_table function_table_;

	friend class ai_default;
	friend ai_context& get_ai_context(wfl::const_formula_callable_ptr for_fai);
};

} //end of namespace ai

#ifdef _MSC_VER
#pragma warning(pop)
#endif
