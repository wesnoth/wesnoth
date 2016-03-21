/*
   Copyright (C) 2012 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
 * @file
 * Define the common filters for the @ref gui2::tpane class.
 */

#ifndef GUI_AUXILIARY_FILTER_HPP_INCLUDED
#define GUI_AUXILIARY_FILTER_HPP_INCLUDED

#include "global.hpp"
#include "gui/widgets/text_box.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"
#include "serialization/unicode.hpp"
#ifndef HAVE_CXX11
#include <stdexcept> // needed for the at() emulation
#endif

namespace gui2
{

template <class T>
inline bool sort(const tpane::titem& lhs,
				 const tpane::titem& rhs,
				 const std::string& tag,
				 const bool ascending)
{
#ifdef HAVE_CXX11
	if(ascending) {
		return lexical_cast<T>(lhs.tags.at(tag))
			   < lexical_cast<T>(rhs.tags.at(tag));
	} else {
		return lexical_cast<T>(lhs.tags.at(tag))
			   > lexical_cast<T>(rhs.tags.at(tag));
	}
#else
	typedef std::map<std::string,std::string>::const_iterator iterator;
	iterator lhs_it = lhs.tags.find(tag), rhs_it = rhs.tags.find(tag);
	if(lhs_it == lhs.tags.end() || rhs_it == rhs.tags.end()) {
		throw std::out_of_range("Key »" + tag + "« doesn't exist.");
	}
	if(ascending) {
		return lexical_cast<T>(*lhs_it) < lexical_cast<T>(*rhs_it);
	} else {
		return lexical_cast<T>(*lhs_it) > lexical_cast<T>(*rhs_it);
	}
#endif
}

/**
 * A filter testing whether a search string is used in a text.
 *
 * The comparison is a lower-case comparison.
 *
 * The function is expected to be used as a functor for
 * @ref gui2::tpane::filter().
 *
 * @param item                    The pane item to search in.
 * @param tag                     The tag of the field containing the text to
 *                                search in. @note This text field should
 *                                already contain lower-cased text only.
 * @param text_box                A text box object whose text is the search
 *                                string. The text in the text box will be
 *                                converted to lower-cased text before the
 *                                comparison is done.
 *
 * @returns                       Whether or not the comparison found a match.
 */
inline bool contains(const tpane::titem& item,
					 const std::string& tag,
					 const ttext_box& text_box)
{
#ifdef HAVE_CXX11
	return item.tags.at(tag).find(utf8::lowercase(text_box.text()))
		   != std::string::npos;
#else
	std::map<std::string,std::string>::const_iterator it = item.tags.find(tag);
	if(it == item.tags.end()) {
		throw std::out_of_range("Key »" + tag + "« doesn't exist.");
	}
	return it->second.find(utf8::lowercase(text_box.text())) != std::string::npos;
#endif
}

} // namespace gui2

#endif
