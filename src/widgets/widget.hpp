#ifndef WIDGET_HPP_INCLUDED
#define WIDGET_HPP_INCLUDED

#include "../events.hpp"
#include "../display.hpp"

#include "SDL.h"

namespace gui {

class widget : public events::handler
{
public:
	const SDL_Rect& location() const;
	void set_location(const SDL_Rect& rect);
	void set_position(int x, int y);
	void set_width(int w);
	void set_height(int h);

	const bool focus() const;
	void set_focus(bool focus);

protected:
	widget(display& disp);
	widget(display& disp, const SDL_Rect& rect);

	void bg_restore();
	void update();

private:
	display& disp_;
	surface_restorer restorer_;
	SDL_Rect rect_;
	bool focus_;

	void bg_backup();
	virtual void draw(display &disp) = 0;
	virtual void handle_event(const SDL_Event& event);
};

}

#endif
