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

#include "ogl/vertex.hpp"

#include <GL/glew.h>

#include <array>

namespace gl
{
/** Represents a single draw operation.
The shape is always a triangle. */
struct draw_op
{
	std::array<vertex, 3> vertices;
	GLuint texture_name;
};

}
