/*
   Copyright (C) 2014 - 2017 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "serialization/string_utils.hpp"

class attack_type;
namespace t_translation { struct terrain_code; }


/// The basic "size" of the unit - flying, small land, large land, etc.
/// This encompasses terrain costs, defenses, and resistances.
class movetype
{
	/// Stores a set of data based on terrain.
	class terrain_info
	{
		/// The terrain-based data.
		class data;
		// The data class is not defined here to keep the header file cleaner.

	public:
		/// The parameters used when calculating a terrain-based value.
		struct parameters;

		explicit terrain_info(const parameters & params,
		                      const terrain_info * fallback=nullptr,
		                      const terrain_info * cascade=nullptr);
		terrain_info(const config & cfg, const parameters & params,
		             const terrain_info * fallback=nullptr,
		             const terrain_info * cascade=nullptr);
		terrain_info(const terrain_info & that,
		             const terrain_info * fallback=nullptr,
		             const terrain_info * cascade=nullptr);
		~terrain_info();

		terrain_info & operator=(const terrain_info & that);


		/// Clears the cache of values.
		void clear_cache() const;
		/// Returns whether or not our data is empty.
		bool empty() const;
		/// Merges the given config over the existing values.
		void merge(const config & new_values, bool overwrite);
		/// Returns the value associated with the given terrain.
		int value(const t_translation::terrain_code & terrain) const;
		/// Writes our data to a config.
		void write(config & cfg, const std::string & child_name="", bool merged=true) const;

	private:
		// Returns a pointer to data the incorporates our fallback.
		const std::shared_ptr<data> & get_merged() const;
		// Ensures our data is not shared, and propagates to our cascade.
		void make_unique_cascade() const;
		// Ensures our data is not shared, and propagates to our fallback.
		void make_unique_fallback() const;

	private:
		std::shared_ptr<data> data_;                 /// Never nullptr
		mutable std::shared_ptr<data> merged_data_;  /// Created as needed.
		const terrain_info * const fallback_;
		const terrain_info * const cascade_;
	};


public:
	/// Magic value that signifies a hex is unreachable.
	/// The UNREACHABLE macro in the data tree should match this value.
	static const int UNREACHABLE = 99;

	/// Stores a set of terrain costs (for movement, vision, or "jamming").
	class terrain_costs : public terrain_info
	{
		static const parameters params_;
	public:
		explicit terrain_costs(const terrain_costs * fallback=nullptr,
		                       const terrain_costs * cascade=nullptr) :
			terrain_info(params_, fallback, cascade)
		{}
		explicit terrain_costs(const config & cfg,
		                       const terrain_costs * fallback=nullptr,
		                       const terrain_costs * cascade=nullptr) :
			terrain_info(cfg, params_, fallback, cascade)
		{}
		terrain_costs(const terrain_costs & that,
		              const terrain_costs * fallback=nullptr,
		              const terrain_costs * cascade=nullptr) :
			terrain_info(that, fallback, cascade)
		{}

		/// Returns the cost associated with the given terrain.
		/// Costs are doubled when @a slowed is true.
		int cost(const t_translation::terrain_code & terrain, bool slowed=false) const
		{ int result = value(terrain);
		  return  slowed  &&  result != movetype::UNREACHABLE ? 2 * result : result; }

		// Inherited from terrain_info:
		//void merge(const config & new_values, bool overwrite);
		//void write(config & cfg, const std::string & child_name="", bool merged=true) const;
	};

	/// Stores a set of defense levels.
	class terrain_defense
	{
		static const terrain_info::parameters params_min_;
		static const terrain_info::parameters params_max_;

	public:
		terrain_defense() : min_(params_min_), max_(params_max_) {}
		explicit terrain_defense(const config & cfg) :
			min_(cfg, params_min_), max_(cfg, params_max_)
		{}

		/// Returns the defense associated with the given terrain.
		int defense(const t_translation::terrain_code & terrain) const
		{ return std::max(min_.value(terrain), max_.value(terrain)); }
		/// Returns whether there is a defense cap associated to this terrain.
		bool capped(const t_translation::terrain_code & terrain) const
		{ return min_.value(terrain) != 0; }
		/// Merges the given config over the existing costs.
		/// (Not overwriting implies adding.)
		void merge(const config & new_data, bool overwrite)
		{ min_.merge(new_data, overwrite);  max_.merge(new_data, overwrite); }
		/// Writes our data to a config, as a child if @a child_name is specified.
		/// (No child is created if there is no data.)
		void write(config & cfg, const std::string & child_name="") const
		{ max_.write(cfg, child_name, false); }

	private:
		// There will be duplication of the config here, but it is a small
		// config, and the duplication allows greater code sharing.
		terrain_info min_;
		terrain_info max_;
	};

	/// Stores a set of resistances.
	class resistances
	{
	public:
		resistances() : cfg_() {}
		explicit resistances(const config & cfg) : cfg_(cfg) {}

		/// Returns a map from attack types to resistances.
		utils::string_map damage_table() const;
		/// Returns the resistance against the indicated attack.
		int resistance_against(const attack_type & attack) const;
		/// Returns the resistance against the indicated damage type.
		int resistance_against(const std::string & damage_type) const;
		/// Merges the given config over the existing costs.
		void merge(const config & new_data, bool overwrite);
		/// Writes our data to a config, as a child if @a child_name is specified.
		void write(config & out_cfg, const std::string & child_name="") const;

	private:
		config cfg_;
	};

public:
	movetype();
	explicit movetype(const config & cfg);
	movetype(const movetype & that);

	// This class is basically just a holder for its various pieces, so
	// provide access to those pieces on demand.
	terrain_costs & get_movement()   { return movement_; }
	terrain_costs & get_vision()     { return vision_; }
	terrain_costs & get_jamming()    { return jamming_; }
	terrain_defense & get_defense()  { return defense_; }
	resistances & get_resistances()  { return resist_; }
	// And const access:
	const terrain_costs & get_movement()  const { return movement_; }
	const terrain_costs & get_vision()    const { return vision_; }
	const terrain_costs & get_jamming()   const { return jamming_; }
	const terrain_defense & get_defense() const { return defense_; }
	const resistances & get_resistances() const { return resist_; }

	/// Returns whether or not *this is flagged as a flying movement type.
	bool is_flying() const { return flying_; }
	/// Sets whether or not *this is flagged as a flying movement type.
	void set_flying(bool flies=true) { flying_ = flies; }

	/// Returns the cost to move through the indicated terrain.
	int movement_cost(const t_translation::terrain_code & terrain, bool slowed=false) const
	{ return movement_.cost(terrain, slowed); }
	/// Returns the cost to see through the indicated terrain.
	int vision_cost(const t_translation::terrain_code & terrain, bool slowed=false) const
	{ return vision_.cost(terrain, slowed); }
	/// Returns the cost to "jam" through the indicated terrain.
	int jamming_cost(const t_translation::terrain_code & terrain, bool slowed=false) const
	{ return jamming_.cost(terrain, slowed); }

	/// Returns the defensive value of the indicated terrain.
	int defense_modifier(const t_translation::terrain_code & terrain) const
	{ return defense_.defense(terrain); }

	/// Returns the resistance against the indicated attack.
	int resistance_against(const attack_type & attack) const
	{ return resist_.resistance_against(attack); }
	/// Returns the resistance against the indicated damage type.
	int resistance_against(const std::string & damage_type) const
	{ return resist_.resistance_against(damage_type); }
	/// Returns a map from attack types to resistances.
	utils::string_map damage_table() const
	{ return resist_.damage_table(); }

	/// Returns whether or not there are any terrain caps with respect to a set of terrains.
	bool has_terrain_defense_caps(const std::set<t_translation::terrain_code> & ts) const;
	/// Returns whether or not there are any vision-specific costs.
	bool has_vision_data()  const { return !vision_.empty(); }
	/// Returns whether or not there are any jamming-specific costs.
	bool has_jamming_data() const { return !jamming_.empty(); }

	/// Merges the given config over the existing data.
	void merge(const config & new_cfg, bool overwrite=true);

	/// The set of applicable effects for movement types
	static const std::set<std::string> effects;

	/// Writes the movement type data to the provided config.
	void write(config & cfg) const;

private:
	terrain_costs movement_;
	terrain_costs vision_;
	terrain_costs jamming_;
	terrain_defense defense_;
	resistances resist_;

	bool flying_;
};
