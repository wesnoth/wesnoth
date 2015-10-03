/* $Id$ */
/*
   Copyright (C) 2003 - 2011 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/** @file theme.cpp */

#include "global.hpp"

#include "font.hpp"
#include "foreach.hpp"
#include "gettext.hpp"
#include "hotkeys.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "theme.hpp"
#include "wml_exception.hpp"


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
		const std::vector<std::string> items = utils::split(cfg["rect"].c_str());
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
	for (config::all_children_iterator i = itors.first; i != itors.second; ++i)
	{
		config &icfg = const_cast<config &>(i->cfg);
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

static void expand_partialresolution(config& dst_cfg, const config& top_cfg)
	{
		std::vector<config> res_cfgs_;
		// resolve all the partialresolutions
		const config::child_list& parts_list = top_cfg.get_children("partialresolution");
		for(config::child_list::const_iterator i = parts_list.begin(); i != parts_list.end(); ++i) {
			// follow the inheritance hierarchy and push all the nodes on the stack
			std::vector<const config*> parent_stack(1, (*i));
			const config* parent;
			const t_string* parent_id = &((**i)["inherits"]);
			while (!*(parent = &top_cfg.find_child("resolution", "id", (*parent_id)))) {
				parent = &top_cfg.find_child("partialresolution", "id", (*parent_id));
				if (!*parent)
					throw config::error("[partialresolution] refers to non-existant [resolution] " + (*parent_id).str());
				parent_stack.push_back(parent);
				parent_id = &((*parent)["inherits"]);
			}

			// Add the parent resolution and apply all the modifications of its children
			res_cfgs_.push_back(*parent);
			while(!parent_stack.empty()) {
				//override attributes
				res_cfgs_.back().merge_attributes(*parent_stack.back());

				BOOST_FOREACH (const config &rm, parent_stack.back()->child_range("remove")) {
					find_ref(rm["id"], res_cfgs_.back(), true);
				}

				BOOST_FOREACH (const config &chg, parent_stack.back()->child_range("change"))
				{
					config &target = find_ref(chg["id"], res_cfgs_.back());
					target.merge_attributes(chg);
				}

				// cannot add [status] sub-elements, but who cares
				if (const config &c = parent_stack.back()->child("add"))
				{
					BOOST_FOREACH (const config::any_child &j, c.all_children_range()) {
						res_cfgs_.back().add_child(j.key, j.cfg);
					}
				}

				parent_stack.pop_back();
			}
		}
		// Add all the resolutions
		BOOST_FOREACH (const config &res, top_cfg.child_range("resolution")) {
			dst_cfg.add_child("resolution", res);
		}
		// Add all the resolved resolutions
		for(std::vector<config>::const_iterator k = res_cfgs_.begin(); k != res_cfgs_.end(); ++k) {
			dst_cfg.add_child("resolution", (*k));
		}
		return;
	}

static void do_resolve_rects(const config& cfg, config& resolved_config, config* resol_cfg = NULL) {

		// recursively resolve children
		BOOST_FOREACH (const config::any_child &value, cfg.all_children_range()) {
			config &childcfg = resolved_config.add_child(value.key);
			do_resolve_rects(value.cfg, childcfg,
				value.key == "resolution" ? &childcfg : resol_cfg);
		}

		// copy all key/values
		resolved_config.merge_attributes(cfg);

		// override default reference rect with "ref" parameter if any
		if (!cfg["ref"].empty()) {
			if (resol_cfg == NULL) {
				ERR_DP << "Use of ref= outside a [resolution] block\n";
			} else {
				//DBG_DP << ">> Looking for " << cfg["ref"] << "\n";
				const config ref = find_ref (cfg["ref"], *resol_cfg);

				if (ref["id"].empty()) {
					ERR_DP << "Reference to non-existent rect id \"" << cfg["ref"] << "\"\n";
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
	loc_(empty_rect),
	relative_loc_(empty_rect),
	last_screen_(empty_rect),
	xanchor_(object::FIXED),
	yanchor_(object::FIXED)
{
}

theme::object::object(const config& cfg) :
		location_modified_(false), id_(cfg["id"]), loc_(read_sdl_rect(cfg)),
		relative_loc_(empty_rect), last_screen_(empty_rect),
		xanchor_(read_anchor(cfg["xanchor"])), yanchor_(read_anchor(cfg["yanchor"]))
{
}

theme::tborder::tborder() :
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

theme::tborder::tborder(const config& cfg) :
	size(lexical_cast_default<double>(cfg["border_size"], 0.0)),

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
							 fixed_anchor = "fixed", proportional_anchor = "proportional";
	if(str == top_anchor || str == left_anchor)
		return TOP_ANCHORED;
	else if(str == bot_anchor || str == right_anchor)
		return BOTTOM_ANCHORED;
	else if(str == proportional_anchor)
		return PROPORTIONAL;
	else
		return FIXED;
}

void theme::object::modify_location(const _rect rect){
	loc_.x = rect.x1;
	loc_.y = rect.y1;
	loc_.w = rect.x2 - rect.x1;
	loc_.h = rect.y2 - rect.y1;
	location_modified_ = true;
}

void theme::object::modify_location(std::string rect_str, SDL_Rect ref_rect){
	_rect rect = { 0, 0, 0, 0 };
	const std::vector<std::string> items = utils::split(rect_str.c_str());
	if(items.size() >= 1) {
		rect.x1 = compute(items[0], ref_rect.x, ref_rect.x + ref_rect.w);
	}
	if(items.size() >= 2) {
		rect.y1 = compute(items[1], ref_rect.y, ref_rect.y + ref_rect.h);
	}
	if(items.size() >= 3) {
		rect.x2 = compute(items[2], ref_rect.x + ref_rect.w, rect.x1);
	}
	if(items.size() >= 4) {
		rect.y2 = compute(items[3], ref_rect.y + ref_rect.h, rect.y1);
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
	font_(atoi(cfg["font_size"].c_str())),
	font_rgb_set_(false),
	font_rgb_(DefaultFontRGB)
{
	if(font_ == 0)
		font_ = DefaultFontSize;

	if(cfg["font_rgb"].size()){
	std::vector<std::string> rgb_vec = utils::split(cfg["font_rgb"]);
	  if(3 <= rgb_vec.size()){
	    std::vector<std::string>::iterator c=rgb_vec.begin();
	    int r,g,b;
	    r = (atoi(c->c_str()));
	    c++;
	    if(c != rgb_vec.end()){
	      g = (atoi(c->c_str()));
	    }else{
	      g=0;
	    }
	    c++;
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

theme::status_item::status_item(const config& cfg) :
	object(cfg),
	prefix_(cfg["prefix"].str() + cfg["prefix_literal"].str()),
	postfix_(cfg["postfix_literal"].str() + cfg["postfix"].str()),
	label_(),
	font_(atoi(cfg["font_size"].c_str())),
	font_rgb_set_(false),
	font_rgb_(DefaultFontRGB)
{
	if(font_ == 0)
		font_ = DefaultFontSize;

	if (const config &label_child = cfg.child("label")) {
		label_ = label(label_child);
	}

	if(cfg["font_rgb"].size()){
	  std::vector<std::string> rgb_vec = utils::split(cfg["font_rgb"]);
	  if(3 <= rgb_vec.size()){
	    std::vector<std::string>::iterator c=rgb_vec.begin();
	    int r,g,b;
	    r = (atoi(c->c_str()));
	    c++;
	    if(c != rgb_vec.end()){
	      g = (atoi(c->c_str()));
	    }else{
	      g=0;
	    }
	    c++;
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

theme::menu::menu() :
	object(),
	context_(false),
	title_(),
	tooltip_(),
	image_(),
	type_(),
	items_()
{}

theme::menu::menu(const config& cfg) : object(cfg),
                                       context_(utils::string_bool(cfg["is_context_menu"])),
                                       title_(cfg["title"].str() + cfg["title_literal"].str()),
									   tooltip_(cfg["tooltip"]),
									   image_(cfg["image"]), type_(cfg["type"]),
									   items_(utils::split(cfg["items"]))
{
	if (utils::string_bool(cfg["auto_tooltip"]) && tooltip_.empty() && items_.size() == 1) {
		tooltip_ = hotkey::get_hotkey(items_[0]).get_description();
	} else if (utils::string_bool(cfg["tooltip_name_prepend"]) && items_.size() == 1) {
		tooltip_ = hotkey::get_hotkey(items_[0]).get_description() + "\n" + tooltip_;
	}
}

theme::theme(const config& cfg, const SDL_Rect& screen) :
	theme_reset_event_("theme_reset"),
	cur_theme(),
	cfg_(),
	panels_(),
	labels_(),
	menus_(),
	context_(),
	status_(),
	main_map_(),
	mini_map_(),
	unit_image_(),
	border_()
{
	config tmp;
	expand_partialresolution(tmp, cfg);
	do_resolve_rects(tmp, cfg_);
	set_resolution(screen);
}

bool theme::set_resolution(const SDL_Rect& screen)
{
	bool result = false;

	const config::child_list& resolutions = cfg_.get_children("resolution");
	int current_rating = 1000000;
	config::child_list::const_iterator i;
	config::child_list::const_iterator current = resolutions.end();
	for(i = resolutions.begin(); i != resolutions.end(); ++i) {
		const int width = atoi((**i)["width"].c_str());
		const int height = atoi((**i)["height"].c_str());
		LOG_DP << "comparing resolution " << screen.w << "," << screen.h << " to " << width << "," << height << "\n";
		if(screen.w >= width && screen.h >= height) {
			LOG_DP << "loading theme: " << width << "," << height << "\n";
			current = i;
			result = true;
			break;
		}

		const int rating = width*height;
		if(rating < current_rating) {
			current = i;
			current_rating = rating;
		}
	}

	if(current == resolutions.end()) {
		if(!resolutions.empty()) {
			ERR_DP << "No valid resolution found\n";
		}
		return false;
	}

	std::map<std::string,std::string> title_stash;
	std::vector<theme::menu>::iterator m;
	for (m = menus_.begin(); m != menus_.end(); ++m) {
		if (!m->title().empty() && !m->get_id().empty())
			title_stash[m->get_id()] = m->title();
	}

	panels_.clear();
	labels_.clear();
	status_.clear();
	menus_.clear();

	const config& cfg = **current;
	add_object(cfg);

	for (m = menus_.begin(); m != menus_.end(); ++m) {
		if (title_stash.find(m->get_id()) != title_stash.end())
			m->set_title(title_stash[m->get_id()]);
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

	if (const config &status_cfg = cfg.child("status"))
	{
		BOOST_FOREACH (const config::any_child &i, status_cfg.all_children_range()) {
			status_.insert(std::pair<std::string, status_item>(i.key, status_item(i.cfg)));
		}
		if (const config &unit_image_cfg = status_cfg.child("unit_image")) {
			unit_image_ = object(unit_image_cfg);
		} else {
			unit_image_ = object();
		}
	}

	BOOST_FOREACH (const config &p, cfg.child_range("panel")) {
		panel new_panel(p);
		set_object_location(new_panel, p["rect"], p["ref"]);
		panels_.push_back(new_panel);
	}

	BOOST_FOREACH (const config &lb, cfg.child_range("label")) {
		label new_label(lb);
		set_object_location(new_label, lb["rect"], lb["ref"]);
		labels_.push_back(new_label);
	}

	BOOST_FOREACH (const config &m, cfg.child_range("menu"))
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

	if (const config &c = cfg.child("main_map_border")) {
		border_ = tborder(c);
	} else {
		border_ = tborder();
	}
}

void theme::remove_object(std::string id){
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
		SDL_Rect ref_rect = ref_element.get_location();
		element.modify_location(rect_str, ref_rect);
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

	// Change existing theme objects.
	BOOST_FOREACH (const config &c, cfg.child_range("change"))
	{
		std::string id = c["id"];
		std::string ref_id = c["ref"];
		theme::object &element = find_element(id);
		if (element.get_id() == id)
			set_object_location(element, c["rect"], ref_id);
	}

	// Add new theme objects.
	BOOST_FOREACH (const config &c, cfg.child_range("add")) {
		add_object(c);
	}

	// Remove existent theme objects.
	BOOST_FOREACH (const config &c, cfg.child_range("remove")) {
		remove_object(c["id"]);
	}

	for (m = menus_.begin(); m != menus_.end(); ++m) {
		if (title_stash.find(m->get_id()) != title_stash.end())
			m->set_title(title_stash[m->get_id()]);
	}
}

theme::object& theme::find_element(std::string id){
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
	if (id == "main-map") { res = &main_map_; }
	if (id == "mini-map") { res = &mini_map_; }
	if (id == "unit-image") { res = &unit_image_; }
	return *res;
}

const theme::status_item* theme::get_status_item(const std::string& key) const
{
	const std::map<std::string,status_item>::const_iterator i = status_.find(key);
	if(i != status_.end())
		return &i->second;
	else
		return NULL;
}

std::map<std::string, config> theme::known_themes;
void theme::set_known_themes(const config* cfg)
{
	known_themes.clear();
	if (!cfg)
		return;

	BOOST_FOREACH (const config &thm, cfg->child_range("theme"))
	{
		std::string thm_name = thm["name"];
		if (thm_name != "null" && thm_name != "editor")
			known_themes[thm_name] = thm;
	}
}

std::vector<std::string> theme::get_known_themes(){
    std::vector<std::string> names;


    for(std::map<std::string, config>::iterator p_thm=known_themes.begin();p_thm!=known_themes.end();++p_thm){
        names.push_back(p_thm->first);
    }
    return(names);
}

const theme::menu *theme::get_menu_item(const std::string &key) const
{
	BOOST_FOREACH (const theme::menu &m, menus_) {
		if (m.get_id() == key) return &m;
	}
	return NULL;
}


theme::menu* theme::refresh_title(const std::string& id, const std::string& new_title){
	theme::menu* res = NULL;

	for (std::vector<theme::menu>::iterator m = menus_.begin(); m != menus_.end(); ++m){
		if (m->get_id() == id) {
			res = &(*m);
			res->set_title(new_title);
		}
	}

	return res;
}

theme::menu* theme::refresh_title2(const std::string& id, const std::string& title_tag){
	std::string new_title;

	config& cfg = find_ref(id, cfg_, false);
	if (! cfg[title_tag].empty())
		new_title = cfg[title_tag];

	return refresh_title(id, new_title);
}

void theme::modify_label(const std::string& id, const std::string& text)
{
	theme::label *label = dynamic_cast<theme::label *>(&find_element(id));
	if (!label) {
		ERR_DP << "Theme contains no label called '" << id << "'.\n";
		return;
	}
	label->set_text(text);
}
