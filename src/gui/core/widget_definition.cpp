/*
	Copyright (C) 2007 - 2022
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/core/widget_definition.hpp"

#include "gettext.hpp"
#include "gui/core/log.hpp"
#include "gui/widgets/helper.hpp"
#include "wml_exception.hpp"

namespace gui2
{

state_definition::state_definition(const config& cfg)
	: canvas_cfg_(cfg ? cfg.child("draw") : cfg)
{
	VALIDATE(canvas_cfg_, _("No state or draw section defined."));
}

resolution_definition::resolution_definition(const config& cfg)
	: window_width(cfg["window_width"])
	, window_height(cfg["window_height"])
	, min_width(cfg["min_width"])
	, min_height(cfg["min_height"])
	, default_width(cfg["default_width"])
	, default_height(cfg["default_height"])
	, max_width(cfg["max_width"])
	, max_height(cfg["max_height"])
	, linked_groups()
	, text_extra_width(cfg["text_extra_width"])
	, text_extra_height(cfg["text_extra_height"])
	, text_font_size(cfg["text_font_size"])
	, text_font_family(font::str_to_family_class(cfg["text_font_family"]))
	, text_font_style(decode_font_style(cfg["text_font_style"]))
	, state()
{
	DBG_GUI_P << "Parsing resolution " << window_width << ", " << window_height;

	linked_groups = parse_linked_group_definitions(cfg);
}

styled_widget_definition::styled_widget_definition(const config& cfg)
	: id(cfg["id"]), description(cfg["description"].t_str()), resolutions()
{
	VALIDATE(!id.empty(), missing_mandatory_wml_key("styled_widget", "id"));
	VALIDATE(!description.empty(),
			 missing_mandatory_wml_key("styled_widget", "description"));

	/*
	 * Do this validation here instead of in load_resolutions so the
	 * translatable string is not in the header and we don't need to pull in
	 * extra header dependencies.
	 */
	config::const_child_itors itors = cfg.child_range("resolution");
	VALIDATE(!itors.empty(), _("No resolution defined."));
}

} // namespace gui2
