#include "halo.hpp"
#include "image.hpp"
#include "sdl_utils.hpp"

#include <cassert>
#include <map>

namespace halo
{

namespace {
display* disp = NULL;

class effect
{
public:
	effect(int xpos, int ypos, const std::string& img);

	void render();
	void unrender();
private:

	void rezoom();

	std::string image_;
	int x_, y_;
	double zoom_;
	shared_sdl_surface surf_, buffer_;
	SDL_Rect rect_;
};

std::map<int,effect> haloes;
int halo_id = 1;

static const SDL_Rect empty_rect = {0,0,0,0};

effect::effect(int xpos, int ypos, const std::string& img)
: image_(img), x_(xpos), y_(ypos), zoom_(disp->zoom()), surf_(NULL), buffer_(NULL), rect_(empty_rect)
{
	assert(disp != NULL);

	const gamemap::location zero_loc(0,0);
	x_ -= disp->get_location_x(zero_loc);
	y_ -= disp->get_location_y(zero_loc);

	rezoom();
}

void effect::rezoom()
{
	const double new_zoom = disp->zoom();
	x_ = (x_*new_zoom)/zoom_;
	y_ = (y_*new_zoom)/zoom_;

	zoom_ = new_zoom;

	surf_.assign(image::get_image(image_,image::UNSCALED));
	if(surf_ != NULL && zoom_ != 1.0) {
		surf_.assign(scale_surface(surf_,surf_->w*zoom_,surf_->h*zoom_));
	}
}

void effect::render()
{
	if(disp == NULL || surf_ == NULL) {
		return;
	}

	if(zoom_ != disp->zoom()) {
		rezoom();
	}

	if(surf_ == NULL) {
		return;
	}

	const gamemap::location zero_loc(0,0);
	const int screenx = disp->get_location_x(zero_loc);
	const int screeny = disp->get_location_y(zero_loc);

	const int xpos = x_ + screenx - surf_->w/2;
	const int ypos = y_ + screeny - surf_->h/2;

	SDL_Rect rect = {xpos,ypos,surf_->w,surf_->h};
	rect_ = rect;
	SDL_Rect clip_rect = disp->map_area();
	if(rects_overlap(rect,clip_rect) == false) {
		buffer_.assign(NULL);
		return;
	}

	SDL_Surface* const screen = disp->video().getSurface();

	const clip_rect_setter clip_setter(screen,clip_rect);
	if(buffer_ == NULL || buffer_->w != rect.w || buffer_->h != rect.h) {
		SDL_Rect rect = rect_;
		buffer_.assign(get_surface_portion(screen,rect));
	} else {
		SDL_Rect rect = rect_;
		SDL_BlitSurface(screen,&rect,buffer_,NULL);
	}

	SDL_BlitSurface(surf_,NULL,screen,&rect);

	update_rect(rect_);
}

void effect::unrender()
{
	if(buffer_ == NULL) {
		return;
	}

	SDL_Surface* const screen = disp->video().getSurface();

	SDL_Rect clip_rect = disp->map_area();
	const clip_rect_setter clip_setter(screen,clip_rect);
	SDL_Rect rect = rect_;
	SDL_BlitSurface(buffer_,NULL,screen,&rect);
	update_rect(rect_);
}

}

manager::manager(display& screen) : old(disp)
{
	disp = &screen;
}

manager::~manager()
{
	haloes.clear();
	disp = old;
}

int add(int x, int y, const std::string& image)
{
	const int id = halo_id++;
	haloes.insert(std::pair<int,effect>(id,effect(x,y,image)));
	return id;
}

void remove(int handle)
{
	haloes.erase(handle);
}

void render()
{
	for(std::map<int,effect>::iterator i = haloes.begin(); i != haloes.end(); ++i) {
		i->second.render();
	}
}

void unrender()
{
	for(std::map<int,effect>::reverse_iterator i = haloes.rbegin(); i != haloes.rend(); ++i) {
		i->second.unrender();
	}
}

}