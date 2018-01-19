/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/editor/new_map.hpp"

#include "gui/widgets/settings.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_editor_new_map
 *
 * == Editor new map ==
 *
 * This shows the dialog to generate a new map in the editor.
 *
 * @begin{table}{dialog_widgets}
 *
 * width & & integer_selector & m &
 *        An integer selector to determine the width of the map to create. $
 *
 * height & & integer_selector & m &
 *        An integer selector to determine the height of the map to create. $
 *
 * @end{table}
 */

REGISTER_DIALOG(editor_new_map)

editor_new_map::editor_new_map(const t_string& title, int& width, int& height)
{
	register_label("title", true, title);

	register_integer("width", true, width);
	register_integer("height", true, height);
}

} // namespace dialogs
} // namespace gui2
