/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef INTRO_HPP_INCLUDED
#define INTRO_HPP_INCLUDED

class config;
class CVideo;
#include "SDL.h"

#include <string>

//function to show an introduction sequence specified by data.
//the format of data is like,
//[part]
//id='id'
//story='story'
//image='img'
//[/part]
//where 'id' is a unique identifier, 'story' is text describing
//storyline, and 'img' is an image.
//
//each part of the sequence will be displayed in turn, with the
//user able to go to the next part, or skip it entirely.
void show_intro(CVideo &video, const config& data, const config& level);

void the_end(CVideo& screen);

#endif
