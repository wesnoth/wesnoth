/* $Id$ */
/*
   Copyright (C) 2008 - 2011 by Tomasz Sniatowski <kailoran@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#define GETTEXT_DOMAIN "wesnoth-editor"

#include "asserts.hpp"
#include "action.hpp"
#include "editor_controller.hpp"
#include "editor_palettes.hpp"
#include "editor_preferences.hpp"
#include "mouse_action.hpp"

#include "gui/dialogs/editor_new_map.hpp"
#include "gui/dialogs/editor_generate_map.hpp"
#include "gui/dialogs/editor_resize_map.hpp"
#include "gui/dialogs/editor_settings.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"

#include "../clipboard.hpp"
#include "../filechooser.hpp"
#include "../filesystem.hpp"
#include "../foreach.hpp"
#include "../gettext.hpp"
#include "../map_create.hpp"
#include "../mapgen.hpp"
#include "../preferences_display.hpp"
#include "../rng.hpp"
#include "../sound.hpp"

#include "formula_string_utils.hpp"

#include <boost/bind.hpp>

namespace editor {

/**
 * Utility class to properly refresh the display when the map context object is replaced
 * without duplicating code.
 */
class map_context_refresher
{
public:
	map_context_refresher(editor_controller& ec, const map_context& other_mc)
	: ec_(ec), size_changed_(!ec.get_map().same_size_as(other_mc.get_map())), refreshed_(false)
	{
	}
	~map_context_refresher() {
		if (!refreshed_) refresh();
	}
	void refresh() {
		ec_.gui().change_map(&ec_.get_map());
		ec_.reload_map();
	}
private:
	editor_controller& ec_;
	bool size_changed_;
	bool refreshed_;
};

editor_controller::editor_controller(const config &game_config, CVideo& video, map_context* init_map_context /*=NULL*/)
	: controller_base(SDL_GetTicks(), game_config, video)
	, mouse_handler_base()
	, rng_(NULL)
	, rng_setter_(NULL)
	, map_contexts_()
	, current_context_index_(0)
	, gui_(NULL)
	, map_generators_()
	, tods_()
	, size_specs_()
	, palette_()
	, brush_bar_()
	, prefs_disp_manager_(NULL)
	, tooltip_manager_(video)
	, floating_label_manager_(NULL)
	, do_quit_(false)
	, quit_mode_(EXIT_ERROR)
	, brushes_()
	, brush_(NULL)
	, mouse_actions_()
	, mouse_action_hints_()
	, mouse_action_(NULL)
	, toolbar_dirty_(true)
	, foreground_terrain_(t_translation::MOUNTAIN)
	, background_terrain_(t_translation::GRASS_LAND)
	, clipboard_()
	, auto_update_transitions_(preferences::editor::auto_update_transitions())
	, use_mdi_(preferences::editor::use_mdi())
	, default_dir_(preferences::editor::default_dir())
{
	if (init_map_context == NULL) {
		create_default_context();
	} else {
		add_map_context(init_map_context);
	}
	if (default_dir_.empty()) {
		default_dir_ = get_dir(get_dir(get_user_data_dir() + "/editor") + "/maps");
	}
	init_gui(video);
	init_brushes(game_config);
	init_mouse_actions(game_config);
	init_map_generators(game_config);
	init_tods(game_config);
	init_sidebar(game_config);
	init_music(game_config);
	hotkey_set_mouse_action(hotkey::HOTKEY_EDITOR_TOOL_PAINT);
	rng_.reset(new rand_rng::rng());
	rng_setter_.reset(new rand_rng::set_random_generator(rng_.get()));
	hotkey::get_hotkey(hotkey::HOTKEY_QUIT_GAME).set_description(_("Quit Editor"));
	get_map_context().set_starting_position_labels(gui());
	cursor::set(cursor::NORMAL);
	image::set_colour_adjustment(preferences::editor::tod_r(), preferences::editor::tod_g(), preferences::editor::tod_b());
	theme& theme = gui().get_theme();
	const theme::menu* default_tool_menu = NULL;
	BOOST_FOREACH (const theme::menu& m, theme.menus()) {
		std::string s = m.get_id();
		if (m.get_id() == "draw_button_editor") {
			default_tool_menu = &m;
			break;
		}
	}
	refresh_all();
	events::raise_draw_event();
	if (default_tool_menu != NULL) {
		const SDL_Rect& menu_loc = default_tool_menu->location(get_display().screen_area());
		show_menu(default_tool_menu->items(),menu_loc.x+1,menu_loc.y + menu_loc.h + 1,false);
		return;
	}
}

void editor_controller::init_gui(CVideo& video)
{
	const config &theme_cfg = get_theme(game_config_, "editor");
	gui_.reset(new editor_display(video, get_map(), theme_cfg, config()));
	gui_->set_grid(preferences::grid());
	prefs_disp_manager_.reset(new preferences::display_manager(&gui()));
	gui_->add_redraw_observer(boost::bind(&editor_controller::display_redraw_callback, this, _1));
	floating_label_manager_.reset(new font::floating_label_context());
	gui().set_draw_coordinates(preferences::editor::draw_hex_coordinates());
	gui().set_draw_terrain_codes(preferences::editor::draw_terrain_codes());
}

void editor_controller::init_sidebar(const config& game_config)
{
	size_specs_.reset(new size_specs());
	adjust_sizes(gui(), *size_specs_);
	palette_.reset(new terrain_palette(gui(), *size_specs_, game_config,
		foreground_terrain_, background_terrain_));
	brush_bar_.reset(new brush_bar(gui(), *size_specs_, brushes_, &brush_));
}

void editor_controller::init_brushes(const config& game_config)
{
	BOOST_FOREACH (const config &i, game_config.child_range("brush")) {
		brushes_.push_back(brush(i));
	}
	if (brushes_.size() == 0) {
		ERR_ED << "No brushes defined!";
		brushes_.push_back(brush());
		brushes_[0].add_relative_location(0, 0);
	}
	brush_ = &brushes_[0];
}

void editor_controller::init_mouse_actions(const config& game_config)
{
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_PAINT,
		new mouse_action_paint(foreground_terrain_, background_terrain_, &brush_, key_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_FILL,
		new mouse_action_fill(foreground_terrain_, background_terrain_, key_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_SELECT,
		new mouse_action_select(&brush_, key_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_TOOL_STARTING_POSITION,
		new mouse_action_starting_position(key_)));
	mouse_actions_.insert(std::make_pair(hotkey::HOTKEY_EDITOR_PASTE,
		new mouse_action_paste(clipboard_, key_)));
	BOOST_FOREACH (const theme::menu& menu, gui().get_theme().menus()) {
		if (menu.items().size() == 1) {
			hotkey::HOTKEY_COMMAND hk = hotkey::get_hotkey(menu.items().front()).get_id();
			mouse_action_map::iterator i = mouse_actions_.find(hk);
			if (i != mouse_actions_.end()) {
				i->second->set_toolbar_button(&menu);
			}
		}
	}
	BOOST_FOREACH (const config &c, game_config.child_range("editor_tool_hint")) {
		mouse_action_map::iterator i =
			mouse_actions_.find(hotkey::get_hotkey(c["id"]).get_id());
		if (i != mouse_actions_.end()) {
			mouse_action_hints_.insert(std::make_pair(i->first, c["text"]));
		}
	}
}

void editor_controller::init_map_generators(const config& game_config)
{
	BOOST_FOREACH (const config &i, game_config.child_range("multiplayer"))
	{
		if (i["map_generation"] == "default") {
			const config &generator_cfg = i.child("generator");
			if (!generator_cfg) {
				ERR_ED << "Scenario \"" << i["name"] << "\" with id " << i["id"]
					<< " has map_generation=default but no [generator] tag";
			} else {
				map_generator* m = create_map_generator("", generator_cfg);
				map_generators_.push_back(m);
			}
		}
	}
}

void editor_controller::init_tods(const config& game_config)
{
	const config &cfg = game_config.child("editor_times");
	if (!cfg) {
		ERR_ED << "No editor time-of-day defined\n";
		return;
	}
	BOOST_FOREACH (const config &i, cfg.child_range("time")) {
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
	BOOST_FOREACH (const config &i, cfg.child_range("music")) {
		sound::play_music_config(i);
	}
	sound::commit_music_changes();
}


void editor_controller::load_tooltips()
{
	// Tooltips for the groups
	palette_->load_tooltips();
}

editor_controller::~editor_controller()
{
	BOOST_FOREACH (const mouse_action_map::value_type a, mouse_actions_) {
		delete a.second;
	}
	BOOST_FOREACH (map_generator* m, map_generators_) {
		delete m;
	}
	BOOST_FOREACH (map_context* mc, map_contexts_) {
		delete mc;
	}
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
	std::vector<std::string> modified;
	BOOST_FOREACH (map_context* mc, map_contexts_) {
		if (mc->modified()) {
			if (!mc->get_filename().empty()) {
				modified.push_back(mc->get_filename());
			} else {
				modified.push_back(_("(New Map)"));
			}
		}
	}
	std::string message;
	if (modified.empty()) {
		message = _("Do you really want to quit?");
	} else if (modified.size() == 1) {
		message = _("Do you really want to quit? Changes in the map since the last save will be lost.");
	} else {
		message = _("Do you really want to quit? The following maps were modified and all changes since the last save will be lost:");
		BOOST_FOREACH (std::string& str, modified) {
			message += "\n" + str;
		}
	}
	int res = gui::dialog(gui(),_("Quit"),message,gui::YES_NO).show();
	if (res == 0) {
		do_quit_ = true;
		quit_mode_ = mode;
	}
}

int editor_controller::add_map_context(map_context* mc)
{
	map_contexts_.push_back(mc);
	return map_contexts_.size() - 1;
}

void editor_controller::create_default_context()
{
	map_context* mc = new map_context(editor_map(game_config_, 44, 33, t_translation::GRASS_LAND));
	add_map_context(mc);
}

void editor_controller::close_current_context()
{
	if (!confirm_discard()) return;
	map_context* current = map_contexts_[current_context_index_];
	if (map_contexts_.size() == 1) {
		create_default_context();
		map_contexts_.erase(map_contexts_.begin());
	} else if (current_context_index_ == static_cast<int>(map_contexts_.size()) - 1) {
		map_contexts_.pop_back();
		current_context_index_--;
	} else {
		map_contexts_.erase(map_contexts_.begin() + current_context_index_);
	}
	map_context_refresher(*this, *current);
	delete current;
}

void editor_controller::switch_context(const int index)
{
	if (index < 0 || static_cast<size_t>(index) >= map_contexts_.size()) {
		WRN_ED << "Invalid index in switch map context: " << index << "\n";
		return;
	}
	map_context_refresher mcr(*this, *map_contexts_[index]);
	current_context_index_ = index;
}

void editor_controller::replace_map_context(map_context* new_mc)
{
	std::auto_ptr<map_context> del(map_contexts_[current_context_index_]);
	map_context_refresher mcr(*this, *new_mc);
	map_contexts_[current_context_index_] = new_mc;
}

void editor_controller::editor_settings_dialog()
{
	if (tods_.empty()) {
		gui2::show_error_message(gui().video(),
				_("No editor time-of-day found."));
		return;
	}

	gui2::teditor_settings dialog;
	dialog.set_tods(tods_);
	dialog.set_current_adjustment(preferences::editor::tod_r(), preferences::editor::tod_g(), preferences::editor::tod_b());
	dialog.set_redraw_callback(boost::bind(&editor_controller::editor_settings_dialog_redraw_callback, this, _1, _2, _3));
	image::colour_adjustment_resetter adjust_resetter;
	dialog.set_use_mdi(use_mdi_);
	dialog.show(gui().video());

	int res = dialog.get_retval();
	if(res == gui2::twindow::OK) {
		image::set_colour_adjustment(dialog.get_red(), dialog.get_green(), dialog.get_blue());
		preferences::editor::set_tod_r(dialog.get_red());
		preferences::editor::set_tod_g(dialog.get_green());
		preferences::editor::set_tod_b(dialog.get_blue());
		use_mdi_ = dialog.get_use_mdi();
		preferences::editor::set_use_mdi(use_mdi_);
	} else {
		adjust_resetter.reset();
	}
	refresh_all();
}

void editor_controller::editor_settings_dialog_redraw_callback(int r, int g, int b)
{
	SCOPE_ED;
	image::set_colour_adjustment(r, g, b);
	gui().redraw_everything();
}

bool editor_controller::confirm_discard()
{
	if (get_map_context().modified()) {
		return !gui::dialog(gui(), _("Unsaved Changes"),
			_("Do you want to discard all changes you made to the map since the last save?"), gui::YES_NO).show();
	} else {
		return true;
	}
}

void editor_controller::set_default_dir(const std::string& str)
{
	default_dir_ = str;
}

void editor_controller::load_map_dialog(bool force_same_context /* = false */)
{
	if (!use_mdi_ && !confirm_discard()) return;
	std::string fn = directory_name(get_map_context().get_filename());
	if (fn.empty()) {
		fn = default_dir_;
	}
	int res = dialogs::show_file_chooser_dialog(gui(), fn, _("Choose a Map to Open"));
	if (res == 0) {
		load_map(fn, force_same_context ? false : use_mdi_);
	}
}

void editor_controller::new_map_dialog()
{
	if (!use_mdi_ && !confirm_discard()) return;
	gui2::teditor_new_map dialog;
	dialog.set_map_width(get_map().w());
	dialog.set_map_height(get_map().h());

	dialog.show(gui().video());
	int res = dialog.get_retval();
	if(res == gui2::twindow::OK) {
		int w = dialog.map_width();
		int h = dialog.map_height();
		t_translation::t_terrain fill = t_translation::GRASS_LAND;
		new_map(w, h, fill, use_mdi_);
	}
}

void editor_controller::save_map_as_dialog()
{
	std::string input_name = get_map_context().get_filename();
	if (input_name.empty()) {
		input_name = default_dir_;
	}
	const std::string old_input_name = input_name;

	int res = 0;
	int overwrite_res = 1;
	do {
		input_name = old_input_name;
		res = dialogs::show_file_chooser_dialog_save(gui(), input_name, _("Save the Map As"));
		if (res == 0) {
			if (file_exists(input_name)) {
				overwrite_res = gui::dialog(gui(), "",
					_("The file already exists. Do you want to overwrite it?"),
					gui::YES_NO).show();
			} else {
				overwrite_res = 0;
			}
		} else {
			return; //cancel pressed
		}
	} while (overwrite_res != 0);

	save_map_as(input_name);
}

void editor_controller::generate_map_dialog()
{
	if (map_generators_.empty()) {
		gui2::show_error_message(gui().video(),
				_("No random map generators found."));
		return;
	}
	gui2::teditor_generate_map dialog;
	dialog.set_map_generators(map_generators_);
	dialog.set_gui(&gui());
	dialog.show(gui().video());
	if (dialog.get_retval() == gui2::twindow::OK) {
		std::string map_string;
		try {
			map_string = dialog.get_selected_map_generator()
				->create_map(std::vector<std::string>());
		} catch (mapgen_exception& e) {
			gui2::show_transient_message(gui().video(), _("Map creation failed."), e.what());
			return;
		}
		if (map_string.empty()) {
			gui2::show_transient_message(gui().video(), "", _("Map creation failed."));
		} else {
			editor_map new_map(game_config_, map_string);
			editor_action_whole_map a(new_map);
			perform_refresh(a);
		}
	}
}

void editor_controller::apply_mask_dialog()
{
	std::string fn = get_map_context().get_filename();
	if (fn.empty()) {
		fn = default_dir_;
	}
	int res = dialogs::show_file_chooser_dialog(gui(), fn, _("Choose a mask to apply"));
	if (res == 0) {
		try {
			map_context mask(game_config_, fn);
			editor_action_apply_mask a(mask.get_map());
			perform_refresh(a);
		} catch (editor_map_load_exception& e) {
			gui2::show_transient_message(gui().video(), _("Error loading mask"), e.what());
			return;
		} catch (editor_action_exception& e) {
			gui2::show_error_message(gui().video(), e.what());
			return;
		}
	}
}

void editor_controller::create_mask_to_dialog()
{
	std::string fn = get_map_context().get_filename();
	if (fn.empty()) {
		fn = default_dir_;
	}
	int res = dialogs::show_file_chooser_dialog(gui(), fn, _("Choose target map"));
	if (res == 0) {
		try {
			map_context map(game_config_, fn);
			editor_action_create_mask a(map.get_map());
			perform_refresh(a);
		} catch (editor_map_load_exception& e) {
			gui2::show_transient_message(gui().video(), _("Error loading map"), e.what());
			return;
		} catch (editor_action_exception& e) {
			gui2::show_error_message(gui().video(), e.what());
			return;
		}
	}
}

void editor_controller::resize_map_dialog()
{
	gui2::teditor_resize_map dialog;
	dialog.set_map_width(get_map().w());
	dialog.set_map_height(get_map().h());
	dialog.set_old_map_width(get_map().w());
	dialog.set_old_map_height(get_map().h());

	dialog.show(gui().video());
	int res = dialog.get_retval();
	if(res == gui2::twindow::OK) {
		int w = dialog.map_width();
		int h = dialog.map_height();
		if (w != get_map().w() || h != get_map().h()) {
			t_translation::t_terrain fill = background_terrain_;
			if (dialog.copy_edge_terrain()) {
				fill = t_translation::NONE_TERRAIN;
			}
			int x_offset = get_map().w() - w;
			int y_offset = get_map().h() - h;
			switch (dialog.expand_direction()) {
				case gui2::teditor_resize_map::EXPAND_BOTTOM_RIGHT:
				case gui2::teditor_resize_map::EXPAND_BOTTOM:
				case gui2::teditor_resize_map::EXPAND_BOTTOM_LEFT:
					y_offset = 0;
					break;
				case gui2::teditor_resize_map::EXPAND_RIGHT:
				case gui2::teditor_resize_map::EXPAND_CENTER:
				case gui2::teditor_resize_map::EXPAND_LEFT:
					y_offset /= 2;
					break;
				case gui2::teditor_resize_map::EXPAND_TOP_RIGHT:
				case gui2::teditor_resize_map::EXPAND_TOP:
				case gui2::teditor_resize_map::EXPAND_TOP_LEFT:
					break;
				default:
					y_offset = 0;
					WRN_ED << "Unknown resize expand direction\n";
			}
			switch (dialog.expand_direction()) {
				case gui2::teditor_resize_map::EXPAND_BOTTOM_RIGHT:
				case gui2::teditor_resize_map::EXPAND_RIGHT:
				case gui2::teditor_resize_map::EXPAND_TOP_RIGHT:
					x_offset = 0;
					break;
				case gui2::teditor_resize_map::EXPAND_BOTTOM:
				case gui2::teditor_resize_map::EXPAND_CENTER:
				case gui2::teditor_resize_map::EXPAND_TOP:
					x_offset /= 2;
					break;
				case gui2::teditor_resize_map::EXPAND_BOTTOM_LEFT:
				case gui2::teditor_resize_map::EXPAND_LEFT:
				case gui2::teditor_resize_map::EXPAND_TOP_LEFT:
					break;
				default:
					x_offset = 0;
			}
			editor_action_resize_map a(w, h, x_offset, y_offset, fill);
			perform_refresh(a);
		}
	}
}

bool editor_controller::save_map_as(const std::string& filename)
{
	size_t is_open = check_open_map(filename);
	if (is_open < map_contexts_.size()
			&& is_open != static_cast<unsigned>(current_context_index_)) {

		gui::dialog(gui(), _("This map is already open."), filename).show();
		return false;
	}
	std::string old_filename = get_map_context().get_filename();
	bool embedded = get_map_context().is_embedded();
	get_map_context().set_filename(filename);
	get_map_context().set_embedded(false);
	if (!save_map(true)) {
		get_map_context().set_filename(old_filename);
		get_map_context().set_embedded(embedded);
		return false;
	} else {
		return true;
	}
}

bool editor_controller::save_map(bool display_confirmation)
{
	try {
		get_map_context().save();
		if (display_confirmation) {
			gui2::show_transient_message(gui().video(), "", _("Map saved."));
		}
	} catch (editor_map_save_exception& e) {
		gui2::show_transient_message(gui().video(), "", e.what());
		return false;
	}
	return true;
}

size_t editor_controller::check_open_map(const std::string& fn) const
{
	size_t i = 0;
	while (i < map_contexts_.size() && map_contexts_[i]->get_filename() != fn) ++i;
	return i;
}


bool editor_controller::check_switch_open_map(const std::string& fn)
{
	size_t i = check_open_map(fn);
	if (i < map_contexts_.size()) {
		gui::dialog(gui(), _("This map is already open."), fn).show();
		switch_context(i);
		return true;
	}
	return false;
}

void editor_controller::load_map(const std::string& filename, bool new_context)
{
	if (new_context && check_switch_open_map(filename)) return;
	LOG_ED << "Load map: " << filename << (new_context ? " (new)" : " (same)") << "\n";
	try {
		std::auto_ptr<map_context> mc(new map_context(game_config_, filename));
		if (mc->get_filename() != filename) {
			if (new_context && check_switch_open_map(mc->get_filename())) return;
		}
		if (new_context) {
			int new_id = add_map_context(mc.release());
			switch_context(new_id);
		} else {
			replace_map_context(mc.release());
		}
		if (get_map_context().is_embedded()) {
			const char* msg = _("Loaded embedded map data");
			gui2::show_transient_message(gui().video(), _("Map loaded from scenario"), msg);
		} else {
			if (get_map_context().get_filename() != filename) {
				if (get_map_context().get_map_data_key().empty()) {
					ERR_ED << "Internal error, map context filename changed: "
						<< filename << " -> " << get_map_context().get_filename()
						<< " with no apparent scenario load\n";
				} else {
					utils::string_map symbols;
					symbols["old"] = filename;
					const char* msg = _("Loaded referenced map file:\n"
						"$new");
					symbols["new"] = get_map_context().get_filename();
					symbols["map_data"] = get_map_context().get_map_data_key();
					gui2::show_transient_message(gui().video(), _("Map loaded from scenario"),
						vgettext(msg, symbols));
				}
			}
		}
	} catch (editor_map_load_exception& e) {
		gui2::show_transient_message(gui().video(), _("Error loading map"), e.what());
		return;
	}
}

void editor_controller::revert_map()
{
	if (!confirm_discard()) return;
	std::string filename = get_map_context().get_filename();
	if (filename.empty()) {
		ERR_ED << "Empty filename in map revert\n";
		return;
	}
	load_map(filename, false);
}

void editor_controller::new_map(int width, int height, t_translation::t_terrain fill, bool new_context)
{
	editor_map m(game_config_, width, height, fill);
	if (new_context) {
		int new_id = add_map_context(new map_context(m));
		switch_context(new_id);
	} else {
		replace_map_context(new map_context(m));
	}
}

void editor_controller::reload_map()
{
	gui().reload_map();
	get_map_context().set_needs_reload(false);
	get_map_context().reset_starting_position_labels(gui());
	refresh_all();
}

void editor_controller::refresh_all()
{
	gui().rebuild_all();
	get_map_context().set_needs_terrain_rebuild(false);
	gui().redraw_everything();
	get_map_context().clear_changed_locations();
	gui().recalculate_minimap();
}

void editor_controller::refresh_after_action(bool drag_part)
{
	if (get_map_context().needs_reload()) {
		reload_map();
		return;
	} else {
		const std::set<map_location>& changed_locs = get_map_context().changed_locations();

		if (get_map_context().needs_terrain_rebuild()) {
			if ((auto_update_transitions_ == preferences::editor::TransitionUpdateMode::on)
			|| ((auto_update_transitions_ == preferences::editor::TransitionUpdateMode::partial)
			&& (!drag_part || get_map_context().everything_changed()))) {
				gui().rebuild_all();
				get_map_context().set_needs_terrain_rebuild(false);
				gui().invalidate_all();
			} else {
				BOOST_FOREACH (const map_location& loc, changed_locs) {
					gui().rebuild_terrain(loc);
				}
				gui().invalidate(changed_locs);
			}
		} else {
			if (get_map_context().everything_changed()) {
				gui().invalidate_all();
			} else {
				gui().invalidate(changed_locs);
			}
		}
		if (get_map_context().needs_labels_reset()) {
			get_map_context().reset_starting_position_labels(gui());
		}
	}
	get_map_context().clear_changed_locations();
	gui().recalculate_minimap();
}

bool editor_controller::can_execute_command(hotkey::HOTKEY_COMMAND command, int index) const
{

	using namespace hotkey; //reduce hotkey:: clutter
	switch (command) {
		case HOTKEY_NULL:
			if (index >= 0) {
				unsigned i = static_cast<unsigned>(index);
				if (i < map_contexts_.size()) {
					return true;
				}
			}
			return false;
		case HOTKEY_ZOOM_IN:
		case HOTKEY_ZOOM_OUT:
		case HOTKEY_ZOOM_DEFAULT:
		case HOTKEY_FULLSCREEN:
		case HOTKEY_SCREENSHOT:
		case HOTKEY_MAP_SCREENSHOT:
		case HOTKEY_TOGGLE_GRID:
		case HOTKEY_MOUSE_SCROLL:
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
		case HOTKEY_EDITOR_QUIT_TO_DESKTOP:
		case HOTKEY_EDITOR_SETTINGS:
		case HOTKEY_EDITOR_MAP_NEW:
		case HOTKEY_EDITOR_MAP_LOAD:
		case HOTKEY_EDITOR_MAP_SAVE_AS:
		case HOTKEY_EDITOR_BRUSH_NEXT:
		case HOTKEY_EDITOR_TOOL_NEXT:
		case HOTKEY_EDITOR_TERRAIN_PALETTE_SWAP:
			return true; //editor hotkeys we can always do
		case HOTKEY_EDITOR_MAP_SAVE:
		case HOTKEY_EDITOR_SWITCH_MAP:
		case HOTKEY_EDITOR_CLOSE_MAP:
			return true;
		case HOTKEY_EDITOR_MAP_REVERT:
			return !get_map_context().get_filename().empty();
			return true;
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			return true; //tool selection always possible
		case HOTKEY_EDITOR_CUT:
		case HOTKEY_EDITOR_COPY:
		case HOTKEY_EDITOR_EXPORT_SELECTION_COORDS:
		case HOTKEY_EDITOR_SELECTION_FILL:
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			return !get_map().selection().empty();
		case HOTKEY_EDITOR_SELECTION_ROTATE:
		case HOTKEY_EDITOR_SELECTION_FLIP:
		case HOTKEY_EDITOR_SELECTION_GENERATE:
			return false; //not implemented
		case HOTKEY_EDITOR_PASTE:
			return !clipboard_.empty();
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW:
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW:
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL:
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL:
			return !clipboard_.empty();
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
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			return is_mouse_action_set(command) ? ACTION_ON : ACTION_OFF;
		case HOTKEY_EDITOR_DRAW_COORDINATES:
			return gui_->get_draw_coordinates() ? ACTION_ON : ACTION_OFF;
		case HOTKEY_EDITOR_DRAW_TERRAIN_CODES:
			return gui_->get_draw_terrain_codes() ? ACTION_ON : ACTION_OFF;
		case HOTKEY_NULL:
			return index == current_context_index_ ? ACTION_ON : ACTION_OFF;
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
			if (index >= 0) {
				unsigned i = static_cast<unsigned>(index);
				if (i < map_contexts_.size()) {
					switch_context(index);
					return true;
				}
			}
			return false;
		case HOTKEY_QUIT_GAME:
			quit_confirm(EXIT_NORMAL);
			return true;
		case HOTKEY_EDITOR_QUIT_TO_DESKTOP:
			quit_confirm(EXIT_QUIT_TO_DESKTOP);
			return true;
		case HOTKEY_EDITOR_SETTINGS:
			editor_settings_dialog();
			return true;
		case HOTKEY_EDITOR_TERRAIN_PALETTE_SWAP:
			palette_->swap();
			set_mouseover_overlay();
			return true;
		case HOTKEY_EDITOR_PARTIAL_UNDO:
			if (dynamic_cast<const editor_action_chain*>(get_map_context().last_undo_action()) != NULL) {
				get_map_context().partial_undo();
				refresh_after_action();
			} else {
				undo();
			}
			return true;
		case HOTKEY_EDITOR_TOOL_PAINT:
		case HOTKEY_EDITOR_TOOL_FILL:
		case HOTKEY_EDITOR_TOOL_SELECT:
		case HOTKEY_EDITOR_TOOL_STARTING_POSITION:
			hotkey_set_mouse_action(command);
			return true;
		case HOTKEY_EDITOR_PASTE: //paste is somewhat different as it might be "one action then revert to previous mode"
			hotkey_set_mouse_action(command);
			return true;
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CW:
			clipboard_.rotate_60_cw();
			update_mouse_action_highlights();
			return true;
		case HOTKEY_EDITOR_CLIPBOARD_ROTATE_CCW:
			clipboard_.rotate_60_ccw();
			update_mouse_action_highlights();
			return true;
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_HORIZONTAL:
			clipboard_.flip_horizontal();
			update_mouse_action_highlights();
			return true;
		case HOTKEY_EDITOR_CLIPBOARD_FLIP_VERTICAL:
			clipboard_.flip_vertical();
			update_mouse_action_highlights();
			return true;
		case HOTKEY_EDITOR_BRUSH_NEXT:
			cycle_brush();
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
			if (!get_map().everything_selected()) {
				perform_refresh(editor_action_select_all());
				return true;
			} //else intentionally fall through
		case HOTKEY_EDITOR_SELECT_INVERSE:
			perform_refresh(editor_action_select_inverse());
			return true;
		case HOTKEY_EDITOR_SELECT_NONE:
			perform_refresh(editor_action_select_none());
		case HOTKEY_EDITOR_SELECTION_FILL:
			fill_selection();
			return true;
		case HOTKEY_EDITOR_SELECTION_RANDOMIZE:
			perform_refresh(editor_action_shuffle_area(get_map().selection()));
			return true;
		case HOTKEY_EDITOR_CLOSE_MAP:
			close_current_context();
			return true;
		case HOTKEY_EDITOR_MAP_LOAD:
			load_map_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_REVERT:
			revert_map();
			return true;
		case HOTKEY_EDITOR_MAP_NEW:
			new_map_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_SAVE:
			if (get_map_context().get_filename().empty()
			|| is_directory(get_map_context().get_filename())) {
				save_map_as_dialog();
			} else {
				save_map();
			}
			return true;
		case HOTKEY_EDITOR_MAP_SAVE_AS:
			save_map_as_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_GENERATE:
			generate_map_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_APPLY_MASK:
			apply_mask_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_CREATE_MASK_TO:
			create_mask_to_dialog();
			return true;
		case HOTKEY_EDITOR_MAP_RESIZE:
			resize_map_dialog();
			return true;
		case HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS:
			auto_update_transitions_ = (auto_update_transitions_ + 1)
				% preferences::editor::TransitionUpdateMode::count;
			preferences::editor::set_auto_update_transitions(auto_update_transitions_);
			if (auto_update_transitions_ != preferences::editor::TransitionUpdateMode::on) {
				return true;
			} // else intentionally fall through
		case HOTKEY_EDITOR_UPDATE_TRANSITIONS:
			refresh_all();
			return true;
		case HOTKEY_EDITOR_REFRESH:
			reload_map();
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
	return false;
}

void editor_controller::expand_open_maps_menu(std::vector<std::string>& items)
{
	for (unsigned int i = 0; i < items.size(); ++i) {
		if (items[i] == "editor-switch-map") {
			items.erase(items.begin() + i);
			std::vector<std::string> contexts;
			for (size_t mci = 0; mci < map_contexts_.size(); ++mci) {
				std::string filename = map_contexts_[mci]->get_filename();
				if (filename.empty()) {
					filename = _("(New Map)");
				}
				std::string label = "[" + lexical_cast<std::string>(mci) + "] "
					+ filename;
				if (map_contexts_[mci]->is_embedded()) {
					label += " (E)";
				}
				contexts.push_back(label);
			}
			items.insert(items.begin() + i, contexts.begin(), contexts.end());
			break;
		}
	}
}

void editor_controller::show_menu(const std::vector<std::string>& items_arg, int xloc, int yloc, bool context_menu)
{
	if (context_menu) {
		if (!get_map().on_board_with_border(gui().hex_clicked_on(xloc, yloc))) {
			return;
		}
	}

	std::vector<std::string> items = items_arg;
	hotkey::HOTKEY_COMMAND command;
	std::vector<std::string>::iterator i = items.begin();
	while(i != items.end()) {
		command = hotkey::get_hotkey(*i).get_id();
		if (command == hotkey::HOTKEY_UNDO) {
			if (get_map_context().can_undo()) {
				hotkey::get_hotkey(*i).set_description(_("Undo"));
			} else {
				hotkey::get_hotkey(*i).set_description(_("Can't Undo"));
			}
		} else if (command == hotkey::HOTKEY_REDO) {
			if (get_map_context().can_redo()) {
				hotkey::get_hotkey(*i).set_description(_("Redo"));
			} else {
				hotkey::get_hotkey(*i).set_description(_("Can't Redo"));
			}
		} else if (command == hotkey::HOTKEY_EDITOR_AUTO_UPDATE_TRANSITIONS) {
			switch (auto_update_transitions_) {
				case preferences::editor::TransitionUpdateMode::on:
					hotkey::get_hotkey(*i).set_description(_("Auto-update Terrain Transitions: Yes"));
					break;
				case preferences::editor::TransitionUpdateMode::partial:
					hotkey::get_hotkey(*i).set_description(_("Auto-update Terrain Transitions: Partial"));
					break;
				case preferences::editor::TransitionUpdateMode::off:
				default:
					hotkey::get_hotkey(*i).set_description(_("Auto-update Terrain Transitions: No"));
			}
		} else if(!can_execute_command(command)
		|| (context_menu && !in_context_menu(command))) {
			i = items.erase(i);
			continue;
		}
		++i;
	}
	if (!items.empty() && items.front() == "editor-switch-map") {
		expand_open_maps_menu(items);
		context_menu = true; //FIXME hack to display a one-item menu
	}
	command_executor::show_menu(items, xloc, yloc, context_menu, gui());
}

void editor_controller::cycle_brush()
{
	if (brush_ == &brushes_.back()) {
		brush_ = &brushes_.front();
	} else {
		++brush_;
	}
	update_mouse_action_highlights();
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
	if (!get_map().selection().empty()) {
		clipboard_ = map_fragment(get_map(), get_map().selection());
		clipboard_.center_by_mass();
	}
}

void editor_controller::cut_selection()
{
	copy_selection();
	perform_refresh(editor_action_paint_area(get_map().selection(), background_terrain_));
}

void editor_controller::export_selection_coords()
{
	std::stringstream ssx, ssy;
	std::set<map_location>::const_iterator i = get_map().selection().begin();
	if (i != get_map().selection().end()) {
		ssx << "x = " << i->x + 1;
		ssy << "y = " << i->y + 1;
		++i;
		while (i != get_map().selection().end()) {
			ssx << ", " << i->x + 1;
			ssy << ", " << i->y + 1;
			++i;
		}
		ssx << "\n" << ssy.str() << "\n";
		copy_to_clipboard(ssx.str(), false);
	}
}

void editor_controller::fill_selection()
{
	perform_refresh(editor_action_paint_area(get_map().selection(), foreground_terrain_));
}

void editor_controller::hotkey_set_mouse_action(hotkey::HOTKEY_COMMAND command)
{
	std::map<hotkey::HOTKEY_COMMAND, mouse_action*>::iterator i = mouse_actions_.find(command);
	if (i != mouse_actions_.end()) {
		mouse_action_ = i->second;
		set_mouseover_overlay();
		redraw_toolbar();
		gui().set_report_content(reports::EDIT_LEFT_BUTTON_FUNCTION,
				hotkey::get_hotkey(command).get_description());
		gui().invalidate_game_status();
	} else {
		ERR_ED << "Invalid hotkey command ("
			<< static_cast<int>(command) << ") passed to set_mouse_action\n";
	}
}

bool editor_controller::is_mouse_action_set(hotkey::HOTKEY_COMMAND command) const
{
	std::map<hotkey::HOTKEY_COMMAND, mouse_action*>::const_iterator i = mouse_actions_.find(command);
	return (i != mouse_actions_.end()) && (i->second == mouse_action_);
}

void editor_controller::update_mouse_action_highlights()
{
	DBG_ED << __func__ << "\n";
	int x, y;
	SDL_GetMouseState(&x, &y);
	map_location hex_clicked = gui().hex_clicked_on(x,y);
	get_mouse_action()->update_brush_highlights(gui(), hex_clicked);
}


events::mouse_handler_base& editor_controller::get_mouse_handler_base()
{
	return *this;
}

editor_display& editor_controller::get_display()
{
	return *gui_;
}

brush* editor_controller::get_brush()
{
	return brush_;
}

mouse_action* editor_controller::get_mouse_action()
{
	return mouse_action_;
}

void editor_controller::perform_delete(editor_action* action)
{
	if (action) {
		std::auto_ptr<editor_action> action_auto(action);
		get_map_context().perform_action(*action);
	}
}

void editor_controller::perform_refresh_delete(editor_action* action, bool drag_part /* =false */)
{
	if (action) {
		std::auto_ptr<editor_action> action_auto(action);
		perform_refresh(*action, drag_part);
	}
}

void editor_controller::perform_refresh(const editor_action& action, bool drag_part /* =false */)
{
	get_map_context().perform_action(action);
	refresh_after_action(drag_part);
}

void editor_controller::redraw_toolbar()
{
	BOOST_FOREACH (mouse_action_map::value_type a, mouse_actions_) {
		if (a.second->toolbar_button() != NULL) {
			SDL_Rect r = a.second->toolbar_button()->location(gui().screen_area());
			SDL_Rect outline = {r.x - 2, r.y - 2, r.h + 4, r.w + 4};
			//outline = intersect_rects(r, gui().screen_area());
			SDL_Surface* const screen = gui().video().getSurface();
			Uint32 color;
			if (a.second == mouse_action_) {
				color = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
			} else {
				color = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
			}
			draw_rectangle(outline.x, outline.y, outline.w, outline.h, color, gui().video().getSurface());
			update_rect(outline);
		}
	}
	toolbar_dirty_ = false;
}

void editor_controller::refresh_image_cache()
{
	image::flush_cache();
	refresh_all();
}

void editor_controller::display_redraw_callback(display&)
{
	adjust_sizes(gui(), *size_specs_);
	palette_->adjust_size();
	brush_bar_->adjust_size();
	palette_->draw(true);
	brush_bar_->draw(true);
	//display::redraw_everything removes our custom tooltips so reload them
	load_tooltips();
	gui().invalidate_all();
}

void editor_controller::undo()
{
	get_map_context().undo();
	refresh_after_action();
}

void editor_controller::redo()
{
	get_map_context().redo();
	refresh_after_action();
}

void editor_controller::mouse_motion(int x, int y, const bool /*browse*/, bool update)
{
	if (mouse_handler_base::mouse_motion_default(x, y, update)) return;
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (get_map().on_board_with_border(drag_from_hex_) && is_dragging()) {
		editor_action* a = NULL;
		bool partial = false;
		editor_action* last_undo = get_map_context().last_undo_action();
		if (dragging_left_ && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1)) != 0) {
			if (!get_map().on_board_with_border(hex_clicked)) return;
			a = get_mouse_action()->drag_left(*gui_, x, y, partial, last_undo);
		} else if (dragging_right_ && (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(3)) != 0) {
			if (!get_map().on_board_with_border(hex_clicked)) return;
			a = get_mouse_action()->drag_right(*gui_, x, y, partial, last_undo);
		}
		//Partial means that the mouse action has modified the
		//last undo action and the controller shouldn't add
		//anything to the undo stack (hence a different
		//perform_ call)
		if (a != NULL) {
			std::auto_ptr<editor_action> aa(a);
			if (partial) {
				get_map_context().perform_partial_action(*a);
			} else {
				get_map_context().perform_action(*a);
			}
			refresh_after_action(true);
		}
	} else {
		get_mouse_action()->move(*gui_, hex_clicked);
	}
	gui().highlight_hex(hex_clicked);
}

bool editor_controller::allow_mouse_wheel_scroll(int x, int y)
{
	return get_map().on_board_with_border(gui().hex_clicked_on(x,y));
}

bool editor_controller::right_click_show_menu(int /*x*/, int /*y*/, const bool /*browse*/)
{
	return get_mouse_action()->has_context_menu();
}

bool editor_controller::left_click(int x, int y, const bool browse)
{
	clear_mouseover_overlay();
	if (mouse_handler_base::left_click(x, y, browse)) return true;
	LOG_ED << "Left click, after generic handling\n";
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (!get_map().on_board_with_border(hex_clicked)) return true;
	LOG_ED << "Left click action " << hex_clicked.x << " " << hex_clicked.y << "\n";
	editor_action* a = get_mouse_action()->click_left(*gui_, x, y);
	perform_refresh_delete(a, true);
	palette_->draw(true);
	return false;
}

void editor_controller::left_drag_end(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action()->drag_end(*gui_, x, y);
	perform_delete(a);
}

void editor_controller::left_mouse_up(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action()->up_left(*gui_, x, y);
	perform_delete(a);
	refresh_after_action();
	set_mouseover_overlay();
}

bool editor_controller::right_click(int x, int y, const bool browse)
{
	clear_mouseover_overlay();
	if (mouse_handler_base::right_click(x, y, browse)) return true;
	LOG_ED << "Right click, after generic handling\n";
	map_location hex_clicked = gui().hex_clicked_on(x, y);
	if (!get_map().on_board_with_border(hex_clicked)) return true;
	LOG_ED << "Right click action " << hex_clicked.x << " " << hex_clicked.y << "\n";
	editor_action* a = get_mouse_action()->click_right(*gui_, x, y);
	perform_refresh_delete(a, true);
	palette_->draw(true);
	return false;
}

void editor_controller::right_drag_end(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action()->drag_end(*gui_, x, y);
	perform_delete(a);
}

void editor_controller::right_mouse_up(int x, int y, const bool /*browse*/)
{
	editor_action* a = get_mouse_action()->up_right(*gui_, x, y);
	perform_delete(a);
	refresh_after_action();
	set_mouseover_overlay();
}

void editor_controller::process_keyup_event(const SDL_Event& event)
{
	editor_action* a = get_mouse_action()->key_event(gui(), event);
	perform_refresh_delete(a);
	set_mouseover_overlay();
}

void editor_controller::set_mouseover_overlay()
{
	get_mouse_action()->set_mouse_overlay(gui());
}

void editor_controller::clear_mouseover_overlay()
{
	gui().clear_mouseover_hex_overlay();
}

} //end namespace editor
