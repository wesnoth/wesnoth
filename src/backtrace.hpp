/*
   Copyright (C) 2015 by Chris Beck <render787@gmail.com>
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
 * Provide a method to print a back trace. Used by CrashReporter in case
 * of a crash. The implementation should adapt to the compiler / OS to
 * give the best possible results, but may also do nothing.
 */

#pragma once

extern void printBacktrace();
