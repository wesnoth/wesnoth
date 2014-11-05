/*
   Copyright (C) 2008 - 2014 by Jörg Hinrichs <joerg.hinrichs@alice-dsl.de>
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

#include "gui/dialogs/addon/addon_review_write.hpp"

#include "gettext.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/settings.hpp"

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
 * lblTitle & & label & m &
 *         The title of the window. $
 *
 * gameplay_input & & text_box & m &
 *         The user's comment about the add-on's gameplay. $
 *
 * visuals_input & & text_box & m &
 *         The user's comment about the add-on's visuals. $
 *
 * story_input & & text_box & m &
 *         The user's comment about the add-on's storyline. $
 *
 * balance_input & & text_box & m &
 *         The user's comment about the add-on's balance. $
 *
 * overall_input & & text_box & m &
 *         The user's overall comment about the add-on.. $
 *
 * @end{table}
 */

REGISTER_DIALOG(addon_review_write)

taddon_review_write::taddon_review_write(addon_info::addon_review& review)
{
	register_text("gameplay_input", false, review.gameplay, true);
	register_text("visuals_input", false, review.visuals, true);
	register_text("story_input", false, review.story, true);
	register_text("balance_input", false, review.balance, true);
	register_text("overall_input", false, review.overall, true);
}

} // namespace gui2

