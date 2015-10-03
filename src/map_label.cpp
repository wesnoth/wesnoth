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

#include "global.hpp"

#include "display.hpp"
#include "foreach.hpp"
#include "gamestatus.hpp"
#include "map_label.hpp"
#include "resources.hpp"
#include "formula_string_utils.hpp"


namespace {
	const size_t max_label_size = 32;
}

//our definition of map labels being obscured is if the tile is obscured,
//or the tile below is obscured. This is because in the case where the tile
//itself is visible, but the tile below is obscured, the bottom half of the
//tile will still be shrouded, and the label being drawn looks weird
static bool is_shrouded(const display& disp, const map_location& loc)
{
	return disp.shrouded(loc) || disp.shrouded(map_location(loc.x,loc.y+1));
}

map_labels::map_labels(const display &disp, const team *team) :
	disp_(disp), team_(team), labels_()
{
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

void map_labels::read(const config &cfg)
{
	clear_all();

	BOOST_FOREACH (const config &i, cfg.child_range("label"))
	{
		const map_location loc(i, resources::state_of_game);
		terrain_label *label = new terrain_label(*this, i);
		add_label(loc, label);
	}
	recalculate_labels();
}


size_t map_labels::get_max_chars()
{
	return max_label_size;
}

const terrain_label* map_labels::get_label(const map_location& loc, const std::string& team_name)
{
	team_label_map::const_iterator label_map = labels_.find(team_name);
	if (label_map != labels_.end()) {
		map_labels::label_map::const_iterator itor = label_map->second.find(loc);;
		if (itor != label_map->second.end())
			return itor->second;
	}
	return NULL;
}

const terrain_label* map_labels::get_label(const map_location& loc)
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
					   const std::string& text,
					   const std::string& team_name,
					   const SDL_Color colour,
					   const bool visible_in_fog,
					   const bool visible_in_shroud)
{
	terrain_label* res = 0;
	team_label_map::iterator current_label_map = labels_.find(team_name);
	label_map::iterator current_label;

	if ( current_label_map != labels_.end()
			&& (current_label = current_label_map->second.find(loc)) != current_label_map->second.end() )
	{
		// Found old checking if need to erase it
		if(text.empty())
		{
			current_label->second->set_text("");
			res = new terrain_label("",team_name,loc,*this,colour,visible_in_fog,visible_in_shroud);
			delete current_label->second;
			current_label_map->second.erase(loc);

			team_label_map::iterator global_label_map = labels_.find("");
			label_map::iterator itor;
			bool update = false;
			if(global_label_map != labels_.end()) {
				itor = global_label_map->second.find(loc);
				update = itor != global_label_map->second.end();
			}
			if (update)
			{
				itor->second->recalculate();
			}

		}
		else
		{
			current_label->second->update_info(text, team_name, colour);
			res = current_label->second;
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
				colour,
				visible_in_fog,
				visible_in_shroud);
		add_label(loc,label);

		res = label;

		if (update)
		{
			itor->second->recalculate();
		}

	}
	return res;
}

void map_labels::add_label(const map_location &loc, terrain_label *new_label)
{
	labels_[new_label->team_name()][loc] = new_label;
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

void map_labels::clear_map(label_map &m)
{
	BOOST_FOREACH (label_map::value_type &v, m)
	{
		delete v.second;
	}
	m.clear();
}

void map_labels::clear_all()
{
	BOOST_FOREACH (team_label_map::value_type &m, labels_)
	{
		clear_map(m.second);
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
	BOOST_FOREACH (team_label_map::value_type &m, labels_)
	{
		BOOST_FOREACH (label_map::value_type &l, m.second)
		{
			l.second->recalculate();
		}
	}
}

bool map_labels::visible_global_label(const map_location& loc) const
{
	const team_label_map::const_iterator glabels = labels_.find(team_name());
	return glabels == labels_.end()
			|| glabels->second.find(loc) == glabels->second.end();
}

void map_labels::recalculate_shroud()
{
	BOOST_FOREACH (team_label_map::value_type &m, labels_)
	{
		BOOST_FOREACH (label_map::value_type &l, m.second)
		{
			l.second->calculate_shroud();
		}
	}
}


/// creating new label
terrain_label::terrain_label(const std::string& text,
							 const std::string& team_name,
							 const map_location& loc,
							 const map_labels& parent,
							 const SDL_Color colour,
							 const bool visible_in_fog,
							 const bool visible_in_shroud)  :
		handle_(0),
		text_(text),
		team_name_(team_name),
		visible_in_fog_(visible_in_fog),
		visible_in_shroud_(visible_in_shroud),
		colour_(colour),
		parent_(&parent),
		loc_(loc)
{
	check_text_length();
	draw();
}

/// Load label from config
terrain_label::terrain_label(const map_labels &parent, const config &cfg) :
		handle_(0),
		text_(),
		team_name_(),
		visible_in_fog_(true),
		visible_in_shroud_(false),
		colour_(),
		parent_(&parent),
		loc_()
{
	read(cfg);
	check_text_length();
}


terrain_label::~terrain_label()
{
	clear();
}

void terrain_label::read(const config &cfg)
{
	const variable_set &vs = *resources::state_of_game;
	loc_ = map_location(cfg, &vs);
	SDL_Color colour = font::LABEL_COLOUR;
	std::string tmp_colour = cfg["colour"];

	text_      = cfg["text"];
	team_name_ = cfg["team_name"];
	visible_in_fog_ = utils::string_bool(cfg["visible_in_fog"],true);
	visible_in_shroud_ = utils::string_bool(cfg["visible_in_shroud"],false);

	text_ = utils::interpolate_variables_into_string(text_, vs);
	team_name_ = utils::interpolate_variables_into_string(team_name_, vs);
	tmp_colour = utils::interpolate_variables_into_string(tmp_colour, vs);

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
	cfg["visible_in_fog"] = visible_in_fog() ? "yes" : "no";
	cfg["visible_in_shroud"] = visible_in_shroud() ? "yes" : "no";
}

const std::string& terrain_label::text() const
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

const map_location& terrain_label::location() const
{
	return loc_;
}

const SDL_Colour& terrain_label::colour() const
{
	return colour_;
}

std::string terrain_label::cfg_colour() const
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
        bool shrouded = visible_in_shroud_ || !is_shrouded(parent_->disp(), loc_);
        font::show_floating_label(handle_, shrouded);
	}
}

void terrain_label::draw()
{
	if (text_.empty())
		return;
	clear();
	if (visible())
	{

		const map_location loc_nextx(loc_.x+1,loc_.y);
		const map_location loc_nexty(loc_.x,loc_.y+1);
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
	if ((!visible_in_fog_ && parent_->disp().fogged(loc_))
        || (!visible_in_shroud_ && parent_->disp().shrouded(loc_))) {
            return false;
	}

	return ((parent_->team_name() == team_name_ && (!is_observer() || !team::nteams()))
			|| (team_name_.empty() && parent_->visible_global_label(loc_)));
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
