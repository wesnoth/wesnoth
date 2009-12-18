/* $Id$ */
/*
   Copyright (C) 2009 by Ignacio R. Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file storyscreen/part.cpp
 * Storyscreen parts and floating images representation.
 */

#include "global.hpp"
#include "asserts.hpp"
#include "foreach.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "storyscreen/part.hpp"

#include "config.hpp"
#include "gamestatus.hpp"
#include "game_events.hpp"
#include "image.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "video.hpp"

namespace storyscreen {

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
	, x_(lexical_cast_default<int>(cfg["x"]))
	, y_(lexical_cast_default<int>(cfg["y"]))
	, delay_(lexical_cast_default<int>(cfg["delay"]))
	, autoscaled_(utils::string_bool(cfg["scaled"], false))
	, centered_(utils::string_bool(cfg["centered"], false))
{
}

void floating_image::assign(const floating_image& fi)
{
	if(&fi == this)
		return;

	file_ = fi.file_; x_ = fi.x_; y_ = fi.y_; delay_ = fi.delay_;
	autoscaled_ = fi.autoscaled_; centered_ = fi.centered_;
}

floating_image::render_input floating_image::get_render_input(double scale, SDL_Rect& dst_rect) const
{
	render_input ri = {
		{0,0,0,0},
		file_.empty() ? NULL : image::get_image(file_)
	};

	if(!ri.image.null()) {
		if(autoscaled_) {
			ri.image = scale_surface(
				ri.image,
				static_cast<int>(ri.image->w * scale),
				static_cast<int>(ri.image->h * scale)
			);
		}

		ri.rect.x = static_cast<int>(x_*scale) + dst_rect.x;
		ri.rect.y = static_cast<int>(y_*scale) + dst_rect.y;
		ri.rect.w = ri.image->w;
		ri.rect.h = ri.image->h;

		if(centered_) {
			ri.rect.x -= ri.rect.w / 2;
			ri.rect.y -= ri.rect.h / 2;
		}
	}
	return ri;
}

part::part(const vconfig &part_cfg)
	: scale_background_(true)
	, background_file_()
	, show_title_()
	, text_()
	, text_title_()
	, text_block_loc_(part::BLOCK_BOTTOM)
	, title_alignment_(part::TEXT_LEFT)
	, music_()
	, sound_()
	, floating_images_()
{
	resolve_wml(part_cfg);
}

part::BLOCK_LOCATION part::string_tblock_loc(const std::string& s)
{
	if(s.empty() != true) {
		if(s == "top") {
			return part::BLOCK_TOP;
		}
		else if (s == "middle") {
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
		}
		else if(s == "center") {
			return part::TEXT_CENTERED;
		}
	}
	return part::TEXT_LEFT;
}

void part::resolve_wml(const vconfig &cfg)
{
	if(cfg.null()) {
		return;
	}

	if(cfg.has_attribute("background")) {
		background_file_ = cfg["background"];
	}
	if(cfg.has_attribute("scale_background")) {
		scale_background_ = utils::string_bool(cfg["scale_background"], true);
	}
	if(cfg.has_attribute("show_title")) {
		show_title_ = utils::string_bool(cfg["show_title"]);
	}
	if(cfg.has_attribute("story")) {
		text_ = cfg["story"];
	}
	if(cfg.has_attribute("title")) {
		text_title_ = cfg["title"];
		if(!cfg.has_attribute("show_title")) {
			show_title_ = true;
		}
	}
	if(cfg.has_attribute("text_layout")) {
		text_block_loc_ = string_tblock_loc(cfg["text_layout"]);
	}
	if(cfg.has_attribute("title_alignment")) {
		title_alignment_ = string_title_align(cfg["title_alignment"]);
	}
	if(cfg.has_attribute("music")) {
		music_ = cfg["music"];
	}
	if(cfg.has_attribute("sound")) {
		sound_ = cfg["sound"];
	}

	// Execution flow/branching/[image]
	for(vconfig::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); ++ i) {
		// i->first and i->second are goddamn temporaries; do not make references
		const std::string key = i->first;
		const vconfig node = i->second;

		// [image]
		if(key == "image") {
			floating_images_.push_back(node.get_parsed_config());
		}
		// [if]
		else if(key == "if") {
			const std::string branch_label =
				game_events::conditional_passed(NULL, node) ?
				"then" : "else";
			const vconfig branch = node.child(branch_label);
			resolve_wml(branch);
		}
		// [switch]
		else if(key == "switch") {
			const std::string var_name = node["variable"];
			const std::string var_actual_value = resources::state_of_game->get_variable_const(var_name);
			bool case_not_found = true;

			for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
				if(j->first != "case") continue;

				// Enter all matching cases.
				const std::string var_expected_value = (j->second)["value"];
			    if(var_actual_value == var_expected_value) {
					case_not_found = false;
					resolve_wml(j->second);
			    }
			}

			if(case_not_found) {
				for(vconfig::all_children_iterator j = node.ordered_begin(); j != node.ordered_end(); ++j) {
					if(j->first != "else") continue;

					// Enter all elses.
					resolve_wml(j->second);
				}
			}
		}
		// [deprecated_message]
		else if(key == "deprecated_message") {
			// Won't appear until the scenario start event finishes.
			game_events::handle_deprecated_message(node.get_parsed_config());
		}
		// [wml_message]
		else if(key == "wml_message") {
			// Pass to game events handler. As with [deprecated_message],
			// it won't appear until the scenario start event is complete.
			game_events::handle_wml_log_message(node.get_parsed_config());
		}
	}
}

} // end namespace storyscreen

