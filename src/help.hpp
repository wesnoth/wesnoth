/*
   Copyright (C) 2003 by David White <davidnwhite@optusnet.com.au>
   Part of the Battle for Wesnoth Project http://wesnoth.whitevine.net

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#ifndef HELP_HPP_INCLUDED
#define HELP_HPP_INCLUDED

#include "config.hpp"
#include "display.hpp"
#include "sdl_utils.hpp"
#include "widgets/button.hpp"
#include "widgets/menu.hpp"
#include "widgets/scrollbar.hpp"

#include "widgets/widget.hpp"

#include <set>
#include <map>
#include <list>

namespace help {

struct help_manager {
	help_manager(const config *help_config);
	~help_manager();
};

struct section;

typedef std::vector<section *> section_list;

/// A topic contains a title, an id and some text.
struct topic {
	topic(const std::string _title, const std::string _id, const std::string _text)
		: title(_title), id(_id), text(_text) {}
	topic() : title(""), id(""), text("") {}
	/// Two topics are equal if their IDs are equal.
	bool operator==(const topic &) const;
	/// Comparison on the ID.
	bool operator<(const topic &) const;
	std::string title, id, text;
};

typedef std::list<topic> topic_list;

/// A section contains topics and sections along with title and ID.
struct section {
	section(const std::string _title, const std::string _id, const topic_list &_topics,
			const std::vector<section> &_sections);
	section() : title(""), id("") {}
	section(const section&);
	section& operator=(const section&);
	~section();
	/// Two sections are equal if their IDs are equal.
	bool operator==(const section &) const;
	/// Comparison on the ID.
	bool operator<(const section &) const;
	
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
private:
	const std::string id_;
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
	help_menu(display& disp, const section &toplevel, int max_height=-1);
	void bg_backup();
	void bg_restore();
	int process(int x, int y, bool button,bool up_arrow,bool down_arrow,
	            bool page_up, bool page_down, int select_item=-1);

	/// Overloaded from menu so that the background can be saved.
	void set_loc(int x, int y);
	/// Overloaded from menu so that the background can be saved.
	void set_width(int w);
	/// Overloaded from menu so that the background can be saved.
	void set_max_height(const int new_height);

	/// Make the topic the currently selected one, and expand all
	/// sections that need to be expanded to show it.
	void select_topic(const topic &t);

	/// If a topic has been chosen, return that topic, otherwise
	/// NULL. If one topic is returned, it will not be returned again,
	/// if it is not re-chosen.
	const topic *chosen_topic();
protected:
	void handle_event(const SDL_Event &event);

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
	
	/// Internal recursive thingie.
	bool select_topic_internal(const topic &t, const section &sec);

	display &disp_;
	std::vector<visible_item> visible_items_;
	const section &toplevel_;
	std::set<const section*> expanded_; 
	surface_restorer restorer_;
	SDL_Rect rect_;
	topic const *chosen_topic_;
	int internal_width_;
	visible_item selected_item_;
};

/// Thrown when the help system fails to parse something.
struct parse_error {
	parse_error(const std::string& msg) : message(msg) {}
	std::string message;
};

/// The area where the content is shown in the help browser.
class help_text_area : public gui::widget, public gui::scrollable {
public:
	help_text_area(display &disp, const section &toplevel);
	/// Display the topic.
	void show_topic(const topic &t);

	/// Return the ID that is crossreferenced at the (screen)
	/// coordinates x, y. If no cross-reference is there, return the
	/// empty string.
	std::string ref_at(const int x, const int y);

	/// Return the width of the area where text fit.
	int text_width() const;

	void scroll(int pos);

	void set_dirty(bool dirty);

	/// Scroll the contents up an amount. If how_much is below zero the
	/// amount will depend on the height.
	void scroll_up(const int how_much=-1);

	/// Scroll the contents down an amount. If how_much is below zero
	/// the amount will depend on the height.
	void scroll_down(const int how_much=-1);
private:
	enum ALIGNMENT {LEFT, MIDDLE, RIGHT, HERE};
	/// Convert a string to an alignment. Throw parse_error if
	/// unsuccesful.
	ALIGNMENT str_to_align(const std::string &s);

	/// An item that is displayed in the text area. Contains the surface
	/// that should be blitted along with some other information.
	struct item {

		item(shared_sdl_surface surface, int x, int y, const std::string text="",
			 const std::string reference_to="", bool floating=false,
			 ALIGNMENT alignment=HERE);

		item(shared_sdl_surface surface, int x, int y,
			 bool floating, ALIGNMENT=HERE);

		/// Relative coordinates of this item.
		SDL_Rect rect;
		shared_sdl_surface surf;

		// If this item contains text, this will contain that text.
		std::string text; 

		// If this item contains a cross-reference, this is the id
		// of the referenced topic.
		std::string ref_to; 

		// If this item is floating, that is, if things should be filled
		// around it.
		bool floating;
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

	/// Update the vector with items, creating surfaces for everything
	/// and putting things where they belong. parsed_items should be a
	/// vector with parsed strings, such as parse_text returns.
	void set_items(const std::vector<std::string> &parsed_items,
				   const std::string &title);

	// Create appropriate items from configs. Items will be added to the
	// internal vector. These methods check that the necessary
	// attributes are specified.
	void handle_ref_cfg(const config &cfg);
	void handle_img_cfg(const config &cfg);
	void handle_bold_cfg(const config &cfg);
	void handle_italic_cfg(const config &cfg);
	void handle_header_cfg(const config &cfg);
	void handle_jump_cfg(const config &cfg);

	void handle_event(const SDL_Event &event);
	void draw();
	void process();

	/// Update the scrollbar to take account for the current items.
	void update_scrollbar();

	/// Get the current amount of scrolling that should be
	/// added/substracted from the locations to get the desired effect.
	unsigned get_scroll_offset() const;

	/// Add an item with text. If ref_dst is something else than the
	/// empty string, the text item will be underlined to show that it
	/// is a cross-reference. The item will also remember what the
	/// reference points to. If font_size is below zero, the default
	/// will be used.
	void add_text_item(const std::string text, const std::string ref_dst="",
					   int font_size=-1, bool bold=false, bool italic=false);

	/// Add an image item with the specified attributes.
	void add_img_item(const std::string path, const std::string alignment, const bool floating);

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
	display &disp_;
	const section &toplevel_;
	topic const *shown_topic_;
	const int title_spacing_;
	// The current input location when creating items.
	std::pair<int, int> curr_loc_;
	const unsigned min_row_height_;
	unsigned curr_row_height_;
	gui::scrollbar scrollbar_;
	bool use_scrollbar_;
	gui::button uparrow_, downarrow_;
	/// The height of all items in total.
	int contents_height_;
};


/// A help browser widget.
class help_browser : public gui::widget {
public:
	help_browser(display &disp, section &toplevel);

	// Overloaded from widget so that the layout may be adjusted to fit
	// the new dimensions.
	void set_location(const SDL_Rect& rect);
	void set_location(int x, int y);
	void set_width(int w);
	void set_height(int h);

	void set_dirty(bool dirty);
	void adjust_layout();

	void process();
	/// Display the topic with the specified identifier. Open the menu
	/// on the right location and display the topic in the text area.
	void show_topic(const std::string &topic_id);

private:
	void handle_event(const SDL_Event &event);
	display &disp_;
	help_menu menu_;
	help_text_area text_area_;
	const section &toplevel_;
};

/// Parse a help config, return the top level section. Return an empty
/// section if cfg is NULL.
section parse_config(const config *cfg); 

/// Search for the topic with the specified identifier in the section
/// and it's subsections. Return the found topic, or NULL if none could
/// be found.
const topic *find_topic(const section &sec, const std::string &id);

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

/// Make a best effort to word wrap s. All parts are less than width.
std::vector<std::string> split_in_width(const std::string &s, const int font_size, 
										const unsigned width);

std::string remove_first_space(const std::string& text);

/// Return a lowercase copy of s.
std::string to_lower(const std::string &s);

/// Return the first word in s, not removing any spaces in the start of
/// it.
std::string get_first_word(const std::string &s);

/// Open a help dialog. The help topic will have the topic with id
/// show_topic open if it is not the empty string.
void show_help(display &disp, const std::string show_topic="", int xloc=-1, int yloc=-1);

} // End namespace help.

#endif
