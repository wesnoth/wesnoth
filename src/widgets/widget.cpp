#include "widget.hpp"
#include "../display.hpp"

namespace {
	const SDL_Rect EmptyRect = {0,0,0,0};
}

namespace gui {

widget::widget(display& disp) : disp_(disp), rect_(EmptyRect)
{
	bg_backup();
}

widget::widget(display& disp, const SDL_Rect& rect) : disp_(disp), rect_(rect)
{
	bg_backup();
}

void widget::set_location(const SDL_Rect& rect)
{
	bg_restore();
	rect_ = rect;
	bg_backup();
	draw(disp_);
}

void widget::set_position(int x, int y)
{
	bg_restore();
	SDL_Rect rect = {x,y,location().w,location().h};
	rect_ = rect;
	bg_backup();
	draw(disp_);
}

void widget::set_width(int w)
{
	bg_restore();
	SDL_Rect rect = {location().x,location().y,w,location().h};
	rect_ = rect;
	bg_backup();
	draw(disp_);
}

void widget::set_height(int h)
{
	bg_restore();
	SDL_Rect rect = {location().x,location().y,location().w,h};
	rect_ = rect;
	bg_backup();
	draw(disp_);
}

const SDL_Rect& widget::location() const
{
	return rect_;
}

void widget::set_focus(bool focus)
{
	focus_ = focus;
}

const bool widget::focus() const
{
	return focus_;
}

void widget::bg_backup()
{
	restorer_ = surface_restorer(&disp_.video(), rect_);
}

void widget::bg_restore()
{
	restorer_.restore();
}

void widget::update()
{
	draw(disp_);
}

void widget::handle_event(const SDL_Event& event)
{
	if (!focus_)
		return;

	draw(disp_);
}

}
