#include "halo.hpp"
#include "image.hpp"
#include "sdl_utils.hpp"
#include "util.hpp"

#include <algorithm>
#include <cassert>
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

	struct frame {
		frame() : time(50) {}

		frame(const std::string& str) : time(50) {
			if(std::find(str.begin(),str.end(),':') != str.end()) {
				const std::vector<std::string>& items = config::split(str,':');
				if(items.size() > 1) {
					file = items.front();
					time = lexical_cast<int>(items.back());
					return;
				}
			}

			file = str;
		}

		int time;
		std::string file;
	};

	std::vector<frame> images_;
	std::string current_image_;

	bool reverse_;
	int start_cycle_, cycle_time_, lifetime_;
	
	int x_, y_;
	double zoom_;
	shared_sdl_surface surf_, buffer_;
	SDL_Rect rect_;
};

std::map<int,effect> haloes;
int halo_id = 1;

bool hide_halo = false;

static const SDL_Rect empty_rect = {0,0,0,0};

effect::effect(int xpos, int ypos, const std::string& img, ORIENTATION orientation, int lifetime)
: reverse_(orientation == REVERSE), start_cycle_(-1), cycle_time_(50), lifetime_(lifetime), x_(xpos), y_(ypos), zoom_(disp->zoom()), surf_(NULL), buffer_(NULL), rect_(empty_rect)
{
	if(std::find(img.begin(),img.end(),',') != img.end()) {
		const std::vector<std::string>& imgs = config::split(img,',');
		images_.resize(imgs.size());
		std::copy(imgs.begin(),imgs.end(),images_.begin());

		cycle_time_ = 0;
		for(std::vector<frame>::const_iterator i = images_.begin(); i != images_.end(); ++i) {
			cycle_time_ += i->time;
		}
	}

	if(images_.empty()) {
		images_.push_back(img);
	}

	assert(disp != NULL);

	set_location(xpos,ypos);

	current_image_ = images_.front().file;
	rezoom();
}

void effect::set_location(int x, int y)
{
	const gamemap::location zero_loc(0,0);
	x_ = x - disp->get_location_x(zero_loc);
	y_ = y - disp->get_location_y(zero_loc);
}

const std::string& effect::current_image()
{
	assert(!images_.empty());
	if(images_.size() == 1 || cycle_time_ <= 0) {
		return images_.front().file;
	} else {
		int current_time = (SDL_GetTicks() - start_cycle_)%cycle_time_;
		for(std::vector<frame>::const_iterator i = images_.begin(); i != images_.end(); ++i) {
			current_time -= i->time;
			if(current_time < 0) {
				return i->file;
			}
		}
	}

	return images_.front().file;
}

void effect::rezoom()
{
	const double new_zoom = disp->zoom();
	x_ = int((x_*new_zoom)/zoom_);
	y_ = int((y_*new_zoom)/zoom_);

	zoom_ = new_zoom;

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

	if(start_cycle_ == -1) {
		start_cycle_ = SDL_GetTicks();
	}

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

bool effect::expired() const
{
	return lifetime_ >= 0 && start_cycle_ >= 0 && SDL_GetTicks() - start_cycle_ > lifetime_*cycle_time_;
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
	hide_halo = true;
}

halo_hider::~halo_hider()
{
	hide_halo = old;
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
	if(hide_halo) {
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
	if(hide_halo) {
		return;
	}

	for(std::map<int,effect>::reverse_iterator i = haloes.rbegin(); i != haloes.rend(); ++i) {
		i->second.unrender();
	}
}

}
