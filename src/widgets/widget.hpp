#ifndef WIDGET_HPP_INCLUDED
#define WIDGET_HPP_INCLUDED

#include "../events.hpp"

#include "SDL.h"

namespace gui {

class widget : public events::handler
{
protected:
	widget();
	widget(const SDL_Rect& rect);

	void set_location(const SDL_Rect& rect);

	const SDL_Rect& location() const;

private:
	SDL_Rect rect_;
};

}

#endif
