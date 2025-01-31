/*
	Copyright (C) 2012 - 2024
	by Fabian Mueller <fabianmueller5@gmx.de>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

/**
 * Manage the unit-palette in the editor.
 */

#pragma once

#include "editor/palette/editor_palettes.hpp"

#include "units/types.hpp"

namespace editor {

class editor_toolkit;

//std::string get_selected_terrain();

/** Palette where the terrain to be drawn can be selected. */
class unit_palette : public editor_palette<const unit_type&> {
public:

	unit_palette(editor_display &gui, editor_toolkit &toolkit);

	virtual void setup(const game_config_view& cfg) override;

	virtual std::string get_help_string() const override;

	virtual bool supports_swap() override { return false; }

	const std::set<std::string>& get_selected_bg_items() { return selected_bg_items_; }

private:
	virtual const std::string& get_id(const unit_type& terrain) override;

	virtual void setup_item(
		const unit_type& item,
		texture& item_base_image,
		texture& item_overlay_image,
		std::stringstream& tooltip
	) override;

	virtual bool is_selected_bg_item(const std::string& id) override;

	virtual void select_bg_item(const std::string& item_id) override;

	std::set<std::string> selected_bg_items_;

};

}
