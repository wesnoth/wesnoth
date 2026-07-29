#include "gui/gui.hpp"
#include "gui/core/event/map_dispatcher.hpp"
#include "gui/core/event/handler.hpp"
#include "gui/dialogs/achievements_dialog.hpp"
#include "hotkey/hotkey_command.hpp"
#include "play_controller.hpp"
#include "resources.hpp"

namespace gui2
{

namespace event
{

map_dispatcher::map_dispatcher(play_controller& controller)
	: controller_(controller)
{
	// Mouse handling
	set_mouse_behavior(dispatcher::mouse_behavior::all);

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

	//Keyboard Hotkeys
	set_want_keyboard_input(true);
	register_hotkeys();
}

void map_dispatcher::register_hotkeys() {

	// "Menu" menu

	register_hotkey(hotkey::HOTKEY_OBJECTIVES, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->objectives();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_ACHIEVEMENTS, [](auto&& ...) {
		gui2::dialogs::achievements_dialog::display();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_STATUS_TABLE, [this](auto&& ...) {
		PLAIN_LOG << "status table";
		controller_.get_hotkey_command_executor()->status_table();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_STATISTICS, [this](auto&& ...) {
		PLAIN_LOG << "statistics";
		controller_.get_hotkey_command_executor()->show_statistics();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_UNIT_LIST, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->unit_list();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_LOAD_GAME, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->load_game();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_SAVE_GAME, [this](auto&& ...) {
		controller_.save_game();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_SAVE_REPLAY, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->save_replay();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_SAVE_MAP, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->save_map();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_LOAD_GAME, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->show_chat_log();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_PREFERENCES, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->preferences();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_HELP, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->show_help();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_QUIT_TO_DESKTOP, [](auto&& ...) {
		quit_confirmation::quit_to_desktop();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_QUIT_GAME, [](auto&& ...) {
		gui2::switch_theme(prefs::get().gui2_theme());
		quit_confirmation::quit_to_title();
		return true;
	});

	// "Actions" menu
	register_hotkey(hotkey::HOTKEY_WB_TOGGLE, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->whiteboard_toggle();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_CYCLE_UNITS, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->cycle_units();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_SPEAK, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->speak();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_RECRUIT, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->recruit();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_RECALL, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->recall();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_SHOW_ENEMY_MOVES, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->show_enemy_moves(false);
		return true;
	});
	register_hotkey(hotkey::HOTKEY_LABEL_SETTINGS, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->label_settings();
		return true;
	});
	register_hotkey(hotkey::HOTKEY_USER_CMD, [this](auto&& ...) {
		controller_.get_hotkey_command_executor()->user_command();
		return true;
	});

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

	hotkey::command_executor* cmd_exec = controller_.get_hotkey_command_executor();
	if(!menu || !cmd_exec) {
		handled = false;
		return;
	}

	// context menus cannot appear outside map area,
	if(menu && !(display::get_singleton()->map_area().contains(p))) {
		handled = false;
		return;
	}

	// TODO: should be migrated to gui2. command_executor shouldn't have menu expansion as responsibility.
	cmd_exec->show_menu(menu->items(), p, menu);
	handled = true;
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

bool map_dispatcher::is_at(const point& /*coordinate*/) const
{
	return true;
}

}

}
