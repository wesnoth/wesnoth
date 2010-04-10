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
#include "team.hpp"
#include <iostream>

#include <map>
#include <string>

class config;
class display;
class terrain_label;
class label;

class labels
{
public:
	typedef std::map<map_location, boost::shared_ptr<label> > label_map;
	typedef std::map<std::string,label_map> team_label_map;

	labels() : labels_() { std::cerr << "\n constructor labels :" << this << "\n"; };
	labels(const config& cfg) : labels_() { read(cfg); std::cerr << "\n constructor labels :" << this << "\n"; };
	~labels(  ) {


	clear_all();
	std::cerr << "\n destructor labels :" << this << "\n"; };

	std::vector<label> delete_labels(const map_location& loc);

	const label* get_label(const map_location& loc) const;

	virtual const team_label_map& get_team_label_map() { return labels_; }

	void write(config& res) const;
	void read(const config& cfg);

	static size_t get_max_chars(bool ingame = true);

	const label* set_label(const map_location& loc,
							   const std::string& text,
							   const std::string& team = "",
							   const SDL_Color colour = font::NORMAL_COLOUR,
							   const bool visible_in_fog = true,
							   const bool visible_in_shroud = false);

	void add_label(const map_location &, label *);

	virtual void clear_all();
	virtual void clear_map(label_map &);

private:
	team_label_map labels_;

};

class map_labels : public labels
{
public:
	typedef std::map<map_location, terrain_label *> label_map;
//	typedef std::map<map_location, terrain_label *> label_map;


//	typedef std::map<std::string,label_map> team_label_map;
	typedef std::map<std::string,label_map> team_label_map;

	map_labels(const display& disp, const team*);
	~map_labels();

	// search a team-only label, if fails then try public labels
	const terrain_label* get_label(const map_location& loc, const std::string& team_name);
	const terrain_label* get_label(const map_location& loc);

	const terrain_label* set_label(const map_location& loc,
								   const std::string& text,
								   const std::string& team = "",
								   const SDL_Color colour = font::NORMAL_COLOUR,
								   const bool visible_in_fog = true,
								   const bool visible_in_shroud = false,
								   const bool ingame = true);
	void add_label(const map_location &, terrain_label *);

	void read(const config &cfg);

	void clear(const std::string&);

	void clear_all();
	void clear_map(label_map &);

	void scroll(double xmove, double ymove);

	bool visible_global_label(const map_location&) const;

	void recalculate_shroud();

	const display& disp() const;
	void recalculate_labels();

	const std::string& team_name() const;
	void set_team(const team*);

private:
	map_labels(const map_labels&);
//	void clear_map(label_map &);

	const display& disp_;
	const team* team_;
	team_label_map labels_;
};


class label
{
public:
	label(const std::string& text,
			const std::string& team_name,
			const map_location& loc,
			const labels& parent,
			const SDL_Color colour = font::NORMAL_COLOUR,
			const bool visible_in_fog = true,
			const bool visible_in_shroud = false);

	label(const labels &, const config &);

	~label();

	void write(config& res) const;
	void read(const config &cfg);

	const std::string& team_name() const;
	bool visible_in_fog() const;
	bool visible_in_shroud() const;
	const map_location& location() const;
	const SDL_Colour& colour() const;
	const std::string& text() const;

	void set_text(const std::string&);

	void update_info(const std::string&,
					 const std::string&,
					 const SDL_Color);
	label(const label&);
protected:
	void clear();
	int handle_;
	std::string text_;
	std::string team_name_;

	bool visible_in_fog_;
	bool visible_in_shroud_;

	SDL_Color	colour_;
	std::string cfg_colour() const;
	virtual void check_text_length();


	const labels* parent_;
	map_location loc_;
};


/// To store label data
/// Class implements logic for rendering
class terrain_label : public label
{
public:
	terrain_label(const std::string&,
				  const std::string&,
				  const map_location&,
				  const map_labels&,
				  const SDL_Color colour = font::NORMAL_COLOUR,
				  const bool visible_in_fog = true,
				  const bool visible_in_shroud = false,
				  const bool ingame = true);

	terrain_label(const map_labels &, const config &, const bool ingame = true);

	~terrain_label();

	void scroll(double xmove, double ymove) const;

	void recalculate();
	void calculate_shroud() const;

	void update_info(const std::string&,
					 const std::string&,
					 const SDL_Color);

	void check_text_length();

private:
	const terrain_label& operator=(const terrain_label&);
	void draw();
	bool visible() const;

	const map_labels* parent_;
	const bool ingame_;

};

#endif
