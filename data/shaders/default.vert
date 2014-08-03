#version 120

// Required by SDL_gpu
attribute vec4 vert_draw_color;
attribute vec3 vert_vertex;
attribute vec2 vert_texture_pos;
attribute vec4 vert_color_mod;
attribute float vert_submerge;

uniform mat4 model_view_proj;

varying vec4 frag_color_mod;
varying vec2 frag_texture_pos;
varying float frag_submerge;
varying vec4 frag_draw_color;

void main()
{
	frag_texture_pos = vert_texture_pos;
	frag_color_mod = vert_color_mod;
	frag_draw_color = vert_draw_color;
	frag_submerge = vert_submerge;
	gl_Position = model_view_proj * vec4(vert_vertex, 1.0);
}
