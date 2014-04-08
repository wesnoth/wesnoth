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

#ifndef MAP_LABEL_HPP_INCLUDED
#define MAP_LABEL_HPP_INCLUDED

#include "map_location.hpp"
#include "font.hpp"
#include "tstring.hpp"
#include "team.hpp"

class config;
class display;
//class team;
class terrain_label;


class map_labels
{
public:
	typedef std::map<map_location, terrain_label *> label_map;
	typedef std::map<std::string,label_map> team_label_map;

	map_labels(const map_labels&);
	map_labels(const display& disp, const team*);
	~map_labels();

	map_labels& operator=(const map_labels&);

	void write(config& res) const;
	void read(const config &cfg);

	const terrain_label* get_label(const map_location& loc, const std::string& team_name) const;
	// search a team-only label, if fails then try public labels
	const terrain_label* get_label(const map_location& loc) const;
	const terrain_label* set_label(const map_location& loc,
							   const t_string& text,
							   const std::string& team = "",
							   const SDL_Color color = font::NORMAL_COLOR,
							   const bool visible_in_fog = true,
							   const bool visible_in_shroud = false,
							   const bool immutable = false);

	bool enabled() const { return enabled_; }
	void enable(bool is_enabled);

	void add_label(const map_location &, terrain_label *);

	void clear(const std::string&, bool force);

	void recalculate_labels();
	bool visible_global_label(const map_location&) const;

	void recalculate_shroud();

	const display& disp() const;

	const std::string& team_name() const;

	void set_team(const team*);

	void clear_all();

private:
	void clear_map(label_map &, bool);
	/// For our private use, a wrapper for get_label() that can return a pointer
	/// to a non-const terrain_label.
	terrain_label* get_label_private(const map_location& loc, const std::string& team_name)
	{ return const_cast<terrain_label*>(get_label(loc, team_name)); }
	// Note: this is not an overload of get_label() so that we do not block
	//       outsiders from calling get_label for a non-const map_labels object.

	const display& disp_;
	const team* team_;

	team_label_map labels_;
	bool enabled_;
};

/// To store label data
/// Class implements logic for rendering
class terrain_label
{
public:
	terrain_label(const t_string&,
				  const std::string&,
				  const map_location&,
				  const map_labels&,
				  const SDL_Color color = font::NORMAL_COLOR,
				  const bool visible_in_fog = true,
				  const bool visible_in_shroud = false,
				  const bool immutable = false);

	terrain_label(const map_labels &, const config &);

	~terrain_label();

	void write(config& res) const;
	void read(const config &cfg);

	const t_string& text() const;
	const std::string& team_name() const;
	bool visible_in_fog() const;
	bool visible_in_shroud() const;
	bool immutable() const;
	const map_location& location() const;
	const SDL_Color& color() const;

	void set_text(const t_string&);

	void update_info(const t_string&,
					 const std::string&,
					 const SDL_Color);

	void update_info(const t_string& text,
			const std::string& team_name,
			const SDL_Color color,
			const bool visible_in_fog,
			const bool visible_in_shroud,
			const bool immutable);

	void recalculate();
	void calculate_shroud() const;

private:
	terrain_label(const terrain_label&);
	const terrain_label& operator=(const terrain_label&);
	void clear();
	void draw();
	bool hidden() const;
	bool viewable() const;
	std::string cfg_color() const;

	int handle_;

	t_string text_;
	std::string team_name_;
	bool visible_in_fog_;
	bool visible_in_shroud_;
	bool immutable_;

	SDL_Color	color_;

	const map_labels* parent_;
	map_location loc_;

};

#endif
