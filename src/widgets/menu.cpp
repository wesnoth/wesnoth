#include "../global.hpp"

#include "menu.hpp"

#include "../display.hpp"
#include "../font.hpp"
#include "../sdl_utils.hpp"
#include "../show_dialog.hpp"
#include "../util.hpp"
#include "../video.hpp"
#include "../wml_separators.hpp"
#include "serialization/string_utils.hpp"

#include <numeric>

namespace {
const size_t menu_font_size = font::SIZE_NORMAL;
const size_t menu_cell_padding = font::SIZE_NORMAL * 3/5;
}

namespace gui {

menu::menu(display& disp, const std::vector<std::string>& items,
           bool click_selects, int max_height, int max_width)
        : scrollarea(disp),
          max_height_(max_height), max_width_(max_width), max_items_(-1), item_height_(-1),
	  cur_help_(-1,-1), help_string_(-1),
	  selected_(0), click_selects_(click_selects),
	  previous_button_(true), show_result_(false),
	  double_clicked_(false),
	  num_selects_(true),
	  ignore_next_doubleclick_(false),
	  last_was_doubleclick_(false)
{
	fill_items(items, true);
}

void menu::fill_items(const std::vector<std::string>& items, bool strip_spaces)
{
	for(std::vector<std::string>::const_iterator item = items.begin();
	    item != items.end(); ++item) {
		items_.push_back(utils::quoted_split(*item, COLUMN_SEPARATOR, !strip_spaces));

		//make sure there is always at least one item
		if(items_.back().empty())
			items_.back().push_back(" ");

		//if the first character in an item is an asterisk,
		//it means this item should be selected by default
		std::string& first_item = items_.back().front();
		if(first_item.empty() == false && first_item[0] == DEFAULT_ITEM) {
			selected_ = items_.size()-1;
			first_item.erase(first_item.begin());
		}
	}

	create_help_strings();
	update_size();
}

void menu::create_help_strings()
{
	help_.clear();
	for(std::vector<std::vector<std::string> >::iterator i = items_.begin(); i != items_.end(); ++i) {
		help_.resize(help_.size()+1);
		for(std::vector<std::string>::iterator j = i->begin(); j != i->end(); ++j) {
			if(std::find(j->begin(),j->end(),static_cast<char>(HELP_STRING_SEPARATOR)) == j->end()) {
				help_.back().push_back("");
			} else {
				const std::vector<std::string>& items = utils::split(*j, HELP_STRING_SEPARATOR, 0);
				if(items.size() >= 2) {
					*j = items.front();
					help_.back().push_back(items.back());
				} else {
					help_.back().push_back("");
				}
			}
		}
	}
}

void menu::update_scrollbar_grip_height()
{
	set_full_size(items_.size());
	set_shown_size(max_items_onscreen());
}

void menu::update_size() {
	unsigned h = 0;
	for(size_t i = get_position(),
	    i_end = minimum(items_.size(), i + max_items_onscreen());
	    i != i_end; ++i)
		h += get_item_rect(i).h;
	h = maximum(h, height());
	if (max_height_ > 0 && h > max_height_)
		h = max_height_;

	std::vector<int> const &widths = column_widths();
	unsigned w = std::accumulate(widths.begin(), widths.end(), 0);
	if (items_.size() > max_items_onscreen())
		w += scrollbar_width();
	w = maximum(w, width());
	if (max_width_ > 0 && w > max_width_)
		w = max_width_;

	update_scrollbar_grip_height();
	set_measurements(w, h);
}

int menu::selection() const { return selected_; }

void menu::set_inner_location(SDL_Rect const &rect)
{
	itemRects_.clear();
	update_scrollbar_grip_height();
	bg_register(rect);
}

void menu::change_item(int pos1, int pos2,std::string str)
{
	items_[pos1][pos2] = str;
	//undrawn_items_.insert(pos1);
	set_dirty();
}

void menu::erase_item(size_t index)
{
	size_t nb_items = items_.size();
	if (index >= nb_items)
		return;

	clear_item(nb_items - 1);
	items_.erase(items_.begin() + index);
	if (selected_ >= nb_items - 1)
		selected_ = nb_items - 2;

	update_scrollbar_grip_height();
	adjust_viewport_to_selection();
	itemRects_.clear();
	set_dirty();
}

void menu::set_items(const std::vector<std::string>& items, bool strip_spaces, bool keep_viewport)
{
	items_.clear();
	itemRects_.clear();
	column_widths_.clear();
	//undrawn_items_.clear();
	max_items_ = -1; // Force recalculation of the max items.
	item_height_ = -1; // Force recalculation of the item height.
	selected_ = 0;
	fill_items(items, strip_spaces);
	if (!keep_viewport)
		set_position(0);
	update_scrollbar_grip_height();
	adjust_viewport_to_selection();
	set_dirty();
}

void menu::set_max_height(const int new_max_height) {
	max_height_ = new_max_height;
}

void menu::set_max_width(const int new_max_width) {
	max_width_ = new_max_width;
}

size_t menu::max_items_onscreen() const
{
	if(max_items_ != -1) {
		return size_t(max_items_);
	}

	const size_t max_height = max_height_ == -1 ? (disp().y()*66)/100 : max_height_;
	std::vector<int> heights;
	size_t n;
	for(n = 0; n != items_.size(); ++n) {
		heights.push_back(get_item_height(n));
	}

	std::sort(heights.begin(),heights.end(),std::greater<int>());
	size_t sum = 0;
	for(n = 0; n != items_.size() && sum < max_height; ++n) {
		sum += heights[n];
	}

	if(sum > max_height && n > 1)
		--n;

	return max_items_ = n;
}

void menu::adjust_viewport_to_selection()
{
	if(click_selects_)
		return;
	adjust_position(selected_);
}

void menu::move_selection_up(size_t dep)
{
	move_selection(selected_ > dep ? selected_ - dep : 0);
}

void menu::move_selection_down(size_t dep)
{
	size_t nb_items = items_.size();
	move_selection(selected_ + dep >= nb_items ? nb_items - 1 : selected_ + dep);
}

void menu::move_selection(size_t new_selected)
{
	if (new_selected == selected_ || new_selected >= items_.size())
		return;

	//undrawn_items_.insert(selected_);
	//undrawn_items_.insert(new_selected);
	set_dirty();
	selected_ = new_selected;
	adjust_viewport_to_selection();
}

void menu::key_press(SDLKey key)
{
	if (!click_selects_) {
		switch(key) {
		case SDLK_UP:
			move_selection_up(1);
			break;
		case SDLK_DOWN:
			move_selection_down(1);
			break;
		case SDLK_PAGEUP:
			move_selection_up(max_items_onscreen());
			break;
		case SDLK_PAGEDOWN:
			move_selection_down(max_items_onscreen());
			break;
		//case SDLK_RETURN:
		//	double_clicked_ = true;
		//	break;
		default:
			break;
		}
	}

	if (num_selects_ && key >= SDLK_1 && key <= SDLK_9)
		move_selection(key - SDLK_1);
}

void menu::handle_event(const SDL_Event& event)
{
	scrollarea::handle_event(event);
	if(event.type == SDL_KEYDOWN) {
		key_press(event.key.keysym.sym);
	} else if(event.type == SDL_MOUSEBUTTONDOWN &&
	          event.button.button == SDL_BUTTON_LEFT ||
		  event.type == DOUBLE_CLICK_EVENT) {
		int x = 0;
		int y = 0;
		if(event.type == SDL_MOUSEBUTTONDOWN) {
			x = event.button.x;
			y = event.button.y;
		} else {
			x = (long)event.user.data1;
			y = (long)event.user.data2;
		}

		const int item = hit(x,y);
		if(item != -1) {
			move_selection(item);

			if(click_selects_) {
				show_result_ = true;
			}

			if(event.type == DOUBLE_CLICK_EVENT) {
				if (ignore_next_doubleclick_) {
					ignore_next_doubleclick_ = false;
				} else {
					double_clicked_ = true;
					last_was_doubleclick_ = true;
				}
			} else if (last_was_doubleclick_) {
				// If we have a double click as the next event, it means
				// this double click was generated from a click that
				// already has helped in generating a double click.
				SDL_Event ev;
				SDL_PeepEvents(&ev, 1, SDL_PEEKEVENT,
							   SDL_EVENTMASK(DOUBLE_CLICK_EVENT));
				if (ev.type == DOUBLE_CLICK_EVENT) {
					ignore_next_doubleclick_ = true;
				}
				last_was_doubleclick_ = false;
			}
		}
	} else if(event.type == SDL_MOUSEMOTION && click_selects_) { 
		const int item = hit(event.motion.x,event.motion.y);
		if (item != -1)
			move_selection(item);
	}
}

int menu::process()
{
	if(show_result_) {
		show_result_ = false;
		return selected_;
	} else {
		return -1;
	}
}

bool menu::double_clicked()
{
	bool old = double_clicked_;
	double_clicked_ = false;
	return old;
}

void menu::set_numeric_keypress_selection(bool value)
{
	num_selects_ = value;
}

void menu::scroll(int)
{
	itemRects_.clear();
	set_dirty();
}

namespace {
	SDL_Rect item_size(const std::string& item) {
		SDL_Rect res = {0,0,0,0};
		std::vector<std::string> img_text_items = utils::split(item, IMG_TEXT_SEPARATOR);
		for (std::vector<std::string>::const_iterator it = img_text_items.begin();
			 it != img_text_items.end(); it++) {
			if (res.w > 0 || res.h > 0) {
				// Not the first item, add the spacing.
				res.w += 5;
			}
			const std::string str = *it;
			if (!str.empty() && str[0] == IMAGE_PREFIX) {
				const std::string image_name(str.begin()+1,str.end());
				surface const img = image::get_image(image_name,image::UNSCALED);
				if(img != NULL) {
					res.w += img->w;
					res.h = maximum<int>(img->h, res.h);
				}
			} else {
				const SDL_Rect area = {0,0,10000,10000};
				const SDL_Rect font_size =
					font::draw_text(NULL,area,menu_font_size,font::NORMAL_COLOUR,str,0,0);
				res.w += font_size.w;
				res.h = maximum<int>(font_size.h, res.h);
			}
		}
		return res;
	}
}

const std::vector<int>& menu::column_widths() const
{
	if(column_widths_.empty()) {
		for(size_t row = 0; row != items_.size(); ++row) {
			for(size_t col = 0; col != items_[row].size(); ++col) {
				const SDL_Rect res = item_size(items_[row][col]);

				if(col == column_widths_.size()) {
					column_widths_.push_back(res.w + menu_cell_padding);
				} else if(res.w > column_widths_[col] - menu_cell_padding) {
					column_widths_[col] = res.w + menu_cell_padding;
				}
			}
		}
	}

	return column_widths_;
}

void menu::clear_item(int item)
{
	SDL_Rect rect = get_item_rect(item);
	if (rect.w == 0)
		return;
	bg_restore(rect);
}

void menu::draw_item(int item)
{
	SDL_Rect rect = get_item_rect(item);
	if(rect.w == 0) {
		return;
	}

	clear_item(item);
	gui::draw_solid_tinted_rectangle(rect.x, rect.y, rect.w, rect.h,
	                                 item == selected_ ? 150:0,0,0,
	                                 item == selected_ ? 0.6 : 0.2,
	                                 disp().video().getSurface());

	SDL_Rect const &area = disp().screen_area();
	//SDL_Rect area = { 0, 0, rect.w, rect.h };
	SDL_Rect const &loc = inner_location();

	const std::vector<int>& widths = column_widths();

	int xpos = rect.x;
	for(size_t i = 0; i != items_[item].size(); ++i) {
		const int last_x = xpos;
		std::string str = items_[item][i];
		std::vector<std::string> img_text_items = utils::split(str, IMG_TEXT_SEPARATOR);
		for (std::vector<std::string>::const_iterator it = img_text_items.begin();
			 it != img_text_items.end(); it++) {
			str = *it;
			if (!str.empty() && str[0] == IMAGE_PREFIX) {
				const std::string image_name(str.begin()+1,str.end());
				surface const img = image::get_image(image_name,image::UNSCALED);
				const int max_width = max_width_ < 0 ? area.w :
					minimum<int>(max_width_, area.w - xpos);
				if(img != NULL && (xpos - rect.x) + img->w < max_width
				   && rect.y + img->h < area.h) {
					const size_t y = rect.y + (rect.h - img->h)/2;
					disp().blit_surface(xpos,y,img);
					xpos += img->w + 5;
				}
			} else {
				const std::string to_show = max_width_ > -1 ? 
					font::make_text_ellipsis(str, menu_font_size, loc.w - (xpos - rect.x)) : str;
				const SDL_Rect& text_size = font::text_area(str,menu_font_size);
				const size_t y = rect.y + (rect.h - text_size.h)/2;
				font::draw_text(&disp(),area,menu_font_size,font::NORMAL_COLOUR,to_show,xpos,y);
				xpos += text_size.w + 5;
			}
		}
		xpos = last_x + widths[i];
	}
}

void menu::draw_contents()
{
#if 0
	if (undrawn_items_.empty())
		return;

	if (!dirty()) {
		for(std::set<size_t>::const_iterator i = undrawn_items_.begin(); i != undrawn_items_.end(); ++i) {
			if(*i < items_.size()) {
				draw_item(*i);
				update_rect(get_item_rect(*i));
			}
		}

		undrawn_items_.clear();
		return;
	}

	undrawn_items_.clear();
#endif

	for(size_t i = 0; i != items_.size(); ++i)
		draw_item(i);
}

int menu::hit(int x, int y) const
{
	SDL_Rect const &loc = inner_location();
	if (x >= loc.x  && x < loc.x + loc.w && y >= loc.y && y < loc.y + loc.h) {
		for(size_t i = 0; i != items_.size(); ++i) {
			const SDL_Rect& rect = get_item_rect(i);
			if (y >= rect.y && y < rect.y + rect.h)
				return i;
		}
	}

	return -1;
}

std::pair<int,int> menu::hit_cell(int x, int y) const
{
	int i = hit(x, y);
	if (i < 0)
		return std::pair<int,int>(-1, -1);

	std::vector<int> const &widths = column_widths();
	x -= location().x;
	for(int j = 0, j_end = widths.size(); j != j_end; ++j) {
		x -= widths[j];
		if (x < 0)
			return std::pair<int,int>(i, j);
	}

	return std::pair<int,int>(-1, -1);
}

SDL_Rect menu::get_item_rect(int item) const
{
	const SDL_Rect empty_rect = {0,0,0,0};
	int first_item_on_screen = get_position();
	if (item < first_item_on_screen ||
	    size_t(item) >= first_item_on_screen + max_items_onscreen()) {
		return empty_rect;
	}

	const std::map<int,SDL_Rect>::const_iterator i = itemRects_.find(item);
	if(i != itemRects_.end())
		return i->second;

	SDL_Rect const &loc = inner_location();

	int y = loc.y;
	if (item != first_item_on_screen) {
		const SDL_Rect& prev = get_item_rect(item-1);
		y = prev.y + prev.h;
	}

	SDL_Rect res = { loc.x, y, loc.w, get_item_height(item) };

	SDL_Rect const &screen_area = disp().screen_area();

	if(res.x > screen_area.w) {
		return empty_rect;
	} else if(res.x + res.w > screen_area.w) {
		res.w = screen_area.w - res.x;
	}

	if(res.y > screen_area.h) {
		return empty_rect;
	} else if(res.y + res.h > screen_area.h) {
		res.h = screen_area.h - res.y;
	}

	//only insert into the cache if the menu's co-ordinates have
	//been initialized
	if (loc.x > 0 && loc.y > 0)
		itemRects_.insert(std::pair<int,SDL_Rect>(item,res));

	return res;
}

size_t menu::get_item_height_internal(int item) const
{
	size_t res = 0;
	for(size_t n = 0; n != items_[item].size(); ++n) {
		SDL_Rect rect = item_size(items_[item][n]);
		res = maximum<int>(rect.h,res);
	}

	return res;
}

size_t menu::get_item_height(int) const
{
	if(item_height_ != -1)
		return size_t(item_height_);

	size_t max_height = 0;
	for(size_t n = 0; n != items_.size(); ++n) {
		max_height = maximum<int>(max_height,get_item_height_internal(n));
	}

	return item_height_ = max_height;
}

void menu::process_help_string(int mousex, int mousey)
{
	const std::pair<int,int> loc = hit_cell(mousex,mousey);
	if(loc == cur_help_) {
		return;
	} else if(loc.first == -1) {
		disp().clear_help_string(help_string_);
		help_string_ = -1;
	} else {
		if(help_string_ != -1) {
			disp().clear_help_string(help_string_);
			help_string_ = -1;
		}

		if(size_t(loc.first) < help_.size()) {
			const std::vector<std::string>& row = help_[loc.first];
			if(size_t(loc.second) < row.size()) {
				const std::string& help = row[loc.second];
				if(help.empty() == false) {
					//std::cerr << "setting help string from menu to '" << help << "'\n";
					help_string_ = disp().set_help_string(help);
				}
			}
		}
	}

	cur_help_ = loc;
}

}
