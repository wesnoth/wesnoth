/*
   Copyright (C) 2003 - 2014 by David White <dave@whitevine.net>
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
 *  Generate race-specific unit-names.
 */

#include "global.hpp"

#include "race.hpp"

#include "log.hpp"
#include "serialization/string_utils.hpp"
#include "random_new.hpp"

/// Dummy race used when a race is not yet known.
const unit_race unit_race::null_race;
/// Standard string id (not translatable) for FEMALE
const std::string unit_race::s_female("female");
/// Standard string id (not translatable) for MALE
const std::string unit_race::s_male("male");


static const config &empty_traits() {
		static config cfg;
		return cfg;
}

static const config &empty_topics() {
  		static config cfg;
		return cfg;
}

static void add_prefixes(const ucs4::string& str, size_t length, markov_prefix_map& res)
{
	for(size_t i = 0; i <= str.size(); ++i) {
		const size_t start = i > length ? i - length : 0;
		const ucs4::string key(str.begin() + start, str.begin() + i);
		const ucs4::char_t c = i != str.size() ? str[i] : 0;
		res[key].push_back(c);
	}
}

static markov_prefix_map markov_prefixes(const std::vector<std::string>& items, size_t length)
{
	markov_prefix_map res;

	for(std::vector<std::string>::const_iterator i = items.begin(); i != items.end(); ++i) {
		add_prefixes(unicode_cast<ucs4::string>(*i),length,res);
	}

	return res;
}

static ucs4::string markov_generate_name(const markov_prefix_map& prefixes,
	size_t chain_size, size_t max_len)
{
	if(prefixes.empty() || chain_size == 0) {
		return ucs4::string();
	}

	ucs4::string prefix, res;

	// Since this function is called in the name description in a MP game it
	// uses the local locale. The locale between players can be different and
	// thus the markov_prefix_map can be different. This resulted in
	// get_random() getting called a different number of times for the
	// generation in different locales (due to the bail out at 'if(c == 0)').
	//
	// This causes a problem since the random state is no longer in sync. The
	// following calls to get_random() return different results, which caused
	// traits to be different. To avoid that problem we call get_random()
	// the maximum number of times and store the result in a lookup table.
	std::vector<int> random(max_len);
	size_t j = 0;
	for(; j < max_len; ++j) {
		random[j] = random_new::generator->next_random();
	}

	j = 0;
	while(res.size() < max_len) {
		const markov_prefix_map::const_iterator i = prefixes.find(prefix);
		if(i == prefixes.end() || i->second.empty()) {
			return res;
		}

		const ucs4::char_t c = i->second[random[j++]%i->second.size()];
		if(c == 0) {
			return res;
		}

		res.resize(res.size()+1);
		res[res.size()-1] = c;
		prefix.resize(prefix.size()+1);
		prefix[prefix.size()-1] = c;
		while(prefix.size() > chain_size) {
			prefix.erase(prefix.begin());
		}
	}

	// Getting here means that the maximum length was reached when
	// generating the name, hence the ending of the name has to be
	// made valid. Otherwise weird names like Unárierini- and
	// Thramboril-G may occur.

	// Strip characters from the end until the last prefix of the
	// name has end-of-string as a possible next character in the
	// markov prefix map. If no valid ending is found, use the
	// originally generated name.
	ucs4::string originalRes = res;
	while(!res.empty()) {
		const int prefixLen = chain_size < res.size() ? chain_size : res.size();
		prefix = ucs4::string(res.end() - prefixLen, res.end());

		const markov_prefix_map::const_iterator i = prefixes.find(prefix);
		if (i == prefixes.end() || i->second.empty()) {
			return res;
		}
		if (std::find(i->second.begin(), i->second.end(), static_cast<ucs4::char_t>(0))
				!= i->second.end()) {
			// This ending is valid.
			return res;
		}
		// The current ending is invalid, remove the last character
		// and retry.
		res.pop_back();
	}
	// No valid ending at all could be found. This generally should
	// not happen, unless the chain length is very long or the
	// maximum length is very small. Return the originally generated
	// name, it's not much we can do about it.
	return originalRes;
}

unit_race::unit_race() :
		cfg_(),
		id_(),
		plural_name_(),
		description_(),
		ntraits_(0),
		chain_size_(0),
		traits_(empty_traits().child_range("trait")),
		topics_(empty_topics().child_range("topic")),
		global_traits_(true),
		undead_variation_()
{
		name_[MALE] = "";
		name_[FEMALE] = "";
}

unit_race::unit_race(const config& cfg) :
		cfg_(cfg),
		id_(cfg["id"]),
		plural_name_(cfg["plural_name"].t_str()),
		description_(cfg["description"].t_str()),
		ntraits_(cfg["num_traits"]),
		chain_size_(cfg["markov_chain_size"]),
		traits_(cfg.child_range("trait")),
		topics_(cfg.child_range("topic")),
		global_traits_(!cfg["ignore_global_traits"].to_bool()),
		undead_variation_(cfg["undead_variation"])

{
	if (id_.empty()) {
		lg::wml_error << "[race] '" << cfg["name"] << "' is missing an id field.";
	}
	if (plural_name_.empty()) {
		lg::wml_error << "[race] '" << cfg["name"] << "' is missing a plural_name field.";
		plural_name_ = (cfg["name"]);
	}
	// use "name" if "male_name" or "female_name" aren't available
	name_[MALE] = cfg["male_name"];
	if(name_[MALE].empty()) {
		name_[MALE] = (cfg["name"]);
	}
	name_[FEMALE] = cfg["female_name"];
	if(name_[FEMALE].empty()) {
		name_[FEMALE] = (cfg["name"]);
	}

	if(chain_size_ <= 0)
		chain_size_ = 2;

	//std::vector<std::string> names = ;
	next_[MALE] = markov_prefixes(utils::split(cfg["male_names"]), chain_size_);
	next_[FEMALE] = markov_prefixes(utils::split(cfg["female_names"]), chain_size_);
}

std::string unit_race::generate_name(
		unit_race::GENDER gender) const
{
	return unicode_cast<utf8::string>(
		markov_generate_name(next_[gender], chain_size_, 12));
}

bool unit_race::uses_global_traits() const
{
	return global_traits_;
}

const config::const_child_itors &unit_race::additional_traits() const
{
	return traits_;
}

const config::const_child_itors &unit_race::additional_topics() const
{
  return topics_;
}

unsigned int unit_race::num_traits() const { return ntraits_; }


std::string const& gender_string(unit_race::GENDER gender) {
	switch(gender) {
	case unit_race::FEMALE:
		return unit_race::s_female;
	default:
	case unit_race::MALE:
		return unit_race::s_male;
	}
}

unit_race::GENDER string_gender(const std::string& str, unit_race::GENDER def) {
	if ( str == unit_race::s_male ) {
		return unit_race::MALE;
	} else if ( str == unit_race::s_female ) {
		return unit_race::FEMALE;
	}
	return def;
}
