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

#pragma once

#ifdef USE_GL_RENDERING

#include "sdl/window.hpp"

#include <GL/glew.h>
#include <GL/gl.h>

namespace gl
{
/** Encapsulates the management of an OpenGL context for the current window. */
class context
{
public:
	context(const context&) = delete;
	context& operator=(const context&) = delete;

	/**
	 * Constructor
	 *
	 * @param window           The SDL window to attach a context to.
	 */
	context(sdl::window* window);

	~context();

private:
	/** Sets any relevant flags for the GL context. */
	void set_context_flags();

	/** The window's OpenGL context. */
	SDL_GLContext gl_context_;
};

} // namespace gl

#endif // USE_GL_RENDERING
