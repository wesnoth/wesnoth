#include "font.hpp"
#include "show_dialog.hpp"
#include "tooltips.hpp"
#include "sdl_utils.hpp"

#include <vector>

namespace {

bool rectangles_overlap(const SDL_Rect& a, const SDL_Rect& b)
{
	const bool xoverlap = a.x >= b.x && a.x < b.x + b.w ||
	                      b.x >= a.x && b.x < a.x + a.w;

	const bool yoverlap = a.y >= b.y && a.y < b.y + b.h ||
	                      b.y >= a.y && b.y < a.y + a.h;
	
	return xoverlap && yoverlap;
}

CVideo* video_ = NULL;
display* display_ = NULL;

struct tooltip
{
	tooltip(const SDL_Rect& r, const std::string& msg) : rect(r), message(msg)
	{
		gui::text_to_lines(message,60);
	}
	SDL_Rect rect;
	std::string message;
};

static const int font_size = 12;

std::vector<tooltip> tips;

std::string current_message;
SDL_Rect current_rect;
SDL_Surface* current_background = NULL;

SDL_Rect get_text_size(const std::string& msg)
{
	SDL_Rect area = {0,0,10000,10000};
	return font::draw_text(NULL,area,font_size,font::BLACK_COLOUR,msg,0,0);
}

void clear_tooltip()
{
	if(current_background == NULL)
		return;

	SDL_BlitSurface(current_background,NULL,video_->getSurface(),&current_rect);
	SDL_FreeSurface(current_background);
	current_background = NULL;

	update_rect(current_rect);
}

void draw_tooltip()
{
	if(current_background != NULL)
		clear_tooltip();

	SDL_Surface* screen = video_->getSurface();

	current_background = get_surface_portion(screen,current_rect);
	if(current_background == NULL)
		return;

	gui::draw_solid_tinted_rectangle(current_rect.x,current_rect.y,
	                                 current_rect.w,current_rect.h,
	                                 0,0,0,0.6,screen);

/*
	gui::draw_solid_tinted_rectangle(current_rect.x,current_rect.y,
	                                 current_rect.w,current_rect.h,
	                                 180,180,0,1.0,screen);
	gui::draw_rectangle(current_rect.x,current_rect.y,
	                    current_rect.w-1,current_rect.h-1,0,screen);
*/
	SDL_Rect text_area = get_text_size(current_message);
	text_area.x = current_rect.x + current_rect.w/2 - text_area.w/2;
	text_area.y = current_rect.y + current_rect.h/2 - text_area.h/2;

	font::draw_text(display_,text_area,font_size,font::NORMAL_COLOUR,
	                current_message,text_area.x,text_area.y);

	update_rect(current_rect);
}

void show_tooltip(const tooltip& tip)
{
	clear_tooltip();

	const size_t xpadding = 10;
	const size_t ypadding = 10;

	SDL_Rect area = get_text_size(tip.message);
	area.w += xpadding;
	area.h += ypadding;

	//see if there is enough room to fit it above the tip area
	if(tip.rect.y > area.h)
		area.y = tip.rect.y - area.h;
	else if(tip.rect.y+tip.rect.h+area.h+1 < display_->y())
		area.y = tip.rect.y + tip.rect.h;
	else
		return;

	if(area.w >= display_->x())
		return;

	if(area.w/2 >= tip.rect.x + tip.rect.w/2)
		area.x = 1;
	else
		area.x = tip.rect.x + tip.rect.w/2 - area.w/2;

	if(area.x + area.w >= display_->x())
		area.x = display_->x() - area.w - 1;

	current_rect = area;
	current_message = tip.message;
	draw_tooltip();
}

}

namespace tooltips {

manager::manager(display& disp)
{
	display_ = &disp;
	video_ = &disp.video();
}

manager::~manager()
{
	clear_tooltips();
	display_ = NULL;
	video_ = NULL;
}

void clear_tooltips()
{
	clear_tooltip();
	tips.clear();
	current_message = "";
}

void clear_tooltips(const SDL_Rect& rect)
{
	clear_tooltip();
	for(std::vector<tooltip>::iterator i = tips.begin(); i != tips.end(); ) {
		if(rectangles_overlap(i->rect,rect)) {
			i = tips.erase(i);
		} else {
			++i;
		}
	}

	current_message = "";
}

void add_tooltip(const SDL_Rect& rect, const std::string& message)
{
	for(std::vector<tooltip>::iterator i = tips.begin(); i != tips.end(); ++i) {
		if(rectangles_overlap(i->rect,rect)) {
			*i = tooltip(rect,message);
			return;
		}
	}

	tips.push_back(tooltip(rect,message));
}

void process(int mousex, int mousey, bool lbutton)
{
	for(std::vector<tooltip>::const_iterator i = tips.begin(); i != tips.end(); ++i) {
		if(mousex > i->rect.x && mousey > i->rect.y &&
		   mousex < i->rect.x + i->rect.w && mousey < i->rect.y + i->rect.h) {
			show_tooltip(*i);
			return;
		}
	}

	clear_tooltip();
}

SDL_Rect draw_text(display* gui, const SDL_Rect& area, int size,
                   const SDL_Color& colour, const std::string& text,
                   int x, int y, SDL_Surface* bg)
{
	return font::draw_text(gui,area,size,colour,text,x,y,bg,true);
}

}
