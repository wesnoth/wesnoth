/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/helper.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

namespace implementation {

unsigned get_v_align(const std::string& v_align)
{
	if(v_align == "top") {
		return tgrid::VERTICAL_ALIGN_TOP;
	} else if(v_align == "bottom") {
		return tgrid::VERTICAL_ALIGN_BOTTOM;
	} else {
		if(!v_align.empty() && v_align != "center") {
			ERR_GUI_E << "Invalid vertical alignment '"
				<< v_align << "' falling back to 'center'.\n";
		}
		return tgrid::VERTICAL_ALIGN_CENTER;
	}
}

unsigned get_h_align(const std::string& h_align)
{
	if(h_align == "left") {
		return tgrid::HORIZONTAL_ALIGN_LEFT;
	} else if(h_align == "right") {
		return tgrid::HORIZONTAL_ALIGN_RIGHT;
	} else {
		if(!h_align.empty() && h_align != "center") {
			ERR_GUI_E << "Invalid horizontal alignment '"
				<< h_align << "' falling back to 'center'.\n";
		}
		return tgrid::HORIZONTAL_ALIGN_CENTER;
	}
}

unsigned get_border(const std::vector<std::string>& border)
{
	if(std::find(border.begin(), border.end(), std::string("all"))
				!= border.end()) {

		return tgrid::BORDER_TOP
			| tgrid::BORDER_BOTTOM | tgrid::BORDER_LEFT | tgrid::BORDER_RIGHT;
	} else {
		if(std::find(border.begin(), border.end(), std::string("top"))
				!= border.end()) {

			return tgrid::BORDER_TOP;
		}
		if(std::find(border.begin(), border.end(), std::string("bottom"))
				!= border.end()) {

			return tgrid::BORDER_BOTTOM;
		}
		if(std::find(border.begin(), border.end(), std::string("left"))
				!= border.end()) {

			return tgrid::BORDER_LEFT;
		}
		if(std::find(border.begin(), border.end(), std::string("right"))
				!= border.end()) {

			return tgrid::BORDER_RIGHT;
		}
	}

	return 0;
}

unsigned read_flags(const config& cfg)
{
	unsigned flags = 0;

	const unsigned v_flags = get_v_align(cfg["vertical_alignment"]);
	const unsigned h_flags = get_h_align(cfg["horizontal_alignment"]);
	flags |= get_border( utils::split(cfg["border"]));

	if(utils::string_bool(cfg["vertical_grow"])) {
		flags |= tgrid::VERTICAL_GROW_SEND_TO_CLIENT;

		if(! (cfg["vertical_alignment"]).empty()) {
			ERR_GUI_P << "vertical_grow and vertical_alignment "
				"can't be combined, alignment is ignored.\n";
		}
	} else {
		flags |= v_flags;
	}

	if(utils::string_bool(cfg["horizontal_grow"])) {
		flags |= tgrid::HORIZONTAL_GROW_SEND_TO_CLIENT;

		if(! (cfg["horizontal_alignment"]).empty()) {
			ERR_GUI_P << "horizontal_grow and horizontal_alignment "
				"can't be combined, alignment is ignored.\n";
		}
	} else {
		flags |= h_flags;
	}

	return flags;
}

tscrollbar_container::tscrollbar_mode
		get_scrollbar_mode(const std::string& scrollbar_mode)
{
	if(scrollbar_mode == "always") {
		return tscrollbar_container::always_visible;
	} else if(scrollbar_mode == "never") {
		return tscrollbar_container::always_invisible;
	} else if(scrollbar_mode == "initial_auto"
			|| (gui2::new_widgets && scrollbar_mode.empty())) {
		return tscrollbar_container::auto_visible_first_run;
	} else {
		if(!scrollbar_mode.empty() && scrollbar_mode != "auto") {
			ERR_GUI_E << "Invalid scrollbar mode '"
				<< scrollbar_mode << "' falling back to 'auto'.\n";
		}
		return tscrollbar_container::auto_visible;
	}
}

int get_retval(const std::string& retval_id,
		const int retval, const std::string& id)
{
	if(!retval_id.empty()) {
		int result = twindow::get_retval_by_id(retval_id);
		if(result) {
			return result;
		} else {
			ERR_GUI_E << "Window builder: retval_id '"
					<< retval_id << "' is unknown.\n";
		}
	}

	if(retval) {
		return retval;
	} else {
		return twindow::get_retval_by_id(id);
	}
}

} // namespace implementation

} // namespace gui2

