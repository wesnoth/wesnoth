/*
   Copyright (C) 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef USE_GL_RENDERING

#include "ogl/context.hpp"

#include <SDL_video.h>

namespace gl
{
context::context(sdl::window* window)
	: gl_context_(SDL_GL_CreateContext(*window))
{
	// Set flags.
	set_context_flags();

	// Initialize GLEW.
	// TODO: should this be moved somewhere else?
	glewExperimental = GL_TRUE;
	glewInit();
}

context::~context()
{
	SDL_GL_DeleteContext(gl_context_);
}

void context::set_context_flags()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

	// Turn on double buffering.
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	// Enable VSync (sync buffer refresh with monitor refresh rate).
	SDL_GL_SetSwapInterval(1);
}

}

#endif // USE_GL_RENDERING
