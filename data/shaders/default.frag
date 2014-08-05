#version 120

varying vec4 frag_color_mod;
varying vec4 frag_draw_color;
varying vec2 frag_texture_pos;
varying float frag_submerge;
uniform sampler2D tex;

void main()
{

	float submerge_alpha = 0;
	if (frag_texture_pos.y > 1.0 - frag_submerge) {
		submerge_alpha = 0.7 + (frag_texture_pos.y - (1 - frag_submerge))
				/ frag_submerge * 0.3;
	}
	vec4 submerge_mod = vec4(0, 0, 0, submerge_alpha);
	gl_FragColor = texture2D(tex, frag_texture_pos)
					+ frag_draw_color + frag_color_mod - submerge_mod;
}
