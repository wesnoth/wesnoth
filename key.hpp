/* $Id$ */
/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef KEY_HPP_INCLUDED
#define KEY_HPP_INCLUDED

#include "SDL.h"

class CKey {
     public:
	CKey();

	int operator[](int);
	void SetEnabled(bool enable);
     private:
	Uint8 *key_list;
	bool is_enabled;
};

#define KEY_PAGEUP SDLK_PAGEUP
#define KEY_PAGEDOWN SDLK_PAGEDOWN
#define KEY_BACKSPACE SDLK_BACKSPACE
#define KEY_DELETE SDLK_DELETE
#define KEY_TAB SDLK_TAB
#define KEY_CLEAR SDLK_CLEAR
#define KEY_RETURN SDLK_RETURN
#define KEY_ENTER KEY_RETURN
#define KEY_ESCAPE SDLK_ESCAPE
#define KEY_SPACE SDLK_SPACE
#define KEY_0 SDLK_0
#define KEY_1 SDLK_1
#define KEY_2 SDLK_2
#define KEY_3 SDLK_3
#define KEY_4 SDLK_4
#define KEY_5 SDLK_5
#define KEY_6 SDLK_6
#define KEY_7 SDLK_7
#define KEY_8 SDLK_8
#define KEY_9 SDLK_9
#define KEY_KP0 SDLK_KP0
#define KEY_KP1 SDLK_KP1
#define KEY_KP2 SDLK_KP2
#define KEY_KP3 SDLK_KP3
#define KEY_KP4 SDLK_KP4
#define KEY_KP5 SDLK_KP5
#define KEY_KP6 SDLK_KP6
#define KEY_KP7 SDLK_KP7
#define KEY_KP8 SDLK_KP8
#define KEY_KP9 SDLK_KP9
#define KEY_A SDLK_a
#define KEY_B SDLK_b
#define KEY_C SDLK_c
#define KEY_D SDLK_d
#define KEY_E SDLK_e
#define KEY_F SDLK_f
#define KEY_G SDLK_g
#define KEY_H SDLK_h
#define KEY_I SDLK_i
#define KEY_J SDLK_j
#define KEY_K SDLK_k
#define KEY_L SDLK_l
#define KEY_M SDLK_m
#define KEY_N SDLK_n
#define KEY_O SDLK_o
#define KEY_P SDLK_p
#define KEY_Q SDLK_q
#define KEY_R SDLK_r
#define KEY_S SDLK_s
#define KEY_T SDLK_t
#define KEY_U SDLK_u
#define KEY_V SDLK_v
#define KEY_W SDLK_w
#define KEY_X SDLK_x
#define KEY_Y SDLK_y
#define KEY_Z SDLK_z
#define KEY_RSHIFT SDLK_RSHIFT
#define KEY_LSHIFT SDLK_LSHIFT
#define KEY_RCONTROL SDLK_RCTRL
#define KEY_LCONTROL SDLK_LCTRL
#define KEY_RALT SDLK_RALT
#define KEY_LALT SDLK_LALT
#define KEY_UP SDLK_UP
#define KEY_DOWN SDLK_DOWN
#define KEY_LEFT SDLK_LEFT
#define KEY_RIGHT SDLK_RIGHT

#endif
