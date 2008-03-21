/* $Id$ */
/*
   Copyright (C) 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_AI_HPP_INCLUDED
#define FORMULA_AI_HPP_INCLUDED

#include "ai.hpp"
#include "ai_interface.hpp"
#include "formula_fwd.hpp"
#include "formula_callable.hpp"

class formula_ai : public ai {
public:
	explicit formula_ai(info& i);
	virtual void play_turn();

	using ai_interface::get_info;
	using ai_interface::current_team;
	using ai_interface::move_map;

	const move_map& srcdst() const { if(!move_maps_valid_) { prepare_move(); } return srcdst_; }

	std::string evaluate(const std::string& formula_str);

	struct move_map_backup {
		move_map_backup() : move_maps_valid(false) {}
		bool move_maps_valid;
		move_map srcdst, dstsrc, full_srcdst, full_dstsrc, enemy_srcdst, enemy_dstsrc;
		variant attacks_cache;
	};

	void swap_move_map(move_map_backup& backup);

private:
	void do_recruitment();
	bool make_move(game_logic::const_formula_ptr formula_, const game_logic::formula_callable& variables);
	virtual variant get_value(const std::string& key) const;
	virtual void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	game_logic::const_formula_ptr recruit_formula_;
	game_logic::const_formula_ptr move_formula_;

	mutable std::map<location,paths> possible_moves_;

	void prepare_move() const;
	mutable bool move_maps_valid_;
	mutable move_map srcdst_, dstsrc_, full_srcdst_, full_dstsrc_, enemy_srcdst_, enemy_dstsrc_;
	mutable variant attacks_cache_;
	mutable variant keeps_cache_;

	variant get_keeps() const;

	game_logic::map_formula_callable vars_;
};

#endif
