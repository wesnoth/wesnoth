#ifndef WIDGET_MENU_HPP_INCLUDED
#define WIDGET_MENU_HPP_INCLUDED

#include <string>
#include <vector>

#include "../display.hpp"
#include "../events.hpp"
#include "../sdl_utils.hpp"

#include "scrollarea.hpp"

#include "SDL.h"

namespace gui {

class menu : public scrollarea
{
public:
	menu(display& disp, const std::vector<std::string>& items,
	     bool click_selects=false, int max_height=-1, int max_width=-1);

	int selection() const;
	void move_selection(size_t pos);

	// allows user to change_item while running (dangerous)
	void change_item(int pos1,int pos2,std::string str);

	void erase_item(size_t index);

	/// Set new items to show and redraw/recalculate everything. If
	/// strip_spaces is false, spaces will remain at the item edges. If
	/// keep_viewport is true, the menu tries to keep the selection at
	/// the same position as it were before the items were set.
	void set_items(const std::vector<std::string>& items, bool strip_spaces=true,
				   bool keep_viewport=false);

	/// Set a new max height for this menu. Note that this does not take
	/// effect immediately, only after certain operations that clear
	/// everything, such as set_items().
	void set_max_height(const int new_max_height);
	void set_max_width(const int new_max_width);

	size_t nitems() const { return items_.size(); }
	
	int process();

	bool double_clicked();

	void set_numeric_keypress_selection(bool value);

	void scroll(int pos);

protected:
	void handle_event(const SDL_Event& event);
	void set_inner_location(const SDL_Rect& rect);

private:
	size_t max_items_onscreen() const;

	int max_height_, max_width_;
	mutable int max_items_, item_height_;

	void adjust_viewport_to_selection();
	void key_press(SDLKey key);

	std::vector<std::vector<std::string> > items_, help_;

	void create_help_strings();
	void process_help_string(int mousex, int mousey);

	std::pair<int,int> cur_help_;
	int help_string_;

	mutable std::vector<int> column_widths_;

	size_t selected_;
	bool click_selects_;
	bool previous_button_;
	//std::set<size_t> undrawn_items_;

	bool show_result_;

	bool double_clicked_;

	const std::vector<int>& column_widths() const;
	void draw_item(int item);
	void clear_item(int item);
	void draw_contents();
	int hit(int x, int y) const;

	std::pair<int,int> hit_cell(int x, int y) const;

	mutable std::map<int,SDL_Rect> itemRects_;

	SDL_Rect get_item_rect(int item) const;
	size_t get_item_height_internal(int item) const;
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

	/// Set new items to show. If strip_spaces is false, spaces will
	/// remain at the item edges.
	void fill_items(const std::vector<std::string>& items, bool strip_spaces);

	void update_size();
	void move_selection_up(size_t dep);
	void move_selection_down(size_t dep);
};

}

#endif
