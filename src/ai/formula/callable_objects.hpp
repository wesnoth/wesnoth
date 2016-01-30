/*
   Copyright (C) 2009 - 2016 by Bartosz Waresiak <dragonking@o2.pl>
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

#include "../game_info.hpp"
#include "../../actions/attack.hpp"
#include "../../callable_objects.hpp"
#include "../../formula.hpp"
#include "../../formula_callable.hpp"

namespace ai {
	class formula_ai;
}

namespace game_logic {

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

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

	/* add to vars all attacks on enemy units around <attack_position> tile. attacker_location is tile where unit is currently standing. It's moved to attack_position first and then performs attack.*/
	void collect_possible_attacks(std::vector<variant>& vars, map_location attacker_location, map_location attack_position) const;
};

class attack_callable : public formula_callable {
	map_location move_from_, src_, dst_;
	battle_context bc_;
	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
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
	int do_compare(const game_logic::formula_callable* callable) const;
};


class move_callable : public game_logic::formula_callable {
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
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("src", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("dst", game_logic::FORMULA_READ_ONLY));
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
};


class move_partial_callable : public game_logic::formula_callable {
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
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const {
		inputs->push_back(game_logic::formula_input("src", game_logic::FORMULA_READ_ONLY));
		inputs->push_back(game_logic::formula_input("dst", game_logic::FORMULA_READ_ONLY));
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
};



class recall_callable : public formula_callable {
	map_location loc_;
	std::string id_;

	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
public:
	recall_callable(const map_location& loc, const std::string& id)
	  : loc_(loc), id_(id)
	{}

	const map_location& loc() const { return loc_; }
	const std::string& id() const { return id_; }
};


class recruit_callable : public formula_callable {
	map_location loc_;
	std::string type_;

	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
public:
	recruit_callable(const map_location& loc, const std::string& type)
	  : loc_(loc), type_(type)
	{}

	const map_location& loc() const { return loc_; }
	const std::string& type() const { return type_; }
};


class set_var_callable : public formula_callable {
	std::string key_;
	variant value_;
	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
public:
	set_var_callable(const std::string& key, const variant& value)
	  : key_(key), value_(value)
	{}

	const std::string& key() const { return key_; }
	variant value() const { return value_; }
};


class set_unit_var_callable : public formula_callable {
	std::string key_;
	variant value_;
	map_location loc_;
	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
public:
	set_unit_var_callable(const std::string& key, const variant& value, const map_location& loc)
	  : key_(key), value_(value), loc_(loc)
	{}

	const std::string& key() const { return key_; }
	variant value() const { return value_; }
	const map_location loc() const { return loc_; }
};

class fallback_callable : public formula_callable {
	std::string key_;
	variant get_value(const std::string& /*key*/) const { return variant(); }
public:
	explicit fallback_callable(const std::string& key) : key_(key) {
	}

	const std::string& key() const { return key_; }
};

class safe_call_callable : public formula_callable {
	variant main_;
	variant backup_;
	expression_ptr backup_formula_;
	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
public:
	safe_call_callable(const variant& main, const expression_ptr& backup)
		: main_(main)
		, backup_()
		, backup_formula_(backup)
	{}

	const variant& get_main() const { return main_; }
	const expression_ptr& get_backup() const { return backup_formula_; }

	void set_backup_result(const variant& v) {
		backup_ = v;
	}
};


class safe_call_result : public formula_callable {
	const formula_callable* failed_callable_;
	const map_location current_unit_location_;
	const int status_;

	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

public:
	safe_call_result(const formula_callable* callable, int status,
			    const map_location& loc = map_location() )
	  : failed_callable_(callable), current_unit_location_(loc), status_(status)
	{}
};


class move_map_callable : public game_logic::formula_callable {
	typedef std::multimap<map_location, map_location> move_map;
	const move_map& srcdst_;
	const move_map& dstsrc_;
        const unit_map& units_;

	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
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

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
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

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
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

