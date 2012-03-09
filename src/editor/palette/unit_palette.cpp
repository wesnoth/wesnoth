//unit_palette::unit_palette(editor_display &gui, const size_specs &sizes,
//								 const config& cfg, unit_type& fore, unit_type& back)
//	: editor_palette<unit_type, std::string>(gui, sizes, cfg, fore, back)
//{
//	//	// Get the available unit groups and add them to the structure
//	//	std::set<std::string> unit_group_names;
//	//	foreach (const unit_type_data::unit_type_map::value_type &i, unit_types.types())
//	//	{
//	//		if (unit_group_names.find(i.second.race()) == unit_group_names.end()) {
//	//			unit_groups_.push_back()
//	//			unit_group_names.insert(i.second.race());
//	//		}
//	//		//ERR_ED << i.first;
//	//		//std::string race_label;
//	//		//if (const unit_race *r = unit_types.find_race(i.second.race())) {
//	//		//	race_label = r->plural_name();
//	//		//}
//	//	}
//}

/* $Id: editor_palettes.cpp -1   $ */
/*
  Copyright (C) 2003 - 2012 by David White <dave@whitevine.net>
  Part of the Battle for Wesnoth Project http://www.wesnoth.org/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY.

  See the COPYING file for more details.
*/

/**
 * Manage the terrain-palette in the editor.
 * Note: this is a near-straight rip from the old editor.
 */

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "../editor_common.hpp"
#include "terrain_palettes.hpp"

#include "editor_palettes.hpp"

#include "../../foreach.hpp"
#include "../../gettext.hpp"
#include "../../serialization/string_utils.hpp"
#include "../../sound.hpp"
#include "../../tooltips.hpp"
#include "../../marked-up_text.hpp"

namespace {
	static std::string selected_terrain;
}

namespace editor {

std::string get_selected_terrain()
{
	return selected_terrain;
}

static bool is_valid_terrain(t_translation::t_terrain c) {
	return !(c == t_translation::VOID_TERRAIN || c == t_translation::FOGGED);
}

/* TODO find a solution where this is not needed. */
terrain_group::terrain_group():
		id(),
		name(),
		icon(),
		core(false)
		{}

terrain_group::terrain_group(const config& cfg):
	id(cfg["id"]), name(cfg["name"].t_str()),
	icon(cfg["icon"]), core(cfg["core"].to_bool())
{
}

void terrain_palette::update_report()
{
//	std::ostringstream msg;
//	msg << _("FG: ") << get_terrain_string(selected_fg_terrain())
//		<< '\n' << _("BG: ") << get_terrain_string(selected_bg_terrain());
//	selected_terrain = msg.str();
}

void terrain_palette::setup(const config& cfg)
{
	// items füllen
	// Get the available terrains temporary in terrains_
	t_translation::t_list items = map().get_terrain_list();

	ERR_ED << "grosse von items_ nach dem get_terrain_list:" << items.size() << "\n";

	//move "invalid" items to the end
	std::stable_partition(items.begin(), items.end(), is_valid_terrain);

	ERR_ED << "grosse von items_ nach dem sortieren:" << items.size() << "\n";

	// Get the available groups and add them to the structure
	std::set<std::string> group_names;
	foreach (const config &g, cfg.child_range("editor_group"))
	{
		if (group_names.find(g["id"]) == group_names.end()) {
			groups_.push_back(terrain_group(g));
			group_names.insert(groups_.back().id);
		}
	}

	std::map<std::string, terrain_group*> id_to_group;
	foreach (terrain_group& group, groups_) {
		id_to_group.insert(std::make_pair(group.id, &group));
	}

	ERR_ED << "grosse von items nach dem group setup:" << items.size() << "\n";

	// add the groups for all terrains to the map
	foreach (const t_translation::t_terrain& t, items) {

		// Terrain specific stuff
		const terrain_type& t_info = map().get_terrain_info(t);
		DBG_ED << "Palette: processing terrain " << t_info.name()
			<< "(editor name: '" << t_info.editor_name() << "') "
			<< "(" << t_info.number() << ")"
			<< ": " << t_info.editor_group() << "\n";

		// don't display terrains that were automatically created from base+overlay
		if (t_info.is_combined()) continue;
		// nor display terrains that have hide_in_editor=true
		if (t_info.hide_in_editor()) continue;

		// add the terrain to the requested groups
		const std::vector<std::string>& keys = utils::split(t_info.editor_group());
		bool core = false;

		ERR_ED << "grosse von items_ nach dem splitt:" << items.size() << "\n";

		item_map_[get_id(t)] = t;

		foreach (const std::string& k, keys) {
			group_map_[k].push_back(get_id(t));
			nmax_items_ = std::max(nmax_items_, group_map_[k].size());
			std::map<std::string, terrain_group*>::iterator i = id_to_group.find(k);
			if (i != id_to_group.end()) {
				if (i->second->core) {
					core = true;
				}
			}
		}

		// A terrain is considered core iff it appears in at least
		// one core terrain group
		if (core) {
		// Add the terrain to the default group
			group_map_["all"].push_back(get_id(t));
		nmax_items_ = std::max(nmax_items_, group_map_["all"].size());
		} else {
			non_core_items_.insert(get_id(t));
		}

	}
	//typedef std::pair<std::string, t_translation::t_list> map_pair;

	ERR_ED << "grosse von items bevor dem set_group:" << items.size() << "\n";

	// Set the default group
	set_group("all");

	ERR_ED << "grosse von items vor dem output:" << items.size() << "\n";
	if(active_group().empty()) {
		ERR_ED << "No items found.\n";
	}

	update_report();
}


void terrain_palette::draw_item(SDL_Rect& dstrect, const t_translation::t_terrain& terrain) {

	const t_translation::t_terrain base_terrain = map().get_terrain_info(terrain).default_base();

	surface screen = gui_.video().getSurface();

	//Draw default base for overlay terrains
			if(base_terrain != t_translation::NONE_TERRAIN) {
				const std::string base_filename = "terrain/" + map().get_terrain_info(base_terrain).editor_image() + ".png";
				surface base_image(image::get_image(base_filename));

				if(base_image == NULL) {
	//				tooltip_text << "BASE IMAGE NOT FOUND\n";
	//				ERR_ED << "image for terrain " << counter << ": '" << base_filename << "' not found\n";
					base_image = image::get_image(game_config::images::missing);
					if (base_image == NULL) {
						ERR_ED << "Placeholder image not found\n";
						return;
					}
				}

				if(static_cast<unsigned>(base_image->w) != size_specs_.terrain_size ||
				   static_cast<unsigned>(base_image->h) != size_specs_.terrain_size) {

					base_image.assign(scale_surface(base_image,
					   size_specs_.terrain_size, size_specs_.terrain_size));
				}

				sdl_blit(base_image, NULL, screen, &dstrect);
			}

			const std::string filename = "terrain/" + map().get_terrain_info(terrain).editor_image() + ".png";
			surface image(image::get_image(filename));
			if(image == NULL) {
	//			tooltip_text << "IMAGE NOT FOUND\n";
	//			ERR_ED << "image for terrain " << counter << ": '" << filename << "' not found\n";
				image = image::get_image(game_config::images::missing);
				if (image == NULL) {
					ERR_ED << "Placeholder image not found\n";
					return;
				}
			}

			if(static_cast<unsigned>(image->w) != size_specs_.terrain_size ||
				static_cast<unsigned>(image->h) != size_specs_.terrain_size) {

				image.assign(scale_surface(image,
					size_specs_.terrain_size, size_specs_.terrain_size));
			}

			sdl_blit(image, NULL, screen, &dstrect);
}


terrain_palette::terrain_palette(editor_display &gui, const size_specs &sizes,
								 const config& cfg,
								 t_translation::t_terrain& fore,
								 t_translation::t_terrain& back)
	:	editor_palette<t_translation::t_terrain, terrain_group>(gui, sizes, cfg, fore, back)
{
}

const std::string& terrain_palette::get_id(const t_translation::t_terrain& terrain)
{
	const terrain_type& t_info = map().get_terrain_info(terrain);
	return t_info.id();
}

const std::string& terrain_palette::get_id(const terrain_group& group)
{
	return group.id;
}

const std::vector<t_translation::t_terrain>& terrain_palette::get_items()
{
	return map().get_terrain_list();
}

}
