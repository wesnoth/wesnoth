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

#ifndef WIDGET_MENU_HPP_INCLUDED
#define WIDGET_MENU_HPP_INCLUDED

#include <map>
#include <set>

#include "scrollarea.hpp"

namespace image{
	class locator;
}

namespace gui {

class menu : public scrollarea
{
public:

	enum ROW_TYPE { NORMAL_ROW, SELECTED_ROW, HEADING_ROW };
	//basic menu style
	class style
	{
	public:
		style();
		virtual ~style();
		virtual void init() {}

		virtual SDL_Rect item_size(const std::string& item) const;
		virtual void draw_row_bg(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		virtual void draw_row(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		void scale_images(int max_width, int max_height);
		surface get_item_image(const image::locator &i_locator) const;
		size_t get_font_size() const;
		size_t get_cell_padding() const;
		size_t get_thickness() const;

	protected:
		size_t font_size_;
		size_t cell_padding_;
		size_t thickness_;  //additional cell padding for style use only

		int normal_rgb_, selected_rgb_, heading_rgb_;
		double normal_alpha_, selected_alpha_, heading_alpha_;
		int max_img_w_, max_img_h_;
	};

	//image-border selection style
	class imgsel_style : public style
	{
	public:
		imgsel_style(const std::string &img_base, bool has_bg,
								 int normal_rgb, int selected_rgb, int heading_rgb,
								 double normal_alpha, double selected_alpha, double heading_alpha);
		virtual ~imgsel_style();

		virtual SDL_Rect item_size(const std::string& item) const;
		virtual void draw_row_bg(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);
		virtual void draw_row(menu& menu_ref, const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);

		virtual void init() { load_images(); }
		bool load_images();

	protected:
		const std::string img_base_;
		std::map<std::string,surface> img_map_;

	private:
		bool load_image(const std::string &img_sub);
		bool has_background_;
		bool initialized_;
		bool load_failed_;
		int normal_rgb2_, selected_rgb2_, heading_rgb2_;
		double normal_alpha2_, selected_alpha2_, heading_alpha2_;
		//FIXME: why is this better than a plain surface?
		struct bg_cache
		{
			bg_cache() : surf(), width(-1), height(-1)
			{}

			surface surf;
			int width, height;
		};
		bg_cache bg_cache_;
	};

	friend class style;
	friend class imgsel_style;
	static style &default_style;
	static style simple_style;
	static imgsel_style bluebg_style;

	struct item
	{
		item() : fields(), help(), id(0)
		{}

		item(const std::vector<std::string>& fields, size_t id)
			: fields(fields), help(), id(id)
		{}

		std::vector<std::string> fields;
		std::vector<std::string> help;
		size_t id;
	};

	class sorter
	{
	public:
		virtual ~sorter() {}
		virtual bool column_sortable(int column) const = 0;
		virtual bool less(int column, const item& row1, const item& row2) const = 0;
	};

	class basic_sorter : public sorter
	{
	public:
		basic_sorter();
		virtual ~basic_sorter() {}

		basic_sorter& set_alpha_sort(int column);
		basic_sorter& set_numeric_sort(int column);
		basic_sorter& set_xp_sort(int column);
		basic_sorter& set_level_sort(int level_column, int xp_column);
		basic_sorter& set_id_sort(int column);
		basic_sorter& set_redirect_sort(int column, int to);
		basic_sorter& set_position_sort(int column, const std::vector<int>& pos);
	protected:
		virtual bool column_sortable(int column) const;
		virtual bool less(int column, const item& row1, const item& row2) const;

	private:
		std::set<int> alpha_sort_, numeric_sort_, id_sort_, xp_sort_, level_sort_;
		std::map<int,int> redirect_sort_;
		std::map<int,std::vector<int> > pos_sort_;
		int xp_col_; //used by level sort
	};

	menu(CVideo& video, const std::vector<std::string>& items,
	     bool click_selects=false, int max_height=-1, int max_width=-1,
		 const sorter* sorter_obj=nullptr, style *menu_style=nullptr, const bool auto_join=true);

	/** Default implementation, but defined out-of-line for efficiency reasons. */
	~menu();

	int selection() const;

	void move_selection(size_t id);
	void move_selection_keeping_viewport(size_t id);
	void reset_selection();

	const item& get_item(int index) const;
	const item& get_selected_item() const;

	// allows user to change_item while running (dangerous)
	void change_item(int pos1,int pos2,const std::string& str);

	virtual void erase_item(size_t index);

	void set_heading(const std::vector<std::string>& heading);

	/// Set new items to show and redraw/recalculate everything. If
	/// strip_spaces is false, spaces will remain at the item edges. If
	/// keep_viewport is true, the menu tries to keep the selection at
	/// the same position as it were before the items were set.
	virtual void set_items(const std::vector<std::string>& items, bool strip_spaces=true,
				   bool keep_viewport=false);

	/// Set a new max height for this menu. Note that this does not take
	/// effect immediately, only after certain operations that clear
	/// everything, such as set_items().
	void set_max_height(const int new_max_height);
	void set_max_width(const int new_max_width);

	int get_max_height() { return max_height_; }
	int get_max_width() { return max_width_; }

	size_t number_of_items() const { return items_.size(); }

	int process();

	bool double_clicked();

	void set_click_selects(bool value);
	void set_numeric_keypress_selection(bool value);

	void scroll(unsigned int pos);

	//currently, menus do not manage the memory of their sorter
	//this should be changed to a more object-oriented approach
	void set_sorter(sorter *s);
	void sort_by(int column);
	int get_sort_by() {return sortby_;}
	bool get_sort_reversed() {return sortreversed_;}

protected:
	bool item_ends_with_image(const std::string& item) const;
	virtual void handle_event(const SDL_Event& event);
	void set_inner_location(const SDL_Rect& rect);

	bool requires_event_focus(const SDL_Event *event=nullptr) const;
	const std::vector<int>& column_widths() const;
	virtual void draw_row(const size_t row_index, const SDL_Rect& rect, ROW_TYPE type);

	style *style_;
	bool silent_;

	int hit(int x, int y) const;

	std::pair<int,int> hit_cell(int x, int y) const;
	int hit_column(int x) const;

	int hit_heading(int x, int y) const;

	void invalidate_row(size_t id);
	void invalidate_row_pos(size_t pos);
	void invalidate_heading();

private:
	size_t max_items_onscreen() const;

	size_t heading_height() const;

	int max_height_, max_width_;
	mutable int max_items_, item_height_;

	void adjust_viewport_to_selection();
	void key_press(SDL_Keycode key);

	std::vector<item> items_;
	std::vector<size_t> item_pos_;

	std::vector<std::string> heading_;
	mutable int heading_height_;

	void create_help_strings();
	void process_help_string(int mousex, int mousey);

	std::pair<int,int> cur_help_;
	int help_string_;

	mutable std::vector<int> column_widths_;

	size_t selected_;
	bool click_selects_;
	bool out_;
	bool previous_button_;
	//std::set<size_t> undrawn_items_;

	bool show_result_;

	bool double_clicked_;

	void column_widths_item(const std::vector<std::string>& row, std::vector<int>& widths) const;

	void clear_item(int item);
	void draw_contents();
	void draw();

	mutable std::map<int,SDL_Rect> itemRects_;

	SDL_Rect get_item_rect(int item) const;
	SDL_Rect get_item_rect_internal(size_t pos) const;
	size_t get_item_height_internal(const std::vector<std::string>& item) const;
	size_t get_item_height(int item) const;
	int items_start() const;

	int items_end() const;
	int items_height() const;

	void update_scrollbar_grip_height();

	///variable which determines whether a numeric keypress should
	///select an item on the dialog
	bool num_selects_;
	// These two variables are used to get the correct double click
	// behavior so that a click that causes one double click wont be
	// counted as a first click in the "next" double click.
	bool ignore_next_doubleclick_;
	bool last_was_doubleclick_;

	//ellipsis calculation is slightly off, so default to false
	bool use_ellipsis_;

	const sorter* sorter_;
	int sortby_;
	bool sortreversed_;
	int highlight_heading_;

	/// Set new items to show. If strip_spaces is false, spaces will
	/// remain at the item edges.
	void fill_items(const std::vector<std::string>& items, bool strip_spaces);

	void do_sort();
	void recalculate_pos();
	void assert_pos();

	void update_size();
	enum SELECTION_MOVE_VIEWPORT { MOVE_VIEWPORT, NO_MOVE_VIEWPORT };
	void set_selection_pos(size_t pos, bool silent=false, SELECTION_MOVE_VIEWPORT move_viewport=MOVE_VIEWPORT);
	void move_selection_to(size_t id, bool silent=false, SELECTION_MOVE_VIEWPORT move_viewport=MOVE_VIEWPORT);
	void move_selection_up(size_t dep);
	void move_selection_down(size_t dep);

	std::set<int> invalid_;
};



}

#endif
