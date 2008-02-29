/* $Id$ */
/*
   Copyright (C) 2003 - 2008 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

//! @file theme.cpp
//!

#include "global.hpp"

#include "config.hpp"
#include "font.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "log.hpp"
#include "sdl_utils.hpp"
#include "theme.hpp"
#include "util.hpp"
#include "serialization/string_utils.hpp"
#include "wml_exception.hpp"

#include <cassert>
#include <cstdlib>
#include <sstream>

#define DBG_DP LOG_STREAM(debug, display)
#define LOG_DP LOG_STREAM(info, display)
#define ERR_DP LOG_STREAM(err, display)

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

namespace {
	config empty_config = config();
}

static config& find_ref(const std::string& id, config& cfg, bool remove = false) {
		for(config::child_map::const_iterator i = cfg.all_children().begin();
		    i != cfg.all_children().end(); i++) {
			for (config::child_list::const_iterator j = i->second.begin();
			     j != i->second.end(); j++) {
				if ((**j)["id"] == id) {
					//DBG_DP << "Found a " << *(*i).first << "\n";
					if (remove) {
						const config* const res = cfg.find_child((*i).first,"id",id);
						const size_t index = std::find((*i).second.begin(), (*i).second.end(),
									       res) - (*i).second.begin();
						cfg.remove_child((*i).first,index);
						return empty_config;
					} else {
						return **j;
					}
				}

				// recursively look in children
				config& c = find_ref(id, **j, remove);
				if (!c["id"].empty()) {
					return c;
				}
			}
		}
		// not found
		return empty_config;
	}

#ifdef DEBUG
	// to be called from gdb
	config& find_ref(const char* id, config& cfg) {
		return find_ref(std::string(id),cfg);
	}
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
			while((parent = top_cfg.find_child("resolution", "id", (*parent_id))) == NULL) {
				parent = top_cfg.find_child("partialresolution", "id", (*parent_id));
				if(parent == NULL)
					throw config::error("[partialresolution] refers to non-existant [resolution] " + (*parent_id).str());
				parent_stack.push_back(parent);
				parent_id = &((*parent)["inherits"]);
			}

			// Add the parent resolution and apply all the modifications of its children
			res_cfgs_.push_back(*parent);
			while(!parent_stack.empty()) {
				//override attributes
				for(string_map::const_iterator j = parent_stack.back()->values.begin(); j != parent_stack.back()->values.end(); ++j) {
					res_cfgs_.back().values[j->first] = j->second;
				}

				{
					const config::child_list& c = parent_stack.back()->get_children("remove");
					for(config::child_list::const_iterator j = c.begin(); j != c.end(); ++j) {
						find_ref ((**j)["id"], res_cfgs_.back(), true);
					}
				}
				{
					const config::child_list& c = parent_stack.back()->get_children("change");
					for(config::child_list::const_iterator j = c.begin(); j != c.end(); ++j) {
						config& target = find_ref ((**j)["id"], res_cfgs_.back());
						for(string_map::iterator k = (**j).values.begin();
								k != (**j).values.end(); ++k) {
							target.values[k->first] = k->second;
						}
					}
				}
				{
					// cannot add [status] sub-elements, but who cares
					const config* c = parent_stack.back()->child("add");
					if (c != NULL) {
						const config::child_map m = c->all_children();
						for(config::child_map::const_iterator j = m.begin(); j != m.end(); ++j) {
							for(config::child_list::const_iterator k = j->second.begin();
									k != j->second.end(); ++k) {
								res_cfgs_.back().add_child(j->first, **k);
							}
						}
					}
				}
				parent_stack.pop_back();
			}
		}
		// Add all the resolutions
		const config::child_list& res_list = top_cfg.get_children("resolution");
		for(config::child_list::const_iterator j = res_list.begin(); j != res_list.end(); ++j) {
			dst_cfg.add_child("resolution", (**j));
		}
		// Add all the resolved resolutions
		for(std::vector<config>::const_iterator k = res_cfgs_.begin(); k != res_cfgs_.end(); ++k) {
			dst_cfg.add_child("resolution", (*k));
		}
		return;
	}

static void do_resolve_rects(const config& cfg, config& resolved_config, config* resol_cfg = NULL) {

		// recursively resolve children
		for(config::all_children_iterator i = cfg.ordered_begin(); i != cfg.ordered_end(); ++i) {
			const std::pair<const std::string*,const config*>& value = *i;
			config& childcfg = resolved_config.add_child(*value.first);
			do_resolve_rects(*value.second, childcfg, (*value.first =="resolution") ? &childcfg : resol_cfg);
		}

		// copy all key/values
		for(string_map::const_iterator j = cfg.values.begin(); j != cfg.values.end(); ++j) {
			resolved_config.values[j->first] = j->second;
		}

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
			resolved_config.values["rect"] = resolve_rect(cfg["rect"]);
		}
	}

theme::object::object() : location_modified_(false), loc_(empty_rect), relative_loc_(empty_rect),
                          last_screen_(empty_rect), xanchor_(object::FIXED), yanchor_(object::FIXED)
{
}

theme::object::object(const config& cfg) :
		location_modified_(false), id_(cfg["id"]), loc_(read_sdl_rect(cfg)),
		relative_loc_(empty_rect), last_screen_(empty_rect),
		xanchor_(read_anchor(cfg["xanchor"])), yanchor_(read_anchor(cfg["yanchor"]))
{
}

theme::tborder::tborder() :
		size(0.0)
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
		relative_loc_.w = screen.w - minimum<size_t>(XDim - loc_.w,screen.w);
		break;
	case BOTTOM_ANCHORED:
		relative_loc_.x = screen.w - minimum<size_t>(XDim - loc_.x,screen.w);
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
		relative_loc_.h = screen.h - minimum<size_t>(YDim - loc_.h,screen.h);
		break;
	case BOTTOM_ANCHORED:
		relative_loc_.y = screen.h - minimum<size_t>(YDim - loc_.y,screen.h);
		relative_loc_.h = loc_.h;
		break;
	case PROPORTIONAL:
		relative_loc_.y = (loc_.y*screen.h)/YDim;
		relative_loc_.h = (loc_.h*screen.h)/YDim;
		break;
	default:
		assert(false);
	}

	relative_loc_.x = minimum<int>(relative_loc_.x,screen.w);
	relative_loc_.w = minimum<int>(relative_loc_.w,screen.w - relative_loc_.x);
	relative_loc_.y = minimum<int>(relative_loc_.y,screen.h);
	relative_loc_.h = minimum<int>(relative_loc_.h,screen.h - relative_loc_.y);

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

theme::label::label()
{}

theme::label::label(const config& cfg)
      : object(cfg), text_(cfg["prefix"].str() + cfg["text"].str() + cfg["postfix"].str()),
	icon_(cfg["icon"]), font_(atoi(cfg["font_size"].c_str()))
{
	if(font_ == 0)
		font_ = DefaultFontSize;

	font_rgb_ = DefaultFontRGB;
	font_rgb_set_ = false;
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

theme::status_item::status_item(const config& cfg)
        : object(cfg),
		  prefix_(cfg["prefix"].str() + cfg["prefix_literal"].str()),
		  postfix_(cfg["postfix_literal"].str() + cfg["postfix"].str()),
          font_(atoi(cfg["font_size"].c_str()))
{
	if(font_ == 0)
		font_ = DefaultFontSize;

	const config* const label_child = cfg.child("label");
	if(label_child != NULL) {
		label_ = label(*label_child);
	}

	font_rgb_ = DefaultFontRGB;
	font_rgb_set_ = false;
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

theme::menu::menu() : context_(false)
{}

theme::menu::menu(const config& cfg) : object(cfg), context_(cfg["is_context_menu"] == "true"),
                                       title_(cfg["title"].str() + cfg["title_literal"].str()),
									   tooltip_(cfg["tooltip"]),
						image_(cfg["image"]), type_(cfg["type"]),
						items_(utils::split(cfg["items"]))
{}

theme::theme(const config& cfg, const SDL_Rect& screen) :
	theme_reset_("theme_reset")
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
		if(!resolutions.empty())
			LOG_STREAM(err, display) << "No valid resolution found\n";
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
	
	theme_reset_.notify_observers();
	
	return result;
}

void theme::add_object(const config& cfg){

	const config* const main_map_cfg = cfg.child("main_map");
	if(main_map_cfg != NULL) {
		main_map_ = object(*main_map_cfg);
	}

	const config* const mini_map_cfg = cfg.child("mini_map");
	if(mini_map_cfg != NULL) {
		mini_map_ = object(*mini_map_cfg);
	}

	const config* const status_cfg = cfg.child("status");
	if(status_cfg != NULL) {
		for(config::child_map::const_iterator i = status_cfg->all_children().begin(); i != status_cfg->all_children().end(); ++i) {
			for(config::child_list::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
				status_.insert(std::pair<std::string,status_item>(i->first,status_item(**j)));
			}
		}
		const config* const unit_image_cfg = status_cfg->child("unit_image");
		if (unit_image_cfg != NULL) {
			unit_image_ = object(*unit_image_cfg);
		} else {
			unit_image_ = object();
		}
	}

	const config::child_list& panel_list = cfg.get_children("panel");
	for(config::child_list::const_iterator p = panel_list.begin(); p != panel_list.end(); ++p) {
		panel new_panel(**p);
		set_object_location(new_panel, (**p)["rect"], (**p)["ref"]);
		panels_.push_back(new_panel);
	}

	const config::child_list& label_list = cfg.get_children("label");
	for(config::child_list::const_iterator lb = label_list.begin(); lb != label_list.end(); ++lb) {
		label new_label(**lb);
		set_object_location(new_label, (**lb)["rect"], (**lb)["ref"]);
		labels_.push_back(new_label);
	}

	const config::child_list& menu_list = cfg.get_children("menu");
	for(config::child_list::const_iterator m = menu_list.begin(); m != menu_list.end(); ++m) {
		menu new_menu(**m);
		DBG_DP << "adding menu: " << (new_menu.is_context() ? "is context" : "not context") << "\n";
		if(new_menu.is_context())
			context_ = new_menu;
		else{
			set_object_location(new_menu, (**m)["rect"], (**m)["ref"]);
			menus_.push_back(new_menu);
		}

		DBG_DP << "done adding menu...\n";
	}

	const config* const border_cfg = cfg.child("main_map_border");
	if (border_cfg != NULL) {
		border_ = tborder(*border_cfg);
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

void theme::modify(const config* cfg){
	std::map<std::string,std::string> title_stash;	
	std::vector<theme::menu>::iterator m;
	for (m = menus_.begin(); m != menus_.end(); ++m) {
		if (!m->title().empty() && !m->get_id().empty())
			title_stash[m->get_id()] = m->title();
	}

	{
		// changes to existing theme objects
		const config::child_list& c = cfg->get_children("change");
		for(config::child_list::const_iterator j = c.begin(); j != c.end(); ++j) {
			std::string id = (**j)["id"];
			std::string ref_id = (**j)["ref"];
			theme::object& element = find_element(id);
			if (element.get_id() == id){
				set_object_location(element, (**j)["rect"], ref_id);
			}
		}
	}
	// adding new theme objects
	{
		const config::child_list& c = cfg->get_children("add");
		for(config::child_list::const_iterator j = c.begin(); j != c.end(); ++j) {
			add_object(**j);
		}
	}
	// removing existent theme objects
	{
		const config::child_list& c = cfg->get_children("remove");
		for(config::child_list::const_iterator j = c.begin(); j != c.end(); ++j) {
			remove_object((**j)["id"]);
		}
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
void theme::set_known_themes(const config* cfg){
        known_themes.clear();
        if(cfg == NULL)
	       return;
	const config& v = *cfg;
	const config::child_list& known_themes_cfg = v.get_children("theme");

	for(config::child_list::const_iterator thm = known_themes_cfg.begin(); thm != known_themes_cfg.end(); ++thm) {
	       std::string thm_name=(**thm)["name"];
	       if(thm_name!="null" && thm_name!="editor"){
	              known_themes[thm_name]=(**thm);
	       }
	}
}

std::vector<std::string> theme::get_known_themes(){
        std::vector<std::string> names;


        for(std::map<std::string, config>::iterator p_thm=known_themes.begin();p_thm!=known_themes.end();p_thm++){
	  names.push_back(p_thm->first);
	}
        return(names);
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
	std::string new_title = "";

	config& cfg = find_ref(id, cfg_, false);
	if (! cfg[title_tag].empty())
		new_title = cfg[title_tag];

	return refresh_title(id, new_title);
}
