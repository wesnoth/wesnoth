/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file */

#include "config_assign.hpp"
#include "font/sdl_ttf.hpp"
#include "gettext.hpp"
#include "hotkey/hotkey_command.hpp"
#include "hotkey/hotkey_item.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "theme.hpp"
#include "wml_exception.hpp"
#include "sdl/rect.hpp"

static lg::log_domain log_display("display");
#define DBG_DP LOG_STREAM(debug, log_display)
#define LOG_DP LOG_STREAM(info, log_display)
#define ERR_DP LOG_STREAM(err, log_display)

namespace {
	const int XDim = 1024;
	const int YDim = 768;

	const size_t DefaultFontSize = font::SIZE_NORMAL;
	const Uint32 DefaultFontRGB = 0x00C8C8C8;

	_rect ref_rect = { 0, 0, 0, 0 };
}

static size_t compute(std::string expr, size_t ref1, size_t ref2=0 ) {
		size_t ref = 0;
		if (expr[0] == '=') {
		  ref = ref1;
		  expr = expr.substr(1);
		} else if ((expr[0] == '+') || (expr[0] == '-')) {
		  ref = ref2;
		}

		return ref + atoi(expr.c_str());
	}

	// If x2 or y2 are not specified, use x1 and y1 values
static _rect read_rect(const config& cfg) {
		_rect rect = { 0, 0, 0, 0 };
		std::vector<std::string> items = utils::split(cfg["rect"].str());
		if(items.size() >= 1)
			rect.x1 = atoi(items[0].c_str());

		if(items.size() >= 2)
			rect.y1 = atoi(items[1].c_str());

		if(items.size() >= 3)
			rect.x2 = atoi(items[2].c_str());
		else
			rect.x2 = rect.x1;

		if(items.size() >= 4)
			rect.y2 = atoi(items[3].c_str());
		else
			rect.y2 = rect.y1;

		return rect;
	}

static SDL_Rect read_sdl_rect(const config& cfg) {
		SDL_Rect sdlrect;
		const _rect rect = read_rect(cfg);
		sdlrect.x = rect.x1;
		sdlrect.y = rect.y1;
		sdlrect.w = (rect.x2 > rect.x1) ? (rect.x2 - rect.x1) : 0;
		sdlrect.h = (rect.y2 > rect.y1) ? (rect.y2 - rect.y1) : 0;

		return sdlrect;
	}

static std::string resolve_rect(const std::string& rect_str) {
		_rect rect = { 0, 0, 0, 0 };
		std::stringstream resolved;
		const std::vector<std::string> items = utils::split(rect_str.c_str());
		if(items.size() >= 1) {
			rect.x1 = compute(items[0], ref_rect.x1, ref_rect.x2);
			resolved << rect.x1;
		}
		if(items.size() >= 2) {
			rect.y1 = compute(items[1], ref_rect.y1, ref_rect.y2);
			resolved << "," << rect.y1;
		}
		if(items.size() >= 3) {
			rect.x2 = compute(items[2], ref_rect.x2, rect.x1);
			resolved << "," << rect.x2;
		}
		if(items.size() >= 4) {
			rect.y2 = compute(items[3], ref_rect.y2, rect.y1);
			resolved << "," << rect.y2;
		}

		// DBG_DP << "Rect " << rect_str << "\t: " << resolved.str() << "\n";

		ref_rect = rect;
		return resolved.str();
	}

static config &find_ref(const std::string &id, config &cfg, bool remove = false)
{
	static config empty_config;

	config::all_children_itors itors = cfg.all_children_range();
	for (config::all_children_iterator i = itors.begin(); i != itors.end(); ++i)
	{
		config &icfg = i->cfg;
		if (i->cfg["id"] == id) {
			if (remove) {
				cfg.erase(i);
				return empty_config;
			} else {
				return icfg;
			}
		}

		// Recursively look in children.
		config &c = find_ref(id, icfg, remove);
		if (&c != &empty_config) {
			return c;
		}
	}

	// Not found.
	return empty_config;
}

#ifdef DEBUG

// to be called from gdb
static config& find_ref(const char* id, config& cfg) {
	return find_ref(std::string(id),cfg);
}

namespace {
	// avoid some compiler warnings in stricter mode.
	static config cfg;
	static config& result = find_ref("", cfg);
} // namespace

#endif

/**
 * Returns a copy of the wanted resolution.
 *
 * The function returns a copy since our caller uses a copy of this resolution
 * as base to expand a partial resolution.
 *
 * @param resolutions             A config object containing the expanded
 *                                resolutions.
 * @param id                      The id of the resolution to return.
 *
 * @throw config::error           If the @p id is not found.
 *
 * @returns                       A copy of the resolution config.
 */
static config get_resolution(const config& resolutions, const std::string& id)
{
	for(const auto& resolution : resolutions.child_range("resolution")) {
		if(resolution["id"] == id) {
			return resolution;
		}
	}

	throw config::error(
			  "[partialresolution] refers to non-existent [resolution] " + id);
}

/**
 * Returns a config with all partial resolutions of a theme expanded.
 *
 * @param theme                   The original object, whose objects need to be
 *                                expanded.
 *
 * @returns                       A new object with the expanded resolutions in
 *                                a theme. This object no longer contains
 *                                partial resolutions.
 */
static config expand_partialresolution(const config& theme)
{
	config result;

	// Add all the resolutions
	for(const auto& resolution : theme.child_range("resolution")) {
		result.add_child("resolution", resolution);
	}

	// Resolve all the partialresolutions
	for(const auto& part : theme.child_range("partialresolution")) {
		config resolution = get_resolution(result, part["inherits"]);
		resolution.merge_attributes(part);

		for(const auto& remove : part.child_range("remove")) {
			VALIDATE(!remove["id"].empty()
					, missing_mandatory_wml_key(
						  "[theme][partialresolution][remove]"
						, "id"));

			find_ref(remove["id"], resolution, true);
		}

		for(const auto& change : part.child_range("change")) {
			VALIDATE(!change["id"].empty()
					, missing_mandatory_wml_key(
						  "[theme][partialresolution][change]"
						, "id"));

			config& target = find_ref(change["id"], resolution, false);
			target.merge_attributes(change);
		}

		// cannot add [status] sub-elements, but who cares
		for(const auto& add : part.child_range("add")) {
			for(const auto& child : add.all_children_range()) {
				resolution.add_child(child.key, child.cfg);
			}
		}

		result.add_child("resolution", resolution);
	}

	return result;
}

static void do_resolve_rects(const config& cfg, config& resolved_config, config* resol_cfg = nullptr) {

		// recursively resolve children
	for(const config::any_child &value : cfg.all_children_range()) {
			config &childcfg = resolved_config.add_child(value.key);
			do_resolve_rects(value.cfg, childcfg,
				value.key == "resolution" ? &childcfg : resol_cfg);
		}

		// copy all key/values
		resolved_config.merge_attributes(cfg);

		// override default reference rect with "ref" parameter if any
		if (!cfg["ref"].empty()) {
			if (resol_cfg == nullptr) {
				ERR_DP << "Use of ref= outside a [resolution] block" << std::endl;
			} else {
				//DBG_DP << ">> Looking for " << cfg["ref"] << "\n";
				const config& ref = find_ref (cfg["ref"], *resol_cfg);

				if (ref["id"].empty()) {
					ERR_DP << "Reference to non-existent rect id \"" << cfg["ref"] << "\"" << std::endl;
				} else if (ref["rect"].empty()) {
					ERR_DP << "Reference to id \"" << cfg["ref"] <<
						"\" which does not have a \"rect\"\n";
				} else {
					ref_rect = read_rect(ref);
				}
			}
		}
		// resolve the rect value to absolute coordinates
		if (!cfg["rect"].empty()) {
			resolved_config["rect"] = resolve_rect(cfg["rect"]);
		}
	}

theme::object::object() :
	location_modified_(false),
	id_(),
	loc_(sdl::empty_rect),
	relative_loc_(sdl::empty_rect),
	last_screen_(sdl::empty_rect),
	xanchor_(object::FIXED),
	yanchor_(object::FIXED)
{
}

theme::object::object(const config& cfg) :
		location_modified_(false), id_(cfg["id"]), loc_(read_sdl_rect(cfg)),
		relative_loc_(sdl::empty_rect), last_screen_(sdl::empty_rect),
		xanchor_(read_anchor(cfg["xanchor"])), yanchor_(read_anchor(cfg["yanchor"]))
{
}

theme::border_t::border_t() :
	size(0.0),
	background_image(),
	tile_image(),
	corner_image_top_left(),
	corner_image_bottom_left(),
	corner_image_top_right_odd(),
	corner_image_top_right_even(),
	corner_image_bottom_right_odd(),
	corner_image_bottom_right_even(),
	border_image_left(),
	border_image_right(),
	border_image_top_odd(),
	border_image_top_even(),
	border_image_bottom_odd(),
	border_image_bottom_even()
{
}

theme::border_t::border_t(const config& cfg) :
	size(cfg["border_size"].to_double()),

	background_image(cfg["background_image"]),
	tile_image(cfg["tile_image"]),

	corner_image_top_left(cfg["corner_image_top_left"]),
	corner_image_bottom_left(cfg["corner_image_bottom_left"]),

	corner_image_top_right_odd(cfg["corner_image_top_right_odd"]),
	corner_image_top_right_even(cfg["corner_image_top_right_even"]),

	corner_image_bottom_right_odd(cfg["corner_image_bottom_right_odd"]),
	corner_image_bottom_right_even(cfg["corner_image_bottom_right_even"]),

	border_image_left(cfg["border_image_left"]),
	border_image_right(cfg["border_image_right"]),

	border_image_top_odd(cfg["border_image_top_odd"]),
	border_image_top_even(cfg["border_image_top_even"]),

	border_image_bottom_odd(cfg["border_image_bottom_odd"]),
	border_image_bottom_even(cfg["border_image_bottom_even"])
{
	VALIDATE(size >= 0.0 && size <= 0.5, _("border_size should be between 0.0 and 0.5."));
}

SDL_Rect& theme::object::location(const SDL_Rect& screen) const
{
	if(last_screen_ == screen && !location_modified_)
		return relative_loc_;

	last_screen_ = screen;

	switch(xanchor_) {
	case FIXED:
		relative_loc_.x = loc_.x;
		relative_loc_.w = loc_.w;
		break;
	case TOP_ANCHORED:
		relative_loc_.x = loc_.x;
		relative_loc_.w = screen.w - std::min<size_t>(XDim - loc_.w,screen.w);
		break;
	case BOTTOM_ANCHORED:
		relative_loc_.x = screen.w - std::min<size_t>(XDim - loc_.x,screen.w);
		relative_loc_.w = loc_.w;
		break;
	case PROPORTIONAL:
		relative_loc_.x = (loc_.x*screen.w)/XDim;
		relative_loc_.w = (loc_.w*screen.w)/XDim;
		break;
	default:
		assert(false);
	}

	switch(yanchor_) {
	case FIXED:
		relative_loc_.y = loc_.y;
		relative_loc_.h = loc_.h;
		break;
	case TOP_ANCHORED:
		relative_loc_.y = loc_.y;
		relative_loc_.h = screen.h - std::min<size_t>(YDim - loc_.h,screen.h);
		break;
	case BOTTOM_ANCHORED:
		relative_loc_.y = screen.h - std::min<size_t>(YDim - loc_.y,screen.h);
		relative_loc_.h = loc_.h;
		break;
	case PROPORTIONAL:
		relative_loc_.y = (loc_.y*screen.h)/YDim;
		relative_loc_.h = (loc_.h*screen.h)/YDim;
		break;
	default:
		assert(false);
	}

	relative_loc_.x = std::min<int>(relative_loc_.x,screen.w);
	relative_loc_.w = std::min<int>(relative_loc_.w,screen.w - relative_loc_.x);
	relative_loc_.y = std::min<int>(relative_loc_.y,screen.h);
	relative_loc_.h = std::min<int>(relative_loc_.h,screen.h - relative_loc_.y);

	return relative_loc_;
}

theme::object::ANCHORING theme::object::read_anchor(const std::string& str)
{
	static const std::string top_anchor = "top", left_anchor = "left",
	                         bot_anchor = "bottom", right_anchor = "right",
							 proportional_anchor = "proportional";
	if(str == top_anchor || str == left_anchor)
		return TOP_ANCHORED;
	else if(str == bot_anchor || str == right_anchor)
		return BOTTOM_ANCHORED;
	else if(str == proportional_anchor)
		return PROPORTIONAL;
	else
		return FIXED;
}

void theme::object::modify_location(const _rect& rect){
	loc_.x = rect.x1;
	loc_.y = rect.y1;
	loc_.w = rect.x2 - rect.x1;
	loc_.h = rect.y2 - rect.y1;
	location_modified_ = true;
}

void theme::object::modify_location(std::string rect_str, SDL_Rect location_ref_rect){
	_rect rect = { 0, 0, 0, 0 };
	const std::vector<std::string> items = utils::split(rect_str.c_str());
	if(items.size() >= 1) {
		rect.x1 = compute(items[0], location_ref_rect.x, location_ref_rect.x + location_ref_rect.w);
	}
	if(items.size() >= 2) {
		rect.y1 = compute(items[1], location_ref_rect.y, location_ref_rect.y + location_ref_rect.h);
	}
	if(items.size() >= 3) {
		rect.x2 = compute(items[2], location_ref_rect.x + location_ref_rect.w, rect.x1);
	}
	if(items.size() >= 4) {
		rect.y2 = compute(items[3], location_ref_rect.y + location_ref_rect.h, rect.y1);
	}
	modify_location(rect);
}

theme::label::label() :
	text_(),
	icon_(),
	font_(),
	font_rgb_set_(false),
	font_rgb_(DefaultFontRGB)
{}

theme::label::label(const config& cfg) :
	object(cfg),
	text_(cfg["prefix"].str() + cfg["text"].str() + cfg["postfix"].str()),
	icon_(cfg["icon"]),
	font_(cfg["font_size"]),
	font_rgb_set_(false),
	font_rgb_(DefaultFontRGB)
{
	if(font_ == 0)
		font_ = DefaultFontSize;

	if (cfg.has_attribute("font_rgb"))
	{
		std::vector<std::string> rgb_vec = utils::split(cfg["font_rgb"]);
		if (3 <= rgb_vec.size()) {
			std::vector<std::string>::iterator c=rgb_vec.begin();
			int r,g,b;
			r = (atoi(c->c_str()));
			++c;
			if (c != rgb_vec.end()) {
				g = (atoi(c->c_str()));
				++c;
			} else {
				g=0;
			}
			if (c != rgb_vec.end()) {
				b=(atoi(c->c_str()));
			} else {
				b=0;
			}
			font_rgb_ = (((r<<16) & 0x00FF0000) + ((g<<8) & 0x0000FF00) + ((b) & 0x000000FF));
			font_rgb_set_=true;
		}
	}
}

theme::status_item::status_item(const config& cfg) :
	object(cfg),
	prefix_(cfg["prefix"].str() + cfg["prefix_literal"].str()),
	postfix_(cfg["postfix_literal"].str() + cfg["postfix"].str()),
	label_(),
	font_(cfg["font_size"]),
	font_rgb_set_(false),
	font_rgb_(DefaultFontRGB)
{
	if(font_ == 0)
		font_ = DefaultFontSize;

	if (const config &label_child = cfg.child("label")) {
		label_ = label(label_child);
	}

	if (cfg.has_attribute("font_rgb"))
	{
	  std::vector<std::string> rgb_vec = utils::split(cfg["font_rgb"]);
	  if(3 <= rgb_vec.size()){
	    std::vector<std::string>::iterator c=rgb_vec.begin();
	    int r,g,b;
	    r = (atoi(c->c_str()));
	    ++c;
	    if(c != rgb_vec.end()){
	      g = (atoi(c->c_str()));
	      ++c;
	    }else{
	      g=0;
	    }
	    if(c != rgb_vec.end()){
	      b=(atoi(c->c_str()));
	    }else{
	      b=0;
	    }
	    font_rgb_ = (((r<<16) & 0x00FF0000) + ((g<<8) & 0x0000FF00) + ((b) & 0x000000FF));
	    font_rgb_set_=true;
	  }
	}
}

theme::panel::panel(const config& cfg) : object(cfg), image_(cfg["image"])
{}

theme::slider::slider() :
		object(),
		title_(),
		tooltip_(),
		image_(),
		overlay_(),
		black_line_(false)
{}
theme::slider::slider(const config &cfg):
	object(cfg),
	title_(cfg["title"].str() + cfg["title_literal"].str()),
	tooltip_(cfg["tooltip"]), image_(cfg["image"]), overlay_(cfg["overlay"]),
	black_line_(cfg["black_line"].to_bool(false))
{}

theme::menu::menu() :
	object(),
	button_(true),
	context_(false),
	title_(),
	tooltip_(),
	image_(),
	overlay_(),
	items_()
{}

theme::menu::menu(const config &cfg):
	object(cfg),
	button_(cfg["button"].to_bool(true)),
	context_(cfg["is_context_menu"].to_bool(false)),
	title_(cfg["title"].str() + cfg["title_literal"].str()),
	tooltip_(cfg["tooltip"]), image_(cfg["image"]), overlay_(cfg["overlay"]),
	items_()
{
	for(const auto& item : utils::split(cfg["items"])) {
		items_.emplace_back(config_of("id", item));
	}

	if (cfg["auto_tooltip"].to_bool() && tooltip_.empty() && items_.size() == 1) {
		tooltip_ = hotkey::get_description(items_[0]["id"])
		+ hotkey::get_names(items_[0]["id"]) +  "\n" + hotkey::get_tooltip(items_[0]["id"]);
	} else if (cfg["tooltip_name_prepend"].to_bool() && items_.size() == 1) {
		tooltip_ = hotkey::get_description(items_[0]["id"])
		+ hotkey::get_names(items_[0]["id"]) + "\n" + tooltip_;
	}
}

theme::action::action() :
	object(),
	context_(false),
	auto_tooltip_(false),
	tooltip_name_prepend_(false),
	title_(),
	tooltip_(),
	image_(),
	overlay_(),
	type_(),
	items_()
{}

theme::action::action(const config &cfg):
	object(cfg), context_(cfg["is_context_menu"].to_bool()),
	auto_tooltip_(cfg["auto_tooltip"].to_bool(false)),
	tooltip_name_prepend_(cfg["tooltip_name_prepend"].to_bool(false)),
	title_(cfg["title"].str() + cfg["title_literal"].str()),
	tooltip_(cfg["tooltip"]), image_(cfg["image"]), overlay_(cfg["overlay"]), type_(cfg["type"]),
	items_(utils::split(cfg["items"]))
{}

const std::string theme::action::tooltip(size_t index) const {

	std::stringstream result;
	if (auto_tooltip_ && tooltip_.empty() && items_.size() > index) {
		result << hotkey::get_description(items_[index]);
		if (!hotkey::get_names(items_[index]).empty())
			result << "\n" << _("Hotkey(s): ") << hotkey::get_names(items_[index]);
		result << "\n" << hotkey::get_tooltip(items_[index]);
	} else if (tooltip_name_prepend_ && items_.size() == 1) {
		result << hotkey::get_description(items_[index]);
		if (!hotkey::get_names(items_[index]).empty())
			result << "\n" << _("Hotkey(s): ") << hotkey::get_names(items_[index]);
		result << "\n" << tooltip_;
	} else {
		result << tooltip_;
	}

	return result.str();
}

theme::theme(const config& cfg, const SDL_Rect& screen) :
	theme_reset_event_("theme_reset"),
	cur_theme(),
	cfg_(),
	panels_(),
	labels_(),
	menus_(),
	actions_(),
	context_(),
	status_(),
	main_map_(),
	mini_map_(),
	unit_image_(),
	palette_(),
	border_()
{
	do_resolve_rects(expand_partialresolution(cfg), cfg_);
	set_resolution(screen);
}

bool theme::set_resolution(const SDL_Rect& screen)
{
	bool result = false;

	int current_rating = 1000000;
	const config *current = nullptr;
	for(const config &i : cfg_.child_range("resolution"))
	{
		int width = i["width"];
		int height = i["height"];
		LOG_DP << "comparing resolution " << screen.w << "," << screen.h << " to " << width << "," << height << "\n";
		if(screen.w >= width && screen.h >= height) {
			LOG_DP << "loading theme: " << width << "," << height << "\n";
			current = &i;
			result = true;
			break;
		}

		const int rating = width*height;
		if(rating < current_rating) {
			current = &i;
			current_rating = rating;
		}
	}

	if (!current) {
		if (cfg_.child_count("resolution")) {
			ERR_DP << "No valid resolution found" << std::endl;
		}
		return false;
	}

	std::map<std::string,std::string> title_stash_menus;
	std::vector<theme::menu>::iterator m;
	for (m = menus_.begin(); m != menus_.end(); ++m) {
		if (!m->title().empty() && !m->get_id().empty())
			title_stash_menus[m->get_id()] = m->title();
	}

	std::map<std::string,std::string> title_stash_actions;
	std::vector<theme::action>::iterator a;
	for (a = actions_.begin(); a != actions_.end(); ++a) {
		if (!a->title().empty() && !a->get_id().empty())
			title_stash_actions[a->get_id()] = a->title();
	}

	panels_.clear();
	labels_.clear();
	status_.clear();
	menus_.clear();
	actions_.clear();
	sliders_.clear();

	add_object(*current);

	for (m = menus_.begin(); m != menus_.end(); ++m) {
		if (title_stash_menus.find(m->get_id()) != title_stash_menus.end())
			m->set_title(title_stash_menus[m->get_id()]);
	}

	for (a = actions_.begin(); a != actions_.end(); ++a) {
		if (title_stash_actions.find(a->get_id()) != title_stash_actions.end())
			a->set_title(title_stash_actions[a->get_id()]);
	}

	theme_reset_event_.notify_observers();

	return result;
}

void theme::add_object(const config& cfg)
{
	if (const config &c = cfg.child("main_map")) {
		main_map_ = object(c);
	}

	if (const config &c = cfg.child("mini_map")) {
		mini_map_ = object(c);
	}

	if (const config &c = cfg.child("palette")) {
		palette_ = object(c);
	}

	if (const config &status_cfg = cfg.child("status"))
	{
		for(const config::any_child &i : status_cfg.all_children_range()) {
			status_.emplace(i.key, status_item(i.cfg));
		}
		if (const config &unit_image_cfg = status_cfg.child("unit_image")) {
			unit_image_ = object(unit_image_cfg);
		} else {
			unit_image_ = object();
		}
	}

	for(const config &p : cfg.child_range("panel")) {
		panel new_panel(p);
		set_object_location(new_panel, p["rect"], p["ref"]);
		panels_.push_back(new_panel);
	}

	for(const config &lb : cfg.child_range("label")) {
		label new_label(lb);
		set_object_location(new_label, lb["rect"], lb["ref"]);
		labels_.push_back(new_label);
	}

	for(const config &m : cfg.child_range("menu"))
	{
		menu new_menu(m);
		DBG_DP << "adding menu: " << (new_menu.is_context() ? "is context" : "not context") << "\n";
		if(new_menu.is_context())
			context_ = new_menu;
		else{
			set_object_location(new_menu, m["rect"], m["ref"]);
			menus_.push_back(new_menu);
		}

		DBG_DP << "done adding menu...\n";
	}

	for(const config &a : cfg.child_range("action"))
	{
			action new_action(a);
			DBG_DP << "adding action: " << (new_action.is_context() ? "is context" : "not context") << "\n";
			if(new_action.is_context())
				action_context_ = new_action;
			else{
				set_object_location(new_action, a["rect"], a["ref"]);
				actions_.push_back(new_action);
			}

			DBG_DP << "done adding action...\n";
	}

	for(const config &s : cfg.child_range("slider"))
	{
			slider new_slider(s);
			DBG_DP << "adding slider\n";
			set_object_location(new_slider, s["rect"], s["ref"]);
			sliders_.push_back(new_slider);

			DBG_DP << "done adding slider...\n";
	}

	if (const config &c = cfg.child("main_map_border")) {
		border_ = border_t(c);
	}
}

void theme::remove_object(const std::string& id){
	for(std::vector<theme::panel>::iterator p = panels_.begin(); p != panels_.end(); ++p) {
		if (p->get_id() == id){
			panels_.erase(p);
			return;
		}
	}
	for(std::vector<theme::label>::iterator l = labels_.begin(); l != labels_.end(); ++l) {
		if (l->get_id() == id){
			labels_.erase(l);
			return;
		}
	}
	for(std::vector<theme::menu>::iterator m = menus_.begin(); m != menus_.end(); ++m) {
		if (m->get_id() == id){
			menus_.erase(m);
			return;
		}
	}
	for(std::vector<theme::action>::iterator a = actions_.begin(); a != actions_.end(); ++a) {
		if (a->get_id() == id){
			actions_.erase(a);
			return;
		}
	}
	for(std::vector<theme::slider>::iterator s = sliders_.begin(); s != sliders_.end(); ++s) {
		if (s->get_id() == id){
			sliders_.erase(s);
			return;
		}
	}
}

void theme::set_object_location(theme::object& element, std::string rect_str, std::string ref_id){
	theme::object ref_element = element;
	if (ref_id.empty()) {
		ref_id = element.get_id();
	}
	else {
		ref_element = find_element(ref_id);
	}
	if (ref_element.get_id() == ref_id){
		SDL_Rect location_ref_rect = ref_element.get_location();
		element.modify_location(rect_str, location_ref_rect);
	}
}

void theme::modify(const config &cfg)
{
	std::map<std::string,std::string> title_stash;
	std::vector<theme::menu>::iterator m;
	for (m = menus_.begin(); m != menus_.end(); ++m) {
		if (!m->title().empty() && !m->get_id().empty())
			title_stash[m->get_id()] = m->title();
	}

	std::vector<theme::action>::iterator a;
	for (a = actions_.begin(); a != actions_.end(); ++a) {
		if (!a->title().empty() && !a->get_id().empty())
			title_stash[a->get_id()] = a->title();
	}

	// Change existing theme objects.
	for(const config &c : cfg.child_range("change"))
	{
		std::string id = c["id"];
		std::string ref_id = c["ref"];
		theme::object &element = find_element(id);
		if (element.get_id() == id)
			set_object_location(element, c["rect"], ref_id);
	}

	// Add new theme objects.
	for(const config &c : cfg.child_range("add")) {
		add_object(c);
	}

	// Remove existent theme objects.
	for(const config &c : cfg.child_range("remove")) {
		remove_object(c["id"]);
	}

	for (m = menus_.begin(); m != menus_.end(); ++m) {
		if (title_stash.find(m->get_id()) != title_stash.end())
			m->set_title(title_stash[m->get_id()]);
	}
	for (a = actions_.begin(); a != actions_.end(); ++a) {
		if (title_stash.find(a->get_id()) != title_stash.end())
			a->set_title(title_stash[a->get_id()]);
	}
}

theme::object& theme::find_element(const std::string& id){
	static theme::object empty_object;
	theme::object* res = &empty_object;
	for (std::vector<theme::panel>::iterator p = panels_.begin(); p != panels_.end(); ++p){
		if (p->get_id() == id) { res = &(*p); }
	}
	for (std::vector<theme::label>::iterator l = labels_.begin(); l != labels_.end(); ++l){
		if (l->get_id() == id) { res = &(*l); }
	}
	for (std::vector<theme::menu>::iterator m = menus_.begin(); m != menus_.end(); ++m){
		if (m->get_id() == id) { res = &(*m); }
	}
	for (std::vector<theme::action>::iterator a = actions_.begin(); a != actions_.end(); ++a){
		if (a->get_id() == id) { res = &(*a); }
	}
	if (id == "main-map") { res = &main_map_; }
	if (id == "mini-map") { res = &mini_map_; }
	if (id == "palette") { res = &palette_; }
	if (id == "unit-image") { res = &unit_image_; }
	return *res;
}

const theme::status_item* theme::get_status_item(const std::string& key) const
{
	const std::map<std::string,status_item>::const_iterator i = status_.find(key);
	if(i != status_.end())
		return &i->second;
	else
		return nullptr;
}

typedef std::map<std::string, config> known_themes_map;
known_themes_map theme::known_themes;

void theme::set_known_themes(const config* cfg)
{
	known_themes.clear();
	if (!cfg)
		return;

	for(const config &thm : cfg->child_range("theme"))
	{
		std::string thm_id = thm["id"];

		if (!thm["hidden"].to_bool(false)) {
			known_themes[thm_id] = thm;
		}
	}
}

std::vector<theme_info> theme::get_known_themes()
{
    std::vector<theme_info> res;

	for(known_themes_map::const_iterator i = known_themes.begin();
		i != known_themes.end();
		++i)
	{
		res.push_back(theme_info());
		res.back().id = i->first;
		res.back().name = i->second["name"].t_str();
		res.back().description = i->second["description"].t_str();
	}

	return res;
}

const theme::menu *theme::get_menu_item(const std::string &key) const
{
	for(const theme::menu &m : menus_) {
		if (m.get_id() == key) return &m;
	}
	return nullptr;
}

const theme::action *theme::get_action_item(const std::string &key) const
{
	for(const theme::action &a : actions_) {
		if (a.get_id() == key) return &a;
	}
	return nullptr;
}

theme::object* theme::refresh_title(const std::string& id, const std::string& new_title){

	theme::object* res = nullptr;

	for (std::vector<theme::action>::iterator a = actions_.begin(); a != actions_.end(); ++a){
		if (a->get_id() == id) {
			res = &(*a);
			a->set_title(new_title);
		}
	}

	for (std::vector<theme::menu>::iterator m = menus_.begin(); m != menus_.end(); ++m){
		if (m->get_id() == id) {
			res = &(*m);
			m->set_title(new_title);
		}
	}

	return res;
}

theme::object* theme::refresh_title2(const std::string& id, const std::string& title_tag){
	std::string new_title;

	const config &cfg = find_ref(id, cfg_, false);
	if (! cfg[title_tag].empty())
		new_title = cfg[title_tag].str();

	return refresh_title(id, new_title);
}

void theme::modify_label(const std::string& id, const std::string& text)
{
	theme::label *label = dynamic_cast<theme::label *>(&find_element(id));
	if (!label) {
		LOG_DP << "Theme contains no label called '" << id << "'.\n";
		return;
	}
	label->set_text(text);
}
