#include "gui/core/event/map_dispatcher.hpp"
#include "gui/core/event/handler.hpp"
#include "hotkey/hotkey_command.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "video.hpp"

namespace gui2
{

namespace event
{

map_dispatcher::map_dispatcher(play_controller& controller)
	: controller_(controller)
{
	// Mouse handling

	// Note: If an hotkey is assigned to the same event as the signals,
	// then the hotkey is executed first. If it returns false, only then the
	// builtin handler is executed.
	connect_signal<SDL_MOUSE_MOTION>(std::bind(
		&map_dispatcher::mouse_motion, this, std::placeholders::_3, std::placeholders::_5));

	connect_signal<SDL_LEFT_BUTTON_UP>(std::bind(
		&map_dispatcher::mouse_left_up, this, std::placeholders::_3, std::placeholders::_5));
	connect_signal<SDL_LEFT_BUTTON_DOWN>(std::bind(
		&map_dispatcher::mouse_left_down, this, std::placeholders::_3, std::placeholders::_5));

	connect_signal<SDL_RIGHT_BUTTON_DOWN>(std::bind(
		&map_dispatcher::mouse_right_down, this, std::placeholders::_3, std::placeholders::_5));

	connect_signal<SDL_MIDDLE_BUTTON_UP>(std::bind(
		&map_dispatcher::mouse_middle_up, this, std::placeholders::_3, std::placeholders::_5));
	connect_signal<SDL_MIDDLE_BUTTON_DOWN>(std::bind(
		&map_dispatcher::mouse_middle_down, this, std::placeholders::_3, std::placeholders::_5));

	connect_signal<SDL_WHEEL_UP>(std::bind(
		&map_dispatcher::mouse_wheel, this, std::placeholders::_3, std::placeholders::_5, std::placeholders::_6));
	connect_signal<SDL_WHEEL_DOWN>(std::bind(
		&map_dispatcher::mouse_wheel, this, std::placeholders::_3, std::placeholders::_5, std::placeholders::_6));

	// Mouse Hotkeys
	register_hotkey(hotkey::HOTKEY_SELECT_AND_ACTION, [this](auto&&...) {
		auto& mhandler = controller_.get_mouse_handler_base();
		bool is_selected = mhandler.get_last_hex().valid();
		if (is_selected) {
			mhandler.select_or_action(controller_.is_browsing());
		}
		return is_selected;
	});
	register_hotkey(hotkey::HOTKEY_DESELECT_HEX, [this](auto&&...) {
		auto& mhandler = controller_.get_mouse_handler_base();
		bool is_selected = mhandler.get_selected_hex().valid();
		if (is_selected) {
			mhandler.deselect_hex();
		}
		return is_selected;
	});

	// Touch hotkey
	register_hotkey(hotkey::HOTKEY_TOUCH_HEX, [this](auto&&...) {
		auto& mhandler = controller_.get_mouse_handler_base();
		map_location loc = mhandler.get_last_hex();
		bool is_selected = loc.valid();
		PLAIN_LOG << "touched (hotkey) at: " << loc;
		if (is_selected) {
			mhandler.touch_action(loc, controller_.is_browsing());
		}
		return is_selected;
	});
}

bool map_dispatcher::execute_hotkey(const hotkey::HOTKEY_COMMAND id, const bool down)
{
	// Local hotkey
	if(down && dispatcher::execute_hotkey(id, down)) {
		PLAIN_LOG << "execute_hotkey called";
		return true;
	}

	// these hotkeys have fallback hardcoded mouse handlers, so return false to let the
	// handler do its work.
	if(id == hotkey::HOTKEY_SELECT_AND_ACTION || id == hotkey::HOTKEY_DESELECT_HEX || id == hotkey::HOTKEY_TOUCH_HEX) return false;

	// If no local hotkey, try controller's hotkey executor
	hotkey::command_executor* cmd_exec = controller_.get_hotkey_command_executor();

	if(!cmd_exec) {
		return false;
	}

	hotkey::ui_command cmd(hotkey::get_hotkey_command(id));
	if(cmd_exec->can_execute_command(cmd)) {
		// we are assuming when the key is not pressed, it's released
		// this suffices for the present usecase, ie., scroll by arrow keys
		return cmd_exec->do_execute_command(cmd, down, !down);
	} else {
		return false;
	}
}

void map_dispatcher::mouse_motion(
	bool& handled,
	const point& p)
{
	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	handled = true;
}

void map_dispatcher::mouse_left_up(
	bool& handled,
	const point& p)
{
	const theme::menu* const m = display::get_singleton()->menu_pressed(p);
	if(m != nullptr) {
		const rect& menu_loc = m->location(video::game_canvas());
		if(show_menu(m, { menu_loc.x + 1, menu_loc.y + menu_loc.h + 1 }, false)) {
			handled = true;
			return;
		}
	}

	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	mhandler.clear_dragging(p.x, p.y, controller_.is_browsing());
	mhandler.left_mouse_up(p.x, p.y, controller_.is_browsing());
	mhandler.clear_drag_from_hex();
	handled = true;
}

void map_dispatcher::mouse_left_down(
	bool& handled,
	const point& p)
{
	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	mhandler.cancel_dragging();
	mhandler.init_dragging_left();
	mhandler.left_click(p.x, p.y, controller_.is_browsing());
	handled = true;
}

void map_dispatcher::mouse_right_down(
	bool& handled,
	const point& p)
{
	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	auto* menu = display::get_singleton()->get_theme().context_menu();

	handled = show_menu(menu, p, true);
}

void map_dispatcher::mouse_middle_up(
	bool& handled,
	const point& p)
{
	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	mhandler.middle_mouse_up(p.x, p.y);
	handled = true;
}

void map_dispatcher::mouse_middle_down(
	bool& handled,
	const point& p)
{
	auto& mhandler = controller_.get_mouse_handler_base();
	map_location loc = display::get_singleton()->hex_clicked_on(p.x, p.y);
	mhandler.mouse_update(controller_.is_browsing(), loc);

	mhandler.middle_mouse_down(p.x, p.y);
	handled = true;
}

void map_dispatcher::mouse_wheel(
	bool& handled,
	const point& /*p*/,
	const point& scroll)
{
	auto& mhandler = controller_.get_mouse_handler_base();
	// SDL and wesnoth vertical scroll directions are opposite, hence -scroll.y.
	mhandler.mouse_wheel(scroll.x, -scroll.y, controller_.is_browsing());

	handled = true;
}

bool map_dispatcher::show_menu(const theme::menu* menu, const point& loc, bool context_menu)
{
	hotkey::command_executor* cmd_exec = controller_.get_hotkey_command_executor();
	if(!menu || !cmd_exec) {
		return false;
	}

	// context menus cannot appear outside map area,
	if(context_menu && !(display::get_singleton()->map_area().contains(loc))) {
		return false;
	}

	// TODO: should be migrated to gui2. command_executor shouldn't have menu expansion as responsibility.
	cmd_exec->show_menu(menu->items(), loc, context_menu);
	return true;
}

bool map_dispatcher::is_at(const point& /*coordinate*/) const
{
	return true;
}

}

}
