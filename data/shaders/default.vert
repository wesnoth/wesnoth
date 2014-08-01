#version 120

// Required by SDL_gpu
attribute vec4 draw_color;
attribute vec3 vertex;
attribute vec2 texture_pos;
attribute vec4 color;
attribute float submerge;

uniform mat4 model_view_proj;

varying vec4 color_mod;
varying vec2 pos;
varying float submerge_amount;
varying vec4 color_draw;

void main()
{
	pos = texture_pos;
	color_mod = color;
	color_draw = draw_color;
	submerge_amount = submerge;
	gl_Position = model_view_proj * vec4(vertex, 1.0);
}
