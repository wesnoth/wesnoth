/*
   Copyright (C) 2014 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "gui/dialogs/wml_error.hpp"

#include "gui/widgets/settings.hpp"

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_wml_error
 *
 * == WML error ==
 *
 * Dialog used to report WML parser or preprocessor errors.
 *
 * @begin{table}{dialog_widgets}
 *
 * summary & & control & m &
 *         Label used for displaying a brief summary of the error(s). $
 *
 * details & & control & m &
 *         Full report of the parser or preprocessor error(s) found. $
 *
 * @end{table}
 */

REGISTER_DIALOG(wml_error)

twml_error::twml_error(const std::string& summary, const std::string& details)
{
	register_label("summary", true, summary);
	register_label("details", true, details);
}

} // end namespace gui2
