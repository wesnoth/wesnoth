/*
   Copyright (C) 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

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
 * Tool to test SDL 2 functions.
 */

#include "tools/sdl2/sdl2.hpp"

#include "sdl/texture.hpp"
#include "sdl/window.hpp"
#include "tools/sdl2/window.hpp"
#include "text.hpp"

#include <SDL.h>

#include <boost/foreach.hpp>

#include <iostream>
#include <deque>

std::vector<sdl::twindow*> windows;

struct texit
{
};

struct tsdl
{
	tsdl()
	{
		if(SDL_Init(SDL_INIT_VIDEO) != 0) {
			std::cerr << "Unable to initialise SDL with error »"
					  << SDL_GetError() << "«.\n";

			throw texit();
		}
	}

	~tsdl()
	{

		SDL_Quit();
	}
};

struct ttext_input
{
	ttext_input()
	{
		SDL_StartTextInput();
	}

	~ttext_input()
	{
		SDL_StopTextInput();
	}
};

static void draw_text(sdl::twindow& window,
					  const std::string& string,
					  const int x,
					  const int y)
{
	if(string.empty()) {
		return;
	}

	static font::ttext text;
	text.set_text(string, false);
	sdl::ttexture texture
			= window.create_texture(SDL_TEXTUREACCESS_STATIC, text.render());

	window.draw(texture, x, y);
}

static void draw_command_line(sdl::twindow& window, const std::string& command)
{
	draw_text(window, command, 15, 575);
}

static void draw_command_history(sdl::twindow& window,
								 const std::deque<std::string>& history)
{
	unsigned offset = 535;
	BOOST_REVERSE_FOREACH(const std::string & item, history)
	{
		draw_text(window, item, 25, offset);
		offset -= 25;
	}
}

static bool execute_command(std::string& command_line)
{
	std::string::const_iterator begin = command_line.begin();
	const std::string command = get_token(command_line, begin, ' ');

	if(command == "quit") {

		utf8::insert(command_line, utf8::size(command_line), " -> Ok");
		return false;
	} else if(command == "window") {

		execute_window(command_line, begin);
	}

	return true;
}

std::string get_token(const std::string& string,
					  std::string::const_iterator& begin,
					  const char separator)
{
	const std::string::const_iterator end
			= std::find(begin, string.end(), separator);

	std::string result = std::string(begin, end);

	begin = end;
	if(begin != string.end()) {
		++begin;
	}

	return result;
}

int main()
{
	try
	{
		const tsdl sdl;
		bool dirty = false;
		std::string line;

		std::deque<std::string> history;

		SDL_Event event;
		sdl::twindow window("SDL2 test application",
							SDL_WINDOWPOS_CENTERED,
							SDL_WINDOWPOS_CENTERED,
							800,
							600,
							0,
							SDL_RENDERER_TARGETTEXTURE);

		SDL_Rect rect = { 25, 575, 750, 25 };
		SDL_SetTextInputRect(&rect);
		const ttext_input text_input;

		bool run = true;
		while(run) {
			if(dirty) {
				window.clear();
				draw_command_line(window, line);
				draw_command_history(window, history);
				dirty = false;
			}
			window.render();
			BOOST_FOREACH(sdl::twindow * window, windows)
			{
				if(window) {
					window->render();
				}
			}
			if(SDL_WaitEvent(&event) == 0) {
				return EXIT_FAILURE;
			}

			switch(event.type) {
				case SDL_TEXTINPUT:
					utf8::insert(line, utf8::size(line), event.text.text);
					dirty = true;
					break;

				case SDL_TEXTEDITING:
					/* Seems not to be called. */
					break;

				case SDL_KEYDOWN:
					if(event.key.keysym.sym == SDLK_BACKSPACE) {
						if(!line.empty()) {
							utf8::erase(line, utf8::size(line) - 1);
							dirty = true;
						}
					}

					break;

				case SDL_KEYUP:
					if(event.key.keysym.sym == SDLK_RETURN) {
						run = execute_command(line);
						history.push_back(line);
						if(history.size() > 22) {
							history.pop_front();
						}
						line.clear();
						dirty = true;
						SDL_RaiseWindow(window);
					}
					break;

				case SDL_QUIT:
					return EXIT_SUCCESS;
			}
		}
	}
	catch(...)
	{
		return EXIT_FAILURE;
	}
}
