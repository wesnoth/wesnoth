/* $Id$ */
/*
 Copyright (C) 2012 by Fabian Mueller <fabianmueller5@gmx.de>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

/**
 *  @file
 *  Manages the hotkey bindings.
 */

#include "preferences_display.hpp"

#include "construct_dialog.hpp"
#include "display.hpp"
#include "formatter.hpp"
#include "formula_string_utils.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/simple_item_selector.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/window.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "marked-up_text.hpp"
#include "wml_separators.hpp"

#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>

namespace preferences {

class hotkey_preferences_parent_dialog;

class hotkey_preferences_dialog: public gui::preview_pane {

private	:
	/** nice little magic number, the size of the truncated hotkey command */
	static const int truncate_at = 25;

public:
	hotkey_preferences_dialog(display& disp);

	/**
	 * Populates, sorts and redraws the hotkey menu
	 * specified by tab_.
	 * @param keep_viewport feeds the keep_viewport param of menu::set_menu_items()
	 */
	void set_hotkey_menu(bool keep_viewport);

private:

	/// overrides events::handler::process_event
	void process_event();

	/// overrides gui::widget::update_location
	void update_location(SDL_Rect const &rect);

	/// overrides gui::preview_pane::handler_members
	virtual handler_vector handler_members();

	/// implements gui::preview_pane::set_selection
	void set_selection(int index);

	/// implements gui::preview_pane::left_side
	bool left_side() const {
		return false;
	}

	/**
	 * Sub-dialog, recognizing the hotkey sequence
	 * @param command add the new binding to this item
	 */
	void show_binding_dialog(const std::string& command);

	/**
	 * Buttons to trigger the tools involved in hotkey assignment.
	 * The buttons are shared by all scope tabs.
	 */
	gui::button add_button_, clear_button_;

	/** The dialog features a tab for each hotkey scope (except the SCOPE_COUNTER)*/
	hotkey::scope tab_;

	/**
	 * These are to map the menu selection to the corresponding command
	 * example: std::string selected_general_command = general_commands_[general_hotkey_.get_selected()];
	 */
	std::vector<std::string> general_commands_;
	std::vector<std::string> game_commands_;
	std::vector<std::string> editor_commands_;

	/** The header of all the scope menus */
	const std::string heading_;

	/**
	 * Every scope gets its own menu and sorter to allow keeping the viewport
	 * while switching through the tabs.
	 */
	int selected_command_;
	gui::menu::basic_sorter general_sorter_, game_sorter_, editor_sorter_;
	gui::menu general_hotkeys_, game_hotkeys_, editor_hotkeys_;

	/** The display, for usage by child dialogs */
	display &disp_;

public:
	/**
	 * @todo Okay, we have here a public member in a class.
	 * Like in game_preferences_dialog.
	 */
	util::scoped_ptr<hotkey_preferences_parent_dialog> parent;
};

class hotkey_resetter : public gui::dialog_button_action
{
public:
	hotkey_resetter(display& disp, hotkey_preferences_dialog& dialog) :
		disp_(disp),
		dialog_(dialog)
	{};

	// This method is called when the button is pressed.
	RESULT button_pressed(int /*selection*/)
	{
		clear_hotkeys();
		gui2::show_transient_message(disp_.video(), _("Hotkeys Reset"),
				_("All hotkeys have been reset to their default values."));
		dialog_.set_hotkey_menu(true);
		return gui::CONTINUE_DIALOG;
	};

private:
	display& disp_;
	hotkey_preferences_dialog& dialog_;
};

class hotkey_preferences_parent_dialog: public gui::dialog {

public:

	hotkey_preferences_parent_dialog(display &disp, hotkey_preferences_dialog& hotkey_preferences_dialog) :
			dialog(disp, _("Hotkey Settings"), "", gui::OK_CANCEL),
			clear_buttons_(false),
			hotkey_cfg_(),
			resetter_(disp, hotkey_preferences_dialog)
	{
		gui::dialog_button* reset_button = new gui::dialog_button(disp.video(), _("Reset Defaults"),
				gui::button::TYPE_PRESS, gui::CONTINUE_DIALOG, &resetter_);
		reset_button->set_help_string(_("Reset all bindings to the default values"));
		add_button(reset_button, dialog::BUTTON_HELP);

		// keep the old config in case the user cancels the dialog
		hotkey::save_hotkeys(hotkey_cfg_);
	}

	~hotkey_preferences_parent_dialog() {
		// save or restore depending on the exit result
		if (result() >= 0)
			save_hotkeys();
		else hotkey::load_hotkeys(hotkey_cfg_, false);
	}

	void action(gui::dialog_process_info &info) {
		if (clear_buttons_) {
			info.clear_buttons();
			clear_buttons_ = false;
		}
	}

	void clear_buttons() {
		clear_buttons_ = true;
	}

private:
	bool clear_buttons_;
	config hotkey_cfg_;
	hotkey_resetter resetter_;
};


void show_hotkeys_preferences_dialog(display& disp) {

	std::vector<std::string> items;
	const std::string pre = IMAGE_PREFIX + std::string("icons/icon-");
	char const sep = COLUMN_SEPARATOR;

	// tab names and icons
	items.push_back(pre + "general.png" + sep + sgettext("Prefs section^General"));
	items.push_back(pre + "game.png"    + sep + sgettext("Prefs section^Game"));
	items.push_back(pre + "editor.png"  + sep + sgettext("Prefs section^Editor"));

	// determine the current scope, but not general
	int scope;
	for (scope = 1; scope != hotkey::SCOPE_COUNT; scope++)
		if (hotkey::is_scope_active(static_cast<hotkey::scope>(scope)))
			break;

	// The restorer will change the scope back to where we came from
	// when it runs out of the function's scope
	hotkey::scope_changer scope_restorer;
	hotkey_preferences_dialog dialog(disp);
	dialog.parent.assign(new hotkey_preferences_parent_dialog(disp, dialog));
	dialog.parent->set_menu(items);
	dialog.parent->add_pane(&dialog);
	// select the tab corresponding to the current scope.
	dialog.parent->get_menu().move_selection(scope);
	dialog.parent->show();
	return;
}


/* hotkey_preferences_dialog members ************************/

hotkey_preferences_dialog::hotkey_preferences_dialog(display& disp) :
		gui::preview_pane(disp.video()),
		add_button_(disp.video(), _("Add Hotkey")),
		clear_button_(disp.video(),	_("Clear Hotkey")),
		tab_(hotkey::SCOPE_GENERAL),
		general_commands_(),
		game_commands_(),
		editor_commands_(),
		heading_( (formatter() << HEADING_PREFIX << _("Action")
						<< COLUMN_SEPARATOR << _("Binding")).str()),
		selected_command_(0),
		general_sorter_(),
		game_sorter_(),
		editor_sorter_(),
		// Note: If you don't instantiate the menus with heading_,
		// the header can't be enabled later, seems to be a bug in gui::menu
		general_hotkeys_(disp.video(), boost::assign::list_of(heading_), false, -1, -1,
				&general_sorter_, &gui::menu::bluebg_style),
		game_hotkeys_(disp.video(), boost::assign::list_of(heading_), false, -1, -1,
				&game_sorter_, &gui::menu::bluebg_style),
		editor_hotkeys_(disp.video(), boost::assign::list_of(heading_), false, -1, -1,
						&editor_sorter_, &gui::menu::bluebg_style),
		disp_(disp),
		parent(NULL)
{

	set_measurements(preferences::width, preferences::height);

	// Populate the command vectors, this needs to happen only once.
	const hotkey::hotkey_command* list = hotkey::get_hotkey_commands();
	for (size_t i = 0; list[i].id != hotkey::HOTKEY_NULL; ++i) {

		if (list[i].hidden)
			continue;

		switch (list[i].scope) {

		case hotkey::SCOPE_GAME:
			game_commands_.push_back(list[i].command);
			break;
		case hotkey::SCOPE_EDITOR:
			editor_commands_.push_back(list[i].command);
			break;
		case hotkey::SCOPE_GENERAL:
			general_commands_.push_back(list[i].command);
			break;
		case hotkey::SCOPE_COUNT:
			break;
		}
	}

	///@todo move to the caller?
	disp_.video().clear_all_help_strings();

	//Add help tooltips to the buttons
	///@todo adjust them corresponding to the selected item
	clear_button_.set_help_string(_("Clears the selected actions's bindings"));
	add_button_.set_help_string(_("Add additional binding to the selected action"));

	// Initialize sorters.
	general_sorter_.set_alpha_sort(0).set_alpha_sort(1);
	game_sorter_.set_alpha_sort(0).set_alpha_sort(1);
	editor_sorter_.set_alpha_sort(0).set_alpha_sort(1);

	// Populate every menu_
	for (int scope = 0; scope != hotkey::SCOPE_COUNT; scope++) {
		tab_ = hotkey::scope(scope);
		set_hotkey_menu(false);
	}

}

void hotkey_preferences_dialog::set_hotkey_menu(bool keep_viewport) {

	/*
	 * First hide all elements of all tabs,
	 * we might have redrawing glitches otherwise.
	 */
	add_button_.hide(true);
	clear_button_.hide(true);
	game_hotkeys_.hide(true);
	editor_hotkeys_.hide(true);
	general_hotkeys_.hide(true);

	// Helpers to keep the switch statement small.
	gui::menu* active_hotkeys = NULL;
	std::vector<std::string>* commands = NULL;

	// Determine the menu corresponding to the selected tab.
	switch (tab_) {
	case hotkey::SCOPE_GAME:
		active_hotkeys = &game_hotkeys_;
		commands = &game_commands_;
		break;
	case hotkey::SCOPE_EDITOR:
		active_hotkeys = &editor_hotkeys_;
		commands = &editor_commands_;
		break;
	case hotkey::SCOPE_GENERAL:
		active_hotkeys = &general_hotkeys_;
		commands = &general_commands_;
		break;
	case hotkey::SCOPE_COUNT:
		assert(false); // should not happen.
		break;
	}

	// Fill the menu rows
	std::vector<std::string> menu_items;
	BOOST_FOREACH(const std::string& command, *commands) {

		const std::string& description = hotkey::get_description(command);
		std::string truncated_description = description;
		if (truncated_description.size() >= (truncate_at + 2) )
			utils::ellipsis_truncate(truncated_description, truncate_at);
		const std::string& name = hotkey::get_names(hotkey::get_id(command));

		menu_items.push_back(
				(formatter() << truncated_description << HELP_STRING_SEPARATOR << description << COLUMN_SEPARATOR
						<< font::NULL_MARKUP << name << HELP_STRING_SEPARATOR << name).str() );
	}
	// No effect if the vector is used in set_items instead of the menu's constructor.
    // menu_items.push_back(heading_);

	// Re-populate the items.
	// workaround for set_items not keeping the selected item.
	selected_command_ = active_hotkeys->selection();
	active_hotkeys->set_items(menu_items, true, keep_viewport);
	// This shall happen only once at the start of the dialog.
	if (!keep_viewport) {
		active_hotkeys->sort_by(0);
		active_hotkeys->reset_selection();
	} else {
		active_hotkeys->move_selection_keeping_viewport(selected_command_);
	    // !hide and thus redraw only the current tab_'s items
		active_hotkeys->hide(false);
		add_button_.hide(false);
		clear_button_.hide(false);
	}
}

handler_vector hotkey_preferences_dialog::handler_members() {
	handler_vector h;
	h.push_back(&add_button_);
	h.push_back(&clear_button_);
	h.push_back(&general_hotkeys_);
	h.push_back(&game_hotkeys_);
	h.push_back(&editor_hotkeys_);
	return h;
}

void hotkey_preferences_dialog::set_selection(int index) {

	tab_ = hotkey::scope(index);

	set_dirty();
	bg_restore();

	hotkey::deactivate_all_scopes();
	switch (tab_) {
	case hotkey::SCOPE_GENERAL:
		hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
		hotkey::set_scope_active(hotkey::SCOPE_GAME);
		hotkey::set_scope_active(hotkey::SCOPE_EDITOR);
		break;
	case hotkey::SCOPE_GAME:
		hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
		hotkey::set_scope_active(hotkey::SCOPE_GAME);
		break;
	case hotkey::SCOPE_EDITOR:
		hotkey::set_scope_active(hotkey::SCOPE_GENERAL);
		hotkey::set_scope_active(hotkey::SCOPE_EDITOR);
		break;
	case hotkey::SCOPE_COUNT:
		break;
	}
	set_hotkey_menu(true);
}

void hotkey_preferences_dialog::process_event() {

	std::string id;
	gui::menu* active_menu_ = NULL;
	switch (tab_) {
	case hotkey::SCOPE_GAME:
		id = game_commands_[game_hotkeys_.selection()];
		active_menu_ = &game_hotkeys_;
		break;
	case hotkey::SCOPE_EDITOR:
		id = editor_commands_[editor_hotkeys_.selection()];
		active_menu_ = &editor_hotkeys_;
		break;
	case hotkey::SCOPE_GENERAL:
		id = general_commands_[general_hotkeys_.selection()];
		active_menu_ = &general_hotkeys_;
		break;
	case hotkey::SCOPE_COUNT:
		break;
	}

	if ( selected_command_ != active_menu_->selection()) {
		selected_command_ = active_menu_->selection();
	}

	if (add_button_.pressed() || active_menu_->double_clicked()) {
		show_binding_dialog(id);
	}

	if (clear_button_.pressed()) {
		// clear hotkey
		hotkey::clear_hotkeys(id);
		set_hotkey_menu(true);
	}

}

void hotkey_preferences_dialog::update_location(SDL_Rect const &rect) {

	bg_register(rect);

	/** some magic numbers :-P
	@TODO they match the ones in game_preferences_dialog. */
	const int top_border = 10;
	const int right_border = font::relative_size(10);
	const int h = height() - 75; // well, this one not.
	const int w = rect.w - right_border;

	int xpos = rect.x;
	int ypos = rect.y + top_border;

	general_hotkeys_.set_location(xpos, ypos);
	general_hotkeys_.set_max_height(h);
	general_hotkeys_.set_height(h);
	general_hotkeys_.set_max_width(w);
	general_hotkeys_.set_width(w);

	game_hotkeys_.set_location(xpos, ypos);
	game_hotkeys_.set_max_height(h);
	game_hotkeys_.set_height(h);
	game_hotkeys_.set_max_width(w);
	game_hotkeys_.set_width(w);

	editor_hotkeys_.set_location(xpos, ypos);
	editor_hotkeys_.set_max_height(h);
	editor_hotkeys_.set_height(h);
	editor_hotkeys_.set_max_width(w);
	editor_hotkeys_.set_width(w);

	ypos += h + font::relative_size(14);

	add_button_.set_location(xpos, ypos);
	xpos += add_button_.width() + font::relative_size(14);
	clear_button_.set_location(xpos, ypos);
}

#ifdef _MSC_VER
#pragma warning (push)
#pragma warning (disable:4701)
#endif
void hotkey_preferences_dialog::show_binding_dialog(const std::string& id) {

	// Lets bind a hotkey......

	SDL_Rect clip_rect = create_rect(0, 0, disp_.w(), disp_.h());
	SDL_Rect text_size = font::draw_text(NULL, clip_rect, font::SIZE_LARGE,
			font::NORMAL_COLOR, _("Press desired hotkey (Esc cancels)"), 0,
			0);

	const int centerx = disp_.w() / 2;
	const int centery = disp_.h() / 2;

	SDL_Rect dlgr = create_rect(centerx - text_size.w / 2 - 30,
			centery - text_size.h / 2 - 16, text_size.w + 60,
			text_size.h + 32);

	surface_restorer restorer(&disp_.video(), dlgr);
	gui::dialog_frame mini_frame(disp_.video());
	mini_frame.layout(centerx - text_size.w / 2 - 20,
			centery - text_size.h / 2 - 6, text_size.w + 40,
			text_size.h + 12);
	mini_frame.draw_background();
	mini_frame.draw_border();
	font::draw_text(&disp_.video(), clip_rect, font::SIZE_LARGE,
			font::NORMAL_COLOR, _("Press desired hotkey (Esc cancels)"),
			centerx - text_size.w / 2, centery - text_size.h / 2);
	disp_.update_display();
	SDL_Event event;
	event.type = 0;
	int character = 0, keycode = 0, mod = 0; // Just to avoid warning
	int device = 0, button = 0, hat = 0, value = 0;
	const int any_mod = KMOD_CTRL | KMOD_ALT | KMOD_LMETA;

	while (event.type != SDL_KEYDOWN && event.type != SDL_JOYBUTTONDOWN
			&& event.type != SDL_JOYHATMOTION
			&& (event.type != SDL_MOUSEBUTTONDOWN || event.button.button < 4))
		SDL_PollEvent(&event);

	do {
		switch (event.type) {

		case SDL_KEYDOWN:
			keycode = event.key.keysym.sym;
			character = event.key.keysym.unicode;
			mod = event.key.keysym.mod;
			break;
		case SDL_JOYBUTTONDOWN:
			device = event.jbutton.which;
			button = event.jbutton.button;
			break;
		case SDL_JOYHATMOTION:
			device = event.jhat.which;
			hat = event.jhat.hat;
			value = event.jhat.value;
			break;
		case SDL_MOUSEBUTTONDOWN:
			device = event.button.which;
			button = event.button.button;
			break;
		}

		SDL_PollEvent(&event);
		disp_.flip();
		disp_.delay(10);
	} while (event.type != SDL_KEYUP && event.type != SDL_JOYBUTTONUP
			&& event.type != SDL_JOYHATMOTION
			&& event.type != SDL_MOUSEBUTTONUP);

	restorer.restore();
	disp_.update_display();

	// only if not canceled.
	if (!(keycode == SDLK_ESCAPE && (mod & any_mod) == 0)) {

		hotkey::hotkey_item newhk(id);
		hotkey::hotkey_item* oldhk = NULL;

		Uint8 *keystate = SDL_GetKeyState(NULL);
		bool alt = keystate[SDLK_RALT] || keystate[SDLK_LALT];
		bool ctrl = keystate[SDLK_RCTRL] || keystate[SDLK_LCTRL];
		bool shift = keystate[SDLK_RSHIFT] || keystate[SDLK_LSHIFT];
		bool cmd = keystate[SDLK_RMETA] || keystate[SDLK_LMETA];

		switch (event.type) {

		case SDL_JOYHATMOTION:
			oldhk = &hotkey::get_hotkey(hotkey::hotkey_item::JHAT, device,
					hat, value, shift, ctrl, alt, cmd);
			newhk.set_jhat(device, hat, value, shift, ctrl, alt, cmd);
			break;
		case SDL_JOYBUTTONUP:
			oldhk = &hotkey::get_hotkey(hotkey::hotkey_item::JBUTTON,
					device, button, 0, shift, ctrl, alt, cmd);
			newhk.set_jbutton(device, button, shift, ctrl, alt, cmd);
			break;
		case SDL_MOUSEBUTTONUP:
			oldhk = &hotkey::get_hotkey(hotkey::hotkey_item::MBUTTON,
					device, button, 0, shift, ctrl, alt, cmd);
			newhk.set_mbutton(device, button, shift, ctrl, alt, cmd);
			break;
		case SDL_KEYUP:
			oldhk =
					&hotkey::get_hotkey(character, keycode,
							(mod & KMOD_SHIFT)!= 0,
							(mod & KMOD_CTRL) != 0, (mod & KMOD_ALT) != 0, (mod & KMOD_LMETA) != 0);newhk.set_key(character, keycode, (mod & KMOD_SHIFT) != 0,
									(mod & KMOD_CTRL) != 0, (mod & KMOD_ALT) != 0, (mod & KMOD_LMETA) != 0);

							if ( (newhk.get_id() == hotkey::HOTKEY_SCREENSHOT
									|| newhk.get_id() == hotkey::HOTKEY_MAP_SCREENSHOT)
									&& (mod & any_mod) == 0 ) {
								gui2::show_transient_message(disp_.video(), _("Warning"),
										_("Screenshot hotkeys should be combined with the Control, Alt or Meta modifiers to avoid problems."));
							}
							break;
		}

		if ((oldhk && (!(oldhk->null())))) {
			if (oldhk->get_command() != id) {

				utils::string_map symbols;
				symbols["hotkey_sequence"] = oldhk->get_name();
				symbols["old_hotkey_action"] = oldhk->get_description();
				symbols["new_hotkey_action"] = newhk.get_description();

				std::string text =
						vgettext("\"$hotkey_sequence|\" is in use by \"$old_hotkey_action|\". Do you wish to reassign it to \"$new_hotkey_action|\"?",
								symbols);

				const int res = gui2::show_message(disp_.video(),
						_("Reassign Hotkey"), text,
						gui2::tmessage::yes_no_buttons);
				if (res == gui2::twindow::OK) {
					oldhk->set_command(id);
					set_hotkey_menu(true);
				}
			}
		} else {
			hotkey::add_hotkey(newhk);
			set_hotkey_menu(true);
		}
	}

}
#ifdef _MSC_VER
#pragma warning (pop)
#endif

} // end namespace preferences

