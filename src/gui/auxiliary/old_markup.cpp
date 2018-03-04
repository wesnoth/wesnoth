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

#include "gui/auxiliary/old_markup.hpp"
#include "deprecation.hpp"
#include "version.hpp"

namespace gui2
{

legacy_menu_item::legacy_menu_item(const std::string& str, const std::string deprecation_msg)
	: icon_(), label_(str), desc_(), default_(false), contained_markup_(false)
{
	if(label_.empty()) {
		return;
	}

	// Handle selection.
	if(label_[0] == '*') {
		default_ = true;
		label_.erase(0, 1);
		contained_markup_ = true;
	}

	// Handle the special case with an image.
	// 99.9% of uses put the image in the first column, so we ignore the slim possibility of it going in a different column
	std::string::size_type pos = label_.find('=');
	if(pos != std::string::npos && (label_[0] == '&' || pos == 0)) {
		if(pos)
			icon_ = label_.substr(1, pos - 1);
		label_.erase(0, pos + 1);
		contained_markup_ = true;
	}

	// Search for an '=' symbol that is not inside markup.
	std::string::size_type prev = 0;
	bool open = false;
	while((pos = label_.find('=', prev)) != std::string::npos) {
		for(std::string::size_type i = prev; i != pos; ++i) {
			switch(label_[i]) {
				case '<':
					open = true;
					break;
				case '>':
					open = false;
					break;
			}
		}
		if(!open)
			break;
		prev = pos + 1;
	}
	if(pos != std::string::npos) {
		desc_ = label_.substr(pos + 1);
		label_.erase(pos);
		contained_markup_ = true;
	}

	if(contained_markup_) {
		deprecated_message("Legacy DescriptionWML markup (&img=col1=col2)", 3, {1, 15, 0}, deprecation_msg);
	}
}
}
