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

#include "ogl/context.hpp"

#include "log.hpp"

#include <GL/glew.h>
#include <SDL.h>

#include <iostream>
#include <stdexcept>

static lg::log_domain log_opengl("opengl");
#define LOG_GL LOG_STREAM(info, log_opengl)
#define ERR_GL LOG_STREAM(err, log_opengl)

namespace gl
{

context::~context()
{
	SDL_GL_DeleteContext(gl_context_);
}

void context::init(sdl::window* window)
{
	// Set flags.
	set_context_flags();

	// Attempt to create the context.
	gl_context_ = SDL_GL_CreateContext(*window);
	if(gl_context_ == nullptr) {
		ERR_GL << "Error creating OpenGL context: too old hardware/drivers?\n";
		ERR_GL << SDL_GetError() << std::endl;
		throw std::runtime_error("error creating OpenGL context");
	}

	// Initialize GLEW.
	glewExperimental = GL_TRUE;
	GLenum result = glewInit();
	if(result != GLEW_OK) {
		ERR_GL << "Error initializing GLEW\n";
		throw std::runtime_error("error initializing GLEW");
	}

	// Print some information.
	GLint profile;
	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
	bool core_profile = profile & GL_CONTEXT_CORE_PROFILE_BIT;

	LOG_GL << "Using OpenGL " << (core_profile ? "core profile " : "") <<
		"version " << glGetString(GL_VERSION) << std::endl;
	LOG_GL << "GPU: " <<
		glGetString(GL_VENDOR) << " " << glGetString(GL_RENDERER) << std::endl;
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
