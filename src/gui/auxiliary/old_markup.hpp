/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include <string>

namespace gui2
{

/**
 * Implements simple parsing of legacy GUI1 item markup.
 */
class legacy_menu_item
{
	/*
	 * Legacy options/menu items have some special markup:
	 * A line starting with a * is selected by default.
	 * A line starting with a & enables the following markup:
	 * - The part until the = is the name of an image.
	 * - The part until the second = is the first column.
	 * - The rest is the third column (the wiki only specifies two columns
	 *   so only two of them are implemented).
	 */
	/**
	 * @todo This syntax looks like a bad hack, it would be nice to write
	 * a new syntax which doesn't use those hacks (also avoids the problem
	 * with special meanings for certain characters.
	 */
public:
	explicit legacy_menu_item(const std::string& str = "", const std::string deprecation_msg = "");

	const std::string& icon() const
	{
		return icon_;
	}

	const std::string& label() const
	{
		return label_;
	}

	const std::string& description() const
	{
		return desc_;
	}

	bool is_default() const
	{
		return default_;
	}

	bool contained_markup() const
	{
		return contained_markup_;
	}

	legacy_menu_item& operator=(const legacy_menu_item& rhs)
	{
		if(&rhs != this) {
			icon_ = rhs.icon_;
			label_ = rhs.label_;
			desc_ = rhs.desc_;
			default_ = rhs.default_;
		}
		return *this;
	}

private:
	/** The icon for the menu item. */
	std::string icon_;

	/** The first text item of the menu item, normally a short string. */
	std::string label_;

	/** The second text item of the menu item, normally a longer string. */
	std::string desc_;

	/**
	 * Is the item the default item and thus initially selected.
	 *
	 * It's unspecified what happens if multiple items in a menu are selected.
	 */
	bool default_;

	/**
	 * Was any old markup actually parsed?
	 */
	bool contained_markup_;
};
}
