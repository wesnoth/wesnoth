
#ifndef HELP_IMPL_INCLUDED
#define HELP_IMPL_INCLUDED

#include "display.hpp"
#include "game_config_manager.hpp" //solely to get terrain info from in some circumstances
#include "game_errors.hpp"
#include "map.hpp"
#include "resources.hpp"
#include "terrain_type_data.hpp"
#include "widgets/widget.hpp"

#include "about.hpp"
#include "construct_dialog.hpp"
#include "display.hpp"
#include "display_context.hpp"
#include "exceptions.hpp"
#include "game_preferences.hpp"
#include "gettext.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "help_impl.hpp"
#include "hotkey/hotkey_command.hpp"
#include "language.hpp"
#include "log.hpp"
#include "map.hpp"
#include "marked-up_text.hpp"
#include "resources.hpp"
#include "sound.hpp"
#include "unit.hpp"
#include "unit_helper.hpp"
#include "wml_separators.hpp"
#include "serialization/parser.hpp"
#include "time_of_day.hpp"
#include "tod_manager.hpp"

#include <boost/foreach.hpp>

#include <queue>


#include <string>
#include <vector>
#include <SDL.h>

namespace help {

/// Generate the help contents from the configurations given to the
/// manager.
void generate_contents();

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

// Generator stuff below. Maybe move to a separate file? This one is
// getting crowded. Dunno if much more is needed though so I'll wait and
// see.

/// Dispatch generators to their appropriate functions.
void generate_sections(const config *help_cfg, const std::string &generator, section &sec, int level);
std::vector<topic> generate_topics(const bool sort_topics,const std::string &generator);
std::string generate_topic_text(const std::string &generator, const config *help_cfg,
const section &sec, const std::vector<topic>& generated_topics);
std::string generate_about_text();
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

/// Return the color the string represents. Return font::NORMAL_COLOR if
/// the string is empty or can't be matched against any other color.
SDL_Color string_to_color(const std::string &s);

/// Make a best effort to word wrap s. All parts are less than width.
std::vector<std::string> split_in_width(const std::string &s, const int font_size, const unsigned width);

std::string remove_first_space(const std::string& text);

/// Prepend all chars with meaning inside attributes with a backslash.
inline std::string escape(const std::string &s)
{
	return utils::escape(s, "'\\");
}

/// Return the first word in s, not removing any spaces in the start of
/// it.
std::string get_first_word(const std::string &s);

/// Load the appropriate terrain types data to use
inline tdata_cache load_terrain_types_data() {
	if (display::get_singleton()) {
		return display::get_singleton()->get_disp_context().map().tdata();
	} else if (resources::config_manager){
		return resources::config_manager->terrain_types();
	} else {
		return tdata_cache();
	}
}

extern const config *game_cfg;
// The default toplevel.
extern help::section toplevel;
// All sections and topics not referenced from the default toplevel.
extern help::section hidden_sections;

extern int last_num_encountered_units;
extern int last_num_encountered_terrains;
extern bool last_debug_state;

extern config dummy_cfg;
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
extern const std::string indentation_img;
// The topic to open by default when opening the help dialog.
extern const std::string default_show_topic;
extern const std::string unknown_unit_topic;
extern const std::string unit_prefix;
extern const std::string terrain_prefix;
extern const std::string race_prefix;
extern const std::string faction_prefix;
extern const std::string era_prefix;
extern const std::string variation_prefix;

// id starting with '.' are hidden
inline std::string hidden_symbol(bool hidden = true) {
	return (hidden ? "." : "");
}

inline bool is_visible_id(const std::string &id) {
	return (id.empty() || id[0] != '.');
}

/// Return true if the id is valid for user defined topics and
/// sections. Some IDs are special, such as toplevel and may not be
/// be defined in the config.
inline bool is_valid_id(const std::string &id) {
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

	typedef std::vector<std::vector<std::pair<std::string, unsigned int > > > table_spec;
	// Create a table using the table specs. Return markup with jumps
	// that create a table. The table spec contains a vector with
	// vectors with pairs. The pairs are the markup string that should
	// be in a cell, and the width of that cell.
inline std::string generate_table(const table_spec &tab, const unsigned int spacing=font::relative_size(20))
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
inline unsigned image_width(const std::string &filename)
	{
		image::locator loc(filename);
		surface surf(image::get_image(loc));
		if (surf != NULL) {
			return surf->w;
		}
		return 0;
	}

inline void push_tab_pair(std::vector<std::pair<std::string, unsigned int> > &v, const std::string &s)
	{
		v.push_back(std::make_pair(s, font::line_width(s, normal_font_size)));
	}


} // end namespace help

#endif
