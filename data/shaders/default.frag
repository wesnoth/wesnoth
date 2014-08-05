#version 130
#define EFFECT_FLIP 1
#define EFFECT_FLOP 2
#define EFFECT_GRAYSCALE 4

varying vec4 frag_color_mod;
varying vec4 frag_draw_color;
varying vec2 frag_texture_pos;
varying float frag_submerge;
flat in int frag_effects;
uniform sampler2D tex;

void main()
{
	vec4 colors = texture2D(tex, frag_texture_pos);
	if ((frag_effects & EFFECT_GRAYSCALE) != 0) {
		float rgb = colors.r * 0.299 + colors.g * 0.587 + colors.b * 0.114;
		colors = vec4(rgb, rgb, rgb, colors.a);
	}
	float submerge_alpha = 0;
	if (frag_texture_pos.y > 1.0 - frag_submerge) {
		submerge_alpha = 0.7 + (frag_texture_pos.y - (1 - frag_submerge))
				/ frag_submerge * 0.3;
	}
	vec4 submerge_mod = vec4(0, 0, 0, submerge_alpha);
	gl_FragColor = colors + frag_draw_color + frag_color_mod - submerge_mod;
}
