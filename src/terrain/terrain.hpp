/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "terrain/translation.hpp"
#include "utils/general.hpp"
#include "utils/math.hpp"

class terrain_type
{
public:

	terrain_type();
	terrain_type(const config& cfg);
	terrain_type(const terrain_type& base, const terrain_type& overlay);

	const std::string& icon_image() const { return icon_image_; }
	const std::string& minimap_image() const { return minimap_image_; }
	const std::string& minimap_image_overlay() const { return minimap_image_overlay_; }
	const std::string& editor_image() const { return editor_image_; }
	const t_string& name() const { return name_; }
	const t_string& editor_name() const { return editor_name_.empty() ? description() : editor_name_; }
	const t_string& description() const { return description_.empty() ? name_ : description_; }
	const t_string& help_topic_text() const { return help_topic_text_; }
	const std::string& id() const { return id_; }

	bool hide_help() const { return hide_help_; }
	bool hide_in_editor() const { return hide_in_editor_; }
	bool hide_if_impassable() const { return hide_if_impassable_; }

	//the character representing this terrain
	t_translation::terrain_code number() const { return number_; }

	//the underlying type of the terrain
	const t_translation::ter_list& mvt_type() const { return mvt_type_; }
	const t_translation::ter_list& def_type() const { return def_type_; }
	const t_translation::ter_list& vision_type() const { return vision_type_; }
	const t_translation::ter_list& union_type() const { return union_type_; }

	bool is_nonnull() const { return  (number_ != t_translation::NONE_TERRAIN) &&
		(number_ != t_translation::VOID_TERRAIN ); }
	/// Returns the light (lawful) bonus for this terrain when the time of day
	/// gives a @a base bonus.
	int light_bonus(int base) const
	{
		return bounded_add(base, light_modification_, max_light_, min_light_);
	}

	int unit_height_adjust() const { return height_adjust_; }
	double unit_submerge() const { return submerge_; }

	int gives_healing() const { return heals_; }
	bool is_village() const { return village_; }
	bool is_castle() const { return castle_; }
	bool is_keep() const { return keep_; }

	//these descriptions are shown for the terrain in the mouse over
	//depending on the owner or the village
	const t_string& income_description() const { return income_description_; }
	const t_string& income_description_ally() const { return income_description_ally_; }
	const t_string& income_description_enemy() const { return income_description_enemy_; }
	const t_string& income_description_own() const { return income_description_own_; }

	const std::string& editor_group() const { return editor_group_; }
	void set_editor_group(const std::string& str) { editor_group_ = str; }

	bool is_overlay() const { return overlay_; }
	bool is_combined() const { return combined_; }

	t_translation::terrain_code default_base() const { return editor_default_base_; }
	t_translation::terrain_code terrain_with_default_base() const;

	bool operator==(const terrain_type& other) const;
private:

	/** The image used as symbol icon */
	std::string icon_image_;

	/** The image used in the minimap */
	std::string minimap_image_;
	std::string minimap_image_overlay_;

	/**
	 *  The image used in the editor palette if not defined in WML it will be
	 *  initialized with the value of minimap_image_
	 */
	std::string editor_image_;
	std::string id_;
	t_string name_;
	t_string editor_name_;
	t_string description_;
	t_string help_topic_text_;

	//the 'number' is the number that represents this
	//terrain type. The 'type' is a list of the 'underlying types'
	//of the terrain. This may simply be the same as the number.
	//This is the internal number used, WML still uses character strings.
	t_translation::terrain_code number_;
	t_translation::ter_list mvt_type_;
	t_translation::ter_list vision_type_;
	t_translation::ter_list def_type_;
	t_translation::ter_list union_type_;

	int height_adjust_;
	bool height_adjust_set_;

	double submerge_;
	bool submerge_set_;

	int light_modification_;
	int max_light_;
	int min_light_;
	int heals_;

	t_string income_description_;
	t_string income_description_ally_;
	t_string income_description_enemy_;
	t_string income_description_own_;

	std::string editor_group_;

	bool village_, castle_, keep_;

	bool overlay_, combined_;
	t_translation::terrain_code editor_default_base_;
	bool hide_help_, hide_in_editor_, hide_if_impassable_;
};

void create_terrain_maps(const config::const_child_itors &cfgs,
                         t_translation::ter_list& terrain_list,
                         std::map<t_translation::terrain_code, terrain_type>& letter_to_terrain);

void merge_alias_lists(t_translation::ter_list& first, const t_translation::ter_list& second);
