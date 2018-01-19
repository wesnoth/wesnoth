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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/core/window_builder/helper.hpp"

#include "config.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/window.hpp"

namespace gui2
{

namespace implementation
{

static std::map<std::string, scrollbar_mode> scrollbar_mode_map {
	{ "always",        scrollbar_mode::ALWAYS_VISIBLE },
	{ "never",         scrollbar_mode::ALWAYS_INVISIBLE },
	{ "auto",          scrollbar_mode::AUTO_VISIBLE },
	{ "initial_auto",  scrollbar_mode::AUTO_VISIBLE_FIRST_RUN },
};

unsigned get_v_align(const std::string& v_align)
{
	if(v_align == "top") {
		return grid::VERTICAL_ALIGN_TOP;
	} else if(v_align == "bottom") {
		return grid::VERTICAL_ALIGN_BOTTOM;
	} else {
		if(!v_align.empty() && v_align != "center") {
			ERR_GUI_E << "Invalid vertical alignment '" << v_align
					  << "' falling back to 'center'.\n";
		}
		return grid::VERTICAL_ALIGN_CENTER;
	}
}

unsigned get_h_align(const std::string& h_align)
{
	if(h_align == "left") {
		return grid::HORIZONTAL_ALIGN_LEFT;
	} else if(h_align == "right") {
		return grid::HORIZONTAL_ALIGN_RIGHT;
	} else {
		if(!h_align.empty() && h_align != "center") {
			ERR_GUI_E << "Invalid horizontal alignment '" << h_align
					  << "' falling back to 'center'.\n";
		}
		return grid::HORIZONTAL_ALIGN_CENTER;
	}
}

unsigned get_border(const std::vector<std::string>& borders)
{
	unsigned result = 0;
	for(const auto & border : borders)
	{
		if(border == "all") {
			return grid::BORDER_ALL;
		} else if(border == "top") {
			result |= grid::BORDER_TOP;
		} else if(border == "bottom") {
			result |= grid::BORDER_BOTTOM;
		} else if(border == "left") {
			result |= grid::BORDER_LEFT;
		} else if(border == "right") {
			result |= grid::BORDER_RIGHT;
		}
	}
	return result;
}

unsigned read_flags(const config& cfg)
{
	unsigned flags = 0;

	const unsigned v_flags = get_v_align(cfg["vertical_alignment"]);
	const unsigned h_flags = get_h_align(cfg["horizontal_alignment"]);
	flags |= get_border(utils::split(cfg["border"]));

	if(cfg["vertical_grow"].to_bool()) {
		flags |= grid::VERTICAL_GROW_SEND_TO_CLIENT;

		if(!(cfg["vertical_alignment"]).empty()) {
			ERR_GUI_P << "vertical_grow and vertical_alignment "
						 "can't be combined, alignment is ignored.\n";
		}
	} else {
		flags |= v_flags;
	}

	if(cfg["horizontal_grow"].to_bool()) {
		flags |= grid::HORIZONTAL_GROW_SEND_TO_CLIENT;

		if(!(cfg["horizontal_alignment"]).empty()) {
			ERR_GUI_P << "horizontal_grow and horizontal_alignment "
						 "can't be combined, alignment is ignored.\n";
		}
	} else {
		flags |= h_flags;
	}

	return flags;
}

scrollbar_mode get_scrollbar_mode(const std::string& scrollbar_mode)
{
	if(scrollbar_mode.empty()) {
		return scrollbar_container::AUTO_VISIBLE_FIRST_RUN;
	}

	if(scrollbar_mode_map.find(scrollbar_mode) == scrollbar_mode_map.end()) {
		ERR_GUI_E << "Invalid scrollbar mode '" << scrollbar_mode << "'."
		          << "Falling back to 'initial_auto'." << std::endl;

		return scrollbar_container::AUTO_VISIBLE_FIRST_RUN;
	}

	return scrollbar_mode_map[scrollbar_mode];
}

int get_retval(const std::string& retval_id,
			   const int retval,
			   const std::string& id)
{
	if(!retval_id.empty()) {
		int result = window::get_retval_by_id(retval_id);
		if(result) {
			return result;
		} else {
			ERR_GUI_E << "Window builder: retval_id '" << retval_id
					  << "' is unknown.\n";
		}
	}

	if(retval) {
		return retval;
	} else {
		return window::get_retval_by_id(id);
	}
}

} // namespace implementation

} // namespace gui2
