/*
	Copyright (C) 2004 - 2024
	by Philippe Plantier <ayin@anathas.org>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
 * Definitions for the terrain builder.
 */

#pragma once

#include "animated.hpp"
#include "map/location.hpp"
#include "terrain/translation.hpp"

class config;
class game_config_view;

class gamemap;
namespace image
{
class locator;
}
/**
 * The class terrain_builder is constructed from a config object, and a
 * gamemap object. On construction, it parses the configuration and extracts
 * the list of [terrain_graphics] rules. Each terrain_graphics rule attaches
 * one or more images to a specific terrain pattern.
 * It then applies the rules loaded from the configuration to the current map,
 * and calculates the list of images that must be associated to each hex of
 * the map.
 *
 * The get_terrain_at method can then be used to obtain the list of images
 * necessary to draw the terrain on a given tile.
 */
class terrain_builder
{
public:
	/** Used as a parameter for the get_terrain_at function. */
	enum TERRAIN_TYPE {
		BACKGROUND, /**<
					 * Represents terrains which are to be
					 * drawn behind unit sprites
					 */
		FOREGROUND  /**<
					 * Represents terrains which are to be
					 * drawn in front of them.
					 */
	};

	/** The position of unit graphics in a tile. Graphics whose y
	 * position is below this value are considered background for
	 * this tile; graphics whose y position is above this value are
	 * considered foreground.
	 */
	static const int UNITPOS = 36 + 18;

	static const unsigned int DUMMY_HASH = 0;

	/** A shorthand typedef for a list of animated image locators,
	 * the base data type returned by the get_terrain_at method.
	 */
	typedef std::vector<animated<image::locator>> imagelist;

	/** Constructor for the terrain_builder class.
	 *
	 * @param level		A level (scenario)-specific configuration file,
	 *		    containing scenario-specific [terrain_graphics] rules.
	 * @param map			A properly-initialized gamemap object representing
	 *						the current terrain map.
	 * @param offmap_image	The filename of the image which will be used as
	 *						off map image (see add_off_map_rule()).
	 *						This image automatically gets the 'terrain/' prefix
	 *						and '.png' suffix
	 * @param draw_border    Whether the map border flag should be set to allow
	 *                      its drawing.
	 */
	terrain_builder(const config& level, const gamemap* map, const std::string& offmap_image, bool draw_border);

	/**  Set the config where we will parse the global terrain rules.
	 *   This also flushes the terrain rules cache.
	 *
	 * @param cfg			The main game configuration object, where the
	 *						[terrain_graphics] rule reside.
	 */
	static void set_terrain_rules_cfg(const game_config_view& cfg);

	const gamemap& map() const
	{
		return *map_;
	}

	/**
	 * Updates internals that cache map size. This should be called when the map
	 * size has changed.
	 */
	void reload_map();

	void change_map(const gamemap* m);

	/** Returns a vector of strings representing the images to load & blit
	 * together to get the built content for this tile.
	 *
	 * @param loc   The location relative the the terrain map,
	 *				where we ask for the image list
	 * @param tod   The string representing the current time-of day.
	 *				Will be used if some images specify several
	 *				time-of-day- related variants.
	 * @param terrain_type BACKGROUND or FOREGROUND,
	 *              depending on whether we ask for the terrain which is
	 *              before, or after the unit sprite.
	 *
	 * @return      Returns a pointer list of animated images corresponding
	 *              to the parameters, or nullptr if there is none.
	 */
	const imagelist* get_terrain_at(const map_location& loc, const std::string& tod, TERRAIN_TYPE const terrain_type);

	/** Updates the animation at a given tile.
	 * Returns true if something has changed, and must be redrawn.
	 *
	 * @param loc   the location to update
	 *
	 * @retval      true: this tile must be redrawn.
	 */
	bool update_animation(const map_location& loc);

	/** Performs a "quick-rebuild" of the terrain in a given location.
	 * The "quick-rebuild" is no proper rebuild: it only clears the
	 * terrain cache for a given location, and replaces it with a single,
	 * default image for this terrain.
	 *
	 * @param loc   the location where to rebuild terrains
	 */
	void rebuild_terrain(const map_location& loc);

	/** Performs a complete rebuild of the list of terrain graphics
	 * attached to a map.
	 * Should be called when a terrain is changed in the map.
	 */
	void rebuild_all();

	void rebuild_cache_all();

	void set_draw_border(bool do_draw)
	{
		draw_border_ = do_draw;
	}

	/**
	 * An image variant. The in-memory representation of the [variant]
	 * WML tag of the [image] WML tag. When an image only has one variant,
	 * the [variant] tag may be omitted.
	 */
	struct rule_image_variant
	{
		/** Constructor for the normal default case */
		rule_image_variant(const std::string& image_string,
			const std::string& variations,
			const std::chrono::milliseconds& random_start = std::chrono::milliseconds{-1});

		/** Constructor for true [variant] cases */
		rule_image_variant(const std::string& image_string,
				const std::string& variations,
				const std::string& tod,
				const std::string& has_flag,
				const std::chrono::milliseconds& random_start = std::chrono::milliseconds{-1});

		/** A string representing either the filename for an image, or
		 *  a list of images, with an optional timing for each image.
		 *  Corresponds to the "name" parameter of the [variant] (or of
		 *  the [image]) WML tag.
		 *
		 *  The timing string is in the following format (expressed in EBNF)
		 *
		 *@verbatim
		 *  <timing_string> ::= <timed_image> ( "," <timed_image> ) +
		 *
		 *  <timed_image> ::= <image_name> [ ":" <timing> ]
		 *
		 *  Where <image_name> represents the actual filename of an image,
		 *  and <timing> the number of milliseconds this image will last
		 *  in the animation.
		 *@endverbatim
		 */
		std::string image_string;

		/** A semi-solon separated list of string used to replace
		 * @verbatim <code>@V</code> @endverbatim in image_string (if present)
		 */
		std::string variations;

		/** An animated image locator built according to the image string.
		 * This will be the image locator which will actually
		 * be returned to the user.
		 */
		std::vector<animated<image::locator>> images;

		/** The Time of Day associated to this variant (if any)*/
		std::set<std::string> tods;

		std::vector<std::string> has_flag;

		/** Specify the allowed amount of random shift (in milliseconds) applied
		 * to the animation start time, -1 for shifting without limitation.*/
		std::chrono::milliseconds random_start;
	};

	/**
	 * Each terrain_graphics rule is associated a set of images, which are
	 * applied on the terrain if the rule matches. An image is more than
	 * graphics: it is graphics (with several possible tod-alternatives,)
	 * and a position for these graphics.
	 * The rule_image structure represents one such image.
	 */
	struct rule_image
	{
		bool is_background() const
		{
			return layer < 0 || (layer == 0 && basey < UNITPOS);
		}

		/** The layer of the image for horizontal layering */
		int layer;
		/** The position of the image base (that is, the point where
		 * the image reaches the floor) for vertical layering
		 */
		int basex, basey;

		/** Set to true if the image was defined as a child of the
		 * [terrain_graphics] tag, set to false if it was defined as a
		 * child of a [tile] tag */
		bool global_image = false;

		/** The position where the center of the image base should be
		 */
		int center_x = -1, center_y = -1;

		bool is_water = false;

		/** A list of variants for this image */
		std::vector<rule_image_variant> variants{};
	};

	/**
	 * A shorthand notation for a vector of rule_images
	 */
	typedef std::vector<rule_image> rule_imagelist;

	/**
	 * The in-memory representation of a [tile] WML rule
	 * inside of a [terrain_graphics] WML rule.
	 */
	struct terrain_constraint
	{
		map_location loc{};
		t_translation::ter_match terrain_types_match{};
		std::vector<std::string> set_flag{};
		std::vector<std::string> no_flag{};
		std::vector<std::string> has_flag{};

		/** Whether to actually draw the images onto this hex or not */
		bool no_draw = false;

		rule_imagelist images{};
	};

	/**
	 * Represents a tile of the game map, with all associated
	 * builder-specific parameters: flags, images attached to this tile,
	 * etc. An array of those tiles is built when terrains are built either
	 * during construction, or upon calling the rebuild_all() method.
	 */
	struct tile
	{
		struct rule_image_rand;
		typedef std::pair<const rule_image_rand*, const rule_image_variant*> log_details;
		typedef std::vector<log_details> logs;
		/** Rebuilds the whole image cache, for a given time-of-day.
		 * Must be called when the time-of-day has changed,
		 * to select the correct images.
		 *
		 * @param tod    The current time-of-day
		 * @param log
		 */
		void rebuild_cache(const std::string& tod, logs* log = nullptr);

		/** Clears all data in this tile, and resets the cache */
		void clear();

		/** The list of flags present in this tile */
		std::set<std::string> flags;

		/** Represent a rule_image applied with a random seed.*/
		struct rule_image_rand
		{
			const rule_image* operator->() const
			{
				return ri;
			}
			/** sort by layer first then by basey */
			bool operator<(const rule_image_rand& o) const
			{
				return std::tie(ri->layer, ri->basey) < std::tie(o.ri->layer, o.ri->basey);
			}

			const rule_image* ri;
			unsigned int rand;
		};

		/** The list of rule_images and random seeds associated to this tile.
		 */
		std::vector<rule_image_rand> images;

		/** The list of images which are in front of the unit sprites,
		 * attached to this tile. This member is considered a cache:
		 * it is built once, and on-demand.
		 */
		imagelist images_foreground;
		/** The list of images which are behind the unit sprites,
		 * attached to this tile. This member is considered a cache:
		 * it is built once, and on-demand.
		 */
		imagelist images_background;
		/**
		 * The time-of-day to which the image caches correspond.
		 */
		std::string last_tod = "invalid_tod";

		/** Indicates if 'images' is sorted */
		bool sorted_images = false;
	};

	tile* get_tile(const map_location& loc);

private:
	/** The tile width used when using basex and basey. This is not,
	 * necessarily, the tile width in pixels, this is totally
	 * arbitrary. However, it will be set to 72 for convenience.
	 */
	const int tilewidth_; // = game_config::tile_size;

	/**
	 * The list of constraints attached to a terrain_graphics WML rule.
	 */
	typedef std::vector<terrain_constraint> constraint_set;

	/**
	 * The in-memory representation of a [terrain_graphics] WML rule.
	 */
	struct building_rule
	{
		/**
		 * The set of [tile] constraints of this rule.
		 */
		constraint_set constraints;

		/**
		 * The location on which this map may match.
		 * Set to a valid map_location if the "x" and "y" parameters
		 * of the [terrain_graphics] rule are set.
		 */
		map_location location_constraints;

		/**
		 * Used to constrain locations to ones with coordinates that are
		 * multiples of the "mod_x" and "mod_y" parameters. Doesn't actually
		 * refer to a real map location.
		 */
		map_location modulo_constraints;

		/**
		 * The probability of this rule to match, when all conditions
		 * are met. Defined if the "probability" parameter of the
		 * [terrain_graphics] element is set.
		 */
		int probability = 100;

		/**
		 * Ordering relation between the rules.
		 */
		int precedence = 0;

		/**
		 * Indicate if the rule is only for this scenario
		 */
		bool local = false;

		bool operator<(const building_rule& that) const
		{
			return precedence < that.precedence;
		}

		unsigned int get_hash() const;

	private:
		mutable unsigned int hash_ = DUMMY_HASH;
	};

	/**
	 * The map of "tile" structures corresponding to the level map.
	 */
	class tilemap
	{
	public:
		/**
		 * Constructs a tilemap of dimensions x * y
		 */
		tilemap(int x, int y);

		/**
		 * Returns a reference to the tile which is at the position
		 * pointed by loc. The location MUST be on the map!
		 *
		 * @param loc    The location of the tile
		 *
		 * @return		A reference to the tile at this location.
		 *
		 */
		tile& operator[](const map_location& loc);
		/**
		 * a const variant of operator[]
		 */
		const tile& operator[](const map_location& loc) const;

		/**
		 * Tests if a location is on the map.
		 *
		 * @param loc   The location to test
		 *
		 * @return		true if loc is on the map, false otherwise.
		 */
		bool on_map(const map_location& loc) const;

		/**
		 * Resets the whole tile map
		 */
		void reset();

		/**
		 * Rebuilds the map to a new set of dimensions
		 */
		void reload(int x, int y);

	private:
		/** The map */
		std::vector<tile> tiles_;
		/** The x dimension of the map */
		int x_;
		/** The y dimension of the map */
		int y_;
	};

	/**
	 * A set of building rules. In-memory representation
	 * of the whole set of [terrain_graphics] rules.
	 */
	typedef std::multiset<building_rule> building_ruleset;

	/**
	 * Load images and tests for validity of a rule. A rule is considered
	 * valid if all its images are present. This method is used, when building
	 * the ruleset, to only add rules which are valid to the ruleset.
	 *
	 * @param rule  The rule to test for validity
	 *
	 * @return		true if the rule is valid, false if it is not.
	 */
	bool load_images(building_rule& rule);

	/**
	 * Starts the animation on a rule.
	 *
	 * @param rule  The rule on which to start animations
	 *
	 * @return		true
	 */
	bool start_animation(building_rule& rule);

	/**
	 *  "Rotates" a constraint from a rule.
	 *  Takes a template constraint from a template rule, and rotates
	 *  to the given angle.
	 *
	 *  On a constraint, the relative position of each rule, and the "base"
	 *  of each vertical images, are rotated according to the given angle.
	 *
	 *  Template terrain constraints are defined like normal terrain
	 *  constraints, except that, flags, and image filenames,
	 *  may contain template strings of the form
	 *@verbatim
	 *  <code>@Rn</code>,
	 *@endverbatim
	 *  n being a number from 0 to 5.
	 *  See the rotate_rule method for more info.
	 *
	 *  @param constraint  A template constraint to rotate
	 *  @param angle       An int, from 0 to 5, representing the rotation angle.
	 */
	void rotate(terrain_constraint& constraint, int angle);

	/**
	 * Replaces, in a given string, rotation tokens with their values.
	 *
	 * @param s            the string in which to do the replacement
	 * @param angle        the angle for substituting the correct replacement.
	 * @param replacement  the replacement strings.
	 */
	void replace_rotate_tokens(std::string& s, int angle, const std::vector<std::string>& replacement);

	/**
	 * Replaces, in a given rule_image, rotation tokens with their values.
	 * The actual substitution is done in all variants of the given image.
	 *
	 * @param image        the rule_image in which to do the replacement.
	 * @param angle        the angle for substituting the correct replacement.
	 * @param replacement  the replacement strings.
	 */
	void replace_rotate_tokens(rule_image& image, int angle, const std::vector<std::string>& replacement);

	/**
	 * Replaces, in a given rule_variant_image, rotation tokens with their values.
	 * The actual substitution is done in the "image_string" parameter
	 * of this rule_variant_image.
	 *
	 * @param variant      the rule_variant_image in which to do the replacement.
	 * @param angle        the angle for substituting the correct replacement.
	 * @param replacement  the replacement strings.
	 */
	void replace_rotate_tokens(rule_image_variant& variant, int angle, const std::vector<std::string>& replacement)
	{
		replace_rotate_tokens(variant.image_string, angle, replacement);
	}

	/**
	 * Replaces, in a given rule_imagelist, rotation tokens with their values.
	 * The actual substitution is done in all rule_images contained
	 * in the rule_imagelist.
	 *
	 * @param list         the rule_imagelist in which to do the replacement.
	 * @param angle        the angle for substituting the correct replacement.
	 * @param replacement  the replacement strings.
	 */
	void replace_rotate_tokens(rule_imagelist& list, int angle, const std::vector<std::string>& replacement);

	/**
	 * Replaces, in a given building_rule, rotation tokens with their values.
	 * The actual substitution is done in the rule_imagelists contained
	 * in all constraints of the building_rule, and in the flags
	 * (has_flag, set_flag and no_flag) contained in all constraints
	 * of the building_rule.
	 *
	 * @param rule         the building_rule in which to do the replacement.
	 * @param angle        the angle for substituting the correct replacement.
	 * @param replacement  the replacement strings.
	 */
	void replace_rotate_tokens(building_rule& rule, int angle, const std::vector<std::string>& replacement);

	/**
	 *  Rotates a template rule to a given angle.
	 *
	 *  Template rules are defined like normal rules, except that:
	 *  * Flags and image filenames may contain template strings of the form
	 *@verbatim
	 *  <code>@Rn</code>, n being a number from 0 to 5.
	 *@endverbatim
	 *  * The rule contains the rotations=r0,r1,r2,r3,r4,r5, with r0 to r5
	 *    being strings describing the 6 different positions, typically,
	 *    n, ne, se, s, sw, and nw (but maybe anything else.)
	 *
	 *  A template rule will generate 6 rules, which are similar
	 *  to the template, except that:
	 *
	 *  * The map of constraints ( [tile]s ) of this rule will be
	 *    rotated by an angle, of 0 to 5 pi / 6
	 *
	 *  * On the rule which is rotated to 0rad, the template strings
	 *@verbatim
	 *    @R0, @R1, @R2, @R3, @R4, @R5,
	 *@endverbatim
	 *    will be replaced by the corresponding r0, r1, r2, r3, r4, r5
	 *    variables given in the rotations= element.
	 *
	 *  * On the rule which is rotated to pi/3 rad, the template strings
	 *@verbatim
	 *    @R0, @R1, @R2 etc.
	 *@endverbatim
	 *    will be replaced by the corresponding
	 *    <strong>r1, r2, r3, r4, r5, r0</strong> (note the shift in indices).
	 *
	 *  * On the rule rotated 2pi/3, those will be replaced by
	 *    r2, r3, r4, r5, r0, r1 and so on.
	 *
	 */
	void rotate_rule(building_rule& rule, int angle, const std::vector<std::string>& angle_name);

	/**
	 * Parses a "config" object, which should contains [image] children,
	 * and adds the corresponding parsed rule_images to a rule_imagelist.
	 *
	 * @param images   The rule_imagelist into which to add the parsed images.
	 * @param cfg      The WML configuration object to parse
	 * @param global   Whether those [image]s elements belong to a
	 *                 [terrain_graphics] element, or to a [tile] child.
	 *                 Set to true if those belong to a [terrain_graphics]
	 *                 element.
	 * @param dx       The X coordinate of the constraint those images
	 *                 apply to, relative to the start of the rule. Only
	 *                 meaningful if global is set to false.
	 * @param dy       The Y coordinate of the constraint those images
	 *                 apply to.
	 */
	void add_images_from_config(rule_imagelist& images, const config& cfg, bool global, int dx = 0, int dy = 0);

	/**
	 * Creates a rule constraint object which matches a given list of
	 * terrains, and adds it to the list of constraints of a rule.
	 *
	 * @param constraints  The constraint set to which to add the constraint.
	 * @param loc           The location of the constraint
	 * @param type          The list of terrains this constraint will match
	 * @param global_images A configuration object containing [image] tags
	 *                      describing rule-global images.
	 */
	terrain_constraint& add_constraints(constraint_set& constraints,
			const map_location& loc,
			const t_translation::ter_match& type,
			const config& global_images);

	/**
	 * Creates a rule constraint object from a config object and
	 * adds it to the list of constraints of a rule.
	 *
	 * @param constraints   The constraint set to which to add the constraint.
	 * @param loc           The location of the constraint
	 * @param cfg           The config object describing this constraint.
	 *                      Usually, a [tile] child of a [terrain_graphics] rule.
	 * @param global_images A configuration object containing [image] tags
	 *                      describing rule-global images.
	 */
	void add_constraints(
			constraint_set& constraints, const map_location& loc, const config& cfg, const config& global_images);

	typedef std::multimap<int, map_location> anchormap;

	/**
	 * Parses a map string (the map= element of a [terrain_graphics] rule,
	 * and adds constraints from this map to a building_rule.
	 *
	 * @param mapstring     The map vector to parse
	 * @param br            The building rule into which to add the extracted
	 *                      constraints
	 * @param anchors       A map where to put "anchors" extracted from the map.
	 * @param global_images A config object representing the images defined
	 *                      as direct children of the [terrain_graphics] rule.
	 */
	void parse_mapstring(
			const std::string& mapstring, struct building_rule& br, anchormap& anchors, const config& global_images);

	/**
	 * Adds a rule to a ruleset. Checks for validity before adding the rule.
	 *
	 * @param rules      The ruleset into which to add the rules.
	 * @param rule       The rule to add.
	 */
	void add_rule(building_ruleset& rules, building_rule& rule);

	/**
	 * Adds a set of rules to a ruleset, from a template rule which spans
	 * 6 rotations (or less if some of the rotated rules are invalid).
	 *
	 * @param rules      The ruleset into which to add the rules.
	 * @param tpl        The template rule
	 * @param rotations  A comma-separated string containing the
	 *                   6 values for replacing rotation template
	 *                   template strings @verbatim (@Rn) @endverbatim
	 */
	void add_rotated_rules(building_ruleset& rules, building_rule& tpl, const std::string& rotations);

	/**
	 * Parses a configuration object containing [terrain_graphics] rules,
	 * and fills the building_rules_ member of the current class according
	 * to those.
	 *
	 * @param cfg       The configuration object to parse.
	 * @param local     Mark the rules as local only.
	 */
	void parse_config(const config& cfg, bool local = true);
	void parse_config(const game_config_view& cfg, bool local = true);

	void parse_global_config(const game_config_view& cfg)
	{
		parse_config(cfg, false);
	}

	/**
	 * Adds a builder rule for the _off^_usr tile, this tile only has 1 image.
	 *
	 * @param image		The filename of the image
	 */
	void add_off_map_rule(const std::string& image);

	void flush_local_rules();

	/**
	 * Checks whether a terrain code matches a given list of terrain codes.
	 *
	 * @param tcode 	The terrain to check
	 * @param terrains	The terrain list against which to check the terrain.
	 *	May contain the metacharacters
	 *	- '*' STAR, meaning "all terrains"
	 *	- '!' NOT,  meaning "all terrains except those present in the list."
	 *
	 * @return			returns true if "tcode" matches the list or the list is empty,
	 *					else false.
	 */
	bool terrain_matches(const t_translation::terrain_code& tcode, const t_translation::ter_list& terrains) const
	{
		return terrains.empty() ? true : t_translation::terrain_matches(tcode, terrains);
	}

	/**
	 * Checks whether a terrain code matches a given list of terrain tcodes.
	 *
	 * @param tcode 	The terrain code to check
	 * @param terrain	The terrain match structure which to check the terrain.
	 *	See previous definition for more details.
	 *
	 * @return			returns true if "tcode" matches the list or the list is empty,
	 *					else false.
	 */
	bool terrain_matches(const t_translation::terrain_code& tcode, const t_translation::ter_match& terrain) const
	{
		return terrain.is_empty ? true : t_translation::terrain_matches(tcode, terrain);
	}

	/**
	 * Checks whether a rule matches a given location in the map.
	 *
	 * @param rule      The rule to check.
	 * @param loc       The location in the map where we want to check
	 *                  whether the rule matches.
	 * @param type_checked The constraint which we already know that its
	 *                  terrain types matches.
	 */
	bool rule_matches(const building_rule& rule, const map_location& loc, const terrain_constraint* type_checked) const;

	/**
	 * Applies a rule at a given location: applies the result of a
	 * matching rule at a given location: attachs the images corresponding
	 * to the rule, and sets the flags corresponding to the rule.
	 *
	 * @param rule      The rule to apply
	 * @param loc       The location to which to apply the rule.
	 */
	void apply_rule(const building_rule& rule, const map_location& loc);

	/**
	 * Calculates the list of terrains, and fills the tile_map_ member,
	 * from the gamemap and the building_rules_.
	 */
	void build_terrains();

	/**
	 * A pointer to the gamemap class used in the current level.
	 */
	const gamemap* map_;

	/**
	 * The tile_map_ for the current level, which is filled by the
	 * build_terrains_ method to contain "tiles" representing images
	 * attached to each tile.
	 */
	tilemap tile_map_;

	/**
	 * Shorthand typedef for a map associating a list of locations to a terrain type.
	 */
	typedef std::map<t_translation::terrain_code, std::vector<map_location>> terrain_by_type_map;

	/**
	 * A map representing all locations whose terrain is of a given type.
	 */
	terrain_by_type_map terrain_by_type_;

	/** Whether the map border should be drawn. */
	bool draw_border_;

	/** Parsed terrain rules. Cached between instances */
	static inline building_ruleset building_rules_{};

	/** Config used to parse global terrain rules */
	static const inline game_config_view* rules_cfg_ = nullptr;
};
