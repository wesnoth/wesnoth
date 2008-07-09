/* $Id$ */
/*
   Copyright (C) 2008 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** 
 * @file editor_common.hpp
 * Main (common) editor header. Contains forward declarations for most classes, 
 * logging macro definitions and base exception declarations
 */

#ifndef EDITOR2_EDITOR_COMMON_HPP_INCLUDED
#define EDITOR2_EDITOR_COMMON_HPP_INCLUDED

#include "../log.hpp"
#include <stdexcept>

#define DBG_ED LOG_STREAM_INDENT(debug, editor)
#define LOG_ED LOG_STREAM_INDENT(info, editor)
#define WRN_ED LOG_STREAM_INDENT(warn, editor)
#define ERR_ED LOG_STREAM_INDENT(err, editor)
#define SCOPE_ED log_scope2(editor, __FUNCTION__)


namespace editor2 {

struct editor_exception : public std::exception
{
};


// forward declarations
class brush;
class editor_action;
class editor_controller;
class editor_display;
class editor_map;
class editor_mouse_handler;
class editor_mode;
class mouse_action;

} //end namespace editor2

#endif
