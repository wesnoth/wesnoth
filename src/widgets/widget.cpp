#include "widget.hpp"
#include "../display.hpp"

namespace {
	const SDL_Rect EmptyRect = {0,0,0,0};
}

namespace gui {

widget::widget(display& disp) :
	disp_(disp), rect_(EmptyRect), focus_(true), dirty_(true)
{
	bg_backup();
}

widget::widget(display& disp, const SDL_Rect& rect) :
	disp_(disp), rect_(rect), focus_(true), dirty_(true)
{
	bg_backup();
}

void widget::set_location(const SDL_Rect& rect)
{
	bg_restore();
	rect_ = rect;
	dirty_ = true;
	bg_backup();
	draw();
}

void widget::set_position(int x, int y)
{
	bg_restore();
	SDL_Rect rect = {x,y,location().w,location().h};
	rect_ = rect;
	dirty_ = true;
	bg_backup();
	draw();
}

void widget::set_width(int w)
{
	bg_restore();
	SDL_Rect rect = {location().x,location().y,w,location().h};
	rect_ = rect;
	dirty_ = true;
	bg_backup();
	draw();
}

void widget::set_height(int h)
{
	bg_restore();
	SDL_Rect rect = {location().x,location().y,location().w,h};
	rect_ = rect;
	dirty_ = true;
	bg_backup();
	draw();
}

const SDL_Rect& widget::location() const
{
	return rect_;
}

void widget::set_focus(bool focus)
{
	focus_ = focus;
	dirty_ = true;
	draw();
}

const bool widget::focus() const
{
	return focus_;
}

void widget::set_dirty(bool dirty)
{
	dirty_ = dirty;
}

const bool widget::dirty() const
{
	return dirty_;
}

void widget::bg_backup()
{
	restorer_ = surface_restorer(&disp_.video(), rect_);
}

void widget::bg_restore() const
{
	restorer_.restore();
}

void widget::handle_event(const SDL_Event& event)
{
	if (!focus_)
		return;

	draw();
}

}
