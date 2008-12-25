/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/dialogs/title_screen.hpp"

#include "game_config.hpp"
#include "gettext.hpp"
#include "gui/dialogs/addon_connect.hpp"
#include "gui/dialogs/language_selection.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/window.hpp"

#define ERR_CONFIG LOG_STREAM(err, config)

namespace gui2 {

namespace {

template<class D>
void show_dialog(twidget* caller)
{
	ttitle_screen *dialog =  dynamic_cast<ttitle_screen*>(caller->dialog());
	assert(dialog);

	D dlg;
	dlg.show(*(dialog->video()));
}

} // namespace

/*WIKI
 * @page = GUIWindowWML
 * @order = 2_title_screen
 *
 * == Title screen ==
 *
 * This shows the title screen.
 */

twindow* ttitle_screen::build_window(CVideo& video)
{
	return build(video, get_id(TITLE_SCREEN));
}

void ttitle_screen::pre_show(CVideo& video, twindow& window)
{
	assert(!video_);
	video_ = &video;

	set_restore(false);

	// Note changing the language doesn't upate the title screen...
	window.get_widget<tbutton>("language", false).
		set_callback_mouse_left_click( show_dialog<gui2::tlanguage_selection>);

	window.get_widget<tbutton>("addons", false).
		set_callback_mouse_left_click( show_dialog<gui2::taddon_connect>);

	window.canvas()[0].set_variable("revision_number", 
		variant(_("Version") + std::string(" ") + game_config::revision));

	/*Select a random game_title*/
	std::vector<std::string> game_title_list =
		utils::split(game_config::game_title, ',', utils::STRIP_SPACES | utils::REMOVE_EMPTY);

	if(game_title_list.empty()) {
		ERR_CONFIG << "No title image defined\n";
	} else {
		window.canvas()[0].set_variable("background_image", 
			variant(game_title_list[rand()%game_title_list.size()]));
	}
}

void ttitle_screen::post_show(twindow& /*window*/)
{
	video_ = NULL;
}

} // namespace gui2
