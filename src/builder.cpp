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


terrain_builder::tile::tile() 
{
       	memset(adjacents, 0, sizeof(adjacents)); 
}

void terrain_builder::tile::clear() 
{
	flags.clear();
	images_foreground.clear();
	images_background.clear();
	memset(adjacents, 0, sizeof(adjacents));
}

void terrain_builder::tilemap::reset()
{
	for(std::vector<tile>::iterator it = map_.begin(); it != map_.end(); ++it)
		it->clear();
}

bool terrain_builder::tilemap::on_map(const gamemap::location &loc) const
{
	if(loc.x < -1 || loc.y < -1 || loc.x > x_ || loc.y > y_)
		return false;
	
	return true;
}

terrain_builder::tile& terrain_builder::tilemap::operator[](const gamemap::location &loc)
{
	assert(on_map(loc));
		
	return map_[(loc.x+1) + (loc.y+1)*(x_+2)];
}

const terrain_builder::tile& terrain_builder::tilemap::operator[] (const gamemap::location &loc) const
{
	assert(on_map(loc));
	
	return map_[(loc.x+1) + (loc.y+1)*(x_+2)];
}

terrain_builder::terrain_builder(const config& cfg, const gamemap& gmap) :
	map_(gmap), tile_map_(gmap.x(), gmap.y())
{
	parse_config(cfg);
	build_terrains();
	//rebuild_terrain(gamemap::location(0,0));
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
	if (tile_map_.on_map(loc)) {
		tile& btile = tile_map_[loc];
		btile.clear();
		const std::string filename =
			map_.get_terrain_info(map_.get_terrain(loc)).default_image();
		image::locator img_loc(filename);
		btile.images_foreground.push_back(img_loc);
	}
}

void terrain_builder::rebuild_all() {
	tile_map_.reset();
	terrain_by_type_.clear();
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

void terrain_builder::replace_token(std::string &s, const std::string &token, const std::string &replacement)
{
	int pos;
	
	if(token.empty()) {
		std::cerr << "Error: empty token in replace_token\n";
		return;
	}
	while((pos = s.find(token)) != std::string::npos) {
		s.replace(pos, token.size(), replacement);
	}
}

void terrain_builder::replace_token(terrain_builder::imagelist &list, const std::string &token, const std::string &replacement)
{
	for(imagelist::iterator itor = list.begin(); itor != list.end(); ++itor) {
		replace_token(itor->second, token, replacement);
		replace_token(itor->second, token, replacement);
	}
}

void terrain_builder::replace_token(terrain_builder::building_rule &rule, const std::string &token, const std::string& replacement)
{
	constraint_set::iterator cons;

	for(cons = rule.constraints.begin(); cons != rule.constraints.end(); ++cons) {	
		//Transforms attributes
		std::vector<std::string>::iterator flag;
		
		for(flag = cons->second.set_flag.begin(); flag != cons->second.set_flag.end(); flag++) {
			replace_token(*flag, token, replacement);
		}
		for(flag = cons->second.no_flag.begin(); flag != cons->second.no_flag.end(); flag++) {
			replace_token(*flag, token, replacement);
		}
		for(flag = cons->second.has_flag.begin(); flag != cons->second.has_flag.end(); flag++) {
			replace_token(*flag, token, replacement);
		}
		replace_token(cons->second.images, token, replacement);
	}

	replace_token(rule.images, token, replacement);
}

terrain_builder::building_rule terrain_builder::rotate_rule(const terrain_builder::building_rule &rule, 
	int angle, const std::vector<std::string>& rot)
{
	building_rule ret;
	if(rot.size() != 6) {
		std::cerr << "Error: invalid rotations\n";
		return ret;
	}
	ret.images = rule.images;
	ret.location_constraints = rule.location_constraints;
	ret.probability = rule.probability;
	ret.precedence = rule.precedence;

	constraint_set tmp_cons;
	constraint_set::const_iterator cons;
	for(cons = rule.constraints.begin(); cons != rule.constraints.end(); ++cons) {
		const terrain_constraint &rcons = rotate(cons->second, angle);

		tmp_cons[rcons.loc] = rcons;
	}

	// Normalize the rotation, so that it starts on a positive location
	int minx = INT_MAX;
	int miny = INT_MAX;

	constraint_set::iterator cons2;
	for(cons2 = tmp_cons.begin(); cons2 != tmp_cons.end(); ++cons2) {
		minx = minimum<int>(cons2->second.loc.x, minx);
		miny = minimum<int>(2*cons2->second.loc.y + (cons2->second.loc.x & 1), miny);
	}

	if((miny & 1) && (minx & 1) && (minx < 0))
		miny += 2;
	if(!(miny & 1) && (minx & 1) && (minx > 0))
		miny -= 2;

	for(cons2 = tmp_cons.begin(); cons2 != tmp_cons.end(); ++cons2) {	
		//Adjusts positions
		cons2->second.loc += gamemap::location(-minx, -((miny-1)/2));
		ret.constraints[cons2->second.loc] = cons2->second;
	}

	for(int i = 0; i < 6; ++i) {
		int a = (angle+i) % 6;
		std::string token = "@R";
		push_back(token,'0' + i);
		replace_token(ret, token, rot[a]);
	}

	return ret;
}

terrain_builder::building_rule terrain_builder::rule_from_terrain_template(const terrain_builder::building_rule&tpl, const gamemap::TERRAIN terrain)
{
	terrain_builder::building_rule ret = tpl;

	std::string ter(1, terrain);
	constraint_set::iterator cons;
	for(cons = ret.constraints.begin(); cons != ret.constraints.end(); ++cons) {
		replace_token(cons->second.terrain_types, "@", ter);
	}
	replace_token(ret, "@T", map_.get_terrain_info(terrain).default_image());

	return ret;
}

void terrain_builder::add_images_from_config(imagelist& images, const config &cfg)
{
	const config::child_list& cimages = cfg.get_children("image");

	for(config::child_list::const_iterator itor = cimages.begin(); itor != cimages.end(); ++itor) {
		const int z_index = atoi((**itor)["z_index"].c_str());
		const std::string &name = (**itor)["name"];
			
		images.insert(std::pair<int, std::string>(z_index, name));
	}	
}

void terrain_builder::add_constraints(std::map<gamemap::location, terrain_builder::terrain_constraint> & constraints, 
				      const gamemap::location& loc, const std::string& type)
{
	if(constraints.find(loc) == constraints.end()) {
		//the terrain at the current location did not exist, so create it
		constraints[loc] = terrain_constraint(loc);
	}

	if(!type.empty())
		constraints[loc].terrain_types = type;			
}

void terrain_builder::add_constraint_item(std::vector<std::string> &list, const config& cfg, const std::string &item)
{
	if(!cfg[item].empty())
		list.push_back(cfg[item]);

	const config::child_list& items = cfg.get_children(item);

	for(config::child_list::const_iterator itor = items.begin(); itor != items.end(); ++itor) {
		if(!(**itor)["name"].empty())
			list.push_back((**itor)["name"]);
	}
}

void terrain_builder::add_constraints(terrain_builder::constraint_set &constraints, const gamemap::location& loc, const config& cfg)
{
	add_constraints(constraints, loc, cfg["type"]);

	terrain_constraint& constraint = constraints[loc];
	
	add_constraint_item(constraint.set_flag, cfg, "set_flag");
	add_constraint_item(constraint.has_flag, cfg, "has_flag");
	add_constraint_item(constraint.no_flag, cfg, "no_flag");

	add_images_from_config(constraint.images, cfg);
}

void terrain_builder::parse_mapstring(const std::string &mapstring, struct building_rule &br,
				      anchormap& anchors)
{
	int lineno = 0;
	int x = 0;

	// std::cerr << "Loading map \"" << mapstring << "\"\n";
	
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

	//std::cerr << "--- Begin map ---\n";
	
	for(; line != lines.end(); ++line) {
		//cuts each line into chunks of 4 characters, ignoring the 2 first ones if the line is odd

		//std::cerr << "Line is " << *line << "\n";
		
		x = 0;
		std::string::size_type lpos = 0;
		if(lineno % 2) {
			lpos = 2;
			x = 1;
		}

		while(lpos < line->size()) {
			std::string types = line->substr(lpos, 4);
			config::strip(types);
			
			//std::cerr << types << "/";
			
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
		//std::cerr << "\n";
		lineno++;
	}
	//std::cerr << "--- End map ---\n";

}

void terrain_builder::add_rotated_rules(building_ruleset& rules, const building_rule& tpl, const std::string &rotations)
{

	if(rotations.empty()) {
		// Adds the parsed built terrain to the list
		building_rules_.insert(std::pair<int, building_rule>(tpl.precedence, tpl));
	} else {
		const std::vector<std::string>& rot = config::split(rotations, ',');
			
		for(int angle = 0; angle < rot.size(); angle++) {
			building_rules_.insert(std::pair<int, building_rule>(tpl.precedence,
						rotate_rule(tpl, angle, rot)));
		}
	}
}

void terrain_builder::parse_config(const config &cfg)
{
	log_scope("terrain_builder::parse_config");
	
	//Parses the list of building rules (BRs)
	const config::child_list& brs = cfg.get_children("terrain_graphics");

	for(config::child_list::const_iterator br = brs.begin(); br != brs.end(); ++br) {
		building_rule pbr; // Parsed Building rule

		add_images_from_config(pbr.images, **br);

		if(!((**br)["x"].empty() || (**br)["y"].empty()))
			pbr.location_constraints = gamemap::location(atoi((**br)["x"].c_str()), atoi((**br)["y"].c_str()));
		
		pbr.probability = (**br)["probability"].empty() ? -1 : atoi((**br)["probability"].c_str());
		pbr.precedence = (**br)["precedence"].empty() ? 0 : atoi((**br)["precedence"].c_str());
	
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
		
		for(constraint_set::iterator constraint = pbr.constraints.begin(); constraint != pbr.constraints.end();
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
		const std::string terrains = (**br)["terrains"];

		if(terrains.empty()) {
			add_rotated_rules(building_rules_, pbr, rotations);
		} else {
			for(std::string::const_iterator terrain = terrains.begin();
					terrain != terrains.end(); ++terrain) {

				const building_rule r = rule_from_terrain_template(pbr, *terrain);
				add_rotated_rules(building_rules_, r, rotations);
			}
		}

	}

#if 0
	std::cerr << "Built terrain rules: \n";
	
	building_ruleset::const_iterator rule;
	for(rule = building_rules_.begin(); rule != building_rules_.end(); ++rule) {
		std::cerr << ">> New rule: image_background = " /* << rule->second.image_background << " , image_foreground = "<< rule->second.image_foreground */ << "\n";
		for(constraint_set::const_iterator constraint = rule->second.constraints.begin();
		    constraint != rule->second.constraints.end(); ++constraint) {

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
#endif

}

namespace {
int matches = 0;
}

bool terrain_builder::terrain_matches(gamemap::TERRAIN letter, const std::string &terrains)
{

	bool res = false;
	bool negative = false;
	std::string::const_iterator itor;

	if(terrains.empty())
		return true;
	for(itor = terrains.begin(); itor != terrains.end(); ++itor) {
		if(*itor == '*')
			return true;
		if(*itor == '!') {
			negative = true;
			continue;
		}
		if(*itor == letter)
			break;
	}

	if(itor == terrains.end())
		return negative;
	return !negative;
}

bool terrain_builder::rule_matches(const terrain_builder::building_rule &rule, const gamemap::location &loc, int rule_index, bool check_loc)
{
	matches++;

	if(rule.location_constraints.valid() && rule.location_constraints != loc)
		return false;
	
		
	if(check_loc) {
		for(constraint_set::const_iterator cons = rule.constraints.begin();
				cons != rule.constraints.end(); ++cons) {

			// translated location
			const gamemap::location tloc = loc + cons->second.loc;

			if(!tile_map_.on_map(tloc))
				return false;

			const tile& btile = tile_map_[tloc];

			if(!terrain_matches(map_.get_terrain(tloc), cons->second.terrain_types))
				return false;
		}
	}

	if(rule.probability != -1) {
		int random = (((loc.x+23293)^827634) * 7613863 + ((loc.y+19827)^87623) * 87987 + (rule_index^198729) * 89237) % 100;
		
		if(random > rule.probability)
			return false;
	}

	for(constraint_set::const_iterator cons = rule.constraints.begin();
	    cons != rule.constraints.end(); ++cons) {

		const gamemap::location tloc = loc + cons->second.loc;
		if(!tile_map_.on_map(tloc))
			return false;
		const tile& btile = tile_map_[tloc];

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
	for(constraint_set::const_iterator constraint = rule.constraints.begin();
	    constraint != rule.constraints.end(); ++constraint) {

		const gamemap::location tloc = loc + constraint->second.loc;
		if(!tile_map_.on_map(tloc))
			return;

		tile& btile = tile_map_[tloc];

		std::multimap<int, std::string>::const_iterator img;
		
		for(img = rule.images.begin(); img != rule.images.end(); ++img) {
			image::locator th(img->second, constraint->second.loc);
			if(img->first < 0) {
				btile.images_background.push_back(th);
			} else {
				btile.images_foreground.push_back(th);
			}

		}

		for(img = constraint->second.images.begin(); img != constraint->second.images.end(); ++img) {
			image::locator th(img->second);
			if(img->first < 0) {
				btile.images_background.push_back(th);
			} else {
				btile.images_foreground.push_back(th);
			}
		}

		// Sets flags
		for(std::vector<std::string>::const_iterator itor = constraint->second.set_flag.begin(); 
		    itor != constraint->second.set_flag.end(); itor++) {
			btile.flags.insert(*itor);
		}

	}
}

int terrain_builder::get_constraint_adjacents(const building_rule& rule, const gamemap::location& loc)
{
	int res = 0;

	gamemap::location adj[6];
	int i;
	get_adjacent_tiles(loc, adj);
	
	for(i = 0; i < 6; ++i) {
		if(rule.constraints.find(adj[i]) != rule.constraints.end()) {
			res++;
		}
	}	
	return res;
}

//returns the "size" of a constraint: that is, the number of map tiles on which
//this constraint may possibly match. INT_MAX means "I don't know / all of them".
int terrain_builder::get_constraint_size(const building_rule& rule, const terrain_constraint& constraint, bool& border)
{
	const std::string &types = constraint.terrain_types;	

	if(types.empty())
		return INT_MAX;
	if(types[0] == '!')
		return INT_MAX;
	if(types.find('*') != std::string::npos)
		return INT_MAX;

	gamemap::location adj[6];
	int i;
	get_adjacent_tiles(constraint.loc, adj);

	border = false;

	//if the current constraint only applies to a non-isolated tile,
	//the "border" flag can be set.
	for(i = 0; i < 6; ++i) {
		if(rule.constraints.find(adj[i]) != rule.constraints.end()) {
			const std::string& atypes = rule.constraints.find(adj[i])->second.terrain_types;
			for(std::string::const_iterator itor = types.begin();
					itor != types.end(); ++itor) {
				if(!terrain_matches(*itor, atypes)) {
					border = true;
					break;
				}

			}
		}
		if(border == true)
			break;
	}	

	int constraint_size = 0;

	for(std::string::const_iterator itor = types.begin();
			itor != types.end(); ++itor) {
		if(border) {
			constraint_size += terrain_by_type_border_[*itor].size();
		} else {
			constraint_size += terrain_by_type_[*itor].size();
		}
	}

	return constraint_size;
}

void terrain_builder::build_terrains()
{
	log_scope("terrain_builder::build_terrains");
	matches = 0;

	//builds the terrain_by_type_ cache
	for(int x = -1; x <= map_.x(); ++x) {
		for(int y = -1; y <= map_.y(); ++y) {
			const gamemap::location loc(x,y);
			const gamemap::TERRAIN t = map_.get_terrain(loc);

			terrain_by_type_[t].push_back(loc);

			gamemap::location adj[6];
			int i;
			bool border = false;

			get_adjacent_tiles(loc, adj);

			tile_map_[loc].adjacents[0] = t;
			for(i = 0; i < 6; ++i) {
				//updates the list of adjacents for this tile
				tile_map_[loc].adjacents[i+1] = map_.get_terrain(adj[i]);

				//determines if this tile is a border tile
				if(map_.get_terrain(adj[i]) != t) 
					border = true;
			}
			if(border)
				terrain_by_type_border_[t].push_back(loc);
		}
	}

	int rule_index = 0;
	building_ruleset::const_iterator rule;
	
	for(rule = building_rules_.begin(); rule != building_rules_.end(); ++rule) {
		//if the rule has no constraints, it is invalid
		if(rule->second.constraints.empty())
			continue;

		//checks if all the images referenced by the current rule are valid.
		//if not, this rule will not match.
		bool absent_image = false;
		imagelist::const_iterator image;
		constraint_set::const_iterator constraint;

		for(image = rule->second.images.begin(); 
				!absent_image && (image != rule->second.images.end()); ++image) {

			if(!image::exists("terrain/" + image->second + ".png"))
				absent_image = true;
		}

		for(constraint = rule->second.constraints.begin();
		    constraint != rule->second.constraints.end(); ++constraint) {
			for(image = constraint->second.images.begin(); 
					!absent_image && (image != constraint->second.images.end());
					++image) {
				if(!image::exists("terrain/" + image->second + ".png"))
					absent_image = true;
			}
		}

		if(absent_image)
			continue;

		//find the constraint that contains the less terrain of all terrain rules.
		constraint_set::const_iterator smallest_constraint;
		constraint_set::const_iterator constraint_most_adjacents;
		int smallest_constraint_size = INT_MAX;
		int biggest_constraint_adjacent = -1;
		bool smallest_constraint_border = false;

		for(constraint = rule->second.constraints.begin();
		    constraint != rule->second.constraints.end(); ++constraint) {
		
			bool border;

			int size = get_constraint_size(rule->second, constraint->second, border);
			if(size < smallest_constraint_size) {
				smallest_constraint_size = size;
				smallest_constraint = constraint;
				smallest_constraint_border = border;
			}

			int nadjacents = get_constraint_adjacents(rule->second, constraint->second.loc);
			if(nadjacents > biggest_constraint_adjacent) {
				biggest_constraint_adjacent = nadjacents;
				constraint_most_adjacents = constraint;
			}
		}

		std::string adjacent_types[7];

		if(biggest_constraint_adjacent > 0) {
			gamemap::location loc[7];
			loc[0] = constraint_most_adjacents->second.loc;
			get_adjacent_tiles(loc[0], loc+1);
			for(int i = 0; i < 7; ++i) {
				constraint_set::const_iterator cons = rule->second.constraints.find(loc[i]) ;
				if(cons != rule->second.constraints.end()) {
					adjacent_types[i] = cons->second.terrain_types;
				} else {
					adjacent_types[i] = "";
				}
			}
				
		}
		if(smallest_constraint_size != INT_MAX) {
			const std::string &types = smallest_constraint->second.terrain_types;
			const gamemap::location loc = smallest_constraint->second.loc;
			const gamemap::location aloc = constraint_most_adjacents->second.loc;

			for(std::string::const_iterator c = types.begin(); c != types.end(); ++c) {
				const std::vector<gamemap::location>* locations;
				if(smallest_constraint_border) {
					locations = &terrain_by_type_border_[*c];
				} else {
					locations = &terrain_by_type_[*c];
				}
			
				for(std::vector<gamemap::location>::const_iterator itor = locations->begin();
						itor != locations->end(); ++itor) {
				
					if(biggest_constraint_adjacent > 0) {
						const gamemap::location pos = (*itor - loc) + aloc;
						if(!tile_map_.on_map(pos))
							continue;

						const gamemap::TERRAIN *adjacents = tile_map_[pos].adjacents;
						int i;
						
						for(i = 0; i < 7; ++i) {
							if(!terrain_matches(adjacents[i], adjacent_types[i])) {
								break;
							}
						}
						// propagates the break
						if (i < 7)
							continue;
					}

					if(rule_matches(rule->second, *itor - loc, rule_index, (biggest_constraint_adjacent + 1) != rule->second.constraints.size())) {
						apply_rule(rule->second, *itor - loc);
					}
				}
			}
		} else {
			for(int x = -1; x <= map_.x(); ++x) {
				for(int y = -1; y <= map_.y(); ++y) {
					const gamemap::location loc(x,y);
					if(rule_matches(rule->second, loc, rule_index, true))
						apply_rule(rule->second, loc);
				}
			}
		}

		rule_index++;
	}

	std::cerr << "Matches = " << matches << "\n";
}
