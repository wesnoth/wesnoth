#ifndef CURSOR_HPP_INCLUDED
#define CURSOR_HPP_INCLUDED

#include "SDL.h"
#include "sdl_utils.hpp"

namespace cursor
{

struct manager
{
	manager();
	~manager();
};

enum CURSOR_TYPE { NORMAL, WAIT, MOVE, ATTACK, HYPERLINK, NUM_CURSORS };

void use_colour(bool value);

void set(CURSOR_TYPE type);

void draw(surface screen);
void undraw(surface screen);

void set_focus(bool focus);

struct setter
{
	setter(CURSOR_TYPE type);
	~setter();

private:
	CURSOR_TYPE old_;
};

}

#endif
