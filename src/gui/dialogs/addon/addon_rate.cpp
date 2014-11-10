/*
   Copyright (C) 2008 - 2014 by Ján Dugáček
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

#include "gui/dialogs/addon/addon_rate.hpp"

#include "gettext.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/slider.hpp"

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_rating
 *
 * == Rate an add-on ==
 *
 * This shows the dialog to insert user rating.
 *
 * @begin{table}{dialog_widgets}
 *
 * title & & label & m &
 *         The title of the window. $
 *
 * rating_input & & text_box & m &
 *         The user's rating. $
 *
 * @end{table}
 */

REGISTER_DIALOG(addon_rate)

taddon_rate::taddon_rate(int& input)
{

	register_integer("rating_input", true, input );
}

void taddon_rate::pre_show(CVideo& /*video*/, twindow& window)
{
	tslider& slider = find_widget<tslider>(&window, "rating_input", false);
}

} // namespace gui2

