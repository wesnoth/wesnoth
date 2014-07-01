/*
   Copyright (C) 2003 by David White <dave@whitevine.net>
   Copyright (C) 2005 - 2014 by Philippe Plantier <ayin@anathas.org>

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

namespace variable_info_3_detail
{
	enum variable_info_3_type {vit_const, vit_create_if_not_existent, vit_throw_if_not_existent,  };
	enum variable_info_3_state_type { 
		state_start = 0, // for internal use
		state_named,     // the result of .someval this can eigher man an attribute value or an 
		                 // child range
		state_indexed,   // the result of .someval[index] this is never an attribute value, 
		                 // this is always a single config.
		state_temporary, // the result of .length this value can never be written, it can onyl be read.
		
	};

	//Special case of boost::enable_if
	template<const variable_info_3_type vit>
	struct enable_if_non_const
	{
		typedef enable_if_non_const<vit> type;
	};

	template<>
	struct enable_if_non_const<vit_const>
	{
	};

	template<const variable_info_3_type vit, typename T>
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

	
	template<const variable_info_3_type vit>
	struct variable_info_3_state
	{
		typedef typename maybe_const<vit,config>::type t_child;

		variable_info_3_state(t_child& vars) 
			: child_(&vars)
			, key_()
			, index_(0)
			, temp_val_()
			, type_(state_start)
		{
			child_ = &vars;
		}


		t_child* child_;
		std::string key_;
		int index_;

		config::attribute_value temp_val_;

		variable_info_3_state_type type_;
	};
}

#endif
