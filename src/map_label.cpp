/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "display.hpp"
#include "gamestatus.hpp"
#include "map_label.hpp"
#include "resources.hpp"
#include "formula_string_utils.hpp"

#include <boost/foreach.hpp>

//Our definition of map labels being obscured is if the tile is obscured,
//or the tile below is obscured. This is because in the case where the tile
//itself is visible, but the tile below is obscured, the bottom half of the
//tile will still be shrouded, and the label being drawn looks weird.
inline bool is_shrouded(const display& disp, const map_location& loc)
{
	return disp.shrouded(loc) || disp.shrouded(map_location(loc.x,loc.y+1));
}

/// Rather simple test for a hex being fogged.
/// This only exists because is_shrouded() does. (The code looks nicer if
/// the test for being fogged looks similar to the test for being shrouded.)
inline bool is_fogged(const display& disp, const map_location& loc)
{
	return disp.fogged(loc);
}

map_labels::map_labels(const display &disp, const team *team) :
	disp_(disp), team_(team), labels_(), enabled_(true)
{
}

map_labels::map_labels(const map_labels& other) :
	disp_(other.disp_), team_(other.team_), labels_(), enabled_(true)
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
		new (this) map_labels(other);
	}
	return *this;
}

void map_labels::write(config& res) const
{
	for (team_label_map::const_iterator labs = labels_.begin(); labs != labels_.end(); ++labs)
	{
		for(label_map::const_iterator i = labs->second.begin(); i != labs->second.end(); ++i) {
			config item;
			i->second->write(item);


			res.add_child("label",item);
		}
	}
}

void map_labels::read(const config &cfg)
{
	clear_all();

	BOOST_FOREACH(const config &i, cfg.child_range("label"))
	{
		const map_location loc(i, resources::gamedata);
		terrain_label *label = new terrain_label(*this, i);
		add_label(loc, label);
	}
	recalculate_labels();
}

const terrain_label* map_labels::get_label(const map_location& loc, const std::string& team_name) const
{
	team_label_map::const_iterator label_map = labels_.find(team_name);
	if (label_map != labels_.end()) {
		map_labels::label_map::const_iterator itor = label_map->second.find(loc);;
		if (itor != label_map->second.end())
			return itor->second;
	}
	return NULL;
}

const terrain_label* map_labels::get_label(const map_location& loc) const
{
	const terrain_label* res = get_label(loc, team_name());
	// no such team label, we try global label, except if it's what we just did
	// NOTE: This also avoid infinite recursion
	if (res == NULL && team_name() != "") {
		return get_label(loc, "");
	}
	return res;
}


const display& map_labels::disp() const
{
	return disp_;
}

const std::string& map_labels::team_name() const
{
	if (team_)
	{
		return team_->team_name();
	}
	static const std::string empty;
	return empty;
}

void map_labels::set_team(const team* team)
{
	if ( team_ != team )
	{
		team_ = team;
	}
}


const terrain_label* map_labels::set_label(const map_location& loc,
					   const t_string& text,
					   const std::string& team_name,
					   const SDL_Color color,
					   const bool visible_in_fog,
					   const bool visible_in_shroud,
					   const bool immutable)
{
	terrain_label* res = NULL;

	// See if there is already a label in this location for this team.
	// (We do not use get_label_private() here because we might need
	// the label_map as well as the terrain_label.)
	team_label_map::iterator current_label_map = labels_.find(team_name);
	label_map::iterator current_label;

	if ( current_label_map != labels_.end()
			&& (current_label = current_label_map->second.find(loc)) != current_label_map->second.end() )
	{
		// Found old checking if need to erase it
		if(text.str().empty())
		{
			// Erase the old label.
			delete current_label->second;
			current_label_map->second.erase(current_label);

			// Restore the global label in the same spot, if any.
			if ( terrain_label* global_label = get_label_private(loc, "") )
				global_label->recalculate();
		}
		else
		{
			current_label->second->update_info(text, team_name, color, visible_in_fog, visible_in_shroud, immutable);
			res = current_label->second;
		}
	}
	else if(!text.str().empty())
	{
		// See if we will be replacing a global label.
		terrain_label* global_label = get_label_private(loc, "");

		// Add the new label.
		terrain_label* res = new terrain_label(text,
				team_name,
				loc,
				*this,
				color,
				visible_in_fog,
				visible_in_shroud,
				immutable);
		add_label(loc, res);

		// Hide the old label.
		if ( global_label != NULL )
			global_label->recalculate();
	}
	return res;
}

void map_labels::add_label(const map_location &loc, terrain_label *new_label)
{
	labels_[new_label->team_name()][loc] = new_label;
}

void map_labels::clear(const std::string& team_name, bool force)
{
	team_label_map::iterator i = labels_.find(team_name);
	if (i != labels_.end())
	{
		clear_map(i->second, force);
	}

	i = labels_.find("");
	if (i != labels_.end())
	{
		clear_map(i->second, force);
	}
}

void map_labels::clear_map(label_map &m, bool force)
{
	label_map::iterator i = m.begin();
	while (i != m.end())
	{
		if (!i->second->immutable() || force) {
			delete i->second;
			m.erase(i++);
		} else ++i;
	}
}

void map_labels::clear_all()
{
	BOOST_FOREACH(team_label_map::value_type &m, labels_)
	{
		clear_map(m.second, true);
	}
	labels_.clear();
}

void map_labels::recalculate_labels()
{
	BOOST_FOREACH(team_label_map::value_type &m, labels_)
	{
		BOOST_FOREACH(label_map::value_type &l, m.second)
		{
			l.second->recalculate();
		}
	}
}

void map_labels::enable(bool is_enabled) {
	if (is_enabled != enabled_) {
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
	return glabels == labels_.end()
			|| glabels->second.find(loc) == glabels->second.end();
}

void map_labels::recalculate_shroud()
{
	BOOST_FOREACH(team_label_map::value_type &m, labels_)
	{
		BOOST_FOREACH(label_map::value_type &l, m.second)
		{
			l.second->calculate_shroud();
		}
	}
}


/// creating new label
terrain_label::terrain_label(const t_string& text,
							 const std::string& team_name,
							 const map_location& loc,
							 const map_labels& parent,
							 const SDL_Color color,
							 const bool visible_in_fog,
							 const bool visible_in_shroud,
							 const bool immutable)  :
		handle_(0),
		text_(text),
		team_name_(team_name),
		visible_in_fog_(visible_in_fog),
		visible_in_shroud_(visible_in_shroud),
		immutable_(immutable),
		color_(color),
		parent_(&parent),
		loc_(loc)
{
	draw();
}

/// Load label from config
terrain_label::terrain_label(const map_labels &parent, const config &cfg) :
		handle_(0),
		text_(),
		team_name_(),
		visible_in_fog_(true),
		visible_in_shroud_(false),
		immutable_(true),
		color_(),
		parent_(&parent),
		loc_()
{
	read(cfg);
}


terrain_label::~terrain_label()
{
	clear();
}

void terrain_label::read(const config &cfg)
{
	const variable_set &vs = *resources::gamedata;
	loc_ = map_location(cfg, &vs);
	SDL_Color color = font::LABEL_COLOR;

	std::string tmp_color = cfg["color"];

	text_ = cfg["text"];
	team_name_ = cfg["team_name"].str();
	visible_in_fog_ = cfg["visible_in_fog"].to_bool(true);
	visible_in_shroud_ = cfg["visible_in_shroud"].to_bool();
	immutable_ = cfg["immutable"].to_bool(true);

	text_ = utils::interpolate_variables_into_tstring(text_, vs); // Not moved to rendering, as that would depend on variables at render-time
	team_name_ = utils::interpolate_variables_into_string(team_name_, vs);
	tmp_color = utils::interpolate_variables_into_string(tmp_color, vs);

	if(!tmp_color.empty()) {
		std::vector<Uint32> temp_rgb;
		if(string2rgb(tmp_color, temp_rgb) && !temp_rgb.empty()) {
			color = int_to_color(temp_rgb[0]);
		}
	}
	color_ = color;
}

void terrain_label::write(config& cfg) const
{
	loc_.write(cfg);
	cfg["text"] = text();
	cfg["team_name"] = (this->team_name());
	cfg["color"] = cfg_color();
	cfg["visible_in_fog"] = visible_in_fog_;
	cfg["visible_in_shroud"] = visible_in_shroud_;
	cfg["immutable"] = immutable_;
}

const t_string& terrain_label::text() const
{
	return text_;
}

const std::string& terrain_label::team_name() const
{
	return team_name_;
}

bool terrain_label::visible_in_fog() const
{
	return visible_in_fog_;
}

bool terrain_label::visible_in_shroud() const
{
	return visible_in_shroud_;
}

bool terrain_label::immutable() const
{
	return immutable_;
}

const map_location& terrain_label::location() const
{
	return loc_;
}

const SDL_Color& terrain_label::color() const
{
	return color_;
}

std::string terrain_label::cfg_color() const
{
	std::stringstream buf;
	const unsigned int red = static_cast<unsigned int>(color_.r);
	const unsigned int green = static_cast<unsigned int>(color_.g);
	const unsigned int blue = static_cast<unsigned int>(color_.b);
#if SDL_VERSION_ATLEAST(2,0,0)
	const unsigned int alpha = static_cast<unsigned int>(color_.a);
#else
	const unsigned int alpha = static_cast<unsigned int>(color_.unused);
#endif
	buf << red << "," << green << "," << blue << "," << alpha;
	return buf.str();
}

void terrain_label::set_text(const t_string& text)
{
	text_ = text;
}

void terrain_label::update_info(const t_string& text,
								const std::string& team_name,
								const SDL_Color color)
{
	color_ = color;
	text_ = text;
	team_name_ = team_name;
	draw();
}

void terrain_label::update_info(const t_string& text,
								const std::string& team_name,
								const SDL_Color color,
								const bool visible_in_fog,
								const bool visible_in_shroud,
								const bool immutable)
{
	visible_in_fog_ = visible_in_fog;
	visible_in_shroud_ = visible_in_shroud;
	immutable_ = immutable;
	update_info(text, team_name, color);
}

void terrain_label::recalculate()
{
	draw();
}

void terrain_label::calculate_shroud() const
{
	if (handle_)
	{
		font::show_floating_label(handle_, !hidden());
	}
}

void terrain_label::draw()
{
	if (text_.empty())
		return;
	clear();

	if ( !viewable() )
		return;

	const map_location loc_nextx(loc_.x+1,loc_.y);
	const map_location loc_nexty(loc_.x,loc_.y+1);
	const int xloc = (parent_->disp().get_location_x(loc_) +
			parent_->disp().get_location_x(loc_nextx)*2)/3;
	const int yloc = parent_->disp().get_location_y(loc_nexty) - font::SIZE_NORMAL;

	// If a color is specified don't allow to override it with markup. (prevents faking map labels for example)
	// FIXME: @todo Better detect if it's team label and not provided by
	// the scenario.
	bool use_markup = color_ == font::LABEL_COLOR;

	font::floating_label flabel(text_.str());
	flabel.set_color(color_);
	flabel.set_position(xloc, yloc);
	flabel.set_clip_rect(parent_->disp().map_outside_area());
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
	// Fog can hide some labels.
	if ( !visible_in_fog_ && is_fogged(parent_->disp(), loc_) )
		return true;

	// Shroud can hide some labels.
	if ( !visible_in_shroud_ && is_shrouded(parent_->disp(), loc_) )
		return true;

	return false;
}

/**
 * This is a test used to see if we should bother with the overhead of actually
 * creating a label. Conditions that can change during unit movement (disregarding
 * potential WML events) should not be listed here; they belong in hidden().
 */
bool terrain_label::viewable() const
{
	if ( !parent_->enabled() )
		return false;

	// In the editor, all labels are viewable.
	if ( team::nteams() == 0 )
		return true;

	// Observers are not privvy to team labels.
	const bool can_see_team_labels = !is_observer();

	// Global labels are shown unless covered by a team label.
	if ( team_name_.empty() )
		return !can_see_team_labels || parent_->visible_global_label(loc_);

	// Team labels are only shown to members of the team.
	return can_see_team_labels  &&  parent_->team_name() == team_name_;
}

void terrain_label::clear()
{
	if (handle_)
	{
		font::remove_floating_label(handle_);
		handle_ = 0;
	}
}
