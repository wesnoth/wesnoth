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

//! @file editor_common.hpp
//! Main (common) editor header

#ifndef EDITOR2_EDITOR_COMMON_HPP_INCLUDED
#define EDITOR2_EDITOR_COMMON_HPP_INCLUDED

#include "../log.hpp"
#define DBG_ED LOG_STREAM_INDENT(debug, editor)
#define LOG_ED LOG_STREAM_INDENT(info, editor)
#define WRN_ED LOG_STREAM_INDENT(warn, editor)
#define ERR_ED LOG_STREAM_INDENT(err, editor)
#define SCOPE_ED log_scope2(editor, __FUNCTION__)


namespace editor2 {

struct editor_exception
{
};


// forward declarations
class editor_map;
class brush;

} //end namespace editor2

#endif
