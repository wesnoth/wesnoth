/* $Id$ */
/*
   Copyright (C) 2006 by Benoit Timbert <benoit.timbert@free.fr>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#ifndef UNIT_ABILITIES_H_INCLUDED
#define UNIT_ABILITIES_H_INCLUDED


#include "config.hpp"

#include <string>
#include <vector>

class ability
{
public:
	ability(const config* cfg);
	class filter
	{
	public:
		filter();
		bool matches_filter(const std::string& terrain, int lawful_bonus) const;
		void add_filters(const config* cfg);
		void unfilter();
	private:
		std::vector<config> filters;
	};

	const std::string description() const;
	ability::filter filter;
private:
	std::string description_;
};

class heals_ability : public ability
{
public : 
	heals_ability(const config* cfg);
	const int amount() const;
	const int max() const;
	void set_heal(int amount, int max);
private :
	int amount_;
	int max_;
};

class regenerates_ability : public ability
{
public : 
	regenerates_ability(const config* cfg);
	const int regeneration() const;
	void set_regeneration(int reg);
private :
	int regeneration_;
};

class leadership_ability : public ability
{
public : 
	leadership_ability(const config* cfg);
	const int perlevel_bonus() const;
	void set_leadership(int perlevel_bonus);
private :
	int perlevel_bonus_;
};

class illuminates_ability : public ability
{
public : 
	illuminates_ability(const config* cfg);
	const int level() const;
	void set_illumination(int level=1);
private :
	int level_;
};

class steadfast_ability : public ability
{
public : 
	steadfast_ability(const config* cfg);
	const int bonus() const;
	const int max() const;
	const bool use_percent() const;
	void set_steadfast(int bonus=100, int max=50, bool use_percent=true);
private :
	int bonus_;
	int max_;
	bool use_percent_;
};

#endif
