#include "widget.hpp"
#include "../display.hpp"

namespace {
	const SDL_Rect EmptyRect = {0,0,0,0};
}

namespace gui {

widget::widget(const widget &o) :
	disp_(o.disp_), rect_(o.rect_), focus_(o.focus_), dirty_(o.dirty_), hidden_(false), volatile_(o.volatile_),
	help_string_(o.help_string_), help_text_(o.help_text_)
{
	bg_backup();
}

widget::widget(display& disp) :
	disp_(&disp), rect_(EmptyRect), focus_(true), dirty_(true), hidden_(false), volatile_(false), help_string_(0)
{
}

widget::widget(display& disp, const SDL_Rect& rect) :
	disp_(&disp), rect_(EmptyRect), focus_(true), dirty_(true), hidden_(false), volatile_(false), help_string_(0)
{
	set_location(rect);
	bg_backup();
}

void widget::set_location(const SDL_Rect& rect)
{
	if(rect_.x == rect.x && rect_.y == rect.y && rect_.w == rect.w && rect_.h == rect.h) {
		return;
	}

	bg_restore();
	rect_ = rect;
	set_dirty(true);
	bg_backup();
}

void widget::set_location(int x, int y)
{
	if(x == rect_.x && y == rect_.y) {
		return;
	}

	bg_restore();
	SDL_Rect rect = {x,y,location().w,location().h};
	rect_ = rect;
	set_dirty(true);
	bg_backup();
}

void widget::set_width(int w)
{
	if(w == rect_.w) {
		return;
	}

	bg_restore();
	SDL_Rect rect = {location().x,location().y,w,location().h};
	rect_ = rect;
	set_dirty(true);
	bg_backup();
}

void widget::set_height(int h)
{
	if(h == rect_.h) {
		return;
	}

	bg_restore();
	SDL_Rect rect = {location().x,location().y,location().w,h};
	rect_ = rect;
	set_dirty(true);
	bg_backup();
}

size_t widget::width() const
{
	return rect_.w;
}

size_t widget::height() const
{
	return rect_.h;
}

const SDL_Rect& widget::location() const
{
	return rect_;
}

void widget::set_focus(bool focus)
{
	if(focus) {
		events::focus_handler(this);
	}

	focus_ = focus;
	set_dirty(true);
}

bool widget::focus() const
{
	return events::has_focus(this) && focus_;
}

void widget::hide(bool value)
{
	if(value != hidden_) {
		if(value) {
			restorer_.restore();
		} else {
			set_dirty(true);
		}

		hidden_ = value;
	}
}

bool widget::hidden() const
{
	return hidden_;
}

void widget::set_dirty(bool dirty)
{
	if(dirty == true && volatile_) {
		return;
	}

	dirty_ = dirty;
}

const bool widget::dirty() const
{
	return dirty_;
}

void widget::bg_backup()
{
	restorer_ = surface_restorer(&disp().video(), rect_);
}

void widget::bg_restore() const
{
	restorer_.restore();
}

void widget::handle_event(const SDL_Event& event)
{
	if(!focus_)
		return;
}

void widget::set_volatile(bool val)
{
	volatile_ = val;
	if(volatile_) {
		dirty_ = false;
	}
}

void widget::volatile_draw()
{
	if(!volatile_) {
		return;
	}

	dirty_ = true;
	bg_backup();
	draw();
}

void widget::volatile_undraw()
{
	if(!volatile_) {
		return;
	}

	bg_restore();
}

void widget::set_help_string(const std::string& str)
{
	help_text_ = str;
}

void widget::process_help_string(int mousex, int mousey)
{
	if(!hidden() && point_in_rect(mousex,mousey,location())) {
		if(help_string_ == 0 && help_text_ != "") {
			std::cerr << "setting help string to '" << help_text_ << "'\n";
			help_string_ = disp().set_help_string(help_text_);
		}
	} else if(help_string_ > 0) {
		disp().clear_help_string(help_string_);
		help_string_ = 0;
	}
}

}
