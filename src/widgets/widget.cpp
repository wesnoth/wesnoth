#include "widget.hpp"

namespace {
	const SDL_Rect EmptyRect = {0,0,0,0};
}

namespace gui {

widget::widget() : rect_(EmptyRect)
{}

widget::widget(const SDL_Rect& rect) : rect_(rect)
{}

void widget::set_location(const SDL_Rect& rect)
{
	rect_ = rect;
}

const SDL_Rect& widget::location() const
{
	return rect_;
}

}