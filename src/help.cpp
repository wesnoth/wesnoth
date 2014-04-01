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
 * @file
 * Routines for showing the help-dialog.
 */

#define GETTEXT_DOMAIN "wesnoth-help"

#include "global.hpp"
#include "asserts.hpp"
#include "help.hpp"

#include "about.hpp"
#include "display.hpp"
#include "exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "marked-up_text.hpp"
#include "sound.hpp"
#include "unit.hpp"
#include "unit_helper.hpp"
#include "wml_separators.hpp"
#include "serialization/parser.hpp"

#include <boost/foreach.hpp>

#include <queue>

static lg::log_domain log_display("display");
#define WRN_DP LOG_STREAM(warn, log_display)

namespace help {

help_button::help_button(display& disp, const std::string &help_topic)
	: dialog_button(disp.video(), _("Help")), disp_(disp), topic_(help_topic), help_hand_(NULL)
{}

help_button::~help_button() {
	delete help_hand_;
}

int help_button::action(gui::dialog_process_info &info) {
	if(!topic_.empty()) {
		show_help();
		info.clear_buttons();
	}
	return gui::CONTINUE_DIALOG;
}

void help_button::show_help()
{
	help::show_help(disp_, topic_);
}

bool help_button::can_execute_command(const hotkey::hotkey_command& cmd, int/*index*/) const
{
	hotkey::HOTKEY_COMMAND command = cmd.id;
	return (topic_.empty() == false && command == hotkey::HOTKEY_HELP) || command == hotkey::HOTKEY_SCREENSHOT;
}

void help_button::join() {
	dialog_button::join();

	//wait until we join the event context to start a hotkey handler
	delete help_hand_;
	help_hand_ = new hotkey::basic_handler(&disp_, this);
}

void help_button::leave() {
	dialog_button::leave();

	//now kill the hotkey handler
	delete help_hand_;
	help_hand_ = NULL;
}

/// Generate the help contents from the configurations given to the
/// manager.
static void generate_contents();

struct section;

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
	text_topic_generator(std::string const &t): text_(t) {}
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
		generator_(NULL)
	{
	}

	topic_text(std::string const &t):
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
	topic_text(topic_text const &t);

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
	bool operator()(const section *s) { return s != NULL && s->id == id_; }
private:
	const std::string id_;
};

/// To be used as a function object when sorting topic lists on the title.
class title_less
{
public:
	bool operator()(const topic &t1, const topic &t2) {
            return strcoll(t1.title.c_str(), t2.title.c_str()) < 0; }
};

/// To be used as a function object when sorting section lists on the title.
class section_less
{
public:
	bool operator()(const section* s1, const section* s2) {
            return strcoll(s1->title.c_str(), s2->title.c_str()) < 0; }
};

class string_less
{
public:
	bool operator() (const std::string &s1, const std::string &s2) const {
		return strcoll(s1.c_str(), s2.c_str()) < 0;
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

/// The menu to the left in the help browser, where topics can be
/// navigated through and chosen.
class help_menu : public gui::menu
{
public:
	help_menu(CVideo &video, const section &toplevel, int max_height=-1);
	int process();

	/// Make the topic the currently selected one, and expand all
	/// sections that need to be expanded to show it.
	void select_topic(const topic &t);

	/// If a topic has been chosen, return that topic, otherwise
	/// NULL. If one topic is returned, it will not be returned again,
	/// if it is not re-chosen.
	const topic *chosen_topic();

private:
	/// Information about an item that is visible in the menu.
	struct visible_item {
		visible_item(const section *_sec, const std::string &visible_string);
		visible_item(const topic *_t, const std::string &visible_string);
		// Invariant, one if these should be NULL. The constructors
		// enforce it.
		const topic *t;
		const section *sec;
		std::string visible_string;
		bool operator==(const visible_item &vis_item) const;
		bool operator==(const section &sec) const;
		bool operator==(const topic &t) const;
	};

	/// Regenerate what items are visible by checking what sections are
	/// expanded.
	void update_visible_items(const section &top_level, unsigned starting_level=0);

	/// Return true if the section is expanded.
	bool expanded(const section &sec);

	/// Mark a section as expanded. Do not update the visible items or
	/// anything.
	void expand(const section &sec);

	/// Contract (close) a section. That is, mark it as not expanded,
	/// visible items are not updated.
	void contract(const section &sec);

	/// Return the string to use as the prefix for the icon part of the
	/// menu-string at the specified level.
	std::string indented_icon(const std::string &icon, const unsigned level);
	/// Return the string to use as the menu-string for sections at the
	/// specified level.
	std::string get_string_to_show(const section &sec, const unsigned level);
	/// Return the string to use as the menu-string for topics at the
	/// specified level.
	std::string get_string_to_show(const topic &topic, const unsigned level);

	/// Draw the currently visible items.
	void display_visible_items();

	/// Internal recursive thingie. did_expand will be true if any
	/// section was expanded, otherwise untouched.
	bool select_topic_internal(const topic &t, const section &sec);

	std::vector<visible_item> visible_items_;
	const section &toplevel_;
	std::set<const section*> expanded_;
	surface_restorer restorer_;
	topic const *chosen_topic_;
	visible_item selected_item_;
};

/// Thrown when the help system fails to parse something.
struct parse_error : public game::error
{
	parse_error(const std::string& msg) : game::error(msg) {}
};

/// The area where the content is shown in the help browser.
class help_text_area : public gui::scrollarea
{
public:
	help_text_area(CVideo &video, const section &toplevel);
	/// Display the topic.
	void show_topic(const topic &t);

	/// Return the ID that is cross-referenced at the (screen)
	/// coordinates x, y. If no cross-reference is there, return the
	/// empty string.
	std::string ref_at(const int x, const int y);

protected:
	virtual void scroll(unsigned int pos);
	virtual void set_inner_location(const SDL_Rect& rect);

private:
	enum ALIGNMENT {LEFT, MIDDLE, RIGHT, HERE};
	/// Convert a string to an alignment. Throw parse_error if
	/// unsuccessful.
	ALIGNMENT str_to_align(const std::string &s);

	/// An item that is displayed in the text area. Contains the surface
	/// that should be blitted along with some other information.
	struct item {

		item(surface surface, int x, int y, const std::string& text="",
			 const std::string& reference_to="", bool floating=false,
			 bool box=false, ALIGNMENT alignment=HERE);

		item(surface surface, int x, int y,
			 bool floating, bool box=false, ALIGNMENT=HERE);

		/// Relative coordinates of this item.
		SDL_Rect rect;

		surface surf;

		// If this item contains text, this will contain that text.
		std::string text;

		// If this item contains a cross-reference, this is the id
		// of the referenced topic.
		std::string ref_to;

		// If this item is floating, that is, if things should be filled
		// around it.
		bool floating;
		bool box;
		ALIGNMENT align;
	};

	/// Function object to find an item at the specified coordinates.
	class item_at {
	public:
		item_at(const int x, const int y) : x_(x), y_(y) {}
		bool operator()(const item&) const;
	private:
		const int x_, y_;
	};

	/// Update the vector with the items of the shown topic, creating
	/// surfaces for everything and putting things where they belong.
	void set_items();

	// Create appropriate items from configs. Items will be added to the
	// internal vector. These methods check that the necessary
	// attributes are specified.
	void handle_ref_cfg(const config &cfg);
	void handle_img_cfg(const config &cfg);
	void handle_bold_cfg(const config &cfg);
	void handle_italic_cfg(const config &cfg);
	void handle_header_cfg(const config &cfg);
	void handle_jump_cfg(const config &cfg);
	void handle_format_cfg(const config &cfg);

	void draw_contents();

	/// Add an item with text. If ref_dst is something else than the
	/// empty string, the text item will be underlined to show that it
	/// is a cross-reference. The item will also remember what the
	/// reference points to. If font_size is below zero, the default
	/// will be used.
	void add_text_item(const std::string& text, const std::string& ref_dst="",
					   bool broken_link = false,
					   int font_size=-1, bool bold=false, bool italic=false,
					   SDL_Color color=font::NORMAL_COLOR);

	/// Add an image item with the specified attributes.
	void add_img_item(const std::string& path, const std::string& alignment, const bool floating,
					  const bool box);

	/// Move the current input point to the next line.
	void down_one_line();

	/// Adjust the heights of the items in the last row to make it look
	/// good .
	void adjust_last_row();

	/// Return the width that remain on the line the current input point is at.
	int get_remaining_width();

	/// Return the least x coordinate at which something of the
	/// specified height can be drawn at the specified y coordinate
	/// without interfering with floating images.
	int get_min_x(const int y, const int height=0);

	/// Analogous with get_min_x but return the maximum X.
	int get_max_x(const int y, const int height=0);

	/// Find the lowest y coordinate where a floating img of the
	/// specified width and at the specified x coordinate can be
	/// placed. Start looking at desired_y and continue downwards. Only
	/// check against other floating things, since text and inline
	/// images only can be above this place if called correctly.
	int get_y_for_floating_img(const int width, const int x, const int desired_y);

	/// Add an item to the internal list, update the locations and row
	/// height.
	void add_item(const item& itm);

	std::list<item> items_;
	std::list<item *> last_row_;
	const section &toplevel_;
	topic const *shown_topic_;
	const int title_spacing_;
	// The current input location when creating items.
	std::pair<int, int> curr_loc_;
	const unsigned min_row_height_;
	unsigned curr_row_height_;
	/// The height of all items in total.
	int contents_height_;
};

/// A help browser widget.
class help_browser : public gui::widget
{
public:
	help_browser(display &disp, const section &toplevel);

	void adjust_layout();

	/// Display the topic with the specified identifier. Open the menu
	/// on the right location and display the topic in the text area.
	void show_topic(const std::string &topic_id);

protected:
	virtual void update_location(SDL_Rect const &rect);
	virtual void process_event();
	virtual void handle_event(const SDL_Event &event);

private:
	/// Update the current cursor, set it to the reference cursor if
	/// mousex, mousey is over a cross-reference, otherwise, set it to
	/// the normal cursor.
	void update_cursor();
	void show_topic(const topic &t, bool save_in_history=true);
	/// Move in the topic history. Pop an element from from and insert
	/// it in to. Pop at the fronts if the maximum number of elements is
	/// exceeded.
	void move_in_history(std::deque<const topic *> &from, std::deque<const topic *> &to);
	display &disp_;
	help_menu menu_;
	help_text_area text_area_;
	const section &toplevel_;
	bool ref_cursor_; // If the cursor currently is the hyperlink cursor.
	std::deque<const topic *> back_topics_, forward_topics_;
	gui::button back_button_, forward_button_;
	topic const *shown_topic_;
};

// Generator stuff below. Maybe move to a separate file? This one is
// getting crowded. Dunno if much more is needed though so I'll wait and
// see.

/// Dispatch generators to their appropriate functions.
static void generate_sections(const config *help_cfg, const std::string &generator, section &sec, int level);
static std::vector<topic> generate_topics(const bool sort_topics,const std::string &generator);
static std::string generate_topic_text(const std::string &generator, const config *help_cfg,
const section &sec, const std::vector<topic>& generated_topics);
static std::string generate_about_text();
static std::string generate_contents_links(const std::string& section_name, config const *help_cfg);
static std::string generate_contents_links(const section &sec, const std::vector<topic>& topics);

/// return a hyperlink with the unit's name and pointing to the unit page
/// return empty string if this unit is hidden. If not yet discovered add the (?) suffix
static std::string make_unit_link(const std::string& type_id);
/// return a list of hyperlinks to unit's pages (ordered or not)
static std::vector<std::string> make_unit_links_list(
		const std::vector<std::string>& type_id_list, bool ordered = false);

static void generate_races_sections(const config *help_cfg, section &sec, int level);
static void generate_terrain_sections(const config* help_cfg, section &sec, int level);
static std::vector<topic> generate_unit_topics(const bool, const std::string& race);
static void generate_unit_sections(const config *help_cfg, section &sec, int level, const bool, const std::string& race);
enum UNIT_DESCRIPTION_TYPE {FULL_DESCRIPTION, NO_DESCRIPTION, NON_REVEALING_DESCRIPTION};
/// Return the type of description that should be shown for a unit of
/// the given kind. This method is intended to filter out information
/// about units that should not be shown, for example due to not being
/// encountered.
static UNIT_DESCRIPTION_TYPE description_type(const unit_type &type);
static std::vector<topic> generate_ability_topics(const bool);
static std::vector<topic> generate_weapon_special_topics(const bool);
static std::vector<topic> generate_faction_topics(const bool);

/// Parse a help config, return the top level section. Return an empty
/// section if cfg is NULL.
static section parse_config(const config *cfg);
/// Recursive function used by parse_config.
static void parse_config_internal(const config *help_cfg, const config *section_cfg,
						   section &sec, int level=0);

/// Return true if the section with id section_id is referenced from
/// another section in the config, or the toplevel.
static bool section_is_referenced(const std::string &section_id, const config &cfg);
/// Return true if the topic with id topic_id is referenced from
/// another section in the config, or the toplevel.
static bool topic_is_referenced(const std::string &topic_id, const config &cfg);

/// Search for the topic with the specified identifier in the section
/// and its subsections. Return the found topic, or NULL if none could
/// be found.
static const topic *find_topic(const section &sec, const std::string &id);

/// Search for the section with the specified identifier in the section
/// and its subsections. Return the found section or NULL if none could
/// be found.
static const section *find_section(const section &sec, const std::string &id);

/// Parse a text string. Return a vector with the different parts of the
/// text. Each markup item is a separate part while the text between
/// markups are separate parts.
static std::vector<std::string> parse_text(const std::string &text);

/// Convert the contents to wml attributes, surrounded within
/// [element_name]...[/element_name]. Return the resulting WML.
static std::string convert_to_wml(const std::string &element_name, const std::string &contents);

/// Return the color the string represents. Return font::NORMAL_COLOR if
/// the string is empty or can't be matched against any other color.
static SDL_Color string_to_color(const std::string &s);

/// Make a best effort to word wrap s. All parts are less than width.
static std::vector<std::string> split_in_width(const std::string &s, const int font_size, const unsigned width);

static std::string remove_first_space(const std::string& text);

/// Prepend all chars with meaning inside attributes with a backslash.
static std::string escape(const std::string &s)
{
	return utils::escape(s, "'\\");
}

/// Return the first word in s, not removing any spaces in the start of
/// it.
static std::string get_first_word(const std::string &s);

} // namespace help

namespace {
	const config *game_cfg = NULL;
	// The default toplevel.
	help::section toplevel;
	// All sections and topics not referenced from the default toplevel.
	help::section hidden_sections;

	int last_num_encountered_units = -1;
	int last_num_encountered_terrains = -1;
	bool last_debug_state = game_config::debug;

	config dummy_cfg;
	std::vector<std::string> empty_string_vector;
	const int max_section_level = 15;
	const int title_size = font::SIZE_LARGE;
	const int title2_size = font::SIZE_15;
	const int box_width = 2;
	const int normal_font_size = font::SIZE_SMALL;
	const unsigned max_history = 100;
	const std::string topic_img = "help/topic.png";
	const std::string closed_section_img = "help/closed_section.png";
	const std::string open_section_img = "help/open_section.png";
	const std::string indentation_img = "help/indentation.png";
	// The topic to open by default when opening the help dialog.
	const std::string default_show_topic = "introduction_topic";
	const std::string unknown_unit_topic = ".unknown_unit";
	const std::string unit_prefix = "unit_";
	const std::string terrain_prefix = "terrain_";
	const std::string race_prefix = "race_";
	const std::string faction_prefix = "faction_";
	const std::string variation_prefix = "variation_";

	// id starting with '.' are hidden
	static std::string hidden_symbol(bool hidden = true) {
		return (hidden ? "." : "");
	}

	static bool is_visible_id(const std::string &id) {
		return (id.empty() || id[0] != '.');
	}

	/// Return true if the id is valid for user defined topics and
	/// sections. Some IDs are special, such as toplevel and may not be
	/// be defined in the config.
	static bool is_valid_id(const std::string &id) {
		if (id == "toplevel") {
			return false;
		}
		if (id.find(unit_prefix) == 0 || id.find(hidden_symbol() + unit_prefix) == 0) {
			return false;
		}
		if (id.find("ability_") == 0) {
			return false;
		}
		if (id.find("weaponspecial_") == 0) {
			return false;
		}
		if (id == "hidden") {
			return false;
		}
		return true;
	}

	/// Class to be used as a function object when generating the about
	/// text. Translate the about dialog formatting to format suitable
	/// for the help dialog.
	class about_text_formatter {
	public:
		std::string operator()(const std::string &s) {
			if (s.empty()) return s;
			// Format + as headers, and the rest as normal text.
			if (s[0] == '+')
				return " \n<header>text='" + help::escape(s.substr(1)) + "'</header>";
			if (s[0] == '-')
				return s.substr(1);
			return s;
		}
	};

}

	// Helpers for making generation of topics easier.

static std::string make_link(const std::string& text, const std::string& dst)
	{
		// some sorting done on list of links may rely on the fact that text is first
		return "<ref>text='" + help::escape(text) + "' dst='" + help::escape(dst) + "'</ref>";
	}

static std::string jump_to(const unsigned pos)
	{
		std::stringstream ss;
		ss << "<jump>to=" << pos << "</jump>";
		return ss.str();
	}

static std::string jump(const unsigned amount)
	{
		std::stringstream ss;
		ss << "<jump>amount=" << amount << "</jump>";
		return ss.str();
	}

static std::string bold(const std::string &s)
	{
		std::stringstream ss;
		ss << "<bold>text='" << help::escape(s) << "'</bold>";
		return ss.str();
	}

	typedef std::vector<std::vector<std::pair<std::string, unsigned int > > > table_spec;
	// Create a table using the table specs. Return markup with jumps
	// that create a table. The table spec contains a vector with
	// vectors with pairs. The pairs are the markup string that should
	// be in a cell, and the width of that cell.
static std::string generate_table(const table_spec &tab, const unsigned int spacing=font::relative_size(20))
  {
		table_spec::const_iterator row_it;
		std::vector<std::pair<std::string, unsigned> >::const_iterator col_it;
		unsigned int num_cols = 0;
		for (row_it = tab.begin(); row_it != tab.end(); ++row_it) {
			if (row_it->size() > num_cols) {
				num_cols = row_it->size();
			}
		}
		std::vector<unsigned int> col_widths(num_cols, 0);
		// Calculate the width of all columns, including spacing.
		for (row_it = tab.begin(); row_it != tab.end(); ++row_it) {
			unsigned int col = 0;
			for (col_it = row_it->begin(); col_it != row_it->end(); ++col_it) {
				if (col_widths[col] < col_it->second + spacing) {
					col_widths[col] = col_it->second + spacing;
				}
				++col;
			}
		}
		std::vector<unsigned int> col_starts(num_cols);
		// Calculate the starting positions of all columns
		for (unsigned int i = 0; i < num_cols; ++i) {
			unsigned int this_col_start = 0;
			for (unsigned int j = 0; j < i; ++j) {
				this_col_start += col_widths[j];
			}
			col_starts[i] = this_col_start;
		}
		std::stringstream ss;
		for (row_it = tab.begin(); row_it != tab.end(); ++row_it) {
			unsigned int col = 0;
			for (col_it = row_it->begin(); col_it != row_it->end(); ++col_it) {
				ss << jump_to(col_starts[col]) << col_it->first;
				++col;
			}
			ss << "\n";
		}
		return ss.str();
	}

	// Return the width for the image with filename.
static unsigned image_width(const std::string &filename)
	{
		image::locator loc(filename);
		surface surf(image::get_image(loc));
		if (surf != NULL) {
			return surf->w;
		}
		return 0;
	}

static void push_tab_pair(std::vector<std::pair<std::string, unsigned int> > &v, const std::string &s)
	{
		v.push_back(std::make_pair(s, font::line_width(s, normal_font_size)));
	}

namespace help {

help_manager::help_manager(const config *cfg) //, gamemap *_map)
{
	game_cfg = cfg == NULL ? &dummy_cfg : cfg;
//	map = _map;
}

void generate_contents()
{
	toplevel.clear();
	hidden_sections.clear();
	if (game_cfg != NULL) {
		const config *help_config = &game_cfg->child("help");
		if (!*help_config) {
			help_config = &dummy_cfg;
		}
		try {
			toplevel = parse_config(help_config);
			// Create a config object that contains everything that is
			// not referenced from the toplevel element. Read this
			// config and save these sections and topics so that they
			// can be referenced later on when showing help about
			// specified things, but that should not be shown when
			// opening the help browser in the default manner.
			config hidden_toplevel;
			std::stringstream ss;
			BOOST_FOREACH(const config &section, help_config->child_range("section"))
			{
				const std::string id = section["id"];
				if (find_section(toplevel, id) == NULL) {
					// This section does not exist referenced from the
					// toplevel. Hence, add it to the hidden ones if it
					// is not referenced from another section.
					if (!section_is_referenced(id, *help_config)) {
						if (ss.str() != "") {
							ss << ",";
						}
						ss << id;
					}
				}
			}
			hidden_toplevel["sections"] = ss.str();
			ss.str("");
			BOOST_FOREACH(const config &topic, help_config->child_range("topic"))
			{
				const std::string id = topic["id"];
				if (find_topic(toplevel, id) == NULL) {
					if (!topic_is_referenced(id, *help_config)) {
						if (ss.str() != "") {
							ss << ",";
						}
						ss << id;
					}
				}
			}
			hidden_toplevel["topics"] = ss.str();
			config hidden_cfg = *help_config;
			// Change the toplevel to our new, custom built one.
			hidden_cfg.clear_children("toplevel");
			hidden_cfg.add_child("toplevel", hidden_toplevel);
			hidden_sections = parse_config(&hidden_cfg);
		}
		catch (parse_error& e) {
			std::stringstream msg;
			msg << "Parse error when parsing help text: '" << e.message << "'";
			std::cerr << msg.str() << std::endl;
		}
	}
}

help_manager::~help_manager()
{
	game_cfg = NULL;
//	map = NULL;
	toplevel.clear();
	hidden_sections.clear();
    // These last numbers must be reset so that the content is regenerated.
    // Upon next start.
	last_num_encountered_units = -1;
	last_num_encountered_terrains = -1;
}

bool section_is_referenced(const std::string &section_id, const config &cfg)
{
	if (const config &toplevel = cfg.child("toplevel"))
	{
		const std::vector<std::string> toplevel_refs
			= utils::quoted_split(toplevel["sections"]);
		if (std::find(toplevel_refs.begin(), toplevel_refs.end(), section_id)
			!= toplevel_refs.end()) {
			return true;
		}
	}

	BOOST_FOREACH(const config &section, cfg.child_range("section"))
	{
		const std::vector<std::string> sections_refd
			= utils::quoted_split(section["sections"]);
		if (std::find(sections_refd.begin(), sections_refd.end(), section_id)
			!= sections_refd.end()) {
			return true;
		}
	}
	return false;
}

bool topic_is_referenced(const std::string &topic_id, const config &cfg)
{
	if (const config &toplevel = cfg.child("toplevel"))
	{
		const std::vector<std::string> toplevel_refs
			= utils::quoted_split(toplevel["topics"]);
		if (std::find(toplevel_refs.begin(), toplevel_refs.end(), topic_id)
			!= toplevel_refs.end()) {
			return true;
		}
	}

	BOOST_FOREACH(const config &section, cfg.child_range("section"))
	{
		const std::vector<std::string> topics_refd
			= utils::quoted_split(section["topics"]);
		if (std::find(topics_refd.begin(), topics_refd.end(), topic_id)
			!= topics_refd.end()) {
			return true;
		}
	}
	return false;
}

void parse_config_internal(const config *help_cfg, const config *section_cfg,
						   section &sec, int level)
{
	if (level > max_section_level) {
		std::cerr << "Maximum section depth has been reached. Maybe circular dependency?"
				  << std::endl;
	}
	else if (section_cfg != NULL) {
		const std::vector<std::string> sections = utils::quoted_split((*section_cfg)["sections"]);
		sec.level = level;
		std::string id = level == 0 ? "toplevel" : (*section_cfg)["id"].str();
		if (level != 0) {
			if (!is_valid_id(id)) {
				std::stringstream ss;
				ss << "Invalid ID, used for internal purpose: '" << id << "'";
				throw parse_error(ss.str());
			}
		}
		std::string title = level == 0 ? "" : (*section_cfg)["title"].str();
		sec.id = id;
		sec.title = title;
		std::vector<std::string>::const_iterator it;
		// Find all child sections.
		for (it = sections.begin(); it != sections.end(); ++it) {
			if (const config &child_cfg = help_cfg->find_child("section", "id", *it))
			{
				section child_section;
				parse_config_internal(help_cfg, &child_cfg, child_section, level + 1);
				sec.add_section(child_section);
			}
			else {
				std::stringstream ss;
				ss << "Help-section '" << *it << "' referenced from '"
				   << id << "' but could not be found.";
				throw parse_error(ss.str());
			}
		}

		generate_sections(help_cfg, (*section_cfg)["sections_generator"], sec, level);
		//TODO: harmonize topics/sections sorting
		if ((*section_cfg)["sort_sections"] == "yes") {
			std::sort(sec.sections.begin(),sec.sections.end(), section_less());
		}

		bool sort_topics = false;
		bool sort_generated = true;

		if ((*section_cfg)["sort_topics"] == "yes") {
		  sort_topics = true;
		  sort_generated = false;
		} else if ((*section_cfg)["sort_topics"] == "no") {
		  sort_topics = false;
		  sort_generated = false;
		} else if ((*section_cfg)["sort_topics"] == "generated") {
		  sort_topics = false;
		  sort_generated = true;
		} else if ((*section_cfg)["sort_topics"] != "") {
		  std::stringstream ss;
		  ss << "Invalid sort option: '" << (*section_cfg)["sort_topics"] << "'";
		  throw parse_error(ss.str());
		}

		std::vector<topic> generated_topics =
		generate_topics(sort_generated,(*section_cfg)["generator"]);

		const std::vector<std::string> topics_id = utils::quoted_split((*section_cfg)["topics"]);
		std::vector<topic> topics;

		// Find all topics in this section.
		for (it = topics_id.begin(); it != topics_id.end(); ++it) {
			if (const config &topic_cfg = help_cfg->find_child("topic", "id", *it))
			{
				std::string text = topic_cfg["text"];
				text += generate_topic_text(topic_cfg["generator"], help_cfg, sec, generated_topics);
				topic child_topic(topic_cfg["title"], topic_cfg["id"], text);
				if (!is_valid_id(child_topic.id)) {
					std::stringstream ss;
					ss << "Invalid ID, used for internal purpose: '" << id << "'";
					throw parse_error(ss.str());
				}
				topics.push_back(child_topic);
			}
			else {
				std::stringstream ss;
				ss << "Help-topic '" << *it << "' referenced from '" << id
				   << "' but could not be found." << std::endl;
				throw parse_error(ss.str());
			}
		}

		if (sort_topics) {
			std::sort(topics.begin(),topics.end(), title_less());
			std::sort(generated_topics.begin(),
			  generated_topics.end(), title_less());
			std::merge(generated_topics.begin(),
			  generated_topics.end(),topics.begin(),topics.end()
			  ,std::back_inserter(sec.topics),title_less());
		}
		else {
			sec.topics.insert(sec.topics.end(),
				topics.begin(), topics.end());
			sec.topics.insert(sec.topics.end(),
				generated_topics.begin(), generated_topics.end());
		}
	}
}

section parse_config(const config *cfg)
{
	section sec;
	if (cfg != NULL) {
		config const &toplevel_cfg = cfg->child("toplevel");
		parse_config_internal(cfg, toplevel_cfg ? &toplevel_cfg : NULL, sec);
	}
	return sec;
}

std::vector<topic> generate_topics(const bool sort_generated,const std::string &generator)
{
	std::vector<topic> res;
	if (generator == "") {
		return res;
	}

	if (generator == "abilities") {
		res = generate_ability_topics(sort_generated);
	} else if (generator == "weapon_specials") {
		res = generate_weapon_special_topics(sort_generated);
	} else if (generator == "factions") {
		res = generate_faction_topics(sort_generated);
	} else {
		std::vector<std::string> parts = utils::split(generator, ':', utils::STRIP_SPACES);
		if (parts[0] == "units" && parts.size()>1) {
			res = generate_unit_topics(sort_generated, parts[1]);
		}
	}

	return res;
}

void generate_sections(const config *help_cfg, const std::string &generator, section &sec, int level)
{
	if (generator == "races") {
		generate_races_sections(help_cfg, sec, level);
	} else if (generator == "terrains") {
		generate_terrain_sections(help_cfg, sec, level);
	} else 	{
		std::vector<std::string> parts = utils::split(generator, ':', utils::STRIP_SPACES);
		if (parts[0] == "units" && parts.size()>1) {
			generate_unit_sections(help_cfg, sec, level, true, parts[1]);
		}
	}
}

std::string generate_topic_text(const std::string &generator, const config *help_cfg, const section &sec, const std::vector<topic>& generated_topics)
{
	std::string empty_string = "";
	if (generator == "") {
		return empty_string;
	} else if (generator == "about") {
		return generate_about_text();
	} else {
		std::vector<std::string> parts = utils::split(generator, ':');
		if (parts.size()>1 && parts[0] == "contents") {
			if (parts[1] == "generated") {
				return generate_contents_links(sec, generated_topics);
			} else {
				return generate_contents_links(parts[1], help_cfg);
			}
		}
	}
	return empty_string;
}

topic_text::~topic_text()
{
	if (generator_ && --generator_->count == 0)
		delete generator_;
}

topic_text::topic_text(topic_text const &t): parsed_text_(t.parsed_text_), generator_(t.generator_)
{
	if (generator_)
		++generator_->count;
}

topic_text &topic_text::operator=(topic_generator *g)
{
	if (generator_ && --generator_->count == 0)
		delete generator_;
	generator_ = g;
	return *this;
}

const std::vector<std::string>& topic_text::parsed_text() const
{
	if (generator_) {
		parsed_text_ = parse_text((*generator_)());
		if (--generator_->count == 0)
			delete generator_;
		generator_ = NULL;
	}
	return parsed_text_;
}

std::vector<topic> generate_weapon_special_topics(const bool sort_generated)
{
	std::vector<topic> topics;

	std::map<t_string, std::string> special_description;
	std::map<t_string, std::set<std::string, string_less> > special_units;

	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		const unit_type &type = i.second;
		// Only show the weapon special if we find it on a unit that
		// detailed description should be shown about.
		if (description_type(type) != FULL_DESCRIPTION)
		 	continue;

		std::vector<attack_type> attacks = type.attacks();
		for (std::vector<attack_type>::const_iterator it = attacks.begin();
					it != attacks.end(); ++it) {

			std::vector<std::pair<t_string, t_string> > specials = it->special_tooltips();
			for ( size_t i = 0; i != specials.size(); ++i )
			{
				special_description.insert(std::make_pair(specials[i].first, specials[i].second));

				if (!type.hide_help()) {
					//add a link in the list of units having this special
					std::string type_name = type.type_name();
					//check for variations (walking corpse/soulless etc)
					const std::string section_prefix = type.variations().empty() ? "" : "..";
					std::string ref_id = section_prefix + unit_prefix + type.id();
					//we put the translated name at the beginning of the hyperlink,
					//so the automatic alphabetic sorting of std::set can use it
					std::string link = make_link(type_name, ref_id);
					special_units[specials[i].first].insert(link);
				}
			}
		}
	}

	for (std::map<t_string, std::string>::iterator s = special_description.begin();
			s != special_description.end(); ++s) {
		// use untranslated name to have universal topic id
		std::string id = "weaponspecial_" + s->first.base_str();
		std::stringstream text;
		text << s->second;
		text << "\n\n" << _("<header>text='Units having this special attack'</header>") << "\n";
		std::set<std::string, string_less> &units = special_units[s->first];
		for (std::set<std::string, string_less>::iterator u = units.begin(); u != units.end(); ++u) {
			text << (*u) << "\n";
		}

		topics.push_back( topic(s->first, id, text.str()) );
	}

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}


std::vector<topic> generate_ability_topics(const bool sort_generated)
{
	std::vector<topic> topics;
	std::map<t_string, std::string> ability_description;
	std::map<t_string, std::set<std::string, string_less> > ability_units;
	// Look through all the unit types, check if a unit of this type
	// should have a full description, if so, add this units abilities
	// for display. We do not want to show abilities that the user has
	// not encountered yet.
	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		const unit_type &type = i.second;
		if (description_type(type) == FULL_DESCRIPTION) {

			std::vector<t_string> const* abil_vecs[2];
			abil_vecs[0] = &type.abilities();
			abil_vecs[1] = &type.adv_abilities();

			std::vector<t_string> const* desc_vecs[2];
			desc_vecs[0] = &type.ability_tooltips();
			desc_vecs[1] = &type.adv_ability_tooltips();

			for(int i=0; i<2; ++i) {
				std::vector<t_string> const& abil_vec = *abil_vecs[i];
				std::vector<t_string> const& desc_vec = *desc_vecs[i];
				for(size_t j=0; j < abil_vec.size(); ++j) {
					t_string const& abil_name = abil_vec[j];
					std::string const abil_desc =
						j >= desc_vec.size() ? "" : desc_vec[j].str();

					ability_description.insert(std::make_pair(abil_name, abil_desc));

					if (!type.hide_help()) {
						//add a link in the list of units having this ability
						std::string type_name = type.type_name();
						std::string ref_id = unit_prefix +  type.id();
						//we put the translated name at the beginning of the hyperlink,
						//so the automatic alphabetic sorting of std::set can use it
						std::string link = make_link(type_name, ref_id);
						ability_units[abil_name].insert(link);
					}
				}
			}
		}
	}

	for (std::map<t_string, std::string>::iterator a = ability_description.begin(); a != ability_description.end(); ++a) {
		// we generate topic's id using the untranslated version of the ability's name
		std::string id = "ability_" + a->first.base_str();
		std::stringstream text;
		text << a->second;  //description
		text << "\n\n" << _("<header>text='Units having this ability'</header>") << "\n";
		std::set<std::string, string_less> &units = ability_units[a->first];
		for (std::set<std::string, string_less>::iterator u = units.begin(); u != units.end(); ++u) {
			text << (*u) << "\n";
		}

		topics.push_back( topic(a->first, id, text.str()) );
	}

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}

std::vector<topic> generate_faction_topics(const bool sort_generated)
{
	std::vector<topic> topics;
	const config& era = game_cfg->child("era");
	if (era) {
		std::vector<std::string> faction_links;
		BOOST_FOREACH(const config &f, era.child_range("multiplayer_side")) {
			const std::string& id = f["id"];
			if (id == "Random")
				continue;

			std::stringstream text;

			const config::attribute_value& description = f["description"];
			if (!description.empty()) {
				text << description.t_str() << "\n";
				text << "\n";
			}

			text << "<header>text='" << _("Leaders:") << "'</header>" << "\n";
			const std::vector<std::string> leaders =
					make_unit_links_list( utils::split(f["leader"]), true );
			BOOST_FOREACH(const std::string &link, leaders) {
				text << link << "\n";
			}

			text << "\n";

			text << "<header>text='" << _("Recruits:") << "'</header>" << "\n";
			const std::vector<std::string> recruits =
					make_unit_links_list( utils::split(f["recruit"]), true );
			BOOST_FOREACH(const std::string &link, recruits) {
				text << link << "\n";
			}

			const std::string name = f["name"];
			const std::string ref_id = faction_prefix + id;
			topics.push_back( topic(name, ref_id, text.str()) );
			faction_links.push_back(make_link(name, ref_id));
		}

		std::stringstream text;
		text << "<header>text='" << _("Era:") << " " << era["name"] << "'</header>" << "\n";
		text << "\n";
		const config::attribute_value& description = era["description"];
		if (!description.empty()) {
			text << description.t_str() << "\n";
			text << "\n";
		}

		text << "<header>text='" << _("Factions:") << "'</header>" << "\n";

		std::sort(faction_links.begin(), faction_links.end());
		BOOST_FOREACH(const std::string &link, faction_links) {
			text << link << "\n";
		}

		topics.push_back( topic(_("Factions"), "..factions_section", text.str()) );
	} else {
		topics.push_back( topic( _("Factions"), "..factions_section",
			_("Factions are only used in multiplayer")) );
	}

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());
	return topics;
}

class terrain_topic_generator: public topic_generator
{
	const terrain_type& type_;


public:
	terrain_topic_generator(const terrain_type& type) : type_(type) {}

	virtual std::string operator()() const {

		std::stringstream ss;

		if (!type_.icon_image().empty())
		ss << "<img>src='images/buttons/icon-base-32.png~RC(magenta>" << type_.id()
				<< ")~BLIT("<< "terrain/" << type_.icon_image() << "_30.png)" << "'</img> ";

		if (!type_.editor_image().empty())
			ss << "<img>src='" << type_.editor_image() << "'</img> ";

		ss << type_.help_topic_text().str() << "\n";

		if (!(type_.union_type().size() == 1 && type_.union_type()[0] == type_.number() && type_.is_nonnull())) {

			const t_translation::t_list& underlying_terrains = resources::game_map->underlying_mvt_terrain(type_.number());

			ss << "\n" << N_("Base Terrain: ");

			bool first = true;
			BOOST_FOREACH(const t_translation::t_terrain& underlying_terrain, underlying_terrains) {
				const terrain_type& mvt_base = resources::game_map->get_terrain_info(underlying_terrain);

				if (mvt_base.editor_name().empty()) continue;
				if (!first) ss << ",";
				else first = false;
				ss << make_link(mvt_base.editor_name(), ".." + terrain_prefix + mvt_base.id());
			}
		}

		if (game_config::debug) {

			ss << "\n";
			ss << "ID: "          << type_.id() << "\n";

			ss << "Village: "     << (type_.is_village()   ? "Yes" : "No") << "\n";
			ss << "Gives Healing: " << type_.gives_healing() << "\n";

			ss << "Keep: "        << (type_.is_keep()      ? "Yes" : "No") << "\n";
			ss << "Castle: "      << (type_.is_castle()    ? "Yes" : "No") << "\n";

			ss << "Overlay: "     << (type_.is_overlay()   ? "Yes" : "No") << "\n";
			ss << "Combined: "    << (type_.is_combined()  ? "Yes" : "No") << "\n";
			ss << "Nonnull: "     << (type_.is_nonnull()   ? "Yes" : "No") << "\n";

			ss << "Terrain string:"  << type_.number() << "\n";

			ss << "Hide in Editor: " << (type_.hide_in_editor() ? "Yes" : "No") << "\n";
			ss << "Hide Help: "      << (type_.hide_help() ? "Yes" : "No") << "\n";
			ss << "Editor Group: "   << type_.editor_group() << "\n";

			ss << "Light Bonus: "   << type_.light_bonus(0) << "\n";

			ss << type_.income_description();
		}

		return ss.str();
	}

};
class unit_topic_generator: public topic_generator
{
	const unit_type& type_;
	const std::string variation_;
	typedef std::pair< std::string, unsigned > item;
	void push_header(std::vector< item > &row, char const *name) const {
		row.push_back(item(bold(name), font::line_width(name, normal_font_size, TTF_STYLE_BOLD)));
	}
public:
	unit_topic_generator(const unit_type &t, std::string variation="") : type_(t), variation_(variation) {}
	virtual std::string operator()() const {
		// Force the lazy loading to build this unit.
		unit_types.build_unit_type(type_, unit_type::WITHOUT_ANIMATIONS);

		std::stringstream ss;
		std::string clear_stringstream;
		const std::string detailed_description = type_.unit_description();
		const unit_type& female_type = type_.get_gender_unit_type(unit_race::FEMALE);
		const unit_type& male_type = type_.get_gender_unit_type(unit_race::MALE);

		// Show the unit's image and its level.
#ifdef LOW_MEM
		ss << "<img>src='" << male_type.image() << "'</img> ";
#else
		ss << "<img>src='" << male_type.image() << "~RC(" << male_type.flag_rgb() << ">1)" << "'</img> ";
#endif

		if (&female_type != &male_type)
#ifdef LOW_MEM
			ss << "<img>src='" << female_type.image() << "'</img> ";
#else
			ss << "<img>src='" << female_type.image() << "~RC(" << female_type.flag_rgb() << ">1)" << "'</img> ";
#endif


		ss << "<format>font_size=" << font::relative_size(11) << " text=' " << escape(_("level"))
		   << " " << type_.level() << "'</format>";

		const std::string &male_portrait = male_type.small_profile();
		const std::string &female_portrait = female_type.small_profile();

		if (male_portrait.empty() == false && male_portrait != male_type.image()) {
			ss << "<img>src='" << male_portrait << "~BG()' align='right'</img> ";
		}

		if (female_portrait.empty() == false && female_portrait != male_portrait && female_portrait != female_type.image()) {
			ss << "<img>src='" << female_portrait << "~BG()' align='right'</img> ";
		}

		ss << "\n";

		// Print cross-references to units that this unit advances from/to.
		// Cross reference to the topics containing information about those units.
		const bool first_reverse_value = true;
		bool reverse = first_reverse_value;
		if (variation_.empty()) {
			do {
				std::vector<std::string> adv_units =
					reverse ? type_.advances_from() : type_.advances_to();
				bool first = true;

				BOOST_FOREACH(const std::string &adv, adv_units)
				{
					const unit_type *type = unit_types.find(adv, unit_type::HELP_INDEXED);
					if (!type || type->hide_help()) continue;

					if (first) {
						if (reverse)
							ss << _("Advances from: ");
						else
							ss << _("Advances to: ");
						first = false;
					} else
						ss << ", ";

					std::string lang_unit = type->type_name();
					std::string ref_id;
					if (description_type(*type) == FULL_DESCRIPTION) {
						const std::string section_prefix = type->variations().empty() ? "" : "..";
						ref_id = section_prefix + unit_prefix + type->id();
					} else {
						ref_id = unknown_unit_topic;
						lang_unit += " (?)";
					}
					ss << make_link(lang_unit, ref_id);
				}
				if (!first) ss << "\n";

				reverse = !reverse; //switch direction
			} while(reverse != first_reverse_value); // don't restart
		}

		const unit_type* parent = variation_.empty() ? &type_ :
				unit_types.find(type_.id(), unit_type::HELP_INDEXED);
		if (!variation_.empty()) {
			ss << _("Base unit: ") << make_link(parent->type_name(), ".." + unit_prefix + type_.id()) << "\n";
		} else {
			bool first = true;
			BOOST_FOREACH(const std::string& base_id, utils::split(type_.get_cfg()["base_ids"])) {
				if (first) {
					ss << _("Base units: ");
					first = false;
				}
				const unit_type* base_type = unit_types.find(base_id, unit_type::HELP_INDEXED);
				const std::string section_prefix = base_type->variations().empty() ? "" : "..";
				ss << make_link(base_type->type_name(), section_prefix + unit_prefix + base_id) << "\n";
			}
		}

		bool first = true;
		BOOST_FOREACH(const std::string &var_id, parent->variations()) {
			const unit_type &type = parent->get_variation(var_id);

			if(type.hide_help()) {
				continue;
			}

			if (first) {
				ss << _("Variations: ");
				first = false;
			} else
				ss << ", ";

			std::string ref_id;

			std::string var_name = type.variation_name();
			if (description_type(type) == FULL_DESCRIPTION) {
				ref_id = variation_prefix + type.id() + "_" + var_id;
			} else {
				ref_id = unknown_unit_topic;
				var_name += " (?)";
			}

			ss << make_link(var_name, ref_id);
		}
		ss << "\n"; //added even if empty, to avoid shifting

		// Print the race of the unit, cross-reference it to the
		// respective topic.
		const std::string race_id = type_.race_id();
		std::string race_name = type_.race()->plural_name();
		if ( race_name.empty() ) {
			race_name = _ ("race^Miscellaneous");
		}
		ss << _("Race: ");
		ss << make_link(race_name, "..race_" + race_id);
		ss << "\n";

		// Print the abilities the units has, cross-reference them
		// to their respective topics.
		if (!type_.abilities().empty()) {
			ss << _("Abilities: ");
			for(std::vector<t_string>::const_iterator ability_it = type_.abilities().begin(),
				 ability_end = type_.abilities().end();
				 ability_it != ability_end; ++ability_it) {
				const std::string ref_id = "ability_" + ability_it->base_str();
				std::string lang_ability = gettext(ability_it->c_str());
				ss << make_link(lang_ability, ref_id);
				if (ability_it + 1 != ability_end)
					ss << ", ";
			}
			ss << "\n";
		}

		// Print the extra AMLA upgrade abilities, cross-reference them
		// to their respective topics.
		if (!type_.adv_abilities().empty()) {
			ss << _("Ability Upgrades: ");
			for(std::vector<t_string>::const_iterator ability_it = type_.adv_abilities().begin(),
				 ability_end = type_.adv_abilities().end();
				 ability_it != ability_end; ++ability_it) {
				const std::string ref_id = "ability_" + ability_it->base_str();
				std::string lang_ability = gettext(ability_it->c_str());
				ss << make_link(lang_ability, ref_id);
				if (ability_it + 1 != ability_end)
					ss << ", ";
			}
			ss << "\n";
		}
		ss << "\n";

		// Print some basic information such as HP and movement points.
		ss << _("HP: ") << type_.hitpoints() << jump(30)
		   << _("Moves: ") << type_.movement() << jump(30);
		if (type_.vision() != type_.movement())
			ss << _("Vision: ") << type_.vision() << jump(30);
		if (type_.jamming() > 0)
			ss << _("Jamming: ") << type_.jamming() << jump(30);
		ss << _("Cost: ") << type_.cost() << jump(30)
		   << _("Alignment: ")
		   << make_link(type_.alignment_description(type_.alignment(), type_.genders().front()), "time_of_day")
		   << jump(30);
		if (type_.can_advance())
			ss << _("Required XP: ") << type_.experience_needed();

		// Print the detailed description about the unit.
		ss << "\n\n" << detailed_description;

		// Print the different attacks a unit has, if it has any.
		std::vector<attack_type> attacks = type_.attacks();
		if (!attacks.empty()) {
			// Print headers for the table.
			ss << "\n\n<header>text='" << escape(_("unit help^Attacks"))
			   << "'</header>\n\n";
			table_spec table;

			std::vector<item> first_row;
			// Dummy element, icons are below.
			first_row.push_back(item("", 0));
			push_header(first_row, _("unit help^Name"));
			push_header(first_row, _("Type"));
			push_header(first_row, _("Strikes"));
			push_header(first_row, _("Range"));
			push_header(first_row, _("Special"));
			table.push_back(first_row);
			// Print information about every attack.
			for(std::vector<attack_type>::const_iterator attack_it = attacks.begin(),
				 attack_end = attacks.end();
				 attack_it != attack_end; ++attack_it) {
				std::string lang_weapon = attack_it->name();
				std::string lang_type = string_table["type_" + attack_it->type()];
				std::vector<item> row;
				std::stringstream attack_ss;
				attack_ss << "<img>src='" << (*attack_it).icon() << "'</img>";
				row.push_back(std::make_pair(attack_ss.str(),
							     image_width(attack_it->icon())));
				push_tab_pair(row, lang_weapon);
				push_tab_pair(row, lang_type);
				attack_ss.str(clear_stringstream);
				attack_ss << attack_it->damage() << utils::unicode_en_dash << attack_it->num_attacks() << " " << attack_it->accuracy_parry_description();
				push_tab_pair(row, attack_ss.str());
				attack_ss.str(clear_stringstream);
				if ((*attack_it).min_range() > 1 || (*attack_it).max_range() > 1)
					attack_ss << (*attack_it).min_range() << "-" << (*attack_it).max_range() << ' ';
				attack_ss << string_table["range_" + (*attack_it).range()];
				push_tab_pair(row, attack_ss.str());
				attack_ss.str(clear_stringstream);
				// Show this attack's special, if it has any. Cross
				// reference it to the section describing the
				// special.
				std::vector<std::pair<t_string, t_string> > specials = attack_it->special_tooltips();
				if(!specials.empty())
				{
					std::string lang_special = "";
					const size_t specials_size = specials.size();
					for ( size_t i = 0; i != specials_size; ++i) {
						const std::string ref_id = std::string("weaponspecial_")
							+ specials[i].first.base_str();
						lang_special = (specials[i].first);
						attack_ss << make_link(lang_special, ref_id);
						if ( i+1 != specials_size )
							attack_ss << ", "; //comma placed before next special
					}
					row.push_back(std::make_pair(attack_ss.str(),
						font::line_width(lang_special, normal_font_size)));
				}
				table.push_back(row);
			}
			ss << generate_table(table);
		}

		// Print the resistance table of the unit.
		ss << "\n\n<header>text='" << escape(_("Resistances"))
		   << "'</header>\n\n";
		table_spec resistance_table;
		std::vector<item> first_res_row;
		push_header(first_res_row, _("Attack Type"));
		push_header(first_res_row, _("Resistance"));
		resistance_table.push_back(first_res_row);
		const movetype &movement_type = type_.movement_type();
		utils::string_map dam_tab = movement_type.damage_table();
		for(utils::string_map::const_iterator dam_it = dam_tab.begin(), dam_end = dam_tab.end();
			 dam_it != dam_end; ++dam_it) {
			std::vector<item> row;
			int resistance = 100 - atoi((*dam_it).second.c_str());
			char resi[16];
			snprintf(resi,sizeof(resi),"% 4d%%",resistance);	// range: -100% .. +70%
			std::string resist = resi;
			const size_t pos = resist.find('-');
			if (pos != std::string::npos)
				resist.replace(pos, 1, utils::unicode_minus);
			std::string color = unit_helper::resistance_color(resistance);
			std::string lang_weapon = string_table["type_" + dam_it->first];
			push_tab_pair(row, lang_weapon);
			std::stringstream str;
			str << "<format>color=" << color << " text='"<< resist << "'</format>";
			const std::string markup = str.str();
			str.str(clear_stringstream);
			str << resist;
			row.push_back(std::make_pair(markup,
						     font::line_width(str.str(), normal_font_size)));
			resistance_table.push_back(row);
		}
		ss << generate_table(resistance_table);

		if (resources::game_map != NULL) {
			// Print the terrain modifier table of the unit.
			ss << "\n\n<header>text='" << escape(_("Terrain Modifiers"))
			   << "'</header>\n\n";
			std::vector<item> first_row;
			table_spec table;
			push_header(first_row, _("Terrain"));
			push_header(first_row, _("Defense"));
			push_header(first_row, _("Movement Cost"));

			const bool has_vision = type_.movement_type().has_vision_data();
			if ( has_vision )
				push_header(first_row, _("Vision Cost"));
			const bool has_jamming = type_.movement_type().has_jamming_data();
			if ( has_jamming )
				push_header(first_row, _("Jamming Cost"));

			table.push_back(first_row);
			std::set<t_translation::t_terrain>::const_iterator terrain_it =
				preferences::encountered_terrains().begin();


			for (; terrain_it != preferences::encountered_terrains().end();
					++terrain_it) {
				const t_translation::t_terrain terrain = *terrain_it;
				if (terrain == t_translation::FOGGED || terrain == t_translation::VOID_TERRAIN || terrain == t_translation::OFF_MAP_USER)
					continue;
				const terrain_type& info = resources::game_map->get_terrain_info(terrain);

				if (info.union_type().size() == 1 && info.union_type()[0] == info.number() && info.is_nonnull()) {
					std::vector<item> row;
					const std::string& name = info.name();
					const std::string id = info.id();
					const int moves = movement_type.movement_cost(terrain);
					const int views = movement_type.vision_cost(terrain);
					const int jams  = movement_type.jamming_cost(terrain);

					bool high_res = false;
					const std::string tc_base = high_res ? "images/buttons/icon-base-32.png" : "images/buttons/icon-base-16.png";
					const std::string terrain_image = "icons/terrain/terrain_type_" + id + (high_res ? "_30.png" : ".png");

					const std::string final_image = tc_base + "~RC(magenta>" + id + ")~BLIT(" + terrain_image + ")";

					row.push_back(std::make_pair( "<img>src='" + final_image + "'</img> " +
							make_link(name, "..terrain_" + id),
							font::line_width(name, normal_font_size) + (high_res ? 32 : 16) ));

					//defense  -  range: +10 % .. +70 %
					const int defense =
							100 - movement_type.defense_modifier(terrain);
					std::string color;
					if (defense <= 10)
						color = "red";
					else if (defense <= 30)
						color = "yellow";
					else if (defense <= 50)
						color = "white";
					else
						color = "green";

					std::stringstream str;
					str << "<format>color=" << color << " text='"<< defense << "%'</format>";
					std::string markup = str.str();
					str.str(clear_stringstream);
					str << defense << "%";
					row.push_back(std::make_pair(markup,
							font::line_width(str.str(), normal_font_size)));

					//movement  -  range: 1 .. 5, movetype::UNREACHABLE=impassable
					str.str(clear_stringstream);
					const bool cannot_move = moves > type_.movement();
					if (cannot_move)		// cannot move in this terrain
						color = "red";
					else if (moves > 1)
						color = "yellow";
					else
						color = "white";
					str << "<format>color=" << color << " text='";
					// A 5 MP margin; if the movement costs go above
					// the unit's max moves + 5, we replace it with dashes.
					if(cannot_move && (moves > type_.movement() + 5)) {
						str << utils::unicode_figure_dash;
					} else {
						str << moves;
					}
					str << "'</format>";
					markup = str.str();
					str.str(clear_stringstream);
					str << moves;
					row.push_back(std::make_pair(markup,
							font::line_width(str.str(), normal_font_size)));

					//vision
					if ( has_vision ) {
						str.str(clear_stringstream);
						const bool cannot_view = views > type_.vision();
						if (cannot_view)		// cannot view in this terrain
							color = "red";
						else if ( views > moves )
							color = "yellow";
						else if ( views == moves )
							color = "white";
						else
							color = "green";
						str << "<format>color=" << color << " text='";
						// A 5 MP margin; if the vision costs go above
						// the unit's vision + 5, we replace it with dashes.
						if(cannot_view && (views > type_.vision() + 5)) {
							str << utils::unicode_figure_dash;
						} else {
							str << views;
						}
						str << "'</format>";
						markup = str.str();
						str.str(clear_stringstream);
						str << views;
						row.push_back(std::make_pair(markup,
								font::line_width(str.str(), normal_font_size)));
					}

					//jamming
					if ( has_jamming ) {
						str.str(clear_stringstream);
						const bool cannot_jam = jams > type_.jamming();
						if ( cannot_jam )		// cannot jamm in this terrain
							color = "red";
						else if ( jams > views )
							color = "yellow";
						else if ( jams == views )
							color = "white";
						else
							color = "green";
						str << "<format>color=" << color << " text='";
						// A 5 MP margin; if the jamming costs go above
						// the unit's jamming + 5, we replace it with dashes.
						if ( cannot_jam  &&  jams > type_.jamming() + 5 ) {
							str << utils::unicode_figure_dash;
						} else {
							str << jams;
						}
						str << "'</format>";

						push_tab_pair(row, str.str());
					}

					table.push_back(row);
				}
			}

			ss << generate_table(table);
		}
		return ss.str();
	}
};

std::string make_unit_link(const std::string& type_id)
{
	std::string link;

	const unit_type *type = unit_types.find(type_id, unit_type::HELP_INDEXED);
	if (!type) {
		std::cerr << "Unknown unit type : " << type_id << "\n";
		// don't return an hyperlink (no page)
		// instead show the id (as hint)
		link = type_id;
	} else if (!type->hide_help()) {
		std::string name = type->type_name();
		std::string ref_id;
		if (description_type(*type) == FULL_DESCRIPTION) {
			const std::string section_prefix = type->variations().empty() ? "" : "..";
			ref_id = section_prefix + unit_prefix + type->id();
		} else {
			ref_id = unknown_unit_topic;
			name += " (?)";
		}
		link =  make_link(name, ref_id);
	} // if hide_help then link is an empty string

	return link;
}

std::vector<std::string> make_unit_links_list(const std::vector<std::string>& type_id_list, bool ordered)
{
	std::vector<std::string> links_list;
	BOOST_FOREACH(const std::string &type_id, type_id_list) {
		std::string unit_link = make_unit_link(type_id);
		if (!unit_link.empty())
			links_list.push_back(unit_link);
	}

	if (ordered)
		std::sort(links_list.begin(), links_list.end());

	return links_list;
}

void generate_races_sections(const config *help_cfg, section &sec, int level)
{
	std::set<std::string, string_less> races;
	std::set<std::string, string_less> visible_races;

	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		const unit_type &type = i.second;
		UNIT_DESCRIPTION_TYPE desc_type = description_type(type);
		if (desc_type == FULL_DESCRIPTION) {
			races.insert(type.race_id());
			if (!type.hide_help())
				visible_races.insert(type.race_id());
		}
	}

	for(std::set<std::string, string_less>::iterator it = races.begin(); it != races.end(); ++it) {
		section race_section;
		config section_cfg;

		bool hidden = (visible_races.count(*it) == 0);

		section_cfg["id"] = hidden_symbol(hidden) + race_prefix + *it;

		std::string title;
		if (const unit_race *r = unit_types.find_race(*it)) {
			title = r->plural_name();
		} else {
			title = _ ("race^Miscellaneous");
		}
		section_cfg["title"] = title;

		section_cfg["sections_generator"] = "units:" + *it;
		section_cfg["generator"] = "units:" + *it;

		parse_config_internal(help_cfg, &section_cfg, race_section, level+1);
		sec.add_section(race_section);
	}
}

void generate_terrain_sections(const config* /*help_cfg*/, section& sec, int /*level*/)
{
	if (resources::game_map == NULL) return;

	std::map<std::string, section> base_map;

	const t_translation::t_list& t_listi = resources::game_map->get_terrain_list();

	BOOST_FOREACH(const t_translation::t_terrain& t, t_listi) {

		const terrain_type& info = resources::game_map->get_terrain_info(t);

		bool hidden = info.is_combined() || info.hide_help();

		if (preferences::encountered_terrains().find(t)
				== preferences::encountered_terrains().end() && !info.is_overlay())
			hidden = true;

		topic terrain_topic;
		terrain_topic.title = info.editor_name();
		terrain_topic.id    = hidden_symbol(hidden) + terrain_prefix + info.id();
		terrain_topic.text  = new terrain_topic_generator(info);

		t_translation::t_list base_terrains = resources::game_map->underlying_union_terrain(t);
		BOOST_FOREACH(const t_translation::t_terrain& base, base_terrains) {

			const terrain_type& base_info = resources::game_map->get_terrain_info(base);

			if (!base_info.is_nonnull() || base_info.hide_help())
				continue;

			section& base_section = base_map[base_info.id()];

			base_section.id = terrain_prefix + base_info.id();
			base_section.title = base_info.editor_name();

			if (base_info.id() == info.id())
				terrain_topic.id = ".." + terrain_prefix + info.id();
			base_section.topics.push_back(terrain_topic);
		}
	}

	for (std::map<std::string, section>::const_iterator it = base_map.begin(); it != base_map.end(); ++it) {
		sec.add_section(it->second);
	}
}

void generate_unit_sections(const config* /*help_cfg*/, section& sec, int level, const bool /*sort_generated*/, const std::string& race)
{
	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types()) {
		const unit_type &type = i.second;

		if (type.race_id() != race)
			continue;

		if (type.variations().empty())
			continue;

		section base_unit;
		BOOST_FOREACH(const std::string &variation_id, type.variations()) {
			// TODO: Do we apply encountered stuff to variations?
			const unit_type &var_type = type.get_variation(variation_id);
			const std::string topic_name = var_type.type_name() + "\n" + var_type.variation_name();
			const std::string var_ref = hidden_symbol(var_type.hide_help()) + variation_prefix + var_type.id() + "_" + variation_id;

			topic var_topic(topic_name, var_ref, "");
			var_topic.text = new unit_topic_generator(var_type, variation_id);
			base_unit.topics.push_back(var_topic);
		}

		const std::string type_name = type.type_name();
		const std::string ref_id = hidden_symbol(type.hide_help()) + unit_prefix +  type.id();

		base_unit.id = ref_id;
		base_unit.title = type_name;
		base_unit.level = level +1;

		sec.add_section(base_unit);
	}
}

std::vector<topic> generate_unit_topics(const bool sort_generated, const std::string& race)
{
	std::vector<topic> topics;
	std::set<std::string, string_less> race_units;
	std::set<std::string, string_less> race_topics;

	BOOST_FOREACH(const unit_type_data::unit_type_map::value_type &i, unit_types.types())
	{
		const unit_type &type = i.second;

		if (type.race_id() != race)
			continue;

		UNIT_DESCRIPTION_TYPE desc_type = description_type(type);
		if (desc_type != FULL_DESCRIPTION)
			continue;

		const std::string type_name = type.type_name();
		const std::string real_prefix = type.variations().empty() ? "" : "..";
		const std::string ref_id = hidden_symbol(type.hide_help()) + real_prefix + unit_prefix +  type.id();
		topic unit_topic(type_name, ref_id, "");
		unit_topic.text = new unit_topic_generator(type);
		topics.push_back(unit_topic);

		if (!type.hide_help()) {
			// we also record an hyperlink of this unit
			// in the list used for the race topic
			std::string link = make_link(type_name, ref_id);
			race_units.insert(link);
		}
	}

	//generate the hidden race description topic
	std::string race_id = "..race_"+race;
	std::string race_name;
	std::string race_description;
	if (const unit_race *r = unit_types.find_race(race)) {
		race_name = r->plural_name();
		race_description = r->description();
		// if (description.empty()) description =  _("No description Available");
		BOOST_FOREACH(const config &additional_topic, r->additional_topics())
		  {
		    std::string id = additional_topic["id"];
		    std::string title = additional_topic["title"];
		    std::string text = additional_topic["text"];
		    //topic additional_topic(title, id, text);
		    topics.push_back(topic(title,id,text));
			std::string link = make_link(title, id);
			race_topics.insert(link);
		  }
	} else {
		race_name = _ ("race^Miscellaneous");
		// description =  _("Here put the description of the Miscellaneous race");
	}

	std::stringstream text;
	text << race_description;
	text << "\n\n" << _("<header>text='Units of this race'</header>") << "\n";
	for (std::set<std::string, string_less>::iterator u = race_units.begin(); u != race_units.end(); ++u) {
		text << (*u) << "\n";
	}

	topics.push_back(topic(race_name, race_id, text.str()) );

	if (sort_generated)
		std::sort(topics.begin(), topics.end(), title_less());

	return topics;
}

UNIT_DESCRIPTION_TYPE description_type(const unit_type &type)
{
	if (game_config::debug || preferences::show_all_units_in_help()	||
			hotkey::is_scope_active(hotkey::SCOPE_EDITOR) ) {
		return FULL_DESCRIPTION;
	}

	const std::set<std::string> &encountered_units = preferences::encountered_units();
	if (encountered_units.find(type.id()) != encountered_units.end()) {
		return FULL_DESCRIPTION;
	}
	return NO_DESCRIPTION;
}

std::string generate_about_text()
{
	std::vector<std::string> about_lines = about::get_text();
	std::vector<std::string> res_lines;
	std::transform(about_lines.begin(), about_lines.end(), std::back_inserter(res_lines),
				   about_text_formatter());
	res_lines.erase(std::remove(res_lines.begin(), res_lines.end(), ""), res_lines.end());
	std::string text = utils::join(res_lines, "\n");
	return text;
}

std::string generate_contents_links(const std::string& section_name, config const *help_cfg)
{
	config const &section_cfg = help_cfg->find_child("section", "id", section_name);
	if (!section_cfg) {
		return std::string();
	}

	std::ostringstream res;

		std::vector<std::string> topics = utils::quoted_split(section_cfg["topics"]);

		// we use an intermediate structure to allow a conditional sorting
		typedef std::pair<std::string,std::string> link;
		std::vector<link> topics_links;

		std::vector<std::string>::iterator t;
		// Find all topics in this section.
		for (t = topics.begin(); t != topics.end(); ++t) {
			if (config const &topic_cfg = help_cfg->find_child("topic", "id", *t)) {
				std::string id = topic_cfg["id"];
				if (is_visible_id(id))
					topics_links.push_back(link(topic_cfg["title"], id));
			}
		}

		if (section_cfg["sort_topics"] == "yes") {
			std::sort(topics_links.begin(),topics_links.end());
		}

		std::vector<link>::iterator l;
		for (l = topics_links.begin(); l != topics_links.end(); ++l) {
			std::string link = make_link(l->first, l->second);
			res << link << "\n";
		}

		return res.str();
}

std::string generate_contents_links(const section &sec, const std::vector<topic>& topics)
{
		std::stringstream res;

		section_list::const_iterator s;
		for (s = sec.sections.begin(); s != sec.sections.end(); ++s) {
			if (is_visible_id((*s)->id)) {
				std::string link = make_link((*s)->title, ".."+(*s)->id);
				res << link << "\n";
			}
		}

		std::vector<topic>::const_iterator t;
		for (t = topics.begin(); t != topics.end(); ++t) {
			if (is_visible_id(t->id)) {
				std::string link = make_link(t->title, t->id);
				res << link << "\n";
			}
		}
		return res.str();
}

bool topic::operator==(const topic &t) const
{
	return t.id == id;
}

bool topic::operator<(const topic &t) const
{
	return id < t.id;
}

section::~section()
{
	std::for_each(sections.begin(), sections.end(), delete_section());
}

section::section(const section &sec) :
	title(sec.title),
	id(sec.id),
	topics(sec.topics),
	sections(),
	level(sec.level)
{
	std::transform(sec.sections.begin(), sec.sections.end(),
				   std::back_inserter(sections), create_section());
}

section& section::operator=(const section &sec)
{
	title = sec.title;
	id = sec.id;
	level = sec.level;
	topics.insert(topics.end(), sec.topics.begin(), sec.topics.end());
	std::transform(sec.sections.begin(), sec.sections.end(),
				   std::back_inserter(sections), create_section());
	return *this;
}


bool section::operator==(const section &sec) const
{
	return sec.id == id;
}

bool section::operator<(const section &sec) const
{
	return id < sec.id;
}

void section::add_section(const section &s)
{
	sections.push_back(new section(s));
}

void section::clear()
{
	topics.clear();
	std::for_each(sections.begin(), sections.end(), delete_section());
	sections.clear();
}

help_menu::help_menu(CVideo &video, section const &toplevel, int max_height) :
	gui::menu(video, empty_string_vector, true, max_height, -1, NULL, &gui::menu::bluebg_style),
	visible_items_(),
	toplevel_(toplevel),
	expanded_(),
	restorer_(),
	chosen_topic_(NULL),
	selected_item_(&toplevel, "")
{
	silent_ = true; //silence the default menu sounds
	update_visible_items(toplevel_);
	display_visible_items();
	if (!visible_items_.empty())
		selected_item_ = visible_items_.front();
}

bool help_menu::expanded(const section &sec)
{
	return expanded_.find(&sec) != expanded_.end();
}

void help_menu::expand(const section &sec)
{
	if (sec.id != "toplevel" && expanded_.insert(&sec).second) {
		sound::play_UI_sound(game_config::sounds::menu_expand);
	}
}

void help_menu::contract(const section &sec)
{
	if (expanded_.erase(&sec)) {
		sound::play_UI_sound(game_config::sounds::menu_contract);
	}
}

void help_menu::update_visible_items(const section &sec, unsigned level)
{
	if (level == 0) {
		// Clear if this is the top level, otherwise append items.
		visible_items_.clear();
	}
	section_list::const_iterator sec_it;
	for (sec_it = sec.sections.begin(); sec_it != sec.sections.end(); ++sec_it) {
		if (is_visible_id((*sec_it)->id)) {
			const std::string vis_string = get_string_to_show(*(*sec_it), level + 1);
			visible_items_.push_back(visible_item(*sec_it, vis_string));
			if (expanded(*(*sec_it))) {
				update_visible_items(*(*sec_it), level + 1);
			}
		}
	}
	topic_list::const_iterator topic_it;
	for (topic_it = sec.topics.begin(); topic_it != sec.topics.end(); ++topic_it) {
		if (is_visible_id(topic_it->id)) {
			const std::string vis_string = get_string_to_show(*topic_it, level + 1);
			visible_items_.push_back(visible_item(&(*topic_it), vis_string));
		}
	}
}

std::string help_menu::indented_icon(const std::string& icon, const unsigned level) {
	std::stringstream to_show;
	for (unsigned i = 1; i < level; ++i) 	{
		to_show << IMAGE_PREFIX << indentation_img << IMG_TEXT_SEPARATOR;
	}

	to_show << IMAGE_PREFIX << icon;
	return to_show.str();
}

std::string help_menu::get_string_to_show(const section &sec, const unsigned level)
{
	std::stringstream to_show;
	to_show << indented_icon(expanded(sec) ? open_section_img : closed_section_img, level)
		 << IMG_TEXT_SEPARATOR << sec.title;
	return to_show.str();
}

std::string help_menu::get_string_to_show(const topic &topic, const unsigned level)
{
	std::stringstream to_show;
	to_show <<  indented_icon(topic_img, level)
		<< IMG_TEXT_SEPARATOR << topic.title;
	return to_show.str();
}

bool help_menu::select_topic_internal(const topic &t, const section &sec)
{
	topic_list::const_iterator tit =
		std::find(sec.topics.begin(), sec.topics.end(), t);
	if (tit != sec.topics.end()) {
		// topic starting with ".." are assumed as rooted in the parent section
		// and so only expand the parent when selected
		if (t.id.size()<2 || t.id[0] != '.' || t.id[1] != '.')
			expand(sec);
		return true;
	}
	section_list::const_iterator sit;
	for (sit = sec.sections.begin(); sit != sec.sections.end(); ++sit) {
		if (select_topic_internal(t, *(*sit))) {
			expand(sec);
			return true;
		}
	}
	return false;
}

void help_menu::select_topic(const topic &t)
{
	if (selected_item_ == t) {
		// The requested topic is already selected.
		return;
	}
	if (select_topic_internal(t, toplevel_)) {
		update_visible_items(toplevel_);
		for (std::vector<visible_item>::const_iterator it = visible_items_.begin();
			 it != visible_items_.end(); ++it) {
			if (*it == t) {
				selected_item_ = *it;
				break;
			}
		}
		display_visible_items();
	}
}

int help_menu::process()
{
	int res = menu::process();
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);

	if (!visible_items_.empty() &&
            static_cast<size_t>(res) < visible_items_.size()) {

		selected_item_ = visible_items_[res];
		const section* sec = selected_item_.sec;
		if (sec != NULL) {
			// Check how we click on the section
			int x = mousex - menu::location().x;

			const std::string icon_img = expanded(*sec) ? open_section_img : closed_section_img;
			// we remove the right thickness (ne present between icon and text)
			int text_start = style_->item_size(indented_icon(icon_img, sec->level)).w - style_->get_thickness();

			// NOTE: if you want to forbid click to the left of the icon
			// also check x >= text_start-image_width(icon_img)
			if (menu::double_clicked() || x < text_start) {
				// Open or close a section if we double-click on it
				// or do simple click on the icon.
				expanded(*sec) ? contract(*sec) : expand(*sec);
				update_visible_items(toplevel_);
				display_visible_items();
			} else if (x >= text_start){
				// click on title open the topic associated to this section
				chosen_topic_ = find_topic(toplevel, ".."+sec->id );
			}
		} else if (selected_item_.t != NULL) {
			/// Choose a topic if it is clicked.
			chosen_topic_ = selected_item_.t;
		}
	}
	return res;
}

const topic *help_menu::chosen_topic()
{
	const topic *ret = chosen_topic_;
	chosen_topic_ = NULL;
	return ret;
}

void help_menu::display_visible_items()
{
	std::vector<std::string> menu_items;
	for(std::vector<visible_item>::const_iterator items_it = visible_items_.begin(),
		 end = visible_items_.end(); items_it != end; ++items_it) {
		std::string to_show = items_it->visible_string;
		if (selected_item_ == *items_it)
			to_show = std::string("*") + to_show;
		menu_items.push_back(to_show);
	}
	set_items(menu_items, false, true);
}

help_menu::visible_item::visible_item(const section *_sec, const std::string &vis_string) :
	t(NULL), sec(_sec), visible_string(vis_string) {}

help_menu::visible_item::visible_item(const topic *_t, const std::string &vis_string) :
	t(_t), sec(NULL), visible_string(vis_string) {}

bool help_menu::visible_item::operator==(const section &_sec) const
{
	return sec != NULL && *sec == _sec;
}

bool help_menu::visible_item::operator==(const topic &_t) const
{
	return t != NULL && *t == _t;
}

bool help_menu::visible_item::operator==(const visible_item &vis_item) const
{
	return t == vis_item.t && sec == vis_item.sec;
}

help_text_area::help_text_area(CVideo &video, const section &toplevel) :
	gui::scrollarea(video),
	items_(),
	last_row_(),
	toplevel_(toplevel),
	shown_topic_(NULL),
	title_spacing_(16),
	curr_loc_(0, 0),
	min_row_height_(font::get_max_height(normal_font_size)),
	curr_row_height_(min_row_height_),
	contents_height_(0)
{
	set_scroll_rate(40);
}

void help_text_area::set_inner_location(SDL_Rect const &rect)
{
	bg_register(rect);
	if (shown_topic_)
		set_items();
}

void help_text_area::show_topic(const topic &t)
{
	shown_topic_ = &t;
	set_items();
	set_dirty(true);
}


help_text_area::item::item(surface surface, int x, int y, const std::string& _text,
						   const std::string& reference_to, bool _floating,
						   bool _box, ALIGNMENT alignment) :
	rect(),
	surf(surface),
	text(_text),
	ref_to(reference_to),
	floating(_floating), box(_box),
	align(alignment)
{
	rect.x = x;
	rect.y = y;
	rect.w = box ? surface->w + box_width * 2 : surface->w;
	rect.h = box ? surface->h + box_width * 2 : surface->h;
}

help_text_area::item::item(surface surface, int x, int y, bool _floating,
						   bool _box, ALIGNMENT alignment) :
	rect(),
	surf(surface),
	text(""),
	ref_to(""),
	floating(_floating),
	box(_box), align(alignment)
{
	rect.x = x;
	rect.y = y;
	rect.w = box ? surface->w + box_width * 2 : surface->w;
	rect.h = box ? surface->h + box_width * 2 : surface->h;
}

void help_text_area::set_items()
{
	last_row_.clear();
	items_.clear();
	curr_loc_.first = 0;
	curr_loc_.second = 0;
	curr_row_height_ = min_row_height_;
	// Add the title item.
	const std::string show_title =
		font::make_text_ellipsis(shown_topic_->title, title_size, inner_location().w);
	surface surf(font::get_rendered_text(show_title, title_size,
					     font::NORMAL_COLOR, TTF_STYLE_BOLD));
	if (surf != NULL) {
		add_item(item(surf, 0, 0, show_title));
		curr_loc_.second = title_spacing_;
		contents_height_ = title_spacing_;
		down_one_line();
	}
	// Parse and add the text.
	std::vector<std::string> const &parsed_items = shown_topic_->text.parsed_text();
	std::vector<std::string>::const_iterator it;
	for (it = parsed_items.begin(); it != parsed_items.end(); ++it) {
		if (*it != "" && (*it)[0] == '[') {
			// Should be parsed as WML.
			try {
				config cfg;
				std::istringstream stream(*it);
				read(cfg, stream);

#define TRY(name) do { \
				if (config &child = cfg.child(#name)) \
					handle_##name##_cfg(child); \
				} while (0)

				TRY(ref);
				TRY(img);
				TRY(bold);
				TRY(italic);
				TRY(header);
				TRY(jump);
				TRY(format);

#undef TRY

			}
			catch (config::error& e) {
				std::stringstream msg;
				msg << "Error when parsing help markup as WML: '" << e.message << "'";
				throw parse_error(msg.str());
			}
		}
		else {
			add_text_item(*it);
		}
	}
	down_one_line(); // End the last line.
	int h = height();
	set_position(0);
	set_full_size(contents_height_);
	set_shown_size(h);
}

void help_text_area::handle_ref_cfg(const config &cfg)
{
	const std::string dst = cfg["dst"];
	const std::string text = cfg["text"];
	bool force = cfg["force"].to_bool();

	if (dst == "") {
		std::stringstream msg;
		msg << "Ref markup must have dst attribute. Please submit a bug"
		       " report if you have not modified the game files yourself. Erroneous config: ";
		write(msg, cfg);
		throw parse_error(msg.str());
	}

	if (find_topic(toplevel_, dst) == NULL && !force) {
		// detect the broken link but quietly silence the hyperlink for normal user
		add_text_item(text, game_config::debug ? dst : "", true);

		// FIXME: workaround: if different campaigns define different
		// terrains, some terrains available in one campaign will
		// appear in the list of seen terrains, and be displayed in the
		// help, even if the current campaign does not handle such
		// terrains. This will lead to the unit page generator creating
		// invalid references.
		//
		// Disabling this is a kludgey workaround until the
		// encountered_terrains system is fixed
		//
		// -- Ayin apr 8 2005
#if 0
		if (game_config::debug) {
			std::stringstream msg;
			msg << "Reference to non-existent topic '" << dst
			    << "'. Please submit a bug report if you have not"
			       "modified the game files yourself. Erroneous config: ";
			write(msg, cfg);
			throw parse_error(msg.str());
		}
#endif
	} else {
		add_text_item(text, dst);
	}
}

void help_text_area::handle_img_cfg(const config &cfg)
{
	const std::string src = cfg["src"];
	const std::string align = cfg["align"];
	bool floating = cfg["float"].to_bool();
	bool box = cfg["box"].to_bool(true);
	if (src == "") {
		throw parse_error("Img markup must have src attribute.");
	}
	add_img_item(src, align, floating, box);
}

void help_text_area::handle_bold_cfg(const config &cfg)
{
	const std::string text = cfg["text"];
	if (text == "") {
		throw parse_error("Bold markup must have text attribute.");
	}
	add_text_item(text, "", false, -1, true);
}

void help_text_area::handle_italic_cfg(const config &cfg)
{
	const std::string text = cfg["text"];
	if (text == "") {
		throw parse_error("Italic markup must have text attribute.");
	}
	add_text_item(text, "", false, -1, false, true);
}

void help_text_area::handle_header_cfg(const config &cfg)
{
	const std::string text = cfg["text"];
	if (text == "") {
		throw parse_error("Header markup must have text attribute.");
	}
	add_text_item(text, "", false, title2_size, true);
}

void help_text_area::handle_jump_cfg(const config &cfg)
{
	const std::string amount_str = cfg["amount"];
	const std::string to_str = cfg["to"];
	if (amount_str == "" && to_str == "") {
		throw parse_error("Jump markup must have either a to or an amount attribute.");
	}
	unsigned jump_to = curr_loc_.first;
	if (amount_str != "") {
		unsigned amount;
		try {
			amount = lexical_cast<unsigned, std::string>(amount_str);
		}
		catch (bad_lexical_cast) {
			throw parse_error("Invalid amount the amount attribute in jump markup.");
		}
		jump_to += amount;
	}
	if (to_str != "") {
		unsigned to;
		try {
			to = lexical_cast<unsigned, std::string>(to_str);
		}
		catch (bad_lexical_cast) {
			throw parse_error("Invalid amount in the to attribute in jump markup.");
		}
		if (to < jump_to) {
			down_one_line();
		}
		jump_to = to;
	}
	if (jump_to != 0 && static_cast<int>(jump_to) <
            get_max_x(curr_loc_.first, curr_row_height_)) {

		curr_loc_.first = jump_to;
	}
}

void help_text_area::handle_format_cfg(const config &cfg)
{
	const std::string text = cfg["text"];
	if (text == "") {
		throw parse_error("Format markup must have text attribute.");
	}
	bool bold = cfg["bold"].to_bool();
	bool italic = cfg["italic"].to_bool();
	int font_size = cfg["font_size"].to_int(normal_font_size);
	SDL_Color color = string_to_color(cfg["color"]);
	add_text_item(text, "", false, font_size, bold, italic, color);
}

void help_text_area::add_text_item(const std::string& text, const std::string& ref_dst,
								   bool broken_link, int _font_size, bool bold, bool italic,
								   SDL_Color text_color
)
{
	const int font_size = _font_size < 0 ? normal_font_size : _font_size;
	if (text.empty())
		return;
	const int remaining_width = get_remaining_width();
	size_t first_word_start = text.find_first_not_of(" ");
	if (first_word_start == std::string::npos) {
		first_word_start = 0;
	}
	if (text[first_word_start] == '\n') {
		down_one_line();
		std::string rest_text = text;
		rest_text.erase(0, first_word_start + 1);
		add_text_item(rest_text, ref_dst, broken_link, _font_size, bold, italic, text_color);
		return;
	}
	const std::string first_word = get_first_word(text);
	int state = ref_dst == "" ? 0 : TTF_STYLE_UNDERLINE;
	state |= bold ? TTF_STYLE_BOLD : 0;
	state |= italic ? TTF_STYLE_ITALIC : 0;
	if (curr_loc_.first != get_min_x(curr_loc_.second, curr_row_height_)
		&& remaining_width < font::line_width(first_word, font_size, state)) {
		// The first word does not fit, and we are not at the start of
		// the line. Move down.
		down_one_line();
		std::string s = remove_first_space(text);
		add_text_item(s, ref_dst, broken_link, _font_size, bold, italic, text_color);
	}
	else {
		std::vector<std::string> parts = split_in_width(text, font_size, remaining_width);
		std::string first_part = parts.front();
		// Always override the color if we have a cross reference.
		SDL_Color color;
		if(ref_dst.empty())
			color = text_color;
		else if(broken_link)
			color = font::BAD_COLOR;
		else
			color = font::YELLOW_COLOR;

		surface surf(font::get_rendered_text(first_part, font_size, color, state));
		if (!surf.null())
			add_item(item(surf, curr_loc_.first, curr_loc_.second, first_part, ref_dst));
		if (parts.size() > 1) {

			std::string& s = parts.back();

			const std::string first_word_before = get_first_word(s);
			const std::string first_word_after = get_first_word(remove_first_space(s));
			if (get_remaining_width() >= font::line_width(first_word_after, font_size, state)
				&& get_remaining_width()
				< font::line_width(first_word_before, font_size, state)) {
				// If the removal of the space made this word fit, we
				// must move down a line, otherwise it will be drawn
				// without a space at the end of the line.
				s = remove_first_space(s);
				down_one_line();
			}
			else if (!(font::line_width(first_word_before, font_size, state)
					   < get_remaining_width())) {
				s = remove_first_space(s);
			}
			add_text_item(s, ref_dst, broken_link, _font_size, bold, italic, text_color);

		}
	}
}

void help_text_area::add_img_item(const std::string& path, const std::string& alignment,
								  const bool floating, const bool box)
{
	surface surf(image::get_image(path));
	if (surf.null())
		return;
	ALIGNMENT align = str_to_align(alignment);
	if (align == HERE && floating) {
		WRN_DP << "Floating image with align HERE, aligning left.\n";
		align = LEFT;
	}
	const int width = surf->w + (box ? box_width * 2 : 0);
	int xpos;
	int ypos = curr_loc_.second;
	int text_width = inner_location().w;
	switch (align) {
	case HERE:
		xpos = curr_loc_.first;
		break;
	case LEFT:
	default:
		xpos = 0;
		break;
	case MIDDLE:
		xpos = text_width / 2 - width / 2 - (box ? box_width : 0);
		break;
	case RIGHT:
		xpos = text_width - width - (box ? box_width * 2 : 0);
		break;
	}
	if (curr_loc_.first != get_min_x(curr_loc_.second, curr_row_height_)
		&& (xpos < curr_loc_.first || xpos + width > text_width)) {
		down_one_line();
		add_img_item(path, alignment, floating, box);
	}
	else {
		if (!floating) {
			curr_loc_.first = xpos;
		}
		else {
			ypos = get_y_for_floating_img(width, xpos, ypos);
		}
		add_item(item(surf, xpos, ypos, floating, box, align));
	}
}

int help_text_area::get_y_for_floating_img(const int width, const int x, const int desired_y)
{
	int min_y = desired_y;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); ++it) {
		const item& itm = *it;
		if (itm.floating) {
			if ((itm.rect.x + itm.rect.w > x && itm.rect.x < x + width)
				|| (itm.rect.x > x && itm.rect.x < x + width)) {
				min_y = std::max<int>(min_y, itm.rect.y + itm.rect.h);
			}
		}
	}
	return min_y;
}

int help_text_area::get_min_x(const int y, const int height)
{
	int min_x = 0;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); ++it) {
		const item& itm = *it;
		if (itm.floating) {
			if (itm.rect.y < y + height && itm.rect.y + itm.rect.h > y && itm.align == LEFT) {
				min_x = std::max<int>(min_x, itm.rect.w + 5);
			}
		}
	}
	return min_x;
}

int help_text_area::get_max_x(const int y, const int height)
{
	int text_width = inner_location().w;
	int max_x = text_width;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); ++it) {
		const item& itm = *it;
		if (itm.floating) {
			if (itm.rect.y < y + height && itm.rect.y + itm.rect.h > y) {
				if (itm.align == RIGHT) {
					max_x = std::min<int>(max_x, text_width - itm.rect.w - 5);
				} else if (itm.align == MIDDLE) {
					max_x = std::min<int>(max_x, text_width / 2 - itm.rect.w / 2 - 5);
				}
			}
		}
	}
	return max_x;
}

void help_text_area::add_item(const item &itm)
{
	items_.push_back(itm);
	if (!itm.floating) {
		curr_loc_.first += itm.rect.w;
		curr_row_height_ = std::max<int>(itm.rect.h, curr_row_height_);
		contents_height_ = std::max<int>(contents_height_, curr_loc_.second + curr_row_height_);
		last_row_.push_back(&items_.back());
	}
	else {
		if (itm.align == LEFT) {
			curr_loc_.first = itm.rect.w + 5;
		}
		contents_height_ = std::max<int>(contents_height_, itm.rect.y + itm.rect.h);
	}
}


help_text_area::ALIGNMENT help_text_area::str_to_align(const std::string &cmp_str)
{
	if (cmp_str == "left") {
		return LEFT;
	} else if (cmp_str == "middle") {
		return MIDDLE;
	} else if (cmp_str == "right") {
		return RIGHT;
	} else if (cmp_str == "here" || cmp_str == "") { // Make the empty string be "here" alignment.
		return HERE;
	}
	std::stringstream msg;
	msg << "Invalid alignment string: '" << cmp_str << "'";
	throw parse_error(msg.str());
}

void help_text_area::down_one_line()
{
	adjust_last_row();
	last_row_.clear();
	curr_loc_.second += curr_row_height_ + (curr_row_height_ == min_row_height_ ? 0 : 2);
	curr_row_height_ = min_row_height_;
	contents_height_ = std::max<int>(curr_loc_.second + curr_row_height_, contents_height_);
	curr_loc_.first = get_min_x(curr_loc_.second, curr_row_height_);
}

void help_text_area::adjust_last_row()
{
	for (std::list<item *>::iterator it = last_row_.begin(); it != last_row_.end(); ++it) {
		item &itm = *(*it);
		const int gap = curr_row_height_ - itm.rect.h;
		itm.rect.y += gap / 2;
	}
}

int help_text_area::get_remaining_width()
{
	const int total_w = get_max_x(curr_loc_.second, curr_row_height_);
	return total_w - curr_loc_.first;
}

void help_text_area::draw_contents()
{
	SDL_Rect const &loc = inner_location();
	bg_restore();
	surface screen = video().getSurface();
	clip_rect_setter clip_rect_set(screen, &loc);
	for(std::list<item>::const_iterator it = items_.begin(), end = items_.end(); it != end; ++it) {
		SDL_Rect dst = it->rect;
		dst.y -= get_position();
		if (dst.y < static_cast<int>(loc.h) && dst.y + it->rect.h > 0) {
			dst.x += loc.x;
			dst.y += loc.y;
			if (it->box) {
				for (int i = 0; i < box_width; ++i) {
					draw_rectangle(dst.x, dst.y, it->rect.w - i * 2, it->rect.h - i * 2,
					                    0, screen);
					++dst.x;
					++dst.y;
				}
			}
			sdl_blit(it->surf, NULL, screen, &dst);
		}
	}
	update_rect(loc);
}

void help_text_area::scroll(unsigned int)
{
	// Nothing will be done on the actual scroll event. The scroll
	// position is checked when drawing instead and things drawn
	// accordingly.
	set_dirty(true);
}

bool help_text_area::item_at::operator()(const item& item) const {
	return point_in_rect(x_, y_, item.rect);
}

std::string help_text_area::ref_at(const int x, const int y)
{
	const int local_x = x - location().x;
	const int local_y = y - location().y;
	if (local_y < static_cast<int>(height()) && local_y > 0) {
		const int cmp_y = local_y + get_position();
		const std::list<item>::const_iterator it =
			std::find_if(items_.begin(), items_.end(), item_at(local_x, cmp_y));
		if (it != items_.end()) {
			if ((*it).ref_to != "") {
				return ((*it).ref_to);
			}
		}
	}
	return "";
}



help_browser::help_browser(display &disp, const section &toplevel) :
	gui::widget(disp.video()),
	disp_(disp),
	menu_(disp.video(),
	toplevel),
	text_area_(disp.video(), toplevel), toplevel_(toplevel),
	ref_cursor_(false),
	back_topics_(),
	forward_topics_(),
	back_button_(disp.video(), _(" < Back"), gui::button::TYPE_PRESS),
	forward_button_(disp.video(), _("Forward >"), gui::button::TYPE_PRESS),
	shown_topic_(NULL)
{
	// Hide the buttons at first since we do not have any forward or
	// back topics at this point. They will be unhidden when history
	// appears.
	back_button_.hide(true);
	forward_button_.hide(true);
	// Set sizes to some default values.
	set_measurements(font::relative_size(400), font::relative_size(500));
}

void help_browser::adjust_layout()
{
  const int menu_buttons_padding = font::relative_size(10);
	const int menu_y = location().y;
	const int menu_x = location().x;
	const int menu_w = 250;
	const int menu_h = height() - back_button_.height() - menu_buttons_padding;

	const int menu_text_area_padding = font::relative_size(10);
	const int text_area_y = location().y;
	const int text_area_x = menu_x + menu_w + menu_text_area_padding;
	const int text_area_w = width() - menu_w - menu_text_area_padding;
	const int text_area_h = height();

	const int button_border_padding = 0;
	const int button_button_padding = font::relative_size(10);
	const int back_button_x = location().x + button_border_padding;
	const int back_button_y = menu_y + menu_h + menu_buttons_padding;
	const int forward_button_x = back_button_x + back_button_.width() + button_button_padding;
	const int forward_button_y = back_button_y;

	menu_.set_width(menu_w);
	menu_.set_location(menu_x, menu_y);
	menu_.set_max_height(menu_h);
	menu_.set_max_width(menu_w);

	text_area_.set_location(text_area_x, text_area_y);
	text_area_.set_width(text_area_w);
	text_area_.set_height(text_area_h);

	back_button_.set_location(back_button_x, back_button_y);
	forward_button_.set_location(forward_button_x, forward_button_y);

	set_dirty(true);
}

void help_browser::update_location(SDL_Rect const &)
{
	adjust_layout();
}

void help_browser::process_event()
{
	CKey key;
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);

	/// Fake focus functionality for the menu, only process it if it has focus.
	if (point_in_rect(mousex, mousey, menu_.location())) {
		menu_.process();
		const topic *chosen_topic = menu_.chosen_topic();
		if (chosen_topic != NULL && chosen_topic != shown_topic_) {
			/// A new topic has been chosen in the menu, display it.
			show_topic(*chosen_topic);
		}
	}
	if (back_button_.pressed()) {
		move_in_history(back_topics_, forward_topics_);
	}
	if (forward_button_.pressed()) {
		move_in_history(forward_topics_, back_topics_);
	}
	back_button_.hide(back_topics_.empty());
	forward_button_.hide(forward_topics_.empty());
}

void help_browser::move_in_history(std::deque<const topic *> &from,
		std::deque<const topic *> &to)
{
	if (!from.empty()) {
		const topic *to_show = from.back();
		from.pop_back();
		if (shown_topic_ != NULL) {
			if (to.size() > max_history) {
				to.pop_front();
			}
			to.push_back(shown_topic_);
		}
		show_topic(*to_show, false);
	}
}


void help_browser::handle_event(const SDL_Event &event)
{
	SDL_MouseButtonEvent mouse_event = event.button;
	if (event.type == SDL_MOUSEBUTTONDOWN) {
		if (mouse_event.button == SDL_BUTTON_LEFT) {
			// Did the user click a cross-reference?
			const int mousex = mouse_event.x;
			const int mousey = mouse_event.y;
			const std::string ref = text_area_.ref_at(mousex, mousey);
			if (ref != "") {
				const topic *t = find_topic(toplevel_, ref);
				if (t == NULL) {
					std::stringstream msg;
					msg << _("Reference to unknown topic: ") << "'" << ref << "'.";
					gui2::show_transient_message(disp_.video(), "", msg.str());
					update_cursor();
				}
				else {
					show_topic(*t);
					update_cursor();
				}
			}
		}
	}
	else if (event.type == SDL_MOUSEMOTION) {
		update_cursor();
	}
}

void help_browser::update_cursor()
{
	int mousex, mousey;
	SDL_GetMouseState(&mousex,&mousey);
	const std::string ref = text_area_.ref_at(mousex, mousey);
	if (ref != "" && !ref_cursor_) {
		cursor::set(cursor::HYPERLINK);
		ref_cursor_ = true;
	}
	else if (ref == "" && ref_cursor_) {
		cursor::set(cursor::NORMAL);
		ref_cursor_ = false;
	}
}


const topic *find_topic(const section &sec, const std::string &id)
{
	topic_list::const_iterator tit =
		std::find_if(sec.topics.begin(), sec.topics.end(), has_id(id));
	if (tit != sec.topics.end()) {
		return &(*tit);
	}
	section_list::const_iterator sit;
	for (sit = sec.sections.begin(); sit != sec.sections.end(); ++sit) {
		const topic *t = find_topic(*(*sit), id);
		if (t != NULL) {
			return t;
		}
	}
	return NULL;
}

const section *find_section(const section &sec, const std::string &id)
{
	section_list::const_iterator sit =
		std::find_if(sec.sections.begin(), sec.sections.end(), has_id(id));
	if (sit != sec.sections.end()) {
		return *sit;
	}
	for (sit = sec.sections.begin(); sit != sec.sections.end(); ++sit) {
		const section *s = find_section(*(*sit), id);
		if (s != NULL) {
			return s;
		}
	}
	return NULL;
}

void help_browser::show_topic(const std::string &topic_id)
{
	const topic *t = find_topic(toplevel_, topic_id);

	if (t != NULL) {
		show_topic(*t);
	} else if (topic_id.find(unit_prefix)==0 || topic_id.find(hidden_symbol() + unit_prefix)==0) {
		show_topic(unknown_unit_topic);
	} else {
		std::cerr << "Help browser tried to show topic with id '" << topic_id
				  << "' but that topic could not be found." << std::endl;
	}
}

void help_browser::show_topic(const topic &t, bool save_in_history)
{
	log_scope("show_topic");

	if (save_in_history) {
		forward_topics_.clear();
		if (shown_topic_ != NULL) {
			if (back_topics_.size() > max_history) {
				back_topics_.pop_front();
			}
			back_topics_.push_back(shown_topic_);
		}
	}

	shown_topic_ = &t;
	text_area_.show_topic(t);
	menu_.select_topic(t);
	update_cursor();
}

std::vector<std::string> parse_text(const std::string &text)
{
	std::vector<std::string> res;
	bool last_char_escape = false;
	const char escape_char = '\\';
	std::stringstream ss;
	size_t pos;
	enum { ELEMENT_NAME, OTHER } state = OTHER;
	for (pos = 0; pos < text.size(); ++pos) {
		const char c = text[pos];
		if (c == escape_char && !last_char_escape) {
			last_char_escape = true;
		}
		else {
			if (state == OTHER) {
				if (c == '<') {
					if (last_char_escape) {
						ss << c;
					}
					else {
						res.push_back(ss.str());
						ss.str("");
						state = ELEMENT_NAME;
					}
				}
				else {
					ss << c;
				}
			}
			else if (state == ELEMENT_NAME) {
				if (c == '/') {
					std::string msg = "Erroneous / in element name.";
					throw parse_error(msg);
				}
				else if (c == '>') {
					// End of this name.
					std::stringstream s;
					const std::string element_name = ss.str();
					ss.str("");
					s << "</" << element_name << ">";
					const std::string end_element_name = s.str();
					size_t end_pos = text.find(end_element_name, pos);
					if (end_pos == std::string::npos) {
						std::stringstream msg;
						msg << "Unterminated element: " << element_name;
						throw parse_error(msg.str());
					}
					s.str("");
					const std::string contents = text.substr(pos + 1, end_pos - pos - 1);
					const std::string element = convert_to_wml(element_name, contents);
					res.push_back(element);
					pos = end_pos + end_element_name.size() - 1;
					state = OTHER;
				}
				else {
					ss << c;
				}
			}
			last_char_escape = false;
		}
	}
	if (state == ELEMENT_NAME) {
		std::stringstream msg;
		msg << "Element '" << ss.str() << "' continues through end of string.";
		throw parse_error(msg.str());
	}
	if (ss.str() != "") {
		// Add the last string.
		res.push_back(ss.str());
	}
	return res;
}

std::string convert_to_wml(const std::string &element_name, const std::string &contents)
{
	std::stringstream ss;
	bool in_quotes = false;
	bool last_char_escape = false;
	const char escape_char = '\\';
	std::vector<std::string> attributes;
	// Find the different attributes.
	// No checks are made for the equal sign or something like that.
	// Attributes are just separated by spaces or newlines.
	// Attributes that contain spaces must be in single quotes.
	for (size_t pos = 0; pos < contents.size(); ++pos) {
		const char c = contents[pos];
		if (c == escape_char && !last_char_escape) {
			last_char_escape = true;
		}
		else {
			if (c == '\'' && !last_char_escape) {
				ss << '"';
				in_quotes = !in_quotes;
			}
			else if ((c == ' ' || c == '\n') && !last_char_escape && !in_quotes) {
				// Space or newline, end of attribute.
				attributes.push_back(ss.str());
				ss.str("");
			}
			else {
				ss << c;
			}
			last_char_escape = false;
		}
	}
	if (in_quotes) {
		std::stringstream msg;
		msg << "Unterminated single quote after: '" << ss.str() << "'";
		throw parse_error(msg.str());
	}
	if (ss.str() != "") {
		attributes.push_back(ss.str());
	}
	ss.str("");
	// Create the WML.
	ss << "[" << element_name << "]\n";
	for (std::vector<std::string>::const_iterator it = attributes.begin();
		 it != attributes.end(); ++it) {
		ss << *it << "\n";
	}
	ss << "[/" << element_name << "]\n";
	return ss.str();
}

SDL_Color string_to_color(const std::string &cmp_str)
{
	if (cmp_str == "green") {
		return font::GOOD_COLOR;
	}
	if (cmp_str == "red") {
		return font::BAD_COLOR;
	}
	if (cmp_str == "black") {
		return font::BLACK_COLOR;
	}
	if (cmp_str == "yellow") {
		return font::YELLOW_COLOR;
	}
	if (cmp_str == "white") {
		return font::BIGMAP_COLOR;
	}
	return font::NORMAL_COLOR;
}

std::vector<std::string> split_in_width(const std::string &s, const int font_size,
		const unsigned width)
{
	std::vector<std::string> res;
	try {
	const std::string& first_line = font::word_wrap_text(s, font_size, width, -1, 1, true);
	res.push_back(first_line);
	if(s.size() > first_line.size()) {
		res.push_back(s.substr(first_line.size()));
	}
	}
	catch (utf8::invalid_utf8_exception&)
	{
		throw parse_error (_("corrupted original file"));
	}

	return res;
}

std::string remove_first_space(const std::string& text)
{
	if (text.length() > 0 && text[0] == ' ') {
		return text.substr(1);
	}
	return text;
}

std::string get_first_word(const std::string &s)
{
	size_t first_word_start = s.find_first_not_of(' ');
	if (first_word_start == std::string::npos) {
		return s;
	}
	size_t first_word_end = s.find_first_of(" \n", first_word_start);
	if( first_word_end == first_word_start ) {
		// This word is '\n'.
		first_word_end = first_word_start+1;
	}

	//if no gap(' ' or '\n') found, test if it is CJK character
	std::string re = s.substr(0, first_word_end);

	utf8::iterator ch(re);
	if (ch == utf8::iterator::end(re))
		return re;

	ucs4::char_t firstchar = *ch;
	if (font::is_cjk_char(firstchar)) {
		re = unicode_cast<utf8::string>(firstchar);
	}
	return re;
}

/**
 * Open the help browser, show topic with id show_topic.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_help(display &disp, const std::string& show_topic, int xloc, int yloc)
{
	show_help(disp, toplevel, show_topic, xloc, yloc);
}

/**
 * Open the help browser, show unit with id unit_id.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_unit_help(display &disp, const std::string& show_topic, bool hidden, int xloc, int yloc)
{
	show_help(disp, toplevel, hidden_symbol(hidden) + unit_prefix + show_topic, xloc, yloc);
}

/**
 * Open the help browser, show terrain with id terrain_id.
 *
 * If show_topic is the empty string, the default topic will be shown.
 */
void show_terrain_help(display &disp, const std::string& show_topic, bool hidden, int xloc, int yloc)
{
	show_help(disp, toplevel, hidden_symbol(hidden) + terrain_prefix + show_topic, xloc, yloc);
}



/**
 * Open the help browser, show the variation of the unit matching.
 */
void show_variation_help(display &disp, const std::string& unit, const std::string &variation, bool hidden, int xloc, int yloc)
{
	show_help(disp, toplevel, hidden_symbol(hidden) + variation_prefix + unit + "_" + variation, xloc, yloc);
}

/**
 * Open a help dialog using a toplevel other than the default.
 *
 * This allows for complete customization of the contents, although not in a
 * very easy way.
 */
void show_help(display &disp, const section &toplevel_sec,
			   const std::string& show_topic,
			   int xloc, int yloc)
{
	const events::event_context dialog_events_context;
	const gui::dialog_manager manager;
	const resize_lock prevent_resizing;

	CVideo& screen = disp.video();
	const surface& scr = screen.getSurface();

	const int width  = std::min<int>(font::relative_size(900), scr->w - font::relative_size(20));
	const int height = std::min<int>(font::relative_size(800), scr->h - font::relative_size(150));
	const int left_padding = font::relative_size(10);
	const int right_padding = font::relative_size(10);
	const int top_padding = font::relative_size(10);
	const int bot_padding = font::relative_size(10);

	// If not both locations were supplied, put the dialog in the middle
	// of the screen.
	if (yloc <= -1 || xloc <= -1) {
		xloc = scr->w / 2 - width / 2;
		yloc = scr->h / 2 - height / 2;
	}
	std::vector<gui::button*> buttons_ptr;
	gui::button close_button_(disp.video(), _("Close"));
	buttons_ptr.push_back(&close_button_);

	gui::dialog_frame f(disp.video(), _("The Battle for Wesnoth Help"), gui::dialog_frame::default_style,
					 true, &buttons_ptr);
	f.layout(xloc, yloc, width, height);
	f.draw();

    // Find all unit_types that have not been constructed yet and fill in the information
    // needed to create the help topics
	unit_types.build_all(unit_type::HELP_INDEXED);

	if (preferences::encountered_units().size() != size_t(last_num_encountered_units) ||
	    preferences::encountered_terrains().size() != size_t(last_num_encountered_terrains) ||
	    last_debug_state != game_config::debug ||
		last_num_encountered_units < 0) {
		// More units or terrains encountered, update the contents.
		last_num_encountered_units = preferences::encountered_units().size();
		last_num_encountered_terrains = preferences::encountered_terrains().size();
		last_debug_state = game_config::debug;
		generate_contents();
	}
	try {
		help_browser hb(disp, toplevel_sec);
		hb.set_location(xloc + left_padding, yloc + top_padding);
		hb.set_width(width - left_padding - right_padding);
		hb.set_height(height - top_padding - bot_padding);
		if (show_topic != "") {
			hb.show_topic(show_topic);
		}
		else {
			hb.show_topic(default_show_topic);
		}
		hb.set_dirty(true);
		events::raise_draw_event();
		disp.flip();
		disp.invalidate_all();
		CKey key;
		for (;;) {
			events::pump();
			events::raise_process_event();
			events::raise_draw_event();
			if (key[SDLK_ESCAPE]) {
				// Escape quits from the dialog.
				return;
			}
			for (std::vector<gui::button*>::iterator button_it = buttons_ptr.begin();
				 button_it != buttons_ptr.end(); ++button_it) {
				if ((*button_it)->pressed()) {
					// There is only one button, close.
					return;
				}
			}
			disp.flip();
			disp.delay(10);
		}
	}
	catch (parse_error& e) {
		std::stringstream msg;
		msg << _("Parse error when parsing help text: ") << "'" << e.message << "'";
		gui2::show_transient_message(disp.video(), "", msg.str());
	}
}

} // End namespace help.
