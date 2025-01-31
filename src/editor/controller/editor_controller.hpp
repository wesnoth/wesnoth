/*
	Copyright (C) 2008 - 2024
	by Tomasz Sniatowski <kailoran@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "editor/editor_display.hpp"
#include "editor/editor_main.hpp"
#include "editor/map/context_manager.hpp"
#include "editor/toolkit/editor_toolkit.hpp"

#include "controller_base.hpp"
#include "help/help.hpp"
#include "hotkey/command_executor.hpp"
#include "mouse_handler_base.hpp"
#include "tooltips.hpp"

#include "sound_music_track.hpp"

namespace font {
struct floating_label_context;
}

namespace editor {


enum menu_type {
	MAP,
	LOAD_MRU,
	PALETTE,
	AREA,
	ADDON,
	SIDE,
	TIME,
	LOCAL_TIME,
	SCHEDULE,
	LOCAL_SCHEDULE,
	MUSIC,
	UNIT_FACING
};

/**
 * The editor_controller class contains the mouse and keyboard event handling
 * routines for the editor. It also serves as the main editor class with the
 * general logic.
 */
class editor_controller : public controller_base,
	public hotkey::command_executor_default,
	public events::mouse_handler_base,
	quit_confirmation
{
	public:
		editor_controller(const editor_controller&) = delete;
		editor_controller& operator=(const editor_controller&) = delete;

		/**
		 * The constructor.
		 */
		editor_controller(bool clear_id);

		~editor_controller();

		/** Editor main loop */
		EXIT_STATUS main_loop();

		/** Takes a screenshot **/
		void do_screenshot(const std::string& screenshot_filename = "map_screenshot.png");

		/** Show a quit confirmation dialog and returns true if the user pressed 'yes' */
		bool quit_confirm();

		/** Show Unit Editor dialog */
		void unit_editor_dialog();

		/** Display the settings dialog, used to control e.g. the lighting settings */
		void custom_tods_dialog();

		/** Updates schedule and the map display */
		void update_map_schedule(const std::vector<time_of_day>& schedule);

		/** Save the map, open dialog if not named yet. */
		void save_map() override {context_manager_->save_map();}

		/** command_executor override */
		bool can_execute_command(const hotkey::ui_command& command) const override;

		/** command_executor override */
		hotkey::ACTION_STATE get_action_state(const hotkey::ui_command& command) const override;

		/** command_executor override */
		bool do_execute_command(const hotkey::ui_command& command, bool press = true, bool release = false) override;

		/** controller_base override */
		void show_menu(const std::vector<config>& items_arg, int xloc, int yloc, bool context_menu, display& disp) override;

		void show_help() override;
		void status_table() override;

		/** Show the preferences dialog */
		void preferences() override;

		/** Handle hotkeys to scroll map */
		void scroll_up(bool on) override;
		void scroll_down(bool on) override;
		void scroll_left(bool on) override;
		void scroll_right(bool on) override;

		/** Grid toggle */
		void toggle_grid() override;

		void terrain_description() override;
		void unit_description() override;
		void change_unit_id();
		void rename_unit() override;

		void unit_list() override;

		/** Copy the selection on the current map to the clipboard */
		void copy_selection();

		/** Cut the selection from the current map to the clipboard */
		void cut_selection();

		/** Export the WML-compatible list of selected tiles to the system clipboard */
		void export_selection_coords();

		/** Save the current selection to the active area. */
		void save_area();

		/** Add a new area to the current context, filled with the selection if any. */
		void add_area();

		/* mouse_handler_base overrides */
		void mouse_motion(int x, int y, const bool browse, bool update, map_location new_loc = map_location::null_location()) override;
		void touch_motion(int x, int y, const bool browse, bool update=false, map_location new_loc = map_location::null_location()) override;
		editor_display& gui() override { return *gui_; }
		const editor_display& gui() const override { return *gui_; }
		bool allow_mouse_wheel_scroll(int x, int y) override;
		bool right_click_show_menu(int x, int y, const bool browse) override;
		bool left_click(int x, int y, const bool browse) override;
		void left_drag_end(int x, int y, const bool browse) override;
		void left_mouse_up(int x, int y, const bool browse) override;
		bool right_click(int x, int y, const bool browse) override;
		void right_drag_end(int x, int y, const bool browse) override;
		void right_mouse_up(int x, int y, const bool browse) override;

		virtual hotkey::command_executor * get_hotkey_command_executor() override;

		map_context& get_current_map_context() const
		{
			return context_manager_->get_map_context();
		}

		/** Initialize an addon if the addon id is empty
		 * @return    If the initialization succeeded.
		 * */
		bool initialize_addon();

	protected:
		/* controller_base overrides */
		void process_keyup_event(const SDL_Event& event) override;
		mouse_handler_base& get_mouse_handler_base() override { return *this; }
		editor_display& get_display() override { return *gui_; }

		/** Get the current mouse action */
		const mouse_action& get_mouse_action() const { return toolkit_->get_mouse_action(); }
		/** Get the current mouse action */
		mouse_action& get_mouse_action() { return toolkit_->get_mouse_action(); }

		/**
		 * Perform an action, then delete the action object.
		 * The pointer can be nullptr, in which case nothing will happen.
		 */
		void perform_delete(std::unique_ptr<editor_action> action);

		/**
		 * Peform an action on the current map_context, then refresh the display
		 * and delete the pointer. The pointer can be nullptr, in which case nothing will happen.
		 */
		void perform_refresh_delete(std::unique_ptr<editor_action> action, bool drag_part = false);


		virtual std::vector<std::string> additional_actions_pressed() override;

	private:

		/** init the display object and general set-up */
		void init_gui();

		/** init the available time-of-day settings */
		void init_tods(const game_config_view& game_config);

		/** init background music for the editor */
		void init_music(const game_config_view& game_config);

		/** Reload images */
		void refresh_image_cache();

		/**
		 * Callback function passed to display to be called on queue_rerender.
		 * Redraws toolbar, brush bar and related items.
		 */
		void display_redraw_callback(display&);

		/**
		 * Undos an action in the current map context
		 */
		void undo() override;

		/**
		 * Redos an action in the current map context
		 */
		void redo() override;

		editor::menu_type active_menu_;

		/** Reports object. Must be initialized before the gui_ */
		const std::unique_ptr<reports> reports_;

		/** The display object used and owned by the editor. */
		const std::unique_ptr<editor_display> gui_;

		/** Pre-defined time of day lighting settings for the settings dialog */
		typedef std::map<std::string, std::pair<std::string ,std::vector<time_of_day>>> tods_map;
		tods_map tods_;

		/* managers */
	public:
		const std::unique_ptr<context_manager> context_manager_;

		static std::string current_addon_id_;
	private:
		std::unique_ptr<editor_toolkit> toolkit_;
		tooltips::manager tooltip_manager_;
		std::unique_ptr<font::floating_label_context> floating_label_manager_;

		std::unique_ptr<help::help_manager> help_manager_;

		/** Quit main loop flag */
		bool do_quit_;
		EXIT_STATUS quit_mode_;

		std::vector<sound::music_track> music_tracks_;
};

} //end namespace editor
