/*
 *  server_main.cpp
 *  Wesnoth
 *
 *  Created by Ben Anderman on 4/5/09.
 *  Copyright 2009 Ben Anderman.
 * 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.
 */

#import "SDL.h"
#import "SDLMain.h"

#ifdef main
#  undef main
#endif

int SDL_main (int argc, char **argv);

int main (int argc, char **argv)
{
	return SDL_main(argc, argv);
}