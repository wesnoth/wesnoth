#ifndef WIDGET_HPP_INCLUDED
#define WIDGET_HPP_INCLUDED

#include "../events.hpp"

#include "../sdl_utils.hpp"

#include "SDL.h"

class display;

namespace gui {

class widget : public events::handler
{
public:
	const SDL_Rect& location() const;
	void set_location(const SDL_Rect& rect);
	void set_location(int x, int y);
	void set_width(int w);
	void set_height(int h);

	size_t width() const;
	size_t height() const;

	virtual bool focus() const;
	void set_focus(bool focus);

	void hide(bool value=true);
	bool hidden() const;

	void bg_backup();

	void set_dirty(bool dirty=true);
	const bool dirty() const;

protected:
	widget(const widget &o);
	widget(display& disp);
	widget(display& disp, SDL_Rect& rect);
	virtual ~widget() { restorer_.cancel(); }

	void bg_restore() const;

	display& disp() const { return *disp_; }

	virtual void handle_event(const SDL_Event& event);

private:
	mutable display* disp_;
	mutable surface_restorer restorer_;
	SDL_Rect rect_;
	bool focus_;		// Should user input be ignored?
	bool dirty_;		// Does the widget need drawn?

	bool hidden_;
};

}

#endif
