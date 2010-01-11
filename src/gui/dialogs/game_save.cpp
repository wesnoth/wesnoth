/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/game_save.hpp"

#include "foreach.hpp"
#include "gettext.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_game_save
 *
 * == Save a game ==
 *
 * This shows the dialog to create a savegame file.
 *
 * @start_table = grid
 *     (lblTitle) (label) ()      The title of the window.
 *     (txtFilename) (text_box) ()
 *                                The name of the savefile.
 * @end_table
 */
tgame_save::tgame_save(const std::string& title, const std::string& filename) :
	txtFilename_(register_text("txtFilename", false)),
	title_(title),
	filename_(filename)
{
}

twindow* tgame_save::build_window(CVideo& video)
{
	return build(video, get_id(GAME_SAVE));
}

void tgame_save::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(txtFilename_);

	find_widget<tlabel>(&window, "lblTitle", false).set_label(title_);

	txtFilename_->set_widget_value(window, filename_);
	window.keyboard_capture(txtFilename_->widget(window));
}

void tgame_save::post_show(twindow& window)
{
	filename_ = txtFilename_->get_widget_value(window);
}

tgame_save_message::tgame_save_message(const std::string& title, const std::string& filename, const std::string& message)
	: tgame_save(title, filename),
	message_(message)
{}

twindow* tgame_save_message::build_window(CVideo& video)
{
	return build(video, get_id(GAME_SAVE_MESSAGE));
}

void tgame_save_message::pre_show(CVideo& video, twindow& window)
{
	find_widget<tlabel>(&window, "lblMessage", false).set_label(message_);

	tgame_save::pre_show(video, window);
}

tgame_save_oos::tgame_save_oos(const std::string& title, const std::string& filename, const std::string& message)
	: tgame_save_message(title, filename, message),
	btnIgnoreAll_(register_bool("ignore_all", false)),
	ignore_all_(false)
{}

twindow* tgame_save_oos::build_window(CVideo& video)
{
	return build(video, get_id(GAME_SAVE_OOS));
}

void tgame_save_oos::post_show(twindow& window)
{
	tgame_save::post_show(window);

	ignore_all_ = btnIgnoreAll_->get_widget_value(window);
}

} // namespace gui2

