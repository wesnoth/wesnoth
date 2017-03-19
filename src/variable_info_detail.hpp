/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2017 by Philippe Plantier <ayin@anathas.org>

   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef VARIABLE_INFO_DETAIL_HPP_INCLUDED
#define VARIABLE_INFO_DETAIL_HPP_INCLUDED

#include <string>
#include "config.hpp"

namespace variable_info_detail
{
	enum variable_info_type {vit_const, vit_create_if_not_existent, vit_throw_if_not_existent,  };
	enum variable_info_state_type {
		state_start = 0, // for internal use
		                 // only used at the 'starting_pos' of the variable_info::calculate_value algorithm
		state_named,     // the result of .someval this can eigher man an attribute value or an
		                 // child range
		state_indexed,   // the result of .someval[index] this is never an attribute value,
		                 // this is always a single config.
		state_temporary, // the result of .length this value can never be written, it can only be read.

	};

	//Special case of std::enable_if
	template<const variable_info_type vit>
	struct enable_if_non_const
	{
		typedef enable_if_non_const<vit> type;
	};

	template<>
	struct enable_if_non_const<vit_const>
	{
	};

	template<const variable_info_type vit, typename T>
	struct maybe_const
	{
		typedef T type;
	};

	template <class T>
	struct maybe_const<vit_const, T>
	{
		typedef const T type;
	};

	template <>
	struct maybe_const<vit_const, config::child_itors>
	{
		typedef config::const_child_itors type;
	};


	template<const variable_info_type vit>
	struct variable_info_state
	{
		typedef typename maybe_const<vit,config>::type child_t;

		variable_info_state(child_t& vars)
			: child_(&vars)
			, key_()
			, index_(0)
			, temp_val_()
			, type_(state_start)
		{
			child_ = &vars;
		}

		// The meaning of the following 3 depends on 'type_', but usualy the case is:
		// the current config is  child_->child_at(key_, index_).
		child_t* child_;
		std::string key_;
		int index_;

		// If we have a temporary value like .length
		// Then we store the result here.
		config::attribute_value temp_val_;

		// See the definition of 'variable_info_state_type'
		variable_info_state_type type_;
	};
}

#endif
