#ifndef WIDGET_MENU_HPP_INCLUDED
#define WIDGET_MENU_HPP_INCLUDED

#include <string>
#include <vector>

#include "../display.hpp"
#include "../events.hpp"
#include "../sdl_utils.hpp"

#include "scrollbar.hpp"
#include "button.hpp"

#include "SDL.h"

namespace gui {

class menu : public events::handler, public scrollable
{
public:
	menu(display& disp, const std::vector<std::string>& items,
	     bool click_selects=false, int max_height=-1, int max_width=-1);

	int height() const;
	int width() const;
	/// Return the width of the area where the items are. That is, the
	/// width excluding the scrollbar.
	int item_area_width() const;
	int selection() const;

	void set_loc(int x, int y);
	void set_width(int w);
	// Get the rect bounding the menu. If the max_height/max_width is
	// set, use those, otherwise use the current values.
	SDL_Rect get_rect() const;

	void redraw(); //forced redraw
	
	// allows user to change_item while running (dangerous)
	void change_item(int pos1,int pos2,std::string str);

	void erase_item(size_t index);

	/// Set new items to show and redraw/recalculate everything. If
	/// strip_spaces is false, spaces will remain at the item edges.
	void set_items(const std::vector<std::string>& items, bool strip_spaces=true);

	/// Set a new max height for this menu. Note that this does not take
	/// effect immideately, only after certain operations that clear
	/// everything, such as set_items().
	void set_max_height(const int new_max_height);
	void set_max_width(const int new_max_width);

	size_t nitems() const { return items_.size(); }
	
	int process(int x, int y, bool button,bool up_arrow,bool down_arrow,
	            bool page_up, bool page_down, int select_item=-1);

	bool double_clicked();

	void set_numeric_keypress_selection(bool value);

	void scroll(int pos);

	void set_dirty() { drawn_ = false; }
	enum { HELP_STRING_SEPERATOR = '|' };
	enum { IMG_TEXT_SEPERATOR = 1 }; // Re-evaluate if this should be
									 // something else to be settable
									 // from WML.

protected:
	void handle_event(const SDL_Event& event);

private:
	size_t max_items_onscreen() const;

	int max_height_, max_width_;
	mutable int max_items_, item_height_;

	void calculate_position();
	void key_press(SDLKey key);


	bool show_scrollbar() const;

	display* display_;
	int x_, y_;
	std::vector<std::vector<std::string> > items_, help_;

	void create_help_strings();
	void process_help_string(int mousex, int mousey);

	std::pair<int,int> cur_help_;
	int help_string_;

	mutable std::vector<int> column_widths_;

	scoped_sdl_surface buffer_;
	int selected_;
	bool click_selects_;
	bool previous_button_;
	bool drawn_;
	std::set<size_t> undrawn_items_;

	bool show_result_;

	mutable int height_;
	mutable int width_;

	mutable int first_item_on_screen_;
	gui::button uparrow_, downarrow_;

	bool double_clicked_;

	const std::vector<int>& column_widths() const;
	void draw_item(int item);
	void clear_item(int item);
	void draw();
	int hit(int x, int y) const;

	std::pair<int,int> hit_cell(int x, int y) const;

	mutable std::map<int,SDL_Rect> itemRects_;

	SDL_Rect get_item_rect(int item) const;
	size_t get_item_height_internal(int item) const;
	size_t get_item_height(int item) const;
	int items_start() const;

	int items_end() const;
	int items_height() const;

	/// Set the vertical size of the scroll bar grip. The size should
	/// vary inversely with the ratio of the number of items to the 
	/// size of the viewing rect
	void set_scrollbar_height();
	gui::scrollbar scrollbar_;
	int scrollbar_width_;
	int scrollbar_height_;

	///variable which determines whether a numeric keypress should
	///select an item on the dialog
	bool num_selects_;
	// These two variables are used to get the correct double click
	// behavior so that a click that causes one double click wont be
	// counted as a first click in the "next" double click.
	bool ignore_next_doubleclick_;
	bool last_was_doubleclick_;
};

}

#endif
