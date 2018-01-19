/*
   Copyright (C) 2014 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Handle movement types.
 */

#include "movetype.hpp"

#include "game_board.hpp"
#include "game_config_manager.hpp"
#include "log.hpp"
#include "map/map.hpp"
#include "terrain/translation.hpp"
#include "terrain/type_data.hpp"
#include "units/types.hpp" // for attack_type

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)


/* *** parameters *** */


namespace { // Some functions for use with parameters::eval.

	/// Converts config defense values to a "max" value.
	int config_to_max(int value)
	{
		return value < 0 ? -value : value;
	}

	/// Converts config defense values to a "min" value.
	int config_to_min(int value)
	{
		return value < 0 ? -value : 0;
	}
}


/// The parameters used when calculating a terrain-based value.
struct movetype::terrain_info::parameters
{
	int min_value;     /// The smallest allowable value.
	int max_value;     /// The largest allowable value.
	int default_value; /// The default value (if no data is available).

	int (*eval)(int);  /// Converter for values taken from a config. May be nullptr.

	bool use_move;     /// Whether to look at underlying movement or defense terrains.
	bool high_is_good; /// Whether we are looking for highest or lowest (unless inverted by the underlying terrain).

	parameters(int min, int max, int (*eval_fun)(int)=nullptr, bool move=true, bool high=false) :
		min_value(min), max_value(max), default_value(high ? min : max),
		eval(eval_fun), use_move(move), high_is_good(high)
	{}
};


const movetype::terrain_info::parameters
	movetype::terrain_costs::params_(1, movetype::UNREACHABLE);

const movetype::terrain_info::parameters
	movetype::terrain_defense::params_min_(0, 100, config_to_min, false, true);
const movetype::terrain_info::parameters
	movetype::terrain_defense::params_max_(0, 100, config_to_max, false, false);


/* *** data *** */


class movetype::terrain_info::data
{
public:
	/// Constructor.
	/// @a params must be long-lived (typically a static variable).
	explicit data(const parameters & params) :
		cfg_(), cache_(), params_(params)
	{}
	/// Constructor.
	/// @a params must be long-lived (typically a static variable).
	data(const config & cfg, const parameters & params) :
		cfg_(cfg), cache_(), params_(params)
	{}

	// The copy constructor does not bother copying the cache since
	// typically the cache will be cleared shortly after the copy.
	data(const data & that) :
		cfg_(that.cfg_), cache_(), params_(that.params_)
	{}

	/// Clears the cached data (presumably our fallback has changed).
	void clear_cache(const terrain_info * cascade) const;
	/// Tests if merging @a new_values would result in changes.
	bool config_has_changes(const config & new_values, bool overwrite) const;
	/// Tests for no data in this object.
	bool empty() const { return cfg_.empty(); }
	/// Merges the given config over the existing costs.
	void merge(const config & new_values, bool overwrite,
	           const terrain_info * cascade);
	/// Read-only access to our parameters.
	const parameters & params() const { return params_; }
	/// Returns the value associated with the given terrain.
	int value(const t_translation::terrain_code & terrain,
	          const terrain_info * fallback) const
	{ return value(terrain, fallback, 0); }
	/// If there is data, writes it to the config.
	void write(config & out_cfg, const std::string & child_name) const;
	/// If there is (merged) data, writes it to the config.
	void write(config & out_cfg, const std::string & child_name,
	           const terrain_info * fallback) const;

private:
	/// Calculates the value associated with the given terrain.
	int calc_value(const t_translation::terrain_code & terrain,
	               const terrain_info * fallback, unsigned recurse_count) const;
	/// Returns the value associated with the given terrain (possibly cached).
	int value(const t_translation::terrain_code & terrain,
	          const terrain_info * fallback, unsigned recurse_count) const;

private:
	typedef std::map<t_translation::terrain_code, int> cache_t;

	/// Config describing the terrain values.
	config cfg_;
	/// Cache of values based on the config.
	mutable cache_t cache_;
	/// Various parameters used when calculating values.
	const parameters & params_;
};


/**
 * Clears the cached data (presumably our fallback has changed).
 * @param[in] cascade  Cache clearing will be cascaded into this terrain_info.
 */
void movetype::terrain_info::data::clear_cache(const terrain_info * cascade) const
{
	cache_.clear();
	// Cascade the clear to whichever terrain_info falls back on us.
	if ( cascade )
		cascade->clear_cache();
}


/**
 * Tests if merging @a new_values would result in changes.
 * This allows the shared data to actually work, as otherwise each unit created
 * via WML (including unstored units) would "overwrite" its movement data with
 * a usually identical copy and thus break the sharing.
 */
bool movetype::terrain_info::data::config_has_changes(const config & new_values,
                                                      bool overwrite) const
{
	if ( overwrite ) {
		for (const config::attribute & a : new_values.attribute_range())
			if ( a.second != cfg_[a.first] )
				return true;
	}
	else {
		for (const config::attribute & a : new_values.attribute_range())
			if ( a.second.to_int() != 0 )
				return true;
	}

	// If we make it here, new_values has no changes for us.
	return false;
}


/**
 * Merges the given config over the existing costs.
 * @param[in] new_values  The new values.
 * @param[in] overwrite   If true, the new values overwrite the old.
 *                        If false, the new values are added to the old.
 * @param[in] cascade     Cache clearing will be cascaded into this terrain_info.
 */
void movetype::terrain_info::data::merge(const config & new_values, bool overwrite,
                                         const terrain_info * cascade)
{
	if ( overwrite )
		// We do not support child tags here, so do not copy any that might
		// be in the input. (If in the future we need to support child tags,
		// change "merge_attributes" to "merge_with".)
		cfg_.merge_attributes(new_values);
	else {
		for (const config::attribute & a : new_values.attribute_range()) {
			config::attribute_value & dest = cfg_[a.first];
			int old = dest.to_int(params_.max_value);

			// The new value is the absolute value of the old plus the
			// provided value, capped between minimum and maximum, then
			// given the sign of the old value.
			// (Think defenses for why we might have negative values.)
			int value = std::abs(old) + a.second.to_int(0);
			value = std::max(params_.min_value, std::min(value, params_.max_value));
			if ( old < 0 )
				value = -value;

			dest = value;
		}
	}

	// The new data has invalidated the cache.
	clear_cache(cascade);
}


/**
 * If there is data, writes it to a config.
 * @param[out] out_cfg     The config that will receive the data.
 * @param[in]  child_name  If not empty, create and write to a child config with this tag.
 *                         This child will *not* be created if there is no data to write.
 */
void movetype::terrain_info::data::write(
	config & out_cfg, const std::string & child_name) const
{
	if ( cfg_.empty() )
		return;

	if ( child_name.empty() )
		out_cfg.merge_with(cfg_);
	else
		out_cfg.add_child(child_name, cfg_);
}


/**
 * Writes merged data to a config.
 * @param[out] out_cfg     The config that will receive the data.
 * @param[in]  child_name  If not empty, create and write to a child config with this tag.
 *                         This *will* be created even if there is no data to write.
 * @param[in]  fallback    If not nullptr, its data will be merged with ours for the write.
 */
void movetype::terrain_info::data::write(
	config & out_cfg, const std::string & child_name, const terrain_info * fallback) const
{
	// Get a place to write to.
	config & merged = child_name.empty() ? out_cfg : out_cfg.add_child(child_name);

	if ( fallback )
		fallback->write(merged, "", true);
	merged.merge_with(cfg_);
}


/**
 * Calculates the value associated with the given terrain.
 * This is separate from value() to separate the calculating of the
 * value from the caching of it.
 * @param[in]  terrain        The terrain whose value is requested.
 * @param[in]  fallback       Consulted if we are missing data.
 * @param[in]  recurse_count  Detects (probable) infinite recursion.
 */
int movetype::terrain_info::data::calc_value(
	const t_translation::terrain_code & terrain,
	const terrain_info * fallback,
	unsigned recurse_count) const
{
	// Infinite recursion detection:
	if ( recurse_count > 100 ) {
		ERR_CF << "infinite terrain_info recursion on "
		       << (params_.use_move ? "movement" : "defense") << ": "
			   << t_translation::write_terrain_code(terrain)
			   << " depth " << recurse_count << '\n';
		return params_.default_value;
	}

	ter_data_cache tdata;
	if (game_config_manager::get()){
		tdata = game_config_manager::get()->terrain_types(); //This permits to get terrain info in unit help pages from the help in title screen, even if there is no residual gamemap object
	}
	assert(tdata);

	// Get a list of underlying terrains.
	const t_translation::ter_list & underlying = params_.use_move ?
			tdata->underlying_mvt_terrain(terrain) :
			tdata->underlying_def_terrain(terrain);
	assert(!underlying.empty());


	if ( underlying.size() == 1  &&  underlying.front() == terrain )
	{
		// This is not an alias; get the value directly.
		int result = params_.default_value;

		const std::string & id = tdata->get_terrain_info(terrain).id();
		if (const config::attribute_value *val = cfg_.get(id)) {
			// Read the value from our config.
			result = val->to_int(params_.default_value);
			if ( params_.eval != nullptr )
				result = params_.eval(result);
		}
		else if ( fallback != nullptr ) {
			// Get the value from our fallback.
			result = fallback->value(terrain);
		}

		// Validate the value.
		if ( result < params_.min_value ) {
			WRN_CF << "Terrain '" << terrain << "' has evaluated to " << result
				   << " (" << (params_.use_move ? "cost" : "defense")
			       << "), which is less than " << params_.min_value
			       << "; resetting to " << params_.min_value << ".\n";
			result = params_.min_value;
		}
		if ( result > params_.max_value ) {
			WRN_CF << "Terrain '" << terrain << "' has evaluated to " << result
				   << " (" << (params_.use_move ? "cost" : "defense")
				   << "), which is more than " << params_.max_value
			       << "; resetting to " << params_.max_value << ".\n";
			result = params_.max_value;
		}

		return result;
	}
	else
	{
		// This is an alias; select the best of all underlying terrains.
		bool prefer_high = params_.high_is_good;
		int result = params_.default_value;
		if ( underlying.front() == t_translation::MINUS )
			// Use the other value as the initial value.
			result =  result == params_.max_value ? params_.min_value :
			                                        params_.max_value;

		// Loop through all underlying terrains.
		t_translation::ter_list::const_iterator i;
		for ( i = underlying.begin(); i != underlying.end(); ++i )
		{
			if ( *i == t_translation::PLUS ) {
				// Prefer what is good.
				prefer_high = params_.high_is_good;
			}
			else if ( *i == t_translation::MINUS ) {
				// Prefer what is bad.
				prefer_high = !params_.high_is_good;
			}
			else {
				// Test the underlying terrain's value against the best so far.
				const int num = value(*i, fallback, recurse_count + 1);

				if ( ( prefer_high  &&  num > result)  ||
					 (!prefer_high  &&  num < result) )
					result = num;
			}
		}

		return result;
	}
}


/**
 * Returns the value associated with the given terrain (possibly cached).
 * @param[in]  terrain        The terrain whose value is requested.
 * @param[in]  fallback       Consulted if we are missing data.
 * @param[in]  recurse_count  Detects (probable) infinite recursion.
 */
int movetype::terrain_info::data::value(
	const t_translation::terrain_code & terrain,
	const terrain_info * fallback,
	unsigned recurse_count) const
{
	// Check the cache.
	std::pair<cache_t::iterator, bool> cache_it =
		cache_.emplace(terrain, -127); // Bogus value that should never be seen.
	if ( cache_it.second )
		// The cache did not have an entry for this terrain, so calculate the value.
		cache_it.first->second = calc_value(terrain, fallback, recurse_count);

	return cache_it.first->second;
}


/* *** terrain_info *** */


/**
 * Constructor.
 * @param[in] params    The parameters to use when calculating values.
 *                      This is stored as a reference, so it must be long-lived (typically a static variable).
 * @param[in] fallback  Used as a backup in case we are asked for data we do not have (think vision costs falling back to movement costs).
 * @param[in] cascade   A terrain_info that uses us as a fallback. (Needed to sync cache clearing.)
 * @note The fallback/cascade mechanism is a bit fragile and really should only
 *       be used by movetype.
 */
movetype::terrain_info::terrain_info(const parameters & params,
                                     const terrain_info * fallback,
                                     const terrain_info * cascade) :
	data_(new data(params)),
	merged_data_(),
	fallback_(fallback),
	cascade_(cascade)
{
}


/**
 * Constructor.
 * @param[in] cfg       An initial data set.
 * @param[in] params    The parameters to use when calculating values.
 *                      This is stored as a reference, so it must be long-lived (typically a static variable).
 * @param[in] fallback  Used as a backup in case we are asked for data we do not have (think vision costs falling back to movement costs).
 * @param[in] cascade   A terrain_info that uses us as a fallback. (Needed to sync cache clearing.)
 * @note The fallback/cascade mechanism is a bit fragile and really should only
 *       be used by movetype.
 */
movetype::terrain_info::terrain_info(const config & cfg, const parameters & params,
                                     const terrain_info * fallback,
                                     const terrain_info * cascade) :
	data_(new data(cfg, params)),
	merged_data_(),
	fallback_(fallback),
	cascade_(cascade)
{
}


/**
 * Copy constructor.
 * @param[in] that      The terran_info to copy.
 * @param[in] fallback  Used as a backup in case we are asked for data we do not have (think vision costs falling back to movement costs).
 * @param[in] cascade   A terrain_info that uses us as a fallback. (Needed to sync cache clearing.)
 * @note The fallback/cascade mechanism is a bit fragile and really should only
 *       be used by movetype.
 */
movetype::terrain_info::terrain_info(const terrain_info & that,
                                     const terrain_info * fallback,
                                     const terrain_info * cascade) :
	// If we do not have a fallback, we need to incorporate that's fallback.
	// (See also the assignment operator.)
	data_(fallback ? that.data_ : that.get_merged()),
	merged_data_(that.merged_data_),
	fallback_(fallback),
	cascade_(cascade)
{
}


/**
 * Destructor
 */
movetype::terrain_info::~terrain_info()
{
	// While this appears to be simply the default destructor, it needs
	// to be defined in this file so that it knows about ~data(), which
	// is called from the smart pointers' destructor.
}


/**
 * Assignment operator.
 */
movetype::terrain_info & movetype::terrain_info::operator=(const terrain_info & that)
{
	if ( this != &that ) {
		// If we do not have a fallback, we need to incorporate that's fallback.
		// (See also the copy constructor.)
		data_ = fallback_ ? that.data_ : that.get_merged();
		merged_data_ = that.merged_data_;
		// We do not change our fallback nor our cascade.
	}

	return *this;
}


/**
 * Clears the cache of values.
 */
void movetype::terrain_info::clear_cache() const
{
	merged_data_.reset();
	data_->clear_cache(cascade_);
}


/**
 * Returns whether or not our data is empty.
 */
bool movetype::terrain_info::empty() const
{
	return data_->empty();
}


/**
 * Merges the given config over the existing values.
 * @param[in] new_values  The new values.
 * @param[in] overwrite   If true, the new values overwrite the old.
 *                        If false, the new values are added to the old.
 */
void movetype::terrain_info::merge(const config & new_values, bool overwrite)
{
	if ( !data_->config_has_changes(new_values, overwrite) )
		// Nothing will change, so skip the copy-on-write.
		return;

	// Reset merged_data_ before seeing if data_ is unique, since the two might
	// point to the same thing.
	merged_data_.reset();

	// Copy-on-write.
	if ( !data_.unique() ) {
		data_.reset(new data(*data_));
		// We also need to make copies of our fallback and cascade.
		// This is to keep the caching manageable, as this means each
		// individual movetype will either share *all* of its cost data
		// or not share *all* of its cost data. In particular, we avoid:
		// 1) many sets of (unshared) vision costs whose cache would need
		//    to be cleared when a shared set of movement costs changes;
		// 2) a caching nightmare when shared vision costs fallback to
		//    unshared movement costs.
		if ( fallback_ )
			fallback_->make_unique_fallback();
		if ( cascade_ )
			cascade_->make_unique_cascade();
	}

	data_->merge(new_values, overwrite, cascade_);
}


/**
 * Returns the value associated with the given terrain.
 */
int movetype::terrain_info::value(const t_translation::terrain_code & terrain) const
{
	return data_->value(terrain, fallback_);
}


/**
 * Writes our data to a config.
 * @param[out] cfg         The config that will receive the data.
 * @param[in]  child_name  If not empty, create and write to a child config with this tag.
 * @param[in]  merged      If true, our data will be merged with our fallback's, and it is possible an empty child will be created.
 *                         If false, data will not be merged, and an empty child will not be created.
 */
void movetype::terrain_info::write(config & cfg, const std::string & child_name,
                                   bool merged) const
{
	if ( !merged )
		data_->write(cfg, child_name);
	else
		data_->write(cfg, child_name, fallback_);
}


/**
 * Returns a pointer to data the incorporates our fallback.
 */
const std::shared_ptr<movetype::terrain_info::data> &
	movetype::terrain_info::get_merged() const
{
	// Create-on-demand.
	if ( !merged_data_ )
	{
		if ( !fallback_ )
			// Nothing to incorporate.
			merged_data_ = data_;

		else if ( data_->empty() )
			// Pure fallback.
			merged_data_ = fallback_->get_merged();

		else {
			// Need to merge data.
			config merged;
			write(merged, "", true);
			merged_data_ = std::make_shared<data>(merged, data_->params());
		}
	}
	return merged_data_;
}


/**
 * Ensures our data is not shared, and propagates to our cascade.
 */
void movetype::terrain_info::make_unique_cascade() const
{
	if ( !data_.unique() )
		// Const hack because this is not really changing the data.
		const_cast<terrain_info *>(this)->data_.reset(new data(*data_));

	if ( cascade_ )
		cascade_->make_unique_cascade();
}


/**
 * Ensures our data is not shared, and propagates to our fallback.
 */
void movetype::terrain_info::make_unique_fallback() const
{
	if ( !data_.unique() )
		// Const hack because this is not really changing the data.
		const_cast<terrain_info *>(this)->data_.reset(new data(*data_));

	if ( fallback_ )
		fallback_->make_unique_fallback();
}


/* *** resistances *** */


/**
 * Returns a map from attack types to resistances.
 */
utils::string_map movetype::resistances::damage_table() const
{
	utils::string_map result;

	for (const config::attribute & attrb : cfg_.attribute_range()) {
		result[attrb.first] = attrb.second;
	}

	return result;
}


/**
 * Returns the resistance against the indicated attack.
 */
int movetype::resistances::resistance_against(const attack_type & attack) const
{
	return cfg_[attack.type()].to_int(100);
}


/**
 * Returns the resistance against the indicated damage type.
 */
int movetype::resistances::resistance_against(const std::string & damage_type) const
{
	return cfg_[damage_type].to_int(100);
}


/**
 * Merges the given config over the existing costs.
 * If @a overwrite is false, the new values will be added to the old.
 */
void movetype::resistances::merge(const config & new_data, bool overwrite)
{
	if ( overwrite )
		// We do not support child tags here, so do not copy any that might
		// be in the input. (If in the future we need to support child tags,
		// change "merge_attributes" to "merge_with".)
		cfg_.merge_attributes(new_data);
	else
		for (const config::attribute & a : new_data.attribute_range()) {
			config::attribute_value & dest = cfg_[a.first];
			dest = std::max(0, dest.to_int(100) + a.second.to_int(0));
		}
}


/**
 * Writes our data to a config, as a child if @a child_name is specified.
 * (No child is created if there is no data.)
 */
void movetype::resistances::write(config & out_cfg, const std::string & child_name) const
{
	if ( cfg_.empty() )
		return;

	if ( child_name.empty() )
		out_cfg.merge_with(cfg_);
	else
		out_cfg.add_child(child_name, cfg_);
}


/* *** movetype *** */


/**
 * Default constructor
 */
movetype::movetype() :
	movement_(nullptr, &vision_),      // This is not access before initialization; the address is merely stored at this point.
	vision_(&movement_, &jamming_), // This is not access before initialization; the address is merely stored at this point.
	jamming_(&vision_, nullptr),
	defense_(),
	resist_(),
	flying_(false)
{
}


/**
 * Constructor from a config
 */
movetype::movetype(const config & cfg) :
	movement_(cfg.child_or_empty("movement_costs"), nullptr, &vision_),    // This is not access before initialization; the address is merely stored at this point.
	vision_(cfg.child_or_empty("vision_costs"), &movement_, &jamming_), // This is not access before initialization; the address is merely stored at this point.
	jamming_(cfg.child_or_empty("jamming_costs"), &vision_, nullptr),
	defense_(cfg.child_or_empty("defense")),
	resist_(cfg.child_or_empty("resistance")),
	flying_(cfg["flies"].to_bool(false))
{
}


/**
 * Copy constructor
 */
movetype::movetype(const movetype & that) :
	movement_(that.movement_, nullptr, &vision_),    // This is not access before initialization; the address is merely stored at this point.
	vision_(that.vision_, &movement_, &jamming_), // This is not access before initialization; the address is merely stored at this point.
	jamming_(that.jamming_, &vision_, nullptr),
	defense_(that.defense_),
	resist_(that.resist_),
	flying_(that.flying_)
{
}

/**
 * Checks if we have a defense cap (nontrivial min value) for any of the given terrain types.
 */
bool movetype::has_terrain_defense_caps(const std::set<t_translation::terrain_code> & ts) const {
	for (const t_translation::terrain_code & t : ts) {
		if (defense_.capped(t))
			return true;
	}
	return false;
}

/**
 * Merges the given config over the existing data.
 * If @a overwrite is false, the new values will be added to the old.
 */
void movetype::merge(const config & new_cfg, bool overwrite)
{
	for (const config & child : new_cfg.child_range("movement_costs")) {
		movement_.merge(child, overwrite);
	}

	for (const config & child : new_cfg.child_range("vision_costs")) {
		vision_.merge(child, overwrite);
	}

	for (const config & child : new_cfg.child_range("jamming_costs")) {
		jamming_.merge(child, overwrite);
	}

	for (const config & child : new_cfg.child_range("defense")) {
		defense_.merge(child, overwrite);
	}

	for (const config & child : new_cfg.child_range("resistance")) {
		resist_.merge(child, overwrite);
	}

	// "flies" is used when WML defines a movetype.
	// "flying" is used when WML defines a unit.
	// It's easier to support both than to track which case we are in.
	flying_ = new_cfg["flies"].to_bool(flying_);
	flying_ = new_cfg["flying"].to_bool(flying_);
}

/**
 * The set of strings defining effects which apply to movetypes.
 */
const std::set<std::string> movetype::effects {"movement_costs",
	"vision_costs", "jamming_costs", "defense", "resistance"};

/**
 * Writes the movement type data to the provided config.
 */
void movetype::write(config & cfg) const
{
	movement_.write(cfg, "movement_costs", false);
	vision_.write(cfg, "vision_costs", false);
	jamming_.write(cfg, "jamming_costs", false);
	defense_.write(cfg, "defense");
	resist_.write(cfg, "resistance");

	if ( flying_ )
		cfg["flying"] = true;
}

