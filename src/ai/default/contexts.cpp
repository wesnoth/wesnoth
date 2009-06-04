/* $Id$ */
/*
   Copyright (C) 2009 by Yurii Chernyi <terraninfo@terraninfo.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * Helper functions for the object which operates in the context of AI for specific side
 * This is part of AI interface
 * @file ai/default/contexts.cpp
 */

#include "contexts.hpp"

// =======================================================================
namespace ai {


default_ai_context::default_ai_context()
{
}


default_ai_context::~default_ai_context()
{
}


default_ai_context_proxy::~default_ai_context_proxy()
{
}

void default_ai_context_proxy::init_default_ai_context_proxy(default_ai_context &target)
{
	init_readwrite_context_proxy(target);
	target_= &target.get_default_ai_context();
}

default_ai_context& default_ai_context_impl::get_default_ai_context(){
	return *this;
}


default_ai_context_impl::~default_ai_context_impl()
{
}

} //of namespace ai
