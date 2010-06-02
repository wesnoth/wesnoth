/* $Id$ */
/*
   Copyright (C) 2003 - 2010 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef MAP_LABEL_HPP_INCLUDED
#define MAP_LABEL_HPP_INCLUDED

#include "map_location.hpp"
#include "font.hpp"

#include <map>
#include <string>

class config;
class display;
class team;
class terrain_label;


class map_labels
{
public:
	typedef std::map<map_location, terrain_label *> label_map;
	typedef std::map<std::string,label_map> team_label_map;

	map_labels(const display& disp, const team*);
	~map_labels();

	void write(config& res) const;
	void read(const config &cfg);

	static size_t get_max_chars();

	const terrain_label* get_label(const map_location& loc, const std::string& team_name);
	// search a team-only label, if fails then try public labels
	const terrain_label* get_label(const map_location& loc);
	const terrain_label* set_label(const map_location& loc,
							   const t_string& text,
							   const std::string& team = "",
							   const SDL_Color colour = font::NORMAL_COLOUR,
							   const bool visible_in_fog = true,
							   const bool visible_in_shroud = false,
							   const bool immutable = false);

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
	map_labels(const map_labels&);
	void operator=(const map_labels&);

	const display& disp_;
	const team* team_;

	team_label_map labels_;
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
				  const SDL_Color colour = font::NORMAL_COLOUR,
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
	const SDL_Color& colour() const;

	void set_text(const t_string&);

	void update_info(const t_string&,
					 const std::string&,
					 const SDL_Color);

	void recalculate();
	void calculate_shroud() const;

private:
	terrain_label(const terrain_label&);
	const terrain_label& operator=(const terrain_label&);
	void clear();
	void draw();
	bool visible() const;
	void check_text_length();
	std::string cfg_colour() const;

	int handle_;

	t_string text_;
	std::string team_name_;
	bool visible_in_fog_;
	bool visible_in_shroud_;
	bool immutable_;

	SDL_Color	colour_;

	const map_labels* parent_;
	map_location loc_;

};

#endif
