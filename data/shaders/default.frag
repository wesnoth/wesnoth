#version 120

varying vec4 color_mod;
varying vec4 color_draw;
varying vec2 pos;
varying float submerge_amount;
uniform sampler2D tex;

void main()
{
/*	float rmod = color_mod.r > 0.5 ? color_mod.r - 1 : color_mod.r;
	float gmod = color_mod.g > 0.5 ? color_mod.g - 1 : color_mod.g;
	float bmod = color_mod.b > 0.5 ? color_mod.b - 1 : color_mod.b;
	float amod = color_mod.a > 0.5 ? color_mod.a - 1 : color_mod.a;
	gl_FragColor = texture2D(tex, pos) + vec4(rmod, gmod, bmod, amod);*/
	gl_FragColor = texture2D(tex, pos) + color_mod + color_draw;
}
