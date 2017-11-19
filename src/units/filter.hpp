/*
   Copyright (C) 2014 - 2017 by Chris Beck <render787@gmail.com>
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
 * This namespace contains the function that checks if a unit matches
 * a filter. It helps by simplifying the unit object (which before now
 * holds the "match" function).
 *
 * TODO:
 * Make a class that abstracts a unit filter, assembles the constituent
 * side filters and terrain filters and conditional filters, and caches
 * these to speed up repeated application of the filter.
 */

#pragma once

#include "units/ptr.hpp"
#include "utils/make_enum.hpp"

#include "display_context.hpp"
#include "units/map.hpp"
#include "filter_context.hpp"
#include "variable.hpp"

#include <memory>
#include <vector>

class filter_context;
class unit;
class config;
class vconfig;
struct map_location;



namespace unit_filter_impl
{
	MAKE_ENUM (CONDITIONAL_TYPE,
		(AND, "and")
		(OR, "or")
		(NOT, "not")
	)

	struct unit_filter_args
	{
		const unit& u;
		map_location loc;
		const unit* u2;
		const filter_context* fc;
		bool use_flat_tod;

		// This constructor is here to shut down warnings that the default constructor couldn't be generated.
		// It's technically unnecessary since lacking of a default constructor doesn't prevent aggregate-initialization, but...
		unit_filter_args(const unit& u, map_location loc, const unit* u2, const filter_context* fc, bool use_flat_tod)
			: u(u), loc(loc), u2(u2), fc(fc), use_flat_tod(use_flat_tod)
		{}
	};

	struct unit_filter_base
	{
		virtual bool matches(const unit_filter_args&) const = 0;
		virtual ~unit_filter_base() {}
	};

	struct unit_filter_compound : public unit_filter_base
	{
		unit_filter_compound(vconfig cfg);

		template<typename C, typename F>
		void create_attribute(const config::attribute_value c, C conv, F func);
		template<typename F>
		void create_child(const vconfig& c, F func);

		void fill(vconfig cfg);

		virtual bool matches(const unit_filter_args& u) const override;
		bool filter_impl(const unit_filter_args& u) const;

		std::vector<std::shared_ptr<unit_filter_base>> children_;
		std::vector<std::pair<CONDITIONAL_TYPE, unit_filter_compound>> cond_children_;
	};

}

class unit_filter
{
public:
	unit_filter(vconfig cfg, const filter_context * fc)
		: cfg_(cfg)
		, fc_(fc)
		, use_flat_tod_(false)
		, impl_(cfg_)
		, max_matches_(-1)
	{
	}

	unit_filter(const unit_filter&) = default;
	unit_filter& operator=(const unit_filter&) = default;

#if defined(_MSC_VER) && _MSC_VER < 1900
	unit_filter(unit_filter&& u)
		: cfg_(std::move(u.cfg_))
		, fc_(u.fc_)
		, use_flat_tod_(u.use_flat_tod_)
		, impl_(std::move(u.impl_))
		, max_matches_(u.max_matches_)
	{}
	unit_filter& operator=(unit_filter&& u)
	{
		cfg_ = std::move(u.cfg_);
		fc_ = u.fc_;
		use_flat_tod_ = u.use_flat_tod_;
		impl_ = std::move(u.impl_);
		max_matches_ = u.max_matches_;
	}
#else
	unit_filter(unit_filter&&) = default;
	unit_filter& operator=(unit_filter&&) = default;
#endif

	unit_filter& set_use_flat_tod(bool value) {
		use_flat_tod_ = value;
		return *this;
	}

	/// Determine if *this matches @a filter at a specified location.
	/// Use this for units on a recall list, or to test for a match if
	/// a unit is hypothetically moved.
	bool matches(const unit & u, const map_location & loc) const {
		return impl_.matches(unit_filter_impl::unit_filter_args{u, loc, nullptr, fc_, use_flat_tod_});
	}

	/// Determine if *this matches @a filter at its current location.
	/// (Only use for units currently on the map; otherwise use the overload
	/// that takes a location, possibly with a null location.)
	bool matches(const unit & u) const;

	bool matches(const unit & u, const map_location & loc, const unit & u2) const {
		return impl_.matches(unit_filter_impl::unit_filter_args{u, loc, &u2, fc_, use_flat_tod_});
	}

	bool matches(const unit & u, const unit & u2) const;

	bool operator()(const unit & u, const map_location & loc) const {
		return matches(u, loc);
	}

	bool operator()(const unit & u) const {
		return matches(u);
	}

	bool operator()(const unit & u, const map_location & loc, const unit & u2) const {
		return matches(u, loc, u2);
	}

	bool operator()(const unit & u, const unit & u2) const {
		return matches(u, u2);
	}

	std::vector<const unit *> all_matches_on_map(const map_location* loc = nullptr, const unit* other_unit = nullptr) const;

	std::vector<const unit*> all_matches_at(const map_location& loc) const {
		return all_matches_on_map(&loc);
	}

	std::vector<const unit*> all_matches_with_unit(const unit& u) const {
		return all_matches_on_map(nullptr, &u);
	}

	std::vector<const unit*> all_matches_with_unit_at(const unit& u, const map_location& loc) const {
		return all_matches_on_map(&loc, &u);
	}

	unit_const_ptr first_match_on_map() const;

	config to_config() const {
		return cfg_.get_config();
	}

	bool empty() const {
		return cfg_.get_config().empty();
	}

private:

	vconfig cfg_;
	const filter_context * fc_;
	bool use_flat_tod_;
	unit_filter_impl::unit_filter_compound impl_;
	int max_matches_;
};

