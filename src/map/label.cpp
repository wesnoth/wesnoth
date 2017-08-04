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

#include "map/label.hpp"
#include "color.hpp"
#include "display.hpp"
#include "floating_label.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_data.hpp"
#include "resources.hpp"
#include "tooltips.hpp"

/**
 * Our definition of map labels being obscured is if the tile is obscured,
 * or the tile below is obscured. This is because in the case where the tile
 * itself is visible, but the tile below is obscured, the bottom half of the
 * tile will still be shrouded, and the label being drawn looks weird.
 */
inline bool is_shrouded(const display* disp, const map_location& loc)
{
	return disp->shrouded(loc) || disp->shrouded(loc.get_direction(map_location::SOUTH));
}

/**
 * Rather simple test for a hex being fogged.
 * This only exists because is_shrouded() does. (The code looks nicer if
 * the test for being fogged looks similar to the test for being shrouded.)
 */
inline bool is_fogged(const display* disp, const map_location& loc)
{
	return disp->fogged(loc);
}

map_labels::map_labels(const team* team)
	: team_(team)
	, labels_()
	, enabled_(true)
	, categories_dirty(true)
{
}

map_labels::map_labels(const map_labels& other)
	: team_(other.team_)
	, labels_()
	, enabled_(true)
{
	config cfg;
	other.write(cfg);
	read(cfg);
}

map_labels::~map_labels()
{
	clear_all();
}

map_labels& map_labels::operator=(const map_labels& other)
{
	if(this != &other) {
		this->~map_labels();
		new(this) map_labels(other);
	}

	return *this;
}

void map_labels::write(config& res) const
{
	for(const auto& group : labels_) {
		for(const auto& label : group.second) {
			config item;
			label.second.write(item);

			res.add_child("label", item);
		}
	}
}

void map_labels::read(const config& cfg)
{
	clear_all();

	for(const config& i : cfg.child_range("label")) {
		add_label(*this, i);
	}

	recalculate_labels();
}

terrain_label* map_labels::get_label_private(const map_location& loc, const std::string& team_name)
{
	auto label_map = labels_.find(team_name);
	if(label_map != labels_.end()) {
		auto itor = label_map->second.find(loc);
		if(itor != label_map->second.end()) {
			return &itor->second;
		}
	}

	return nullptr;
}

const terrain_label* map_labels::get_label(const map_location& loc) const
{
	const terrain_label* res = get_label(loc, team_name());

	// No such team label. Try to find global label, except if that's what we just did.
	// NOTE: This also avoid infinite recursion
	if(res == nullptr && team_name() != "") {
		return get_label(loc, "");
	}

	return res;
}

const std::string& map_labels::team_name() const
{
	if(team_) {
		return team_->team_name();
	}

	static const std::string empty;
	return empty;
}

void map_labels::set_team(const team* team)
{
	if(team_ != team) {
		team_ = team;
		categories_dirty = true;
	}
}

const terrain_label* map_labels::set_label(const map_location& loc,
		const t_string& text,
		const int creator,
		const std::string& team_name,
		const color_t color,
		const bool visible_in_fog,
		const bool visible_in_shroud,
		const bool immutable,
		const std::string& category,
		const t_string& tooltip)
{
	terrain_label* res = nullptr;

	// See if there is already a label in this location for this team.
	// (We do not use get_label_private() here because we might need
	// the label_map as well as the terrain_label.)
	team_label_map::iterator current_label_map = labels_.find(team_name);
	label_map::iterator current_label;

	if(current_label_map != labels_.end() &&
		(current_label = current_label_map->second.find(loc)) != current_label_map->second.end())
	{
		// Found old checking if need to erase it
		if(text.str().empty()) {
			// Erase the old label.
			current_label_map->second.erase(current_label);

			// Restore the global label in the same spot, if any.
			if(terrain_label* global_label = get_label_private(loc, "")) {
				global_label->recalculate();
			}
		} else {
			current_label->second.update_info(
				text, creator, tooltip, team_name, color, visible_in_fog, visible_in_shroud, immutable, category);

			res = &current_label->second;
		}
	} else if(!text.str().empty()) {
		// See if we will be replacing a global label.
		terrain_label* global_label = get_label_private(loc, "");

		// Add the new label.
		res = add_label(
			*this, text, creator, team_name, loc, color, visible_in_fog, visible_in_shroud, immutable, category, tooltip);

		// Hide the old label.
		if(global_label != nullptr) {
			global_label->recalculate();
		}
	}

	categories_dirty = true;
	return res;
}

template<typename... T>
terrain_label* map_labels::add_label(T&&... args)
{
	categories_dirty = true;

	terrain_label t(std::forward<T>(args)...);
	return &(*labels_[t.team_name()].emplace(t.location(), std::move(t)).first).second;
}

void map_labels::clear(const std::string& team_name, bool force)
{
	team_label_map::iterator i = labels_.find(team_name);
	if(i != labels_.end()) {
		clear_map(i->second, force);
	}

	i = labels_.find("");
	if(i != labels_.end()) {
		clear_map(i->second, force);
	}

	categories_dirty = true;
}

void map_labels::clear_map(label_map& m, bool force)
{
	label_map::iterator i = m.begin();
	while(i != m.end()) {
		if(!i->second.immutable() || force) {
			m.erase(i++);
		} else {
			++i;
		}
	}

	categories_dirty = true;
}

void map_labels::clear_all()
{
	labels_.clear();
}

void map_labels::recalculate_labels()
{
	for(auto& m : labels_) {
		for(auto& l : m.second) {
			l.second.recalculate();
		}
	}
}

void map_labels::enable(bool is_enabled)
{
	if(is_enabled != enabled_) {
		enabled_ = is_enabled;
		recalculate_labels();
	}
}

/**
 * Returns whether or not a global (non-team) label can be shown at a
 * specified location.
 * (Global labels are suppressed in favor of team labels.)
 */
bool map_labels::visible_global_label(const map_location& loc) const
{
	const team_label_map::const_iterator glabels = labels_.find(team_name());
	return glabels == labels_.end() || glabels->second.find(loc) == glabels->second.end();
}

void map_labels::recalculate_shroud()
{
	for(auto& m : labels_) {
		for(auto& l : m.second) {
			l.second.calculate_shroud();
		}
	}
}

const std::vector<std::string>& map_labels::all_categories() const
{
	if(categories_dirty) {
		categories_dirty = false;
		categories.clear();
		categories.push_back("team");

		for(size_t i = 1; i <= resources::gameboard->teams().size(); i++) {
			categories.push_back("side:" + std::to_string(i));
		}

		std::set<std::string> unique_cats;
		for(const auto& m : labels_) {
			for(const auto& l : m.second) {
				if(l.second.category().empty()) {
					continue;
				}

				unique_cats.insert("cat:" + l.second.category());
			}
		}

		std::copy(unique_cats.begin(), unique_cats.end(), std::back_inserter(categories));
	}

	return categories;
}

/** Create a new label. */
terrain_label::terrain_label(const map_labels& parent,
		const t_string& text,
		const int creator,
		const std::string& team_name,
		const map_location& loc,
		const color_t color,
		const bool visible_in_fog,
		const bool visible_in_shroud,
		const bool immutable,
		const std::string& category,
		const t_string& tooltip)
	: handle_(0)
	, text_(text)
	, tooltip_(tooltip)
	, category_(category)
	, team_name_(team_name)
	, visible_in_fog_(visible_in_fog)
	, visible_in_shroud_(visible_in_shroud)
	, immutable_(immutable)
	, creator_(creator)
	, color_(color)
	, parent_(&parent)
	, loc_(loc)
{
	draw();
}

/** Load label from config. */
terrain_label::terrain_label(const map_labels& parent, const config& cfg)
	: handle_(0)
	, text_()
	, tooltip_()
	, team_name_()
	, visible_in_fog_(true)
	, visible_in_shroud_(false)
	, immutable_(true)
	, creator_(-1)
	, color_()
	, parent_(&parent)
	, loc_()
{
	read(cfg);
}

terrain_label::~terrain_label()
{
	clear();
}

void terrain_label::read(const config& cfg)
{
	const variable_set& vs = *resources::gamedata;

	loc_ = map_location(cfg, &vs);
	color_t color = font::LABEL_COLOR;

	std::string tmp_color = cfg["color"];

	text_ = cfg["text"];
	tooltip_ = cfg["tooltip"];
	team_name_ = cfg["team_name"].str();
	visible_in_fog_ = cfg["visible_in_fog"].to_bool(true);
	visible_in_shroud_ = cfg["visible_in_shroud"].to_bool();
	immutable_ = cfg["immutable"].to_bool(true);
	category_ = cfg["category"].str();

	int side = cfg["side"].to_int(-1);
	if(side >= 0) {
		creator_ = side - 1;
	} else if(cfg["side"].str() == "current") {
		config::attribute_value current_side = vs.get_variable_const("side_number");
		if(!current_side.empty()) {
			creator_ = current_side.to_int();
		}
	}

	// Not moved to rendering, as that would depend on variables at render-time
	text_ = utils::interpolate_variables_into_tstring(text_, vs);

	team_name_ = utils::interpolate_variables_into_string(team_name_, vs);
	tmp_color = utils::interpolate_variables_into_string(tmp_color, vs);

	if(!tmp_color.empty()) {
		try {
			color = color_t::from_rgb_string(tmp_color);
		} catch(std::invalid_argument&) {
			// Prior to the color_t conversion, labels were written to savefiles with an alpha key, despite alpha not
			// being accepted in color=. Because of this, this enables the loading of older saves without an exception
			// throwing.
			color = color_t::from_rgba_string(tmp_color);
		}
	}

	color_ = color;
}

void terrain_label::write(config& cfg) const
{
	loc_.write(cfg);

	cfg["text"] = text();
	cfg["tooltip"] = tooltip();
	cfg["team_name"] = (this->team_name());
	cfg["color"] = color_.to_rgb_string();
	cfg["visible_in_fog"] = visible_in_fog_;
	cfg["visible_in_shroud"] = visible_in_shroud_;
	cfg["immutable"] = immutable_;
	cfg["category"] = category_;
	cfg["side"] = creator_ + 1;
}

void terrain_label::update_info(const t_string& text,
		const int creator,
		const t_string& tooltip,
		const std::string& team_name,
		const color_t color)
{
	color_ = color;
	text_ = text;
	tooltip_ = tooltip;
	team_name_ = team_name;
	creator_ = creator;

	draw();
}

void terrain_label::update_info(const t_string& text,
		const int creator,
		const t_string& tooltip,
		const std::string& team_name,
		const color_t color,
		const bool visible_in_fog,
		const bool visible_in_shroud,
		const bool immutable,
		const std::string& category)
{
	visible_in_fog_ = visible_in_fog;
	visible_in_shroud_ = visible_in_shroud;
	immutable_ = immutable;
	category_ = category;

	update_info(text, creator, tooltip, team_name, color);
}

void terrain_label::recalculate()
{
	draw();
}

void terrain_label::calculate_shroud()
{
	if(handle_) {
		font::show_floating_label(handle_, !hidden());
	}

	if(tooltip_.empty() || hidden()) {
		tooltips::remove_tooltip(tooltip_handle_);
		tooltip_handle_ = 0;
		return;
	}

	// tooltips::update_tooltip(tooltip_handle, get_rect(), tooltip_.str(), "", true);

	if(tooltip_handle_) {
		tooltips::update_tooltip(tooltip_handle_, get_rect(), tooltip_.str(), "", true);
	} else {
		tooltip_handle_ = tooltips::add_tooltip(get_rect(), tooltip_.str());
	}
}

SDL_Rect terrain_label::get_rect() const
{
	SDL_Rect rect {0, 0, 0, 0};

	display* disp = display::get_singleton();
	if(!disp) {
		return rect;
	}

	int hex_size = disp->hex_size();

	rect.x = disp->get_location_x(loc_) + hex_size / 4;
	rect.y = disp->get_location_y(loc_);
	rect.h = disp->hex_size();
	rect.w = disp->hex_size() - hex_size / 2;

	return rect;
}

void terrain_label::draw()
{
	display* disp = display::get_singleton();
	if(!disp) {
		return;
	}

	if(text_.empty() && tooltip_.empty()) {
		return;
	}

	clear();

	if(!viewable(disp->get_disp_context())) {
		return;
	}

	// Note: the y part of loc_nextx is not used at all.
	const map_location loc_nextx = loc_.get_direction(map_location::NORTH_EAST);
	const map_location loc_nexty = loc_.get_direction(map_location::SOUTH);
	const int xloc = (disp->get_location_x(loc_) + disp->get_location_x(loc_nextx) * 2) / 3;
	const int yloc = disp->get_location_y(loc_nexty) - font::SIZE_NORMAL;

	// If a color is specified don't allow to override it with markup. (prevents faking map labels for example)
	// FIXME: @todo Better detect if it's team label and not provided by the scenario.
	bool use_markup = color_ == font::LABEL_COLOR;

	font::floating_label flabel(text_.str());
	flabel.set_color(color_);
	flabel.set_position(xloc, yloc);
	flabel.set_clip_rect(disp->map_outside_area());
	flabel.set_width(font::SIZE_NORMAL * 13);
	flabel.set_height(font::SIZE_NORMAL * 4);
	flabel.set_scroll_mode(font::ANCHOR_LABEL_MAP);
	flabel.use_markup(use_markup);

	handle_ = font::add_floating_label(flabel);

	calculate_shroud();
}

/**
 * This is a lightweight test used to see if labels are revealed as a result
 * of unit actions (i.e. fog/shroud clearing). It should not contain any tests
 * that are invariant during unit movement (disregarding potential WML events);
 * those belong in visible().
 */
bool terrain_label::hidden() const
{
	display* disp = display::get_singleton();
	if(!disp) {
		return false;
	}

	// Respect user's label preferences
	std::string category = "cat:" + category_;
	std::string creator = "side:" + std::to_string(creator_ + 1);
	const std::vector<std::string>& hidden_categories = disp->get_disp_context().hidden_label_categories();

	if(std::find(hidden_categories.begin(), hidden_categories.end(), category) != hidden_categories.end()) {
		return true;
	}

	if(creator_ >= 0 &&
		std::find(hidden_categories.begin(), hidden_categories.end(), creator) != hidden_categories.end())
	{
		return true;
	}

	if(!team_name().empty() &&
		std::find(hidden_categories.begin(), hidden_categories.end(), "team") != hidden_categories.end())
	{
		return true;
	}

	// Fog can hide some labels.
	if(!visible_in_fog_ && is_fogged(disp, loc_)) {
		return true;
	}

	// Shroud can hide some labels.
	if(!visible_in_shroud_ && is_shrouded(disp, loc_)) {
		return true;
	}

	return false;
}

/**
 * This is a test used to see if we should bother with the overhead of actually
 * creating a label. Conditions that can change during unit movement (disregarding
 * potential WML events) should not be listed here; they belong in hidden().
 */
bool terrain_label::viewable(const display_context& dc) const
{
	if(!parent_->enabled()) {
		return false;
	}

	// In the editor, all labels are viewable.
	if(dc.teams().empty()) {
		return true;
	}

	// Observers are not privvy to team labels.
	const bool can_see_team_labels = !dc.is_observer();

	// Global labels are shown unless covered by a team label.
	if(team_name_.empty()) {
		return !can_see_team_labels || parent_->visible_global_label(loc_);
	}

	// Team labels are only shown to members of the team.
	return can_see_team_labels && parent_->team_name() == team_name_;
}

void terrain_label::clear()
{
	if(handle_) {
		font::remove_floating_label(handle_);
		handle_ = 0;
	}

	if(tooltip_handle_) {
		tooltips::remove_tooltip(tooltip_handle_);
		tooltip_handle_ = 0;
	}
}
