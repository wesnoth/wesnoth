/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "global.hpp"

#include "about.hpp"
#include "cursor.hpp"
#include "display.hpp"
#include "events.hpp"
#include "font.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "help.hpp"
#include "image.hpp"
#include "language.hpp"
#include "preferences.hpp"
#include "sdl_utils.hpp"
#include "show_dialog.hpp"
#include "unit.hpp"
#include "util.hpp"
#include "video.hpp"
#include "wassert.hpp"
#include "wml_separators.hpp"
#include "serialization/parser.hpp"
#include "serialization/string_utils.hpp"
#include "widgets/button.hpp"
#include "widgets/menu.hpp"
#include "widgets/scrollbar.hpp"
#include "widgets/widget.hpp"

#include <algorithm>
#include <iostream>
#include <list>
#include <locale>
#include <map>
#include <queue>
#include <set>
#include <sstream>

namespace help {

/// Generate the help contents from the configurations given to the
/// manager.
void generate_contents();

struct section;

typedef std::vector<section *> section_list;

/// Generate a topic text on the fly.
class topic_generator {
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
class topic_text {
	mutable std::vector< std::string > parsed_text_;
	mutable topic_generator *generator_;
public:
	~topic_text();
	topic_text(): generator_(NULL) {}
	topic_text(std::string const &t): generator_(new text_topic_generator(t)) {}
	explicit topic_text(topic_generator *g): generator_(g) {}
	topic_text &operator=(topic_generator *g);
	topic_text(topic_text const &t);
	operator std::vector< std::string > const &() const;
};

/// A topic contains a title, an id and some text.
struct topic {
	topic() {}
	topic(const std::string &_title, const std::string &_id)
		: title(_title), id(_id) {}
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
	topic_text text;
};

typedef std::list<topic> topic_list;

/// A section contains topics and sections along with title and ID.
struct section {
	section(const std::string &_title, const std::string &_id, const topic_list &_topics,
			const std::vector<section> &_sections);
	section() : title(""), id("") {}
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
};


/// To be used as a function object to locate sections and topics
/// with a specified ID.
class has_id {
public:
	has_id(const std::string &id) : id_(id) {}
	bool operator()(const topic &t) { return t.id == id_; }
	bool operator()(const section &s) { return s.id == id_; }
	bool operator()(const section *s) { return s != NULL && s->id == id_; }
private:
	const std::string id_;
};

/// To be used as a function object when sorting topic lists on the title.
class title_less {
public:
	bool operator()(const topic &t1, const topic &t2) { return t1.title < t2.title; }
};

struct delete_section {
	void operator()(section *s) { delete s; }
};

struct create_section {
	section *operator()(const section *s) { return new section(*s); }
	section *operator()(const section &s) { return new section(s); }
};
		
/// The menu to the left in the help browser, where topics can be
/// navigated through and chosen.
class help_menu : public gui::menu {
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
	SDL_Rect rect_;
	topic const *chosen_topic_;
	visible_item selected_item_;
};

/// Thrown when the help system fails to parse something.
struct parse_error {
	parse_error(const std::string& msg) : message(msg) {}
	std::string message;
};

/// The area where the content is shown in the help browser.
class help_text_area : public gui::scrollarea {
public:
	help_text_area(CVideo &video, const section &toplevel);
	/// Display the topic.
	void show_topic(const topic &t);

	/// Return the ID that is crossreferenced at the (screen)
	/// coordinates x, y. If no cross-reference is there, return the
	/// empty string.
	std::string ref_at(const int x, const int y);

protected:
	virtual void scroll(int pos);
	virtual void set_inner_location(const SDL_Rect& rect);

private:
	enum ALIGNMENT {LEFT, MIDDLE, RIGHT, HERE};
	/// Convert a string to an alignment. Throw parse_error if
	/// unsuccesful.
	ALIGNMENT str_to_align(const std::string &s);

	/// An item that is displayed in the text area. Contains the surface
	/// that should be blitted along with some other information.
	struct item {

		item(surface surface, int x, int y, const std::string text="",
			 const std::string reference_to="", bool floating=false,
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
	void add_text_item(const std::string text, const std::string ref_dst="",
					   int font_size=-1, bool bold=false, bool italic=false,
					   SDL_Color color=font::NORMAL_COLOUR);

	/// Add an image item with the specified attributes.
	void add_img_item(const std::string path, const std::string alignment, const bool floating,
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
class help_browser : public gui::widget {
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
std::vector<section> generate_sections(const std::string &generator);
std::vector<topic> generate_topics(const std::string &generator);
std::string generate_topic_text(const std::string &generator);
std::string generate_about_text();
std::string generate_traits_text();
std::vector<topic> generate_unit_topics();
enum UNIT_DESCRIPTION_TYPE {FULL_DESCRIPTION, NO_DESCRIPTION, NON_REVEALING_DESCRIPTION};
/// Return the type of description that should be shown for a unit of
/// the given kind. This method is intended to filter out information
/// about units that should not be shown, for example due to not being
/// encountered.
UNIT_DESCRIPTION_TYPE description_type(const unit_type &type);
std::vector<topic> generate_ability_topics();
std::vector<topic> generate_weapon_special_topics();
std::vector<topic> generate_terrains_topics();

/// Parse a help config, return the top level section. Return an empty
/// section if cfg is NULL.
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
/// and its subsections. Return the found topic, or NULL if none could
/// be found.
const topic *find_topic(const section &sec, const std::string &id);

/// Search for the section with the specified identifier in the section
/// and its subsections. Return the found section or NULL if none could
/// be found.
const section *find_section(const section &sec, const std::string &id);

/// Parse a text string. Return a vector with the different parts of the
/// text. Each markup item is a separate part while the text between
/// markups are separate parts.
std::vector<std::string> parse_text(const std::string &text);

/// Convert the contents to wml attributes, surrounded within
/// [element_name]...[/element_name]. Return the resulting WML.
std::string convert_to_wml(const std::string &element_name, const std::string &contents);

/// Return true if s is a representation of a truth value
/// (yes/true/...), otherwise false.
bool get_bool(const std::string &s);

/// Return the color the string represents. Return font::NORMAL_COLOUR if
/// the string is empty or can't be matched against any other color.
SDL_Color string_to_color(const std::string &s);

/// Make a best effort to word wrap s. All parts are less than width.
std::vector<std::string> split_in_width(const std::string &s, const int font_size, const unsigned width);

std::string remove_first_space(const std::string& text);

/// Return a lowercase copy of s.
std::string to_lower(const std::string &s);

/// Return a copy of s with the first letter capitalized.
std::string cap(const std::string &s);

/// Prepend all chars with meaning inside attributes with a backslash.
std::string escape(const std::string &s);

/// Return the first word in s, not removing any spaces in the start of
/// it.
std::string get_first_word(const std::string &s);

} // namespace help

namespace {
	const config *game_cfg = NULL;
	const game_data *game_info = NULL;
	gamemap *map = NULL;
	// The default toplevel.
	help::section toplevel; 
	// All sections and topics not referenced from the default toplevel.
	help::section hidden_sections; 

	int last_num_encountered_units = -1;
	int last_num_encountered_terrains = -1;
								   
	config dummy_cfg;
	std::vector<std::string> empty_string_vector;
	const int max_section_level = 15;
	const int menu_font_size = font::SIZE_NORMAL;
	const int title_size = font::SIZE_LARGE;
	const int title2_size = font::SIZE_15;
	const int box_width = 2;
	const int normal_font_size = font::SIZE_SMALL;
	const unsigned max_history = 100;
	const std::string topic_img = "help/topic.png";
	const std::string closed_section_img = "help/closed_section.png";
	const std::string open_section_img = "help/open_section.png";
	// The topic to open by default when opening the help dialog.
	const std::string default_show_topic = "introduction_topic"; 
	
	/// Return true if the id is valid for user defined topics and
	/// sections. Some IDs are special, such as toplevel and may not be
	/// be defined in the config.
	bool is_valid_id(const std::string &id) {
		if (id == "toplevel") {
			return false;
		}
		if (id.find("unit_") == 0) {
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
		about_text_formatter() : text_started_(false) {}
		std::string operator()(const std::string &s) {
			std::string res = s;
			if (res.size() > 0) {
				bool header = false;
				// Format + as headers, and the rest as normal text.
				if (res[0] == '+') {
					header = true;
					res.erase(res.begin());
				}
				else if (res[0] == '-') {
					res.erase(res.begin());
				}
				// There is a bunch of empty rows in the start in about.cpp,
				// we do not want to show these here. Thus, if we still
				// encounter one of those, return an empty string that will
				// be removed totally at a later stage.
				if (!text_started_ && res.find_first_not_of(' ') != std::string::npos) {
					text_started_ = true;
				}
				if (text_started_) {
					std::stringstream ss;
					if (header) {
						ss << "<header>text='" << help::escape(res) << "'</header>";
						res = ss.str();
					}
				}
				else {
					res = "";
				}
			}
			return res;
		}
	private:
		bool text_started_;
	};

	// Helpers for making generation of topics easier.
	std::string jump_to(const unsigned pos) {
		std::stringstream ss;
		ss << "<jump>to=" << pos << "</jump>";
		return ss.str();
	}

	std::string jump(const unsigned amount) {
		std::stringstream ss;
		ss << "<jump>amount=" << amount << "</jump>";
		return ss.str();
	}
	
	std::string bold(const std::string &s) {
		std::stringstream ss;
		ss << "<bold>text='" << help::escape(s) << "'</bold>";
		return ss.str();
	}

	typedef std::vector<std::vector<std::pair<std::string, unsigned int > > > table_spec;
	// Create a table using the table specs. Return markup with jumps
	// that create a table. The table spec contains a vector with
	// vectors with pairs. The pairs are the markup string that should
	// be in a cell, and the width of that cell.
  std::string generate_table(const table_spec &tab, const unsigned int spacing=font::relative_size(20)) {
		table_spec::const_iterator row_it;
		std::vector<std::pair<std::string, unsigned> >::const_iterator col_it;
		unsigned int num_cols = 0;
		for (row_it = tab.begin(); row_it != tab.end(); row_it++) {
			if (row_it->size() > num_cols) {
				num_cols = row_it->size();
			}
		}
		std::vector<unsigned int> col_widths(num_cols, 0);
		// Calculate the width of all columns, including spacing.
		for (row_it = tab.begin(); row_it != tab.end(); row_it++) {
			unsigned int col = 0;
			for (col_it = row_it->begin(); col_it != row_it->end(); col_it++) {
				if (col_widths[col] < col_it->second + spacing) {
					col_widths[col] = col_it->second + spacing;
				}
				col++;
			}
		}
		std::vector<unsigned int> col_starts(num_cols);
		// Calculate the starting positions of all columns
		for (unsigned int i = 0; i < num_cols; i++) {
			unsigned int this_col_start = 0;
			for (unsigned int j = 0; j < i; j++) {
				this_col_start += col_widths[j];
			}
			col_starts[i] = this_col_start;
		}
		std::stringstream ss;
		for (row_it = tab.begin(); row_it != tab.end(); row_it++) {
			unsigned int col = 0;
			for (col_it = row_it->begin(); col_it != row_it->end(); col_it++) {
				ss << jump_to(col_starts[col]) << col_it->first;
				col++;
			}
			ss << "\n";
		}
		return ss.str();
	}

	// Return the width for the image with filename.
	unsigned image_width(const std::string &filename) {
		image::locator loc(filename);
		surface surf(image::get_image(loc, image::UNSCALED));
		if (surf != NULL) {
			return surf->w;
		}
		return 0;
	}

	void push_tab_pair(std::vector<std::pair<std::string, unsigned int> > &v, const std::string &s) {
		v.push_back(std::make_pair(s, font::line_width(s, normal_font_size)));
	}
}

namespace help {

help_manager::help_manager(const config *cfg, const game_data *gameinfo, gamemap *_map) {
	game_cfg = cfg == NULL ? &dummy_cfg : cfg;
	game_info = gameinfo;
	map = _map;
}

void generate_contents() {
	toplevel.clear();
	hidden_sections.clear();
	if (game_cfg != NULL) {
		const config *help_config = game_cfg->child("help");
		if (help_config == NULL) {
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
			config::const_child_itors itors;
			for (itors = help_config->child_range("section"); itors.first != itors.second;
				 itors.first++) {
				const std::string id = (*(*itors.first))["id"];
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
			for (itors = help_config->child_range("topic"); itors.first != itors.second;
				 itors.first++) {
				const std::string id = (*(*itors.first))["id"];
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
		catch (parse_error e) {
			std::stringstream msg;
			msg << "Parse error when parsing help text: '" << e.message << "'";
			std::cerr << msg.str() << std::endl;
		}
	}
}

help_manager::~help_manager() {
	game_cfg = NULL;
	game_info = NULL;
	map = NULL;
	toplevel.clear();
	hidden_sections.clear();
    // These last numbers must be reset so that the content is regenreated.
    // Upon next start.
	last_num_encountered_units = -1;
	last_num_encountered_terrains = -1;
}

bool section_is_referenced(const std::string &section_id, const config &cfg) {
	const config *toplevel = cfg.child("toplevel");
	if (toplevel != NULL) {
		const std::vector<std::string> toplevel_refs
			= utils::quoted_split((*toplevel)["sections"]);
		if (std::find(toplevel_refs.begin(), toplevel_refs.end(), section_id)
			!= toplevel_refs.end()) {
			return true;
		}
	}
	for (config::const_child_itors itors = cfg.child_range("section");
		 itors.first != itors.second; itors.first++) {
		const std::vector<std::string> sections_refd
			= utils::quoted_split((*(*itors.first))["sections"]);
		if (std::find(sections_refd.begin(), sections_refd.end(), section_id)
			!= sections_refd.end()) {
			return true;
		}
	}
	return false;
}

bool topic_is_referenced(const std::string &topic_id, const config &cfg) {
	const config *toplevel = cfg.child("toplevel");
	if (toplevel != NULL) {
		const std::vector<std::string> toplevel_refs
			= utils::quoted_split((*toplevel)["topics"]);
		if (std::find(toplevel_refs.begin(), toplevel_refs.end(), topic_id)
			!= toplevel_refs.end()) {
			return true;
		}
	}
	for (config::const_child_itors itors = cfg.child_range("section");
		 itors.first != itors.second; itors.first++) {
		const std::vector<std::string> topics_refd
			= utils::quoted_split((*(*itors.first))["topics"]);
		if (std::find(topics_refd.begin(), topics_refd.end(), topic_id)
			!= topics_refd.end()) {
			return true;
		}
	}
	return false;
}
	
void parse_config_internal(const config *help_cfg, const config *section_cfg,
						   section &sec, int level) {
	if (level > max_section_level) {
		std::cerr << "Maximum section depth has been reached. Maybe circular dependency?"
				  << std::endl;
	}
	else if (section_cfg != NULL) {
		const std::vector<std::string> sections = utils::quoted_split((*section_cfg)["sections"]);
		const std::string id = level == 0 ? "toplevel" : (*section_cfg)["id"];
		if (level != 0) {
			if (!is_valid_id(id)) {
				std::stringstream ss;
				ss << "Invalid ID, used for internal purpose: '" << id << "'";
				throw parse_error(ss.str());
			}
		}
		const std::string title = level == 0 ? "" : (*section_cfg)["title"];
		sec.id = id;
		sec.title = title;
		std::vector<std::string>::const_iterator it;
		// Find all child sections.
		for (it = sections.begin(); it != sections.end(); it++) {
			config const *child_cfg = help_cfg->find_child("section", "id", *it);
			if (child_cfg != NULL) {
				section child_section;
				parse_config_internal(help_cfg, child_cfg, child_section, level + 1);
				sec.add_section(child_section);
			}
			else {
				std::stringstream ss;
				ss << "Help-section '" << *it << "' referenced from '"
				   << id << "' but could not be found.";
				throw parse_error(ss.str());
			}
		}
		const std::vector<section> generated_sections =
			generate_sections((*section_cfg)["generator"]);
		std::transform(generated_sections.begin(), generated_sections.end(),
					   std::back_inserter(sec.sections), create_section());
		const std::vector<std::string> topics = utils::quoted_split((*section_cfg)["topics"]);
		// Find all topics in this section.
		for (it = topics.begin(); it != topics.end(); it++) {
			config const *topic_cfg = help_cfg->find_child("topic", "id", *it);
			if (topic_cfg != NULL) {
				std::string text = (*topic_cfg)["text"];
				text += generate_topic_text((*topic_cfg)["generator"]);
				topic child_topic((*topic_cfg)["title"], (*topic_cfg)["id"], text);
				if (!is_valid_id(child_topic.id)) {
					std::stringstream ss;
					ss << "Invalid ID, used for internal purpose: '" << id << "'";
					throw parse_error(ss.str());
				}
				sec.topics.push_back(child_topic);
			}
			else {
				std::stringstream ss;
				ss << "Help-topic '" << *it << "' referenced from '" << id
				   << "' but could not be found." << std::endl;
				throw parse_error(ss.str());
			}
		}
		const std::vector<topic> generated_topics =
			generate_topics((*section_cfg)["generator"]);
		std::copy(generated_topics.begin(), generated_topics.end(),
				  std::back_inserter(sec.topics));
	}
}	

section parse_config(const config *cfg) {
	section sec;
	if (cfg != NULL) {
		config const *toplevel_cfg = cfg->child("toplevel");
		parse_config_internal(cfg, toplevel_cfg, sec);
	}
	return sec;
}


std::vector<section> generate_sections(const std::string &generator) {
	std::vector<section> empty_vec;
	if (generator == "") {
		return empty_vec;
	}
	return empty_vec;
}

std::vector<topic> generate_topics(const std::string &generator) {
	std::vector<topic> res;
	if (generator == "units") {
		res = generate_unit_topics();
	}
	else if (generator == "abilities") {
		res = generate_ability_topics();
	}
	else if (generator == "weapon_specials") {
		res = generate_weapon_special_topics();
	}
	else if (generator == "terrains") {
		res = generate_terrains_topics();
	}
	std::sort(res.begin(), res.end(), title_less());
	return res;
}

std::string generate_topic_text(const std::string &generator) {
	std::string empty_string = "";
	if (generator == "") {
		return empty_string;
	}
	if (generator == "about") {
		return generate_about_text();
	}
	if (generator == "traits") {
		// See comment on generate_traits_text()
		//return generate_traits_text();
	}
	return empty_string;
}

topic_text::~topic_text() {
	if (generator_ && --generator_->count == 0)
		delete generator_;
}

topic_text::topic_text(topic_text const &t): parsed_text_(t.parsed_text_), generator_(t.generator_) {
	if (generator_)
		++generator_->count;
}

topic_text &topic_text::operator=(topic_generator *g) {
	if (generator_ && --generator_->count == 0)
		delete generator_;
	generator_ = g;
	return *this;
}

topic_text::operator std::vector< std::string > const &() const {
	if (generator_) {
		parsed_text_ = parse_text((*generator_)());
		if (--generator_->count == 0)
			delete generator_;
		generator_ = NULL;
	}
	return parsed_text_;
}

std::vector<topic> generate_weapon_special_topics() {
	std::vector<topic> topics;
	if (game_info == NULL) {
		return topics;
	}
	std::set<std::string> checked_specials;
	for(game_data::unit_type_map::const_iterator i = game_info->unit_types.begin();
	    i != game_info->unit_types.end(); i++) {
		const unit_type &type = (*i).second;
		// Only show the weapon special if we find it on a unit that
		// detailed description should be shown about.
		if (description_type(type) == FULL_DESCRIPTION) {
			std::vector<attack_type> attacks = type.attacks();
			for (std::vector<attack_type>::const_iterator it = attacks.begin();
				 it != attacks.end(); it++) {
				const std::string special = (*it).special();
				if (special != "") {
					if (checked_specials.find(special) == checked_specials.end()) {
						std::string lang_special = gettext(special.c_str());
						lang_special = cap(lang_special);
						std::string description
							= string_table["weapon_special_" + special + "_description"];
						const size_t colon_pos = description.find(':');
						if (colon_pos != std::string::npos) {
							// Remove the first colon and the following newline.
							description.erase(0, colon_pos + 2); 
						}
						topic t(lang_special, "weaponspecial_" + special, description);
						topics.push_back(t);
						checked_specials.insert(special);
					}
				}
			}
		}
	}
	return topics;
}

std::vector<topic> generate_ability_topics() {
	std::vector<topic> topics;
	if (game_info == NULL) {
		return topics;
	}
	std::set<std::string> checked_abilities;
	// Look through all the unit types, check if a unit of this type
	// should have a full description, if so, add this units abilities
	// for display. We do not want to show abilities that the user has
	// not encountered yet.
	for(game_data::unit_type_map::const_iterator i = game_info->unit_types.begin();
	    i != game_info->unit_types.end(); i++) {
		const unit_type &type = (*i).second;
		if (description_type(type) == FULL_DESCRIPTION) {
			for (std::vector<std::string>::const_iterator it = type.abilities().begin();
				 it != type.abilities().end(); it++) {
				if (checked_abilities.find(*it) == checked_abilities.end()) {
					const std::string id = "ability_" + *it;
					std::string lang_ability = cap(string_table[id]);
					std::string description = string_table[*it + "_description"];
					const size_t colon_pos = description.find(':');
					if (colon_pos != std::string::npos) {
						// Remove the first colon and the following newline.
						description.erase(0, colon_pos + 2); 
					}
					topic t(lang_ability, id, description);
					topics.push_back(t);
					checked_abilities.insert(*it);
				}
			}
		}
		
	}
	return topics;
}

class unit_topic_generator: public topic_generator
{
	unit_type type_;
public:
	unit_topic_generator(unit_type const &t): type_(t) {}
	virtual std::string operator()() const {
		std::stringstream ss;
		std::string clear_stringstream;
		const std::string detailed_description = type_.unit_description();
		const unit_type& female_type = type_.get_gender_unit_type(unit_race::FEMALE);
		const unit_type& male_type = type_.get_gender_unit_type(unit_race::MALE);

		// Show the unit's image and its level.
		ss << "<img>src='" << male_type.image() << "'</img> ";

		if (&female_type != &male_type)
			ss << "<img>src='" << female_type.image() << "'</img> ";
		ss << "<format>font_size=" << font::relative_size(11) << " text=' " << escape(_("level"))
		   << " " << type_.level() << "'</format>\n";

		// Print the units this unit can advance to. Cross reference
		// to the topics containing information about those units.
		std::vector<std::string> next_units = type_.advances_to();
		if (!next_units.empty()) {
			ss << _("Advances to: ");
			for (std::vector<std::string>::const_iterator advance_it = next_units.begin(),
				 advance_end = next_units.end();
				 advance_it != advance_end; ++advance_it) {
				std::string unit_id = *advance_it;
				std::map<std::string,unit_type>::const_iterator new_type = game_info->unit_types.find(unit_id);
			 	if(new_type != game_info->unit_types.end()) {
					std::string lang_unit = new_type->second.language_name();
					std::string ref_id = std::string("unit_") + new_type->second.id();
					ss << "<ref>dst='" << escape(ref_id) << "' text='" << escape(lang_unit)
					   << "'</ref>";
					if (advance_it + 1 != advance_end)
						ss << ", ";
				}
			}
			ss << "\n";
		}

		// Print the abilities the units has, cross-reference them
		// to their respective topics.
		if (!type_.abilities().empty()) {
			ss << _("Abilities: ");
			for(std::vector<std::string>::const_iterator ability_it = type_.abilities().begin(),
				 ability_end = type_.abilities().end();
				 ability_it != ability_end; ++ability_it) {
				const std::string ref_id = std::string("ability_") + *ability_it;
				std::string lang_ability = string_table[ref_id];
				ss << "<ref>dst='" << escape(ref_id) << "' text='" << escape(lang_ability)
				   << "'</ref>";
				if (ability_it + 1 != ability_end)
					ss << ", ";
			}
			ss << "\n";
		}

		if (!next_units.empty() || !type_.abilities().empty())
			ss << "\n";
		// Print some basic information such as HP and movement points.
		ss << _("HP: ") << type_.hitpoints() << jump(30)
		   << _("Moves: ") << type_.movement() << jump(30)
		   << _("Alignment: ")
		   << type_.alignment_description(type_.alignment())
		   << jump(30);
		if (type_.can_advance())
			ss << _("Required XP: ") << type_.experience_needed();

		// Print the detailed description about the unit.
		ss << "\n\n" << detailed_description;

		typedef std::pair<std::string,unsigned int> item;

		// Print the different attacks a unit has, if it has any.
		std::vector<attack_type> attacks = type_.attacks();
		if (!attacks.empty()) {
			// Print headers for the table.
			ss << "\n\n<header>text='" << escape(cap(_("attacks")))
			   << "'</header>\n\n";
			table_spec table;

			std::vector<item> first_row;
			// Dummy element, icons are below.
			first_row.push_back(item("", 0));
			first_row.push_back(item(bold(_("Name")),
						 font::line_width(cap(_("Name")),
								  normal_font_size,
								  TTF_STYLE_BOLD)));
			first_row.push_back(item(bold(_("Type")),
						 font::line_width(_("Type"),
								  normal_font_size,
								  TTF_STYLE_BOLD)));
			first_row.push_back(item(bold(_("Dmg")),
						 font::line_width(_("Dmg"),
								  normal_font_size,
								  TTF_STYLE_BOLD)));
			first_row.push_back(item(bold(_("Strikes")),
						 font::line_width(_("Strikes"),
								  normal_font_size,
								  TTF_STYLE_BOLD)));
			first_row.push_back(item(bold(_("Range")),
						 font::line_width(_("Range"),
								  normal_font_size,
								  TTF_STYLE_BOLD)));
			first_row.push_back(item(bold(_("Special")),
						 font::line_width(_("Special"),
								  normal_font_size,
								  TTF_STYLE_BOLD)));
			table.push_back(first_row);
			// Print information about every attack.
			for(std::vector<attack_type>::const_iterator attack_it = attacks.begin(),
				 attack_end = attacks.end();
				 attack_it != attack_end; ++attack_it) {
				std::string lang_weapon = gettext(attack_it->name().c_str());
				std::string lang_type = gettext(attack_it->type().c_str());
				std::vector<item> row;
				std::stringstream attack_ss;
				attack_ss << "<img>src='" << (*attack_it).icon() << "'</img>";
				row.push_back(std::make_pair(attack_ss.str(),
							     image_width(attack_it->icon())));
				attack_ss.str(clear_stringstream);
				push_tab_pair(row, lang_weapon);
				push_tab_pair(row, lang_type);
				attack_ss << attack_it->damage();
				push_tab_pair(row, attack_ss.str());
				attack_ss.str(clear_stringstream);
				attack_ss << attack_it->num_attacks();
				push_tab_pair(row, attack_ss.str());
				attack_ss.str(clear_stringstream);
				push_tab_pair(row, (*attack_it).range() == attack_type::SHORT_RANGE ?
							  _("melee") : _("ranged"));
				// Show this attack's special, if it has any. Cross
				// reference it to the section describing the
				// special.
				if (!attack_it->special().empty()) {
					const std::string ref_id = std::string("weaponspecial_")
						+ (*attack_it).special();
					std::string lang_special = gettext(attack_it->special().c_str());
					attack_ss << "<ref>dst='" << escape(ref_id)
					          << "' text='" << escape(lang_special) << "'</ref>";
					row.push_back(std::make_pair(attack_ss.str(),
								     font::line_width(lang_special,
										      normal_font_size)));
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
		first_res_row.push_back(std::make_pair(bold(_("Attack Type")),
						       font::line_width(_("Attack Type"),
									normal_font_size,
									TTF_STYLE_BOLD)));
		first_res_row.push_back(std::make_pair(bold(_("Resistance")),
						       font::line_width(_("Resistance"),
									normal_font_size,
									TTF_STYLE_BOLD)));
		resistance_table.push_back(first_res_row);
		const unit_movement_type &movement_type = type_.movement_type();
		string_map dam_tab = movement_type.damage_table();
		for(string_map::const_iterator dam_it = dam_tab.begin(), dam_end = dam_tab.end();
			 dam_it != dam_end; ++dam_it) {
			std::vector<item> row;
			int resistance = 100 - atoi((*dam_it).second.c_str());
			std::string color;
			if (resistance < 0)
				color = "red";
			std::string lang_weapon = gettext(dam_it->first.c_str());
			push_tab_pair(row, lang_weapon);
			std::stringstream str;
			str << "<format>color=" << color << " text='"<< resistance << "%'</format>";
			const std::string markup = str.str();
			str.str(clear_stringstream);
			str << resistance << "%";
			row.push_back(std::make_pair(markup,
						     font::line_width(str.str(), normal_font_size)));
			resistance_table.push_back(row);
		}
		ss << generate_table(resistance_table);
		if (map != NULL) {
			// Print the terrain modifier table of the unit.
			ss << "\n\n<header>text='" << escape(_("Terrain Modifiers"))
			   << "'</header>\n\n";
			std::vector<item> first_row;
			table_spec table;
			first_row.push_back(std::make_pair(bold(_("Terrain")),
								   font::line_width(_("Terrain"),
										    normal_font_size,
										    TTF_STYLE_BOLD)));
			first_row.push_back(std::make_pair(bold(_("Movement")),
							   font::line_width(_("Movement"),
									    normal_font_size,
									    TTF_STYLE_BOLD)));
			first_row.push_back(std::make_pair(bold(_("Defense")),
							   font::line_width(_("Defense"),
									    normal_font_size,
									    TTF_STYLE_BOLD)));
			table.push_back(first_row);
			for (std::set<std::string>::const_iterator terrain_it =
					 preferences::encountered_terrains().begin(),
				 terrain_end = preferences::encountered_terrains().end();
				 terrain_it != terrain_end; terrain_it++) {
				wassert(terrain_it->size() > 0);
				const gamemap::TERRAIN terrain = (*terrain_it)[0];
				if (terrain == gamemap::FOGGED || terrain == gamemap::VOID_TERRAIN)
					continue;
				const terrain_type& info = map->get_terrain_info(terrain);
				if (!info.is_alias()) {
					std::vector<item> row;
					const std::string& name = info.name();
					const int moves = movement_type.movement_cost(*map,terrain);
					std::stringstream str;
					str << "<ref>text='" << escape(name) << "' dst='"
					    << escape(std::string("terrain_") + terrain) << "'</ref>";
					row.push_back(std::make_pair(str.str(), 
								     font::line_width(name,
										      normal_font_size)));
					str.str(clear_stringstream);
					if(moves < 100)
						str << moves;
					else
						str << "--";
					push_tab_pair(row, str.str());
					str.str(clear_stringstream);
					const int defense =
						100 - movement_type.defense_modifier(*map,terrain);
					str << defense << "%";
					push_tab_pair(row, str.str());
					table.push_back(row);
				}
			}
			ss << generate_table(table);
		}
		return ss.str();
	}
};

std::vector<topic> generate_unit_topics() {
	std::vector<topic> topics;
	if (game_info == NULL) {
		return topics;
	}
	for(game_data::unit_type_map::const_iterator i = game_info->unit_types.begin();
	    i != game_info->unit_types.end(); i++) {
		const unit_type &type = (*i).second;
		UNIT_DESCRIPTION_TYPE desc_type = description_type(type);
		if (desc_type == NO_DESCRIPTION) {
			continue;
		}
		const std::string lang_name = type.language_name();
		const std::string id = type.id();
		topic unit_topic(lang_name, std::string("unit_") + id, "");
		if (desc_type == NON_REVEALING_DESCRIPTION) {
		} else if (desc_type == FULL_DESCRIPTION) {
			unit_topic.text = new unit_topic_generator(type);
		} else {
			wassert(false);
		}
		topics.push_back(unit_topic);
	}
	return topics;
}

UNIT_DESCRIPTION_TYPE description_type(const unit_type &type) {
	const std::string id = type.name();
	const std::set<std::string> &encountered_units = preferences::encountered_units();
	if (game_config::debug) {
		return FULL_DESCRIPTION;
	}
	if (encountered_units.find(id) != encountered_units.end()) {
		return FULL_DESCRIPTION;
	}
	return NO_DESCRIPTION;
}

struct terrain_topic_generator: topic_generator
{
	terrain_topic_generator(terrain_type const &t): type(t) {}
	terrain_type type;
	virtual std::string operator()() const {
		std::stringstream ss;
		ss << "<img>src='terrain/" << type.symbol_image() << ".png'</img>\n\n";
		if (type.is_alias()) {
			const std::string aliased_terrains = type.type();
			std::stringstream alias_ss;
			for (std::string::const_iterator it = aliased_terrains.begin();
				 it != aliased_terrains.end(); it++) {
				const gamemap::TERRAIN t = *it;
				const std::string &alias_name = map->get_terrain_info(t).name();
				alias_ss << "<ref>text='" << escape(alias_name) << "' dst='"
					 << escape(std::string("terrain_") + t) << "'</ref>";
				if (it + 2 == aliased_terrains.end())
					alias_ss << " " << _("or") << " ";
				else if (it + 1 != aliased_terrains.end())
					alias_ss << ", ";
			}
			string_map sm;
			sm["terrains"] = alias_ss.str();
			ss << utils::interpolate_variables_into_string(
				_("This terrain acts as $terrains for movement and defense purposes."), &sm);
			if (aliased_terrains.size() > 1)
				ss << " " << _("The terrain with the best modifier is chosen automatically.");
			ss << "\n\n";
		}
		if (type.is_keep())
			ss << _("This terrain acts as keep, i.e., you can recruit units when a leader is in a location with this terrain.") << "\n\n";
		if (type.is_castle())
			ss << _("This terrain acts as castle, i.e., you can recruit units onto a location with this terrain.") << "\n\n";
		if (type.gives_healing())
			ss << _("This terrain gives healing.") << "\n\n";
		return ss.str();
	}
};

std::vector<topic> generate_terrains_topics() {
	std::vector<topic> res;
	std::vector<gamemap::TERRAIN> show_info_about;
	if (game_config::debug) {
		show_info_about = map->get_terrain_list();
	}
	else {
		for (std::set<std::string>::const_iterator terrain_it =
				 preferences::encountered_terrains().begin();
			 terrain_it != preferences::encountered_terrains().end();
			 terrain_it++) {
			wassert(terrain_it->size() > 0);
			const gamemap::TERRAIN terrain = (*terrain_it)[0];
			show_info_about.push_back(terrain);
		}
	}
	show_info_about.erase(std::remove(show_info_about.begin(), show_info_about.end(),
									  (char)gamemap::VOID_TERRAIN), show_info_about.end());
	show_info_about.erase(std::remove(show_info_about.begin(), show_info_about.end(),
									  (char)gamemap::FOGGED), show_info_about.end());
	for (std::vector<gamemap::TERRAIN>::const_iterator terrain_it = show_info_about.begin();
		 terrain_it != show_info_about.end(); terrain_it++) {
		const terrain_type& info = map->get_terrain_info(*terrain_it);
		const std::string &name = info.name();
		topic t(name, std::string("terrain_") + *terrain_it, new terrain_topic_generator(info));
		res.push_back(t);
	}
	return res;
}

std::string generate_traits_text() {
	// Ok, this didn't go as well as I thought since the information
	// generated from this is rather short and not suitable for the help
	// system. Hence, this method is not used currently :).
	std::stringstream ss;
	if (game_cfg != NULL) {
		const config *unit_cfg = game_cfg->child("units");
		if (unit_cfg != NULL) {
			const config::child_list child_list = unit_cfg->get_children("trait");
			for (config::const_child_iterator it = child_list.begin();
				 it != child_list.end(); it++) {
				if (game_info->unit_types.size() > 0) {
					unit dummy_unit(&(*(game_info->unit_types.begin())).second, 0, false, true);
					dummy_unit.add_modification("trait", *(*it), true);
					std::string s = dummy_unit.modification_description("trait");
					size_t pos = 0;
					while (pos != std::string::npos) {
						// Remove paranthesis, they do not look good in the help.
						pos = s.find_first_of("()");
						if (pos != std::string::npos) {
							s.replace(pos, pos+1, "");
						}
					}
					ss << s << '\n';
				}
			}
		}
	}
	return ss.str();
}


std::string generate_about_text() {
	std::vector<std::string> about_lines = about::get_text();
	std::vector<std::string> res_lines;
	std::transform(about_lines.begin(), about_lines.end(), std::back_inserter(res_lines),
				   about_text_formatter());
	std::vector<std::string>::iterator it =
		std::remove(res_lines.begin(), res_lines.end(), "");
	std::vector<std::string> res_lines_rem(res_lines.begin(), it);
	std::string text = utils::join(res_lines_rem, '\n');
	return text;
}

bool topic::operator==(const topic &t) const {
	return t.id == id;
}

bool topic::operator<(const topic &t) const {
	return id < t.id;
}


section::section(const std::string &_title, const std::string &_id, const topic_list &_topics,
		const std::vector<section> &_sections)
	: title(_title), id(_id), topics(_topics) {
	std::transform(_sections.begin(), _sections.end(), std::back_inserter(sections),
				   create_section());
}

section::~section() {
	std::for_each(sections.begin(), sections.end(), delete_section());
}

section::section(const section &sec) 
	: title(sec.title), id(sec.id), topics(sec.topics) {
	std::transform(sec.sections.begin(), sec.sections.end(),
				   std::back_inserter(sections), create_section());
}

section& section::operator=(const section &sec) {
	title = sec.title;
	id = sec.id;
	std::copy(sec.topics.begin(), sec.topics.end(), std::back_inserter(topics));
	std::transform(sec.sections.begin(), sec.sections.end(),
				   std::back_inserter(sections), create_section());
	return *this;
}
	

bool section::operator==(const section &sec) const {
	return sec.id == id;
}

bool section::operator<(const section &sec) const {
	return id < sec.id;
}

void section::add_section(const section &s) {
	sections.push_back(new section(s));
}

void section::clear() {
	topics.clear();
	std::for_each(sections.begin(), sections.end(), delete_section());
	sections.clear();
}

help_menu::help_menu(CVideo &video, section const &toplevel, int max_height)
	: gui::menu(video, empty_string_vector, false, max_height),
	  toplevel_(toplevel), chosen_topic_(NULL), selected_item_(&toplevel, "") {
	update_visible_items(toplevel_);
	display_visible_items();
	if (!visible_items_.empty())
		selected_item_ = visible_items_.front();
}

bool help_menu::expanded(const section &sec) {
	return expanded_.find(&sec) != expanded_.end();
}

void help_menu::expand(const section &sec) {
	if (sec.id != "toplevel") {
		expanded_.insert(&sec);
	}
}

void help_menu::contract(const section &sec) {
	expanded_.erase(&sec);
}

void help_menu::update_visible_items(const section &sec, unsigned level) {
	if (level == 0) {
		// Clear if this is the top level, otherwise append items.
		visible_items_.clear();
	}
	section_list::const_iterator sec_it;
	for (sec_it = sec.sections.begin(); sec_it != sec.sections.end(); sec_it++) {
		const std::string vis_string = get_string_to_show(*(*sec_it), level);
		visible_items_.push_back(visible_item(*sec_it, vis_string));
		if (expanded(*(*sec_it))) {
			update_visible_items(*(*sec_it), level + 1);
		}
	}
	topic_list::const_iterator topic_it;
	for (topic_it = sec.topics.begin(); topic_it != sec.topics.end(); topic_it++) {
		const std::string vis_string = get_string_to_show(*topic_it, level);
		visible_items_.push_back(visible_item(&(*topic_it), vis_string));
	}
}


std::string help_menu::get_string_to_show(const section &sec, const unsigned level) {
	std::stringstream to_show;
	std::string pad_string;
	// Indentation is represented as three spaces per level.
	pad_string.resize(level * 3, ' ');
	to_show << pad_string << IMG_TEXT_SEPARATOR << IMAGE_PREFIX;
	if (expanded(sec)) {
		to_show << open_section_img;
	}
	else {
		to_show << closed_section_img;
	}
	to_show << IMG_TEXT_SEPARATOR << sec.title;
	return to_show.str();
}

std::string help_menu::get_string_to_show(const topic &topic, const unsigned level) {
	std::string pad_string;
	pad_string.resize(level * 3, ' ');
	std::stringstream to_show;
	to_show << pad_string << IMG_TEXT_SEPARATOR << IMAGE_PREFIX << topic_img
	        << IMG_TEXT_SEPARATOR << topic.title;
	return to_show.str();
}

bool help_menu::select_topic_internal(const topic &t, const section &sec) {
	topic_list::const_iterator tit =
		std::find(sec.topics.begin(), sec.topics.end(), t);
	if (tit != sec.topics.end()) {
		expand(sec);
		return true;
	}
	section_list::const_iterator sit;
	for (sit = sec.sections.begin(); sit != sec.sections.end(); sit++) {
		if (select_topic_internal(t, *(*sit))) {
			expand(sec);
			return true;
		}
	}
	return false;
}

void help_menu::select_topic(const topic &t) {
	if (selected_item_ == t) {
		// The requested topic is already selected.
		return;
	}
	if (select_topic_internal(t, toplevel_)) {
		update_visible_items(toplevel_);
		for (std::vector<visible_item>::const_iterator it = visible_items_.begin();
			 it != visible_items_.end(); it++) {
			if (*it == t) {
				selected_item_ = *it;
				break;
			}
		}
		display_visible_items();
	}
}
	
int help_menu::process() {
	int res = menu::process();
	if (double_clicked())
		res = selection();
	if (!visible_items_.empty() && (unsigned)res < visible_items_.size()) {
		selected_item_ = visible_items_[res];
		if (selected_item_.sec != NULL) {
			// Open or close a section if it is clicked.
			expanded(*selected_item_.sec) ? contract(*selected_item_.sec) : expand(*selected_item_.sec);
			update_visible_items(toplevel_);
			display_visible_items();
		} else if (selected_item_.t != NULL) {
			/// Choose a topic if it is clicked.
			chosen_topic_ = selected_item_.t;
		}
	}
	return res;
}

const topic *help_menu::chosen_topic() {
	const topic *ret = chosen_topic_;
	chosen_topic_ = NULL;
	return ret;
}
	
void help_menu::display_visible_items() {
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

bool help_menu::visible_item::operator==(const section &_sec) const {
	return sec != NULL && *sec == _sec;
}

bool help_menu::visible_item::operator==(const topic &_t) const {
	return t != NULL && *t == _t;
}

bool help_menu::visible_item::operator==(const visible_item &vis_item) const {
	return t == vis_item.t && sec == vis_item.sec;
}

help_text_area::help_text_area(CVideo &video, const section &toplevel)
	: gui::scrollarea(video), toplevel_(toplevel), shown_topic_(NULL),
	  title_spacing_(16), curr_loc_(0, 0),
	  min_row_height_(font::get_max_height(normal_font_size)), curr_row_height_(min_row_height_),
	  contents_height_(0)
{
	set_scroll_rate(40);
}

void help_text_area::set_inner_location(SDL_Rect const &rect) {
	bg_register(rect);
	if (shown_topic_)
		set_items();
}

void help_text_area::show_topic(const topic &t) {
	shown_topic_ = &t;
	set_items();
	set_dirty(true);
}

	
help_text_area::item::item(surface surface, int x, int y, const std::string _text,
						   const std::string reference_to, bool _floating,
						   bool _box, ALIGNMENT alignment)
	: surf(surface), text(_text), ref_to(reference_to), floating(_floating), box(_box),
	  align(alignment) {
	rect.x = x;
	rect.y = y;
	rect.w = box ? surface->w + box_width * 2 : surface->w;
	rect.h = box ? surface->h + box_width * 2 : surface->h;
}

help_text_area::item::item(surface surface, int x, int y, bool _floating,
						   bool _box, ALIGNMENT alignment)
	: surf(surface), text(""), ref_to(""), floating(_floating), box(_box), align(alignment) {
	rect.x = x;
	rect.y = y;
	rect.w = box ? surface->w + box_width * 2 : surface->w;
	rect.h = box ? surface->h + box_width * 2 : surface->h;
}

void help_text_area::set_items() {
	last_row_.clear();
	items_.clear();
	curr_loc_.first = 0;
	curr_loc_.second = 0;
	curr_row_height_ = min_row_height_;
	// Add the title item.
	const std::string show_title =
		font::make_text_ellipsis(shown_topic_->title, title_size, inner_location().w);
	surface surf(font::get_rendered_text(show_title, title_size,
					     font::NORMAL_COLOUR, TTF_STYLE_BOLD));
	if (surf != NULL) {
		add_item(item(surf, 0, 0, show_title));
		curr_loc_.second = title_spacing_;
		contents_height_ = title_spacing_;
		down_one_line();
	}
	// Parse and add the text.
	std::vector<std::string> const &parsed_items = shown_topic_->text;
	std::vector<std::string>::const_iterator it;
	for (it = parsed_items.begin(); it != parsed_items.end(); it++) {
		if (*it != "" && (*it)[0] == '[') {
			// Should be parsed as WML.
			try {
				config cfg;
				std::istringstream stream(*it);
				read(cfg, stream);
				config *child = cfg.child("ref");
				if (child != NULL) {
					handle_ref_cfg(*child);
				}
				child = cfg.child("img");
				if (child != NULL) {
					handle_img_cfg(*child);
				}
				child = cfg.child("bold");
				if (child != NULL) {
					handle_bold_cfg(*child);
				}
				child = cfg.child("italic");
				if (child != NULL) {
					handle_italic_cfg(*child);
				}
				child = cfg.child("header");
				if (child != NULL) {
					handle_header_cfg(*child);
				}
				child = cfg.child("jump");
				if (child != NULL) {
					handle_jump_cfg(*child);
				}
				child = cfg.child("format");
				if (child != NULL) {
					handle_format_cfg(*child);
				}
			}
			catch (config::error e) {
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

void help_text_area::handle_ref_cfg(const config &cfg) {
	const std::string dst = cfg["dst"];
	const std::string text = cfg["text"];
	const bool force = get_bool(cfg["force"]);
	bool show_ref = true;
	if (find_topic(toplevel_, dst) == NULL && !force) {
		show_ref = false;
		if (game_config::debug) {
			std::string msg = "Reference to non-existent topic '" + dst +
				"'. Please submit a bug report if you have not modified the game files yourself. Errornous config: ";
			msg += write(cfg);
			throw parse_error(msg);
		}
	}
	if (dst == "") {
		std::string msg = 
			"Ref markup must have dst attribute. Please submit a bug report if you have not modified the game files yourself. Errornous config: ";
		msg += write(cfg);
		throw parse_error(msg);
	}
	if (show_ref) {
		add_text_item(text, dst);
	}
	else {
		add_text_item(text);
	}
}

void help_text_area::handle_img_cfg(const config &cfg) {
	const std::string src = cfg["src"];
	const std::string align = cfg["align"];
	const bool floating = get_bool(cfg["float"]);
	bool box = true;
	if (cfg["box"] != "" && !get_bool(cfg["box"])) {
		box = false;
	}
	if (src == "") {
		throw parse_error("Img markup must have src attribute.");
	}
	add_img_item(src, align, floating, box);
}

void help_text_area::handle_bold_cfg(const config &cfg) {
	const std::string text = cfg["text"];
	if (text == "") {
		throw parse_error("Bold markup must have text attribute.");
	}
	add_text_item(text, "", -1, true);
}

void help_text_area::handle_italic_cfg(const config &cfg) {
	const std::string text = cfg["text"];
	if (text == "") {
		throw parse_error("Italic markup must have text attribute.");
	}
	add_text_item(text, "", -1, false, true);
}

void help_text_area::handle_header_cfg(const config &cfg) {
	const std::string text = cfg["text"];
	if (text == "") {
		throw parse_error("Header markup must have text attribute.");
	}
	add_text_item(text, "", title2_size, true);
}

void help_text_area::handle_jump_cfg(const config &cfg) {
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
		if (to < (unsigned)jump_to) {
			down_one_line();
		}
		jump_to = to;
	}
	if (jump_to > 0 && (int)jump_to < get_max_x(curr_loc_.first, curr_row_height_)) {
		curr_loc_.first = jump_to;
	}
}

void help_text_area::handle_format_cfg(const config &cfg) {
	const std::string text = cfg["text"];
	if (text == "") {
		throw parse_error("Format markup must have text attribute.");
	}
	const bool bold = get_bool(cfg["bold"]);
	const bool italic = get_bool(cfg["italic"]);
	int font_size = normal_font_size;
	if (cfg["font_size"] != "") {
		try {
			font_size = lexical_cast<int, std::string>(cfg["font_size"]);
		} catch (bad_lexical_cast) {
			throw parse_error("Invalid font_size in format markup.");
		}
	}
	SDL_Color color = string_to_color(cfg["color"]);
	add_text_item(text, "", font_size, bold, italic, color);
}

void help_text_area::add_text_item(const std::string text, const std::string ref_dst,
								   int _font_size, bool bold, bool italic,
								   SDL_Color text_color) {
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
		add_text_item(rest_text, ref_dst, _font_size, bold, italic, text_color);
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
		add_text_item(text, ref_dst, _font_size, bold, italic, text_color);
	}
	else {
		std::vector<std::string> parts = split_in_width(text, font_size, remaining_width);
		std::string first_part = parts.front();
		// Always override the color if we have a cross reference.
		const SDL_Color color = ref_dst == "" ? text_color : font::YELLOW_COLOUR;
		surface surf(font::get_rendered_text(first_part, font_size, color, state));
		if (!surf.null())
			add_item(item(surf, curr_loc_.first, curr_loc_.second, first_part, ref_dst));
		if (parts.size() > 1) {
			// Parts remain, remove the first part from the string and
			// add the remaining parts.
			std::string s = text;
			s.erase(0, first_part.size());
			if (s.length() < 1) {
				return;
			}
			const std::string first_word_before = get_first_word(s);
			const std::string first_word_after = get_first_word(remove_first_space(s));
			//std::cout << "before: '" << first_word_before << "'\n"
			//		  << "after: '" << first_word_after << "'\n"
			//		  << "before linewidth: " << font::line_width(first_word_before, font_size)
			//		  << "\nafter linewidth: " << font::line_width(first_word_after, font_size)
			//		  << "\nremaining width: " << get_remaining_width() << std::endl;
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
			add_text_item(s, ref_dst, _font_size, bold, italic, text_color);
				
		}
	}
}

void help_text_area::add_img_item(const std::string path, const std::string alignment,
								  const bool floating, const bool box) {
	surface surf(image::get_image(path, image::UNSCALED));
	if (surf == NULL) {
		std::stringstream msg;
		msg << "Image " << path << " could not be loaded.";
		std::cerr << msg.str();
		return;
	}
	ALIGNMENT align = str_to_align(alignment);
	if (align == HERE && floating) {
		std::cerr << "Floating image with align HERE, aligning left." << std::endl;
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

int help_text_area::get_y_for_floating_img(const int width, const int x, const int desired_y) {
	int min_y = desired_y;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); it++) {
		const item& itm = *it;
		if (itm.floating) {
			if ((itm.rect.x + itm.rect.w > x && itm.rect.x < x + width)
				|| (itm.rect.x > x && itm.rect.x < x + width)) {
				min_y = maximum<int>(min_y, itm.rect.y + itm.rect.h);
			}
		}
	}
	return min_y;
}

int help_text_area::get_min_x(const int y, const int height) {
	int min_x = 0;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); it++) {
		const item& itm = *it;
		if (itm.floating) {
			if (itm.rect.y < y + height && itm.rect.y + itm.rect.h > y && itm.align == LEFT) {
				min_x = maximum<int>(min_x, itm.rect.w + 5);
			}
		}
	}
	return min_x;
}

int help_text_area::get_max_x(const int y, const int height) {
	int text_width = inner_location().w;
	int max_x = text_width;
	for (std::list<item>::const_iterator it = items_.begin(); it != items_.end(); it++) {
		const item& itm = *it;
		if (itm.floating) {
			if (itm.rect.y < y + height && itm.rect.y + itm.rect.h > y) {
				if (itm.align == RIGHT) {
					max_x = minimum<int>(max_x, text_width - itm.rect.w - 5);
				} else if (itm.align == MIDDLE) {
					max_x = minimum<int>(max_x, text_width / 2 - itm.rect.w / 2 - 5);
				}
			}
		}
	}
	return max_x;
}

void help_text_area::add_item(const item &itm) {
	items_.push_back(itm);
	if (!itm.floating) {
		curr_loc_.first += itm.rect.w;
		curr_row_height_ = maximum<int>(itm.rect.h, curr_row_height_);
		contents_height_ = maximum<int>(contents_height_, curr_loc_.second + curr_row_height_);
		last_row_.push_back(&items_.back());
	}
	else {
		if (itm.align == LEFT) {
			curr_loc_.first = itm.rect.w + 5;
		}
		contents_height_ = maximum<int>(contents_height_, itm.rect.y + itm.rect.h);
	}
}
	
	
help_text_area::ALIGNMENT help_text_area::str_to_align(const std::string &s) {
	const std::string cmp_str = to_lower(s);
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
	msg << "Invalid alignment string: '" << s << "'";
	throw parse_error(msg.str());
}
	
void help_text_area::down_one_line() {
	adjust_last_row();
	last_row_.clear();
	curr_loc_.second += curr_row_height_ + (curr_row_height_ == min_row_height_ ? 0 : 2);
	curr_row_height_ = min_row_height_;
	contents_height_ = maximum<int>(curr_loc_.second + curr_row_height_, contents_height_);
	curr_loc_.first = get_min_x(curr_loc_.second, curr_row_height_);
}

void help_text_area::adjust_last_row() {
	for (std::list<item *>::iterator it = last_row_.begin(); it != last_row_.end(); it++) {
		item &itm = *(*it);
		const int gap = curr_row_height_ - itm.rect.h;
		itm.rect.y += gap / 2;
	}
}

int help_text_area::get_remaining_width() {
	const int total_w = (int)get_max_x(curr_loc_.second, curr_row_height_);
	return total_w - curr_loc_.first;
}

void help_text_area::draw_contents() {
	SDL_Rect const &loc = inner_location();
	bg_restore();
	surface const screen = video().getSurface();
	clip_rect_setter clip_rect_set(screen, loc);
	for(std::list<item>::const_iterator it = items_.begin(), end = items_.end(); it != end; ++it) {
		SDL_Rect dst = it->rect;
		dst.y -= get_position();
		if (dst.y < (int)loc.h && dst.y + it->rect.h > 0) {
			dst.x += loc.x;
			dst.y += loc.y;
			if (it->box) {
				for (int i = 0; i < box_width; i++) {
					draw_rectangle(dst.x, dst.y, it->rect.w - i * 2, it->rect.h - i * 2,
					                    0, screen);
					dst.x++;
					dst.y++;
				}
			}
			SDL_BlitSurface(it->surf, NULL, screen, &dst);
		}
	}
	update_rect(loc);
}

void help_text_area::scroll(int) {
	// Nothing will be done on the actual scroll event. The scroll
	// position is checked when drawing instead and things drawn
	// accordingly.
	set_dirty(true);
}

bool help_text_area::item_at::operator()(const item& item) const {
	return point_in_rect(x_, y_, item.rect);
}

std::string help_text_area::ref_at(const int x, const int y) {
	const int local_x = x - location().x;
	const int local_y = y - location().y;
	if (local_y < (int)height() && local_y > 0) {
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



help_browser::help_browser(display &disp, const section &toplevel)
	: gui::widget(disp.video()), disp_(disp), menu_(disp.video(), toplevel),
	  text_area_(disp.video(), toplevel), toplevel_(toplevel), ref_cursor_(false),
	  back_button_(disp.video(), _("< Back"), gui::button::TYPE_PRESS),
	  forward_button_(disp.video(), _("Forward >"), gui::button::TYPE_PRESS),
	  shown_topic_(NULL) {
	// Hide the buttons at first since we do not have any forward or
	// back topics at this point. They will be unhidden when history
	// appears.
	back_button_.hide(true);
	forward_button_.hide(true);
	// Set sizes to some default values.
	set_measurements(font::relative_size(400), font::relative_size(500));
}

void help_browser::adjust_layout() {
  const int menu_buttons_padding = font::relative_size(10);
	const int menu_y = location().y;
	const int menu_x = location().x;
	const int menu_w = font::relative_size(250);
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

void help_browser::update_location(SDL_Rect const &) {
	adjust_layout();
}

void help_browser::process_event() {
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
								   std::deque<const topic *> &to) {
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


void help_browser::handle_event(const SDL_Event &event) {
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
					msg << "Reference to unknown topic: '" << ref << "'.";
					gui::show_dialog(disp_, NULL, "", msg.str(), gui::OK_ONLY);
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

void help_browser::update_cursor() {
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


const topic *find_topic(const section &sec, const std::string &id) {
	topic_list::const_iterator tit =
		std::find_if(sec.topics.begin(), sec.topics.end(), has_id(id));
	if (tit != sec.topics.end()) {
		return &(*tit);
	}
	section_list::const_iterator sit;
	for (sit = sec.sections.begin(); sit != sec.sections.end(); sit++) {
		const topic *t = find_topic(*(*sit), id);
		if (t != NULL) {
			return t;
		}
	}
	return NULL;
}

const section *find_section(const section &sec, const std::string &id) {
	section_list::const_iterator sit =
		std::find_if(sec.sections.begin(), sec.sections.end(), has_id(id));
	if (sit != sec.sections.end()) {
		return *sit;
	}
	for (sit = sec.sections.begin(); sit != sec.sections.end(); sit++) {
		const section *s = find_section(*(*sit), id);
		if (s != NULL) {
			return s;
		}
	}
	return NULL;
}

void help_browser::show_topic(const std::string &topic_id) {
	std::cerr << "showing topic '" << topic_id << "'\n";
	const topic *t = find_topic(toplevel_, topic_id);
	if (t != NULL) {
		show_topic(*t);
	}
	else {
		std::cerr << "Help browser tried to show topic with id '" << topic_id
				  << "' but that topic could not be found." << std::endl;
	}
}

void help_browser::show_topic(const topic &t, bool save_in_history) {
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

std::vector<std::string> parse_text(const std::string &text) {
	std::vector<std::string> res;
	bool last_char_escape = false;
	const char escape_char = '\\';
	std::stringstream ss;
	size_t pos;
	enum { ELEMENT_NAME, OTHER } state = OTHER;
	for (pos = 0; pos < text.size(); pos++) {
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
					std::string msg = "Errornous / in element name.";
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

std::string convert_to_wml(const std::string &element_name, const std::string &contents) {
	std::stringstream ss;
	bool in_quotes = false;
	bool last_char_escape = false;
	const char escape_char = '\\';
	std::vector<std::string> attributes;
	// Find the different attributes. No checks are made for the equal
	// sign or something like that. Attributes are just separated by
	// spaces or newlines. Attributes that contain spaces must be in
	// single quotes.
	for (size_t pos = 0; pos < contents.size(); pos++) {
		const char c = contents[pos];
		if (c == escape_char && !last_char_escape) {
			last_char_escape = true;
		}
		else {
			if (c == '\'' && !last_char_escape) {
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
		 it != attributes.end(); it++) {
		ss << *it << "\n";
	}
	ss << "[/" << element_name << "]\n";
	return ss.str();
}

bool get_bool(const std::string &s) {
	const std::string cmp_str = to_lower(s);
	if (cmp_str == "yes" || cmp_str == "true" || cmp_str == "1" || cmp_str == "on") {
		return true;
	}
	return false;
}

SDL_Color string_to_color(const std::string &s) {
	const std::string cmp_str = to_lower(s);
	if (cmp_str == "green") {
		return font::GOOD_COLOUR;
	}
	if (cmp_str == "red") {
		return font::BAD_COLOUR;
	}
	if (cmp_str == "black") {
		return font::BLACK_COLOUR;
	}
	if (cmp_str == "yellow") {
		return font::YELLOW_COLOUR;
	}
	return font::NORMAL_COLOUR;
}

std::vector<std::string> split_in_width(const std::string &s, const int font_size,
										const unsigned width) {
	std::string wrapped = font::word_wrap_text(s, font_size, width);
	std::vector<std::string> parts = utils::split(wrapped, '\n', 0);
	return parts;
}

std::string remove_first_space(const std::string& text) {
  if (text.length() > 0 && text[0] == ' ') {
    return text.substr(1);
  }
  return text;
}

std::string to_lower(const std::string &s) {
	std::string lower_string;
	lower_string.resize(s.size());
	std::transform(s.begin(), s.end(), lower_string.begin(), tolower);
	return lower_string;
}

std::string cap(const std::string &s) {
	if (s.size() > 0) {
		utils::utf8_iterator itor(s);
		std::string res = utils::wchar_to_string(towupper(*itor));
		res.append(itor.substr().second, s.end());
		return res;
	}
	return s;
}
	
std::string escape(const std::string &s) {
	std::string res = s;
	if(!res.empty()) {
		std::string::size_type pos = 0;
		do {
			pos = res.find_first_of("'\\", pos);
			if(pos != std::string::npos) {
				res.insert(pos, 1, '\\');
				pos += 2;
			}
		} while(pos < res.size() && pos != std::string::npos);
	}
	return res;
}
		
std::string get_first_word(const std::string &s) {
	if (s == "") {
		return s;
	}
	size_t first_word_start = s.find_first_not_of(" ");
	if (first_word_start == std::string::npos) {
		first_word_start = 0;
	}
	size_t first_word_end = s.find_first_of(" \n", first_word_start);
	if (first_word_end == std::string::npos || first_word_end == first_word_start) {
		// Either this word contains no spaces/newlines, or it consists
		// of only spaces and newlines. In either case, use the whole
		// chunk as a word.
		first_word_end = s.size();
	}
	const std::string first_word = s.substr(0, first_word_end);
	return first_word;
}

void show_help(display &disp, std::string show_topic, int xloc, int yloc) {
	show_help(disp, toplevel, show_topic, xloc, yloc);
}

void show_help(display &disp, const std::vector<std::string> &topics_to_show,
			   const std::vector<std::string> &sections_to_show, const std::string show_topic,
			   int xloc, int yloc) {
	section to_show;
	std::vector<std::string>::const_iterator it;
	for (it = topics_to_show.begin(); it != topics_to_show.end(); it++) {
		// Check both the visible toplevel and the hidden sections.
		const topic *t = find_topic(toplevel, *it);
		t = t == NULL ? find_topic(hidden_sections, *it) : t;
		if (t != NULL) {
			to_show.topics.push_back(*t);
		}
		else {
			std::cerr << "Warning: topic with id " << *it << " does not exist." << std::endl;
		}
	}
	for (it = sections_to_show.begin(); it != sections_to_show.end(); it++) {
		const section *s = find_section(toplevel, *it);
		s = s == NULL ? find_section(hidden_sections, *it) : s;
		if (s != NULL) {
			to_show.add_section(*s);
		}
		else {
			std::cerr << "Warning: section with id " << *it << " does not exist." << std::endl;
		}
	}
	show_help(disp, to_show, show_topic, xloc, yloc);
}

/// Open a help dialog using a toplevel other than the default.
void show_help(display &disp, const section &toplevel_sec, const std::string show_topic,
			   int xloc, int yloc) {
	const events::event_context dialog_events_context;
	const gui::dialog_manager manager;
	const events::resize_lock prevent_resizing;

	CVideo& screen = disp.video();
	surface const scr = screen.getSurface();

	const int width = minimum<int>(font::relative_size(900), scr->w - font::relative_size(20));
	const int height = minimum<int>(font::relative_size(800), scr->h - font::relative_size(150));
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
	surface_restorer restorer;
	gui::draw_dialog(xloc, yloc, width, height, disp.video(), _("The Battle for Wesnoth Help"),
					 NULL, &buttons_ptr, &restorer);

	if (preferences::encountered_units().size() != size_t(last_num_encountered_units) ||
	    preferences::encountered_terrains().size() != size_t(last_num_encountered_terrains)) {
		// More units or terrains encountered, update the contents.
		last_num_encountered_units = preferences::encountered_units().size();
		last_num_encountered_terrains = preferences::encountered_terrains().size();
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
				 button_it != buttons_ptr.end(); button_it++) {
				if ((*button_it)->pressed()) {
					// There is only one button, close.
					return;
				}
			}
			disp.flip();
			SDL_Delay(10);
		}
	}
	catch (parse_error e) {
		std::stringstream msg;
		msg << "Parse error when parsing help text: '" << e.message << "'";
		gui::show_dialog(disp, NULL, "", msg.str(), gui::OK_ONLY);
	}
}

} // End namespace help.
