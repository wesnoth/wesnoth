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
	widget(const widget &o);
	widget(display& disp);
	widget(display& disp, SDL_Rect& rect);
	virtual ~widget() { restorer_.cancel(); }

	void bg_restore() const;
	void set_dirty(bool dirty);
	const bool dirty() const;

	display& disp() const { return *disp_; }

private:
	mutable display* disp_;
	mutable surface_restorer restorer_;
	SDL_Rect rect_;
	bool focus_;		// Should user input be ignored?
	bool dirty_;		// Does the widget need drawn?

	void bg_backup();
	
	virtual void handle_event(const SDL_Event& event);
};

}

#endif
