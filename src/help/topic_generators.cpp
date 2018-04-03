/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-help"

#include "help/topic_generators.hpp"

#include "game_config_manager.hpp"
#include "gettext.hpp"
#include "help/constants.hpp"
#include "help/topic.hpp"
#include "help/topic_text_generators.hpp"
#include "help/utils.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "serialization/string_utils.hpp"
#include "tod_manager.hpp"
#include "units/types.hpp"

#include <map>
#include <set>
#include <string>

static lg::log_domain log_help("help");
#define DBG_HP LOG_STREAM(debug, log_help)

namespace help
{
topic_list generate_ability_topics(const bool sort_generated)
{
	topic_list topics;

	std::map<std::string, const unit_type::ability_metadata*> ability_topic_data;
	std::map<std::string, std::set<std::string, string_less>> ability_units;

	const auto parse = [&](const unit_type& type, const unit_type::ability_metadata& ability) {
		// NOTE: neither ability names nor ability ids are necessarily unique. Creating
		// topics for either each unique name or each unique id means certain abilities
		// will be excluded from help. So... the ability topic ref id is a combination
		// of id and (untranslated) name. It's rather ugly, but it works.
		const std::string topic_ref = ability.id + ability.name.base_str();

		ability_topic_data.emplace(topic_ref, &ability);

		if(!type.hide_help()) {
			// Add a link in the list of units with this ability
			// We put the translated name at the beginning of the hyperlink,
			// so the automatic alphabetic sorting of std::set can use it
			const std::string link = make_link(type.type_name(), unit_prefix + type.id());
			ability_units[topic_ref].insert(link);
		}
	};

	// Look through all the unit types. If a unit of that type would have a full
	// description, add its abilities to the potential topic list. We don't want
	// to show abilities that the user has not encountered yet.
	for(const auto& type_mapping : unit_types.types()) {
		const unit_type& type = type_mapping.second;

		if(description_type(type) != FULL_DESCRIPTION) {
			continue;
		}

		for(const unit_type::ability_metadata& ability : type.abilities_metadata()) {
			parse(type, ability);
		}

		for(const unit_type::ability_metadata& ability : type.adv_abilities_metadata()) {
			parse(type, ability);
		}
	}

	for(const auto& a : ability_topic_data) {
		std::ostringstream text;
		text << a.second->description;
		text << "\n\n" << "<big>" << _("Units with this ability") << "</big>"<< "\n";

		for(const auto& u : ability_units[a.first]) { // first is the topic ref id
			text << font::unicode_bullet << " " << u << "\n";
		}

		topics.emplace_back(ability_prefix + a.first, a.second->name, text.str());
	}

	if(sort_generated) {
		topics.sort();
	}

	return topics;
}

topic_list generate_weapon_special_topics(const bool sort_generated)
{
	topic_list topics;

	std::map<t_string, std::string> special_description;
	std::map<t_string, std::set<std::string, string_less>> special_units;

	// FIXME: make this use the same id + name ref method as the ability topics.
	const auto parse = [&](const unit_type& type, const std::pair<t_string, t_string>& data) {
		special_description.emplace(data.first, data.second);

		if(!type.hide_help()) {
			// Check for variations (walking corpse/soulless etc)
			const std::string section_prefix = type.show_variations_in_help() ? ".." : "";
			const std::string ref_id = section_prefix + unit_prefix + type.id();

			// We put the translated name at the beginning of the hyperlink,
			// so the automatic alphabetic sorting of std::set can use it
			special_units[data.first].insert(make_link(type.type_name(), ref_id));
		}
	};

	const auto parse_config = [&parse](const unit_type& type, const config& specials) {
		for(const config::any_child& spec : specials.all_children_range()) {
			if(!spec.cfg["name"].empty()) {
				parse(type, std::make_pair(spec.cfg["name"].t_str(), spec.cfg["description"].t_str()));
			}
		}
	};

	for(const auto& type_mapping : unit_types.types()) {
		const unit_type& type = type_mapping.second;

		// Only show the weapon special if we find it on a unit that
		// detailed description should be shown about.
		if(description_type(type) != FULL_DESCRIPTION) {
			continue;
		}

		for(const attack_type& atk : type.attacks()) {
			for(const auto& s_tooltip : atk.special_tooltips()) {
				parse(type, s_tooltip);
			}
		}

		for(const config& adv : type.modification_advancements()) {
			for(const config& effect : adv.child_range("effect")) {
				if(effect["apply_to"] == "new_attack" && effect.has_child("specials")) {
					parse_config(type, effect.child("specials"));
				} else if(effect["apply_to"] == "attack" && effect.has_child("set_specials")) {
					parse_config(type, effect.child("set_specials"));
				}
			}
		}
	}

	for(auto& s : special_description) {
		// use untranslated name to have universal topic id
		std::string id = weapon_special_prefix + s.first.base_str();

		std::ostringstream text;
		text << s.second;
		text << "\n\n" << "<big>" << _("Units with this special attack") << "</big>" << "\n";

		for(const std::string& u : special_units[s.first]) {
			text << font::unicode_bullet << " " << u << "\n";
		}

		topics.emplace_back(id, s.first, text.str());
	}

	if(sort_generated) {
		topics.sort();
	}

	return topics;
}

topic_list generate_time_of_day_topics(const bool /*sort_generated*/)
{
	topic_list topics;
	std::ostringstream toplevel;

	if(!resources::tod_manager) {
		toplevel << _("Only available during a scenario.");
		topics.emplace_back("..schedule", _("Time of Day Schedule"), toplevel.str());
		return topics;
	}

	for(const time_of_day& time : resources::tod_manager->times()) {
		const std::string id = tod_prefix + time.id;
		const std::string image = "<img>src='" + time.image + "'</img>";

		toplevel << make_link(time.name.str(), id) << jump_to(160) << image << jump(30) << time.lawful_bonus << '\n';

		std::ostringstream text;
		text << image << '\n'
			 << time.description.str() << '\n'
			 << _("Lawful Bonus:") << ' ' << time.lawful_bonus << '\n'
			 << '\n'
			 << make_link(_("Schedule"), "..schedule");

		topics.emplace_back(id, time.name.str(), text.str());
	}

	topics.emplace_back("..schedule", _("Time of Day Schedule"), toplevel.str());
	return topics;
}

topic_list generate_trait_topics(const bool sort_generated)
{
	topic_list topics;
	std::map<t_string, const config> trait_list;

	for(const config& trait : unit_types.traits()) {
		trait_list.emplace(trait["id"], trait);
	}

	for(const auto& i : unit_types.types()) {
		const unit_type& type = i.second;

		if(description_type(type) == FULL_DESCRIPTION) {
			if(auto traits = type.possible_traits()) {
				for(const config& trait : traits) {
					trait_list.emplace(trait["id"], trait);
				}
			}

			if(const unit_race* r = unit_types.find_race(type.race_id())) {
				for(const config& trait : r->additional_traits()) {
					trait_list.emplace(trait["id"], trait);
				}
			}
		}
	}

	for(auto& a : trait_list) {
		const std::string id = trait_prefix + a.first;
		const config& trait = a.second;

		std::string name = trait["male_name"].str();
		if(name.empty()) {
			name = trait["female_name"].str();
		}

		if(name.empty()) {
			name = trait["name"].str();
		}

		if(name.empty()) {
			continue; // Hidden trait
		}

		std::ostringstream text;

		if(!trait["help_text"].empty()) {
			text << trait["help_text"];
		} else if(!trait["description"].empty()) {
			text << trait["description"];
		} else {
			text << _("No description available.");
		}

		text << "\n\n";

		if(trait["availability"] == "musthave") {
			text << _("Availability: Must-have") << "\n";
		} else if(trait["availability"] == "none") {
			text << _("Availability: Unavailable") << "\n";
		}

		topics.emplace_back(id, name, text.str());
	}

	if(sort_generated) {
		topics.sort();
	}

	return topics;
}

topic_list generate_unit_topics(const bool sort_generated, const std::string& race)
{
	topic_list topics;

	std::set<std::string, string_less> race_units;
	std::set<std::string, string_less> race_topics;
	std::set<std::string> alignments;

	for(const auto& i : unit_types.types()) {
		const unit_type& type = i.second;

		if(type.race_id() != race) {
			continue;
		}

		if(description_type(type) != FULL_DESCRIPTION) {
			continue;
		}

		const std::string type_name = type.type_name();
		const std::string real_prefix = type.show_variations_in_help() ? ".." : "";
		const std::string ref_id = hidden_symbol(type.hide_help()) + real_prefix + unit_prefix + type.id();

		topics.emplace_back(ref_id, type_name, std::make_unique<unit_topic_generator>(type));

		if(!type.hide_help()) {
			// we also record an hyperlink of this unit in the list used for the race topic/
			race_units.insert(make_link(type_name, ref_id));

			alignments.insert(
				make_link(type.alignment_description(type.alignment(), type.genders().front()), "time_of_day"));
		}
	}

	// generate the hidden race description topic
	std::string race_id = ".." + race_prefix + race;
	std::string race_name;
	std::string race_description;

	if(const unit_race* r = unit_types.find_race(race)) {
		race_name = r->plural_name();
		race_description = r->description();

		for(const config& additional_topic : r->additional_topics()) {
			std::string id = additional_topic["id"];
			std::string title = additional_topic["title"];
			std::string text = additional_topic["text"];

			topics.emplace_back(id, title, text);

			race_topics.insert(make_link(title, id));
		}
	} else {
		race_name = _("race^Miscellaneous");
	}

	std::ostringstream text;

	if(!race_description.empty()) {
		text << race_description << "\n\n";
	}

	if(!alignments.empty()) {
		text << (alignments.size() > 1 ? _("Alignments:") : _("Alignment:")) << ' ';
		text << utils::join(alignments, ", ") << "\n\n";
	}

	text << "<big>" << _("Units of this race") << "</big>\n";

	for(const std::string& u : race_units) {
		text << font::unicode_bullet << " " << u << "\n";
	}

	topics.emplace_back(race_id, race_name, text.str());

	if(sort_generated) {
		topics.sort();
	}

	return topics;
}

topic_list generate_faction_topics(const bool sort_generated, const config& era)
{
	topic_list topics;

	for(const config& f : era.child_range("multiplayer_side")) {
		const std::string& id = f["id"];
		if(id == "Random") {
			continue;
		}

		std::ostringstream text;

		const config::attribute_value& description = f["description"];
		if(!description.empty()) {
			text << description.t_str() << "\n";
			text << "\n";
		}

		const std::vector<std::string> recruit_ids = utils::split(f["recruit"]);
		std::set<std::string> races;
		std::set<std::string> alignments;

		for(const std::string& u_id : recruit_ids) {
			if(const unit_type* t = unit_types.find(u_id, unit_type::HELP_INDEXED)) {
				assert(t);
				const unit_type& type = *t;

				if(const unit_race* r = unit_types.find_race(type.race_id())) {
					races.insert(make_link(r->plural_name(), std::string("..") + race_prefix + r->id()));
				}

				DBG_HP << type.alignment() << " -> "
					   << type.alignment_description(type.alignment(), type.genders().front()) << "\n";

				alignments.insert(
					make_link(type.alignment_description(type.alignment(), type.genders().front()), "time_of_day"));
			}
		}

		if(!races.empty()) {
			text << _("Races:") << ' ' << utils::join(races, ", ");
			text << "\n\n";
		}

		if(!alignments.empty()) {
			text << _("Alignments:") << ' ' << utils::join(alignments, ", ");
			text << "\n\n";
		}

		text << "<big>" << _("Leaders") << "</big>";
		text << "\n";

		for(const std::string& link : make_unit_links_list(utils::split(f["leader"]), true)) {
			text << font::unicode_bullet << " " << link << "\n";
		}

		text << "\n";
		text << "<big>" << _("Recruits") << "</big>";
		text << "\n";

		for(const std::string& link : make_unit_links_list(recruit_ids, true)) {
			text << font::unicode_bullet << " " << link << "\n";
		}

		const std::string ref_id = faction_prefix + era["id"] + "_" + id;
		topics.emplace_back(ref_id, f["name"], text.str());
	}

	if(sort_generated) {
		topics.sort();
	}

	return topics;
}

topic_list generate_era_topics(const bool sort_generated, const std::string& era_id)
{
	topic_list topics;

	// TODO: should we be using the gcm here?
	const config& era = game_config_manager::get()->game_config().find_child("era", "id", era_id);

	if(era && !era["hide_help"].to_bool()) {
		topics = generate_faction_topics(sort_generated, era);

		std::vector<std::string> faction_links;
		for(const topic& t : topics) {
			faction_links.push_back(make_link(t.title, t.id));
		}

		std::ostringstream text;
		text << "<big>" << _("Era:") << " " << era["name"] << "</big>";
		text << "\n\n";

		const config::attribute_value& description = era["description"];
		if(!description.empty()) {
			text << description.t_str() << "\n\n";
		}

		text << "<big>" << _("Factions") << "</big>";
		text << "\n";

		std::sort(faction_links.begin(), faction_links.end());

		for(const std::string& link : faction_links) {
			text << font::unicode_bullet << " " << link << "\n";
		}

		const std::string ref_id = ".." + era_prefix + era["id"].str();
		topics.emplace_back(ref_id, era["name"], text.str());
	}

	return topics;
}

} // namespace help
