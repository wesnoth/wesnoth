#version 130
#define EFFECT_FLIP 1
#define EFFECT_FLOP 2
#define EFFECT_GRAYSCALE 4

in vec4 frag_color_mod;
in vec4 frag_draw_color;
in vec2 frag_texture_pos;
in float frag_submerge;
in vec2 frag_overlay_pos;
flat in int frag_effects;

uniform sampler2D tex;
uniform sampler2D overlay;

void main()
{
	vec4 texture_color = texture2D(tex, frag_texture_pos);
	vec4 overlay_color = texture2D(overlay, frag_overlay_pos);

	if ((frag_effects & EFFECT_GRAYSCALE) != 0) {
		float rgb = texture_color.r * 0.299 + texture_color.g * 0.587
					+ texture_color.b * 0.114;
		texture_color = vec4(rgb, rgb, rgb, texture_color.a);
	}
	float submerge_alpha = 0;
	if (frag_texture_pos.y > 1.0 - frag_submerge) {
		submerge_alpha = 0.7 + (frag_texture_pos.y - (1 - frag_submerge))
				/ frag_submerge * 0.3;
	}
	vec4 submerge_mod = vec4(0, 0, 0, submerge_alpha);
	vec4 final_base_color = texture_color + frag_draw_color + frag_color_mod - submerge_mod;
	gl_FragColor = mix(final_base_color, overlay_color, overlay_color.a);
}
