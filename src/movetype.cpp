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

/**
 *  @file
 *  Handle movement types.
 */

#include "movetype.hpp"

#include "game_config_manager.hpp"
#include "log.hpp"
#include "terrain/translation.hpp"

static lg::log_domain log_config("config");
#define ERR_CF LOG_STREAM(err, log_config)
#define WRN_CF LOG_STREAM(warn, log_config)


/* *** parameters *** */


namespace { // Some functions for use with parameters::eval.

	/** Converts config defense values to a "max" value. */
	int config_to_max(int value)
	{
		return value < 0 ? -value : value;
	}

	/** Converts config defense values to a "min" value. */
	int config_to_min(int value)
	{
		return value < 0 ? -value : 0;
	}
}


/** The parameters used when calculating a terrain-based value. */
struct movetype::terrain_info::parameters
{
	/** The smallest allowable value. */
	int min_value;
	/** The largest allowable value. */
	int max_value;
	/** The default value (if no data is available). */
	int default_value;

	/** Converter for values taken from a config. May be nullptr. */
	int (*eval)(int);

	/** Whether to look at underlying movement or defense terrains. */
	bool use_move;
	/** Whether we are looking for highest or lowest (unless inverted by the underlying terrain). */
	bool high_is_good;

	parameters(int min, int max, int (*eval_fun)(int)=nullptr, bool move=true, bool high=false) :
		min_value(min), max_value(max), default_value(high ? min : max),
		eval(eval_fun), use_move(move), high_is_good(high)
	{}
};


/** Limits for movement, vision and jamming */
const movetype::terrain_info::parameters
	movetype::mvj_params_{1, movetype::UNREACHABLE};

const movetype::terrain_info::parameters
	movetype::terrain_defense::params_min_(0, 100, config_to_min, false, true);
const movetype::terrain_info::parameters
	movetype::terrain_defense::params_max_(0, 100, config_to_max, false, false);


/* *** data *** */


class movetype::terrain_info::data
{
public:
	/**
	 * Constructor.
	 * @a params must be long-lived (typically a static variable).
	 */
	explicit data(const parameters & params) :
		cfg_(), cache_(), params_(params)
	{}
	/**
	 * Constructor.
	 * @a params must be long-lived (typically a static variable).
	 */
	data(const config & cfg, const parameters & params) :
		cfg_(cfg), cache_(), params_(params)
	{}

	// The copy constructor does not bother copying the cache since
	// typically the cache will be cleared shortly after the copy.
	data(const data & that) :
		cfg_(that.cfg_), cache_(), params_(that.params_)
	{}

	/** Clears the cached data (presumably our fallback has changed). */
	void clear_cache() const;
	/** Tests if merging @a new_values would result in changes. */
	bool config_has_changes(const config & new_values, bool overwrite) const;
	/** Tests for no data in this object. */
	bool empty() const { return cfg_.empty(); }
	/** Merges the given config over the existing costs. */
	void merge(const config & new_values, bool overwrite);
	/** Read-only access to our parameters. */
	const parameters & params() const { return params_; }
	/** Returns the value associated with the given terrain. */
	int value(const t_translation::terrain_code & terrain,
	          const terrain_info * fallback) const
	{ return value(terrain, fallback, 0); }
	/** If there is data, writes it to the config. */
	void write(config & out_cfg, const std::string & child_name) const;
	/** If there is (merged) data, writes it to the config. */
	void write(config & out_cfg, const std::string & child_name,
	           const terrain_info * fallback) const;

private:
	/** Calculates the value associated with the given terrain. */
	int calc_value(const t_translation::terrain_code & terrain,
	               const terrain_info * fallback, unsigned recurse_count) const;
	/** Returns the value associated with the given terrain (possibly cached). */
	int value(const t_translation::terrain_code & terrain,
	          const terrain_info * fallback, unsigned recurse_count) const;

private:
	typedef std::map<t_translation::terrain_code, int> cache_t;

	/** Config describing the terrain values. */
	config cfg_;
	/** Cache of values based on the config. */
	mutable cache_t cache_;
	/** Various parameters used when calculating values. */
	const parameters & params_;
};


/**
 * Clears the cached data (presumably our fallback has changed).
 */
void movetype::terrain_info::data::clear_cache() const
{
	cache_.clear();
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
		for (const auto& [key, value] : new_values.attribute_range())
			if ( value != cfg_[key] )
				return true;
	}
	else {
		for(const auto& [_, value] : new_values.attribute_range())
			if ( value.to_int() != 0 )
				return true;
	}

	// If we make it here, new_values has no changes for us.
	return false;
}


/**
 * Merges the given config over the existing costs.
 *
 * After calling this function, the caller must call clear_cache on any
 * terrain_info that uses this one as a fallback.
 *
 * @param[in] new_values  The new values.
 * @param[in] overwrite   If true, the new values overwrite the old.
 *                        If false, the new values are added to the old.
 */
void movetype::terrain_info::data::merge(const config & new_values, bool overwrite)
{
	if ( overwrite )
		// We do not support child tags here, so do not copy any that might
		// be in the input. (If in the future we need to support child tags,
		// change "merge_attributes" to "merge_with".)
		cfg_.merge_attributes(new_values);
	else {
		for(const auto& [new_key, new_value] : new_values.attribute_range()) {
			config::attribute_value & dest = cfg_[new_key];
			int old = dest.to_int(params_.max_value);

			// The new value is the absolute value of the old plus the
			// provided value, capped between minimum and maximum, then
			// given the sign of the old value.
			// (Think defenses for why we might have negative values.)
			int value = std::abs(old) + new_value.to_int(0);
			value = std::max(params_.min_value, std::min(value, params_.max_value));
			if ( old < 0 )
				value = -value;

			dest = value;
		}
	}

	// The new data has invalidated the cache.
	clear_cache();
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
			   << " depth " << recurse_count;
		return params_.default_value;
	}

	std::shared_ptr<terrain_type_data> tdata;
	if (game_config_manager::get()){
		tdata = game_config_manager::get()->terrain_types(); //This permits to get terrain info in unit help pages from the help in title screen, even if there is no residual gamemap object
	}
	assert(tdata);

	// Get a list of underlying terrains.
	const t_translation::ter_list & underlying = params_.use_move ?
			tdata->underlying_mvt_terrain(terrain) :
			tdata->underlying_def_terrain(terrain);

	if (terrain_type::is_indivisible(terrain, underlying))
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
			       << "; resetting to " << params_.min_value << ".";
			result = params_.min_value;
		}
		if ( result > params_.max_value ) {
			WRN_CF << "Terrain '" << terrain << "' has evaluated to " << result
				   << " (" << (params_.use_move ? "cost" : "defense")
				   << "), which is more than " << params_.max_value
			       << "; resetting to " << params_.max_value << ".";
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
 */
movetype::terrain_info::terrain_info(const parameters & params,
	const terrain_info * fallback) :
	unique_data_(new data(params)),
	fallback_(fallback)
{
}


/**
 * Constructor.
 * @param[in] cfg       An initial data set.
 * @param[in] params    The parameters to use when calculating values.
 *                      This is stored as a reference, so it must be long-lived (typically a static variable).
 * @param[in] fallback  Used as a backup in case we are asked for data we do not have (think vision costs falling back to movement costs).
 */
movetype::terrain_info::terrain_info(const config & cfg, const parameters & params,
	const terrain_info * fallback) :
	unique_data_(new data(cfg, params)),
	fallback_(fallback)
{
}

/**
 * Reverse of terrain_costs::write. Never returns nullptr.
 * @param[in] cfg An initial data set
 */
std::unique_ptr<movetype::terrain_costs> movetype::read_terrain_costs(const config & cfg)
{
	return std::make_unique<terrain_info> (cfg, movetype::mvj_params_, nullptr);
}

/**
 * Copy constructor for callers that handle the fallback and cascade. This is
 * intended for terrain_defense or movetype's copy constructors, where a
 * similar set of terrain_infos will be created, complete with the same
 * relationships between parts of the set.
 *
 * @param[in] that      The terrain_info to copy.
 * @param[in] fallback  Used as a backup in case we are asked for data we do not have (think vision costs falling back to movement costs).
 */
movetype::terrain_info::terrain_info(const terrain_info & that,
	const terrain_info * fallback) :
	fallback_(fallback)
{
	assert(fallback ? !! that.fallback_ : ! that.fallback_);
	copy_data(that);
}

movetype::terrain_info::terrain_info(terrain_info && that,
	const terrain_info * fallback) :
	unique_data_(std::move(that.unique_data_)),
	shared_data_(std::move(that.shared_data_)),
	fallback_(fallback)
{
	assert(fallback ? !! that.fallback_ : ! that.fallback_);
}

/**
 * Destructor
 *
 * While this is simply the default destructor, it needs
 * to be defined in this file so that it knows about ~data(), which
 * is called from the smart pointers' destructor.
 */
movetype::terrain_info::~terrain_info() = default;

/**
 * This is only expected to be called either when
 * 1) both this and @a that have no siblings, as happens when terrain_defense is copied, or
 * 2) all of the siblings are being copied, as happens when movetype is copied.
 */
void movetype::terrain_info::copy_data(const movetype::terrain_info & that)
{
	that.make_data_shareable();
	this->unique_data_.reset();
	this->shared_data_ = that.shared_data_;
}

/**
 * Swap function for the terrain_info class
 *
 * This is only expected to be called either when
 * 1) both this and @a that have no siblings, as happens when swapping two terrain_defenses, or
 * 2) all of the siblings are being swapped, as happens when two movetypes are swapped.
 */
void movetype::terrain_info::swap_data(movetype::terrain_info & that)
{
	// It doesn't matter whether they're both unique, both shared, or
	// one unique with the other shared.
	std::swap(this->unique_data_, that.unique_data_);
	std::swap(this->shared_data_, that.shared_data_);
}
/**
 * Swap function for the terrain_defense class
 *
 * This relies on all of the terrain_infos having no fallback and no cascade,
 * an assumption which is provided by terrain_defense's constructors.
 */
void swap(movetype::terrain_defense & a, movetype::terrain_defense & b)
{
	a.min_.swap_data(b.min_);
	a.max_.swap_data(b.max_);
}

/**
 * Swap function for the movetype class, including its terrain_info members
 *
 * This relies on the two sets of the terrain_infos having their movement,
 * vision and jamming cascaded in the same way. This assumption is provided by
 * movetype's constructors.
 */
void swap(movetype & a, movetype & b)
{
	a.movement_.swap_data(b.movement_);
	a.vision_.swap_data(b.vision_);
	a.jamming_.swap_data(b.jamming_);
	swap(a.defense_, b.defense_);
	std::swap(a.resist_, b.resist_);
	std::swap(a.flying_, b.flying_);
	std::swap(a.special_notes_, b.special_notes_);
}

movetype & movetype::operator=(const movetype & that)
{
	movetype m(that);
	swap(*this, m);
	return *this;
}

movetype & movetype::operator=(movetype && that)
{
	swap(*this, that);
	return *this;
}

/**
 * Returns whether or not our data is empty.
 */
bool movetype::terrain_info::empty() const
{
	return get_data().empty();
}


/**
 * Merges the given config over the existing values.
 * @param[in] new_values  The new values.
 * @param[in] overwrite   If true, the new values overwrite the old.
 *                        If false, the new values are added to the old.
 * @param[in] dependants  Other instances that use this as a fallback.
 */
void movetype::terrain_info::merge(const config & new_values, bool overwrite,
	const std::vector<movetype::terrain_info * > & dependants)
{
	if ( !get_data().config_has_changes(new_values, overwrite) )
		// Nothing will change, so skip the copy-on-write.
		return;

	// Copy-on-write.
	//
	// We also need to make our cascade writeable, because changes to this
	// instance will change data that they receive when using this as their
	// fallback. However, it's no problem for a writable instance to have a
	// shareable instance as its fallback.
	make_data_writable();
	for (auto & dependant : dependants) {
		// This will automatically clear the dependant's cache
		dependant->make_data_writable();
	}

	unique_data_->merge(new_values, overwrite);
}


/**
 * Returns the value associated with the given terrain.
 */
int movetype::terrain_info::value(const t_translation::terrain_code & terrain) const
{
	return get_data().value(terrain, fallback_);
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
		get_data().write(cfg, child_name);
	else
		get_data().write(cfg, child_name, fallback_);
}


/**
 * Does a sufficiently deep copy so that the returned object's lifespan
 * is independent of other objects' lifespan. Never returns nullptr.
 *
 * This implements terrain_costs's virtual method for getting an instance that
 * doesn't depend on the lifespan of a terrain_defense or movetype object.
 * This will do a deep copy of the data (with fallback_ already merged) if
 * needed.
 */
std::unique_ptr<movetype::terrain_costs> movetype::terrain_info::make_standalone() const
{
	std::unique_ptr<terrain_costs> t;
	if(!fallback_) {
		// Call the copy constructor, which will make_data_shareable().
		t = std::make_unique<terrain_info>(*this, nullptr);
	}
	else if(get_data().empty()) {
		// Pure fallback.
		t = fallback_->make_standalone();
	}
	else {
		// Need to merge data.
		config merged;
		write(merged, "", true);
		t = std::make_unique<terrain_info>(merged, get_data().params(), nullptr);
	}
	return t;
}

const movetype::terrain_info::data & movetype::terrain_info::get_data() const
{
	assert(unique_data_ || shared_data_);
	assert(! (unique_data_ && shared_data_));
	if(unique_data_)
		return *unique_data_;
	return *shared_data_;
}

/**
 * Copy the immutable data back to unique_data_, no-op if the data
 * is already in unique_data_.
 *
 * Ensures our data is not shared, and therefore that changes only
 * affect this instance of terrain_info (and any instances using it
 * as a fallback).
 *
 * This does not need to affect the fallback - it's no problem if a
 * writable instance has a fallback to a shareable instance, although
 * a shareable instance must not fallback to a writable instance.
 */
void movetype::terrain_info::make_data_writable() const
{
	if(!unique_data_)
	{
		// Const hack because this is not really changing the data.
		auto t = const_cast<terrain_info *>(this);
		t->unique_data_.reset(new data(*shared_data_));
		t->shared_data_.reset();
	}

	// As we're about to write data, invalidate the cache
	unique_data_->clear_cache();
}

/**
 * Move data to an immutable copy in shared_data_, no-op if the data
 * is already in shared_data_.
 *
 * This is recursive on the fallback chain, because if the data shouldn't be
 * writable then the data shouldn't be writable via the fallback either.
 */
void movetype::terrain_info::make_data_shareable() const
{
	if(!unique_data_)
		return;

	if(fallback_)
		fallback_->make_data_shareable();

	// Const hack because this is not really changing the data.
	auto t = const_cast<terrain_info *>(this);
	t->shared_data_ = std::move(t->unique_data_);
}

/* *** terrain_defense *** */

movetype::terrain_defense::terrain_defense(const terrain_defense & that) :
	min_(that.min_, nullptr),
	max_(that.max_, nullptr)
{
}

movetype::terrain_defense::terrain_defense(terrain_defense && that) :
	min_(std::move(that.min_), nullptr),
	max_(std::move(that.max_), nullptr)
{
}

movetype::terrain_defense & movetype::terrain_defense::operator=(const terrain_defense & that)
{
	min_.copy_data(that.min_);
	max_.copy_data(that.max_);
	return *this;
}

movetype::terrain_defense & movetype::terrain_defense::operator=(terrain_defense && that)
{
	min_.swap_data(that.min_);
	max_.swap_data(that.max_);
	return *this;
}
/**
 * Merges the given config over the existing costs.
 * (Not overwriting implies adding.)
 */
void movetype::terrain_defense::merge(const config & new_data, bool overwrite)
{
	min_.merge(new_data, overwrite, {});
	max_.merge(new_data, overwrite, {});
}

/* *** resistances *** */


/**
 * Returns a map from damage types to resistances.
 */
utils::string_map_res movetype::resistances::damage_table() const
{
	utils::string_map_res result;

	for(const auto& [key, value] : cfg_.attribute_range()) {
		result[key] = value;
	}

	return result;
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
		for(const auto& [key, value] : new_data.attribute_range()) {
			config::attribute_value & dest = cfg_[key];
			dest = std::max(0, dest.to_int(100) + value.to_int(0));
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
	movement_(mvj_params_, nullptr),
	vision_(mvj_params_, &movement_),
	jamming_(mvj_params_, &vision_),
	defense_(),
	resist_(),
	flying_(false),
	special_notes_()
{
}


/**
 * Constructor from a config
 */
movetype::movetype(const config & cfg) :
	movement_(cfg.child_or_empty("movement_costs"), mvj_params_, nullptr),
	vision_(cfg.child_or_empty("vision_costs"), mvj_params_, &movement_),
	jamming_(cfg.child_or_empty("jamming_costs"), mvj_params_, &vision_),
	defense_(cfg.child_or_empty("defense")),
	resist_(cfg.child_or_empty("resistance")),
	flying_(cfg["flies"].to_bool(false))
{
	// 1.15 will support both "flying" and "flies", with "flies" being deprecated
	flying_ = cfg["flying"].to_bool(flying_);

	for(const config& sn : cfg.child_range("special_note")) {
		special_notes_.push_back(sn["note"]);
	}
}


/**
 * Copy constructor
 */
movetype::movetype(const movetype & that) :
	movement_(that.movement_, nullptr),
	vision_(that.vision_, &movement_),
	jamming_(that.jamming_, &vision_),
	defense_(that.defense_),
	resist_(that.resist_),
	flying_(that.flying_),
	special_notes_(that.special_notes_)
{
}

/**
 * Move constructor.
 */
movetype::movetype(movetype && that) :
	movement_(std::move(that.movement_), nullptr),
	vision_(std::move(that.vision_), &movement_),
	jamming_(std::move(that.jamming_), &vision_),
	defense_(std::move(that.defense_)),
	resist_(std::move(that.resist_)),
	flying_(that.flying_),
	special_notes_(std::move(that.special_notes_))
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

void movetype::merge(const config & new_cfg, bool overwrite)
{
	for (const auto & applies_to : movetype::effects) {
		for (const config & child : new_cfg.child_range(applies_to)) {
			merge(child, applies_to, overwrite);
		}
	}

	// "flies" is used when WML defines a movetype.
	// "flying" is used when WML defines a unit.
	// It's easier to support both than to track which case we are in.
	// Note: in 1.15 "flies" is deprecated, with "flying" preferred in movetype too.
	flying_ = new_cfg["flies"].to_bool(flying_);
	flying_ = new_cfg["flying"].to_bool(flying_);
}

void movetype::merge(const config & new_cfg, const std::string & applies_to, bool overwrite)
{
	if(applies_to == "movement_costs") {
		movement_.merge(new_cfg, overwrite, {&vision_, &jamming_});
	}
	else if(applies_to == "vision_costs") {
		vision_.merge(new_cfg, overwrite, {&jamming_});
	}
	else if(applies_to == "jamming_costs") {
		jamming_.merge(new_cfg, overwrite, {});
	}
	else if(applies_to == "defense") {
		defense_.merge(new_cfg, overwrite);
	}
	else if(applies_to == "resistance") {
		resist_.merge(new_cfg, overwrite);
	}
	else {
		ERR_CF << "movetype::merge with unknown applies_to: " << applies_to;
	}
}

/**
 * The set of strings defining effects which apply to movetypes.
 */
const std::set<std::string> movetype::effects {"movement_costs",
	"vision_costs", "jamming_costs", "defense", "resistance"};

void movetype::write(config& cfg, bool include_notes) const
{
	movement_.write(cfg, "movement_costs", false);
	vision_.write(cfg, "vision_costs", false);
	jamming_.write(cfg, "jamming_costs", false);
	defense_.write(cfg, "defense");
	resist_.write(cfg, "resistance");

	if(flying_)
		cfg["flying"] = true;

	if(include_notes) {
		for(const auto& note : special_notes_) {
			cfg.add_child("special_note", config{"note", note});
		}
	}
}
