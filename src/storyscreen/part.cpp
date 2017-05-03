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
	, text_alignment_("left")
	, title_alignment_("left")
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
		text_alignment_ = cfg["text_alignment"].str();
	}

	if(cfg.has_attribute("title_alignment")) {
		title_alignment_ = cfg["title_alignment"].str();
	}

	if(cfg.has_attribute("music")) {
		music_ = cfg["music"].str();
	}

	if(cfg.has_attribute("sound")) {
		sound_ = cfg["sound"].str();
	}

	// Inherited
	story_parser::resolve_wml(cfg);
}

bool part::resolve_wml_helper(const std::string& key, const vconfig& node)
{
	bool found = false;

	// [background_layer]
	if(key == "background_layer") {
		background_layers_.push_back(node.get_parsed_config());
		found = true;
	}
	// [image]
	else if(key == "image") {
		floating_images_.push_back(node.get_parsed_config());
		found = true;
	}

	return found;
}

} // end namespace storyscreen
