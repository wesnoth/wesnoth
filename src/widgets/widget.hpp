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

	//Function to set the widget to draw in 'volatile' mode.
	//When in 'volatile' mode, instead of using the normal
	//save-background-redraw-when-dirty procedure, redrawing is done
	//every frame, and then after every frame the area under the widget
	//is restored to the state it was in before the frame. This is useful
	//for drawing widgets with alpha components in volatile settings where
	//the background may change at any time.
	//(e.g. for putting widgets on top of the game map)
	void set_volatile(bool val=true);

	void set_dirty(bool dirty=true);
	bool dirty() const;

	void set_help_string(const std::string& str);

	virtual void process_help_string(int mousex, int mousey);

protected:
	widget(const widget &o);
	widget(display& disp);
	widget(display& disp, const SDL_Rect& rect);
	virtual ~widget() { restorer_.cancel(); }

	void bg_restore() const;

	display& disp() const { return *disp_; }

	virtual void handle_event(const SDL_Event& event);

private:
	void volatile_draw();
	void volatile_undraw();

	mutable display* disp_;
	mutable surface_restorer restorer_;
	SDL_Rect rect_;
	bool focus_;		// Should user input be ignored?
	bool dirty_;		// Does the widget need drawn?
	mutable bool needs_restore_; //have we drawn ourselves, so that if moved, we need to restore the background?

	bool hidden_;

	bool volatile_;

	std::string help_text_;
	int help_string_;
};

}

#endif
