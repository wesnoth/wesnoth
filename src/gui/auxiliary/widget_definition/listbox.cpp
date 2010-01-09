/* $Id$ */
/*
   Copyright (C) 2007 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/widget_definition/listbox.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/log.hpp"
#include "wml_exception.hpp"

namespace gui2 {

tlistbox_definition::tlistbox_definition(const config& cfg) :
	tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing listbox " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

tlistbox_definition::tresolution::tresolution(const config& cfg) :
	tresolution_definition_(cfg),
	grid(NULL)
{
/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_listbox
 *
 * == Listbox ==
 *
 * @macro = listbox_description
 *
 * The definition of a listbox contains the definition of it's scrollbar.
 *
 * The resolution for a listbox also contains the following keys:
 * @start_table = config
 *     scrollbar (section)             A grid containing the widgets for the
 *                                     scrollbar. The scrollbar has some special
 *                                     widgets so it can make default behaviour
 *                                     for certain widgets.
 * @end_table
 *
 * @start_table = container
 *     [_begin] (clickable)            Moves the position to the beginning of
 *                                     the list.
 *     [_line_up] (clickable)          Move the position one item up. (NOTE if
 *                                     too many items to move per item it might
 *                                     be more items.)
 *     [_half_page_up] (clickable)     Move the position half the number of the
 *                                     visible items up. (See note at _line_up.)
 *     [_page_up] (clickable)          Move the position the number of visible
 *                                     items up. (See note at _line_up.)
 *
 *
 *     [_end] (clickable)              Moves the position to the end of the
 *                                     list.
 *     [_line_down] (clickable)        Move the position one item down.(See note
 *                                     at _line_up.)
 *     [_half_page_down] (clickable)   Move the position half the number of the
 *                                     visible items down. (See note at _line_up.)
 *     [_page_down] (clickable)         Move the position the number of visible
 *                                     items down. (See note at _line_up.)
 *
 *     _scrollbar (vertical_scrollbar) This is the scrollbar so the user can
 *                                     scroll through the list.
 * @end_table
 *
 * A clickable is one of:
 * * button
 * * repeating_button
 *
 * The following states exist:
 * * state_enabled, the listbox is enabled.
 * * state_disabled, the listbox is disabled.
 *
 */

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_horizonal_listbox
 *
 * == Horizontal listbox ==
 *
 * @macro = horizontal_listbox_description
 *
 * The definition of a horizontal listbox is the same as for a normal listbox.
 */
	// Note the order should be the same as the enum tstate is listbox.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
	state.push_back(tstate_definition(cfg.child("state_disabled")));

	const config &child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = new tbuilder_grid(child);
}

} // namespace gui2

