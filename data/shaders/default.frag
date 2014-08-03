#version 120

varying vec4 frag_color_mod;
varying vec4 frag_draw_color;
varying vec2 frag_texture_pos;
varying float frag_submerge;
uniform sampler2D tex;

void main()
{
	gl_FragColor = texture2D(tex, frag_texture_pos)
					+ frag_draw_color + frag_color_mod;
}
