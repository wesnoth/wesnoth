/*
	Copyright (C) 2003 - 2025
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
 * Note:
 * Prior to the creation of this file, all the code associated to the help
 * browser existed in a single file src/help.cpp, including all of the
 * widgets, topic generators, and implementation details. This totaled
 * ~4000 lines of code.
 *
 * I have split it all up now, so that the gui aspects are separated from
 * the content, the "front facing" part which the rest of the code base
 * interacts with is in src/help/help.?pp, and the topic generators are
 * separated. The remaining "guts" are here. It is implemented in a static
 * singleton pattern, using "extern"'d variables, simply for ease of translation
 * from the previous state. It would probably be a good idea to rewrite this
 * guy as a proper C++ object. Feel free to do so, or to adopt some other
 * design pattern.
 */

#pragma once

#include "color.hpp"
#include "exceptions.hpp"               // for error
#include "font/constants.hpp"
#include "font/standard_colors.hpp"
#include "gettext.hpp"
#include "serialization/string_utils.hpp"
#include "utils/optional_fwd.hpp"

#include <cstring>
#include <list>                         // for list
#include <memory>
#include <sstream>
#include <string>                       // for string, allocator, etc
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, etc
#include "config.hpp"

class game_config_view;
class unit_type;

namespace help {

/** Generate a topic text on the fly. */
class topic_generator
{
public:
	virtual std::string operator()() const = 0;
	virtual ~topic_generator() {}
};

class text_topic_generator: public topic_generator {
	std::string text_;
public:
	text_topic_generator(const std::string& t): text_(t) {}
	virtual std::string operator()() const { return text_; }
};

/**
 * The text displayed in a topic. It is generated on the fly with the information contained in generator_
 */
class topic_text
{
	mutable config parsed_text_;
	mutable std::shared_ptr<topic_generator> generator_;
public:
	explicit topic_text(std::shared_ptr<topic_generator> g):
		parsed_text_(),
		generator_(std::move(g))
	{
	}

	const config& parsed_text() const;
};

/** A topic contains a title, an id and some text. */
struct topic
{
	topic(const std::string& topic_title, const std::string& topic_id, const std::string& text)
		: title(topic_title), id(topic_id), text(std::make_shared<text_topic_generator>(text)) {}
	topic(const std::string& topic_title, const std::string& topic_id, std::shared_ptr<topic_generator> g)
		: title(topic_title), id(topic_id), text(std::move(g)) {}
	/** Two topics are equal if their IDs are equal. */
	bool operator==(const topic& ) const;
	bool operator!=(const topic& t) const { return !operator==(t); }
	/** Comparison on the ID. */
	bool operator<(const topic& ) const;
	std::string title, id;
	mutable topic_text text;
};

struct section;
typedef std::list<section> section_list;
typedef std::list<topic> topic_list;

/** A section contains topics and sections along with title and ID. */
struct section {
	/** Two sections are equal if their IDs are equal. */
	bool operator==(const section&) const;
	/** Comparison on the ID. */
	bool operator<(const section&) const;

	/** Allocate memory for and add the section. */
	void add_section(const section& s);
	void add_section(section&& s);

	void clear();
	std::string title, id;
	topic_list topics;
	section_list sections;
};


/**
 * To be used as a function object to locate sections and topics with a specified ID.
 */
class has_id
{
public:
	has_id(const std::string& id) : id_(id) {}
	bool operator()(const topic& t) { return t.id == id_; }
	bool operator()(const section& s) { return s.id == id_; }
	bool operator()(const section* s) { return s != nullptr && s->id == id_; }
private:
	const std::string id_;
};

/** To be used as a function object when sorting topic lists on the title. */
class title_less
{
public:
	bool operator()(const topic& t1, const topic& t2)
	{
		return translation::compare(t1.title, t2.title) < 0;
	}
};

/** To be used as a function object when sorting section lists on the title. */
class section_less
{
public:
	bool operator()(const section& s1, const section& s2)
	{
		return translation::compare(s1.title, s2.title) < 0;
	}
};

class string_less
{
public:
	bool operator() (const std::string& s1, const std::string& s2) const {
		return translation::compare(s1, s2) < 0;
	}
};

// Generator stuff below. Maybe move to a separate file? This one is
// getting crowded. Dunno if much more is needed though so I'll wait and
// see.

/** Dispatch generators to their appropriate functions. */
void generate_sections(const config& help_cfg, const std::string& generator, section& sec, int level);
std::vector<topic> generate_topics(const bool sort_topics,const std::string& generator);
std::string generate_topic_text(const std::string& generator, const config& help_cfg, const section& sec);
std::string generate_contents_links(const std::string& section_name, const config& help_cfg);
std::string generate_contents_links(const section& sec);

/** Thrown when the help system fails to parse something. */
struct parse_error : public game::error
{
	parse_error(const std::string& msg) : game::error(msg) {}
};

/**
 * return a hyperlink with the unit's name and pointing to the unit page
 * return empty string if this unit is hidden. If not yet discovered add the (?) suffix
 */
std::string make_unit_link(const std::string& type_id);
/** return a list of hyperlinks to unit's pages (ordered or not) */
std::vector<std::string> make_unit_links_list(
		const std::vector<std::string>& type_id_list, bool ordered = false);

void generate_races_sections(const config& help_cfg, section& sec, int level);
void generate_terrain_sections(section& sec, int level);
std::vector<topic> generate_unit_topics(const std::string& race, const bool sort_generated);
void generate_unit_sections(const config& help_cfg, section& sec, int level, const bool, const std::string& race);
enum UNIT_DESCRIPTION_TYPE {
	FULL_DESCRIPTION,
	/** Ignore this unit for documentation purposes. */
	NO_DESCRIPTION,
	/**
	 * Although the unit itself is hidden, traits reachable via this unit are not hidden.
	 *
	 * This is a bug workaround - traits are defined by WML macros, and therefore the help
	 * system has to use a place where that macro is instanciated to provide the documentation.
	 * None of the normal unit types has the "loyal" trait, but there is a hidden unit which
	 * does, purely to support the help system.
	 */
	HIDDEN_BUT_SHOW_MACROS
};
/**
 * Return the type of description that should be shown for a unit of
 * the given kind. This method is intended to filter out information
 * about units that should not be shown, for example due to not being
 * encountered.
 */
UNIT_DESCRIPTION_TYPE description_type(const unit_type& type);
std::vector<topic> generate_ability_topics(const bool sort_generated);
std::vector<topic> generate_time_of_day_topics(const bool);
std::vector<topic> generate_weapon_special_topics(const bool sort_generated);

void generate_era_sections(const config& help_cfg, section& sec, int level);
std::vector<topic> generate_faction_topics(const config& era, const bool sort_generated);
std::vector<topic> generate_era_topics(const std::string& era_id, const bool sort_generated);
std::vector<topic> generate_trait_topics(const bool sort_generated);

/**
 * Generate the help contents from the configurations given to the manager.
 * @returns A pair consisting of the toplevel section and any hidden sections.
 */
std::pair<section, section> generate_contents();

/**
 * Parse a help config, return the top level section. Return an empty
 * section if cfg is nullptr.
 */
section parse_config(const config& cfg);

/** Recursive function used by parse_config. */
section parse_config_internal(const config& help_cfg, const config& section_cfg, int level = 0);

/**
 * Return true if the section with id section_id is referenced from
 * another section in the config, or the toplevel.
 */
bool section_is_referenced(const std::string& section_id, const config& cfg);
/**
 * Return true if the topic with id topic_id is referenced from
 * another section in the config, or the toplevel.
 */
bool topic_is_referenced(const std::string& topic_id, const config& cfg);

/**
 * Search for the topic with the specified identifier in the section
 * and its subsections. Return the found topic, or nullptr if none could
 * be found.
 */
const topic* find_topic(const section& sec, const std::string& id);

/**
 * Search for the section with the specified identifier in the section
 * and its subsections. Return the found section or nullptr if none could
 * be found.
 */
const section* find_section(const section& sec, const std::string& id);
section* find_section(section& sec, const std::string& id);

extern const int max_section_level;
// The topic to open by default when opening the help dialog.
extern const std::string default_show_topic;
extern const std::string unknown_unit_topic;
extern const std::string unit_prefix;
extern const std::string terrain_prefix;
extern const std::string race_prefix;
extern const std::string faction_prefix;
extern const std::string era_prefix;
extern const std::string variation_prefix;
extern const std::string ability_prefix;

// id starting with '.' are hidden
std::string hidden_symbol(bool hidden = true);

bool is_visible_id(const std::string& id);

/**
 * Return true if the id is valid for user defined topics and
 * sections. Some IDs are special, such as toplevel and may not be
 * be defined in the config.
 */
bool is_valid_id(const std::string& id);

} // end namespace help
