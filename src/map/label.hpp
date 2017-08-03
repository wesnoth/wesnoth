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

#pragma once

#include "color.hpp"
#include "font/standard_colors.hpp"
#include "map/location.hpp"
#include "tstring.hpp"

#include <SDL_rect.h>
#include <map>
#include <string>

class config;
class display_context;
class team;
class terrain_label;

class map_labels
{
public:
	typedef std::map<map_location, terrain_label*> label_map;
	typedef std::map<std::string, label_map> team_label_map;

	map_labels(const map_labels&);
	map_labels(const team*);
	~map_labels();

	map_labels& operator=(const map_labels&);

	void write(config& res) const;
	void read(const config& cfg);

	const terrain_label* get_label(const map_location& loc, const std::string& team_name) const
	{
		return const_cast<map_labels*>(this)->get_label_private(loc, team_name);
	}

	// search a team-only label, if fails then try public labels
	const terrain_label* get_label(const map_location& loc) const;
	const terrain_label* set_label(const map_location& loc,
			const t_string& text,
			const int creator = -1,
			const std::string& team = "",
			const color_t color = font::NORMAL_COLOR,
			const bool visible_in_fog = true,
			const bool visible_in_shroud = false,
			const bool immutable = false,
			const std::string& category = "",
			const t_string& tooltip = "");

	bool enabled() const
	{
		return enabled_;
	}
	void enable(bool is_enabled);

	void clear(const std::string&, bool force);

	void recalculate_labels();
	void recalculate_shroud();

	bool visible_global_label(const map_location&) const;

	const std::string& team_name() const;
	const std::vector<std::string>& all_categories() const;

	void set_team(const team*);

	void clear_all();

private:
	void add_label(const map_location&, terrain_label*);

	void clear_map(label_map&, bool);
	terrain_label* get_label_private(const map_location& loc, const std::string& team_name);
	// Note: this is not an overload of get_label() so that we do not block
	//       outsiders from calling get_label for a non-const map_labels object.

	const team* team_;

	team_label_map labels_;
	bool enabled_;

	mutable std::vector<std::string> categories;
	mutable bool categories_dirty;
};

/// To store label data
/// Class implements logic for rendering
class terrain_label
{
public:
	terrain_label(const t_string& text,
			const int creator,
			const std::string& team_name,
			const map_location& loc,
			const map_labels& parent,
			const color_t color = font::NORMAL_COLOR,
			const bool visible_in_fog = true,
			const bool visible_in_shroud = false,
			const bool immutable = false,
			const std::string& category = "",
			const t_string& tooltip = "");

	terrain_label(const map_labels&, const config&);

	~terrain_label();

	void write(config& res) const;
	void read(const config& cfg);

	const t_string& text() const
	{
		return text_;
	}

	const t_string& tooltip() const
	{
		return tooltip_;
	}

	int creator() const
	{
		return creator_;
	}

	const std::string& team_name() const
	{
		return team_name_;
	}

	const std::string& category() const
	{
		return category_;
	}

	bool visible_in_fog() const
	{
		return visible_in_fog_;
	}

	bool visible_in_shroud() const
	{
		return visible_in_shroud_;
	}

	bool immutable() const
	{
		return immutable_;
	}

	const map_location& location() const
	{
		return loc_;
	}

	const color_t& color() const
	{
		return color_;
	}

	void set_text(const t_string& text)
	{
		text_ = text;
	}

	void update_info(const t_string&, const int creator, const t_string&, const std::string&, const color_t);

	void update_info(const t_string& text,
			const int creator,
			const t_string& tooltip,
			const std::string& team_name,
			const color_t color,
			const bool visible_in_fog,
			const bool visible_in_shroud,
			const bool immutable,
			const std::string& category);

	void recalculate();
	void calculate_shroud();

private:
	terrain_label(const terrain_label&);
	const terrain_label& operator=(const terrain_label&);

	void clear();
	void draw();
	bool hidden() const;
	bool viewable(const display_context& dc) const;

	int handle_;
	int tooltip_handle_;

	t_string text_;
	t_string tooltip_;

	std::string category_;
	std::string team_name_;

	bool visible_in_fog_;
	bool visible_in_shroud_;
	bool immutable_;
	int creator_;

	color_t color_;

	const map_labels* parent_;
	map_location loc_;

	SDL_Rect get_rect() const;
};
