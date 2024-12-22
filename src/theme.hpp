/*
	Copyright (C) 2003 - 2024
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
 *  @file
 *  Definitions related to theme-support.
 */

#pragma once

#include "color.hpp"
#include "config.hpp"
#include "generic_event.hpp"
#include "global.hpp"
#include "sdl/rect.hpp"

#include <memory>
#include <SDL2/SDL_rect.h>

class game_config_view;

struct _rect { std::size_t x1,y1,x2,y2; };

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
		object(std::size_t sw, std::size_t sh, const config& cfg);
		virtual ~object() { }

		virtual rect& location(const SDL_Rect& screen) const;
		const rect& get_location() const { return loc_; }
		const std::string& get_id() const { return id_; }

		// This supports relocating of theme elements ingame.
		// It is needed for [change] tags in theme WML.
		void modify_location(const _rect& rect);
		void modify_location(const std::string& rect_str, SDL_Rect rect_ref);

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
		rect loc_;
		mutable rect relative_loc_;
		mutable rect last_screen_;

		ANCHORING xanchor_, yanchor_;
		std::size_t spec_width_, spec_height_;

		static ANCHORING read_anchor(const std::string& str);
	};

	struct border_t
	{

		border_t();
		border_t(const config& cfg);

		double size;

		std::string background_image;
		std::string tile_image;

		bool show_border;
	};

public:

	class label : public object
	{
	public:
		label();
		explicit label(std::size_t sw, std::size_t sh, const config& cfg);

		using object::location;

		const std::string& text() const { return text_; }
		void set_text(const std::string& text) { text_ = text; }
		const std::string& icon() const { return icon_; }

		bool empty() const { return text_.empty() && icon_.empty(); }

		std::size_t font_size() const { return font_; }
		color_t font_rgb() const { return font_rgb_; }
		bool font_rgb_set() const { return font_rgb_set_; }
	private:
		std::string text_, icon_;
		std::size_t font_;
		bool font_rgb_set_;
		color_t font_rgb_;
	};

	class status_item : public object
	{
	public:

		explicit status_item(std::size_t sw, std::size_t sh, const config& cfg);

		using object::location;

		const std::string& prefix() const { return prefix_; }
		const std::string& postfix() const { return postfix_; }

		// If the item has a label associated with it, Show where the label is
		const label* get_label() const { return label_.empty() ? nullptr : &label_; }

		std::size_t font_size() const { return font_; }
		color_t font_rgb() const { return font_rgb_; }
		bool font_rgb_set() const { return font_rgb_set_; }

	private:
		std::string prefix_, postfix_;
		label label_;
		std::size_t font_;
		bool font_rgb_set_;
		color_t font_rgb_;
	};

	class panel : public object
	{
	public:
		explicit panel(std::size_t sw, std::size_t sh, const config& cfg);

		using object::location;

		const std::string& image() const { return image_; }

	private:
		std::string image_;
	};

	class action : public object
	{
	public:
		action();
		explicit action(std::size_t sw, std::size_t sh, const config& cfg);

		using object::location;

		bool is_context() const  { return context_; }

		const std::string& title() const { return title_; }

		const std::string tooltip(std::size_t index) const;

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
		explicit slider(std::size_t sw, std::size_t sh, const config& cfg);

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
		explicit menu(std::size_t sw, std::size_t sh, const config& cfg);

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
	theme(const theme&) = delete;
	theme& operator=(const theme&) = delete;
	theme& operator=(theme&&);

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

	const rect& main_map_location(const SDL_Rect& screen) const
		{ return main_map_.location(screen); }
	const rect& mini_map_location(const SDL_Rect& screen) const
		{ return mini_map_.location(screen); }
	const rect& unit_image_location(const SDL_Rect& screen) const
		{ return unit_image_.location(screen); }
	const rect& palette_location(const SDL_Rect& screen) const
		{ return palette_.location(screen); }

	const border_t& border() const { return border_; }

	events::generic_event& theme_reset_event() { return theme_reset_event_; }

private:
	theme::object& find_element(const std::string& id);
	void add_object(std::size_t sw, std::size_t sh, const config& cfg);
	void remove_object(const std::string& id);
	void set_object_location(theme::object& element, const std::string& rect_str, std::string ref_id);

	//notify observers that the theme has been rebuilt completely
	//atm this is used for replay_controller to add replay controls to the standard theme
	events::generic_event theme_reset_event_;

	std::string cur_theme;
	config cfg_;
	std::vector<panel> panels_;
	std::vector<label> labels_;
	std::vector<menu> menus_;
	std::vector<action> actions_;
	std::vector<slider> sliders_;

	menu context_;
	action action_context_;

	std::map<std::string, std::unique_ptr<status_item>> status_;

	object main_map_, mini_map_, unit_image_, palette_;

	border_t border_;

	SDL_Rect screen_dimensions_;
	std::size_t cur_spec_width_, cur_spec_height_;

	static inline std::map<std::string, config> known_themes{};

public:
	/** Copies the theme configs from the main game config. */
	static void set_known_themes(const game_config_view* cfg);

	/** Returns the saved config for the theme with the given ID. */
	NOT_DANGLING static const config& get_theme_config(const std::string& id);

	/** Returns minimal info about saved themes, optionally including hidden ones. */
	static std::vector<theme_info> get_basic_theme_info(bool include_hidden = false);
};
