/*
   Copyright (C) 2009 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
 * Define the common log macros for the gui toolkit.
 */

#pragma once

#include "../../log.hpp" // We want the file in src/

namespace gui2
{

extern lg::log_domain log_gui_draw;
#define DBG_GUI_D LOG_STREAM_INDENT(debug, gui2::log_gui_draw)
#define LOG_GUI_D LOG_STREAM_INDENT(info, gui2::log_gui_draw)
#define WRN_GUI_D LOG_STREAM_INDENT(warn, gui2::log_gui_draw)
#define ERR_GUI_D LOG_STREAM_INDENT(err, gui2::log_gui_draw)

extern lg::log_domain log_gui_event;
#define DBG_GUI_E LOG_STREAM_INDENT(debug, gui2::log_gui_event)
#define LOG_GUI_E LOG_STREAM_INDENT(info, gui2::log_gui_event)
#define WRN_GUI_E LOG_STREAM_INDENT(warn, gui2::log_gui_event)
#define ERR_GUI_E LOG_STREAM_INDENT(err, gui2::log_gui_event)

extern lg::log_domain log_gui_general;
#define DBG_GUI_G LOG_STREAM_INDENT(debug, gui2::log_gui_general)
#define LOG_GUI_G LOG_STREAM_INDENT(info, gui2::log_gui_general)
#define WRN_GUI_G LOG_STREAM_INDENT(warn, gui2::log_gui_general)
#define ERR_GUI_G LOG_STREAM_INDENT(err, gui2::log_gui_general)

extern lg::log_domain log_gui_iterator;
#define TST_GUI_I                                                              \
	if(lg::debug().dont_log(gui2::log_gui_iterator))                             \
		;                                                                      \
	else                                                                       \
	lg::debug()(gui2::log_gui_iterator, false, false)
#define DBG_GUI_I LOG_STREAM_INDENT(debug, gui2::log_gui_iterator)
#define LOG_GUI_I LOG_STREAM_INDENT(info, gui2::log_gui_iterator)
#define WRN_GUI_I LOG_STREAM_INDENT(warn, gui2::log_gui_iterator)
#define ERR_GUI_I LOG_STREAM_INDENT(err, gui2::log_gui_iterator)

extern lg::log_domain log_gui_layout;
#define DBG_GUI_L LOG_STREAM_INDENT(debug, gui2::log_gui_layout)
#define LOG_GUI_L LOG_STREAM_INDENT(info, gui2::log_gui_layout)
#define WRN_GUI_L LOG_STREAM_INDENT(warn, gui2::log_gui_layout)
#define ERR_GUI_L LOG_STREAM_INDENT(err, gui2::log_gui_layout)

extern lg::log_domain log_gui_lifetime;
// lifetime logging only makes sense in debug level anyway
#define DBG_GUI_LF LOG_STREAM_INDENT(debug, gui2::log_gui_lifetime)


extern lg::log_domain log_gui_parse;
#define DBG_GUI_P LOG_STREAM_INDENT(debug, gui2::log_gui_parse)
#define LOG_GUI_P LOG_STREAM_INDENT(info, gui2::log_gui_parse)
#define WRN_GUI_P LOG_STREAM_INDENT(warn, gui2::log_gui_parse)
#define ERR_GUI_P LOG_STREAM_INDENT(err, gui2::log_gui_parse)

} // namespace gui2
