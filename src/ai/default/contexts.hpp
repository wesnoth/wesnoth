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
 * @file ai/default/contexts.hpp
 * Default AI contexts
 */

#ifndef AI_DEFAULT_CONTEXTS_HPP_INCLUDED
#define AI_DEFAULT_CONTEXTS_HPP_INCLUDED

#include "../../global.hpp"

#include "../contexts.hpp"
#include <vector>

//============================================================================
namespace ai {


class default_ai_context;
class default_ai_context : public virtual readwrite_context{
public:

	default_ai_context();

	virtual ~default_ai_context();


	virtual default_ai_context& get_default_ai_context() = 0;
};


// proxies
class default_ai_context_proxy : public virtual default_ai_context, public virtual readwrite_context_proxy {
public:


	default_ai_context_proxy()
		: target_(NULL)
	{
	}


	void init_default_ai_context_proxy(default_ai_context &target);


	virtual	~default_ai_context_proxy();


	virtual default_ai_context& get_default_ai_context()
	{
		return target_->get_default_ai_context();
	}

private:
	default_ai_context *target_;
};

class default_ai_context_impl : public virtual readwrite_context_proxy, public default_ai_context {
public:


	default_ai_context_impl(readwrite_context &context)
		: recursion_counter_(context.get_recursion_count())
	{
		init_readwrite_context_proxy(context);
	}


	virtual ~default_ai_context_impl();


	virtual default_ai_context& get_default_ai_context();

	int get_recursion_count() const
	{
		return recursion_counter_.get_count();
	}

private:
	recursion_counter recursion_counter_;
};

} //end of namespace ai

#endif
