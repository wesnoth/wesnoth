#ifndef TOOLTIPS_HPP_INCLUDED
#define TOOLTIPS_HPP_INCLUDED

class display;

#include "SDL.h"

namespace tooltips {

struct manager
{
	manager(display& disp);
	~manager();
};

void clear_tooltips();
void clear_tooltips(const SDL_Rect& rect);
void add_tooltip(const SDL_Rect& rect, const std::string& message);
void process(int mousex, int mousey);


//a function exactly the same as font::draw_text, but will also register
//a tooltip
SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
                   int x, int y);

}

#endif
