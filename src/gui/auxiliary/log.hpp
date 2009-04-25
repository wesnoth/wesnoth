/* $Id$ */
/*
   Copyright (C) 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef GUI_AUXILIRY_LOG_HPP_INCLUDED
#define GUI_AUXILIRY_LOG_HPP_INCLUDED

#include "../../log.hpp"

namespace gui2 {

extern lg::log_domain log_gui;
#define DBG_G_L LOG_STREAM_INDENT(debug, log_gui_layout)
#define LOG_G_L LOG_STREAM_INDENT(info, log_gui_layout)
#define WRN_G_L LOG_STREAM_INDENT(warn, log_gui_layout)
#define ERR_G_L LOG_STREAM_INDENT(err, log_gui_layout)

extern lg::log_domain log_gui_draw;
#define DBG_G_D LOG_STREAM_INDENT(debug, log_gui_draw)
#define LOG_G_D LOG_STREAM_INDENT(info, log_gui_draw)
#define WRN_G_D LOG_STREAM_INDENT(warn, log_gui_draw)
#define ERR_G_D LOG_STREAM_INDENT(err, log_gui_draw)

extern lg::log_domain log_gui_event;
#define DBG_G_E LOG_STREAM_INDENT(debug, log_gui_event)
#define LOG_G_E LOG_STREAM_INDENT(info, log_gui_event)
#define WRN_G_E LOG_STREAM_INDENT(warn, log_gui_event)
#define ERR_G_E LOG_STREAM_INDENT(err, log_gui_event)

extern lg::log_domain log_gui_layout;
#define DBG_GUI LOG_STREAM_INDENT(debug, log_gui)
#define LOG_GUI LOG_STREAM_INDENT(info, log_gui)
#define WRN_GUI LOG_STREAM_INDENT(warn, log_gui)
#define ERR_GUI LOG_STREAM_INDENT(err, log_gui)

extern lg::log_domain log_gui_parse;
#define DBG_G_P LOG_STREAM_INDENT(debug, log_gui_parse)
#define LOG_G_P LOG_STREAM_INDENT(info, log_gui)_parse
#define WRN_G_P LOG_STREAM_INDENT(warn, log_gui_parse)
#define ERR_G_P LOG_STREAM_INDENT(err, log_gui_parse)

} // namespace gui2

#endif
