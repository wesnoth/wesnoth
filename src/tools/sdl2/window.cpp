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

#include "tools/sdl2/window.hpp"

#include "tools/sdl2/sdl2.hpp"

#include "sdl/window.hpp"
#include "serialization/unicode.hpp"

#include <boost/lexical_cast.hpp>

static void create(std::string& command, std::string::const_iterator begin)
{
	std::string title;
	int x = SDL_WINDOWPOS_CENTERED;
	int y = SDL_WINDOWPOS_CENTERED;
	int w = 800;
	int h = 600;

	while(begin != command.end()) {
		const std::string argument = get_token(command, begin, ' ');

		if(argument.find("x=") == 0) {
			x = boost::lexical_cast<int>(argument.substr(2));
		} else if(argument.find("y=") == 0) {
			y = boost::lexical_cast<int>(argument.substr(2));
		} else if(argument.find("w=") == 0) {
			w = boost::lexical_cast<int>(argument.substr(2));
		} else if(argument.find("h=") == 0) {
			h = boost::lexical_cast<int>(argument.substr(2));
		} else if(argument.find("title=") == 0) {
			title = argument.substr(6); /* todo allow spaces. */
		} else {
			utf8::insert(command, utf8::size(command), " -> Unknown argument");
		}
	}

	windows.push_back(new sdl::twindow(
			title.c_str(), x, y, w, h, 0, SDL_RENDERER_TARGETTEXTURE));

	utf8::insert(command,
				 utf8::size(command),
				 " -> OK ID "
				 + boost::lexical_cast<std::string>(windows.size() - 1));
}

static void modify(std::string& command, std::string::const_iterator begin)
{
	std::size_t id
			= boost::lexical_cast<std::size_t>(get_token(command, begin, ' '));

	if(id >= windows.size()) {
		utf8::insert(command, utf8::size(command), " -> ID out of range");
	} else if(windows[id] == NULL) {
		utf8::insert(command, utf8::size(command), " -> ID destroyed");
	} else {

		std::string title = SDL_GetWindowTitle(*windows[id]);
		int w;
		int h;
		SDL_GetWindowSize(*windows[id], &w, &h);

		while(begin != command.end()) {
			const std::string argument = get_token(command, begin, ' ');

			if(argument.find("w=") == 0) {
				w = boost::lexical_cast<int>(argument.substr(2));
			} else if(argument.find("h=") == 0) {
				h = boost::lexical_cast<int>(argument.substr(2));
			} else if(argument.find("title=") == 0) {
				title = argument.substr(6); /* todo allow spaces. */
			} else {
				utf8::insert(
						command, utf8::size(command), " -> Unknown argument");
			}
		}

		windows[id]->set_title(title);
		windows[id]->set_size(w, h);
		utf8::insert(command, utf8::size(command), " -> OK");
	}
}

static void destroy(std::string& command, std::string::const_iterator begin)
{
	std::size_t id
			= boost::lexical_cast<std::size_t>(get_token(command, begin, ' '));

	if(id >= windows.size()) {
		utf8::insert(command, utf8::size(command), " -> ID out of range");
	} else if(windows[id] == NULL) {
		utf8::insert(command, utf8::size(command), " -> ID already destroyed");
	} else {
		delete windows[id];
		windows[id] = NULL;
		utf8::insert(command, utf8::size(command), " -> OK");
	}
}

void execute_window(std::string& command, std::string::const_iterator begin)
{
	const std::string action = get_token(command, begin, ' ');

	if(action == "create") {
		create(command, begin);
	} else if(action == "modify") {
		modify(command, begin);
	} else if(action == "destroy") {
		destroy(command, begin);
	} else {
		utf8::insert(command, utf8::size(command), " -> Unknown action");
	}
}
