/*
   Copyright (C) 2008 - 2017 by Tomasz Sniatowski <kailoran@gmail.com>
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

#include "editor/action/action.hpp"
#include "editor/action/action_unit.hpp"
#include "editor/action/action_select.hpp"
#include "editor/controller/editor_controller.hpp"

#include "editor/palette/terrain_palettes.hpp"
#include "editor/palette/location_palette.hpp"

#include "editor/action/mouse/mouse_action.hpp"

#include "preferences/editor.hpp"

#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/editor/custom_tod.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/preferences_dialog.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/dialogs/unit_list.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

#include "resources.hpp"
#include "reports.hpp"

#include "desktop/clipboard.hpp"
#include "floating_label.hpp"
#include "game_board.hpp"
#include "preferences/game.hpp"
#include "gettext.hpp"
#include "preferences/display.hpp"
#include "sound.hpp"
#include "units/unit.hpp"
#include "units/animation_component.hpp"

#include "quit_confirmation.hpp"

#include "utils/functional.hpp"

namespace {
static std::vector<std::string> saved_windows_;
}

namespace editor {

editor_controller::editor_controller(const config &game_config, CVideo& video)
	: controller_base(game_config)
	, mouse_handler_base()
	, quit_confirmation(std::bind(&editor_controller::quit_confirm, this))
	, active_menu_(editor::MAP)
	, reports_(new reports())
	, gui_(new editor_display(*this, video, *reports_, controller_base::get_theme(game_config, "editor")))
	, tods_()
	, context_manager_(new context_manager(*gui_.get(), game_config_))
	, toolkit_(nullptr)
	, tooltip_manager_(video)
	, floating_label_manager_(nullptr)
	, help_manager_(nullptr)
	, do_quit_(false)
	, quit_mode_(EXIT_ERROR)
	, music_tracks_()
{
	init_gui();
	toolkit_.reset(new editor_toolkit(*gui_.get(), key_, game_config_, *context_manager_.get()));
	help_manager_.reset(new help::help_manager(&game_config));
	context_manager_->switch_context(0, true);
	init_tods(game_config);
	init_music(game_config);
	get_current_map_context().set_starting_position_labels(gui());
	cursor::set(cursor::NORMAL);

	join();

	gui().create_buttons();
}

void editor_controller::init_gui()
{
	gui_->change_display_context(&get_current_map_context());
	preferences::set_preference_display_settings();
	gui_->add_redraw_observer(std::bind(&editor_controller::display_redraw_callback, this, _1));
	floating_label_manager_.reset(new font::floating_label_context());
	gui().set_draw_coordinates(preferences::editor::draw_hex_coordinates());
	gui().set_draw_terrain_codes(preferences::editor::draw_terrain_codes());
	gui().set_draw_num_of_bitmaps(preferences::editor::draw_num_of_bitmaps());
//	halo_manager_.reset(new halo::manager(*gui_));
//	resources::halo = halo_manager_.get();
//	^ These lines no longer necessary, the gui owns its halo manager.
//	TODO: Should the editor map contexts actually own the halo manager and swap them in and out from the gui?
//	Note that if that is what happens it might not actually be a good idea for the gui to own the halo manager, so that it can be swapped out
//	without deleting it.
}

void editor_controller::init_tods(const config& game_config)
{
	for (const config &schedule : game_config.child_range("editor_times")) {

		const std::string& schedule_id = schedule["id"];
		const std::string& schedule_name = schedule["name"];
		if (schedule_id.empty()) {
			ERR_ED << "Missing ID attribute in a TOD Schedule." << std::endl;
			continue;
		}

		tods_map::iterator times = tods_.find(schedule_id);
		if (times == tods_.end()) {
			std::pair<tods_map::iterator, bool> new_times =
					tods_.insert( std::pair<std::string, std::pair<std::string, std::vector<time_of_day> > >
			(schedule_id, std::pair<std::string, std::vector<time_of_day> >(schedule_name, std::vector<time_of_day>())) );
			times = new_times.first;
		} else {
			ERR_ED << "Duplicate TOD Schedule identifiers." << std::endl;
			continue;
		}

		for (const config &time : schedule.child_range("time")) {
			times->second.second.emplace_back(time);
		}

	}

	if (tods_.empty()) {
		ERR_ED << "No editor time-of-day defined" << std::endl;
	}
}

void editor_controller::init_music(const config& game_config)
{
	const std::string tag_name = "editor_music";
	if (!game_config.has_child(tag_name))
		ERR_ED << "No editor music defined" << std::endl;
	else {
		for (const config& editor_music : game_config.child_range(tag_name)) {
			for (const config& music : editor_music.child_range("music")) {
				sound::music_track track(music);
				if (track.file_path().empty())
					WRN_ED << "Music track " << track.id() << " not found." << std::endl;
				else
					music_tracks_.emplace_back(music);
			}
		}
	}
}

editor_controller::~editor_controller()
{
	resources::units = nullptr;
	resources::tod_manager = nullptr;
	resources::filter_con = nullptr;

	resources::classification = nullptr;
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
	} catch (wml_exception& e) {
		e.show(gui().video());
	}
	return quit_mode_;
}

void editor_controller::status_table() {
}

void editor_controller::do_screenshot(const std::string& screenshot_filename /* = "map_screenshot.bmp" */)
{
	try {
		if (!gui().screenshot(screenshot_filename,true)) {
			ERR_ED << "Screenshot creation failed!\n";
		}
	} catch (wml_exception& e) {
		e.show(gui().video());
	}
}

bool editor_controller::quit_confirm()
{
	std::string modified;
	size_t amount = context_manager_->modified_maps(modified);

	std::string message;
	if (amount == 0) {
		message = _("Do you really want to quit?");
	} else if (amount == 1) {
		message = _("Do you really want to quit? Changes to this map since the last save will be lost.");
	} else {
		message = _("Do you really want to quit? The following maps were modified and all changes since the last save will be lost:");
		message += "\n" + modified;
	}
	return quit_confirmation::show_prompt(message);
}

void editor_controller::custom_tods_dialog()
{
	if (tods_.empty()) {
		gui2::show_error_message(gui().video(),
				_("No editor time-of-day found."));
		return;
	}

	tod_manager& manager = *get_current_map_context().get_time_manager();

	if(gui2::dialogs::custom_tod::execute(manager.times(), manager.get_current_time(), gui().video())) {
		// TODO save the new tod here
	}

	gui_->update_tod();

	context_manager_->refresh_all();
}

bool editor_controller::can_execute_command(const hotkey::hotkey_command& cmd, int index) const
{
	using namespace hotkey; //reduce hotkey:: clutter
	switch (cmd.id) {
		case HOTKEY_NULL:
			if (index >= 0) {
				unsigned i = static_cast<unsigned>(index);

				switch (active_menu_) {
					case editor::MAP:
						if (i < context_manager_->open_maps()) {
							return true;
						}
						return false;
					case editor::LOAD_MRU:
					case editor::PALETTE:
					case editor::AREA:
					case editor::SIDE:
					case editor::TIME:
					case editor::SCHEDULE:
					case editor::LOCAL_SCHEDULE:
					case editor::MUSIC:
					case editor::LOCAL_TIME:
					case editor::UNIT_FACING:
						return true;
				}
			}
			return false;
		case HOTKEY_EDITOR_PALETTE_GROUPS:
			return true;
		case HOTKEY_EDITOR_PALETTE_UPSCROLL:
			return toolkit_->get_palette_manager()->can_scroll_up();
		case HOTKEY_EDITOR_PALETTE_DOWNSCROLL:
			return toolkit_->get_palette_manager()->can_scroll_down();
		case HOTKEY_ZOOM_IN:
			return !gui_->zoom_at_max();
		case HOTKEY_ZOOM_OUT:
			return !gui_->zoom_at_min();
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
		case HOTKEY_SCROLL_UP:
		case HOTKEY_SCROLL_DOWN:
		case HOTKEY_SCROLL_LEFT:
		case HOTKEY_SCROLL_RIGHT:
			return true; //general hotkeys we can always do

		case hotkey::HOTKEY_UNIT_LIST:
			return get_current_map_context().get_units().size() != 0;

		case HOTKEY_STATUS_TABLE:
			return !get_current_map_context().get_teams().empty();

		case HOTKEY_TERRAIN_DESCRIPTION:
			return gui().mouseover_hex().valid();

			// unit tool related
		case HOTKEY_DELETE_UNIT:
		case HOTKEY_RENAME_UNIT:
		case HOTKEY_EDITOR_UNIT_CHANGE_ID:
		case HOTKEY_EDITOR_UNIT_TOGGLE_CANRECRUIT:
		case HOTKEY_EDITOR_UNIT_TOGGLE_RENAMEABLE:
		case HOTKEY_EDITOR_UNIT_TOGGLE_LOYAL:
		case HOTKEY_EDITOR_UNIT_RECRUIT_ASSIGN:
		case HOTKEY_EDITOR_UNIT_FACING:
		case HOTKEY_UNIT_DESCRIPTION:
		{
			map_location loc = gui_->mouseover_hex();
			const unit_map& units = get_current_map_context().get_units();
			return (toolkit_->is_mouse_action_set(HOTKEY_EDITOR_TOOL_UNIT) &&
					units.find(loc) != units.end());
		}
		case HOTKEY_UNDO:
			return get_current_map_context().can_undo();
		case HOTKEY_REDO:
			return get_current_map_context().can_redo();
		case HOTKEY_EDITOR_PARTIAL_UNDO:
			return get_current_map_context().can_undo();
		case TITLE_SCREEN__RELOAD_WML:
		case HOTKEY_QUIT_TO_DESKTOP:
		case HOTKEY_EDITOR_CUSTOM_TODS:
		case HOTKEY_EDITOR_MAP_NEW:
		case HOTKEY_EDITOR_SCENARIO_NEW:
		case HOTKEY_EDITOR_MAP_LOAD:
		case HOTKEY_EDITOR_MAP_SAVE_AS:
		case HOTKEY_EDITOR_SCENARIO_SAVE_AS:
			return true;

		case HOTKEY_EDITOR_AREA_ADD:
		case HOTKEY_EDITOR_SIDE_NEW:
			return !get_current_map_context().is_pure_map();

		case HOTKEY_EDITOR_SIDE_EDIT:
		case HOTKEY_EDITOR_SIDE_REMOVE:
			return !get_current_map_context().get_teams().empty();

		// brushes
		case HOTKEY_EDITOR_BRUSH_NEXT:
		case HOTKEY_EDITOR_BRUSH_1:
		case HOTKEY_EDITOR_BRUSH_2:
		case HOTKEY_EDITOR_BRUSH_3:
		case HOTKEY_EDITOR_BRUSH_NW_SE:
		case HOTKEY_EDITOR_BRUSH_SW_NE:
			return get_mouse_action().supports_brushes();

		case HOTKEY_EDITOR_TOOL_NEXT:
			return true;
		case HOTKEY_EDITOR_PALETTE_ITEM_SWAP:
			return toolkit_->get_palette_manager()->active_palette().supports_swap();
		case HOTKEY_EDITOR_MAP_SAVE:
			return get_current_map_context().modified();
		case HOTKEY_EDITOR_MAP_SAVE_ALL:
			{
				std::string dummy;
				return context_manager_->modified_maps(dummy) > 1;
			}
		case HOTKEY_EDITOR_PLAYLIST:
		case HOTKEY_EDITOR_SCHEDULE:
			return !get_current_map_context().is_pure_map();
		case HOTKEY_EDITOR_MAP_SWITCH:
		case HOTKEY_EDITOR_MAP_CLOSE:
			return true;
		case HOTKEY_EDITOR_MAP_REVERT:
			return !get_current_map_context().get_filename().empty()
					&& get_current_map_context().modified();

		// Tools
		// Pure map editing tools this can be used all the time.
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			return true;
		// WWL dependent tools which don't rely on defined sides.
		case HOTKEY_EDITOR_SCENARIO_EDIT:
		case HOTKEY_EDITOR_TOOL_LABEL:
		case HOTKEY_EDITOR_TOOL_ITEM:
			return !get_current_map_context().is_pure_map();
		case HOTKEY_EDITOR_TOOL_UNIT:
		case HOTKEY_EDITOR_TOOL_VILLAGE:
			return !get_current_map_context().get_teams().empty();

		case HOTKEY_EDITOR_AREA_REMOVE:
		case HOTKEY_EDITOR_AREA_RENAME:
		case HOTKEY_EDITOR_LOCAL_TIME:
			return !get_current_map_context().is_pure_map() &&
					!get_current_map_context().get_time_manager()->get_area_ids().empty();

		case HOTKEY_EDITOR_AREA_SAVE:
			return 	!get_current_map_context().is_pure_map() &&
					!get_current_map_context().get_time_manager()->get_area_ids().empty()
					&& !context_manager_->get_map().selection().empty();

		case HOTKEY_EDITOR_SELECTION_EXPORT:
		case HOTKEY_EDITOR_SELECTION_CUT:
		case HOTKEY_EDITOR_SELECTION_COPY:
		case HOTKEY_EDITOR_SELECTION_FILL:
			return !context_manager_->get_map().selection().empty()
					&& !toolkit_->is_mouse_action_set(HOTKEY_EDITOR_CLIPBOARD_PASTE);
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			return (context_manager_->get_map().selection().size() > 1
					&& !toolkit_->is_mouse_action_set(HOTKEY_EDITOR_CLIPBOARD_PASTE));
		case HOTKEY_EDITOR_SELECTION_ROTATE:
		case HOTKEY_EDITOR_SELECTION_FLIP:
		case HOTKEY_EDITOR_SELECTION_GENERATE:
			return false; //not implemented
		case HOTKEY_EDITOR_CLIPBOARD_PASTE:
			return !context_manager_->clipboard_empty();
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW:
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW:
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL:
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL:
			return !context_manager_->clipboard_empty()
					&& toolkit_->is_mouse_action_set(HOTKEY_EDITOR_CLIPBOARD_PASTE);
		case HOTKEY_EDITOR_SELECT_ALL:
		case HOTKEY_EDITOR_SELECT_NONE:
			return !toolkit_->is_mouse_action_set(HOTKEY_EDITOR_CLIPBOARD_PASTE);
		case HOTKEY_EDITOR_SELECT_INVERSE:
			return !get_current_map_context().get_map().selection().empty()
					&& !get_current_map_context().get_map().everything_selected()
					&& !toolkit_->is_mouse_action_set(HOTKEY_EDITOR_CLIPBOARD_PASTE);
		case HOTKEY_EDITOR_MAP_RESIZE:
		case HOTKEY_EDITOR_MAP_GENERATE:
		case HOTKEY_EDITOR_MAP_APPLY_MASK:
		case HOTKEY_EDITOR_MAP_CREATE_MASK_TO:
		case HOTKEY_EDITOR_REFRESH:
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_PARTIAL_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_NO_UPDATE_TRANSITIONS:
		case HOTKEY_EDITOR_REFRESH_IMAGE_CACHE:
		case HOTKEY_MINIMAP_CODING_TERRAIN:
		case HOTKEY_MINIMAP_CODING_UNIT:
		case HOTKEY_MINIMAP_DRAW_UNITS:
		case HOTKEY_MINIMAP_DRAW_TERRAIN:
		case HOTKEY_MINIMAP_DRAW_VILLAGES:
		case HOTKEY_EDITOR_REMOVE_LOCATION:
			return true;
		case HOTKEY_EDITOR_MAP_ROTATE:
			return false; //not implemented
		case HOTKEY_EDITOR_DRAW_COORDINATES:
		case HOTKEY_EDITOR_DRAW_TERRAIN_CODES:
		case HOTKEY_EDITOR_DRAW_NUM_OF_BITMAPS:
			return true;
		default:
			return false;
	}
}

hotkey::ACTION_STATE editor_controller::get_action_state(hotkey::HOTKEY_COMMAND command, int index) const {
	using namespace hotkey;
	switch (command) {

	case HOTKEY_EDITOR_UNIT_TOGGLE_LOYAL:
	{
		unit_map::const_unit_iterator un =
				get_current_map_context().get_units().find(gui_->mouseover_hex());
		return un->loyal() ? ACTION_ON : ACTION_OFF;

	}
	case HOTKEY_EDITOR_UNIT_TOGGLE_CANRECRUIT:
	{
		unit_map::const_unit_iterator un =
				get_current_map_context().get_units().find(gui_->mouseover_hex());
		return un->can_recruit() ? ACTION_ON : ACTION_OFF;
	}
	case HOTKEY_EDITOR_UNIT_TOGGLE_RENAMEABLE:
	{
		unit_map::const_unit_iterator un =
				get_current_map_context().get_units().find(gui_->mouseover_hex());
		return (!un->unrenamable()) ? ACTION_ON : ACTION_OFF;
	}
	//TODO remove hardcoded hotkey names
	case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
		return context_manager_->is_active_transitions_hotkey("editor-auto-update-transitions")
				? ACTION_SELECTED : ACTION_DESELECTED;
	case HOTKEY_EDITOR_PARTIAL_UPDATE_TRANSITIONS:
		return context_manager_->is_active_transitions_hotkey("editor-partial-update-transitions")
				? ACTION_SELECTED : ACTION_DESELECTED;
	case HOTKEY_EDITOR_NO_UPDATE_TRANSITIONS:
		return context_manager_->is_active_transitions_hotkey("editor-no-update-transitions")
				? ACTION_SELECTED : ACTION_DESELECTED;
	case HOTKEY_EDITOR_BRUSH_1:
		return toolkit_->is_active_brush("brush-1") ? ACTION_ON : ACTION_OFF;
	case HOTKEY_EDITOR_BRUSH_2:
		return toolkit_->is_active_brush("brush-2") ? ACTION_ON : ACTION_OFF;
	case HOTKEY_EDITOR_BRUSH_3:
		return toolkit_->is_active_brush("brush-3") ? ACTION_ON : ACTION_OFF;
	case HOTKEY_EDITOR_BRUSH_NW_SE:
		return toolkit_->is_active_brush("brush-nw-se") ? ACTION_ON : ACTION_OFF;
	case HOTKEY_EDITOR_BRUSH_SW_NE:
		return toolkit_->is_active_brush("brush-sw-ne") ? ACTION_ON : ACTION_OFF;

	case HOTKEY_TOGGLE_GRID:
		return preferences::grid() ? ACTION_ON : ACTION_OFF;
	case HOTKEY_EDITOR_SELECT_ALL:
		return get_current_map_context().get_map().everything_selected() ?
				ACTION_SELECTED : ACTION_DESELECTED;
	case HOTKEY_EDITOR_SELECT_NONE:
		return get_current_map_context().get_map().selection().empty() ?
				ACTION_SELECTED : ACTION_DESELECTED;
	case HOTKEY_EDITOR_TOOL_FILL:
	case HOTKEY_EDITOR_TOOL_LABEL:
	case HOTKEY_EDITOR_TOOL_PAINT:
	case HOTKEY_EDITOR_TOOL_SELECT:
	case HOTKEY_EDITOR_CLIPBOARD_PASTE:
	case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
	case HOTKEY_EDITOR_TOOL_UNIT:
	case HOTKEY_EDITOR_TOOL_VILLAGE:
	case HOTKEY_EDITOR_TOOL_ITEM:
		return toolkit_->is_mouse_action_set(command) ? ACTION_ON : ACTION_OFF;
	case HOTKEY_EDITOR_DRAW_COORDINATES:
		return gui_->get_draw_coordinates() ? ACTION_ON : ACTION_OFF;
	case HOTKEY_EDITOR_DRAW_TERRAIN_CODES:
		return gui_->get_draw_terrain_codes() ? ACTION_ON : ACTION_OFF;
	case HOTKEY_EDITOR_DRAW_NUM_OF_BITMAPS:
		return gui_->get_draw_num_of_bitmaps() ? ACTION_ON : ACTION_OFF;

	case HOTKEY_MINIMAP_DRAW_VILLAGES:
		return (preferences::minimap_draw_villages()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case HOTKEY_MINIMAP_CODING_UNIT:
		return (preferences::minimap_movement_coding()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case HOTKEY_MINIMAP_CODING_TERRAIN:
		return (preferences::minimap_terrain_coding()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case HOTKEY_MINIMAP_DRAW_UNITS:
		return (preferences::minimap_draw_units()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case HOTKEY_MINIMAP_DRAW_TERRAIN:
		return (preferences::minimap_draw_terrain()) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;
	case HOTKEY_ZOOM_DEFAULT:
		return (gui_->get_zoom_factor() == 1.0) ? hotkey::ACTION_ON : hotkey::ACTION_OFF;

	case HOTKEY_NULL:
		switch (active_menu_) {
		case editor::MAP:
			return index == context_manager_->current_context_index()
					? ACTION_SELECTED : ACTION_DESELECTED;
		case editor::LOAD_MRU:
			return ACTION_STATELESS;
		case editor::PALETTE:
			return ACTION_STATELESS;
		case editor::AREA:
			return index == get_current_map_context().get_active_area()
					? ACTION_SELECTED : ACTION_DESELECTED;
		case editor::SIDE:
			return static_cast<size_t>(index) == gui_->playing_team()
					? ACTION_SELECTED : ACTION_DESELECTED;
		case editor::TIME:
			return index ==	get_current_map_context().get_time_manager()->get_current_time()
					? ACTION_SELECTED : ACTION_DESELECTED;
		case editor::LOCAL_TIME:
			return index ==	get_current_map_context().get_time_manager()->get_current_area_time(
					get_current_map_context().get_active_area())
					? ACTION_SELECTED : ACTION_DESELECTED;
		case editor::MUSIC:
			return get_current_map_context().is_in_playlist(music_tracks_[index].id())
					? ACTION_ON : ACTION_OFF;
		case editor::SCHEDULE:
			{
				tods_map::const_iterator it = tods_.begin();
				std::advance(it, index);
				const std::vector<time_of_day>& times1 = it->second.second;
				const std::vector<time_of_day>& times2 = get_current_map_context().get_time_manager()->times();
				return (times1 == times2) ? ACTION_SELECTED : ACTION_DESELECTED;
			}
		case editor::LOCAL_SCHEDULE:
			{
				tods_map::const_iterator it = tods_.begin();
				std::advance(it, index);
				const std::vector<time_of_day>& times1 = it->second.second;
				int active_area = get_current_map_context().get_active_area();
				const std::vector<time_of_day>& times2 = get_current_map_context().get_time_manager()->times(active_area);
				return (times1 == times2) ? ACTION_SELECTED : ACTION_DESELECTED;
			}
		case editor::UNIT_FACING:
			{
				unit_map::const_unit_iterator un = get_current_map_context().get_units().find(gui_->mouseover_hex());
				assert(un != get_current_map_context().get_units().end());
				return un->facing() == index ? ACTION_SELECTED : ACTION_DESELECTED;
			}
		}
		return ACTION_ON;
		default:
			return command_executor::get_action_state(command, index);
	}
}

bool editor_controller::execute_command(const hotkey::hotkey_command& cmd, int index, bool press)
{
	hotkey::HOTKEY_COMMAND command = cmd.id;
	SCOPE_ED;
	using namespace hotkey;

	// nothing here handles release; fall through to base implementation
	if (!press) {
		return hotkey::command_executor::execute_command(cmd, index, press);
	}

	switch (command) {
		case HOTKEY_NULL:
			switch (active_menu_) {
			case MAP:
				if (index >= 0) {
					unsigned i = static_cast<unsigned>(index);
					if (i < context_manager_->size()) {
						context_manager_->switch_context(index);
						toolkit_->hotkey_set_mouse_action(HOTKEY_EDITOR_TOOL_PAINT);
						return true;
					}
				}
				return false;
			case LOAD_MRU:
				if (index >= 0) {
					context_manager_->load_mru_item(static_cast<unsigned>(index));
				}
				return true;
			case PALETTE:
				toolkit_->get_palette_manager()->set_group(index);
				return true;
			case SIDE:
				gui_->set_team(index, true);
				gui_->set_playing_team(index);
				toolkit_->get_palette_manager()->draw_contents();
				return true;
			case AREA:
				{
					get_current_map_context().set_active_area(index);
					const std::set<map_location>& area =
							get_current_map_context().get_time_manager()->get_area_by_index(index);
					std::vector<map_location> locs(area.begin(), area.end());
					get_current_map_context().select_area(index);
					gui_->scroll_to_tiles(locs.begin(), locs.end());
					return true;
				}
			case TIME:
				{
					get_current_map_context().set_starting_time(index);
					gui_->update_tod();
					return true;
				}
			case LOCAL_TIME:
				{
					get_current_map_context().set_local_starting_time(index);
					return true;
				}
			case MUSIC:
				{
					//TODO mark the map as changed
					sound::play_music_once(music_tracks_[index].id());
					get_current_map_context().add_to_playlist(music_tracks_[index]);
					std::vector<config> items;
					items.emplace_back(config {"id", "editor-playlist"});
					std::shared_ptr<gui::button> b = gui_->find_menu_button("menu-playlist");
					show_menu(items, b->location().x +1, b->location().y + b->height() +1, false, *gui_);
					return true;
				}
			case SCHEDULE:
				{
					tods_map::iterator iter = tods_.begin();
					std::advance(iter, index);
					get_current_map_context().replace_schedule(iter->second.second);
					// TODO: test again after the assign-schedule menu is fixed. Should work, though.
					gui_->update_tod();
					return true;
				}
			case LOCAL_SCHEDULE:
				{
					tods_map::iterator iter = tods_.begin();
					std::advance(iter, index);
					get_current_map_context().replace_local_schedule(iter->second.second);
					return true;
				}
			case UNIT_FACING:
				{
					unit_map::unit_iterator un = get_current_map_context().get_units().find(gui_->mouseover_hex());
					assert(un != get_current_map_context().get_units().end());
					un->set_facing(map_location::DIRECTION(index));
					un->anim_comp().set_standing();
					return true;
				}
			}
			return true;

			//Zoom
		case HOTKEY_ZOOM_IN:
			gui_->set_zoom(true);
			get_current_map_context().get_labels().recalculate_labels();
			toolkit_->set_mouseover_overlay(*gui_);
			return true;
		case HOTKEY_ZOOM_OUT:
			gui_->set_zoom(false);
			get_current_map_context().get_labels().recalculate_labels();
			toolkit_->set_mouseover_overlay(*gui_);
			return true;
		case HOTKEY_ZOOM_DEFAULT:
			gui_->set_default_zoom();
			get_current_map_context().get_labels().recalculate_labels();
			toolkit_->set_mouseover_overlay(*gui_);
			return true;

			//Palette
		case HOTKEY_EDITOR_PALETTE_GROUPS:
		{
			//TODO this code waits for the gui2 dialog to get ready
//			std::vector< std::pair< std::string, std::string > > blah_items;
//			toolkit_->get_palette_manager()->active_palette().expand_palette_groups_menu(blah_items);
//			int selected = 1; //toolkit_->get_palette_manager()->active_palette().get_selected;
//			gui2::teditor_select_palette_group::execute(selected, blah_items, gui_->video());
		}
			return true;
		case HOTKEY_EDITOR_PALETTE_UPSCROLL:
			toolkit_->get_palette_manager()->scroll_up();
			return true;
		case HOTKEY_EDITOR_PALETTE_DOWNSCROLL:
			toolkit_->get_palette_manager()->scroll_down();
			return true;

		case HOTKEY_QUIT_GAME:
			if(quit_confirmation::quit()) {
				do_quit_ = true;
				quit_mode_ = EXIT_NORMAL;
			}
			return true;
		case HOTKEY_QUIT_TO_DESKTOP:
			quit_confirmation::quit_to_desktop();
			return true;
		case TITLE_SCREEN__RELOAD_WML:
			context_manager_->save_all_maps(true);
			do_quit_ = true;
			quit_mode_ = EXIT_RELOAD_DATA;
			return true;
		case HOTKEY_EDITOR_CUSTOM_TODS:
			custom_tods_dialog();
			return true;
		case HOTKEY_EDITOR_PALETTE_ITEM_SWAP:
			toolkit_->get_palette_manager()->active_palette().swap();
			return true;
		case HOTKEY_EDITOR_PARTIAL_UNDO:
			if (dynamic_cast<const editor_action_chain*>(get_current_map_context().last_undo_action()) != nullptr) {
				get_current_map_context().partial_undo();
				context_manager_->refresh_after_action();
			} else {
				undo();
			}
			return true;

			//Tool Selection
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
		case HOTKEY_EDITOR_TOOL_LABEL:
		case HOTKEY_EDITOR_TOOL_UNIT:
		case HOTKEY_EDITOR_TOOL_VILLAGE:
		case HOTKEY_EDITOR_TOOL_ITEM:
			toolkit_->hotkey_set_mouse_action(command);
			return true;

		case HOTKEY_EDITOR_AREA_ADD:
			add_area();
			return true;

		case HOTKEY_EDITOR_UNIT_CHANGE_ID:
			change_unit_id();
			return true;

		case HOTKEY_EDITOR_UNIT_RECRUIT_ASSIGN:
		{
			map_location loc = gui_->mouseover_hex();
			const unit_map::unit_iterator un = get_current_map_context().get_units().find(loc);
			const std::set<std::string>& recruit_set = toolkit_->get_palette_manager()->unit_palette_->get_selected_bg_items();
			std::vector<std::string> recruits(recruit_set.begin(), recruit_set.end());
			un->set_recruits(recruits);
		}
		return true;
		case HOTKEY_EDITOR_UNIT_TOGGLE_RENAMEABLE:
		{
			map_location loc = gui_->mouseover_hex();
			const unit_map::unit_iterator un = get_current_map_context().get_units().find(loc);
			bool unrenamable = un->unrenamable();
			un->set_unrenamable(!unrenamable);
		}
		return true;
		case HOTKEY_EDITOR_UNIT_TOGGLE_CANRECRUIT:
		{
			map_location loc = gui_->mouseover_hex();
			const unit_map::unit_iterator un = get_current_map_context().get_units().find(loc);
			bool canrecruit = un->can_recruit();
			un->set_can_recruit(!canrecruit);
			un->anim_comp().set_standing();
		}
		return true;
		case HOTKEY_DELETE_UNIT:
		{
			map_location loc = gui_->mouseover_hex();
			perform_delete(new editor_action_unit_delete(loc));
		}
		return true;
		case HOTKEY_EDITOR_CLIPBOARD_PASTE: //paste is somewhat different as it might be "one action then revert to previous mode"
			toolkit_->hotkey_set_mouse_action(command);
			return true;

			//Clipboard
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

			//Brushes
		case HOTKEY_EDITOR_BRUSH_NEXT:
			toolkit_->cycle_brush();
			return true;
		case HOTKEY_EDITOR_BRUSH_1:
			toolkit_->set_brush("brush-1");
			return true;
		case HOTKEY_EDITOR_BRUSH_2:
			toolkit_->set_brush("brush-2");
			return true;
		case HOTKEY_EDITOR_BRUSH_3:
			toolkit_->set_brush("brush-3");
			return true;
		case HOTKEY_EDITOR_BRUSH_NW_SE:
			toolkit_->set_brush("brush-nw-se");
			return true;
		case HOTKEY_EDITOR_BRUSH_SW_NE:
			toolkit_->set_brush("brush-sw-ne");
			return true;

		case HOTKEY_EDITOR_SELECTION_COPY:
			copy_selection();
			return true;
		case HOTKEY_EDITOR_SELECTION_CUT:
			cut_selection();
			return true;
		case HOTKEY_EDITOR_AREA_RENAME:
			context_manager_->rename_area_dialog();
			return true;
		case HOTKEY_EDITOR_AREA_SAVE:
			save_area();
			return true;
		case HOTKEY_EDITOR_SELECTION_EXPORT:
			export_selection_coords();
			return true;
		case HOTKEY_EDITOR_SELECT_ALL:
			if(!context_manager_->get_map().everything_selected()) {
				context_manager_->perform_refresh(editor_action_select_all());
				return true;
			}
			FALLTHROUGH;
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

		case HOTKEY_EDITOR_SCENARIO_EDIT:
			context_manager_->edit_scenario_dialog();
			return true;

		case HOTKEY_EDITOR_AREA_REMOVE:
			get_current_map_context().remove_area(
					get_current_map_context().get_active_area());
			return true;

		// map specific
		case HOTKEY_EDITOR_MAP_CLOSE:
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
		case HOTKEY_EDITOR_SCENARIO_NEW:
			context_manager_->new_scenario_dialog();
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
		case HOTKEY_EDITOR_SCENARIO_SAVE_AS:
			context_manager_->save_scenario_as_dialog();
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

		// Side specific ones
		case HOTKEY_EDITOR_SIDE_NEW:
			get_current_map_context().new_side();
			gui_->init_flags();
			return true;
		case HOTKEY_EDITOR_SIDE_REMOVE:
			gui_->set_team(0, true);
			gui_->set_playing_team(0);
			get_current_map_context().remove_side();
			return true;
		case HOTKEY_EDITOR_SIDE_EDIT:
			context_manager_->edit_side_dialog(gui_->viewing_team());
			return true;

		// Transitions
		case HOTKEY_EDITOR_PARTIAL_UPDATE_TRANSITIONS:
			context_manager_->set_update_transitions_mode(2);
			return true;
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
			context_manager_->set_update_transitions_mode(1);
			return true;
		case HOTKEY_EDITOR_NO_UPDATE_TRANSITIONS:
			context_manager_->set_update_transitions_mode(0);
			return true;
		case HOTKEY_EDITOR_TOGGLE_TRANSITIONS:
			if(context_manager_->toggle_update_transitions()) {
				return true;
			}
			FALLTHROUGH;
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
			context_manager_->refresh_all();
			return true;
		// Refresh
		case HOTKEY_EDITOR_REFRESH:
			context_manager_->reload_map();
			return true;
		case HOTKEY_EDITOR_REFRESH_IMAGE_CACHE:
			refresh_image_cache();
			return true;

		case HOTKEY_EDITOR_DRAW_COORDINATES:
			gui().set_draw_coordinates(!gui().get_draw_coordinates());
			preferences::editor::set_draw_hex_coordinates(gui().get_draw_coordinates());
			return true;
		case HOTKEY_EDITOR_DRAW_TERRAIN_CODES:
			gui().set_draw_terrain_codes(!gui().get_draw_terrain_codes());
			preferences::editor::set_draw_terrain_codes(gui().get_draw_terrain_codes());
			return true;
		case HOTKEY_EDITOR_DRAW_NUM_OF_BITMAPS:
			gui().set_draw_num_of_bitmaps(!gui().get_draw_num_of_bitmaps());
			preferences::editor::set_draw_num_of_bitmaps(gui().get_draw_num_of_bitmaps());
			return true;
		case HOTKEY_EDITOR_REMOVE_LOCATION: {
			location_palette* lp = dynamic_cast<location_palette*>(&toolkit_->get_palette_manager()->active_palette());
			if (lp) {
				perform_delete(new editor_action_starting_position(map_location(), lp->selected_item()));
			}
			return true;
		}
		default:
			return hotkey::command_executor::execute_command(cmd, index, press);
	}
}

void editor_controller::show_help()
{
	help::show_help(gui_->video(), "..editor");
}

void editor_controller::show_menu(const std::vector<config>& items_arg, int xloc, int yloc, bool context_menu, display& disp)
{
	if(context_menu) {
		if(!context_manager_->get_map().on_board_with_border(gui().hex_clicked_on(xloc, yloc))) {
			return;
		}
	}

	std::vector<config> items;
	for(const auto& c : items_arg) {
		const std::string& id = c["id"];
		const hotkey::hotkey_command& command = hotkey::get_hotkey_command(id);

		if((can_execute_command(command) && (!context_menu || in_context_menu(command.id)))
			|| command.id == hotkey::HOTKEY_NULL)
		{
			items.emplace_back(config {"id", id});
		}
	}

	// No point in showing an empty menu.
	if(items.empty()) {
		return;
	}

	// Based on the ID of the first entry, we fill the menu contextually.
	const std::string& first_id = items.front()["id"];

	if(first_id == "EDITOR-LOAD-MRU-PLACEHOLDER") {
		active_menu_ = editor::LOAD_MRU;
		context_manager_->expand_load_mru_menu(items, 0);
	}

	if(first_id == "editor-switch-map") {
		active_menu_ = editor::MAP;
		context_manager_->expand_open_maps_menu(items, 0);
	}

	if(first_id == "editor-palette-groups") {
		active_menu_ = editor::PALETTE;
		toolkit_->get_palette_manager()->active_palette().expand_palette_groups_menu(items, 0);
	}

	if(first_id == "editor-switch-side") {
		active_menu_ = editor::SIDE;
		context_manager_->expand_sides_menu(items, 0);
	}

	if(first_id == "editor-switch-area") {
		active_menu_ = editor::AREA;
		context_manager_->expand_areas_menu(items, 0);
	}

	if(!items.empty() && items.front()["id"] == "editor-switch-time") {
		active_menu_ = editor::TIME;
		context_manager_->expand_time_menu(items, 0);
	}

	if(first_id == "editor-assign-local-time") {
		active_menu_ = editor::LOCAL_TIME;
		context_manager_->expand_local_time_menu(items, 0);
	}

	if(first_id == "menu-unit-facings") {
		active_menu_ = editor::UNIT_FACING;
		auto pos = items.erase(items.begin());
		int dir = 0;
		std::generate_n(std::inserter<std::vector<config>>(items, pos), int(map_location::NDIRECTIONS), [&dir]() -> config {
			return config {"label", map_location::write_translated_direction(map_location::DIRECTION(dir++))};
		});
	}

	if(first_id == "editor-playlist") {
		active_menu_ = editor::MUSIC;
		auto pos = items.erase(items.begin());
		std::transform(music_tracks_.begin(), music_tracks_.end(), std::inserter<std::vector<config>>(items, pos), [](const sound::music_track& track) -> config {
			return config {"label", track.title().empty() ? track.id() : track.title()};
		});
	}

	if(first_id == "editor-assign-schedule") {
		active_menu_ = editor::SCHEDULE;
		auto pos = items.erase(items.begin());
		std::transform(tods_.begin(), tods_.end(), std::inserter<std::vector<config>>(items, pos), [](const tods_map::value_type& tod) -> config {
			return config {"label", tod.second.first};
		});
	}

	if(first_id == "editor-assign-local-schedule") {
		active_menu_ = editor::LOCAL_SCHEDULE;
		auto pos = items.erase(items.begin());
		std::transform(tods_.begin(), tods_.end(), std::inserter<std::vector<config>>(items, pos), [](const tods_map::value_type& tod) -> config {
			return config {"label", tod.second.first};
		});
	}

	command_executor::show_menu(items, xloc, yloc, context_menu, disp);
}

void editor_controller::preferences()
{
	gui_->video().clear_all_help_strings();
	gui2::dialogs::preferences_dialog::display(gui_->video(), game_config_);
}

void editor_controller::toggle_grid()
{
	preferences::set_grid(!preferences::grid());
}

void editor_controller::unit_description()
{
	map_location loc = gui_->mouseover_hex();
	const unit_map & units = get_current_map_context().get_units();
	const unit_map::const_unit_iterator un = units.find(loc);
	if(un != units.end()) {
		help::show_unit_help(gui_->video(), un->type_id(), un->type().show_variations_in_help(), false);
	} else {
		help::show_help(gui_->video(), "..units");
	}
}


void editor_controller::copy_selection()
{
	if (!context_manager_->get_map().selection().empty()) {
		context_manager_->get_clipboard() = map_fragment(context_manager_->get_map(), context_manager_->get_map().selection());
		context_manager_->get_clipboard().center_by_mass();
	}
}

void editor_controller::change_unit_id()
{
	map_location loc = gui_->mouseover_hex();
	unit_map& units = get_current_map_context().get_units();
	const unit_map::unit_iterator& un = units.find(loc);

	const std::string title(N_("Change Unit ID"));
	const std::string label(N_("ID:"));

	if(un != units.end()) {
		std::string id = un->id();
		if (gui2::dialogs::edit_text::execute(title, label, id, gui_->video())) {
			un->set_id(id);
		}
	}
}

void editor_controller::rename_unit()
{
	map_location loc = gui_->mouseover_hex();
	unit_map& units = get_current_map_context().get_units();
	const unit_map::unit_iterator& un = units.find(loc);

	const std::string title(N_("Rename Unit"));
	const std::string label(N_("Name:"));

	if(un != units.end()) {
		std::string name = un->name();
		if(gui2::dialogs::edit_text::execute(title, label, name, gui_->video())) {
			//TODO we may not want a translated name here.
			un->set_name(name);
		}
	}
}

void editor_controller::unit_list()
{
	gui2::dialogs::show_unit_list(*gui_);
}

void editor_controller::cut_selection()
{
	copy_selection();
	context_manager_->perform_refresh(editor_action_paint_area(context_manager_->get_map().selection(), get_selected_bg_terrain()));
}

void editor_controller::save_area()
{
	const std::set<map_location>& area = context_manager_->get_map().selection();
	get_current_map_context().save_area(area);
}

void editor_controller::add_area()
{
	const std::set<map_location>& area = context_manager_->get_map().selection();
	get_current_map_context().new_area(area);
}

void editor_controller::export_selection_coords()
{
	std::stringstream ssx, ssy;
	std::set<map_location>::const_iterator i = context_manager_->get_map().selection().begin();
	if (i != context_manager_->get_map().selection().end()) {
		ssx << "x = " << i->wml_x();
		ssy << "y = " << i->wml_y();
		++i;
		while (i != context_manager_->get_map().selection().end()) {
			ssx << ", " << i->wml_x();
			ssy << ", " << i->wml_y();
			++i;
		}
		ssx << "\n" << ssy.str() << "\n";
		desktop::clipboard::copy_to_clipboard(ssx.str(), false);
	}
}

void editor_controller::perform_delete(editor_action* action)
{
	if (action) {
		const std::unique_ptr<editor_action> action_auto(action);
		get_current_map_context().perform_action(*action);
	}
}

void editor_controller::perform_refresh_delete(editor_action* action, bool drag_part /* =false */)
{
	if (action) {
		const std::unique_ptr<editor_action> action_auto(action);
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
	set_button_state();
	toolkit_->adjust_size();
	toolkit_->get_palette_manager()->draw_contents();
	get_current_map_context().get_labels().recalculate_labels();
}

void editor_controller::undo()
{
	get_current_map_context().undo();
	context_manager_->refresh_after_action();
}

void editor_controller::redo()
{
	get_current_map_context().redo();
	context_manager_->refresh_after_action();
}

void editor_controller::mouse_motion(int x, int y, const bool /*browse*/,
		bool update, map_location /*new_loc*/)
{
	if (mouse_handler_base::mouse_motion_default(x, y, update)) return;
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (context_manager_->get_map().on_board_with_border(drag_from_hex_) && is_dragging()) {
		editor_action* a = nullptr;
		bool partial = false;
		editor_action* last_undo = get_current_map_context().last_undo_action();
		if (dragging_left_ && (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(1)) != 0) {
			if (!context_manager_->get_map().on_board_with_border(hex_clicked)) return;
			a = get_mouse_action().drag_left(*gui_, x, y, partial, last_undo);
		} else if (dragging_right_ && (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(3)) != 0) {
			if (!context_manager_->get_map().on_board_with_border(hex_clicked)) return;
			a = get_mouse_action().drag_right(*gui_, x, y, partial, last_undo);
		}
		//Partial means that the mouse action has modified the
		//last undo action and the controller shouldn't add
		//anything to the undo stack (hence a different perform_ call)
		if (a != nullptr) {
			const std::unique_ptr<editor_action> aa(a);
			if (partial) {
				get_current_map_context().perform_partial_action(*a);
			} else {
				get_current_map_context().perform_action(*a);
			}
			context_manager_->refresh_after_action(true);
		}
	} else {
		get_mouse_action().move(*gui_, hex_clicked);
	}
	gui().highlight_hex(hex_clicked);
}

bool editor_controller::allow_mouse_wheel_scroll(int x, int y)
{
	return context_manager_->get_map().on_board_with_border(gui().hex_clicked_on(x,y));
}

bool editor_controller::right_click_show_menu(int /*x*/, int /*y*/, const bool /*browse*/)
{
	return get_mouse_action().has_context_menu();
}

bool editor_controller::left_click(int x, int y, const bool browse)
{
	toolkit_->clear_mouseover_overlay();
	if (mouse_handler_base::left_click(x, y, browse))
		return true;

	LOG_ED << "Left click, after generic handling\n";
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (!context_manager_->get_map().on_board_with_border(hex_clicked))
		return true;

	LOG_ED << "Left click action " << hex_clicked << "\n";
	editor_action* a = get_mouse_action().click_left(*gui_, x, y);
	perform_refresh_delete(a, true);
	if (a) set_button_state();

	return false;
}

void editor_controller::left_drag_end(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action().drag_end_left(*gui_, x, y);
	perform_delete(a);
}

void editor_controller::left_mouse_up(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action().up_left(*gui_, x, y);
	perform_delete(a);
	if (a) set_button_state();
	toolkit_->set_mouseover_overlay();
	context_manager_->refresh_after_action();
}

bool editor_controller::right_click(int x, int y, const bool browse)
{
	toolkit_->clear_mouseover_overlay();
	if (mouse_handler_base::right_click(x, y, browse)) return true;
	LOG_ED << "Right click, after generic handling\n";
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (!context_manager_->get_map().on_board_with_border(hex_clicked)) return true;
	LOG_ED << "Right click action " << hex_clicked << "\n";
	editor_action* a = get_mouse_action().click_right(*gui_, x, y);
	perform_refresh_delete(a, true);
	if (a) set_button_state();
	return false;
}

void editor_controller::right_drag_end(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action().drag_end_right(*gui_, x, y);
	perform_delete(a);
}

void editor_controller::right_mouse_up(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action().up_right(*gui_, x, y);
	perform_delete(a);
	if (a) set_button_state();
	toolkit_->set_mouseover_overlay();
	context_manager_->refresh_after_action();
}

void editor_controller::terrain_description()
{
	const map_location& loc = gui().mouseover_hex();
	if (context_manager_->get_map().on_board(loc) == false)
		return;

	const terrain_type& type = context_manager_->get_map().get_terrain_info(loc);
	help::show_terrain_description(gui().video(), type);
}

void editor_controller::process_keyup_event(const SDL_Event& event)
{
	editor_action* a = get_mouse_action().key_event(gui(), event);
	perform_refresh_delete(a);
	toolkit_->set_mouseover_overlay();
}

hotkey::command_executor * editor_controller::get_hotkey_command_executor() {
	return this;
}

void editor_controller::scroll_up(bool on)
{
	controller_base::set_scroll_up(on);
}

void editor_controller::scroll_down(bool on)
{
	controller_base::set_scroll_down(on);
}

void editor_controller::scroll_left(bool on)
{
	controller_base::set_scroll_left(on);
}

void editor_controller::scroll_right(bool on)
{
	controller_base::set_scroll_right(on);
}

std::vector<std::string> editor_controller::additional_actions_pressed()
{
	return toolkit_->get_palette_manager()->active_palette().action_pressed();
}

} //end namespace editor
