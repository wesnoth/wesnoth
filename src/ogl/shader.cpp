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

#include "ogl/shader.hpp"

#include "log.hpp"
#include "ogl/vertex.hpp"

#include <array>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <tuple>
#include <utility>

static lg::log_domain log_opengl("opengl");
#define LOG_GL LOG_STREAM(info, log_opengl)
#define ERR_GL LOG_STREAM(err, log_opengl)

namespace
{

std::pair<bool, std::string> compile_subshader(GLuint shader, const std::string& source_code)
{
	GLint compile_status;
	std::array<char, 1024> compile_log;

	int src_length = source_code.length();
	const char* src_char_array = source_code.data();
	glShaderSource(shader, 1, &src_char_array, &src_length);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_status);
	glGetShaderInfoLog(shader, compile_log.size(), nullptr, compile_log.data());

	return {compile_status == GL_TRUE, {compile_log.data()}};
}

}

namespace gl
{

shader::~shader()
{
	std::array<GLuint, 2> subshaders;
	glGetAttachedShaders(shader_program_, subshaders.size(), nullptr, subshaders.data());

	glDeleteProgram(shader_program_);
	for(const GLuint s : subshaders) {
		glDeleteShader(s);
	}
}

void shader::activate() const
{
	glUseProgram(shader_program_);

	GLint position_index = glGetAttribLocation(shader_program_, "position");
	GLint texcoord_index = glGetAttribLocation(shader_program_, "tex_coord");

	glVertexAttribPointer(position_index, 2, GL_FLOAT, false, sizeof(vertex),
		reinterpret_cast<const void*>(offsetof(vertex, x)));
	glVertexAttribPointer(texcoord_index, 2, GL_FLOAT, false, sizeof(vertex),
		reinterpret_cast<const void*>(offsetof(vertex, u)));

	glEnableVertexAttribArray(position_index);
	glEnableVertexAttribArray(texcoord_index);
}

void shader::compile(const std::string& vertex_shader_src, const std::string& pixel_shader_src)
{
	bool success;
	std::string compile_log;

	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	std::tie(success, compile_log) = compile_subshader(vertex_shader, vertex_shader_src);
	if(!success) {
		ERR_GL << "Error compiling vertex shader\n" << compile_log << std::endl;
		throw std::invalid_argument("error compiling vertex shader");
	} else if(compile_log != "") {
		LOG_GL << compile_log << std::endl;
	}

	GLuint pixel_shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::tie(success, compile_log) = compile_subshader(pixel_shader, pixel_shader_src);
	if(!success) {
		ERR_GL << "Error compiling pixel shader\n" << compile_log << std::endl;
		throw std::invalid_argument("error compiling pixel shader");
	} else if(compile_log != "") {
		LOG_GL << compile_log << std::endl;
	}

	shader_program_ = glCreateProgram();
	glAttachShader(shader_program_, vertex_shader);
	glAttachShader(shader_program_, pixel_shader);

	glLinkProgram(shader_program_);
}

}