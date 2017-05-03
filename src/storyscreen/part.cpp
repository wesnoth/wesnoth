/*
   Copyright (C) 2009 - 2017 by Ignacio R. Morelle <shadowm2006@gmail.com>
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
 * @file
 * Storyscreen parts and floating images representation.
 */

#include "storyscreen/part.hpp"

#include "config.hpp"
#include "game_data.hpp"
#include "game_events/conditional_wml.hpp"
#include "game_events/manager.hpp"
#include "game_events/pump.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "serialization/string_utils.hpp"
#include "variable.hpp"

namespace storyscreen
{
floating_image::floating_image(const floating_image& fi)
	: file_()
	, x_(0)
	, y_(0)
	, delay_(0)
	, autoscaled_(false)
	, centered_(false)
{
	this->assign(fi);
}

floating_image::floating_image(const config& cfg)
	: file_(cfg["file"])
	, x_(cfg["x"])
	, y_(cfg["y"])
	, delay_(cfg["delay"])
	, autoscaled_(cfg["scaled"].to_bool())
	, centered_(cfg["centered"].to_bool())
{
}

void floating_image::assign(const floating_image& fi)
{
	if(&fi == this) {
		return;
	}

	file_ = fi.file_;
	x_ = fi.x_;
	y_ = fi.y_;
	delay_ = fi.delay_;
	autoscaled_ = fi.autoscaled_;
	centered_ = fi.centered_;
}

background_layer::background_layer()
	: scale_horizontally_(true)
	, scale_vertically_(true)
	, tile_horizontally_(false)
	, tile_vertically_(false)
	, keep_aspect_ratio_(true)
	, is_base_layer_(false)
	, image_file_()
{
}

background_layer::background_layer(const config& cfg)
	: scale_horizontally_(true)
	, scale_vertically_(true)
	, tile_horizontally_(false)
	, tile_vertically_(false)
	, keep_aspect_ratio_(true)
	, is_base_layer_(false)
	, image_file_()
{
	if(cfg.has_attribute("image")) {
		image_file_ = cfg["image"].str();
	}

	if(cfg.has_attribute("scale")) {
		scale_vertically_ = cfg["scale"].to_bool(true);
		scale_horizontally_ = cfg["scale"].to_bool(true);
	} else {
		if(cfg.has_attribute("scale_vertically")) {
			scale_vertically_ = cfg["scale_vertically"].to_bool(true);
		}

		if(cfg.has_attribute("scale_horizontally")) {
			scale_horizontally_ = cfg["scale_horizontally"].to_bool(true);
		}
	}

	if(cfg.has_attribute("tile")) {
		tile_vertically_ = cfg["tile"].to_bool(false);
		tile_horizontally_ = cfg["tile"].to_bool(false);
	} else {
		if(cfg.has_attribute("tile_vertically")) {
			tile_vertically_ = cfg["tile_vertically"].to_bool(false);
		}

		if(cfg.has_attribute("tile_horizontally")) {
			tile_horizontally_ = cfg["tile_horizontally"].to_bool(false);
		}
	}

	if(cfg.has_attribute("keep_aspect_ratio")) {
		keep_aspect_ratio_ = cfg["keep_aspect_ratio"].to_bool(true);
	}

	if(cfg.has_attribute("base_layer")) {
		is_base_layer_ = cfg["base_layer"].to_bool(false);
	}
}

part::part(const vconfig& part_cfg)
	: show_title_()
	, text_()
	, text_title_()
	, text_block_loc_(part::BLOCK_BOTTOM)
	, text_alignment_(part::TEXT_LEFT)
	, title_alignment_(part::TEXT_LEFT)
	, music_()
	, sound_()
	, background_layers_()
	, floating_images_()
{
	resolve_wml(part_cfg);
}

part::BLOCK_LOCATION part::string_tblock_loc(const std::string& s)
{
	if(s.empty() != true) {
		if(s == "top") {
			return part::BLOCK_TOP;
		} else if(s == "middle") {
			return part::BLOCK_MIDDLE;
		}
	}

	return part::BLOCK_BOTTOM;
}

part::TEXT_ALIGNMENT part::string_title_align(const std::string& s)
{
	if(s.empty() != true) {
		if(s == "right") {
			return part::TEXT_RIGHT;
		} else if(s == "center") {
			return part::TEXT_CENTERED;
		}
	}

	return part::TEXT_LEFT;
}

void part::resolve_wml(const vconfig& cfg)
{
	if(cfg.null()) {
		return;
	}

	// Converts shortcut syntax to members of [background_layer]
	background_layer bl;

	if(cfg.has_attribute("background")) {
		bl.set_file(cfg["background"].str());
	}

	if(cfg.has_attribute("scale_background")) {
		bl.set_scale_horizontally(cfg["scale_background"].to_bool(true));
		bl.set_scale_vertically(cfg["scale_background"].to_bool(true));
	} else {
		if(cfg.has_attribute("scale_background_vertically")) {
			bl.set_scale_vertically(cfg["scale_background_vertically"].to_bool(true));
		}

		if(cfg.has_attribute("scale_background_horizontally")) {
			bl.set_scale_horizontally(cfg["scale_background_horizontally"].to_bool(true));
		}
	}

	if(cfg.has_attribute("tile_background")) {
		bl.set_tile_horizontally(cfg["tile_background"].to_bool(false));
		bl.set_tile_vertically(cfg["tile_background"].to_bool(false));
	} else {
		if(cfg.has_attribute("tile_background_vertically")) {
			bl.set_tile_vertically(cfg["tile_background_vertically"].to_bool(false));
		}

		if(cfg.has_attribute("tile_background_horizontally")) {
			bl.set_tile_vertically(cfg["tile_background_horizontally"].to_bool(false));
		}
	}

	if(cfg.has_attribute("keep_aspect_ratio")) {
		bl.set_keep_aspect_ratio(cfg["keep_aspect_ratio"].to_bool(true));
	}

	background_layers_.push_back(bl);

	if(cfg.has_attribute("show_title")) {
		show_title_ = cfg["show_title"].to_bool();
	}

	if(cfg.has_attribute("story")) {
		text_ = cfg["story"].str();
	}

	if(cfg.has_attribute("title")) {
		text_title_ = cfg["title"].str();
		if(!cfg.has_attribute("show_title")) {
			show_title_ = true;
		}
	}

	if(cfg.has_attribute("text_layout")) {
		text_block_loc_ = string_tblock_loc(cfg["text_layout"]);
	}

	if(cfg.has_attribute("text_alignment")) {
		text_alignment_ = string_title_align(cfg["text_alignment"]);
	}

	if(cfg.has_attribute("title_alignment")) {
		title_alignment_ = string_title_align(cfg["title_alignment"]);
	}

	if(cfg.has_attribute("music")) {
		music_ = cfg["music"].str();
	}

	if(cfg.has_attribute("sound")) {
		sound_ = cfg["sound"].str();
	}

	// Execution flow/branching/[image]
	for(vconfig::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); ++i) {
		// i->first and i->second are goddamn temporaries; do not make references
		const std::string key = i->first;
		const vconfig node = i->second;

		// [background_layer]
		if(key == "background_layer") {
			background_layers_.push_back(node.get_parsed_config());
		}
		// [image]
		else if(key == "image") {
			floating_images_.push_back(node.get_parsed_config());
		}
		// [if]
		else if(key == "if") {
			// check if the [if] tag has a [then] child;
			// if we try to execute a non-existing [then], we get a segfault
			if(game_events::conditional_passed(node)) {
				if(node.has_child("then")) {
					resolve_wml(node.child("then"));
				}
			}
			// condition not passed, check [elseif] and [else]
			else {
				// get all [elseif] children and set a flag
				vconfig::child_list elseif_children = node.get_children("elseif");
				bool elseif_flag = false;
				// for each [elseif]: test if it has a [then] child
				// if the condition matches, execute [then] and raise flag
				for(vconfig::child_list::const_iterator elseif = elseif_children.begin();
						elseif != elseif_children.end(); ++elseif) {
					if(game_events::conditional_passed(*elseif)) {
						if(elseif->has_child("then")) {
							resolve_wml(elseif->child("then"));
						}

						elseif_flag = true;
						break;
					}
				}

				// if we have an [else] tag and no [elseif] was successful (flag not raised), execute it
				if(node.has_child("else") && !elseif_flag) {
					resolve_wml(node.child("else"));
				}
			}
		}
		// [switch]
		else if(key == "switch") {
			const std::string var_name = node["variable"];
			const std::string var_actual_value = resources::gamedata->get_variable_const(var_name);
			bool case_not_found = true;

			for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
				if(j->first != "case") {
					continue;
				}

				// Enter all matching cases.
				const std::string var_expected_value = (j->second)["value"];
				if(var_actual_value == var_expected_value) {
					case_not_found = false;
					resolve_wml(j->second);
				}
			}

			if(case_not_found) {
				for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
					if(j->first != "else") {
						continue;
					}

					// Enter all elses.
					resolve_wml(j->second);
				}
			}
		}
		// [deprecated_message]
		else if(key == "deprecated_message") {
			// Won't appear until the scenario start event finishes.
			lg::wml_error() << node["message"] << '\n';
		}
		// [wml_message]
		else if(key == "wml_message") {
			// As with [deprecated_message],
			// it won't appear until the scenario start event is complete.
			resources::game_events->pump().put_wml_message(
				node["logger"], node["message"], node["in_chat"].to_bool(false));
		}
	}
}

} // end namespace storyscreen
