#ifndef TOOLTIPS_HPP_INCLUDED
#define TOOLTIPS_HPP_INCLUDED

#include "font.hpp"
#include "SDL.h"
#include "video.hpp"

namespace tooltips {

struct manager
{
	manager(display& disp);
	~manager();
};

void clear_tooltips();
void add_tooltip(const SDL_Rect& rect, const std::string& message);
void process(int mousex, int mousey, bool lbutton);


//a function exactly the same as font::draw_text, but will also register
//a tooltip
SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   font::COLOUR colour,
                   const std::string& text, int x, int y, SDL_Surface* bg=NULL);

}

#endif
