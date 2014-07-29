#version 120

attribute vec4 color;
attribute vec3 vertex;
attribute vec2 texture_pos;

uniform mat4 model_view_proj;

varying vec4 color_mod;
varying vec2 pos;

void main()
{
	pos = texture_pos;
	color_mod = color;
	gl_Position = model_view_proj * vec4(vertex, 1.0);
}
