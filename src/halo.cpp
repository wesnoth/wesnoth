#include "global.hpp"

#include "halo.hpp"
#include "image.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wassert.hpp"
#include "serialization/string_utils.hpp"

#include <algorithm>
#include <map>

namespace halo
{

namespace {
display* disp = NULL;

class effect
{
public:
	effect(int xpos, int ypos, const std::string& img, ORIENTATION orientation, int lifetime);

	void set_location(int x, int y);

	void render();
	void unrender();

	bool expired() const;
private:

	const std::string& current_image();
	void rezoom();

	animated<std::string> images_;

	std::string current_image_;

	bool reverse_;
	
	int origx_, origy_, x_, y_;
	double origzoom_, zoom_;
	surface surf_, buffer_;
	SDL_Rect rect_;
};

std::map<int,effect> haloes;
int halo_id = 1;

bool hide_halo = false;

static const SDL_Rect empty_rect = {0,0,0,0};

effect::effect(int xpos, int ypos, const std::string& img, ORIENTATION orientation, int lifetime)
: images_(img), reverse_(orientation == REVERSE), origx_(xpos), origy_(ypos), x_(xpos), y_(ypos),
  origzoom_(disp->zoom()), zoom_(disp->zoom()), surf_(NULL), buffer_(NULL), rect_(empty_rect)
{
	wassert(disp != NULL);
	// std::cerr << "Constructing halo sequence from image " << img << "\n";

	set_location(xpos,ypos);

	images_.start_animation(0, lifetime);

	if(!images_.animation_finished()) {
		images_.update_current_frame();
		SDL_Delay(20);
	}

	current_image_ = "";
	rezoom();
}

void effect::set_location(int x, int y)
{
	const gamemap::location zero_loc(0,0);
	x_ = origx_ = x - disp->get_location_x(zero_loc);
	y_ = origy_ = y - disp->get_location_y(zero_loc);
	origzoom_ = disp->zoom();

	if(zoom_ != origzoom_) {
		rezoom();
	}
}

const std::string& effect::current_image()
{
	static const std::string r = "";

	const std::string& res = images_.get_current_frame();

	return res;
}

void effect::rezoom()
{
	zoom_ = disp->zoom();
	x_ = int((origx_*zoom_)/origzoom_);
	y_ = int((origy_*zoom_)/origzoom_);

	surf_.assign(image::get_image(current_image_,image::UNSCALED));
	if(surf_ != NULL && reverse_) {
		surf_.assign(image::reverse_image(surf_));
	}

	if(surf_ != NULL && zoom_ != 1.0) {
		surf_.assign(scale_surface(surf_,int(surf_->w*zoom_),int(surf_->h*zoom_)));
	}
}

void effect::render()
{
	if(disp == NULL) {
		return;
	}

	images_.update_current_frame();
	const std::string& img = current_image();
	if(surf_ == NULL || zoom_ != disp->zoom() || current_image_ != img) {
		current_image_ = img;
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

	surface const screen = disp->video().getSurface();

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

	surface const screen = disp->video().getSurface();

	SDL_Rect clip_rect = disp->map_area();
	const clip_rect_setter clip_setter(screen,clip_rect);
	SDL_Rect rect = rect_;
	SDL_BlitSurface(buffer_,NULL,screen,&rect);
	update_rect(rect_);
}

bool effect::expired() const
{
	return images_.animation_finished();
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

halo_hider::halo_hider() : old(hide_halo)
{
	render();
	hide_halo = true;
}

halo_hider::~halo_hider()
{
	hide_halo = old;
	unrender();
}

int add(int x, int y, const std::string& image, ORIENTATION orientation, int lifetime_cycles)
{
	const int id = halo_id++;
	haloes.insert(std::pair<int,effect>(id,effect(x,y,image,orientation,lifetime_cycles)));
	return id;
}

void set_location(int handle, int x, int y)
{
	const std::map<int,effect>::iterator itor = haloes.find(handle);
	if(itor != haloes.end()) {
		itor->second.set_location(x,y);
	}
}

void remove(int handle)
{
	haloes.erase(handle);
}

void render()
{
	if(hide_halo || preferences::show_haloes() == false) {
		return;
	}

	for(std::map<int,effect>::iterator i = haloes.begin(); i != haloes.end(); ) {
		if(i->second.expired()) {
			haloes.erase(i++);
		} else {
			i->second.render();
			++i;
		}
	}
}

void unrender()
{
	if(hide_halo || preferences::show_haloes() == false) {
		return;
	}

	for(std::map<int,effect>::reverse_iterator i = haloes.rbegin(); i != haloes.rend(); ++i) {
		i->second.unrender();
	}
}

}
