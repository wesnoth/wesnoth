/*
   Copyright (C) 2014 by Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "shader.hpp"

#ifdef SDL_GPU
namespace sdl
{

shader_program::shader_program(const std::string &vsrc, const std::string &fsrc)
	: program_object_(0)
	, vertex_object_(0)
	, fragment_object_(0)
	, block_()
	, attr_color_mod_(0)
	, attr_submerge_(0)
	, refcount_(new unsigned(1))
{
	vertex_object_ = GPU_LoadShader(GPU_VERTEX_SHADER, vsrc.c_str());
	if (!vertex_object_) {
		//TODO: report error
	}

	fragment_object_ = GPU_LoadShader(GPU_FRAGMENT_SHADER, fsrc.c_str());
	if (!fragment_object_) {
		//TODO: report error
	}

	program_object_ = GPU_LinkShaders(vertex_object_, fragment_object_);
	if (!program_object_) {
		//TODO: report error
	}

	attr_color_mod_ = GPU_GetAttributeLocation(program_object_, "color");
	set_color_mod(0, 0, 0, 0);
	set_submerge(0);
}

shader_program::shader_program()
	: program_object_(0)
	, vertex_object_(0)
	, fragment_object_(0)
	, block_()
	, attr_color_mod_(0)
	, attr_submerge_(0)
	, refcount_(new unsigned(1))
{}

shader_program::~shader_program()
{
	(*refcount_)--;
	if (!*refcount_) {
		deactivate();
		GPU_FreeShader(vertex_object_);
		GPU_FreeShader(fragment_object_);
		GPU_FreeShaderProgram(program_object_);
	}
}

shader_program::shader_program(const shader_program &prog)
	: program_object_(prog.program_object_)
	, vertex_object_(prog.vertex_object_)
	, fragment_object_(prog.fragment_object_)
	, block_(prog.block_)
	, attr_color_mod_(prog.attr_color_mod_)
	, attr_submerge_(prog.attr_submerge_)
	, refcount_(prog.refcount_)
{
	(*refcount_)++;
}

const shader_program &shader_program::operator =(const shader_program &prog)
{
	if (&prog != this) {
		this->~shader_program();
		return *(new(this) shader_program(prog));
	}

	return *this;
}

void shader_program::activate()
{
	block_ = GPU_LoadShaderBlock(program_object_, "vertex", "texture_pos",
								 "draw_color", "model_view_proj");
	GPU_ActivateShaderProgram(program_object_, &block_);
}

void shader_program::deactivate()
{
	if (GPU_GetCurrentShaderProgram() == program_object_) {
		GPU_DeactivateShaderProgram();
	}
}

void shader_program::set_color_mod(int r, int g, int b, int a)
{
	static float color_mod[4];
	color_mod[0] = float(r) / 255;
	color_mod[1] = float(g) / 255;
	color_mod[2] = float(b) / 255;
	color_mod[3] = float(a) / 255;

	GPU_SetAttributefv(attr_color_mod_, 4, color_mod);
}

void shader_program::set_submerge(float val)
{
	GPU_SetAttributef(attr_submerge_, val);
}

}
#endif
