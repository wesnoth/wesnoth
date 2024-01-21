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

#pragma once

#include <optional>
#include <map>
#include <set>

#include "scrollarea.hpp"

class texture;

namespace image{
	class locator;
}

namespace gui {

/**
 * The only kind of row still supported by the menu class.
 *
 * If comparing to 1.17.24 or before, these three items were held as a single string, thus a single member
 * of the "fields" and a single "column" of the multi-column support.
 */
struct indented_menu_item
{
	/** An amount of blank space at the start of the row, measured in tab-stops (so 1 is around 4 en-widths) */
	int indent_level;
	/** If non-empty, a picture to display before the text */
	std::string icon;
	std::string text;
};

/**
 * Superclass of the help_menu, which displays the left-hand pane of the GUI1 help browser.
 * Historically a more generic class, but now only used for that singular purpose.
 */
class menu : public scrollarea
{
public:

	enum ROW_TYPE { NORMAL_ROW, SELECTED_ROW };
	//basic menu style
	class style
	{
	public:
		style();
		virtual ~style();
		virtual void init() {}

		virtual SDL_Rect item_size(const indented_menu_item& imi) const;
		virtual void draw_row_bg(menu& menu_ref, const std::size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		virtual void draw_row(menu& menu_ref, const std::size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		std::size_t get_font_size() const;
		std::size_t get_cell_padding() const;
		std::size_t get_thickness() const;

	protected:
		std::size_t font_size_;
		std::size_t cell_padding_;
		std::size_t thickness_;  //additional cell padding for style use only

		int normal_rgb_, selected_rgb_;
		double normal_alpha_, selected_alpha_;
	};

	//image-border selection style
	class imgsel_style : public style
	{
	public:
		imgsel_style(const std::string &img_base, bool has_bg,
								 int normal_rgb, int selected_rgb,
								 double normal_alpha, double selected_alpha);
		virtual ~imgsel_style();

		virtual SDL_Rect item_size(const indented_menu_item& imi) const;
		virtual void draw_row_bg(menu& menu_ref, const std::size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		virtual void draw_row(menu& menu_ref, const std::size_t row_index, const SDL_Rect& rect, ROW_TYPE type);

		virtual void init() { load_images(); }
		bool load_images();
		void unload_images();

	protected:
		const std::string img_base_;
		std::map<std::string,texture> img_map_;

	private:
		bool load_image(const std::string &img_sub);
		bool has_background_;
		bool initialized_;
		bool load_failed_;
		int normal_rgb2_, selected_rgb2_;
		double normal_alpha2_, selected_alpha2_;
	};

	friend class style;
	friend class imgsel_style;
	static style &default_style;
	static imgsel_style bluebg_style;

	struct item
	{
		item(const indented_menu_item& fields, std::size_t id)
			: fields(fields), id(id)
		{}

		indented_menu_item fields;
		std::size_t id;
	};

	menu(
	     bool click_selects=false, int max_height=-1, int max_width=-1,
		 style *menu_style=nullptr, const bool auto_join=true);

	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~menu();

	int selection() const;

	void move_selection(std::size_t id);
	void move_selection_keeping_viewport(std::size_t id);
	void reset_selection();

	/**
	 * Set new items to show and redraw/recalculate everything. The menu tries
	 * to keep the selection at the same position as it were before the items
	 * were set.
	 */
	virtual void set_items(const std::vector<indented_menu_item>& items, std::optional<std::size_t> selected);

	/**
	 * Set a new max height for this menu. Note that this does not take
	 * effect immediately, only after certain operations that clear
	 * everything, such as set_items().
	 */
	void set_max_height(const int new_max_height);
	void set_max_width(const int new_max_width);

	int get_max_height() const { return max_height_; }
	int get_max_width() const { return max_width_; }

	std::size_t number_of_items() const { return items_.size(); }

	int process();

	bool double_clicked();

	void set_click_selects(bool value);
	void set_numeric_keypress_selection(bool value);

	// scrollarea override
	void scroll(unsigned int pos) override;

protected:
	virtual void handle_event(const SDL_Event& event) override;
	void set_inner_location(const SDL_Rect& rect) override;

	bool requires_event_focus(const SDL_Event *event=nullptr) const override;
	int widest_row_width() const;
	virtual void draw_row(const std::size_t row_index, const SDL_Rect& rect, ROW_TYPE type);

	style *style_;
	bool silent_;

	int hit(int x, int y) const;

	/**
	 * Returns true if a mouse-click with the given x-coordinate, and an
	 * appropriate y-coordinate would lie within the indent or icon part of
	 * the given row.
	 *
	 * The unusual combination of arguments fit with this being called when
	 * handling a mouse event, where we already know which row was selected,
	 * and are just inquiring a bit more about the details of that row.
	 */
	bool hit_on_indent_or_icon(std::size_t row_index, int x) const;

	void invalidate_row(std::size_t id);
	void invalidate_row_pos(std::size_t pos);

private:
	std::size_t max_items_onscreen() const;

	int max_height_, max_width_;
	mutable int max_items_, item_height_;

	void adjust_viewport_to_selection();
	void key_press(SDL_Keycode key);

	std::vector<item> items_;
	std::vector<std::size_t> item_pos_;

	/**
	 * Cached return value of widest_row_width(), calculated on demand when calling that function.
	 */
	mutable std::optional<int> widest_row_width_;

	std::size_t selected_;
	bool click_selects_;
	bool out_;
	bool previous_button_;
	//std::set<std::size_t> undrawn_items_;

	bool show_result_;

	bool double_clicked_;

	void draw_contents() override;

	mutable std::map<int,SDL_Rect> itemRects_;

	SDL_Rect get_item_rect(int item) const;
	SDL_Rect get_item_rect_internal(std::size_t pos) const;
	std::size_t get_item_height_internal(const indented_menu_item& imi) const;
	std::size_t get_item_height(int item) const;
	int items_start() const;

	int items_end() const;
	int items_height() const;

	void update_scrollbar_grip_height();

	/**
	 * variable which determines whether a numeric keypress should
	 * select an item on the dialog
	 */
	bool num_selects_;
	// These two variables are used to get the correct double click
	// behavior so that a click that causes one double click won't be
	// counted as a first click in the "next" double click.
	bool ignore_next_doubleclick_;
	bool last_was_doubleclick_;

	//ellipsis calculation is slightly off, so default to false
	bool use_ellipsis_;

	/**
	 * Set new items to show.
	 */
	void fill_items(const std::vector<indented_menu_item>& imi);

	void update_size();
	enum SELECTION_MOVE_VIEWPORT { MOVE_VIEWPORT, NO_MOVE_VIEWPORT };
	void set_selection_pos(std::size_t pos, bool silent=false, SELECTION_MOVE_VIEWPORT move_viewport=MOVE_VIEWPORT);
	void move_selection_to(std::size_t id, bool silent=false, SELECTION_MOVE_VIEWPORT move_viewport=MOVE_VIEWPORT);
	void move_selection_up(std::size_t dep);
	void move_selection_down(std::size_t dep);

	std::set<int> invalid_;
};

}
