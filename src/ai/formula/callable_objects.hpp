/*
   Copyright (C) 2009 - 2017 by Bartosz Waresiak <dragonking@o2.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_AI_CALLABLE_OBJECTS_HPP_INCLUDED
#define	FORMULA_AI_CALLABLE_OBJECTS_HPP_INCLUDED

#include "ai/game_info.hpp"
#include "actions/attack.hpp"
#include "formula/callable_objects.hpp"
#include "formula/formula.hpp"
#include "formula/callable.hpp"

namespace ai {
	class formula_ai;
}

namespace wfl {

class attack_map_callable : public formula_callable {
public:
	typedef std::multimap<map_location, map_location> move_map;
	attack_map_callable(const ai::formula_ai& ai, const unit_map& units)
		: units_(units), ai_(ai)
	{}
private:
	const unit_map& units_;
	const ai::formula_ai& ai_;

	variant get_value(const std::string& key) const;

	void get_inputs(formula_input_vector* inputs) const;

	/* add to vars all attacks on enemy units around <attack_position> tile. attacker_location is tile where unit is currently standing. It's moved to attack_position first and then performs attack.*/
	void collect_possible_attacks(std::vector<variant>& vars, map_location attacker_location, map_location attack_position) const;
};

class attack_callable : public action_callable {
	map_location move_from_, src_, dst_;
	battle_context bc_;
	variant get_value(const std::string& key) const;

	void get_inputs(formula_input_vector* inputs) const;
public:
	attack_callable(const map_location& move_from,
			    const map_location& src, const map_location& dst, int weapon);

	const map_location& move_from() const { return move_from_; }
	const map_location& src() const { return src_; }
	const map_location& dst() const { return dst_; }
	int weapon() const { return bc_.get_attacker_stats().attack_num; }
	int defender_weapon() const { return bc_.get_defender_stats().attack_num; }

	/** Compare two attacks in deterministic way or compare pointers
	 * (nondeterministic in consequent game runs) if method argument is not
	 * attack_callable */
	int do_compare(const formula_callable* callable) const;
	variant execute_self(variant ctxt) override;
};

class move_callable : public action_callable {
	map_location src_, dst_;
	variant get_value(const std::string& key) const {
		if(key == "src") {
			return variant(new location_callable(src_));
		} else if(key == "dst") {
			return variant(new location_callable(dst_));
		} else {
			return variant();
		}
	}
	void get_inputs(formula_input_vector* inputs) const {
		add_input(inputs, "src");
		add_input(inputs, "dst");
	}

	int do_compare(const formula_callable* callable) const;
public:
	move_callable(const map_location& src, const map_location& dst) :
	  src_(src), dst_(dst)
	{
		type_ = MOVE_C;
	}

	const map_location& src() const { return src_; }
	const map_location& dst() const { return dst_; }
	variant execute_self(variant ctxt) override;
};

class move_partial_callable : public action_callable {
	map_location src_, dst_;
	variant get_value(const std::string& key) const {
		if(key == "src") {
			return variant(new location_callable(src_));
		} else if(key == "dst") {
			return variant(new location_callable(dst_));
		} else {
			return variant();
		}
	}
	void get_inputs(formula_input_vector* inputs) const {
		add_input(inputs, "src");
		add_input(inputs, "dst");
	}

	int do_compare(const formula_callable* callable) const;
public:
	move_partial_callable(const map_location& src, const map_location& dst) :
	  src_(src), dst_(dst)
	{
		type_ = MOVE_PARTIAL_C;
	}

	const map_location& src() const { return src_; }
	const map_location& dst() const { return dst_; }
	variant execute_self(variant ctxt) override;
};

class recall_callable : public action_callable {
	map_location loc_;
	std::string id_;

	variant get_value(const std::string& key) const;

	void get_inputs(formula_input_vector* inputs) const;
public:
	recall_callable(const map_location& loc, const std::string& id)
	  : loc_(loc), id_(id)
	{}

	const map_location& loc() const { return loc_; }
	const std::string& id() const { return id_; }
	variant execute_self(variant ctxt) override;
};

class recruit_callable : public action_callable {
	map_location loc_;
	std::string type_;

	variant get_value(const std::string& key) const;

	void get_inputs(formula_input_vector* inputs) const;
public:
	recruit_callable(const map_location& loc, const std::string& type)
	  : loc_(loc), type_(type)
	{}

	const map_location& loc() const { return loc_; }
	const std::string& type() const { return type_; }
	variant execute_self(variant ctxt) override;
};

class set_unit_var_callable : public action_callable {
	std::string key_;
	variant value_;
	map_location loc_;
	variant get_value(const std::string& key) const;

	void get_inputs(formula_input_vector* inputs) const;
public:
	set_unit_var_callable(const std::string& key, const variant& value, const map_location& loc)
	  : key_(key), value_(value), loc_(loc)
	{}

	const std::string& key() const { return key_; }
	variant value() const { return value_; }
	const map_location loc() const { return loc_; }
	variant execute_self(variant ctxt) override;
};

class fallback_callable : public action_callable {
	variant get_value(const std::string& /*key*/) const { return variant(); }
public:
	explicit fallback_callable() {
	}
	variant execute_self(variant ctxt) override;
};

class move_map_callable : public formula_callable {
	typedef std::multimap<map_location, map_location> move_map;
	const move_map& srcdst_;
	const move_map& dstsrc_;
        const unit_map& units_;

	variant get_value(const std::string& key) const;
	void get_inputs(formula_input_vector* inputs) const;
public:
	move_map_callable(const move_map& srcdst, const move_map& dstsrc, const unit_map& units)
	  : srcdst_(srcdst), dstsrc_(dstsrc), units_(units)
	{
		type_ = MOVE_MAP_C;
	}

	const move_map& srcdst() const { return srcdst_; }
	const move_map& dstsrc() const { return dstsrc_; }
};

class position_callable : public formula_callable {
	//unit_map units_;
	int chance_;
	variant get_value(const std::string& key) const;

	void get_inputs(formula_input_vector* inputs) const;
public:
	position_callable(/*unit_map* units,*/ int chance) :
		//units_(),
		chance_(chance)
	{
		//units->swap(units_);
	}

	struct move_map_backup {
		move_map_backup() :
			srcdst(),
			dstsrc(),
			full_srcdst(),
			full_dstsrc(),
			enemy_srcdst(),
			enemy_dstsrc(),
			attacks_cache()
		{
		}

		ai::move_map srcdst, dstsrc, full_srcdst, full_dstsrc, enemy_srcdst, enemy_dstsrc;
		variant attacks_cache;
	};
};


class outcome_callable : public formula_callable {
	std::vector<variant> hitLeft_, prob_, status_;
	variant get_value(const std::string& key) const;

	void get_inputs(formula_input_vector* inputs) const;
public:
	outcome_callable(		const std::vector<variant>& hitLeft,
					const std::vector<variant>& prob,
					const std::vector<variant>& status)
	  : hitLeft_(hitLeft), prob_(prob), status_(status)
	{
	}

	const std::vector<variant>& hitLeft() const { return hitLeft_; }
	const std::vector<variant>& prob() const { return prob_; }
	const std::vector<variant>& status() const { return status_; }
};

}

#endif	/* FORMULA_AI_CALLABLE_OBJECTS_HPP_INCLUDED */

