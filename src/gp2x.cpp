/* $Id: gp2x.cpp 13760 2006-09-30 20:55:50Z grzywacz $ */
/*
   Copyright (C) 2006 by Karol Nowak <grzywacz@sul.uni.lodz.pl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef GP2X

#include <iostream>
#include <unistd.h>	// for chdir() and execl()
#include "SDL.h"

#include "gp2x.hpp"
#include "preferences.hpp"
#include "util.hpp"

/*
 * For the MMU hacking code
 */
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <unistd.h>
#include <stropts.h> 
#include <string.h>
#include <errno.h>

namespace gp2x {

void remove_mmuhack()
{
	system("/sbin/rmmod mmuhack");
}

void mmu_hack()
{
	system("/sbin/insmod mmuhack.o");
	//atexit(remove_mmuhack);

	int mmufd = open("/dev/mmuhack", O_RDWR);
	if(mmufd < 0)
	{
	  std::cerr << "MMU hack failed\n";
	}
	else
	{
	  std::cerr << "MMU hack loaded\n";
	  close(mmufd);
	}
}


namespace {

/*
 * GP2X joystick mapping from gp2x wiki
 */
#define GP2X_BUTTON_UP              (0)
#define GP2X_BUTTON_DOWN            (4)
#define GP2X_BUTTON_LEFT            (2)
#define GP2X_BUTTON_RIGHT           (6)
#define GP2X_BUTTON_UPLEFT          (1)
#define GP2X_BUTTON_UPRIGHT         (7)
#define GP2X_BUTTON_DOWNLEFT        (3)
#define GP2X_BUTTON_DOWNRIGHT       (5)
#define GP2X_BUTTON_CLICK           (18)
#define GP2X_BUTTON_A               (12)
#define GP2X_BUTTON_B               (13)
#define GP2X_BUTTON_X               (14)
#define GP2X_BUTTON_Y               (15)
#define GP2X_BUTTON_L               (10)
#define GP2X_BUTTON_R               (11)
#define GP2X_BUTTON_START           (8)
#define GP2X_BUTTON_SELECT          (9)
#define GP2X_BUTTON_VOLUP           (16)
#define GP2X_BUTTON_VOLDOWN         (17)


enum MouseDirection {
	UP        = 1 << 0,
	DOWN      = 1 << 1,
	LEFT      = 1 << 2,
	RIGHT     = 1 << 3,
	DOWNLEFT  = DOWN | LEFT,
	DOWNRIGHT = DOWN | RIGHT,
	UPLEFT    = UP | LEFT,
	UPRIGHT   = UP | RIGHT
};

/**
 * Current mouse position, valid only with emulated mouse
 */
struct mousepos {
	unsigned int x;
	unsigned int y;
} mouse_position_;


/**
 * Initialisaed joystick
 */
SDL_Joystick *joystick_;

void movemouse(MouseDirection dir)
{
	#define MOTION_SPEED (5)

	if(dir & UP) {
		mouse_position_.y -= MOTION_SPEED;
		mouse_position_.y = maximum<int>(mouse_position_.y, 0);
	}
	if(dir & DOWN) {
		mouse_position_.y += MOTION_SPEED;
		mouse_position_.y = minimum<int>(mouse_position_.y, 240 - 1);	// FIXME
	}
	if(dir & LEFT) {
		mouse_position_.x -= MOTION_SPEED;
		mouse_position_.x = maximum<int>(mouse_position_.x, 0);
	}
	if(dir & RIGHT) {
		mouse_position_.x += MOTION_SPEED;
		mouse_position_.x = minimum<int>(mouse_position_.x, 320 - 1); // FIXME
	}

	/*
	 * Move mouse cursor and generate MOUSEMOTION event
	 */
	SDL_WarpMouse(mouse_position_.x, mouse_position_.y);
}

void handle_joybutton(int button, bool down)
{
	static SDL_KeyboardEvent keyevent;
	static SDL_MouseButtonEvent mouseevent;

	if(button == GP2X_BUTTON_VOLDOWN && down) {
		preferences::set_music_volume( maximum<int>(preferences::music_volume() - 10, 0) );
		preferences::set_sound_volume( maximum<int>(preferences::sound_volume() - 10, 0) );
	}
	else if(button == GP2X_BUTTON_VOLUP && down)  {
		preferences::set_music_volume(minimum<int>(preferences::music_volume() + 10, 100));
		preferences::set_sound_volume(minimum<int>(preferences::sound_volume() + 10, 100));
	}
	else if(button == GP2X_BUTTON_A || button == GP2X_BUTTON_B) {

		if(down) {
			mouseevent.type = SDL_MOUSEBUTTONDOWN;
			mouseevent.state = SDL_PRESSED;
		} else {
			mouseevent.type = SDL_MOUSEBUTTONUP;
			mouseevent.state = SDL_RELEASED;
		}

		mouseevent.button = (button == GP2X_BUTTON_A ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT);
		mouseevent.x = mouse_position_.x;
		mouseevent.y = mouse_position_.y;

		SDL_PushEvent( (SDL_Event *) &mouseevent);

	}
	else {
	
		keyevent.which = 0;

		if(down) {
			keyevent.type = SDL_KEYDOWN;
			keyevent.state = SDL_PRESSED;
		} else {
			keyevent.type = SDL_KEYUP;
			keyevent.state = SDL_RELEASED;
		}

		switch(button) {
			case GP2X_BUTTON_R:
				keyevent.keysym.sym = SDLK_n;	// this is a default hotkey for "next unit"
				keyevent.keysym.unicode = 'n';	// ???
				keyevent.keysym.mod = KMOD_NONE;
				break;

			case GP2X_BUTTON_L:
				keyevent.keysym.sym = SDLK_n;	// this is a default hotkey for "previous unit"
				keyevent.keysym.unicode = 'N';	// ???
				keyevent.keysym.mod = KMOD_LSHIFT;
				break;

			case GP2X_BUTTON_X:
				keyevent.keysym.sym = SDLK_l;	// this is a default hotkey for "go to leader"
				keyevent.keysym.unicode = 'l';	// ???
				keyevent.keysym.mod = KMOD_NONE;
				break;

			case GP2X_BUTTON_Y:
				keyevent.keysym.sym = SDLK_v;	// this is a default hotkey for "show enemy moves"
				keyevent.keysym.unicode = 'v';	// ???
				keyevent.keysym.mod = KMOD_LCTRL;
				break;

			case GP2X_BUTTON_CLICK:
				keyevent.keysym.sym = SDLK_u;	// this is a default hotkey for "undo"
				keyevent.keysym.unicode = 'u';	// ???
				keyevent.keysym.mod = KMOD_NONE;
				break;

			case GP2X_BUTTON_START:
				keyevent.keysym.sym = SDLK_a;	// this is a default hotkey for "accelerated"
				keyevent.keysym.unicode = 'a';	// ???
				keyevent.keysym.mod = KMOD_LCTRL;
				break;

			case GP2X_BUTTON_SELECT:
				keyevent.keysym.sym = SDLK_r;	// this is a default hotkey for "recruit"
				keyevent.keysym.unicode = 'r';	// ???
				keyevent.keysym.mod = KMOD_LCTRL;
				break;

			default:
				return;	// huh?
		}

		SDL_PushEvent((SDL_Event *) &keyevent);
	}
}

} // namespace $$

int init_joystick()
{
	std::cerr << "Initializing joystick...\n";

	if(SDL_InitSubSystem(SDL_INIT_JOYSTICK) >= 0) {
		if((joystick_ = SDL_JoystickOpen(0)) != NULL)
			return 0;
	}

	return -1;
}

void handle_joystick(SDL_JoyButtonEvent *ev)
{
	SDL_JoyButtonEvent e = *ev;
	bool down = (e.type == SDL_JOYBUTTONDOWN ? true : false);

	if(e.button == GP2X_BUTTON_UP && down)
		movemouse(UP);
	else if(e.button == GP2X_BUTTON_DOWN && down)
		movemouse(DOWN);
	else if(e.button == GP2X_BUTTON_LEFT && down)
		movemouse(LEFT);
	else if(e.button == GP2X_BUTTON_RIGHT && down)
		movemouse(RIGHT);
	else if(e.button == GP2X_BUTTON_UPLEFT && down)
		movemouse(UPLEFT);
	else if(e.button == GP2X_BUTTON_DOWNLEFT && down)
		movemouse(DOWNLEFT);
	else if(e.button == GP2X_BUTTON_UPRIGHT && down)
		movemouse(UPRIGHT);
	else if(e.button == GP2X_BUTTON_DOWNRIGHT && down)
		movemouse(DOWNRIGHT);
	else
		handle_joybutton(e.button, down);
}

// We try to emulate SDL_GetMouseState() here
Uint8 get_joystick_state(int *x, int *y)
{
	Uint8 result = 0;

	if(x)
		*x = mouse_position_.x;
	if(y)
		*y = mouse_position_.y;

	if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_A) == SDL_PRESSED)
		result |= SDL_BUTTON_LEFT;
	if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_B) == SDL_PRESSED)
		result |= SDL_BUTTON_RIGHT;

	return result;
}

/**
 * Pushes mouse motion events into the queue when joystick directional
 * buttons are pressed
 */
void makeup_events()
{
	static Uint32 last_time;
	Uint32 time = SDL_GetTicks();

	if(time - last_time >= 50)	{
		last_time = time;

		if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_LEFT) == SDL_PRESSED)
			movemouse(LEFT);
		else if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_RIGHT) == SDL_PRESSED)
			movemouse(RIGHT);
		else if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_UP) == SDL_PRESSED)
			movemouse(UP);
		else if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_DOWN) == SDL_PRESSED)
			movemouse(DOWN);
		else if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_UPLEFT) == SDL_PRESSED)
			movemouse(UPLEFT);
		else if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_UPRIGHT) == SDL_PRESSED)
			movemouse(UPRIGHT);
		else if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_DOWNLEFT) == SDL_PRESSED)
			movemouse(DOWNLEFT);
		else if(SDL_JoystickGetButton(joystick_, GP2X_BUTTON_DOWNRIGHT) == SDL_PRESSED)
			movemouse(DOWNRIGHT);
	}
}

/**
 * Return to gp2x menu
 */
void return_to_menu()
{
	chdir("/usr/gp2x");
	execl("/usr/gp2x/gp2xmenu", "/usr/gp2x/gp2xmenu", 0);
}

} // namespace gp2x

#endif

/* vim: set ts=4 sw=4: */
