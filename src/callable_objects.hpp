/* $Id$ */
/*
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


#ifndef CALLABLE_OBJECTS_HPP_INCLUDED
#define CALLABLE_OBJECTS_HPP_INCLUDED

#include <map>

#include "formula_callable.hpp"
#include "map.hpp"
#include "unit.hpp"
#include "foreach.hpp"

#define CALLABLE_WRAPPER_START(klass) \
class klass##_callable : public game_logic::formula_callable { \
	const klass& object_; \
public: \
	explicit klass##_callable(const klass& object) : object_(object) \
	{} \
	\
	const klass& get_##klass() const { return object_; } \
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const \
	{ \
		using game_logic::FORMULA_READ_ONLY;

#define CALLABLE_WRAPPER_INPUT(VAR) \
	inputs->push_back(game_logic::formula_input(#VAR, FORMULA_READ_ONLY));

#define CALLABLE_WRAPPER_INPUT_END \
	} \
	\
	variant get_value(const std::string& key) const {

#define CALLABLE_WRAPPER_VAR(VAR) \
	if(key == #VAR) { \
		return variant(object_.VAR); \
	} else

#define CALLABLE_WRAPPER_FN(VAR) \
	if(key == #VAR) { \
		return variant(object_.VAR()); \
	} else



#define CALLABLE_WRAPPER_END \
		{ return variant(); } \
	} \
};

template <typename T, typename K> variant convert_map( const std::map<T,K>& map );
template <typename T> variant convert_vector( const std::vector<T>& input_vector );

class terrain_callable : public game_logic::formula_callable {
public:
	typedef map_location location;
	terrain_callable(const terrain_type& t, const location loc)
	  : loc_(loc), t_(t)
	{}

	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
private:
        int do_compare(const formula_callable* callable) const;
        
	const location loc_;
	const terrain_type &t_;
};

CALLABLE_WRAPPER_START(gamemap)
CALLABLE_WRAPPER_INPUT(terrain)
CALLABLE_WRAPPER_INPUT(w)
CALLABLE_WRAPPER_INPUT(h)
CALLABLE_WRAPPER_INPUT_END
	if(key == "terrain") {
		int w = object_.w();
		int h = object_.h();
		std::vector<variant> vars;
		for(int i = 0;i < w; i++) {
			for(int j = 0;j < h; j++) {
				const map_location loc(i,j);
				vars.push_back(variant(new terrain_callable(object_.get_terrain_info(loc), loc)));
			}
		}
		return variant(&vars);
	} else
	CALLABLE_WRAPPER_FN(w)
	CALLABLE_WRAPPER_FN(h)
CALLABLE_WRAPPER_END

class location_callable : public game_logic::formula_callable {
	map_location loc_;

	variant get_value(const std::string& key) const;

	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;
	int do_compare(const game_logic::formula_callable* callable) const;

public:
	explicit location_callable(const map_location& loc) : loc_(loc)
	{}
	explicit location_callable(int x, int y) : loc_(map_location(x,y))
	{}

	const map_location& loc() const { return loc_; }

	void serialize_to_string(std::string& str) const;
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
	{}

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
	{}

	const map_location& src() const { return src_; }
	const map_location& dst() const { return dst_; }
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
	{}

	const move_map& srcdst() const { return srcdst_; }
	const move_map& dstsrc() const { return dstsrc_; }
};

class attack_type_callable : public game_logic::formula_callable {
public:
	typedef map_location location;
	attack_type_callable(const attack_type attack)
	  : att_(attack)
	{}

	const attack_type& get_attack_type() const { return att_; }
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

        int do_compare(const formula_callable* callable) const;
private:
	const attack_type att_;
};

class unit_callable : public game_logic::formula_callable {
public:
	typedef map_location location;
	unit_callable(const std::pair<location, unit>& pair)
	  : loc_(pair.first), u_(pair.second)
	{}

	const unit& get_unit() const { return u_; }
	const location& get_location() const { return loc_; }
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

        int do_compare(const formula_callable* callable) const;
private:
	const location& loc_;
	const unit& u_;
};

class unit_type_callable : public game_logic::formula_callable {
public:
	unit_type_callable(const unit_type& u)
	  : u_(u)
	{}

	const unit_type& get_unit_type() const { return u_; }
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<game_logic::formula_input>* inputs) const;

        int do_compare(const formula_callable* callable) const;
private:
	const unit_type& u_;
};

CALLABLE_WRAPPER_START(team)
CALLABLE_WRAPPER_INPUT(gold)
CALLABLE_WRAPPER_INPUT(start_gold)
CALLABLE_WRAPPER_INPUT(base_income)
CALLABLE_WRAPPER_INPUT(village_gold)
CALLABLE_WRAPPER_INPUT(name)
CALLABLE_WRAPPER_INPUT(is_human)
CALLABLE_WRAPPER_INPUT(is_ai)
CALLABLE_WRAPPER_INPUT(is_network)
CALLABLE_WRAPPER_INPUT_END
CALLABLE_WRAPPER_FN(gold)
	if(key == "start_gold") { \
		return variant(lexical_cast<int>(object_.start_gold())); \
	} else
CALLABLE_WRAPPER_FN(base_income)
CALLABLE_WRAPPER_FN(village_gold)
CALLABLE_WRAPPER_FN(name)
CALLABLE_WRAPPER_FN(is_human)
CALLABLE_WRAPPER_FN(is_ai)
CALLABLE_WRAPPER_FN(is_network)
CALLABLE_WRAPPER_END
#endif
