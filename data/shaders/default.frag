#version 120

varying vec4 color_mod;
varying vec4 color_draw;
varying vec2 pos;
varying float submerge_amount;
uniform sampler2D tex;

void main()
{
	gl_FragColor = texture2D(tex, pos) + color_draw + color_mod;
}
