/* $Id$ */
/*
   Copyright (C) 2005 - 2008 by Joeri Melis <joeri_melis@hotmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file loadscreen_empty.cpp
//!

#include "loadscreen.hpp"

#include "font.hpp"
#include "marked-up_text.hpp"

//#include <iostream>

#define MIN_PERCENTAGE   0
#define MAX_PERCENTAGE 100

void loadscreen::set_progress(const int /*percentage*/, const std::string &/*text*/, const bool /*commit*/)
{}

void loadscreen::increment_progress(const int /*percentage*/, const std::string &/*text*/, const bool /*commit*/) {}

void loadscreen::clear_screen(const bool /*commit*/) {}

loadscreen *loadscreen::global_loadscreen = 0;

#define CALLS_TO_FILESYSTEM 112
#define PRCNT_BY_FILESYSTEM  20
#define CALLS_TO_BINARYWML 9561
#define PRCNT_BY_BINARYWML   20
#define CALLS_TO_SETCONFIG  306
#define PRCNT_BY_SETCONFIG   30
#define CALLS_TO_PARSER   50448
#define PRCNT_BY_PARSER      20

void increment_filesystem_progress () {}

void increment_binary_wml_progress () {}

void increment_set_config_progress () {}

void increment_parser_progress () {}

