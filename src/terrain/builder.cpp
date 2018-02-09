/*
   Copyright (C) 2004 - 2018 by Philippe Plantier <ayin@anathas.org>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 * @file
 * Terrain builder.
 */

#include "terrain/builder.hpp"

#include "gui/dialogs/loading_screen.hpp"
#include "image.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "preferences/game.hpp"
#include "serialization/string_utils.hpp"

static lg::log_domain log_engine("engine");
#define ERR_NG LOG_STREAM(err, log_engine)
#define WRN_NG LOG_STREAM(warn, log_engine)

/**
 *
 * These legacy map_location functions moved here from map_location.?pp.
 * We have refactored them out of everything but this class. Hopefully
 * the end is near...
 *
	// Adds an absolute location to a "delta" location
	// This is not the mathematically correct behavior, it is neither
	// commutative nor associative. Negative coordinates may give strange
	// results. It is retained because terrain builder code relies in this
	// broken behavior. Best avoid.
	map_location legacy_negation() const;
	map_location legacy_sum(const map_location &a) const;
	map_location& legacy_sum_assign(const map_location &a);
	map_location legacy_difference(const map_location &a) const;
 *
 */

static map_location legacy_negation(const map_location& me)
{
	return map_location(-me.x, -me.y);
}

static map_location& legacy_sum_assign(map_location& me, const map_location& a)
{
	bool parity = (me.x & 1) != 0;
	me.x += a.x;
	me.y += a.y;
	if((a.x > 0) && (a.x % 2) && parity)
		me.y++;
	if((a.x < 0) && (a.x % 2) && !parity)
		me.y--;

	return me;
}

static map_location legacy_sum(const map_location& me, const map_location& a)
{
	map_location ret(me);
	legacy_sum_assign(ret, a);
	return ret;
}

static map_location legacy_difference(const map_location& me, const map_location& a)
{
	return legacy_sum(me, legacy_negation(a));
}

/**
 *
 * This file holds the terrain_builder implementation.
 *
 */

terrain_builder::building_ruleset terrain_builder::building_rules_;
const config* terrain_builder::rules_cfg_ = nullptr;

terrain_builder::rule_image::rule_image(int layer, int x, int y, bool global_image, int cx, int cy, bool is_water)
	: layer(layer)
	, basex(x)
	, basey(y)
	, variants()
	, global_image(global_image)
	, center_x(cx)
	, center_y(cy)
	, is_water(is_water)
{
}

terrain_builder::tile::tile()
	: flags()
	, images()
	, images_foreground()
	, images_background()
	, last_tod("invalid_tod")
	, sorted_images(false)
{
}

void terrain_builder::tile::rebuild_cache(const std::string& tod, logs* log)
{
	images_background.clear();
	images_foreground.clear();

	if(!sorted_images) {
		// sort images by their layer (and basey)
		// but use stable to keep the insertion order in equal cases
		std::stable_sort(images.begin(), images.end());
		sorted_images = true;
	}

	for(const rule_image_rand& ri : images) {
		bool is_background = ri->is_background();
		bool animate = (!ri.ri->is_water || preferences::animate_water());

		imagelist& img_list = is_background ? images_background : images_foreground;

		for(const rule_image_variant& variant : ri->variants) {
			if(!variant.has_flag.empty()) {
				bool has_flag_match = true;
				for(const std::string& s : variant.has_flag) {
					// If a flag listed in "has_flag" is not present, this variant does not match
					if(flags.find(s) == flags.end()) {
						has_flag_match = false;
						break;
					}
				}

				if(!has_flag_match) {
					continue;
				}
			}

			if(!variant.tods.empty() && variant.tods.find(tod) == variant.tods.end())
				continue;

			// need to break parity pattern in RNG
			/** @todo improve this */
			unsigned int rnd = ri.rand / 7919; // just the 1000th prime
			const animated<image::locator>& anim = variant.images[rnd % variant.images.size()];

			bool is_empty = true;
			for(size_t i = 0; i < anim.get_frames_count(); ++i) {
				if(!image::is_empty_hex(anim.get_frame(i))) {
					is_empty = false;
					break;
				}
			}

			if(is_empty)
				continue;

			img_list.push_back(anim);

			assert(anim.get_animation_duration() != 0);

			if(variant.random_start < 0)
				img_list.back().set_animation_time(ri.rand % img_list.back().get_animation_duration());
			else if(variant.random_start > 0)
				img_list.back().set_animation_time(ri.rand % variant.random_start);

			if(!animate) {
				img_list.back().pause_animation();
			}

			if(log) {
				log->emplace_back(&ri, &variant);
			}

			break; // found a matching variant
		}
	}
}

void terrain_builder::tile::clear()
{
	flags.clear();
	images.clear();
	sorted_images = false;
	images_foreground.clear();
	images_background.clear();
	last_tod = "invalid_tod";
}

static unsigned int get_noise(const map_location& loc, unsigned int index)
{
	unsigned int a = (loc.x + 92872973) ^ 918273;
	unsigned int b = (loc.y + 1672517) ^ 128123;
	unsigned int c = (index + 127390) ^ 13923787;
	unsigned int abc = a * b * c + a * b + b * c + a * c + a + b + c;
	return abc * abc;
}

void terrain_builder::tilemap::reset()
{
	for(std::vector<tile>::iterator it = tiles_.begin(); it != tiles_.end(); ++it)
		it->clear();
}

void terrain_builder::tilemap::reload(int x, int y)
{
	x_ = x;
	y_ = y;
	std::vector<terrain_builder::tile> new_tiles((x + 4) * (y + 4));
	tiles_.swap(new_tiles);
	reset();
}

bool terrain_builder::tilemap::on_map(const map_location& loc) const
{
	if(loc.x < -2 || loc.y < -2 || loc.x > (x_ + 1) || loc.y > (y_ + 1)) {
		return false;
	}

	return true;
}

terrain_builder::tile& terrain_builder::tilemap::operator[](const map_location& loc)
{
	assert(on_map(loc));

	return tiles_[(loc.x + 2) + (loc.y + 2) * (x_ + 4)];
}

const terrain_builder::tile& terrain_builder::tilemap::operator[](const map_location& loc) const
{
	assert(on_map(loc));

	return tiles_[(loc.x + 2) + (loc.y + 2) * (x_ + 4)];
}

terrain_builder::terrain_builder(const config& level, const gamemap* m, const std::string& offmap_image, bool draw_border)
	: tilewidth_(game_config::tile_size)
	, map_(m)
	, tile_map_(m ? map().w() : 0, m ? map().h() : 0)
	, terrain_by_type_()
	, draw_border_(draw_border)
{
	image::precache_file_existence("terrain/");

	if(building_rules_.empty() && rules_cfg_) {
		// off_map first to prevent some default rule seems to block it
		add_off_map_rule(offmap_image);
		// parse global terrain rules
		parse_global_config(*rules_cfg_);
	} else {
		// use cached global rules but clear local rules
		flush_local_rules();
	}

	// parse local rules
	parse_config(level);

	if(m)
		build_terrains();
}

void terrain_builder::rebuild_cache_all()
{
	for(int x = -2; x <= map().w(); ++x) {
		for(int y = -2; y <= map().h(); ++y) {
			tile_map_[map_location(x, y)].rebuild_cache("");
		}
	}
}

void terrain_builder::flush_local_rules()
{
	building_ruleset::iterator i = building_rules_.begin();
	for(; i != building_rules_.end();) {
		if(i->local)
			building_rules_.erase(i++);
		else
			++i;
	}
}

void terrain_builder::set_terrain_rules_cfg(const config& cfg)
{
	rules_cfg_ = &cfg;
	// use the swap trick to clear the rules cache and get a fresh one.
	// because simple clear() seems to cause some progressive memory degradation.
	building_ruleset empty;
	std::swap(building_rules_, empty);
}

void terrain_builder::reload_map()
{
	tile_map_.reload(map().w(), map().h());
	terrain_by_type_.clear();
	build_terrains();
}

void terrain_builder::change_map(const gamemap* m)
{
	map_ = m;
	reload_map();
}

const terrain_builder::imagelist* terrain_builder::get_terrain_at(
		const map_location& loc, const std::string& tod, const TERRAIN_TYPE terrain_type)
{
	if(!tile_map_.on_map(loc))
		return nullptr;

	tile& tile_at = tile_map_[loc];

	if(tod != tile_at.last_tod) {
		tile_at.rebuild_cache(tod);
		tile_at.last_tod = tod;
	}

	const imagelist& img_list = (terrain_type == BACKGROUND) ? tile_at.images_background : tile_at.images_foreground;

	if(!img_list.empty()) {
		return &img_list;
	}

	return nullptr;
}

bool terrain_builder::update_animation(const map_location& loc)
{
	if(!tile_map_.on_map(loc))
		return false;

	bool changed = false;

	tile& btile = tile_map_[loc];

	for(animated<image::locator>& a : btile.images_background) {
		if(a.need_update())
			changed = true;
		a.update_last_draw_time();
	}
	for(animated<image::locator>& a : btile.images_foreground) {
		if(a.need_update())
			changed = true;
		a.update_last_draw_time();
	}

	return changed;
}

/** @todo TODO: rename this function */
void terrain_builder::rebuild_terrain(const map_location& loc)
{
	if(tile_map_.on_map(loc)) {
		tile& btile = tile_map_[loc];
		// btile.images.clear();
		btile.images_foreground.clear();
		btile.images_background.clear();
		const std::string filename = map().get_terrain_info(loc).minimap_image();

		if(!filename.empty()) {
			animated<image::locator> img_loc;
			img_loc.add_frame(100, image::locator("terrain/" + filename + ".png"));
			img_loc.start_animation(0, true);
			btile.images_background.push_back(img_loc);
		}

		// Combine base and overlay image if necessary
		if(map().get_terrain_info(loc).is_combined()) {
			const std::string filename_ovl = map().get_terrain_info(loc).minimap_image_overlay();

			if(!filename_ovl.empty()) {
				animated<image::locator> img_loc_ovl;
				img_loc_ovl.add_frame(100, image::locator("terrain/" + filename_ovl + ".png"));
				img_loc_ovl.start_animation(0, true);
				btile.images_background.push_back(img_loc_ovl);
			}
		}
	}
}

void terrain_builder::rebuild_all()
{
	tile_map_.reset();
	terrain_by_type_.clear();
	build_terrains();
}

static bool image_exists(const std::string& name)
{
	bool precached = name.find("..") == std::string::npos;

	if(precached && image::precached_file_exists(name)) {
		return true;
	} else if(image::exists(name)) {
		return true;
	}

	return false;
}

static std::vector<std::string> get_variations(const std::string& base, const std::string& variations)
{
	/** @todo optimize this function */
	std::vector<std::string> res;
	if(variations.empty()) {
		res.push_back(base);
		return res;
	}
	std::string::size_type pos = base.find("@V", 0);
	if(pos == std::string::npos) {
		res.push_back(base);
		return res;
	}
	std::vector<std::string> vars = utils::split(variations, ';', 0);

	for(const std::string& v : vars) {
		res.push_back(base);
		pos = 0;
		while((pos = res.back().find("@V", pos)) != std::string::npos) {
			res.back().replace(pos, 2, v);
			pos += v.size();
		}
	}
	return res;
}

bool terrain_builder::load_images(building_rule& rule)
{
	// If the rule has no constraints, it is invalid
	if(rule.constraints.empty())
		return false;

	// Parse images and animations data
	// If one is not valid, return false.
	for(terrain_constraint& constraint : rule.constraints) {
		for(rule_image& ri : constraint.images) {
			for(rule_image_variant& variant : ri.variants) {
				std::vector<std::string> var_strings = get_variations(variant.image_string, variant.variations);
				for(const std::string& var : var_strings) {
					/** @todo improve this, 99% of terrains are not animated. */
					std::vector<std::string> frames = utils::square_parenthetical_split(var, ',');
					animated<image::locator> res;

					for(const std::string& frame : frames) {
						const std::vector<std::string> items = utils::split(frame, ':');
						const std::string& str = items.front();

						const size_t tilde = str.find('~');
						bool has_tilde = tilde != std::string::npos;
						const std::string filename = "terrain/" + (has_tilde ? str.substr(0, tilde) : str);

						if(!image_exists(filename)) {
							continue; // ignore missing frames
						}

						const std::string modif = (has_tilde ? str.substr(tilde + 1) : "");

						int time = 100;
						if(items.size() > 1) {
							try {
								time = std::stoi(items.back());
							} catch(std::invalid_argument&) {
								ERR_NG << "Invalid 'time' value in terrain image builder: " << items.back() << "\n";
							}
						}
						image::locator locator;
						if(ri.global_image) {
							locator = image::locator(filename, constraint.loc, ri.center_x, ri.center_y, modif);
						} else {
							locator = image::locator(filename, modif);
						}
						res.add_frame(time, locator);
					}
					if(res.get_frames_count() == 0)
						break; // no valid images, don't register it

					res.start_animation(0, true);
					variant.images.push_back(std::move(res));
				}
				if(variant.images.empty())
					return false; // no valid images, rule is invalid
			}
		}
	}

	return true;
}

void terrain_builder::rotate(terrain_constraint& ret, int angle)
{
	static const struct
	{
		int ii;
		int ij;
		int ji;
		int jj;
	} rotations[6] {{1, 0, 0, 1}, {1, 1, -1, 0}, {0, 1, -1, -1}, {-1, 0, 0, -1}, {-1, -1, 1, 0}, {0, -1, 1, 1}};

	// The following array of matrices is intended to rotate the (x,y)
	// coordinates of a point in a wesnoth hex (and wesnoth hexes are not
	// regular hexes :) ).
	// The base matrix for a 1-step rotation with the wesnoth tile shape
	// is:
	//
	// r = s^-1 * t * s
	//
	// with s = [[ 1   0         ]
	//           [ 0   -sqrt(3)/2 ]]
	//
	// and t =  [[ -1/2       sqrt(3)/2 ]
	//           [ -sqrt(3)/2  1/2        ]]
	//
	// With t being the rotation matrix (pi/3 rotation), and s a matrix
	// that transforms the coordinates of the wesnoth hex to make them
	// those of a regular hex.
	//
	// (demonstration left as an exercise for the reader)
	//
	// So we have
	//
	// r = [[ 1/2  -3/4 ]
	//      [ 1    1/2  ]]
	//
	// And the following array contains I(2), r, r^2, r^3, r^4, r^5
	// (with r^3 == -I(2)), which are the successive rotations.
	static const struct
	{
		double xx;
		double xy;
		double yx;
		double yy;
	} xyrotations[6] {
		{ 1.,         0.,  0., 1.    },
		{ 1./2. , -3./4.,  1., 1./2. },
		{ -1./2., -3./4.,   1, -1./2.},
		{ -1.   ,     0.,  0., -1.   },
		{ -1./2.,  3./4., -1., -1./2.},
		{ 1./2. ,  3./4., -1., 1./2. },
	};

	assert(angle >= 0);

	angle %= 6;

	// Vector i is going from n to s, vector j is going from ne to sw.
	int vi = ret.loc.y - ret.loc.x / 2;
	int vj = ret.loc.x;

	int ri = rotations[angle].ii * vi + rotations[angle].ij * vj;
	int rj = rotations[angle].ji * vi + rotations[angle].jj * vj;

	ret.loc.x = rj;
	ret.loc.y = ri + (rj >= 0 ? rj / 2 : (rj - 1) / 2);

	for(rule_imagelist::iterator itor = ret.images.begin(); itor != ret.images.end(); ++itor) {
		double vx, vy, rx, ry;

		vx = double(itor->basex) - double(tilewidth_) / 2;
		vy = double(itor->basey) - double(tilewidth_) / 2;

		rx = xyrotations[angle].xx * vx + xyrotations[angle].xy * vy;
		ry = xyrotations[angle].yx * vx + xyrotations[angle].yy * vy;

		itor->basex = int(rx + tilewidth_ / 2);
		itor->basey = int(ry + tilewidth_ / 2);

		// std::cerr << "Rotation: from " << vx << ", " << vy << " to " << itor->basex <<
		//	", " << itor->basey << "\n";
	}
}

void terrain_builder::replace_rotate_tokens(std::string& s, int angle, const std::vector<std::string>& replacement)
{
	std::string::size_type pos = 0;
	while((pos = s.find("@R", pos)) != std::string::npos) {
		if(pos + 2 >= s.size())
			return;
		unsigned i = s[pos + 2] - '0' + angle;
		if(i >= 6)
			i -= 6;
		if(i >= 6) {
			pos += 2;
			continue;
		}
		const std::string& r = replacement[i];
		s.replace(pos, 3, r);
		pos += r.size();
	}
}

void terrain_builder::replace_rotate_tokens(rule_image& image, int angle, const std::vector<std::string>& replacement)
{
	for(rule_image_variant& variant : image.variants) {
		replace_rotate_tokens(variant, angle, replacement);
	}
}

void terrain_builder::replace_rotate_tokens(
		rule_imagelist& list, int angle, const std::vector<std::string>& replacement)
{
	for(rule_image& img : list) {
		replace_rotate_tokens(img, angle, replacement);
	}
}

void terrain_builder::replace_rotate_tokens(building_rule& rule, int angle, const std::vector<std::string>& replacement)
{
	for(terrain_constraint& cons : rule.constraints) {
		// Transforms attributes
		for(std::string& flag : cons.set_flag) {
			replace_rotate_tokens(flag, angle, replacement);
		}
		for(std::string& flag : cons.no_flag) {
			replace_rotate_tokens(flag, angle, replacement);
		}
		for(std::string& flag : cons.has_flag) {
			replace_rotate_tokens(flag, angle, replacement);
		}
		replace_rotate_tokens(cons.images, angle, replacement);
	}

	// replace_rotate_tokens(rule.images, angle, replacement);
}

void terrain_builder::rotate_rule(building_rule& ret, int angle, const std::vector<std::string>& rot)
{
	if(rot.size() != 6) {
		ERR_NG << "invalid rotations" << std::endl;
		return;
	}

	for(terrain_constraint& cons : ret.constraints) {
		rotate(cons, angle);
	}

	// Normalize the rotation, so that it starts on a positive location
	int minx = INT_MAX;
	int miny = INT_MAX;

	for(const terrain_constraint& cons : ret.constraints) {
		minx = std::min<int>(cons.loc.x, minx);
		miny = std::min<int>(2 * cons.loc.y + (cons.loc.x & 1), miny);
	}

	if((miny & 1) && (minx & 1) && (minx < 0))
		miny += 2;
	if(!(miny & 1) && (minx & 1) && (minx > 0))
		miny -= 2;

	for(terrain_constraint& cons : ret.constraints) {
		legacy_sum_assign(cons.loc, map_location(-minx, -((miny - 1) / 2)));
	}

	replace_rotate_tokens(ret, angle, rot);
}

terrain_builder::rule_image_variant::rule_image_variant(const std::string& image_string,
		const std::string& variations,
		const std::string& tod,
		const std::string& has_flag,
		int random_start)
	: image_string(image_string)
	, variations(variations)
	, images()
	, tods()
	, has_flag()
	, random_start(random_start)
{
	if(!has_flag.empty()) {
		this->has_flag = utils::split(has_flag);
	}
	if(!tod.empty()) {
		const std::vector<std::string> tod_list = utils::split(tod);
		tods.insert(tod_list.begin(), tod_list.end());
	}
}

void terrain_builder::add_images_from_config(rule_imagelist& images, const config& cfg, bool global, int dx, int dy)
{
	for(const config& img : cfg.child_range("image")) {
		int layer = img["layer"];

		int basex = tilewidth_ / 2 + dx, basey = tilewidth_ / 2 + dy;
		if(const config::attribute_value* base_ = img.get("base")) {
			std::vector<std::string> base = utils::split(*base_);
			if(base.size() >= 2) {
				try {
					basex = std::stoi(base[0]);
					basey = std::stoi(base[1]);
				} catch(std::invalid_argument&) {
					ERR_NG << "Invalid 'base' value in terrain image builder: " << base[0] << ", " << base[1] << "\n";
				}
			}
		}

		int center_x = -1, center_y = -1;
		if(const config::attribute_value* center_ = img.get("center")) {
			std::vector<std::string> center = utils::split(*center_);
			if(center.size() >= 2) {
				try {
					center_x = std::stoi(center[0]);
					center_y = std::stoi(center[1]);
				} catch(std::invalid_argument&) {
					ERR_NG << "Invalid 'center' value in terrain image builder: " << center[0] << ", " << center[1]
						   << "\n";
				}
			}
		}

		bool is_water = img["is_water"].to_bool();

		images.push_back(rule_image(layer, basex - dx, basey - dy, global, center_x, center_y, is_water));

		// Adds the other variants of the image
		for(const config& variant : img.child_range("variant")) {
			const std::string& name = variant["name"];
			const std::string& variations = img["variations"];
			const std::string& tod = variant["tod"];
			const std::string& has_flag = variant["has_flag"];

			// If an integer is given then assign that, but if a bool is given, then assign -1 if true and 0 if false
			int random_start = variant["random_start"].to_bool(true) ? variant["random_start"].to_int(-1) : 0;

			images.back().variants.push_back(rule_image_variant(name, variations, tod, has_flag, random_start));
		}

		// Adds the main (default) variant of the image at the end,
		// (will be used only if previous variants don't match)
		const std::string& name = img["name"];
		const std::string& variations = img["variations"];

		int random_start = img["random_start"].to_bool(true) ? img["random_start"].to_int(-1) : 0;

		images.back().variants.push_back(rule_image_variant(name, variations, random_start));
	}
}

terrain_builder::terrain_constraint& terrain_builder::add_constraints(terrain_builder::constraint_set& constraints,
		const map_location& loc,
		const t_translation::ter_match& type,
		const config& global_images)
{
	terrain_constraint* cons = nullptr;
	for(terrain_constraint& c : constraints) {
		if(c.loc == loc) {
			cons = &c;
			break;
		}
	}

	if(!cons) {
		// The terrain at the current location did not exist, so create it
		constraints.emplace_back(loc);
		cons = &constraints.back();
	}

	if(!type.terrain.empty()) {
		cons->terrain_types_match = type;
	}

	int x = loc.x * tilewidth_ * 3 / 4;
	int y = loc.y * tilewidth_ + (loc.x % 2) * tilewidth_ / 2;
	add_images_from_config(cons->images, global_images, true, x, y);

	return *cons;
}

void terrain_builder::add_constraints(terrain_builder::constraint_set& constraints,
		const map_location& loc,
		const config& cfg,
		const config& global_images)

{
	terrain_constraint& constraint = add_constraints(
			constraints, loc, t_translation::ter_match(cfg["type"], t_translation::WILDCARD), global_images);

	std::vector<std::string> item_string = utils::square_parenthetical_split(cfg["set_flag"], ',', "[", "]");
	constraint.set_flag.insert(constraint.set_flag.end(), item_string.begin(), item_string.end());

	item_string = utils::square_parenthetical_split(cfg["has_flag"], ',', "[", "]");
	constraint.has_flag.insert(constraint.has_flag.end(), item_string.begin(), item_string.end());

	item_string = utils::square_parenthetical_split(cfg["no_flag"], ',', "[", "]");
	constraint.no_flag.insert(constraint.no_flag.end(), item_string.begin(), item_string.end());

	item_string = utils::square_parenthetical_split(cfg["set_no_flag"], ',', "[", "]");
	constraint.set_flag.insert(constraint.set_flag.end(), item_string.begin(), item_string.end());
	constraint.no_flag.insert(constraint.no_flag.end(), item_string.begin(), item_string.end());

	constraint.no_draw = cfg["no_draw"].to_bool(false);

	add_images_from_config(constraint.images, cfg, false);
}

void terrain_builder::parse_mapstring(
		const std::string& mapstring, struct building_rule& br, anchormap& anchors, const config& global_images)
{
	const t_translation::ter_map map = t_translation::read_builder_map(mapstring);

	// If there is an empty map leave directly.
	// Determine after conversion, since a
	// non-empty string can return an empty map.
	if(map.data.empty()) {
		return;
	}

	int lineno = (map.get(0, 0) == t_translation::NONE_TERRAIN) ? 1 : 0;
	int x = lineno;
	int y = 0;
	for(int y_off = 0; y_off < map.w; ++y_off) {
		for(int x_off = x; x_off < map.h; ++x_off) {
			const t_translation::terrain_code terrain = map.get(y_off, x_off);

			if(terrain.base == t_translation::TB_DOT) {
				// Dots are simple placeholders,
				// which do not represent actual terrains.
			} else if(terrain.overlay != 0) {
				anchors.emplace(terrain.overlay, map_location(x, y));
			} else if(terrain.base == t_translation::TB_STAR) {
				add_constraints(br.constraints, map_location(x, y), t_translation::STAR, global_images);
			} else {
				ERR_NG << "Invalid terrain (" << t_translation::write_terrain_code(terrain) << ") in builder map"
					   << std::endl;
				assert(false);
				return;
			}
			x += 2;
		}

		if(lineno % 2 == 1) {
			++y;
			x = 0;
		} else {
			x = 1;
		}
		++lineno;
	}
}

void terrain_builder::add_rule(building_ruleset& rules, building_rule& rule)
{
	if(load_images(rule)) {
		rules.insert(rule);
	}
}

void terrain_builder::add_rotated_rules(building_ruleset& rules, building_rule& tpl, const std::string& rotations)
{
	if(rotations.empty()) {
		// Adds the parsed built terrain to the list

		add_rule(rules, tpl);
	} else {
		const std::vector<std::string>& rot = utils::split(rotations, ',');

		for(size_t angle = 0; angle < rot.size(); ++angle) {
			/* Only 5% of the rules have valid images, so most of
			   them will be discarded. If the ratio was higher,
			   it would be more efficient to insert a copy of the
			   template rule into the ruleset, modify it in place,
			   and remove it if invalid. But since the ratio is so
			   low, the speedup is not worth the extra multiset
			   manipulations. */

			if(rot.at(angle) == "skip") {
				continue;
			}

			building_rule rule = tpl;
			rotate_rule(rule, angle, rot);
			add_rule(rules, rule);
		}
	}
}

void terrain_builder::parse_config(const config& cfg, bool local)
{
	log_scope("terrain_builder::parse_config");
	int n = 0;

	// Parses the list of building rules (BRs)
	for(const config& br : cfg.child_range("terrain_graphics")) {
		building_rule pbr; // Parsed Building rule
		pbr.local = local;

		// add_images_from_config(pbr.images, **br);

		pbr.location_constraints = map_location(br["x"].to_int() - 1, br["y"].to_int() - 1);

		pbr.modulo_constraints = map_location(br["mod_x"].to_int(), br["mod_y"].to_int());

		pbr.probability = br["probability"].to_int(100);

		// Mapping anchor indices to anchor locations.
		anchormap anchors;

		// Parse the map= , if there is one (and fill the anchors list)
		parse_mapstring(br["map"], pbr, anchors, br);

		// Parses the terrain constraints (TCs)
		for(const config& tc : br.child_range("tile")) {
			// Adds the terrain constraint to the current built terrain's list
			// of terrain constraints, if it does not exist.
			map_location loc;
			if(const config::attribute_value* v = tc.get("x")) {
				loc.x = *v;
			}
			if(const config::attribute_value* v = tc.get("y")) {
				loc.y = *v;
			}
			if(loc.valid()) {
				add_constraints(pbr.constraints, loc, tc, br);
			}
			if(const config::attribute_value* v = tc.get("pos")) {
				int pos = *v;
				if(anchors.find(pos) == anchors.end()) {
					WRN_NG << "Invalid anchor!" << std::endl;
					continue;
				}

				std::pair<anchormap::const_iterator, anchormap::const_iterator> range = anchors.equal_range(pos);

				for(; range.first != range.second; ++range.first) {
					loc = range.first->second;
					add_constraints(pbr.constraints, loc, tc, br);
				}
			}
		}

		const std::vector<std::string> global_set_flag = utils::split(br["set_flag"]);
		const std::vector<std::string> global_no_flag = utils::split(br["no_flag"]);
		const std::vector<std::string> global_has_flag = utils::split(br["has_flag"]);
		const std::vector<std::string> global_set_no_flag = utils::split(br["set_no_flag"]);

		for(terrain_constraint& constraint : pbr.constraints) {
			constraint.set_flag.insert(constraint.set_flag.end(), global_set_flag.begin(), global_set_flag.end());
			constraint.no_flag.insert(constraint.no_flag.end(), global_no_flag.begin(), global_no_flag.end());
			constraint.has_flag.insert(constraint.has_flag.end(), global_has_flag.begin(), global_has_flag.end());
			constraint.set_flag.insert(constraint.set_flag.end(), global_set_no_flag.begin(), global_set_no_flag.end());
			constraint.no_flag.insert(constraint.no_flag.end(), global_set_no_flag.begin(), global_set_no_flag.end());
		}

		// Handles rotations
		const std::string& rotations = br["rotations"];

		pbr.precedence = br["precedence"];

		add_rotated_rules(building_rules_, pbr, rotations);

		n++;
		if(n % 10 == 0) {
			gui2::dialogs::loading_screen::progress();
		}
	}

// Debug output for the terrain rules
#if 0
	std::cerr << "Built terrain rules: \n";

	building_ruleset::const_iterator rule;
	for(rule = building_rules_.begin(); rule != building_rules_.end(); ++rule) {
		std::cerr << ">> New rule: image_background = "
			<< "\n>> Location " << rule->second.location_constraints
			<< "\n>> Probability " << rule->second.probability

		for(constraint_set::const_iterator constraint = rule->second.constraints.begin();
		    constraint != rule->second.constraints.end(); ++constraint) {

			std::cerr << ">>>> New constraint: location = (" << constraint->second.loc
			          << "), terrain types = '" << t_translation::write_list(constraint->second.terrain_types_match.terrain) << "'\n";

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

void terrain_builder::add_off_map_rule(const std::string& image)
{
	// Build a config object
	config cfg;

	config& item = cfg.add_child("terrain_graphics");

	config& tile = item.add_child("tile");
	tile["x"] = 0;
	tile["y"] = 0;
	tile["type"] = t_translation::write_terrain_code(t_translation::OFF_MAP_USER);

	config& tile_image = tile.add_child("image");
	tile_image["layer"] = -1000;
	tile_image["name"] = image;

	item["probability"] = 100;
	item["no_flag"] = "base";
	item["set_flag"] = "base";

	// Parse the object
	parse_global_config(cfg);
}

bool terrain_builder::rule_matches(const terrain_builder::building_rule& rule,
		const map_location& loc,
		const terrain_constraint* type_checked) const
{
	// Don't match if the location isn't a multiple of mod_x and mod_y
	if(rule.modulo_constraints.x > 0 && (loc.x % rule.modulo_constraints.x != 0)) {
		return false;
	}
	if(rule.modulo_constraints.y > 0 && (loc.y % rule.modulo_constraints.y != 0)) {
		return false;
	}

	if(rule.location_constraints.valid() && rule.location_constraints != loc) {
		return false;
	}

	if(rule.probability != 100) {
		unsigned int random = get_noise(loc, rule.get_hash()) % 100;
		if(random > static_cast<unsigned int>(rule.probability)) {
			return false;
		}
	}

	for(const terrain_constraint& cons : rule.constraints) {
		// Translated location
		const map_location tloc = legacy_sum(loc, cons.loc);

		if(!tile_map_.on_map(tloc)) {
			return false;
		}

		// std::cout << "testing..." << builder_letter(map().get_terrain(tloc))

		// check if terrain matches except if we already know that it does
		if(&cons != type_checked && !terrain_matches(map().get_terrain(tloc), cons.terrain_types_match)) {
			return false;
		}

		const std::set<std::string>& flags = tile_map_[tloc].flags;

		for(const std::string& s : cons.no_flag) {
			// If a flag listed in "no_flag" is present, the rule does not match
			if(flags.find(s) != flags.end()) {
				return false;
			}
		}
		for(const std::string& s : cons.has_flag) {
			// If a flag listed in "has_flag" is not present, this rule does not match
			if(flags.find(s) == flags.end()) {
				return false;
			}
		}
	}

	return true;
}

void terrain_builder::apply_rule(const terrain_builder::building_rule& rule, const map_location& loc)
{
	unsigned int rand_seed = get_noise(loc, rule.get_hash());

	for(const terrain_constraint& constraint : rule.constraints) {
		const map_location tloc = legacy_sum(loc, constraint.loc);
		if(!tile_map_.on_map(tloc)) {
			return;
		}

		tile& btile = tile_map_[tloc];

		if(!constraint.no_draw) {
			for(const rule_image& img : constraint.images) {
				btile.images.push_back(tile::rule_image_rand(&img, rand_seed));
			}
		}

		// Sets flags
		for(const std::string& flag : constraint.set_flag) {
			btile.flags.insert(flag);
		}
	}
}

// copied from text_surface::hash()
// but keep it separated because the needs are different
// and changing it will modify the map random variations
static unsigned int hash_str(const std::string& str)
{
	unsigned int h = 0;
	for(std::string::const_iterator it = str.begin(), it_end = str.end(); it != it_end; ++it)
		h = ((h << 9) | (h >> (sizeof(int) * 8 - 9))) ^ (*it);
	return h;
}

unsigned int terrain_builder::building_rule::get_hash() const
{
	if(hash_ != DUMMY_HASH)
		return hash_;

	for(const terrain_constraint& constraint : constraints) {
		for(const rule_image& ri : constraint.images) {
			for(const rule_image_variant& variant : ri.variants) {
				// we will often hash the same string, but that seems fast enough
				hash_ += hash_str(variant.image_string);
			}
		}
	}

	// don't use the reserved dummy hash
	if(hash_ == DUMMY_HASH)
		hash_ = 105533; // just a random big prime number

	return hash_;
}

void terrain_builder::build_terrains()
{
	log_scope("terrain_builder::build_terrains");

	// Builds the terrain_by_type_ cache
	for(int x = -2; x <= map().w(); ++x) {
		for(int y = -2; y <= map().h(); ++y) {
			const map_location loc(x, y);
			const t_translation::terrain_code t = map().get_terrain(loc);

			terrain_by_type_[t].push_back(loc);

			// Flag all hexes according to whether they're on the border or not,
			// to make it easier for WML to draw the borders
			if(draw_border_&& !map().on_board(loc)) {
				tile_map_[loc].flags.insert("_border");
			} else {
				tile_map_[loc].flags.insert("_board");
			}
		}
	}

	for(const building_rule& rule : building_rules_) {
		// Find the constraint that contains the less terrain of all terrain rules.
		// We will keep a track of the matching terrains of this constraint
		// and later try to apply the rule only on them
		size_t min_size = INT_MAX;
		t_translation::ter_list min_types = t_translation::ter_list(); // <-- This must be explicitly initialized, just
																	   // as min_constraint is, at start of loop, or we
																	   // get a null pointer dereference when we go
																	   // through on later times.
		const terrain_constraint* min_constraint = nullptr;

		for(const terrain_constraint& constraint : rule.constraints) {
			const t_translation::ter_match& match = constraint.terrain_types_match;
			t_translation::ter_list matching_types;
			size_t constraint_size = 0;

			for(terrain_by_type_map::iterator type_it = terrain_by_type_.begin(); type_it != terrain_by_type_.end();
					++type_it) {
				const t_translation::terrain_code t = type_it->first;
				if(terrain_matches(t, match)) {
					const size_t match_size = type_it->second.size();
					constraint_size += match_size;
					if(constraint_size >= min_size) {
						break; // not a minimum, bail out
					}
					matching_types.push_back(t);
				}
			}

			if(constraint_size < min_size) {
				min_size = constraint_size;
				min_types = matching_types;
				min_constraint = &constraint;
				if(min_size == 0) {
					// a constraint is never matched on this map
					// we break with a empty type list
					break;
				}
			}
		}

		assert(min_constraint != nullptr);

		// NOTE: if min_types is not empty, we have found a valid min_constraint;
		for(t_translation::ter_list::const_iterator t = min_types.begin(); t != min_types.end(); ++t) {
			const std::vector<map_location>* locations = &terrain_by_type_[*t];

			for(std::vector<map_location>::const_iterator itor = locations->begin(); itor != locations->end(); ++itor) {
				const map_location loc = legacy_difference(*itor, min_constraint->loc);

				if(rule_matches(rule, loc, min_constraint)) {
					apply_rule(rule, loc);
				}
			}
		}
	}
}

terrain_builder::tile* terrain_builder::get_tile(const map_location& loc)
{
	if(tile_map_.on_map(loc))
		return &(tile_map_[loc]);
	return nullptr;
}
