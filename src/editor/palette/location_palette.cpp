/*
   Copyright (C) 2003 - 2018 by David White <dave@whitevine.net>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-editor"

#include "editor/palette/location_palette.hpp"

#include "gettext.hpp"
#include "font/marked-up_text.hpp"
#include "font/standard_colors.hpp"
#include "tooltips.hpp"

#include "editor/editor_common.hpp"
#include "editor/toolkit/editor_toolkit.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/transient_message.hpp"

#include "formula/string_utils.hpp"

#include <boost/regex.hpp>

static bool is_positive_integer(const std::string& str) {
	return str != "0" && std::find_if(str.begin(), str.end(), [](char c) { return !std::isdigit(c); }) == str.end();
}

class location_palette_item : public gui::widget
{
public:
	struct state_t {
		state_t()
			: selected(false)
			, mouseover(false)
		{}
		bool selected;
		bool mouseover;
		friend bool operator==(state_t r, state_t l)
		{
			return r.selected == l.selected && r.mouseover == l.mouseover;
		}

	};
	location_palette_item(CVideo& video, editor::location_palette& parent)
		: gui::widget(video, true)
		, parent_(parent)
	{
	}

	void draw_contents() override
	{
		if (state_.mouseover) {
			sdl::fill_rectangle(location(), {200, 200, 200, 26});
		}
		if (state_.selected) {
			sdl::draw_rectangle(location(), {255, 255, 255, 255});
		}
		font::draw_text(&video(), location(), 16, font::NORMAL_COLOR, desc_.empty() ? id_ : desc_, location().x + 2, location().y, 0);
	}

	//TODO move to widget
	bool hit(int x, int y) const
	{
		return sdl::point_in_rect(x, y, location());
	}

	void mouse_up(const SDL_MouseButtonEvent& e)
	{
		if (!(hit(e.x, e.y)))
			return;
		if (e.button == SDL_BUTTON_LEFT) {
			parent_.select_item(id_);
		}
		if (e.button == SDL_BUTTON_RIGHT) {
			//TODO: add a context menu with the following options:
			// 1) 'copy it to clipboard'
			// 2) 'jump to item'
			// 3) 'delete item'.
		}
	}

	void handle_event(const SDL_Event& e) override
	{
		gui::widget::handle_event(e);

		if (hidden() || !enabled() || mouse_locked())
			return;

		state_t start_state = state_;

		switch (e.type) {
		case SDL_MOUSEBUTTONUP:
			mouse_up(e.button);
			break;
		case SDL_MOUSEMOTION:
			state_.mouseover = hit(e.motion.x, e.motion.y);
			break;
		default:
			return;
		}

		if (!(start_state == state_))
			set_dirty(true);
	}

	void set_item_id(const std::string& id)
	{
		id_ = id;
		if (is_positive_integer(id)) {
			desc_ = VGETTEXT("Player $side_num", utils::string_map{ {"side_num", id} });
		}
		else {
			desc_ = "";
		}
	}
	void set_selected(bool selected)
	{
		state_.selected = selected;
	}
	void draw() override { gui::widget::draw(); }
private:
	std::string id_;
	std::string desc_;
	state_t state_;
	editor::location_palette& parent_;
};

class location_palette_button : public gui::button
{
public:
	location_palette_button(CVideo& video, const SDL_Rect& location, const std::string& text, const std::function<void (void)>& callback)
		: gui::button(video, text)
		, callback_(callback)
	{
		this->set_location(location.x, location.y);
		this->hide(false);
	}
protected:
	virtual void mouse_up(const SDL_MouseButtonEvent& e) override
	{
		gui::button::mouse_up(e);
		if (callback_) {
			if (this->pressed()) {
				callback_();
			}
		}
	}
	std::function<void (void)> callback_;

};
namespace editor {
location_palette::location_palette(editor_display &gui, const config& /*cfg*/,
                                   editor_toolkit &toolkit)
		: common_palette(gui.video())
		, item_size_(20)
		//TODO avoid magic number
		, item_space_(20 + 3)
		, palette_y_(0)
		, palette_x_(0)
		, items_start_(0)
		, selected_item_()
		, items_()
		, toolkit_(toolkit)
		, buttons_()
		, button_add_()
		, button_delete_()
		, button_goto_()
		, help_handle_(-1)
		, disp_(gui)
	{
		for (int i = 1; i < 10; ++i) {
			items_.push_back(std::to_string(i));
		}
		selected_item_ = items_[0];
	}

sdl_handler_vector location_palette::handler_members()
{
	sdl_handler_vector h;
	for (gui::widget& b : buttons_) {
		h.push_back(&b);
	}
	if (button_add_) { h.push_back(button_add_.get()); }
	if (button_delete_) { h.push_back(button_delete_.get()); }
	if (button_goto_) { h.push_back(button_goto_.get()); }
	return h;
}

void location_palette::hide(bool hidden)
{
	widget::hide(hidden);

	disp_.video().clear_help_string(help_handle_);

	std::shared_ptr<gui::button> palette_menu_button = disp_.find_menu_button("menu-editor-terrain");
	palette_menu_button->set_overlay("");
	palette_menu_button->enable(false);

	for(auto& w : handler_members()) {
		static_cast<gui::widget&>(*w).hide(hidden);
	}
}

bool location_palette::scroll_up()
{
	int decrement = 1;
	if(items_start_ >= decrement) {
		items_start_ -= decrement;
		draw();
		return true;
	}
	return false;
}
bool location_palette::can_scroll_up()
{
	return (items_start_ != 0);
}

bool location_palette::can_scroll_down()
{
	return (items_start_ + num_visible_items() + 1 <= num_items());
}

bool location_palette::scroll_down()
{
	bool end_reached = (!(items_start_ + num_visible_items() + 1 <= num_items()));
	bool scrolled = false;

	// move downwards
	if(!end_reached) {
		items_start_ += 1;
		scrolled = true;
		set_dirty(true);
	}
	draw();
	return scrolled;
}

void location_palette::adjust_size(const SDL_Rect& target)
{
	palette_x_ = target.x;
	palette_y_ = target.y;
	const int button_height = 22;
	const int button_y = 30;
	int bottom = target.y + target.h;
	if (!button_goto_) {
		button_goto_.reset(new location_palette_button(video(), SDL_Rect{ target.x , bottom -= button_y, target.w - 10, button_height }, _("Go To"), [this]() {
			//static_cast<mouse_action_starting_position&>(toolkit_.get_mouse_action()). ??
			map_location pos = disp_.get_map().special_location(selected_item_);
			if (pos.valid()) {
				disp_.scroll_to_tile(pos, display::WARP);
			}
		}));
		button_add_.reset(new location_palette_button(video(), SDL_Rect{ target.x , bottom -= button_y, target.w - 10, button_height }, _("Add"), [this]() {
			std::string newid;
			if (gui2::dialogs::edit_text::execute(_("New Location Identifier"), "", newid)) {
				static const boost::regex valid_id("[a-zA-Z_]+");
				if(boost::regex_match(newid, valid_id)) {
					add_item(newid);
				}
				else {
					gui2::show_transient_message(
						_("Error"),
						_("Invalid location id")
					);
					//TODO: a user visible messae would be nice.
					ERR_ED  << "entered invalid location id\n";
				}
			}
		}));
		button_delete_.reset(new location_palette_button(video(), SDL_Rect{ target.x , bottom -= button_y, target.w - 10, button_height }, _("Delete"), nullptr));
	}
	else {
		button_goto_->set_location(SDL_Rect{ target.x , bottom -= button_y, target.w - 10, button_height });
		button_add_->set_location(SDL_Rect{ target.x , bottom -= button_y, target.w - 10, button_height });
		button_delete_->set_location(SDL_Rect{ target.x , bottom -= button_y, target.w - 10, button_height });
	}

	const int space_for_items = bottom - target.y;
	const int items_fitting = space_for_items / item_space_;
	// This might be called while the palette is not visible onscreen.
	// If that happens, no items will fit and we'll have a negative number here.
	// Just skip it in that case.
	if(items_fitting > 0 && num_visible_items() != items_fitting) {
		location_palette_item lpi(disp_.video(), *this);
		//Why does this need a pointer to a non-const as second paraeter?
		//TODO: we should write our own ptr_vector class, boost::ptr_vector has a lot of flaws.
		buttons_.resize(items_fitting, &lpi);
	}

	set_location(target);
	set_dirty(true);
	disp_.video().clear_help_string(help_handle_);
	help_handle_ = disp_.video().set_help_string(get_help_string());
}

void location_palette::select_item(const std::string& item_id)
{
	if (selected_item_ != item_id) {
		selected_item_ = item_id;
		set_dirty();
	}
	disp_.video().clear_help_string(help_handle_);
	help_handle_ = disp_.video().set_help_string(get_help_string());
}

int location_palette::num_items()
{
	return items_.size();
}
int location_palette::num_visible_items()
{
	return buttons_.size();
}

bool location_palette::is_selected_item(const std::string& id)
{
	return selected_item_ == id;
}

void location_palette::draw_contents()
{
	toolkit_.set_mouseover_overlay(disp_);
	int y = palette_y_;
	const int x = palette_x_;
	const int starting = items_start_;
	int ending = std::min<int>(starting + num_visible_items(), num_items());
	std::shared_ptr<gui::button> upscroll_button = disp_.find_action_button("upscroll-button-editor");
	if (upscroll_button)
		upscroll_button->enable(starting != 0);
	std::shared_ptr<gui::button> downscroll_button = disp_.find_action_button("downscroll-button-editor");
	if (downscroll_button)
		downscroll_button->enable(ending != num_items());

	if (button_goto_) {
		button_goto_->set_dirty(true);
	}
	if (button_add_) {
		button_add_->set_dirty(true);
	}
	if (button_delete_) {
		button_delete_->set_dirty(true);
	}
	for (int i = 0, size = num_visible_items(); i < size; i++) {

		location_palette_item & tile = buttons_[i];

		tile.hide(true);

		if (i >= ending) {
			//We want to hide all following buttons so we cannot use break here.
			continue;
		}

		const std::string item_id = items_[starting + i];

		std::stringstream tooltip_text;

		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = y;
		dstrect.w = location().w - 10;
		dstrect.h = item_size_ + 2;

		tile.set_location(dstrect);
		tile.set_tooltip_string(tooltip_text.str());
		tile.set_item_id(item_id);
		tile.set_selected(is_selected_item(item_id));
		tile.set_dirty(true);
		tile.hide(false);
		tile.draw();

		// Adjust location
		y += item_space_;
	}
}

std::vector<std::string> location_palette::action_pressed() const
{
	std::vector<std::string> res;
	if (button_delete_ && button_delete_->pressed()) {
		res.push_back("editor-remove-location");
	}
	return res;
}

location_palette::~location_palette()
{
}

// Sort numbers before all other strings.
static bool loc_id_comp(const std::string& lhs, const std::string& rhs) {
	if(is_positive_integer(lhs)) {
		if(is_positive_integer(rhs)) {
			return std::stoi(lhs) < std::stoi(rhs);
		} else {
			return true;
		}
	}
	if(is_positive_integer(rhs)) {
		return false;
	}
	return lhs < rhs;
}

void location_palette::add_item(const std::string& id)
{
	int pos;
	// Insert the new ID at the sorted location, unless it's already in the list
	const auto itor = std::upper_bound(items_.begin(), items_.end(), id, loc_id_comp);
	if(itor == items_.begin() || *(itor - 1) != id) {
		pos = std::distance(items_.begin(), items_.insert(itor, id));
	}
	else {
		pos = std::distance(items_.begin(), itor);
	}
	selected_item_ = id;
	if(num_visible_items() == 0) {
		items_start_ = 0;
	} else {
		items_start_ = std::max(pos - num_visible_items() + 1, items_start_);
		items_start_ = std::min(pos, items_start_);
	}
	adjust_size(location());
}

} // end namespace editor
