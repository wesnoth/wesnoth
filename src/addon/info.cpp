/*
   Copyright (C) 2012 - 2014 by Ignacio Riquelme Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "addon/info.hpp"

#include "addon/manager.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "image.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

#include <boost/foreach.hpp>

static lg::log_domain log_addons_client("addons-client");
#define ERR_AC LOG_STREAM(err ,  log_addons_client)
#define LOG_AC LOG_STREAM(info,  log_addons_client)

namespace {
	const std::string fallback_addon_icon = "misc/blank-hex.png";

	void resolve_deps_recursive(const addons_list& addons, const std::string& base_id, std::set<std::string>& dest)
	{
		addons_list::const_iterator it = addons.find(base_id);
		if(it == addons.end()) {
			LOG_AC << "resolve_deps_recursive(): " << base_id << " not in add-ons list\n";
			return;
		}

		const std::vector<std::string>& base_deps = it->second.depends;

		if(base_deps.empty()) {
			return;
		}

		BOOST_FOREACH(const std::string& dep, base_deps) {
			if(base_id == dep) {
				LOG_AC << dep << " depends upon itself; breaking circular dependency\n";
				continue;
			} else if(dest.find(dep) != dest.end()) {
				LOG_AC << dep << " already in dependency tree; breaking circular dependency\n";
				continue;
			}

			dest.insert(dep);

			resolve_deps_recursive(addons, dep, dest);
		}
	}
}

void addon_info::read(const config& cfg)
{
	this->id = cfg["name"].str();
	this->title = cfg["title"].str();
	this->description = cfg["description"].str();
	this->icon = cfg["icon"].str();
	this->version = cfg["version"].str();
	this->author = cfg["author"].str();
	this->size = cfg["size"];
	this->downloads = cfg["downloads"];
	this->uploads = cfg["uploads"];
	this->user_rating = cfg["user_rating"];
	this->hours_played = cfg["hours_played"];

	int rating = cfg["user_rating"].to_int() / 2;
	int downloads_per_year = 0;
	int hours_played_per_year = 0;


	if (!cfg["original_timestamp"].empty() && cfg["original_timestamp"].to_int() != time(NULL)) {
		float years_it_exists = (time(NULL) - cfg["original_timestamp"].to_int()) / (365.3 * 24.0 * 600.0);
		downloads_per_year = cfg["downloads"].to_int() / years_it_exists;
		hours_played_per_year = cfg["hours_played"].to_int() / years_it_exists;
	}	else {
		// This shouldn't normally happen without old 1.11's addons.
		downloads_per_year = cfg["downloads"].to_int();
		hours_played_per_year = cfg["hours_played"].to_int();
	}

	// Apologising for using magic numbers, but a number reflecting as many quality
	// numbers as possible is highly desirable

	// The point is that the rating is a weighted average between the players' ratings
	// and objective ratings like number of downloads and hours played per unit of time
	// These two ratings are exponential functions desiged to give some score even to
	// the ones with little downloads and not too much to the ones with many, they will
	// have to be adjusted over time.

	rating += 25 * (1.0 - pow(2.0,(downloads_per_year / -300.0 )));

	rating += 25 * (1.0 - pow(2.0,(-1 * log10( 1 + hours_played_per_year / 1000.0))));
	// Addons like resource packs are not played, but they will not be compensated for
	// that handicap, because they are usually resource packs and such, things that
	// are rather dependencies and files for UMC authors, not playable stuff players
	// want to download.

	this->score = rating;

	const config::const_child_itors& reviews = cfg.child_range("review");
	BOOST_FOREACH(const config& review, reviews) {
		this->reviews.push_back((addon_info::addon_review){
									review["id"], review["overall"], review["gameplay"], review["visuals"],
									review["story"], review["balance"], review["likes"], false});
	}

	this->type = get_addon_type(cfg["type"].str());

	const config::const_child_itors& locales = cfg.child_range("translation");

	BOOST_FOREACH(const config& locale, locales) {
		this->locales.push_back(locale["language"].str());
	}

	this->core = cfg["core"].str();
	this->depends = utils::split(cfg["dependencies"].str());
	this->feedback_url = cfg["feedback_url"].str();

	this->updated = cfg["timestamp"].to_time_t();
	this->created = cfg["original_timestamp"].to_time_t();
}

void addon_info::write(config& cfg) const
{
	cfg["id"] = this->id;
	cfg["title"] = this->title;
	cfg["description"] = this->description;
	cfg["icon"] = this->icon;
	cfg["version"] = this->version.str();
	cfg["author"] = this->author;
	cfg["size"] = this->size;
	cfg["downloads"] = this->downloads;
	cfg["uploads"] = this->uploads;
	cfg["user_rating"] = this->user_rating;
	cfg["hours_played"] = this->hours_played;

	BOOST_FOREACH(const addon_info::addon_review& review, this->reviews) {
		config review_wml;
		config& review_wml_to_append = review_wml;
		review_wml["id"] = review.id;
		review_wml["overall"] = review.overall;
		review_wml["gameplay"] = review.gameplay;
		review_wml["visuals"] = review.visuals;
		review_wml["story"] = review.story;
		review_wml["balance"] = review.balance;
		review_wml["likes"] = review.likes;
		cfg.add_child("review") = review_wml_to_append;
	}

	cfg["type"] = get_addon_type_string(this->type);

	BOOST_FOREACH(const std::string& locale_id, this->locales) {
		cfg.add_child("translation")["language"] = locale_id;
	}

	cfg["core"] = this->core;
	cfg["dependencies"] = utils::join(this->depends);
	cfg["feedback_url"] = this->feedback_url;

	cfg["timestamp"] = this->updated;
	cfg["original_timestamp"] = this->created;
}

void addon_info::write_minimal(config& cfg) const
{
	cfg["version"] = this->version.str();
	cfg["uploads"] = this->uploads;
	cfg["type"] = get_addon_type_string(this->type);
	cfg["title"] = this->title;
	cfg["dependencies"] = utils::join(this->depends);
	cfg["core"] = this->core;
}

std::string addon_info::display_title() const
{
	if(this->title.empty()) {
		return make_addon_title(this->id);
	} else {
		return this->title;
	}
}

std::string addon_info::display_icon() const
{
	std::string ret = icon;

	if(ret.empty()) {
		ERR_AC << "add-on '" << id << "' doesn't have an icon path set" << std::endl;
		ret = fallback_addon_icon;
	}
	else if(!image::exists(ret)) {
		ERR_AC << "add-on '" << id << "' has an icon which cannot be found: '" << ret << "'" << std::endl;
		ret = game_config::debug ? game_config::images::missing : fallback_addon_icon;
	}
	else if(ret.find("units/") != std::string::npos && ret.find_first_of('~') == std::string::npos) {
		// HACK: prevent magenta icons, because they look awful
		LOG_AC << "add-on '" << id << "' uses a unit baseframe as icon without TC/RC specifications\n";
		ret += "~RC(magenta>red)";
	}

	return ret;
}

std::string addon_info::display_type() const
{
	switch (type) {
	case ADDON_SP_CAMPAIGN:
		return _("addon_type^Campaign");
	case ADDON_SP_SCENARIO:
		return _("addon_type^Scenario");
	case ADDON_SP_MP_CAMPAIGN:
		return _("addon_type^SP/MP Campaign");
	case ADDON_MP_ERA:
		return _("addon_type^MP era");
	case ADDON_MP_FACTION:
		return _("addon_type^MP faction");
	case ADDON_MP_MAPS:
		return _("addon_type^MP map-pack");
	case ADDON_MP_SCENARIO:
		return _("addon_type^MP scenario");
	case ADDON_MP_CAMPAIGN:
		return _("addon_type^MP campaign");
	case ADDON_MP_MOD:
		return _("addon_type^MP modification");
	case ADDON_CORE:
		return _("addon_type^Core");
	case ADDON_MEDIA:
		return _("addon_type^Resources");
	case ADDON_OTHER:
		return _("addon_type^Other");
	default:
		return _("addon_type^(unknown)");
	}
}

std::set<std::string> addon_info::resolve_dependencies(const addons_list& addons) const
{
	std::set<std::string> deps;
	resolve_deps_recursive(addons, this->id, deps);

	if(deps.find(this->id) != deps.end()) {
		LOG_AC << this->id << " depends upon itself; breaking circular dependency\n";
		deps.erase(this->id);
	}

	return deps;
}

void read_addons_list(const config& cfg, addons_list& dest)
{
	dest.clear();

	unsigned order = 0;

	/** @todo FIXME: get rid of this legacy "campaign"/"campaigns" silliness */
	const config::const_child_itors &addon_cfgs = cfg.child_range("campaign");
	BOOST_FOREACH(const config& addon_cfg, addon_cfgs) {
		const std::string& id = addon_cfg["name"].str();
		if(dest.find(id) != dest.end()) {
			ERR_AC << "add-ons list has multiple entries for '" << id << "', not good; ignoring them" << std::endl;
			continue;
		}
		dest[id].read(addon_cfg);
		dest[id].order = order++;
	}
}

std::string size_display_string(double size)
{
	if(size > 0.0) {
		return utils::si_string(size, true, _("unit_byte^B"));
	} else {
		return "";
	}
}

std::string make_addon_title(const std::string& id)
{
	std::string ret(id);
	std::replace(ret.begin(), ret.end(), '_', ' ');
	return ret;
}
