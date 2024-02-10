/*
	Copyright (C) 2014 - 2024
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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


/**
 * The basic "size" of the unit - flying, small land, large land, etc.
 * This encompasses terrain costs, defenses, and resistances.
 *
 * This class is used for both [movetype] and [unit] configs, which use the
 * same data in their configs for [movement_costs], [defense], etc. However,
 * the data for whether the unit flies is historically held in [movetype]'s
 * "flies" vs [unit]'s "flying".
 *
 * Existing behavior of 1.14:
 * * movetype::movetype(const config & cfg) will read only the "flies" key
 * * movetype::merge(const config & cfg, bool overwrite) will read both keys,
 *     with "flying" taking priority if both are supplied
 * * movetype::write() will write only the "flying" key
 *
 * @todo make this more logical. Ideas:
 * * for 1.15, support both "flying" and "flies" in [movetype]
 * * for 1.17 or later, drop the "flies"
 */
class movetype
{
public:
	/**
	 * A const-only interface for how many (movement, vision, or "jamming") points a
	 * unit needs for each hex. Functions to modify the costs are exposed by the
	 * movetype instance owning this terrain_costs, so that changes to movement will
	 * cascade to the vision, etc.
	 */
	class terrain_costs
	{
	public:
		virtual ~terrain_costs() = default;

		/**
		 * Returns the value associated with the given terrain.
		 *
		 * Calculated values are cached for later queries.
		 */
		virtual int value(const t_translation::terrain_code & terrain) const = 0;

		/**
		 * Returns the cost associated with the given terrain.
		 * Costs are doubled when @a slowed is true.
		 */
		int cost(const t_translation::terrain_code & terrain, bool slowed=false) const
		{
			int result = value(terrain);
			return  slowed  &&  result != movetype::UNREACHABLE ? 2 * result : result;
		}

		/**
		 * Does a sufficiently deep copy so that the returned object's lifespan
		 * is independent of other objects' lifespan. Never returns nullptr.
		 */
		virtual std::unique_ptr<terrain_costs> make_standalone() const = 0;

		/** Writes our data to a config. */
		virtual void write(config & cfg, const std::string & child_name="", bool merged=true) const = 0;
	};

	/** Reverse of terrain_costs::write. Never returns nullptr. */
	static std::unique_ptr<terrain_costs> read_terrain_costs(const config & cfg);

	// Forward declaration so that terrain_info can friend the
	// swap(terrain_defense, terrain_defense) function
	class terrain_defense;

private:

	/**
	 * Stores a set of data based on terrain, in some cases with raw pointers to
	 * other instances of terrain_info (the fallback_).
	 *
	 * The data can either be a single instance (in which case it's
	 * writable and stored in unique_data_) or may have already been shared
	 * (via make_data_shareable()), in which case it's stored in shared_data_.
	 * There will always be exactly one of those two that's non-null,
	 * get_data() returns it from where-ever it is.
	 */
	class terrain_info : public terrain_costs
	{
		/** The terrain-based data. */
		class data;
		// The data class is not defined here to keep the header file cleaner.

	public:
		/** The parameters used when calculating a terrain-based value. */
		struct parameters;

		explicit terrain_info(const parameters & params,
		                      const terrain_info * fallback);
		terrain_info(const config & cfg, const parameters & params,
		             const terrain_info * fallback);
		~terrain_info() override;

		// Instead of the standard copy and move constructors, there are ones
		// that copy the data but require the caller to specify the fallback.
		terrain_info(const terrain_info & that) = delete;
		terrain_info(terrain_info && that) = delete;
		explicit terrain_info(terrain_info && that,
		                      const terrain_info * fallback);
		terrain_info(const terrain_info & that,
		             const terrain_info * fallback);

		// Similarly to the copy and move constructors, the default assignments
		// are deleted, because the caller needs to know about the siblings.
		terrain_info & operator=(const terrain_info & that) = delete;
		terrain_info & operator=(terrain_info && that) = delete;
		void copy_data(const movetype::terrain_info & that);
		void swap_data(movetype::terrain_info & that);

		/** Returns whether or not our data is empty. */
		bool empty() const;
		/** Merges the given config over the existing values. */
		void merge(const config & new_values, bool overwrite,
			const std::vector<movetype::terrain_info *> & dependants);

		// Implementation of terrain_costs
		int value(const t_translation::terrain_code & terrain) const override;
		void write(config & cfg, const std::string & child_name="", bool merged=true) const override;
		std::unique_ptr<terrain_costs> make_standalone() const override;

	private:
		/**
		 * Move data to an immutable copy in shared_data_, no-op if the data
		 * is already in shared_data_.
		 */
		void make_data_shareable() const;
		/**
		 * Copy the immutable data back to unique_data_, no-op if the data
		 * is already in unique_data_.
		 */
		void make_data_writable() const;
		/**
		 * Returns either *unique_data_ or *shared_data_, choosing the one that
		 * currently holds the data.
		 */
		const data & get_data() const;

	private:
		std::unique_ptr<data> unique_data_;
		std::shared_ptr<const data> shared_data_;
		const terrain_info * const fallback_;
	};


public:
	/**
	 * Magic value that signifies a hex is unreachable.
	 * The UNREACHABLE macro in the data tree should match this value.
	 */
	static const int UNREACHABLE = 99;

	/** Stores a set of defense levels. */
	class terrain_defense
	{
		static const terrain_info::parameters params_min_;
		static const terrain_info::parameters params_max_;

	public:
		terrain_defense() : min_(params_min_, nullptr), max_(params_max_, nullptr) {}
		explicit terrain_defense(const config & cfg) :
			min_(cfg, params_min_, nullptr), max_(cfg, params_max_, nullptr)
		{}
		terrain_defense(const terrain_defense & that);
		terrain_defense(terrain_defense && that);
		terrain_defense & operator=(const terrain_defense & that);
		terrain_defense & operator=(terrain_defense && that);

		/** Returns the defense associated with the given terrain. */
		int defense(const t_translation::terrain_code & terrain) const
		{ return std::max(min_.value(terrain), max_.value(terrain)); }
		/** Returns whether there is a defense cap associated to this terrain. */
		bool capped(const t_translation::terrain_code & terrain) const
		{ return min_.value(terrain) != 0; }
		/**
		 * Merges the given config over the existing costs.
		 * (Not overwriting implies adding.)
		 */
		void merge(const config & new_data, bool overwrite);
		/**
		 * Writes our data to a config, as a child if @a child_name is specified.
		 * (No child is created if there is no data.)
		 */
		void write(config & cfg, const std::string & child_name="") const
		{ max_.write(cfg, child_name, false); }

		friend void swap(movetype::terrain_defense & a, movetype::terrain_defense & b);

	private:
		// There will be duplication of the config here, but it is a small
		// config, and the duplication allows greater code sharing.
		terrain_info min_;
		terrain_info max_;
	};

	/** Stores a set of resistances. */
	class resistances
	{
	public:
		resistances() : cfg_() {}
		explicit resistances(const config & cfg) : cfg_(cfg) {}

		/** Returns a map from attack types to resistances. */
		utils::string_map_res damage_table() const;
		/** Returns the resistance against the indicated attack. */
		int resistance_against(const attack_type & attack) const;
		/** Returns the resistance against the indicated damage type. */
		int resistance_against(const std::string & damage_type) const;
		/** Merges the given config over the existing costs. */
		void merge(const config & new_data, bool overwrite);
		/** Writes our data to a config, as a child if @a child_name is specified. */
		void write(config & out_cfg, const std::string & child_name="") const;

	private:
		config cfg_;
	};

private:
	static const terrain_info::parameters mvj_params_;

public:
	movetype();
	explicit movetype(const config & cfg);
	movetype(const movetype & that);
	movetype(movetype && that);
	movetype &operator=(const movetype & that);
	movetype &operator=(movetype && that);
	// The default destructor is sufficient, despite the Rule of Five.
	// The copy and assignment functions handle the pointers between
	// terrain_cost_impl instances, but all of these instances are owned
	// by this instance of movetype.
	~movetype() = default;

	friend void swap(movetype & a, movetype & b);
	friend void swap(movetype::terrain_info & a, movetype::terrain_info & b);

	// This class is basically just a holder for its various pieces, so
	// provide access to those pieces on demand. There's no non-const
	// getters for terrain_costs, as that's now an interface with only
	// const functions in it, and because the logic for how the cascade and
	// fallback mechanism works would be easier to handle in movetype itself.
	terrain_defense & get_defense()  { return defense_; }
	resistances & get_resistances()  { return resist_; }
	// And const access:
	const terrain_costs & get_movement()  const { return movement_; }
	const terrain_costs & get_vision()    const { return vision_; }
	const terrain_costs & get_jamming()   const { return jamming_; }
	const terrain_defense & get_defense() const { return defense_; }
	const resistances & get_resistances() const { return resist_; }

	/** Returns whether or not *this is flagged as a flying movement type. */
	bool is_flying() const { return flying_; }
	/** Sets whether or not *this is flagged as a flying movement type. */
	void set_flying(bool flies=true) { flying_ = flies; }

	/** Returns the cost to move through the indicated terrain. */
	int movement_cost(const t_translation::terrain_code & terrain, bool slowed=false) const
	{ return movement_.cost(terrain, slowed); }
	/** Returns the cost to see through the indicated terrain. */
	int vision_cost(const t_translation::terrain_code & terrain, bool slowed=false) const
	{ return vision_.cost(terrain, slowed); }
	/** Returns the cost to "jam" through the indicated terrain. */
	int jamming_cost(const t_translation::terrain_code & terrain, bool slowed=false) const
	{ return jamming_.cost(terrain, slowed); }

	/** Returns the defensive value of the indicated terrain. */
	int defense_modifier(const t_translation::terrain_code & terrain) const
	{ return defense_.defense(terrain); }

	/** Returns the resistance against the indicated attack. */
	int resistance_against(const attack_type & attack) const
	{ return resist_.resistance_against(attack); }
	/** Returns the resistance against the indicated damage type. */
	int resistance_against(const std::string & damage_type) const
	{ return resist_.resistance_against(damage_type); }
	/** Returns a map from attack types to resistances. */
	utils::string_map_res damage_table() const
	{ return resist_.damage_table(); }

	/** Returns whether or not there are any terrain caps with respect to a set of terrains. */
	bool has_terrain_defense_caps(const std::set<t_translation::terrain_code> & ts) const;
	/** Returns whether or not there are any vision-specific costs. */
	bool has_vision_data()  const { return !vision_.empty(); }
	/** Returns whether or not there are any jamming-specific costs. */
	bool has_jamming_data() const { return !jamming_.empty(); }

	/**
	 * Merges the given config over the existing data, the config should have zero or more
	 * children named "movement_costs", "defense", etc. Only those children will be affected
	 * (in the case of movement and vision the cascaded values in vision and jamming will also
	 * be affected).
	 *
	 * If @a overwrite is true, the new values will replace the old ones. If it's false, the
	 * new values are relative improvements or maluses which will be applied on top of the old
	 * values.
	 *
	 * If the old values included defense caps and @a overwrite is false, the calculations are
	 * done with absolute values and then changed back to the old sign. This means that merge()
	 * doesn't create or remove defense caps when @a overwrite is false.
	 *
	 * If @a new_cfg["flying"] is provided, it overrides the old value, regardless of the value
	 * of @a overwrite.
	 *
	 * This neither adds nor removes special notes. One purpose of this function is to have
	 * [unit_type][movement_costs] partially overwrite data from [unit_type]movetype=, and it
	 * would be unhelpful if an unrelated [unit_type][special_note] cleared the movetype's
	 * special notes.
	 */
	void merge(const config & new_cfg, bool overwrite=true);

	/**
	 * Merges the given config over the existing data; this 3-argument version affects only the
	 * subelement identified by the @a applies_to argument.
	 *
	 * @param applies_to which type of movement to change ("movement_costs", etc)
	 * @param new_cfg data which could be one of the children of the config for the two-argument form of this function.
	 * @param overwrite if false, the new values will be added to the old.
	 */
	void merge(const config & new_cfg, const std::string & applies_to, bool overwrite=true);

	/** The set of applicable effects for movement types */
	static const std::set<std::string> effects;

	/** Contents of any [special_note] tags */
	const std::vector<t_string>& special_notes() const { return special_notes_; }

	/**
	 * Writes the movement type data to the provided config.
	 *
	 * There is no default value for the include_notes argument. Given the implied contract that a
	 * class with a constructor(const config&) and a write(config&) supports round-tripping the
	 * data, the default would need to be true. However, this method has only two callers, and
	 * neither of them want to include the notes:
	 *
	 * Movetype patching is unaffected by the notes, as they will be ignored by movetype::merge().
	 *
	 * [store_unit] is broken by the notes, because they end up in unit::special_notes_ instead of
	 * movetype::special_notes_ after the subsequent [unstore_unit].
	 *
	 * @param cfg output
	 * @param include_notes if false, omits any special notes
	 */
	void write(config& cfg, bool include_notes) const;

private:
	terrain_info movement_;
	terrain_info vision_;
	terrain_info jamming_;
	terrain_defense defense_;
	resistances resist_;

	bool flying_;
	std::vector<t_string> special_notes_;
};
