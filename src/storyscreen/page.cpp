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
 * @file storyscreen/page.cpp
 * This code is work in progress, and the interfaces may change.
 * It is supposed to completely replace the old story screens code
 * at intro.cpp, introducing new WML conventions while at it.
 */

#include "global.hpp"
#include "asserts.hpp"
#include "log.hpp"
#include "storyscreen/page.hpp"

#include "config.hpp"
#include "gamestatus.hpp"
#include "game_events.hpp"
#include "serialization/string_utils.hpp"
#include "util.hpp"
#include "variable.hpp"

// TODO: remove when completed
#include "stub.hpp"

namespace storyscreen {

floating_image::floating_image(const config& cfg)
	: file_(cfg["file"])
	, x_(lexical_cast_default<int>(cfg["x"]))
	, y_(lexical_cast_default<int>(cfg["y"]))
	, delay_(lexical_cast_default<int>(cfg["delay"]))
	, autoscaled_(utils::string_bool(cfg["scaled"], false))
	, centered_(utils::string_bool(cfg["centered"], false))
{
}

page::page(game_state& state_of_game, const vconfig& page_cfg)
	: scale_background_(utils::string_bool(page_cfg["scale_background"], true))
	, background_file_(page_cfg["background"])
	, show_title_(utils::string_bool(page_cfg["show_title"], false))
	, text_(page_cfg["story"])
	, text_title_(page_cfg["title"])
	, text_block_loc_(string_tblock_loc(page_cfg["text_layout"]))
	, music_(page_cfg["music"])
	, floating_images_()
{
	resolve_wml(page_cfg);
}

page::TEXT_BLOCK_LOCATION page::string_tblock_loc(const std::string& s)
{
	if(s.empty() != true) {
		if(s == "top") {
			return page::TOP;
		}
		else if(s == "middle" || s == "center") {
			return page::MIDDLE;
		}
	}

	return page::BOTTOM;
}

void page::resolve_wml(const vconfig& page_cfg)
{
	STUB();
	for(vconfig::all_children_iterator i = page_cfg.ordered_begin(); i != page_cfg.ordered_end(); ++ i) {
		const std::pair<const std::string, const vconfig> xi = *i;
		if(xi.first == "image") {
			floating_images_.push_back(xi.second.get_parsed_config());
		}
		else if(xi.first == "if") {
			const std::string type = game_events::conditional_passed(
				NULL, xi.second) ? "then":"else";
			const vconfig branch = xi.second.child(type);
			if(!branch.empty()) {
				if(branch.has_attribute("background")) {
					this->background_file_ = branch["background"];
				}
				if(branch.has_attribute("scale_background")) {
					this->scale_background_ = utils::string_bool(branch["scale_background"], true);
				}
				if(branch.has_attribute("show_title")) {
					this->show_title_ = utils::string_bool(branch["show_title"], false);
				}
				if(branch.has_attribute("title")) {
					this->text_title_ = branch["title"];
				}
				if(branch.has_attribute("story")) {
					this->text_ = branch["story"];
				}
				if(branch.has_attribute("text_layout")) {
					this->text_block_loc_ = string_tblock_loc(branch["text_layout"]);
				}
				if(branch.has_attribute("music")) {
					this->text_ = branch["music"];
				}

				// TODO recursive eval
				// TODO image stack
				
			}
		}
		else if(xi.first == "switch") {
		}
		else if(xi.first == "deprecated_message") {
			const std::string dmsg = (xi.second)["message"];
			if(!dmsg.empty()) {
				lg::wml_error << dmsg << '\n';
			}
		}
	}
}

floating_image::floating_image()
	: file_(), x_(0), y_(0), delay_(0), autoscaled_(false), centered_(false)
{
	ASSERT_EQ(0xBAD,0xBEEF);
}

page::page()
	: scale_background_()
	, background_file_()
	, show_title_()
	, text_()
	, text_title_()
	, text_block_loc_()
	, music_()
	, floating_images_()
{
	ASSERT_EQ(0xDEAD,0xBEEF);
}

} // end namespace storyscreen

