/*
   Copyright (C) 2012 - 2018 by Iris Morelle <shadowm2006@gmail.com>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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
#include "config.hpp"
#include "font/pango/escape.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "picture.hpp"
#include "log.hpp"
#include "serialization/string_utils.hpp"

static lg::log_domain log_addons_client("addons-client");
#define ERR_AC LOG_STREAM(err ,  log_addons_client)
#define LOG_AC LOG_STREAM(info,  log_addons_client)

namespace {
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

		for(const std::string& dep : base_deps) {
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

void addon_info_translation::read(const config& cfg)
{
	this->supported = cfg["supported"].to_bool(true);
	this->title = cfg["title"].str();
	this->description = cfg["description"].str();
}

void addon_info_translation::write(config& cfg) const
{
	cfg["supported"] = this->supported;
	cfg["title"] = this->title;
	cfg["description"] = this->description;
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
	this->type = get_addon_type(cfg["type"].str());

	const config::const_child_itors& locales_as_configs = cfg.child_range("translation");

	for(const config& locale : locales_as_configs) {
		if(locale["supported"].to_bool(true))
			this->locales.emplace_back(locale["language"].str());
		this->info_translations.emplace(locale["language"].str(), addon_info_translation(locale));
	}

	this->core = cfg["core"].str();
	this->depends = utils::split(cfg["dependencies"].str());
	this->tags = utils::split(cfg["tags"].str());
	this->feedback_url = cfg["feedback_url"].str();

	this->updated = cfg["timestamp"].to_time_t();
	this->created = cfg["original_timestamp"].to_time_t();

	this->local_only = cfg["local_only"].to_bool();
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
	cfg["type"] = get_addon_type_string(this->type);

	for(const auto& element : this->info_translations) {
		config& locale = cfg.add_child("translation");
		locale["language"] = element.first;
		element.second.write(locale);
	}

	cfg["core"] = this->core;
	cfg["dependencies"] = utils::join(this->depends);
	cfg["tags"] = utils::join(this->tags);
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
		return font::escape_text(make_addon_title(this->id));
	} else {
		return font::escape_text(this->title);
	}
}

addon_info_translation addon_info_translation::invalid = {false, "", ""};

addon_info_translation addon_info::translated_info() const
{
	std::string locale = get_language().localename;

	if(locale != "en_US") {
		auto info = info_translations.find(locale);
		if(info != info_translations.end()) {
			return info->second;
		}

		info = info_translations.find(locale.substr(0, 2));
		if(info != info_translations.end()) {
			return info->second;
		}
	}

	return addon_info_translation::invalid;
}

std::string addon_info::display_title_translated() const
{
	addon_info_translation info = this->translated_info();

	if(info.valid()) {
		return info.title;
	}

	return "";
}

std::string addon_info::display_title_translated_or_original() const
{
	std::string title = display_title_translated();
	return title.empty() ? display_title() : title;
}

std::string addon_info::description_translated() const
{
	addon_info_translation info = this->translated_info();

	if(info.valid() && !info.description.empty()) {
		return info.description;
	}

	return this->description;
}

std::string addon_info::display_title_full() const
{
	std::string local_title = display_title_translated();
	if(local_title.empty())
		return display_title();
	return local_title + " (" + display_title() + ")";
}

std::string addon_info::display_icon() const
{
	std::string ret = icon;

	if(ret.empty()) {
		ERR_AC << "add-on '" << id << "' doesn't have an icon path set" << std::endl;
	} else if(!image::exists(ret)) {
		ERR_AC << "add-on '" << id << "' has an icon which cannot be found: '" << ret << "'" << std::endl;
	} else if(ret.find("units/") != std::string::npos && ret.find_first_of('~') == std::string::npos) {
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
	case ADDON_MOD:
		return _("addon_type^Modification");
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

	/** @todo FIXME: get rid of this legacy "campaign"/"campaigns" silliness
	 */
	const config::const_child_itors &addon_cfgs = cfg.child_range("campaign");
	for(const config& addon_cfg : addon_cfgs) {
		const std::string& id = addon_cfg["name"].str();
		if(dest.find(id) != dest.end()) {
			ERR_AC << "add-ons list has multiple entries for '" << id << "', not good; ignoring them" << std::endl;
			continue;
		}
		dest[id].read(addon_cfg);
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
