/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/image.hpp"

#include "../../image.hpp" // We want the file in src/

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(image)

tpoint timage::calculate_best_size() const
{
	surface image(image::get_image(image::locator(label())));

	if(!image) {
		DBG_GUI_L << LOG_HEADER << " empty image return default.\n";
		return get_config_default_size();
	}

	const tpoint minimum = get_config_default_size();
	const tpoint maximum = get_config_maximum_size();

	tpoint result = tpoint(image->w, image->h);

	if(minimum.x > 0 && result.x < minimum.x) {
		DBG_GUI_L << LOG_HEADER << " increase width to minimum.\n";
		result.x = minimum.x;
	} else if(maximum.x > 0 && result.x > maximum.x) {
		DBG_GUI_L << LOG_HEADER << " decrease width to maximum.\n";
		result.x = maximum.x;
	}

	if(minimum.y > 0 && result.y < minimum.y) {
		DBG_GUI_L << LOG_HEADER << " increase height to minimum.\n";
		result.y = minimum.y;
	} else if(maximum.y > 0 && result.y > maximum.y) {
		DBG_GUI_L << LOG_HEADER << " decrease height to maximum.\n";
		result.y = maximum.y;
	}

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

void timage::set_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool timage::get_active() const
{
	return true;
}

unsigned timage::get_state() const
{
	return ENABLED;
}

bool timage::disable_click_dismiss() const
{
	return false;
}

const std::string& timage::get_control_type() const
{
	static const std::string type = "image";
	return type;
}

// }---------- DEFINITION ---------{

timage_definition::timage_definition(const config& cfg)
	: tcontrol_definition(cfg)
{
	DBG_GUI_P << "Parsing image " << id << '\n';

	load_resolutions<tresolution>(cfg);
}

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_image
 *
 * == Image ==
 *
 * @macro = image_description
 *
 * The definition of an image. The label field of the widget is used as the
 * name of file to show. The widget normally has no event interaction so only
 * one state is defined.
 *
 * The following states exist:
 * * state_enabled, the image is enabled.
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="image_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="generic/widget_definition/resolution"}
 * @begin{tag}{name="state_enabled"}{min=0}{max=1}{super="generic/state"}
 * @end{tag}{name="state_enabled"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="image_definition"}
 * @end{parent}{name="gui/"}
 */
timage_definition::tresolution::tresolution(const config& cfg)
	: tresolution_definition_(cfg)
{
	// Note the order should be the same as the enum tstate in image.hpp.
	state.push_back(tstate_definition(cfg.child("state_enabled")));
}

// }---------- BUILDER -----------{

/*WIKI_MACRO
 * @begin{macro}{image_description}
 * An image shows a static image.
 * @end{macro}
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_image
 *
 * == Image ==
 *
 * @macro = image_description
 *
 * An image has no extra fields.
 * @begin{parent}{name="gui/window/resolution/grid/row/column/"}
 * @begin{tag}{name="image"}{min=0}{max=-1}{super="generic/widget_instance"}
 * @end{tag}{name="image"}
 * @end{parent}{name="gui/window/resolution/grid/row/column/"}
 */

namespace implementation
{

tbuilder_image::tbuilder_image(const config& cfg) : tbuilder_control(cfg)
{
}

twidget* tbuilder_image::build() const
{
	timage* widget = new timage();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed image '" << id << "' with definition '"
			  << definition << "'.\n";

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
