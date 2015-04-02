/*
   Copyright (C) 2014 by Chris Beck <render787@gmail.com>
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
 * This template function is meant to handle the details of parsing tags
 * which may contain boolean expressions built with [and] [or] [not]
 * children. It should be appropriate to use when parsing conditional
 * expressions, unit filters, side filters, or location filters.
 */

// F should be a function or function object with a call operator
// which takes such vconfigs to bools.
template <typename F>
bool eval_boolean( const vconfig & vcfg, F match_internal)
{
	bool matches = match_internal(vcfg);

	//handle [and], [or], and [not] with in-order precedence
	vconfig::all_children_iterator cond = vcfg.ordered_begin();
	vconfig::all_children_iterator cond_end = vcfg.ordered_end();
	while (cond != cond_end) {
		const std::string& cond_name = cond.get_key();
		const vconfig & cond_cfg = cond.get_child();

		//handle [and]
		if(cond_name == "and")
		{
			matches = matches && eval_boolean(cond_cfg, match_internal);
		}
		//handle [or]
		else if(cond_name == "or")
		{
			matches = matches || eval_boolean(cond_cfg, match_internal);
		}
		//handle [not]
		else if(cond_name == "not")
		{
			matches = matches && ! eval_boolean(cond_cfg, match_internal);
		}
		++cond;
	}
	return matches;
}
