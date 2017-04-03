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

#ifndef FORMULA_CALLABLE_HPP_INCLUDED
#define FORMULA_CALLABLE_HPP_INCLUDED

#include "formula/variant.hpp"

#include <iostream>
#include <memory>
#include <set>

namespace wfl
{

enum FORMULA_ACCESS_TYPE { FORMULA_READ_ONLY, FORMULA_WRITE_ONLY, FORMULA_READ_WRITE };

struct formula_input
{
	explicit formula_input(const std::string& name, FORMULA_ACCESS_TYPE access = FORMULA_READ_WRITE)
		: name(name), access(access)
	{}

	std::string name;
	FORMULA_ACCESS_TYPE access;
};

using formula_input_vector = std::vector<formula_input>;

// Interface for objects that can have formulae run on them
class formula_callable
{
public:
	explicit formula_callable(bool has_self = true) : type_(FORMULA_C), has_self_(has_self) {}

	virtual ~formula_callable() {}

	variant query_value(const std::string& key) const
	{
		if(has_self_ && key == "self") {
			return variant(this);
		}
		return get_value(key);
	}

	void mutate_value(const std::string& key, const variant& value)
	{
		set_value(key, value);
	}

	formula_input_vector inputs() const
	{
		formula_input_vector res;
		get_inputs(res);
		return res;
	}

	virtual void get_inputs(formula_input_vector& /*inputs*/) const {}

	bool equals(const formula_callable* other) const
	{
		return do_compare(other) == 0;
	}

	bool less(const formula_callable* other) const
	{
		return do_compare(other) < 0;
	}

	bool has_key(const std::string& key) const
	{
		return !query_value(key).is_null();
	}

	// Note: this function should NOT overwrite str, but append text to it!
	// TODO: return str instead of taking str.
	void serialize(std::string& str) const
	{
		serialize_to_string(str);
	}

protected:
	template<typename T, typename K>
	static variant convert_map(const std::map<T, K>& input_map)
	{
		std::map<variant,variant> tmp;
		for(const auto& p : input_map) {
			tmp[variant(p.first)] = variant(p.second);
		}

		return variant(tmp);
	}

	template<typename T>
	static variant convert_set(const std::set<T>& input_set)
	{
		std::map<variant,variant> tmp;
		for(const auto& elem : input_set) {
			tmp[variant(elem)] = variant(1);
		}

		return variant(tmp);
	}

	template<typename T>
	static variant convert_vector(const std::vector<T>& input_vector)
	{
		std::vector<variant> tmp;
		for(const auto& elem : input_vector) {
			tmp.emplace_back(elem);
		}

		return variant(tmp);
	}

	static inline void add_input(formula_input_vector& inputs, const std::string& key, FORMULA_ACCESS_TYPE access_type = FORMULA_READ_ONLY)
	{
		inputs.emplace_back(key, access_type);
	}

	virtual void set_value(const std::string& key, const variant& /*value*/)
	{
		std::cerr << "ERROR: cannot set key '" << key << "' on object" << std::endl;
	}

	virtual int do_compare(const formula_callable* callable) const
	{
		if(type_ < callable->type_) {
			return -1;
		}

		if(type_ > callable->type_) {
			return 1;
		}

		return this < callable ? -1 : (this == callable ? 0 : 1);
	}

	// Note: this function should NOT overwrite str, but append text to it!
	// TODO: return string not take string
	virtual void serialize_to_string(std::string& /*str*/) const
	{
		throw type_error("Tried to serialize type which cannot be serialized");
	}

	// Priority for objects that are derived from this class, used in do_compare
	// when comparing objects of different types.
	// For example: formula_callable < terrain_callable < unit_type_callable ...
	enum TYPE {
		FORMULA_C,TERRAIN_C, LOCATION_C, UNIT_TYPE_C, UNIT_C,
		ATTACK_TYPE_C, MOVE_PARTIAL_C, MOVE_C, ATTACK_C, MOVE_MAP_C
	};

	TYPE type_;

private:
	virtual variant get_value(const std::string& key) const = 0;
	bool has_self_;
};

class action_callable : public formula_callable
{
public:
	virtual variant execute_self(variant ctxt) = 0;
};

class formula_callable_with_backup : public formula_callable
{
public:
	formula_callable_with_backup(const formula_callable& main, const formula_callable& backup)
		: formula_callable(false), main_(main), backup_(backup)
	{}

private:
	variant get_value(const std::string& key) const override
	{
		variant var = main_.query_value(key);
		if(var.is_null()) {
			return backup_.query_value(key);
		}

		return var;
	}

	void get_inputs(formula_input_vector& inputs) const override
	{
		main_.get_inputs(inputs);
		backup_.get_inputs(inputs);
	}

	const formula_callable& main_;
	const formula_callable& backup_;
};

class formula_variant_callable_with_backup : public formula_callable
{
public:
	formula_variant_callable_with_backup(const variant& var, const formula_callable& backup)
		: formula_callable(false), var_(var), backup_(backup)
	{}

private:
	variant get_value(const std::string& key) const override
	{
		variant var = var_.get_member(key);
		if(var.is_null()) {
			return backup_.query_value(key);
		}

		return var;
	}

	void get_inputs(formula_input_vector& inputs) const override
	{
		backup_.get_inputs(inputs);
	}

	variant var_;
	const formula_callable& backup_;
};

class map_formula_callable : public formula_callable
{
public:
	explicit map_formula_callable(const formula_callable* fallback = nullptr)
		: formula_callable(false)
		, values_()
		, fallback_(fallback)
	{}

	map_formula_callable& add(const std::string& key, const variant& value)
	{
		values_[key] = value;
		return *this;
	}

	void set_fallback(const formula_callable* fallback)
	{
		fallback_ = fallback;
	}

	bool empty() const { return values_.empty(); }

	void clear() { values_.clear(); }

	typedef std::map<std::string,variant>::const_iterator const_iterator;

	const_iterator begin() const { return values_.begin(); }
	const_iterator end()   const { return values_.end();   }

private:
	void set_value(const std::string& key, const variant& value) override
	{
		values_[key] = value;
	}

	variant get_value(const std::string& key) const
	{
		auto it = values_.find(key);
		if(it != values_.end()) {
			return it->second;
		} else if(fallback_) {
			return fallback_->query_value(key);
		}

		return variant();
	}

	void get_inputs(formula_input_vector& inputs) const override
	{
		if(fallback_) {
			fallback_->get_inputs(inputs);
		}

		for(const auto& i : values_) {
			add_input(inputs, i.first, FORMULA_READ_WRITE);
		}
	}

	std::map<std::string, variant> values_;
	const formula_callable* fallback_;
};

using map_formula_callable_ptr = std::shared_ptr<map_formula_callable>;
using const_map_formula_callable_ptr = std::shared_ptr<const map_formula_callable>;

} // namespace wfl

#endif
