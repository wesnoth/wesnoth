#include "gui/core/event/map_dispatcher.hpp"
#include "log.hpp"
#include "resources.hpp"

namespace gui2
{

namespace event
{

map_dispatcher::map_dispatcher(play_controller& controller)
	: controller_(controller)
{
	connect();
	set_mouse_behavior(gui2::event::dispatcher::mouse_behavior::all);
	connect_signal<gui2::event::SDL_LEFT_BUTTON_DOWN>(std::bind(
		&map_dispatcher::mouse_left_click, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5));
	connect_signal<gui2::event::SDL_RIGHT_BUTTON_DOWN>(std::bind(
		&map_dispatcher::mouse_right_click, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5));
}

void map_dispatcher::mouse_left_click(
	gui2::event::ui_event e,
	bool& handled,
	const point& p)
{
	// PLAIN_LOG << "mouse clicked at: " << p;
	auto& mhandler = controller_.get_mouse_handler_base();
	// show_menu_ = false;
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);
	mhandler.left_click(p.x, p.y, controller_.is_browsing());
	handled = true;
}

void map_dispatcher::mouse_right_click(
	gui2::event::ui_event e,
	bool& handled,
	const point& p)
{
	// TODO incomplete
	// PLAIN_LOG << "mouse right clicked at: " << p;
	controller_.get_mouse_handler_base().right_click(p.x, p.y, controller_.is_browsing());
	handled = true;
}

bool map_dispatcher::is_at(const point& /*coordinate*/) const
{
	return true;
}

}

}
