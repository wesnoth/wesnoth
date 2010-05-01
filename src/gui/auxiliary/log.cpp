/* $Id$ */
/*
   Copyright (C) 2009 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file gui/auxiliary/log.cpp
 * Define the loggers for the gui toolkit.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/log.hpp"

namespace gui2 {

lg::log_domain log_gui_draw    ("gui/draw");
lg::log_domain log_gui_event   ("gui/event");
lg::log_domain log_gui_general ("gui/general");
lg::log_domain log_gui_lifetime("gui/lifetime");
lg::log_domain log_gui_layout  ("gui/layout");
lg::log_domain log_gui_parse   ("gui/parse");

} // namespace gui2

