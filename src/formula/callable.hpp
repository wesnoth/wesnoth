/*
   Copyright (C) 2008 - 2016 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef FORMULA_CALLABLE_HPP_INCLUDED
#define FORMULA_CALLABLE_HPP_INCLUDED

#include "reference_counted_object.hpp"
#include "formula/variant.hpp"

namespace game_logic
{

enum FORMULA_ACCESS_TYPE { FORMULA_READ_ONLY, FORMULA_WRITE_ONLY, FORMULA_READ_WRITE };
struct formula_input {
	std::string name;
	FORMULA_ACCESS_TYPE access;
	explicit formula_input(const std::string& name, FORMULA_ACCESS_TYPE access=FORMULA_READ_WRITE)
			: name(name), access(access)
	{}
};

//interface for objects that can have formulae run on them
class formula_callable : public reference_counted_object {
public:
	explicit formula_callable(bool has_self=true) : type_(FORMULA_C), has_self_(has_self)
	{}

	variant query_value(const std::string& key) const {
		if(has_self_ && key == "self") {
			return variant(this);
		}
		return get_value(key);
	}

	void mutate_value(const std::string& key, const variant& value) {
		set_value(key, value);
	}

	std::vector<formula_input> inputs() const {
		std::vector<formula_input> res;
		get_inputs(&res);
		return res;
	}

	bool equals(const formula_callable* other) const {
		return do_compare(other) == 0;
	}

	bool less(const formula_callable* other) const {
		return do_compare(other) < 0;
	}

	virtual void get_inputs(std::vector<formula_input>* /*inputs*/) const {}

	//note: this function should NOT overwrite str, but append text to it!
	void serialize(std::string& str) const {
		serialize_to_string(str);
	}

	bool has_key(const std::string& key) const
		{ return !query_value(key).is_null(); }

protected:
	virtual ~formula_callable() {}

	virtual void set_value(const std::string& key, const variant& value);
	virtual int do_compare(const formula_callable* callable) const {
		if( type_ < callable->type_ )
			return -1;

		if( type_ > callable->type_ )
			return 1;

		return this < callable ? -1 : (this == callable ? 0 : 1);
	}

	//note: this function should NOT overwrite str, but append text to it!
	virtual void serialize_to_string(std::string& /*str*/) const {
		throw type_error("Tried to serialize type which cannot be serialized");
	}

	//priority for objects that are derived from this class, used in do_compare
	//when comparing objects of different types
	//for example: formula_callable < terrain_callable < unit_type_callable ...
	enum TYPE { FORMULA_C, TERRAIN_C, LOCATION_C, UNIT_TYPE_C, UNIT_C,
		ATTACK_TYPE_C, MOVE_PARTIAL_C, MOVE_C, ATTACK_C, MOVE_MAP_C };

	TYPE type_;
private:
	virtual variant get_value(const std::string& key) const = 0;
	bool has_self_;
};

class formula_callable_no_ref_count : public formula_callable {
public:
	formula_callable_no_ref_count() {
		turn_reference_counting_off();
	}
	virtual ~formula_callable_no_ref_count() {}
};

class formula_callable_with_backup : public formula_callable {
	const formula_callable& main_;
	const formula_callable& backup_;
	variant get_value(const std::string& key) const {
		variant var = main_.query_value(key);
		if(var.is_null()) {
			return backup_.query_value(key);
		}

		return var;
	}

	void get_inputs(std::vector<formula_input>* inputs) const {
		main_.get_inputs(inputs);
		backup_.get_inputs(inputs);
	}
public:
	formula_callable_with_backup(const formula_callable& main, const formula_callable& backup) : formula_callable(false), main_(main), backup_(backup)
	{}
};

class formula_variant_callable_with_backup : public formula_callable {
	variant var_;
	const formula_callable& backup_;
	variant get_value(const std::string& key) const {
		variant var = var_.get_member(key);
		if(var.is_null()) {
			return backup_.query_value(key);
		}

		return var;
	}

	void get_inputs(std::vector<formula_input>* inputs) const {
		backup_.get_inputs(inputs);
	}

public:
	formula_variant_callable_with_backup(const variant& var, const formula_callable& backup) : formula_callable(false), var_(var), backup_(backup)
	{}
};

class map_formula_callable : public formula_callable {
public:
	explicit map_formula_callable(const formula_callable* fallback=nullptr);
	map_formula_callable& add(const std::string& key, const variant& value);
	void set_fallback(const formula_callable* fallback) { fallback_ = fallback; }
	bool empty() const { return values_.empty(); }
	void clear() { values_.clear(); }

	typedef std::map<std::string,variant>::const_iterator const_iterator;

	const_iterator begin() const { return values_.begin(); }
	const_iterator end() const { return values_.end(); }

private:
	variant get_value(const std::string& key) const;
	void get_inputs(std::vector<formula_input>* inputs) const;
	void set_value(const std::string& key, const variant& value);
	std::map<std::string,variant> values_;
	const formula_callable* fallback_;
};

typedef boost::intrusive_ptr<map_formula_callable> map_formula_callable_ptr;
typedef boost::intrusive_ptr<const map_formula_callable> const_map_formula_callable_ptr;

}

#endif
