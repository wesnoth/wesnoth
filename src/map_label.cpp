/* $Id$ */
/*
   Copyright (C) 2003 - 2007 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include <vector>

#include "display.hpp"
#include "font.hpp"
#include "language.hpp"
#include "map_label.hpp"
#include "wassert.hpp"

namespace {
	const size_t max_label_size = 32;
}

//our definition of map labels being obscured is if the tile is obscured,
//or the tile below is obscured. This is because in the case where the tile
//itself is visible, but the tile below is obscured, the bottom half of the
//tile will still be shrouded, and the label being drawn looks weird
static bool is_shrouded(const display& disp, const gamemap::location& loc)
{
	return disp.shrouded(loc) || disp.shrouded(gamemap::location(loc.x,loc.y+1));
}

map_labels::map_labels(const display& disp,
					   const gamemap& map,
					   const team* team) :
		disp_(disp),
		team_(team),
		map_(map)
{
}

map_labels::map_labels(const display& disp,
		       const config& cfg,
		       const gamemap& map,
		       const team* team,
		       const variable_set *variables) :
		disp_(disp),
		team_(team),
		map_(map)
{
	read(cfg, variables);
}

map_labels::~map_labels()
{
	clear_all();
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

void map_labels::read(const config& cfg, const variable_set *variables)
{
	clear_all();

	const config::child_list& items = cfg.get_children("label");
	for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
		const gamemap::location loc(**i, variables);
		terrain_label* label = new terrain_label(*this, **i, variables);
		add_label(loc, label);
	}
	recalculate_labels();
}


size_t map_labels::get_max_chars()
{
	return max_label_size;
}

const terrain_label* map_labels::get_label(const gamemap::location& loc, const std::string& team_name)
{
	team_label_map::const_iterator label_map = labels_.find(team_name);
	if (label_map != labels_.end()) {
		map_labels::label_map::const_iterator itor = label_map->second.find(loc);;	
		if (itor != label_map->second.end())
			return itor->second;
	}
	return NULL;
}

const terrain_label* map_labels::get_label(const gamemap::location& loc)
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


const terrain_label* map_labels::set_label(const gamemap::location& loc,
					   const std::string& text,
					   const std::string team_name,
					   const SDL_Color colour)
{
	terrain_label* res = 0;
	const team_label_map::const_iterator current_label_map = labels_.find(team_name);
	label_map::const_iterator current_label;

	if ( current_label_map != labels_.end()
			&& (current_label = current_label_map->second.find(loc)) != current_label_map->second.end() )
	{
		// Found old checking if need to erase it
		if(text.empty())
		{
			const_cast<terrain_label*>(current_label->second)->set_text("");
			res = new terrain_label("",team_name,loc,*this,colour);
			delete current_label->second;
			const_cast<label_map&>(current_label_map->second).erase(loc);

			team_label_map::iterator global_label_map = labels_.find("");
			label_map::iterator itor;
			bool update = false;
			if(global_label_map != labels_.end()) {
				itor = global_label_map->second.find(loc);
				update = itor != global_label_map->second.end();
			}
			if (update)
			{
				const_cast<terrain_label*>(itor->second)->recalculate();
			}

		}
		else
		{
			const_cast<terrain_label*>(current_label->second)->update_info(text,
																		   team_name,
																		    colour);
			res = const_cast<terrain_label*>(current_label->second);
		}
	}
	else if(!text.empty())
	{
		team_label_map::iterator global_label_map = labels_.find("");
		label_map::iterator itor;
		bool update = false;
		if(global_label_map != labels_.end()) {
			itor = global_label_map->second.find(loc);
			update = itor != global_label_map->second.end();
		}

		terrain_label* label = new terrain_label(text,
				team_name,
				loc,
				*this,
				colour);
		add_label(loc,label);

		res = label;

		if (update)
		{
			const_cast<terrain_label*>(itor->second)->recalculate();
		}

	}
	return res;
}

				void map_labels::add_label(const gamemap::location& loc,
const terrain_label* new_label)
{
	team_label_map::const_iterator labs = labels_.find(new_label->team_name());
	if(labs == labels_.end())
	{
		labels_.insert(std::pair<std::string,label_map>(new_label->team_name(), label_map()));
		labs = labels_.find(new_label->team_name());

		wassert(labs != labels_.end());
	}

	const_cast<label_map&>(labs->second).insert(std::pair<gamemap::location,const terrain_label*>(loc,new_label));

}

void map_labels::clear(const std::string& team_name)
{
	team_label_map::iterator i = labels_.find(team_name);
	if (i != labels_.end())
	{
		clear_map(i->second);
	}

	i = labels_.find("");
	if (i != labels_.end())
	{
		clear_map(i->second);
	}
}

void map_labels::clear_map(const label_map& m)
{
	for (label_map::const_iterator j = m.begin(); j != m.end(); ++j)
	{
		delete j->second;
	}
	const_cast<label_map&>(m).clear();
}

void map_labels::clear_all()
{
	for(team_label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i)
	{
		clear_map(i->second);
	}
	labels_.clear();
}

void map_labels::scroll(double xmove, double ymove)
{
	for(team_label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i)
	{
		for (label_map::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			j->second->scroll(xmove, ymove);
		}
	}
}

void map_labels::recalculate_labels()
{
	for(team_label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i)
	{
		for (label_map::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			const_cast<terrain_label*>(j->second)->recalculate();
		}
	}
}

bool map_labels::visible_global_label(const gamemap::location& loc) const
{
	const team_label_map::const_iterator glabels = labels_.find(team_name());
	return glabels == labels_.end()
			|| glabels->second.find(loc) == glabels->second.end();
}

void map_labels::recalculate_shroud()
{
	for(team_label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i)
	{
		for (label_map::const_iterator j = i->second.begin(); j != i->second.end(); ++j)
		{
			const_cast<terrain_label*>(j->second)->calculate_shroud();
		}
	}
}


/// creating new label
terrain_label::terrain_label(const std::string& text,
							 const std::string& team_name,
							 const gamemap::location& loc,
							 const map_labels& parent,
							 const SDL_Color colour)  :
		handle_(0),
		text_(text),
		team_name_(team_name),
		colour_(colour),
		parent_(&parent),
		loc_(loc)
{
	check_text_length();
	draw();
}

terrain_label::terrain_label(const map_labels& parent)  :
		handle_(0),
		text_(),
		team_name_(),
		colour_(),
		parent_(&parent),
		loc_()
{
}


/// Load label from config
terrain_label::terrain_label(const map_labels& parent,
			     const config& cfg,
			     const variable_set *variables) :
		handle_(0),
		parent_(&parent)
{
	read(cfg, variables);
	check_text_length();
}


terrain_label::~terrain_label()
{
	clear();
}

void terrain_label::read(const config& cfg, const variable_set *variables)
{
	loc_ = gamemap::location(cfg, variables);
	SDL_Color colour = font::LABEL_COLOUR;
	std::string tmp_colour = cfg["colour"];

	text_      = cfg["text"];
	team_name_ = cfg["team_name"];

	if (variables)
	{
		text_ = utils::interpolate_variables_into_string(
				text_, *variables);
		team_name_ = utils::interpolate_variables_into_string(
					team_name_, *variables);
		tmp_colour = utils::interpolate_variables_into_string(
				  tmp_colour, *variables);
	}

	if(!tmp_colour.empty()) {
		std::vector<Uint32> temp_rgb;
		try {
			temp_rgb = string2rgb(tmp_colour);
		} catch(bad_lexical_cast&) {
			//throw config::error(_("Invalid color range: ") + name);
		}
		if(!temp_rgb.empty()) {
			colour = int_to_color(temp_rgb[0]);
		}
	}
	colour_ = colour;
}

void terrain_label::write(config& cfg) const
{
	loc_.write(cfg);
	cfg["text"] = text();
	cfg["team_name"] = (this->team_name());
	cfg["colour"] = cfg_colour();

}

const std::string& terrain_label::text() const
{
	return text_;
}

const std::string& terrain_label::team_name() const
{
	return team_name_;
}

const gamemap::location& terrain_label::location() const
{
	return loc_;
}

const SDL_Colour& terrain_label::colour() const
{
	return colour_;
}

const std::string terrain_label::cfg_colour() const
{
	std::stringstream buf;
	const unsigned int red = static_cast<unsigned int>(colour_.r);
	const unsigned int green = static_cast<unsigned int>(colour_.g);
	const unsigned int blue = static_cast<unsigned int>(colour_.b);
	const unsigned int alpha = static_cast<unsigned int>(colour_.unused);
	buf << red << ","
			<< green << ","
			<< blue << ","
			<< alpha;
	return buf.str();
}

void terrain_label::set_text(const std::string& text)
{
	text_ = text;
}

void terrain_label::update_info(const std::string& text,
								const std::string& team_name,
								const SDL_Color colour)
{
	colour_ = colour;
	text_ = text;
	check_text_length();
	team_name_ = team_name;
	draw();
}

void terrain_label::scroll(const double xmove,
						   const double ymove) const
{
	if(handle_)
	{
		font::move_floating_label(handle_,
								  xmove,
								  ymove);
	}
}

void terrain_label::recalculate()
{
	draw();
}

void terrain_label::calculate_shroud() const
{

	if (handle_)
	{
		font::show_floating_label(handle_,
								  !is_shrouded(parent_->disp(),
											   loc_));
	}
}

void terrain_label::draw()
{
	clear();
	if (visible())
	{

		const gamemap::location loc_nextx(loc_.x+1,loc_.y);
		const gamemap::location loc_nexty(loc_.x,loc_.y+1);
		const int xloc = (parent_->disp().get_location_x(loc_) +
				parent_->disp().get_location_x(loc_nextx)*2)/3;
		const int yloc = parent_->disp().get_location_y(loc_nexty) - font::SIZE_NORMAL;

		cfg_colour();

		handle_ = font::add_floating_label(text_,
										   font::SIZE_NORMAL,
										   colour_,
										   xloc, yloc,
										   0,0,
										   -1,
										   parent_->disp().map_outside_area());

		calculate_shroud();
	}
}

bool terrain_label::visible() const
{
	return  parent_->team_name() == team_name_
			|| (team_name_.empty() && parent_->visible_global_label(loc_));
}

void terrain_label::check_text_length()
{
	// The actual data is wide_strings so test in wide_string mode
	// also cutting a wide_string at an arbritary place gives odd 
	// problems. 
	utils::truncate_as_wstring(text_, parent_->get_max_chars());
}

void terrain_label::clear()
{
	if (handle_)
	{
		font::remove_floating_label(handle_);
		handle_ = 0;
	}
}
