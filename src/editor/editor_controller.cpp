/* $Id$ */
/*
   Copyright (C) 2008 - 2012 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/map/context_manager.hpp"

#include "asserts.hpp"
#include "editor/action/action.hpp"
#include "editor_controller.hpp"

#include "editor/palette/terrain_palettes.hpp"

#include "editor/action/mouse/mouse_action.hpp"

#include "editor_preferences.hpp"

#include "gui/dialogs/editor_settings.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"

#include "../clipboard.hpp"
#include "../game_preferences.hpp"
#include "../gettext.hpp"
#include "../preferences_display.hpp"
#include "../rng.hpp"
#include "../sound.hpp"

#include "halo.hpp"

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

namespace {
static std::vector<std::string> saved_windows_;
}

namespace editor {


editor_controller::editor_controller(const config &game_config, CVideo& video)
	: controller_base(SDL_GetTicks(), game_config, video)
	, mouse_handler_base()
	, active_menu_(editor::MAP)
	, rng_(NULL)
	, rng_setter_(NULL)
	, units_()
	, gui_(new editor_display(NULL, video, NULL, NULL, get_theme(game_config, "editor"), config()))
	, teams_()
	, tods_()
	, context_manager_(new context_manager(*gui_.get(), game_config_))
	, toolkit_()
	, prefs_disp_manager_(NULL)
	, tooltip_manager_(video)
	, floating_label_manager_(NULL)
	, halo_manager_(NULL)
	, do_quit_(false)
	, quit_mode_(EXIT_ERROR)
{
	init_gui();
	toolkit_.reset(new editor_toolkit(*gui_.get(), key_, game_config_));
	init_tods(game_config);
	init_music(game_config);
	rng_.reset(new rand_rng::rng());
	rng_setter_.reset(new rand_rng::set_random_generator(rng_.get()));
	hotkey::get_hotkey(hotkey::HOTKEY_QUIT_GAME).set_description(_("Quit Editor"));
	context_manager_->get_map_context().set_starting_position_labels(gui());
	cursor::set(cursor::NORMAL);
	image::set_color_adjustment(preferences::editor::tod_r(), preferences::editor::tod_g(), preferences::editor::tod_b());
//  TODO enable if you can say what the purpose of the code is. I think it is old stuff and deserves to be removed.
/*	theme& theme = gui().get_theme();
	const theme::menu* default_tool_menu = NULL;
	BOOST_FOREACH(const theme::menu& m, theme.menus()) {
		if (m.get_id() == "draw_button_editor") {
			default_tool_menu = &m;
			break;
		}
	}*/

	gui().redraw_everything();
    events::raise_draw_event();
/*  TODO enable if you can say what the purpose of the code is.
	if (default_tool_menu != NULL) {
		const SDL_Rect& menu_loc = default_tool_menu->location(get_display().screen_area());
		show_menu(default_tool_menu->items(),menu_loc.x+1,menu_loc.y + menu_loc.h + 1,false);
		return;
	}
*/
}

void editor_controller::init_gui()
{
	//TODO duplicate code, see the map_context_refresher
	resources::game_map = &context_manager_->get_map();
	resources::units = &context_manager_->get_map().get_units();
	resources::tod_manager = &context_manager_->get_map().get_time_manager();
	resources::teams = &context_manager_->get_map().get_teams();

	gui_->change_map(&context_manager_->get_map());
	gui_->change_units(&context_manager_->get_map().get_units());
	gui_->change_teams(&context_manager_->get_map().get_teams());
	gui_->set_grid(preferences::grid());
	prefs_disp_manager_.reset(new preferences::display_manager(&gui()));
	gui_->add_redraw_observer(boost::bind(&editor_controller::display_redraw_callback, this, _1));
	floating_label_manager_.reset(new font::floating_label_context());
	gui().set_draw_coordinates(preferences::editor::draw_hex_coordinates());
	gui().set_draw_terrain_codes(preferences::editor::draw_terrain_codes());
	halo_manager_.reset(new halo::manager(*gui_));
}

void editor_controller::init_tods(const config& game_config)
{
	const config &cfg = game_config.child("editor_times");
	if (!cfg) {
		ERR_ED << "No editor time-of-day defined\n";
		return;
	}
	BOOST_FOREACH(const config &i, cfg.child_range("time")) {
		tods_.push_back(time_of_day(i));
	}
}

void editor_controller::init_music(const config& game_config)
{
	const config &cfg = game_config.child("editor_music");
	if (!cfg) {
		ERR_ED << "No editor music defined\n";
		return;
	}
	BOOST_FOREACH(const config &i, cfg.child_range("music")) {
		sound::play_music_config(i);
	}
	sound::commit_music_changes();
}

editor_controller::~editor_controller()
{
	//TODO is not working like expected
//	delete toolkit_.get();
//	delete context_manager_.get();
}

EXIT_STATUS editor_controller::main_loop()
{
	try {
		while (!do_quit_) {
			play_slice();
		}
	} catch (editor_exception& e) {
		gui2::show_transient_message(gui().video(), _("Fatal error"), e.what());
		return EXIT_ERROR;
	} catch (twml_exception& e) {
		e.show(gui());
	}
	return quit_mode_;
}

void editor_controller::do_screenshot(const std::string& screenshot_filename /* = "map_screenshot.bmp" */)
{
	try {
		gui().screenshot(screenshot_filename,true);
	} catch (twml_exception& e) {
		e.show(gui());
	}
}

void editor_controller::quit_confirm(EXIT_STATUS mode)
{
	std::string modified;
	size_t amount = context_manager_->modified_maps(modified);

	std::string message;
	if (amount == 0) {
		message = _("Do you really want to quit?");
	} else if (amount == 1) {
		message = _("Do you really want to quit? Changes in the map since the last save will be lost.");
	} else {
		message = _("Do you really want to quit? The following maps were modified and all changes since the last save will be lost:");
		message += modified;
	}
	const int res = gui2::show_message(gui().video(), _("Quit"), message, gui2::tmessage::yes_no_buttons);
	if(res != gui2::twindow::CANCEL) {
		do_quit_ = true;
		quit_mode_ = mode;
	}
}

void editor_controller::editor_settings_dialog()
{
	if (tods_.empty()) {
		gui2::show_error_message(gui().video(),
				_("No editor time-of-day found."));
		return;
	}

	image::color_adjustment_resetter adjust_resetter;
	if(!gui2::teditor_settings::execute(&(gui()), tods_, gui().video())) {
		adjust_resetter.reset();
	}
	context_manager_->refresh_all();
}

void editor_controller::editor_settings_dialog_redraw_callback(int r, int g, int b)
{
	SCOPE_ED;
	image::set_color_adjustment(r, g, b);
	gui().redraw_everything();
}

bool editor_controller::can_execute_command(hotkey::HOTKEY_COMMAND command, int index) const
{
	using namespace hotkey; //reduce hotkey:: clutter
	switch (command) {
		case HOTKEY_NULL:
			if (index >= 0) {
				unsigned i = static_cast<unsigned>(index);

				switch (active_menu_) {
					case editor::MAP:
						if (i < context_manager_->open_maps()) {
							return true;
						}
						return false;
					case editor::PALETTE:
					case editor::AREA:
					case editor::SIDE:
						return true;
				}
			}
			return false;
		case HOTKEY_EDITOR_PALETTE_GROUPS:
		case HOTKEY_EDITOR_PALETTE_UPSCROLL:
		case HOTKEY_EDITOR_PALETTE_DOWNSCROLL:
			return true;
		case HOTKEY_ZOOM_IN:
		case HOTKEY_ZOOM_OUT:
		case HOTKEY_ZOOM_DEFAULT:
		case HOTKEY_FULLSCREEN:
		case HOTKEY_SCREENSHOT:
		case HOTKEY_MAP_SCREENSHOT:
		case HOTKEY_TOGGLE_GRID:
		case HOTKEY_MOUSE_SCROLL:
		case HOTKEY_ANIMATE_MAP:
		case HOTKEY_MUTE:
		case HOTKEY_PREFERENCES:
		case HOTKEY_HELP:
		case HOTKEY_QUIT_GAME:
			return true; //general hotkeys we can always do
		case HOTKEY_UNDO:
			return true;
		case HOTKEY_REDO:
			return true;
		case HOTKEY_EDITOR_PARTIAL_UNDO:
			return true;
		case TITLE_SCREEN__RELOAD_WML:
		case HOTKEY_EDITOR_QUIT_TO_DESKTOP:
		case HOTKEY_EDITOR_SETTINGS:
		case HOTKEY_EDITOR_MAP_NEW:
		case HOTKEY_EDITOR_SIDE_NEW:
		case HOTKEY_EDITOR_SIDE_SWITCH:
		case HOTKEY_EDITOR_MAP_LOAD:
		case HOTKEY_EDITOR_MAP_SAVE_AS:
		case HOTKEY_EDITOR_BRUSH_NEXT:
		case HOTKEY_EDITOR_TOOL_NEXT:
		case HOTKEY_EDITOR_PALETTE_ITEM_SWAP:
			return true; //editor hotkeys we can always do
		case HOTKEY_EDITOR_MAP_SAVE:
		case HOTKEY_EDITOR_MAP_SAVE_ALL:
		case HOTKEY_EDITOR_SWITCH_MAP:
		case HOTKEY_EDITOR_CLOSE_MAP:
			return true;
		case HOTKEY_EDITOR_MAP_REVERT:
			return !context_manager_->get_map_context().get_filename().empty();
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
		case HOTKEY_EDITOR_TOOL_LABEL:
			return true;
		case HOTKEY_EDITOR_TOOL_UNIT:
			return !context_manager_->get_map().get_teams().empty();
		case HOTKEY_EDITOR_CUT:
		case HOTKEY_EDITOR_COPY:
		case HOTKEY_EDITOR_EXPORT_SELECTION_COORDS:
		case HOTKEY_EDITOR_SELECTION_FILL:
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			return !context_manager_->get_map().selection().empty();
		case HOTKEY_EDITOR_SELECTION_ROTATE:
		case HOTKEY_EDITOR_SELECTION_FLIP:
		case HOTKEY_EDITOR_SELECTION_GENERATE:
			return false; //not implemented
		case HOTKEY_EDITOR_PASTE:
			return !context_manager_->clipboard_empty();
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW:
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW:
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL:
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL:
			return !context_manager_->clipboard_empty();
		case HOTKEY_EDITOR_SELECT_ALL:
		case HOTKEY_EDITOR_SELECT_INVERSE:
		case HOTKEY_EDITOR_SELECT_NONE:
		case HOTKEY_EDITOR_MAP_RESIZE:
		case HOTKEY_EDITOR_MAP_GENERATE:
		case HOTKEY_EDITOR_MAP_APPLY_MASK:
		case HOTKEY_EDITOR_MAP_CREATE_MASK_TO:
		case HOTKEY_EDITOR_REFRESH:
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_REFRESH_IMAGE_CACHE:
			return true;
		case HOTKEY_EDITOR_MAP_ROTATE:
			return false; //not implemented
		case HOTKEY_EDITOR_DRAW_COORDINATES:
		case HOTKEY_EDITOR_DRAW_TERRAIN_CODES:
			return true;
		default:
			return false;
	}
}

hotkey::ACTION_STATE editor_controller::get_action_state(hotkey::HOTKEY_COMMAND command, int index) const {
	using namespace hotkey;
	switch (command) {
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_LABEL:
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
		case HOTKEY_EDITOR_TOOL_UNIT:
			return toolkit_->is_mouse_action_set(command) ? ACTION_ON : ACTION_OFF;
		case HOTKEY_EDITOR_DRAW_COORDINATES:
			return gui_->get_draw_coordinates() ? ACTION_ON : ACTION_OFF;
		case HOTKEY_EDITOR_DRAW_TERRAIN_CODES:
			return gui_->get_draw_terrain_codes() ? ACTION_ON : ACTION_OFF;
		case HOTKEY_NULL:
			switch (active_menu_) {
				case editor::MAP:
					return index == context_manager_->current_context_index() ? ACTION_ON : ACTION_OFF;
				case editor::PALETTE:
					return ACTION_STATELESS;
				case editor::AREA:
				case editor::SIDE:
					return (size_t)index == gui_->playing_team() ? ACTION_ON : ACTION_OFF;
			}
			return ACTION_ON;
		default:
			return command_executor::get_action_state(command, index);
	}
}

bool editor_controller::execute_command(hotkey::HOTKEY_COMMAND command, int index)
{
	SCOPE_ED;
	using namespace hotkey;
	switch (command) {
		case HOTKEY_NULL:
			switch (active_menu_) {
			case MAP:
				if (index >= 0) {
					unsigned i = static_cast<unsigned>(index);
					if (i < context_manager_->size()) {
						context_manager_->switch_context(index);
						return true;
					}
				}
				return false;
			case PALETTE:
				toolkit_->get_palette_manager()->set_group(index);
				return true;
			case SIDE:
				gui_->set_team(index, true);
				gui_->set_playing_team(index);
				toolkit_->get_palette_manager()->draw(true);
				return true;
			case AREA:
				//TODO
				return true;
			}
			return true;
		case HOTKEY_EDITOR_PALETTE_GROUPS:
			return true;
		case HOTKEY_EDITOR_PALETTE_UPSCROLL:
			toolkit_->get_palette_manager()->scroll_up();
			return true;
		case HOTKEY_EDITOR_PALETTE_DOWNSCROLL:
			toolkit_->get_palette_manager()->scroll_down();
			return true;
		case HOTKEY_QUIT_GAME:
			quit_confirm(EXIT_NORMAL);
			return true;
		case HOTKEY_EDITOR_QUIT_TO_DESKTOP:
			quit_confirm(EXIT_QUIT_TO_DESKTOP);
			return true;
		case TITLE_SCREEN__RELOAD_WML:
			context_manager_->save_all_maps(true);
			do_quit_ = true;
			quit_mode_ = EXIT_RELOAD_DATA;
			return true;
		case HOTKEY_EDITOR_SETTINGS:
			editor_settings_dialog();
			return true;
		case HOTKEY_EDITOR_PALETTE_ITEM_SWAP:
			toolkit_->get_palette_manager()->active_palette().swap();
			toolkit_->set_mouseover_overlay();
			return true;
		case HOTKEY_EDITOR_PARTIAL_UNDO:
			if (dynamic_cast<const editor_action_chain*>(context_manager_->get_map_context().last_undo_action()) != NULL) {
				context_manager_->get_map_context().partial_undo();
				context_manager_->refresh_after_action();
			} else {
				undo();
			}
			return true;
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
		case HOTKEY_EDITOR_TOOL_LABEL:
		case HOTKEY_EDITOR_TOOL_UNIT:
			toolkit_->hotkey_set_mouse_action(command);
			return true;
		case HOTKEY_EDITOR_PASTE: //paste is somewhat different as it might be "one action then revert to previous mode"
			toolkit_->hotkey_set_mouse_action(command);
			return true;
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW:
			context_manager_->get_clipboard().rotate_60_cw();
			toolkit_->update_mouse_action_highlights();
			return true;
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW:
			context_manager_->get_clipboard().rotate_60_ccw();
			toolkit_->update_mouse_action_highlights();
			return true;
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL:
			context_manager_->get_clipboard().flip_horizontal();
			toolkit_->update_mouse_action_highlights();
			return true;
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL:
			context_manager_->get_clipboard().flip_vertical();
			toolkit_->update_mouse_action_highlights();
			return true;
		case HOTKEY_EDITOR_BRUSH_NEXT:
			toolkit_->cycle_brush();
			return true;
		case HOTKEY_EDITOR_COPY:
			copy_selection();
			return true;
		case HOTKEY_EDITOR_CUT:
			cut_selection();
			return true;
		case HOTKEY_EDITOR_EXPORT_SELECTION_COORDS:
			export_selection_coords();
			return true;
		case HOTKEY_EDITOR_SELECT_ALL:
			if (!context_manager_->get_map().everything_selected()) {
				context_manager_->perform_refresh(editor_action_select_all());
				return true;
			} //else intentionally fall through
		case HOTKEY_EDITOR_SELECT_INVERSE:
			context_manager_->perform_refresh(editor_action_select_inverse());
			return true;
		case HOTKEY_EDITOR_SELECT_NONE:
			context_manager_->perform_refresh(editor_action_select_none());
			return true;
		case HOTKEY_EDITOR_SELECTION_FILL:
            context_manager_->fill_selection();
			return true;
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			context_manager_->perform_refresh(editor_action_shuffle_area(
					context_manager_->get_map().selection()));
			return true;
		case HOTKEY_EDITOR_CLOSE_MAP:
			context_manager_->close_current_context();
			return true;
		case HOTKEY_EDITOR_MAP_LOAD:
			context_manager_->load_map_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_REVERT:
			context_manager_->revert_map();
			return true;
		case HOTKEY_EDITOR_MAP_NEW:
			context_manager_->new_map_dialog();
			return true;
		case HOTKEY_EDITOR_SIDE_NEW:
			context_manager_->get_map().new_side();
			return true;
		case HOTKEY_EDITOR_MAP_SAVE:
			save_map();
			return true;
		case HOTKEY_EDITOR_MAP_SAVE_ALL:
			context_manager_->save_all_maps();
			return true;
		case HOTKEY_EDITOR_MAP_SAVE_AS:
			context_manager_->save_map_as_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_GENERATE:
			context_manager_->generate_map_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_APPLY_MASK:
			context_manager_->apply_mask_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_CREATE_MASK_TO:
			context_manager_->create_mask_to_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_RESIZE:
			context_manager_->resize_map_dialog();
			return true;
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
			if (context_manager_->toggle_update_transitions())
				return true;
			// else intentionally fall through
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
			context_manager_->refresh_all();
			return true;
		case HOTKEY_EDITOR_REFRESH:
			context_manager_->reload_map();
			return true;
		case HOTKEY_EDITOR_REFRESH_IMAGE_CACHE:
			refresh_image_cache();
			return true;
		case HOTKEY_EDITOR_DRAW_COORDINATES:
			gui().set_draw_coordinates(!gui().get_draw_coordinates());
			preferences::editor::set_draw_hex_coordinates(gui().get_draw_coordinates());
			gui().invalidate_all();
			return true;
		case HOTKEY_EDITOR_DRAW_TERRAIN_CODES:
			gui().set_draw_terrain_codes(!gui().get_draw_terrain_codes());
			preferences::editor::set_draw_terrain_codes(gui().get_draw_terrain_codes());
			gui().invalidate_all();
			return true;
		default:
			return controller_base::execute_command(command, index);
	}
}

void editor_controller::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu)
{
	if (context_menu) {
		if (!context_manager_->get_map().on_board_with_border(gui().hex_clicked_on(xloc, yloc))) {
			return;
		}
	}

	std::vector<std::string> items = items_arg;
	hotkey::HOTKEY_COMMAND command;
	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		command = hotkey::get_hotkey(*i).get_id();
		if (command == hotkey::HOTKEY_UNDO) {
			if (context_manager_->get_map_context().can_undo()) {
				hotkey::get_hotkey(*i).set_description(_("Undo"));
			} else {
				hotkey::get_hotkey(*i).set_description(_("Can’t Undo"));
			}
		} else if (command == hotkey::HOTKEY_REDO) {
			if (context_manager_->get_map_context().can_redo()) {
				hotkey::get_hotkey(*i).set_description(_("Redo"));
			} else {
				hotkey::get_hotkey(*i).set_description(_("Can’t Redo"));
			}
		} else if (command == hotkey::HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS) {
			context_manager_->set_update_transitions_hotkey(command);
		} else if(!can_execute_command(command)
		|| (context_menu && !in_context_menu(command))) {
			i = items.erase(i);
			continue;
		}
		++i;
	}
	if (!items.empty() && items.front() == "editor-switch-map") {
		active_menu_ = editor::MAP;
		context_manager_->expand_open_maps_menu(items);
		context_menu = true; //FIXME hack to display a one-item menu
	}
	if (!items.empty() && items.front() == "editor-palette-groups") {
		active_menu_ = editor::PALETTE;
		toolkit_->get_palette_manager()->active_palette().expand_palette_groups_menu(items);
		context_menu = true; //FIXME hack to display a one-item menu
	}
	if (!items.empty() && items.front() == "editor-side-switch") {
		active_menu_ = editor::SIDE;
		context_manager_->expand_sides_menu(items);
		context_menu = true; //FIXME hack to display a one-item menu
	}

	command_executor::show_menu(items, xloc, yloc, context_menu, gui());
}

void editor_controller::preferences()
{
	preferences::show_preferences_dialog(*gui_, game_config_);
	gui_->redraw_everything();
}

void editor_controller::toggle_grid()
{
	preferences::set_grid(!preferences::grid());
	gui_->invalidate_all();
}

void editor_controller::copy_selection()
{
	if (!context_manager_->get_map().selection().empty()) {
		context_manager_->get_clipboard() = map_fragment(context_manager_->get_map(), context_manager_->get_map().selection());
		context_manager_->get_clipboard().center_by_mass();
	}
}

void editor_controller::cut_selection()
{
	copy_selection();
	context_manager_->perform_refresh(editor_action_paint_area(context_manager_->get_map().selection(), get_selected_bg_terrain()));
}

void editor_controller::export_selection_coords()
{
	std::stringstream ssx, ssy;
	std::set<map_location>::const_iterator i = context_manager_->get_map().selection().begin();
	if (i != context_manager_->get_map().selection().end()) {
		ssx << "x = " << i->x + 1;
		ssy << "y = " << i->y + 1;
		++i;
		while (i != context_manager_->get_map().selection().end()) {
			ssx << ", " << i->x + 1;
			ssy << ", " << i->y + 1;
			++i;
		}
		ssx << "\n" << ssy.str() << "\n";
		copy_to_clipboard(ssx.str(), false);
	}
}

void editor_controller::perform_delete(editor_action* action)
{
	if (action) {
		boost::scoped_ptr<editor_action> action_auto(action);
		context_manager_->get_map_context().perform_action(*action);
	}
}

void editor_controller::perform_refresh_delete(editor_action* action, bool drag_part /* =false */)
{
	if (action) {
		boost::scoped_ptr<editor_action> action_auto(action);
		context_manager_->perform_refresh(*action, drag_part);
	}
}

void editor_controller::refresh_image_cache()
{
	image::flush_cache();
	context_manager_->refresh_all();
}

void editor_controller::display_redraw_callback(display&)
{
	toolkit_->adjust_size();
	//TODO seems not to be needed and slows down the drawing?
	//gui().invalidate_all();
}

void editor_controller::undo()
{
	context_manager_->get_map_context().undo();
	context_manager_->refresh_after_action();
}

void editor_controller::redo()
{
	context_manager_->get_map_context().redo();
	context_manager_->refresh_after_action();
}

void editor_controller::mouse_motion(int x, int y, const bool /*browse*/,
		bool update, map_location /*new_loc*/)
{
	if (mouse_handler_base::mouse_motion_default(x, y, update)) return;
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (context_manager_->get_map().on_board_with_border(drag_from_hex_) && is_dragging()) {
		editor_action* a = NULL;
		bool partial = false;
		editor_action* last_undo = context_manager_->get_map_context().last_undo_action();
		if (dragging_left_ && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1)) != 0) {
			if (!context_manager_->get_map().on_board_with_border(hex_clicked)) return;
			a = toolkit_->get_mouse_action()->drag_left(*gui_, x, y, partial, last_undo);
		} else if (dragging_right_ && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3)) != 0) {
			if (!context_manager_->get_map().on_board_with_border(hex_clicked)) return;
			a = toolkit_->get_mouse_action()->drag_right(*gui_, x, y, partial, last_undo);
		}
		//Partial means that the mouse action has modified the
		//last undo action and the controller shouldn't add
		//anything to the undo stack (hence a different perform_ call)
		if (a != NULL) {
			boost::scoped_ptr<editor_action> aa(a);
			if (partial) {
				context_manager_->get_map_context().perform_partial_action(*a);
			} else {
				context_manager_->get_map_context().perform_action(*a);
			}
			context_manager_->refresh_after_action(true);
		}
	} else {
		toolkit_->get_mouse_action()->move(*gui_, hex_clicked);
	}
	gui().highlight_hex(hex_clicked);
}

bool editor_controller::allow_mouse_wheel_scroll(int x, int y)
{
	return context_manager_->get_map().on_board_with_border(gui().hex_clicked_on(x,y));
}

bool editor_controller::right_click_show_menu(int /*x*/, int /*y*/, const bool /*browse*/)
{
	return toolkit_->get_mouse_action()->has_context_menu();
}

bool editor_controller::left_click(int x, int y, const bool browse)
{
	toolkit_->clear_mouseover_overlay();
	if (mouse_handler_base::left_click(x, y, browse)) return true;
	LOG_ED << "Left click, after generic handling\n";
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (!context_manager_->get_map().on_board_with_border(hex_clicked)) return true;
	LOG_ED << "Left click action " << hex_clicked.x << " " << hex_clicked.y << "\n";
	editor_action* a = toolkit_->get_mouse_action()->click_left(*gui_, x, y);
	perform_refresh_delete(a, true);
	toolkit_->get_mouse_action()->get_palette().draw(true);
	return false;
}

void editor_controller::left_drag_end(int x, int y, const bool /*browse*/)
{
	editor_action* a = toolkit_->get_mouse_action()->drag_end(*gui_, x, y);
	perform_delete(a);
}

void editor_controller::left_mouse_up(int x, int y, const bool /*browse*/)
{
	editor_action* a = toolkit_->get_mouse_action()->up_left(*gui_, x, y);
	perform_delete(a);
	context_manager_->refresh_after_action();
	toolkit_->set_mouseover_overlay();
}

bool editor_controller::right_click(int x, int y, const bool browse)
{
	toolkit_->clear_mouseover_overlay();
	if (mouse_handler_base::right_click(x, y, browse)) return true;
	LOG_ED << "Right click, after generic handling\n";
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (!context_manager_->get_map().on_board_with_border(hex_clicked)) return true;
	LOG_ED << "Right click action " << hex_clicked.x << " " << hex_clicked.y << "\n";
	editor_action* a = toolkit_->get_mouse_action()->click_right(*gui_, x, y);
	perform_refresh_delete(a, true);
	toolkit_->get_mouse_action()->get_palette().draw(true);
	return false;
}

void editor_controller::right_drag_end(int x, int y, const bool /*browse*/)
{
	editor_action* a = toolkit_->get_mouse_action()->drag_end(*gui_, x, y);
	perform_delete(a);
}

void editor_controller::right_mouse_up(int x, int y, const bool /*browse*/)
{
	editor_action* a = toolkit_->get_mouse_action()->up_right(*gui_, x, y);
	perform_delete(a);
	context_manager_->refresh_after_action();
	toolkit_->set_mouseover_overlay();
}

void editor_controller::process_keyup_event(const SDL_Event& event)
{
	editor_action* a = toolkit_->get_mouse_action()->key_event(gui(), event);
	perform_refresh_delete(a);
	toolkit_->set_mouseover_overlay();
}


} //end namespace editor
