/*
	Copyright (C) 2013 by Pierre Talbot <ptalbot@mopong.net>
	Part of the Battle for Wesnoth Project http://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/** This file declare preprocessor macros that help to declare and define
Boost.Spirit Qi and Karma rules.

Precondition: The class must define an iterator_type typedef to the underlining iterator.
							The namespace qi and karma must be available.
*/

#ifndef SPIRIT_PREPROCESSOR_RULE_HELPER_HPP
#define SPIRIT_PREPROCESSOR_RULE_HELPER_HPP

#include <boost/version.hpp>

// Rule declaration.

namespace detail{ // Useless, but inform that everything in should not be used outside this file.
#define RULE_DECL_IMPL(ns, attribute, locals, name) ns::rule< iterator_type, attribute, locals > name
#define RULE_LOC_IMPL(ns, attribute, locals_, name) RULE_DECL_IMPL(ns, attribute, ns::locals< locals_ >, name)
#define RULE_IMPL(ns, attribute, name) RULE_DECL_IMPL(ns, attribute, ns::unused_type, name)
#define RULE0_IMPL(ns, name) RULE_IMPL(ns, ns::unused_type, name)
}

// Boost.Spirit.Qi rule declaration helpers.
// Declare a Qi rule with attribute and local variables.
#define QI_RULE_LOC(attribute, locals, name) RULE_LOC_IMPL(qi, attribute, locals, name)

// Declare a Qi rule with attribute variables.
#define QI_RULE(attribute, name) RULE_IMPL(qi, attribute, name)

// Declare a simple Qi rule.
#define QI_RULE0(name) RULE0_IMPL(qi, name)

// Boost.Spirit.Karma rule declaration helpers.
// Declare a Karma rule with attribute and local variables.
#define KA_RULE_LOC(attribute, locals, name) RULE_LOC_IMPL(karma, attribute, locals, name)

// Declare a Qi rule with attribute variables.
#define KA_RULE(attribute, name) RULE_IMPL(karma, attribute, name)

// Declare a simple Qi rule.
#define KA_RULE0(name) RULE0_IMPL(karma, name)


// Rule definition.

/** Define a rule without the debugging facilities provided by Boost.Spirit.
*/
#define RULE_NDEF(rule_name, ...) rule_name __VA_ARGS__ ;\
		rule_name.name(#rule_name);

/** Enclose your rule inside RULE_DEF and they will be automatically
* named and add to the debugging facility.
*
* WARNING: Compilation error will occur if you enclose rule that have attribute
* or local variables that have not a streaming operator defined.
* Enclose them inside RULE_NDEF so they will not be added to the debug engine.
*
* BOOST_SPIRIT_DEBUG_NODE is not available with all Boost version.
*/
#if BOOST_VERSION > 104400
	#define RULE_DEF(rule_name, ...) RULE_NDEF(rule_name, __VA_ARGS__ ) \
			BOOST_SPIRIT_DEBUG_NODE(rule_name);
#else
	#define RULE_DEF(rule_name, ...) RULE_NDEF(rule_name, __VA_ARGS__ ) 
#endif

#endif // SPIRIT_PREPROCESSOR_RULE_HELPER_HPP