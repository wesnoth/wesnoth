/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2018 by Philippe Plantier <ayin@anathas.org>

   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "game_config.hpp"

#include <stdexcept>

namespace variable_info_implementation
{
// ==================================================================
// General helper functions
// ==================================================================

// TConfig is either 'config' or 'const config'
template<typename TConfig>
auto get_child_range(TConfig& cfg, const std::string& key, int start, int count) -> decltype(cfg.child_range(key))
{
	auto res = cfg.child_range(key);
	return {res.begin() + start, res.begin() + start + count};
}

void resolve_negative_value(int size, int& val);
void resolve_negative_value(int size, int& val)
{
	if(val < 0) {
		val = size + val;
	}

	// val is still < 0? We don't accept!
	if(val < 0) {
		throw invalid_variablename_exception();
	}
}

const config non_empty_const_cfg("_");

/**
 * Parses a ']' terminated string.
 * This is a important optimization of lexical_cast_default
 */
int parse_index(const char* index_str);
int parse_index(const char* index_str)
{
	char* endptr;
	int res = strtol(index_str, &endptr, 10);

	if(*endptr != ']' || res > static_cast<int>(game_config::max_loop) || endptr == index_str) {
		throw invalid_variablename_exception();
	}

	return res;
}

// ==================================================================
// Visitor interface
// ==================================================================

/**
 * Visitor base class.
 *
 * This provides the interface for the main functions each visitor can implement. The default implementation of
 * each function simply throws @ref invalid_variablename_exception.
 *
 * This class also provides two type aliases corresponding to the function return value and argument types.
 * Note that visitors shouldn't inherit from this directly and instead use the @ref info_visitor and
 * @ref info_visitor_const wrappers, since both fully specify the parameter type.
 *
 * @tparam R                   Return value type.
 * @tparam P                   Argument type.
 */
template<typename R, typename P>
class info_visitor_base
{
public:
	using result_t = R;
	using param_t = P&;

#define DEFAULTHANDLER(name)                                                                                           \
	result_t name(param_t) const                                                                                       \
	{                                                                                                                  \
		throw invalid_variablename_exception();                                                                        \
	}

	/** For use if the variable name was previously empty. This can only happen during calculate_value. */
	DEFAULTHANDLER(from_start)

	/** For use if the variable ended with a .somename. */
	DEFAULTHANDLER(from_named)

	/** For use if the variable ended with .somename[someindex]. */
	DEFAULTHANDLER(from_indexed)

	/** For use if the variable is a readonly value (.somename.length). */
	DEFAULTHANDLER(from_temporary)

#undef DEFAULTHANDLER
};

template<typename V, typename TResult>
using info_visitor = info_visitor_base<TResult, variable_info_state<V>>;

template<typename V, typename TResult>
using info_visitor_const = info_visitor_base<TResult, const variable_info_state<V>>;

/** Adds a '.<key>' to the current variable. */
template<typename V>
class get_variable_key_visitor : public info_visitor<V, void>
{
public:
	// Import typedefs from base class.
	using param_t = typename get_variable_key_visitor::param_t;

	get_variable_key_visitor(const std::string& key)
		: key_(key)
	{
		if(!config::valid_id(key_)) {
			throw invalid_variablename_exception();
		}
	}

	void from_named(param_t state) const
	{
		if(key_ == "length") {
			state.temp_val_ = state.child_->child_count(state.key_);
			state.type_ = state_temporary;
			return;
		}

		return do_from_config(V::get_child_at(*state.child_, state.key_, 0), state);
	}

	void from_start(param_t state) const
	{
		return do_from_config(*state.child_, state);
	}

	void from_indexed(param_t state) const
	{
		// We do not support aaa[0].length
		return do_from_config(V::get_child_at(*state.child_, state.key_, state.index_), state);
	}

private:
	void do_from_config(maybe_const_t<config, V>& cfg, param_t state) const
	{
		state.type_ = state_named;
		state.key_ = key_;
		state.child_ = &cfg;
	}

	const std::string& key_;
};

/**
 * Appends an [index] to the variable.
 * We only support from_named since [index][index2] or a.length[index] both don't make sense.
 */
template<typename V>
class get_variable_index_visitor : public info_visitor<V, void>
{
public:
	get_variable_index_visitor(int n)
		: n_(n)
	{
	}

	void from_named(typename get_variable_index_visitor::param_t state) const
	{
		state.index_ = n_;
		resolve_negative_value(state.child_->child_count(state.key_), state.index_);
		state.type_ = state_indexed;
	}

private:
	const int n_;
};

/** Tries to convert it to an (maybe const) attribute value. */
template<typename V>
class as_scalar_visitor : public info_visitor_const<V, maybe_const_t<config::attribute_value, V>&>
{
public:
	// Import typedefs from base class.
	using result_t = typename as_scalar_visitor::result_t;
	using param_t = typename as_scalar_visitor::param_t;

	result_t from_named(param_t state) const
	{
		return (*state.child_)[state.key_];
	}

	/**
	 * Only implemented for read-only variable_info. Other types use the default throw implementation.
	 */
	result_t from_temporary(param_t /*state*/) const
	{
		throw invalid_variablename_exception();
	}
};

/**
 * Values like '.length' are readonly so we only support reading them, especially since we don't
 * want to return non-const references.
 */
template<>
const config::attribute_value& as_scalar_visitor<const vi_policy_const>::from_temporary(
	as_scalar_visitor::param_t state) const
{
	return state.temp_val_;
}

/**
 * Tries to convert to a [const] config&. Unlike range based operation this also supports 'from_start'.
 * NOTE: Currently getting the 'from_start' case here is impossible, because we always apply at least one string key.
 */
template<typename V>
class as_container_visitor : public info_visitor_const<V, maybe_const_t<config, V>&>
{
public:
	// Import typedefs from base class.
	using result_t = typename as_container_visitor::result_t;
	using param_t = typename as_container_visitor::param_t;

	result_t from_named(param_t state) const
	{
		return V::get_child_at(*state.child_, state.key_, 0);
	}

	result_t from_start(param_t state) const
	{
		return *state.child_;
	}

	result_t from_indexed(param_t state) const
	{
		return V::get_child_at(*state.child_, state.key_, state.index_);
	}
};

/**
 * This currently isn't implemented as a range-based operation because doing it on something like range
 * 2-5 on vi_policy_const if child_ has only 4 elements would be too hard to implement.
 */
template<typename V>
class as_array_visitor : public info_visitor_const<V, maybe_const_t<config::child_itors, V>>
{
public:
	// Import typedefs from base class.
	using result_t = typename as_array_visitor::result_t;
	using param_t = typename as_array_visitor::param_t;

	result_t from_named(param_t state) const
	{
		return get_child_range(*state.child_, state.key_, 0, state.child_->child_count(state.key_));
	}

	result_t from_indexed(param_t state) const
	{
		// Ensure we have a config at the given explicit position.
		V::get_child_at(*state.child_, state.key_, state.index_);
		return get_child_range(*state.child_, state.key_, state.index_, 1);
	}
};

template<>
config::const_child_itors as_array_visitor<const vi_policy_const>::from_indexed(as_array_visitor::param_t state) const
{
	if(static_cast<int>(state.child_->child_count(state.key_)) <= state.index_) {
		return get_child_range(non_empty_const_cfg, "_", 0, 1);
	}

	return get_child_range(*state.child_, state.key_, state.index_, 1);
}

/**
 * @tparam THandler           Handler type. Should implement an operator() with the signature:
 *                            '(config&, const std::string&, int, int) -> THandler::result_t'
 *
 * That does the actual work on the range of children of cfg with name 'name'.
 * Note this is currently only used by the insert/append/replace/merge operations, so V is always
 * vi_policy_create.
 */
template<typename V, typename THandler, typename... T>
class as_range_visitor_base : public info_visitor_const<V, typename THandler::result_t>
{
public:
	// Import typedefs from base class.
	using result_t = typename as_range_visitor_base::result_t;
	using param_t = typename as_range_visitor_base::param_t;

	as_range_visitor_base(T&&... args)
		: handler_(std::forward<T>(args)...)
	{
	}

	result_t from_named(param_t state) const
	{
		return handler_(*state.child_, state.key_, 0, state.child_->child_count(state.key_));
	}

	result_t from_indexed(param_t state) const
	{
		return handler_(*state.child_, state.key_, state.index_, state.index_ + 1);
	}

protected:
	THandler handler_;
};

template<typename V>
class clear_value_visitor : public info_visitor_const<V, void>
{
public:
	// Import typedefs from base class.
	using param_t = typename clear_value_visitor::param_t;

	clear_value_visitor(bool only_tables)
		: only_tables_(only_tables)
	{
	}

	void from_named(param_t state) const
	{
		if(!only_tables_) {
			state.child_->remove_attribute(state.key_);
		}

		state.child_->clear_children(state.key_);
	}

	void from_indexed(param_t state) const
	{
		state.child_->remove_child(state.key_, state.index_);
	}

private:
	bool only_tables_;
};

template<typename V>
class exists_as_container_visitor : public info_visitor_const<V, bool>
{
public:
	// Import typedefs from base class.
	using param_t = typename exists_as_container_visitor::param_t;

	bool from_named(param_t state) const
	{
		return state.child_->has_child(state.key_);
	}

	bool from_indexed(param_t state) const
	{
		return state.child_->child_count(state.key_) > static_cast<size_t>(state.index_);
	}

	bool from_start(param_t) const
	{
		return true;
	}

	bool from_temporary(param_t) const
	{
		return false;
	}
};

// ==================================================================
// Range manipulation interface
// ==================================================================

/**
 * Replaces the child in [startindex, endindex) with 'source'
 * 'insert' and 'append' are subcases of this.
 */
class replace_range_h
{
public:
	typedef config::child_itors result_t;
	replace_range_h(std::vector<config>& source)
		: datasource_(source)
	{
	}

	result_t operator()(config& child, const std::string& key, int startindex, int endindex) const
	{
		assert(endindex - startindex >= 0);
		if(endindex > 0) {
			// NOTE: currently this is only called from as_range_visitor_base<vi_policy_create>
			// Based on that assumption we use vi_policy_create::get_child_at here instead of making this
            // a class template.
			vi_policy_create::get_child_at(child, key, endindex - 1);
		}

		int size_diff = datasource_.size() - (endindex - startindex);

		// remove configs first
		while(size_diff < 0) {
			child.remove_child(key, startindex);
			++size_diff;
		}

		size_t index = 0;
		for(index = 0; index < static_cast<size_t>(size_diff); ++index) {
			child.add_child_at(key, config(), startindex + index).swap(datasource_[index]);
		}

		for(; index < datasource_.size(); ++index) {
			child.child(key, startindex + index).swap(datasource_[index]);
		}

		return get_child_range(child, key, startindex, datasource_.size());
	}

private:
	std::vector<config>& datasource_;
};

class insert_range_h : replace_range_h
{
public:
	typedef config::child_itors result_t;
	insert_range_h(std::vector<config>& source)
		: replace_range_h(source)
	{
	}

	result_t operator()(config& child, const std::string& key, int startindex, int /*endindex*/) const
	{
		// insert == replace empty range with data.
		return replace_range_h::operator()(child, key, startindex, startindex);
	}
};

class append_range_h : insert_range_h
{
public:
	typedef config::child_itors result_t;
	append_range_h(std::vector<config>& source)
		: insert_range_h(source)
	{
	}

	result_t operator()(config& child, const std::string& key, int /*startindex*/, int /*endindex*/) const
	{
		// append == insert at end.
		int insert_pos = child.child_count(key);
		return insert_range_h::operator()(child, key, insert_pos, insert_pos /*ignored by insert_range_h*/);
	}
};

class merge_range_h
{
public:
	typedef void result_t;
	merge_range_h(std::vector<config>& source)
		: datasource_(source)
	{
	}

	void operator()(config& child, const std::string& key, int startindex, int /*endindex*/) const
	{
		// The merge_with function only accepts configs so we convert vector -> config.
		config datatemp;

		// Add emtpy config to 'shift' the merge to startindex
		for(int index = 0; index < startindex; ++index) {
			datatemp.add_child(key);
		}

		// move datasource_ -> datatemp
		for(size_t index = 0; index < datasource_.size(); ++index) {
			datatemp.add_child(key).swap(datasource_[index]);
		}

		child.merge_with(datatemp);
	}

private:
	std::vector<config>& datasource_;
};
} // end namespace variable_info_implementation
