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


#ifdef SDL_GPU

#include "gpu.hpp"
#include <string>
#include <vector>
#include <boost/noncopyable.hpp>

namespace sdl {

class shader_program
{
public:
	shader_program(const std::string &vsrc, const std::string &fsrc);
	shader_program();
	~shader_program();

	shader_program(const shader_program &prog);
	const shader_program& operator =(const shader_program &prog);

	void activate();
	void deactivate();

private:
	Uint32 program_object_, vertex_object_, fragment_object_;
	GPU_ShaderBlock block_;
	unsigned *refcount_;
};

}
#endif
