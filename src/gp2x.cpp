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

void * mmu_hack::trymmap (void *start, size_t length, int prot, int flags, int fd, off_t offset)
{
	char *p;
	int aa;

	//printf ("mmap(%X, %X, %X, %X, %X, %X) ... ", (unsigned int)start, length, prot, flags, fd, (unsigned int)offset);
	p = (char *) mmap(start, length, prot, flags, fd, offset);
	if (p == (char *)0xFFFFFFFF)
	{
		aa = errno;
		printf ("failed mmap(%X, %X, %X, %X, %X, %X) errno = %d\n", (unsigned int)start, length, prot, flags, fd, (unsigned int)offset, aa);
	}
	else
	{
		//printf ("OK! (%X)\n", (unsigned int)p);
	}

	return p;
}

unsigned char mmu_hack::initphys()
{
	memfd = open("/dev/mem", O_RDWR);
	if (memfd == -1)
	{
		printf ("Open failed\n");
		return 0;
	}

	printf ("/dev/mem opened successfully - fd = %d\n", memfd);

	return 1;
}

void mmu_hack::closephys()
{
	close(memfd);
}

int mmu_hack::myuname(char *buffer)
{
	asm volatile ("swi #0x90007a");
}

void mmu_hack::DecodeCoarse(unsigned int indx, unsigned int sa)
{
	unsigned int cpt[256];
	unsigned int dom = (sa >> 5) & 15;
	unsigned int temp;
	unsigned int i = 0;
	unsigned int wb = 0;
	
	sa &= 0xfffffc00;
	indx *= 1048576;
	
	//printf ("  > %08X\n", sa);
	//printf ("%d\n", 
	lseek (memfd, sa, SEEK_SET);
	memset (cpt, 0, 256*4);
	temp = read (memfd, cpt, 256*4);
	//printf ("%d\n", temp);
	if (temp != 256*4)
	{
		printf ("  # Bad read\n");
		return;
	}

	//printf ("%08X %08X %08X %08X\n", cpt[0], cpt[4], cpt[8], cpt[12]);
	
	for (i = 0; i < 256; i ++)
	{
		if (cpt[i])
		{
			switch (cpt[i] & 3)
			{
				case 0:
					//printf ("  -- [%08X] Invalid --\n", cpt[i]);
					break;
				case 1:
					printf ("  VA: %08X PA: %08X - %08X A: %d %d %d %d D: %d C: %d B: %d\n", indx,
							cpt[i] & 0xFFFF0000, (cpt[i] & 0xFFFF0000) | 0xFFFF,
							(cpt[i] >> 10) & 3, (cpt[i] >> 8) & 3, (cpt[i] >> 6) & 3, 
							(cpt[i] >> 4) & 3, dom, (cpt[i] >> 3) & 1, (cpt[i] >> 2) & 1); 
					break;
				case 2:
					printf ("  VA: %08X PA: %08X - %08X A: %d %d %d %d D: %d C: %d B: %d\n", indx,
							cpt[i] & 0xfffff000, (cpt[i] & 0xfffff000) | 0xFFF,
							(cpt[i] >> 10) & 3, (cpt[i] >> 8) & 3, (cpt[i] >> 6) & 3, 
							(cpt[i] >> 4) & 3, dom, (cpt[i] >> 3) & 1, (cpt[i] >> 2) & 1); 
					// This is where we look for any virtual addresses that map to physical address 0x03000000 and
					// alter the cachable and bufferable bits...
					if (((cpt[i] & 0xffff0000) == 0x03000000) && ((cpt[i] & 12)==0))
					{
						//printf("c and b bits not set, overwriting\n");
						cpt[i] |= 0xFFC;
						wb = 1;
					}
					break;
				case 3:
					//printf ("  -- [%08X/%d] Unsupported --\n", cpt[i],cpt[i] & 3);
					break;
				default:
					//printf ("  -- [%08X/%d] Unknown --\n", cpt[i], cpt[i] & 3);
					break;
			}
		}
		indx += 4096;
	}
	//printf ("%08X %08X %08X %08X\n", cpt[0], cpt[4], cpt[8], cpt[12]);
	if (wb) 
	{
		//printf("Hacking cpt\n");
		lseek (memfd, sa, SEEK_SET);
		temp = write (memfd, cpt, 256*4);
		printf("%d bytes written, %s\n", temp, temp == 1024 ? "yay!" : "oh fooble :(!");
	}
}

void mmu_hack::dumppgtable(unsigned int ttb)
{
	unsigned int pgtable[4096];
	char *desctypes[] = {"Invalid", "Coarse", "Section", "Fine"}; 

	memset (pgtable, 0, 4096*4);
	lseek (memfd, ttb, SEEK_SET);
	read (memfd, pgtable, 4096*4);

	int i;
	for (i = 0; i < 4096; i ++)
	{
		int temp;
		
		if (pgtable[i])
		{
			printf ("Indx: %d VA: %08X Type: %d [%s] \n", i, i * 1048576, pgtable[i] & 3, desctypes[pgtable[i]&3]);
			switch (pgtable[i]&3)
			{
				case 0:
					//printf (" -- Invalid --\n");
					break;
				case 1:
					DecodeCoarse(i, pgtable[i]);
					break;
				case 2:
					temp = pgtable[i] & 0xFFF00000;
					//printf ("  PA: %08X - %08X A: %d D: %d C: %d B: %d\n", temp, temp | 0xFFFFF, 
					//        (pgtable[i] >> 10) & 3, (pgtable[i] >> 5) & 15, (pgtable[i] >> 3) & 1, 
					//        (pgtable[i] >> 2) & 1);
							
					break;
				case 3:
					printf ("  -- Unsupported! --\n");
					break;
			}
		}
	}
}

void mmu_hack::benchmark(void *memptr)
{
	int starttime = time (NULL);
	int a,b,c,d;
	volatile unsigned int *pp = (unsigned int *) memptr;

	while (starttime == time (NULL));

	printf ("\n\nmemory benchmark of volatile VA: %08X\n\nread test\n", (unsigned int) memptr);
	for (d = 0; d < 3; d ++) 
	{
		starttime = time (NULL);
		b = 0;
		c = 0;
		while (starttime == time (NULL))
		{
			for (a = 0; a < 20000; a ++) 
			{
				b += pp[a];
			}
			c ++;
		}
		printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
	}

	printf ("write test\n");
	for (d = 0; d < 3; d ++) 
	{
		starttime = time (NULL);
		b = 0;
		c = 0;
		while (starttime == time (NULL))
		{
			for (a = 0; a < 20000; a ++) 
			{
				pp[a] = 0x37014206;
			}
			c ++;
		}
		printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
	}

	printf  ("combined test (read, write back)\n");
	for (d = 0; d < 3; d ++) 
	{
		starttime = time (NULL);
		b = 0;
		c = 0;
		while (starttime == time (NULL))
		{
			for (a = 0; a < 20000; a ++) 
			{
				pp[a] += 0x55017601;
			}
			c ++;
		}
		printf ("Count is %d. %dMB/sec\n",  c, (c * 20000)/1024/1024);
	}

	printf ("test complete\n");
}

void mmu_hack::hackpgtable()
{
	unsigned int oldc1, oldc2, oldc3, oldc4;
	unsigned int newc1 = 0xee120f10, newc2 = 0xe12fff1e;
	unsigned int ttb, ttx;
	char name[256];

	// We need to execute a "MRC p15, 0, r0, c2, c0, 0", to get the pointer to the translation table base, but we can't 
	// do this in user mode, so we have to patch the kernel to get it to run it for us in supervisor mode. We do this 
	// at the moment by overwriting the sys_newuname function and then calling it.

	lseek (memfd, 0x6ec00, SEEK_SET); // fixme: We should ask the kernel for this address rather than assuming it... 
	read (memfd, &oldc1, 4);
	read (memfd, &oldc2, 4);
	read (memfd, &oldc3, 4);
	read (memfd, &oldc4, 4);

	printf ("0:%08X %08X\n", oldc1, oldc2);

	lseek (memfd, 0x6ec00, SEEK_SET); 
	write (memfd, &newc1, 4);
	write (memfd, &newc2, 4);    
	
	ttb = myuname(name);
	
	lseek (memfd, 0x6ec00, SEEK_SET); 
	write (memfd, &oldc1, 4);
	write (memfd, &oldc2, 4);    

	printf ("1:%08X\n", ttb);

	//printf ("Restored contents\n");
	
	// We now have the translation table base ! Walk the table looking for our allocation and hack it :)
	dumppgtable(ttb);    

	// Now drain the write buffer and flush the tlb caches. Something else we can't do in user mode...
	unsigned int tlbc1 = 0xe3a00000; // mov    r0, #0
	unsigned int tlbc2 = 0xee070f9a; // mcr    15, 0, r0, cr7, cr10, 4
	unsigned int tlbc3 = 0xee080f17; // mcr    15, 0, r0, cr8, cr7, 0
	unsigned int tlbc4 = 0xe1a0f00e; // mov    pc, lr

	lseek (memfd, 0x6ec00, SEEK_SET); 
	write (memfd, &tlbc1, 4);
	write (memfd, &tlbc2, 4);    
	write (memfd, &tlbc3, 4);    
	write (memfd, &tlbc4, 4);    
	
	ttx = myuname(name);
	
	//printf ("Return from uname: %08X\n", ttx);
	
	lseek (memfd, 0x6ec00, SEEK_SET); 
	write (memfd, &oldc1, 4);
	write (memfd, &oldc2, 4);    
	write (memfd, &oldc3, 4);    
	write (memfd, &oldc4, 4);    

	//printf ("Restored contents\n");

	//printf ("Pagetable after modification!\n");
	//printf ("-------------------------------\n");
	//dumppgtable(ttb);
}

mmu_hack::mmu_hack()
{
	if (!initphys()) {
		return;
	}

	volatile unsigned int *myBuf = (unsigned int *)trymmap((void *)0, 65536, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, 0x03000000);
	volatile unsigned int *secbuf = (unsigned int *)malloc (204800);

	//memset ((void *)myBuf, 0x55, 65536);
	//memset ((void *)secbuf, 0x55, 65536);

	printf("mmaped 0x03000000 buffer @ VA: %08X malloc'd buffer @ VA: %08X\n", (unsigned int)myBuf, secbuf);

	hackpgtable();

	//benchmark ((void*)myBuf);
	//benchmark ((void*)secbuf);
}

mmu_hack::~mmu_hack()
{
	close(memfd);
}

mmu_hack & mmu_hack::instance() 
{
	static mmu_hack obj;
	return obj;	
}

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
