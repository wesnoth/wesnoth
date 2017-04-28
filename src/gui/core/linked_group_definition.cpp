/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "linked_group_definition.hpp"

#include "formula/string_utils.hpp"
#include "serialization/string_utils.hpp"
#include "wml_exception.hpp"

namespace gui2
{

std::vector<linked_group_definition> parse_linked_group_definitions(const config& cfg)
{
	std::vector<linked_group_definition> definitions;

	for(const auto & lg : cfg.child_range("linked_group")) {
		linked_group_definition linked_group;
		linked_group.id = lg["id"].str();
		linked_group.fixed_width = lg["fixed_width"].to_bool();
		linked_group.fixed_height = lg["fixed_height"].to_bool();

		VALIDATE(!linked_group.id.empty(),
			missing_mandatory_wml_key("linked_group", "id"));

		if(!(linked_group.fixed_width || linked_group.fixed_height)) {
			utils::string_map symbols;
			symbols["id"] = linked_group.id;
			t_string msg
				= vgettext("Linked '$id' group needs a 'fixed_width' or "
				"'fixed_height' key.",
				symbols);

			FAIL(msg);
		}

		definitions.push_back(linked_group);
	}

	return definitions;
}

}
