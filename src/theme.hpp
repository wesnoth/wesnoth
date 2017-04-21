/*
   Copyright (C) 2003 - 2017 by David White <dave@whitevine.net>
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
 *  Definitions related to theme-support.
 */

#ifndef THEME_HPP_INCLUDED
#define THEME_HPP_INCLUDED

#include "config.hpp"
#include "generic_event.hpp"

#include <SDL_video.h>

struct _rect { size_t x1,y1,x2,y2; };

struct theme_info
{
	std::string id;
	t_string name;
	t_string description;
};

class theme
{

	class object
	{
	public:
		object();
		object(const config& cfg);
		virtual ~object() { }

		SDL_Rect& location(const SDL_Rect& screen) const;
		const SDL_Rect& get_location() const { return loc_; }
		const std::string& get_id() const { return id_; }

		// This supports relocating of theme elements ingame.
		// It is needed for [change] tags in theme WML.
		void modify_location(const _rect& rect);
		void modify_location(std::string rect_str, SDL_Rect rect_ref);

		// All on-screen objects have 'anchoring' in the x and y dimensions.
		// 'fixed' means that they have fixed co-ordinates and don't move.
		// 'top anchored' means they are anchored to the top (or left) side
		// of the screen - the top (or left) edge stays a constant distance
		// from the top of the screen.
		// 'bottom anchored' is the inverse of top anchored.
		// 'proportional' means the location and dimensions change
		// proportionally to the screen size.
		enum ANCHORING { FIXED, TOP_ANCHORED, PROPORTIONAL, BOTTOM_ANCHORED };

	private:
		bool location_modified_;
		std::string id_;
		SDL_Rect loc_;
		mutable SDL_Rect relative_loc_;
		mutable SDL_Rect last_screen_;

		ANCHORING xanchor_, yanchor_;

		static ANCHORING read_anchor(const std::string& str);
	};

	struct border_t
	{

		border_t();
		border_t(const config& cfg);

		double size;

		std::string background_image;
		std::string tile_image;

		std::string corner_image_top_left;
		std::string corner_image_bottom_left;

		std::string corner_image_top_right_odd;
		std::string corner_image_top_right_even;

		std::string corner_image_bottom_right_odd;
		std::string corner_image_bottom_right_even;

		std::string border_image_left;
		std::string border_image_right;

		std::string border_image_top_odd;
		std::string border_image_top_even;

		std::string border_image_bottom_odd;
		std::string border_image_bottom_even;

	};

public:

	class label : public object
	{
	public:
		label();
		explicit label(const config& cfg);

		using object::location;

		const std::string& text() const { return text_; }
		void set_text(const std::string& text) { text_ = text; }
		const std::string& icon() const { return icon_; }

		bool empty() const { return text_.empty() && icon_.empty(); }

		size_t font_size() const { return font_; }
		Uint32 font_rgb() const { return font_rgb_; }
		bool font_rgb_set() const { return font_rgb_set_; }
	private:
		std::string text_, icon_;
		size_t font_;
		bool font_rgb_set_;
		Uint32 font_rgb_;
	};

	class status_item : public object
	{
	public:

		explicit status_item(const config& cfg);

		using object::location;

		const std::string& prefix() const { return prefix_; }
		const std::string& postfix() const { return postfix_; }

		// If the item has a label associated with it, Show where the label is
		const label* get_label() const { return label_.empty() ? nullptr : &label_; }

		size_t font_size() const { return font_; }
		Uint32 font_rgb() const { return font_rgb_; }
		bool font_rgb_set() const { return font_rgb_set_; }

	private:
		std::string prefix_, postfix_;
		label label_;
		size_t font_;
		bool font_rgb_set_;
		Uint32 font_rgb_;
	};

	class panel : public object
	{
	public:
		explicit panel(const config& cfg);

		using object::location;

		const std::string& image() const { return image_; }

	private:
		std::string image_;
	};

	class action : public object
	{
	public:
		action();
		explicit action(const config& cfg);

		using object::location;

		bool is_context() const  { return context_; }

		const std::string& title() const { return title_; }

		const std::string tooltip(size_t index) const;

		const std::string& type() const { return type_; }

		const std::string& image() const { return image_; }

		const std::string& overlay() const { return overlay_; }

		const std::vector<std::string>& items() const { return items_; }

		void set_title(const std::string& new_title) { title_ = new_title; }
	private:
		bool context_, auto_tooltip_, tooltip_name_prepend_;
		std::string title_, tooltip_, image_, overlay_,  type_;
		std::vector<std::string> items_;
	};

	class slider : public object
	{
	public:
		slider();
		explicit slider(const config& cfg);

		using object::location;

		const std::string& title() const { return title_; }

		const std::string& tooltip() const { return tooltip_; }

		const std::string& image() const { return image_; }

		const std::string& overlay() const { return overlay_; }

		bool black_line() const { return black_line_; }

		void set_title(const std::string& new_title) { title_ = new_title; }
	private:
		std::string title_, tooltip_, image_, overlay_;
		bool black_line_;
	};

	class menu : public object
	{
	public:
		menu();
		explicit menu(const config& cfg);

		using object::location;

		bool is_button() const { return button_; }

		bool is_context() const  { return context_; }

		const std::string& title() const { return title_; }

		const std::string& tooltip() const { return tooltip_; }

		const std::string& image() const { return image_; }

		const std::string& overlay() const { return overlay_; }

		const std::vector<config>& items() const { return items_; }

		void set_title(const std::string& new_title) { title_ = new_title; }
	private:
		bool button_;
		bool context_;
		std::string title_, tooltip_, image_, overlay_;
		std::vector<config> items_;
	};

	explicit theme(const config& cfg, const SDL_Rect& screen);
	bool set_resolution(const SDL_Rect& screen);
	void modify(const config &cfg);

	const std::vector<panel>& panels() const { return panels_; }
	const std::vector<label>& labels() const { return labels_; }
	const std::vector<menu>& menus() const { return menus_; }
	const std::vector<slider>& sliders() const { return sliders_; }
	const std::vector<action>& actions() const { return actions_; }

	const menu* context_menu() const
		{ return context_.is_context() ? &context_ : nullptr; }

	//refresh_title2 changes the title of a menu entry, identified by id.
	//If no menu entry is found, an empty menu object is returned.
	object* refresh_title(const std::string& id, const std::string& new_title);
	object* refresh_title2(const std::string& id, const std::string& title_tag);
	void modify_label(const std::string& id, const std::string& text);

	const status_item* get_status_item(const std::string& item) const;
	const menu *get_menu_item(const std::string &key) const;
	const action* get_action_item(const std::string &key) const;

	const SDL_Rect& main_map_location(const SDL_Rect& screen) const
		{ return main_map_.location(screen); }
	const SDL_Rect& mini_map_location(const SDL_Rect& screen) const
		{ return mini_map_.location(screen); }
	const SDL_Rect& unit_image_location(const SDL_Rect& screen) const
		{ return unit_image_.location(screen); }
	const SDL_Rect& palette_location(const SDL_Rect& screen) const
		{ return palette_.location(screen); }

    static void set_known_themes(const config* cfg);
    static std::vector<theme_info> get_known_themes();

	const border_t& border() const { return border_; }

	events::generic_event& theme_reset_event() { return theme_reset_event_; }

private:
	theme::object& find_element(const std::string& id);
	void add_object(const config& cfg);
	void remove_object(const std::string& id);
	void set_object_location(theme::object& element, std::string rect_str, std::string ref_id);

	//notify observers that the theme has been rebuilt completely
	//atm this is used for replay_controller to add replay controls to the standard theme
	events::generic_event theme_reset_event_;

	static std::map<std::string, config> known_themes;
	std::string cur_theme;
	config cfg_;
	std::vector<panel> panels_;
	std::vector<label> labels_;
	std::vector<menu> menus_;
	std::vector<action> actions_;
	std::vector<slider> sliders_;

	menu context_;
	action action_context_;

	std::map<std::string,status_item> status_;

	object main_map_, mini_map_, unit_image_, palette_;

	border_t border_;
};

#endif
