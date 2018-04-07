/*
   Copyright (C) 2018 by Jyrki Vesterinen <sandgtx@gmail.com>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include <GL/glew.h>

#include <string>

namespace gl
{

class shader
{
public:
	/** Constructor.
	@param vertex_shader_src Vertex shader source code.
	@param pixel_shader_src  Pixel shader source code.
	@throw std::invalid_argument if compiling the shader fails.
	*/
	shader(const std::string& vertex_shader_src, const std::string& pixel_shader_src)
	{
		compile(pixel_shader_src, vertex_shader_src);
	}

	/// Destructor.
	~shader();

	/// Makes the shader the active shader.
	/// The right VBO and VAO must be already bound.
	void activate() const;

private:
	void compile(const std::string& vertex_shader_src, const std::string& pixel_shader_src);

	GLuint shader_program_ = 0u;
};

}
