/* $Id$ */
/*
   Copyright (C) 2004 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "terrain.hpp"
#include "display.hpp"
#include "util.hpp"
#include "log.hpp"

void terrain_builder::tile::clear() 
{
	flags.clear();
	images_foreground.clear();
	images_background.clear();
}

void terrain_builder::tilemap::reset()
{
	for(std::vector<tile>::iterator it = map_.begin(); it != map_.end(); ++it)
		it->clear();
}

terrain_builder::terrain_builder(const config& cfg, const gamemap& gmap) :
	map_(gmap), tile_map_(gmap.x(), gmap.y())
{
	parse_config(cfg);
	build_terrains();
}

const std::vector<image::locator> *terrain_builder::get_terrain_at(const gamemap::location &loc,
								   ADJACENT_TERRAIN_TYPE terrain_type) const 
{
	if(terrain_type == ADJACENT_BACKGROUND) {
		if(tile_map_[loc].images_background.size())
			return &tile_map_[loc].images_background;
	}
	
	if(terrain_type == ADJACENT_FOREGROUND) {
		if(tile_map_[loc].images_foreground.size())
			return &tile_map_[loc].images_foreground;
	}

	return NULL;
}

void terrain_builder::rebuild_terrain(const gamemap::location &loc)
{
	tile_map_.reset();
	terrain_by_type_.clear();
	
	// For now, rebuild the whole map on each rebuilt_terrain. This is highly slow and
	// inefficient, but this is simple
	build_terrains();
}

terrain_builder::terrain_constraint terrain_builder::rotate(const terrain_builder::terrain_constraint &constraint, int angle)
{
	static struct { int ii; int ij; int ji; int jj; }  rotations[6] = 
		{ {  1, 0, 0,  1 }, {  1,  1, -1, 0 }, { 0,  1, -1, -1 },
		  { -1, 0, 0, -1 }, { -1, -1,  1, 0 }, { 0, -1,  1,  1 } };

	assert(angle >= 0);

	angle %= 6;	
	terrain_constraint ret = constraint;

	// Vector i is going from n to s, vector j is going from ne to sw.
	int vi = ret.loc.y - ret.loc.x/2;
	int vj = ret.loc.x;
	
	int ri = rotations[angle].ii * vi + rotations[angle].ij * vj;
	int rj = rotations[angle].ji * vi + rotations[angle].jj * vj;
		
	ret.loc.x = rj;
	ret.loc.y = ri + (rj >= 0 ? rj/2 : (rj-1)/2);

	return ret;
}

void terrain_builder::replace_rotation(std::string &s, const std::string &replacement)
{
	int pos;
	
	while((pos = s.find("$R")) != std::string::npos) {
		s.replace(pos, 2, replacement);
	}
}

terrain_builder::building_rule terrain_builder::rotate_rule(const terrain_builder::building_rule &rule, 
							    int angle, const std::string &angle_name)
{
	building_rule ret;
	ret.image_foreground = rule.image_foreground;
	ret.image_background = rule.image_background;
	ret.location_constraints = rule.location_constraints;
	ret.probability = rule.probability;

	building_rule::constraint_set::const_iterator cons;
	for(cons = rule.constraints.begin(); cons != rule.constraints.end(); ++cons) {
		const terrain_constraint &rcons = rotate(cons->second, angle);
		ret.constraints[rcons.loc] = rcons;
	}

	// Normalize the rotation, so that it starts on a positive location
	int minx = INT_MAX;
	int miny = INT_MAX;

	building_rule::constraint_set::iterator cons2;
	for(cons2 = ret.constraints.begin(); cons2 != ret.constraints.end(); ++cons2) {
		minx = minimum<int>(cons2->second.loc.x, minx);
		miny = minimum<int>(2*cons2->second.loc.y + (cons2->second.loc.x & 1), miny);
	}

	if((miny & 1) && (minx & 1) && (minx < 0))
		miny += 2;
	if(!(miny & 1) && (minx & 1) && (minx > 0))
		miny -= 2;

	for(cons2 = ret.constraints.begin(); cons2 != ret.constraints.end(); ++cons2) {	
		//Adjusts positions
		cons2->second.loc += gamemap::location(-minx, -((miny-1)/2));
		
		//Transforms attributes
		std::vector<std::string>::iterator flag;
		
		for(flag = cons2->second.set_flag.begin(); flag != cons2->second.set_flag.end(); flag++) {
			replace_rotation(*flag, angle_name);
		}
		for(flag = cons2->second.no_flag.begin(); flag != cons2->second.no_flag.end(); flag++) {
			replace_rotation(*flag, angle_name);
		}
		for(flag = cons2->second.has_flag.begin(); flag != cons2->second.has_flag.end(); flag++) {
			replace_rotation(*flag, angle_name);
		}
	}

	replace_rotation(ret.image_foreground, angle_name);
	replace_rotation(ret.image_background, angle_name);
	
	return ret;
}

void terrain_builder::add_constraint_item(std::vector<std::string> &list, const std::string &item)
{
	if(!item.empty())
		list.push_back(item);
}

void terrain_builder::add_constraints(std::map<gamemap::location, terrain_builder::terrain_constraint> & constraints, 
				      const gamemap::location& loc, const std::string& type)
{
	if(constraints.find(loc) == constraints.end()) {
		//the terrain at the current location did not exist, so create it
		constraints[loc] = terrain_constraint(loc);
	}

	if(type.size())
		constraints[loc].terrain_types = type;			
}

void terrain_builder::add_constraints(std::map<gamemap::location, terrain_builder::terrain_constraint> &constraints, 
				      const gamemap::location& loc, const config& cfg)
{
	add_constraints(constraints, loc, cfg["type"]);

	add_constraint_item(constraints[loc].set_flag, cfg["set_flag"]);
	add_constraint_item(constraints[loc].has_flag, cfg["has_flag"]);
	add_constraint_item(constraints[loc].no_flag, cfg["no_flag"]);		
}

void terrain_builder::parse_mapstring(const std::string &mapstring, struct building_rule &br,
				      anchormap& anchors)
{
	int lineno = 0;
	int x = 0;

	std::cerr << "Loading map \"" << mapstring << "\"\n";
	
	const std::vector<std::string> &lines = config::split(mapstring, '\n', 0);
	std::vector<std::string>::const_iterator line = lines.begin();
	
	//Strips trailing empty lines
	while(line != lines.end() && std::find_if(line->begin(),line->end(),config::notspace) == line->end()) {
		line++;
	}
	//Break if there only are blank lines
	if(line == lines.end())
		return;
	
	//If the strings starts with a space, the first line is an odd line, else it is an even one
	if((*line)[0] == ' ')
		lineno = 1;

	std::cerr << "--- Begin map ---\n";
	
	for(; line != lines.end(); ++line) {
		//cuts each line into chunks of 4 characters, ignoring the 2 first ones if the line is odd

		std::cerr << "Line is " << *line << "\n";
		
		x = 0;
		std::string::size_type lpos = 0;
		if(lineno % 2) {
			lpos = 2;
			x = 1;
		}

		while(lpos < line->size()) {
			std::string types = line->substr(lpos, 4);
			config::strip(types);
			
			std::cerr << types << "/";
			
			//If there are numbers in the types string, consider it is an anchor
			if(types[0] == '.') {
				// Dots are simple placeholders, which do not represent actual terrains.
			} else if(types.find_first_of("0123456789") != std::string::npos) {
				int anchor = atoi(types.c_str());
				anchors.insert(std::pair<int, gamemap::location>(anchor, gamemap::location(x, lineno / 2)));
			} else {
				const gamemap::location loc(x, lineno / 2);
				add_constraints(br.constraints, loc, types);
			}
			lpos += 4;
			x += 2;
		}
		std::cerr << "\n";
		lineno++;
	}
	std::cerr << "--- End map ---\n";

}

void terrain_builder::parse_config(const config &cfg)
{
	log_scope("terrain_builder::parse_config");
	
	//Parses the list of building rules (BRs)
	config::child_list brs(cfg.get_children("building_rule"));

	for(config::child_list::const_iterator br = brs.begin(); br != brs.end(); ++br) {
		building_rule pbr; // Parsed Building rule

		pbr.image_foreground = (**br)["image_foreground"];
		pbr.image_background = (**br)["image_background"];
		if(!((**br)["x"].empty() || (**br)["y"].empty()))
			pbr.location_constraints = gamemap::location(atoi((**br)["x"].c_str()), atoi((**br)["y"].c_str()));
		
		pbr.probability = (**br)["probability"].empty() ? -1 : atoi((**br)["probability"].c_str());
	
		//Mapping anchor indices to anchor locations. 
		anchormap anchors;
		
		// Parse the map= , if there is one (and fill the anchors list)
		parse_mapstring((**br)["map"], pbr, anchors);

		// Parses the terrain constraints (TCs)
		config::child_list tcs((*br)->get_children("tile"));
		
		for(config::child_list::const_iterator tc = tcs.begin(); tc != tcs.end(); tc++) {
			//Adds the terrain constraint to the current built terrain's list of terrain 
			//constraints, if it does not exist.
			gamemap::location loc;
			if((**tc)["x"].size()) {
				loc.x = atoi((**tc)["x"].c_str());
			} 
			if((**tc)["y"].size()) {
				loc.y = atoi((**tc)["y"].c_str());
			} 
			if(loc.valid()) {
				add_constraints(pbr.constraints, loc, **tc);
			}
			if((**tc)["pos"].size()) {
				int pos = atoi((**tc)["pos"].c_str());
				if(anchors.find(pos) == anchors.end()) {
					std::cerr << "Invalid anchor!\n";
					continue;
				}

				std::pair<anchormap::const_iterator, anchormap::const_iterator> range =
					anchors.equal_range(pos);
				
				for(; range.first != range.second; range.first++) {
					loc = range.first->second;
					add_constraints(pbr.constraints, loc, **tc);
				}					
			}
		}

		const std::string global_set_flag = (**br)["set_flag"];
		const std::string global_no_flag = (**br)["no_flag"];
		const std::string global_has_flag = (**br)["has_flag"];
		
		for(building_rule::constraint_set::iterator constraint = pbr.constraints.begin(); constraint != pbr.constraints.end();
		    constraint++) {
			
			if(global_set_flag.size())
				constraint->second.set_flag.push_back(global_set_flag);
			
			if(global_no_flag.size())
				constraint->second.no_flag.push_back(global_no_flag);
			
			if(global_has_flag.size())
				constraint->second.has_flag.push_back(global_has_flag);

		}

		// Handles rotations
		const std::string rotations = (**br)["rotations"];
		if(rotations.size()) {
			const std::vector<std::string>& rot = config::split(rotations, ',');
			
			for(int angle = 0; angle < rot.size(); angle++) {
				building_rules_.push_back(rotate_rule(pbr, angle, rot[angle]));
			}
		} else {
			// Adds the parsed built terrain to the list
			building_rules_.push_back(pbr);
		}
	}
}

bool terrain_builder::rule_matches(const terrain_builder::building_rule &rule, const gamemap::location &loc, int rule_index)
{
	if(!loc.valid())
		return false;
	if(rule.location_constraints.valid() && rule.location_constraints != loc)
		return false;
	
	if(rule.probability != -1) {
		int random = ((loc.x^827634) * 7613863 + (loc.y^87623) * 87987 + (rule_index^198729) * 89237) % 100;
		
		if(random > rule.probability)
			return false;
	}
		
	for(building_rule::constraint_set::const_iterator cons = rule.constraints.begin();
	    cons != rule.constraints.end(); ++cons) {
		
		// translated location
		const gamemap::location tloc = loc + cons->second.loc;
		if(!map_.on_board(tloc))
			return false;
		
		const tile& btile = tile_map_[tloc];

		if(!map_.get_terrain_info(tloc).matches(cons->second.terrain_types))
			return false;
		
		std::vector<std::string>::const_iterator itor;
		for(itor = cons->second.no_flag.begin(); itor != cons->second.no_flag.end(); ++itor) {
			
			//If a flag listed in "no_flag" is present, the rule does not match
			if(btile.flags.find(*itor) != btile.flags.end())
				return false;
		}
		for(itor = cons->second.has_flag.begin(); itor != cons->second.has_flag.end(); ++itor) {
			
			//If a flag listed in "has_flag" is not present, this rule does not match
			if(btile.flags.find(*itor) == btile.flags.end())
				return false;
		}
	}

	return true;
}

void terrain_builder::apply_rule(const terrain_builder::building_rule &rule, const gamemap::location &loc)
{
	for(building_rule::constraint_set::const_iterator constraint = rule.constraints.begin();
	    constraint != rule.constraints.end(); ++constraint) {

		const gamemap::location tloc = loc + constraint->second.loc;
		if(!map_.on_board(tloc)) {
			std::cerr << "Error: out-of map tile!\n";
			return;
		}

		tile& btile = tile_map_[tloc];

		if(rule.image_foreground.size()) {
			image::locator th(rule.image_foreground, constraint->second.loc);
			btile.images_foreground.push_back(th);
		}

		if(rule.image_background.size()) {
			image::locator th(rule.image_background, constraint->second.loc);
			btile.images_background.push_back(th);
		}


		for(std::vector<std::string>::const_iterator itor = constraint->second.set_flag.begin(); 
		    itor != constraint->second.set_flag.end(); itor++) {
			btile.flags.insert(*itor);
		}

	}
}

void terrain_builder::build_terrains()
{
	std::cerr << "Built terrain rules: \n";
	
	building_ruleset::const_iterator rule;
	for(rule = building_rules_.begin(); rule != building_rules_.end(); ++rule) {
		std::cerr << ">> New rule: image_background = " << rule->image_background << " , image_foreground = "<< rule->image_foreground << "\n";
		for(building_rule::constraint_set::const_iterator constraint = rule->constraints.begin();
		    constraint != rule->constraints.end(); ++constraint) {

			std::cerr << ">>>> New constraint: location = (" << constraint->second.loc.x << ", " << constraint->second.loc.y << "), terrain types = " << constraint->second.terrain_types << "\n";

			std::vector<std::string>::const_iterator flag;
			
			for(flag  = constraint->second.set_flag.begin(); flag != constraint->second.set_flag.end(); ++flag) {
				std::cerr << ">>>>>> Set_flag: " << *flag << "\n";
			}

			for(flag = constraint->second.no_flag.begin(); flag != constraint->second.no_flag.end(); ++flag) {
				std::cerr << ">>>>>> No_flag: " << *flag << "\n";
			}	
		}

	}

	log_scope("terrain_builder::build_terrains");

	//builds the terrain_by_type_ cache
	for(int x = -1; x <= map_.x(); ++x) {
		for(int y = -1; y <= map_.y(); ++y) {
			const gamemap::location loc(x,y);
			const gamemap::TERRAIN t = map_.get_terrain(loc);

			terrain_by_type_[t].push_back(loc);
		}
	}

	int rule_index = 0;
	
	for(rule = building_rules_.begin(); rule != building_rules_.end(); ++rule) {

		//find a constraint that contains an unique terrain type on the current
		//rule
		building_rule::constraint_set::const_iterator constraint;
		for(constraint = rule->constraints.begin();
		    constraint != rule->constraints.end(); ++constraint) {
			
			if(constraint->second.terrain_types.size() == 1 && (constraint->second.terrain_types[0] != '*')) {
				break;
			}
		}

		if(constraint != rule->constraints.end()) {
			gamemap::TERRAIN c = constraint->second.terrain_types[0];
			gamemap::location loc = constraint->second.loc;
			const std::vector<gamemap::location>& locations = terrain_by_type_[c];
			
			for(std::vector<gamemap::location>::const_iterator itor = locations.begin();
			    itor != locations.end(); ++itor) {
				
				if(rule_matches(*rule, *itor - loc, rule_index)) {
					apply_rule(*rule, *itor - loc);
				}
			}
		} else {
			for(int x = -1; x <= map_.x(); ++x) {
				for(int y = -1; y <= map_.y(); ++y) {
					const gamemap::location loc(x,y);
					if(rule_matches(*rule, loc, rule_index))
						apply_rule(*rule, loc);
				}
			}
		}

		rule_index++;
	}
}
