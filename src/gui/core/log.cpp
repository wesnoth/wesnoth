/*
   Copyright (C) 2009 - 2017 by Mark de Wever <koraq@xs4all.nl>
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
 * Define the loggers for the gui toolkit.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/core/log.hpp"

namespace gui2
{

lg::log_domain log_gui_draw("gui/draw");
lg::log_domain log_gui_event("gui/event");
lg::log_domain log_gui_general("gui/general");
lg::log_domain log_gui_iterator("gui/iterator");
lg::log_domain log_gui_lifetime("gui/lifetime");
lg::log_domain log_gui_layout("gui/layout");
lg::log_domain log_gui_parse("gui/parse");

} // namespace gui2
