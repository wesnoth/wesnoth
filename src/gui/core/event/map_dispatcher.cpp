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

	connect_signal<gui2::event::SDL_LEFT_BUTTON_UP>(std::bind(
		&map_dispatcher::mouse_left_up, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5));
	connect_signal<gui2::event::SDL_LEFT_BUTTON_DOWN>(std::bind(
		&map_dispatcher::mouse_left_down, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5));

	connect_signal<gui2::event::SDL_RIGHT_BUTTON_DOWN>(std::bind(
		&map_dispatcher::mouse_right_down, this, std::placeholders::_2, std::placeholders::_3, std::placeholders::_5));
}

void map_dispatcher::mouse_left_up(
	gui2::event::ui_event e,
	bool& handled,
	const point& p)
{
	// PLAIN_LOG << "mouse left up at: " << p;
	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	mhandler.left_mouse_up(p.x, p.y, controller_.is_browsing());
	handled = true;
}

void map_dispatcher::mouse_left_down(
	gui2::event::ui_event e,
	bool& handled,
	const point& p)
{
	// PLAIN_LOG << "mouse left down at: " << p;
	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	mhandler.left_click(p.x, p.y, controller_.is_browsing());
	handled = true;
}

void map_dispatcher::mouse_right_down(
	gui2::event::ui_event e,
	bool& handled,
	const point& p)
{
	// PLAIN_LOG << "mouse right down at: " << p;
	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	auto* menu = display::get_singleton()->get_theme().context_menu();
	hotkey::command_executor* cmd_exec = controller_.get_hotkey_command_executor();
	if(!menu || !cmd_exec) {
		handled = false;
	}

	// context menus cannot appear outside map area,
	// but main top-panel menus can.
	if(menu && !(display::get_singleton()->map_area().contains(p))) {
		handled = false;
	}

	cmd_exec->show_menu(menu->items(), p, menu);
	handled = true;
}

bool map_dispatcher::is_at(const point& /*coordinate*/) const
{
	return true;
}

}

}
