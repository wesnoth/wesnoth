#include "widget.hpp"
#include "../display.hpp"

namespace {
	const SDL_Rect EmptyRect = {-1234,-1234,0,0};
}

namespace gui {

widget::widget(const widget &o)
	: events::handler(), disp_(o.disp_), restorer_(o.restorer_), rect_(o.rect_),
	  clip_rect_(o.clip_rect_), focus_(o.focus_), needs_restore_(o.needs_restore_),
	  state_(o.state_), volatile_(o.volatile_),
	  help_text_(o.help_text_), help_string_(o.help_string_)
{
}

widget::widget(display& disp)
	: disp_(&disp), rect_(EmptyRect), focus_(true), needs_restore_(false),
	  state_(UNINIT), volatile_(false), help_string_(0)
{
	clip_rect_.x = 0;
	clip_rect_.y = 0;
	clip_rect_.w = disp.screen_area().w;
	clip_rect_.h = disp.screen_area().h;
}

widget::~widget()
{
	bg_cancel();
}

void widget::bg_cancel()
{
	for(std::vector< surface_restorer >::iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->cancel();
	restorer_.clear();
}

void widget::set_location(SDL_Rect const &rect)
{
	if (rect_.x == rect.x && rect_.y == rect.y && rect_.w == rect.w && rect_.h == rect.h)
		return;
	if (state_ == UNINIT && rect.x != -1234 && rect.y != -1234)
		state_ = DRAWN;
	bg_restore();
	rect_ = rect;
	set_dirty(true);
	bg_cancel();
}

void widget::register_rectangle(SDL_Rect const &rect)
{
	restorer_.push_back(surface_restorer(&disp().video(), rect));
}

void widget::set_location(int x, int y)
{
	SDL_Rect rect = { x, y, location().w, location().h };
	set_location(rect);
}

void widget::set_width(int w)
{
	SDL_Rect rect = { location().x, location().y, w, location().h };
	set_location(rect);
}

void widget::set_height(int h)
{
	SDL_Rect rect = { location().x, location().y, location().w, h };
	set_location(rect);
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
	if (focus)
		events::focus_handler(this);
	focus_ = focus;
	set_dirty(true);
}

bool widget::focus() const
{
	return events::has_focus(this) && focus_;
}

void widget::hide(bool value)
{
	if (value) {
		if (state_ == DIRTY || state_ == DRAWN)
			bg_restore();
		state_ = HIDDEN;
	} else if (state_ == HIDDEN) {
		state_ = DRAWN;
		bg_update();
		set_dirty(true);
	}
}

void widget::set_clip_rect(const SDL_Rect& rect)
{
	clip_rect_ = rect;
	set_dirty(true);
}

bool widget::hidden() const
{
	return state_ == HIDDEN || state_ == UNINIT;
}

void widget::set_dirty(bool dirty)
{
	if (dirty && (volatile_ || state_ != DRAWN) || !dirty && state_ != DIRTY)
		return;

	state_ = dirty ? DIRTY : DRAWN;
	if (!dirty)
		needs_restore_ = true;
}

bool widget::dirty() const
{
	return state_ == DIRTY;
}

void widget::bg_update()
{
	for(std::vector< surface_restorer >::iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->update();
}

void widget::bg_restore() const
{
	clip_rect_setter set_clip_rect(disp().video().getSurface(), clip_rect_);

	if (needs_restore_) {
		for(std::vector< surface_restorer >::const_iterator i = restorer_.begin(),
		    i_end = restorer_.end(); i != i_end; ++i)
			i->restore();
		needs_restore_ = false;
	} else {
		//this function should be able to be relied upon to update the rectangle,
		//so do that even if we don't restore
		update_rect(rect_);
	}
}

void widget::bg_restore(SDL_Rect const &rect) const
{
	clip_rect_setter set_clip_rect(disp().video().getSurface(), clip_rect_);

	for(std::vector< surface_restorer >::const_iterator i = restorer_.begin(),
	    i_end = restorer_.end(); i != i_end; ++i)
		i->restore(rect);
}

void widget::set_volatile(bool val)
{
	volatile_ = val;
	if (volatile_ && state_ == DIRTY)
		state_ = DRAWN;
}

void widget::draw()
{
	if (hidden() || !dirty())
		return;

	bg_restore();

	clip_rect_setter set_clip_rect(disp().video().getSurface(), clip_rect_);
	draw_contents();

	update_rect(rect_);
	set_dirty(false);
}

void widget::volatile_draw()
{
	if (!volatile_ || state_ != DRAWN)
		return;
	state_ = DIRTY;
	bg_update();
	draw();
}

void widget::volatile_undraw()
{
	if (!volatile_)
		return;
	bg_restore();
}

void widget::set_help_string(const std::string& str)
{
	help_text_ = str;
}

void widget::process_help_string(int mousex, int mousey)
{
	if (!hidden() && point_in_rect(mousex, mousey, rect_)) {
		if(help_string_ == 0 && help_text_ != "") {
			//std::cerr << "setting help string to '" << help_text_ << "'\n";
			help_string_ = disp().set_help_string(help_text_);
		}
	} else if(help_string_ > 0) {
		disp().clear_help_string(help_string_);
		help_string_ = 0;
	}
}

}
