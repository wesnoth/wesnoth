#include "events.hpp"
#include "mouse.hpp"
#include "preferences.hpp"

#include "SDL.h"

#include <utility>

namespace events
{

namespace {
	int disallow_resize = 0;
}

resize_lock::resize_lock()
{
	++disallow_resize;
}

resize_lock::~resize_lock()
{
	--disallow_resize;
}

void pump()
{
	SDL_PumpEvents();

	static std::pair<int,int> resize_dimensions(0,0);

	SDL_Event event;
	while(SDL_PollEvent(&event)) {
		switch(event.type) {
			case SDL_VIDEORESIZE: {
				const SDL_ResizeEvent* const resize
				              = reinterpret_cast<SDL_ResizeEvent*>(&event);

				if(resize->w < 1024 || resize->h < 768) {
					resize_dimensions.first = 0;
					resize_dimensions.second = 0;
				} else {
					resize_dimensions.first = resize->w;
					resize_dimensions.second = resize->h;
				}

				break;
			}

			//mouse wheel support
			case SDL_MOUSEBUTTONDOWN: {
				if(event.button.button == 4) {
					gui::scroll_dec();
				} else if(event.button.button == 5) {
					gui::scroll_inc();
				}

				break;
			}

			case SDL_QUIT: {
				throw CVideo::quit();
			}
		}
	}

	if(resize_dimensions.first > 0 && disallow_resize == 0) {
		preferences::set_resolution(resize_dimensions);
		resize_dimensions.first = 0;
		resize_dimensions.second = 0;
	}
}

}
