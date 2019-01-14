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

#include "exceptions.hpp"               // for error
#include "font/sdl_ttf.hpp"             // for line_width, relative_size
#include "gettext.hpp"
#include <cstring>
#include <list>                         // for list
#include <memory>
#include <ostream>                      // for operator<<, stringstream, etc
#include <string>                       // for string, allocator, etc
#include <utility>                      // for pair, make_pair
#include <vector>                       // for vector, etc
#include <SDL.h>                  // for SDL_Surface
#include <boost/logic/tribool.hpp>

class config;
class unit_type;
class terrain_type_data;
typedef std::shared_ptr<terrain_type_data> ter_data_cache;
namespace help { struct section; }  // lines 51-51

namespace help {

/// Generate the help contents from the configurations given to the
/// manager.
void generate_contents();

typedef std::vector<section *> section_list;

/// Generate a topic text on the fly.
class topic_generator
{
	unsigned count;
	friend class topic_text;
public:
	topic_generator(): count(1) {}
	virtual std::string operator()() const = 0;
	virtual ~topic_generator() {}
};

class text_topic_generator: public topic_generator {
	std::string text_;
public:
	text_topic_generator(const std::string& t): text_(t) {}
	virtual std::string operator()() const { return text_; }
};

/// The text displayed in a topic. It is generated on the fly with the information
/// contained in generator_.
class topic_text
{
	mutable std::vector< std::string > parsed_text_;
	mutable topic_generator *generator_;
public:
	~topic_text();
	topic_text():
		parsed_text_(),
		generator_(nullptr)
	{
	}

	topic_text(const std::string& t):
		parsed_text_(),
		generator_(new text_topic_generator(t))
	{
	}

	explicit topic_text(topic_generator *g):
		parsed_text_(),
		generator_(g)
	{
	}
	topic_text &operator=(topic_generator *g);
	topic_text(const topic_text& t);

    const std::vector<std::string>& parsed_text() const;
};

/// A topic contains a title, an id and some text.
struct topic
{
	topic() :
		title(),
		id(),
		text()
	{
	}

	topic(const std::string &_title, const std::string &_id) :
		title(_title),
		id(_id),
		text()
	{
	}

	topic(const std::string &_title, const std::string &_id, const std::string &_text)
		: title(_title), id(_id), text(_text) {}
	topic(const std::string &_title, const std::string &_id, topic_generator *g)
		: title(_title), id(_id), text(g) {}
	/// Two topics are equal if their IDs are equal.
	bool operator==(const topic &) const;
	bool operator!=(const topic &t) const { return !operator==(t); }
	/// Comparison on the ID.
	bool operator<(const topic &) const;
	std::string title, id;
	mutable topic_text text;
};

typedef std::list<topic> topic_list;

/// A section contains topics and sections along with title and ID.
struct section {
	section() :
		title(""),
		id(""),
		topics(),
		sections(),
		level()
	{
	}

	section(const section&);
	section& operator=(const section&);
	~section();
	/// Two sections are equal if their IDs are equal.
	bool operator==(const section &) const;
	/// Comparison on the ID.
	bool operator<(const section &) const;

	/// Allocate memory for and add the section.
	void add_section(const section &s);

	void clear();
	std::string title, id;
	topic_list topics;
	section_list sections;
	int level;
};


/// To be used as a function object to locate sections and topics
/// with a specified ID.
class has_id
{
public:
	has_id(const std::string &id) : id_(id) {}
	bool operator()(const topic &t) { return t.id == id_; }
	bool operator()(const section &s) { return s.id == id_; }
	bool operator()(const section *s) { return s != nullptr && s->id == id_; }
private:
	const std::string id_;
};

/// To be used as a function object when sorting topic lists on the title.
class title_less
{
public:
	bool operator()(const topic &t1, const topic &t2) {
            return translation::compare(t1.title, t2.title) < 0; }
};

/// To be used as a function object when sorting section lists on the title.
class section_less
{
public:
	bool operator()(const section* s1, const section* s2) {
            return translation::compare(s1->title, s2->title) < 0; }
};

class string_less
{
public:
	bool operator() (const std::string &s1, const std::string &s2) const {
		return translation::compare(s1, s2) < 0;
	}
};

struct delete_section
{
	void operator()(section *s) { delete s; }
};

struct create_section
{
	section *operator()(const section *s) { return new section(*s); }
	section *operator()(const section &s) { return new section(s); }
};

/// Thrown when the help system fails to parse something.
struct parse_error : public game::error
{
	parse_error(const std::string& msg) : game::error(msg) {}
};

// Generator stuff below. Maybe move to a separate file? This one is
// getting crowded. Dunno if much more is needed though so I'll wait and
// see.

/// Dispatch generators to their appropriate functions.
void generate_sections(const config *help_cfg, const std::string &generator, section &sec, int level);
std::vector<topic> generate_topics(const bool sort_topics,const std::string &generator);
std::string generate_topic_text(const std::string &generator, const config *help_cfg,
const section &sec, const std::vector<topic>& generated_topics);
std::string generate_contents_links(const std::string& section_name, config const *help_cfg);
std::string generate_contents_links(const section &sec, const std::vector<topic>& topics);

/// return a hyperlink with the unit's name and pointing to the unit page
/// return empty string if this unit is hidden. If not yet discovered add the (?) suffix
std::string make_unit_link(const std::string& type_id);
/// return a list of hyperlinks to unit's pages (ordered or not)
std::vector<std::string> make_unit_links_list(
		const std::vector<std::string>& type_id_list, bool ordered = false);

void generate_races_sections(const config *help_cfg, section &sec, int level);
void generate_terrain_sections(const config* help_cfg, section &sec, int level);
std::vector<topic> generate_unit_topics(const bool, const std::string& race);
void generate_unit_sections(const config *help_cfg, section &sec, int level, const bool, const std::string& race);
enum UNIT_DESCRIPTION_TYPE {FULL_DESCRIPTION, NO_DESCRIPTION, NON_REVEALING_DESCRIPTION};
/// Return the type of description that should be shown for a unit of
/// the given kind. This method is intended to filter out information
/// about units that should not be shown, for example due to not being
/// encountered.
UNIT_DESCRIPTION_TYPE description_type(const unit_type &type);
std::vector<topic> generate_ability_topics(const bool);
std::vector<topic> generate_time_of_day_topics(const bool);
std::vector<topic> generate_weapon_special_topics(const bool);

void generate_era_sections(const config *help_cfg, section &sec, int level);
std::vector<topic> generate_faction_topics(const config &, const bool);
std::vector<topic> generate_era_topics(const bool, const std::string & era_id);
std::vector<topic> generate_trait_topics(const bool);

/// Parse a help config, return the top level section. Return an empty
/// section if cfg is nullptr.
section parse_config(const config *cfg);
/// Recursive function used by parse_config.
void parse_config_internal(const config *help_cfg, const config *section_cfg,
						   section &sec, int level=0);

/// Return true if the section with id section_id is referenced from
/// another section in the config, or the toplevel.
bool section_is_referenced(const std::string &section_id, const config &cfg);
/// Return true if the topic with id topic_id is referenced from
/// another section in the config, or the toplevel.
bool topic_is_referenced(const std::string &topic_id, const config &cfg);

/// Search for the topic with the specified identifier in the section
/// and its subsections. Return the found topic, or nullptr if none could
/// be found.
const topic *find_topic(const section &sec, const std::string &id);

/// Search for the section with the specified identifier in the section
/// and its subsections. Return the found section or nullptr if none could
/// be found.
const section *find_section(const section &sec, const std::string &id);

/// Parse a text string. Return a vector with the different parts of the
/// text. Each markup item is a separate part while the text between
/// markups are separate parts.
std::vector<std::string> parse_text(const std::string &text);

/// Convert the contents to wml attributes, surrounded within
/// [element_name]...[/element_name]. Return the resulting WML.
std::string convert_to_wml(const std::string &element_name, const std::string &contents);

/// Return the color the string represents. Return font::NORMAL_COLOR if
/// the string is empty or can't be matched against any other color.
color_t string_to_color(const std::string &s);

/// Make a best effort to word wrap s. All parts are less than width.
std::vector<std::string> split_in_width(const std::string &s, const int font_size, const unsigned width);

std::string remove_first_space(const std::string& text);

/// Prepend all chars with meaning inside attributes with a backslash.
std::string escape(const std::string &s);

/// Return the first word in s, not removing any spaces in the start of
/// it.
std::string get_first_word(const std::string &s);

/// Load the appropriate terrain types data to use
ter_data_cache load_terrain_types_data();

extern const config *game_cfg;
// The default toplevel.
extern help::section default_toplevel;
// All sections and topics not referenced from the default toplevel.
extern help::section hidden_sections;

extern int last_num_encountered_units;
extern int last_num_encountered_terrains;
extern boost::tribool last_debug_state;

extern std::vector<std::string> empty_string_vector;
extern const int max_section_level;
extern const int title_size;
extern const int title2_size;
extern const int box_width;
extern const int normal_font_size;
extern const unsigned max_history;
extern const std::string topic_img;
extern const std::string closed_section_img;
extern const std::string open_section_img;
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

bool is_visible_id(const std::string &id);

/// Return true if the id is valid for user defined topics and
/// sections. Some IDs are special, such as toplevel and may not be
/// be defined in the config.
bool is_valid_id(const std::string &id);

	// Helpers for making generation of topics easier.

inline std::string make_link(const std::string& text, const std::string& dst)
	{
		// some sorting done on list of links may rely on the fact that text is first
		return "<ref>text='" + help::escape(text) + "' dst='" + help::escape(dst) + "'</ref>";
	}

inline std::string jump_to(const unsigned pos)
	{
		std::stringstream ss;
		ss << "<jump>to=" << pos << "</jump>";
		return ss.str();
	}

inline std::string jump(const unsigned amount)
	{
		std::stringstream ss;
		ss << "<jump>amount=" << amount << "</jump>";
		return ss.str();
	}

inline std::string bold(const std::string &s)
	{
		std::stringstream ss;
		ss << "<bold>text='" << help::escape(s) << "'</bold>";
		return ss.str();
	}

typedef std::vector<std::vector<std::pair<std::string, unsigned int >> > table_spec;
// Create a table using the table specs. Return markup with jumps
// that create a table. The table spec contains a vector with
// vectors with pairs. The pairs are the markup string that should
// be in a cell, and the width of that cell.
std::string generate_table(const table_spec &tab, const unsigned int spacing=font::relative_size(20));

// Return the width for the image with filename.
unsigned image_width(const std::string &filename);

void push_tab_pair(std::vector<std::pair<std::string, unsigned int>> &v, const std::string &s);

} // end namespace help
