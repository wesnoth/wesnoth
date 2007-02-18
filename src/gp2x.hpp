/* $Id: gp2x.hpp 13758 2006-09-30 19:09:45Z grzywacz $ */
/*
   Copyright (C) 2003-6 by David White <davidnwhite@verizon.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifdef GP2X

#ifndef GP2X_HPP_INCLUDED
#define GP2X_HPP_INCLUDED

#include "SDL.h"

namespace gp2x {

int init_joystick();
void handle_joystick(SDL_JoyButtonEvent *);
void makeup_events();
Uint8 get_joystick_state(int *x, int *y);
void return_to_menu();

/**
 * Singleton encapsulating Squidge's MMU hack for GP2X, which enables
 * caching of the upper 32MB of console's memory, to speed up
 * access. For gp2x only; uid == 0 required! Original code at: 
 * http://www.gp32x.com/board/index.php?showtopic=29451&st=0
 */
class mmu_hack {
public:
	static mmu_hack & instance();

private:
	int memfd;

	void *trymmap(void *start, size_t length, int prot, int flags, int fd, off_t offset);
	unsigned char initphys();
	void closephys();
	int myuname(char *buffer);
	void DecodeCoarse(unsigned int indx, unsigned int sa);
	void dumppgtable(unsigned int ttb);
	void benchmark(void *memptr);
	void hackpgtable();

	mmu_hack();
	mmu_hack(const mmu_hack &);	// undefined
	mmu_hack & operator=(const mmu_hack &);	// undefined
	~mmu_hack();
};

} // namespace gp2x

#endif

#endif

/* vim: set ts=4 sw=4: */
