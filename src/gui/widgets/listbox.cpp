/*
	Copyright (C) 2008 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/listbox.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/layout_exception.hpp"
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/viewport.hpp"
#include "gui/widgets/widget_helpers.hpp"
#include "gui/widgets/window.hpp"
#include "sdl/rect.hpp"
#include "wml_exception.hpp"
#include <functional>
#include <optional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{
// ------------ WIDGET -----------{

REGISTER_WIDGET(listbox)
REGISTER_WIDGET3(listbox_definition, horizontal_listbox, nullptr)
REGISTER_WIDGET3(listbox_definition, grid_listbox, nullptr)

listbox::listbox(const implementation::builder_styled_widget& builder,
		const generator_base::placement placement,
		builder_grid_ptr list_builder)
	: scrollbar_container(builder, type())
	, generator_(nullptr)
	, is_horizontal_(placement == generator_base::horizontal_list)
	, list_builder_(list_builder)
	, orders_()
	, callback_order_change_()
{
}

grid& listbox::add_row(const widget_item& item, const int index)
{
	assert(generator_);
	grid& row = generator_->create_item(index, *list_builder_, item, std::bind(&listbox::list_item_clicked, this, std::placeholders::_1));

	resize_content(row);

	return row;
}

grid& listbox::add_row(const widget_data& data, const int index)
{
	assert(generator_);
	grid& row = generator_->create_item(index, *list_builder_, data, std::bind(&listbox::list_item_clicked, this, std::placeholders::_1));

	resize_content(row);

	return row;
}

void listbox::remove_row(const unsigned row, unsigned count)
{
	assert(generator_);

	if(row >= get_item_count()) {
		return;
	}

	if(!count || count + row > get_item_count()) {
		count = get_item_count() - row;
	}

	int height_reduced = 0;
	int width_reduced = 0;

	// TODO: Fix this for horizontal listboxes
	// Note the we have to use content_grid_ and cannot use "_list_grid" which is what generator_ uses.
	int row_pos_y = is_horizontal_ ? -1 : generator_->item(row).get_y() - content_grid_->get_y();
	int row_pos_x = is_horizontal_ ? -1 : 0;

	for(; count; --count) {
		if(generator_->item(row).get_visible() != visibility::invisible) {
			if(is_horizontal_) {
				width_reduced += generator_->item(row).get_width();
			} else {
				height_reduced += generator_->item(row).get_height();
			}
		}

		generator_->delete_item(row);
	}

	if((height_reduced != 0 || width_reduced != 0) && get_item_count() != 0) {
		resize_content(-width_reduced, -height_reduced, row_pos_x, row_pos_y);
	} else {
		update_content_size();
	}
}

void listbox::clear()
{
	generator_->clear();
	update_content_size();
}

unsigned listbox::get_item_count() const
{
	assert(generator_);
	return generator_->get_item_count();
}

void listbox::set_row_active(const unsigned row, const bool active)
{
	assert(generator_);
	generator_->item(row).set_active(active);
}

void listbox::set_row_shown(const unsigned row, const bool shown)
{
	assert(generator_);

	window* window = get_window();
	assert(window);

	const int selected_row = get_selected_row();

	bool resize_needed = false;

	// Local scope for invalidate_layout_blocker
	{
		window::invalidate_layout_blocker invalidate_layout_blocker(*window);

		generator_->set_item_shown(row, shown);

		point best_size = generator_->calculate_best_size();
		generator_->place(generator_->get_origin(), {std::max(best_size.x, content_visible_area().w), best_size.y});

		resize_needed = !content_resize_request();
	}

	if(resize_needed) {
		window->invalidate_layout();
	} else {
		content_grid_->set_visible_rectangle(content_visible_area());
		queue_redraw(); // TODO: draw_manager - does this get the right area?
	}

	if(selected_row != get_selected_row()) {
		fire(event::NOTIFY_MODIFIED, *this, nullptr);
	}
}

void listbox::set_row_shown(const boost::dynamic_bitset<>& shown)
{
	assert(generator_);
	assert(shown.size() == get_item_count());

	if(generator_->get_items_shown() == shown) {
		LOG_GUI_G << LOG_HEADER << " returning early";
		return;
	}

	window* window = get_window();
	assert(window);

	const int selected_row = get_selected_row();

	bool resize_needed = false;

	// Local scope for invalidate_layout_blocker
	{
		window::invalidate_layout_blocker invalidate_layout_blocker(*window);

		for(std::size_t i = 0; i < shown.size(); ++i) {
			generator_->set_item_shown(i, shown[i]);
		}

		point best_size = generator_->calculate_best_size();
		generator_->place(generator_->get_origin(), {std::max(best_size.x, content_visible_area().w), best_size.y});

		resize_needed = !content_resize_request();
	}

	if(resize_needed) {
		window->invalidate_layout();
	} else {
		content_grid_->set_visible_rectangle(content_visible_area());
		queue_redraw(); // TODO: draw_manager - does this get the right area?
	}

	if(selected_row != get_selected_row()) {
		fire(event::NOTIFY_MODIFIED, *this, nullptr);
	}
}

boost::dynamic_bitset<> listbox::get_rows_shown() const
{
	return generator_->get_items_shown();
}

bool listbox::any_rows_shown() const
{
	for(std::size_t i = 0; i < get_item_count(); i++) {
		if(generator_->get_item_shown(i)) {
			return true;
		}
	}

	return false;
}

const grid* listbox::get_row_grid(const unsigned row) const
{
	assert(generator_);
	// rename this function and can we return a reference??
	return &generator_->item(row);
}

grid* listbox::get_row_grid(const unsigned row)
{
	assert(generator_);
	return &generator_->item(row);
}

bool listbox::select_row(const unsigned row, const bool select)
{
	if(row >= get_item_count()) {
		throw std::invalid_argument("invalid listbox index");
	}
	assert(generator_);

	unsigned int before = generator_->get_selected_item_count();
	generator_->select_item(row, select);

	return before != generator_->get_selected_item_count();
}

bool listbox::select_row_at(const unsigned row, const bool select)
{
	assert(generator_);
	return select_row(generator_->get_item_at_ordered(row), select);
}

bool listbox::row_selected(const unsigned row)
{
	assert(generator_);
	return generator_->is_selected(row);
}

int listbox::get_selected_row() const
{
	assert(generator_);
	return generator_->get_selected_item();
}

void listbox::list_item_clicked(widget& caller)
{
	assert(generator_);

	/** @todo Hack to capture the keyboard focus. */
	get_window()->keyboard_capture(this);

	for(std::size_t i = 0; i < generator_->get_item_count(); ++i) {
		if(generator_->item(i).has_widget(caller)) {
			toggle_button* checkbox = dynamic_cast<toggle_button*>(&caller);

			if(checkbox != nullptr) {
				generator_->select_item(i, checkbox->get_value_bool());
			} else {
				generator_->toggle_item(i);
			}

			// TODO: enable this code once toggle_panel::set_value dispatches
			// NOTIFY_MODIFED events. See comment in said function for more details.
#if 0
			selectable_item& selectable = dynamic_cast<selectable_item&>(caller);

			generator_->select_item(i, selectable.get_value_bool());
#endif

			fire(event::NOTIFY_MODIFIED, *this, nullptr);
			break;
		}
	}

	const int selected_item = generator_->get_selected_item();
	if(selected_item == -1) {
		return;
	}

	const rect& visible = content_visible_area();
	rect r = generator_->item(selected_item).get_rectangle();

	if(visible.overlaps(r)) {
		r.x = visible.x;
		r.w = visible.w;

		show_content_rect(r);
	}
}

void listbox::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

bool listbox::update_content_size()
{
	if(get_visible() == widget::visibility::invisible) {
		return true;
	}

	if(get_size() == point()) {
		return false;
	}

	if(content_resize_request(true)) {
		content_grid_->set_visible_rectangle(content_visible_area());
		queue_redraw(); // TODO: draw_manager - does this get the right area?
		return true;
	}

	return false;
}

void listbox::place(const point& origin, const point& size)
{
	std::optional<unsigned> vertical_scrollbar_position, horizontal_scrollbar_position;

	// Check if this is the first time placing the list box
	if(get_origin() != point {-1, -1}) {
		vertical_scrollbar_position = get_vertical_scrollbar_item_position();
		horizontal_scrollbar_position = get_horizontal_scrollbar_item_position();
	}

	// Inherited.
	scrollbar_container::place(origin, size);

	const int selected_item = generator_->get_selected_item();
	if(vertical_scrollbar_position && horizontal_scrollbar_position) {
		LOG_GUI_L << LOG_HEADER << " restoring scroll position";

		set_vertical_scrollbar_item_position(*vertical_scrollbar_position);
		set_horizontal_scrollbar_item_position(*horizontal_scrollbar_position);
	} else if(selected_item != -1) {
		LOG_GUI_L << LOG_HEADER << " making the initially selected item visible";

		const SDL_Rect& visible = content_visible_area();
		SDL_Rect rect = generator_->item(selected_item).get_rectangle();

		rect.x = visible.x;
		rect.w = visible.w;

		show_content_rect(rect);
	}
}

void listbox::resize_content(const int width_modification,
		const int height_modification,
		const int width_modification_pos,
		const int height_modification_pos)
{
	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size() << " width_modification "
			  << width_modification << " height_modification " << height_modification << ".";

	if(content_resize_request(
		width_modification, height_modification, width_modification_pos, height_modification_pos))
	{
		// Calculate new size.
		point size = content_grid()->get_size();
		size.x += width_modification;
		size.y += height_modification;

		// Set new size.
		content_grid()->set_size(size);
		update_layout();

		// If the content grows assume it "overwrites" the old content.
		if(width_modification < 0 || height_modification < 0) {
			queue_redraw();
		}

		DBG_GUI_L << LOG_HEADER << " succeeded.";
	} else {
		DBG_GUI_L << LOG_HEADER << " failed.";
	}
}

void listbox::resize_content(const widget& row)
{
	if(row.get_visible() == visibility::invisible) {
		return;
	}

	DBG_GUI_L << LOG_HEADER << " current size " << content_grid()->get_size() << " row size " << row.get_best_size()
			  << ".";

	const point content = content_grid()->get_size();
	point size = row.get_best_size();

	if(size.x < content.x) {
		size.x = 0;
	} else {
		size.x -= content.x;
	}

	resize_content(size.x, size.y);
}

point listbox::calculate_best_size() const
{
	// Get the size from the base class, then add any extra space for the header and footer.
	point result = scrollbar_container::calculate_best_size();

	if(const grid* header = find_widget<const grid>(&get_grid(), "_header_grid", false, false)) {
		result.y += header->get_best_size().y;
	}

	if(const grid* footer = find_widget<const grid>(&get_grid(), "_footer_grid", false, false)) {
		result.y += footer->get_best_size().y;
	}

	return result;
}

void listbox::update_visible_area_on_key_event(const KEY_SCROLL_DIRECTION direction)
{
	const SDL_Rect& visible = content_visible_area();
	SDL_Rect rect = generator_->item(generator_->get_selected_item()).get_rectangle();

	// When scrolling make sure the new items are visible...
	if(direction == KEY_VERTICAL) {
		// ...but leave the horizontal scrollbar position.
		rect.x = visible.x;
		rect.w = visible.w;
	} else {
		// ...but leave the vertical scrollbar position.
		rect.y = visible.y;
		rect.h = visible.h;
	}

	show_content_rect(rect);

	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

void listbox::handle_key_up_arrow(SDL_Keymod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_up_arrow(modifier, handled);

	if(handled) {
		update_visible_area_on_key_event(KEY_VERTICAL);
	} else {
		// Inherited.
		scrollbar_container::handle_key_up_arrow(modifier, handled);
	}
}

void listbox::handle_key_down_arrow(SDL_Keymod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_down_arrow(modifier, handled);

	if(handled) {
		update_visible_area_on_key_event(KEY_VERTICAL);
	} else {
		// Inherited.
		scrollbar_container::handle_key_up_arrow(modifier, handled);
	}
}

void listbox::handle_key_left_arrow(SDL_Keymod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_left_arrow(modifier, handled);

	// Inherited.
	if(handled) {
		update_visible_area_on_key_event(KEY_HORIZONTAL);
	} else {
		scrollbar_container::handle_key_left_arrow(modifier, handled);
	}
}

void listbox::handle_key_right_arrow(SDL_Keymod modifier, bool& handled)
{
	assert(generator_);

	generator_->handle_key_right_arrow(modifier, handled);

	// Inherited.
	if(handled) {
		update_visible_area_on_key_event(KEY_HORIZONTAL);
	} else {
		scrollbar_container::handle_key_left_arrow(modifier, handled);
	}
}

void listbox::finalize(std::unique_ptr<generator_base> generator,
		builder_grid_const_ptr header,
		builder_grid_const_ptr footer,
		const std::vector<widget_data>& list_data)
{
	// "Inherited."
	scrollbar_container::finalize_setup();

	if(header) {
		swap_grid(&get_grid(), content_grid(), header->build(), "_header_grid");
	}

	grid& p = find_widget<grid>(this, "_header_grid", false);

	for(unsigned i = 0, max = std::max(p.get_cols(), p.get_rows()); i < max; ++i) {
		//
		// TODO: I had to change this to case to a toggle_button in order to use a signal handler.
		// Should probably look into a way to make it more general like it was before (used to be
		// cast to selectable_item).
		//
		// - vultraz, 2017-08-23
		//
		if(toggle_button* selectable = find_widget<toggle_button>(&p, "sort_" + std::to_string(i), false, false)) {
			// Register callback to sort the list.
			connect_signal_notify_modified(*selectable, std::bind(&listbox::order_by_column, this, i, std::placeholders::_1));

			if(orders_.size() < max) {
				orders_.resize(max);
			}

			orders_[i].first = selectable;
		}
	}

	if(footer) {
		swap_grid(&get_grid(), content_grid(), footer->build(), "_footer_grid");
	}

	// Save our *non-owning* pointer before this gets moved into the grid.
	generator_ = generator.get();
	assert(generator_);

	generator->create_items(-1, *list_builder_, list_data, std::bind(&listbox::list_item_clicked, this, std::placeholders::_1));
	swap_grid(nullptr, content_grid(), std::move(generator), "_list_grid");
}

void listbox::order_by_column(unsigned column, widget& widget)
{
	selectable_item& selectable = dynamic_cast<selectable_item&>(widget);
	if(column >= orders_.size()) {
		return;
	}

	for(auto& pair : orders_) {
		if(pair.first != nullptr && pair.first != &selectable) {
			pair.first->set_value(static_cast<unsigned int>(sort_order::type::none));
		}
	}

	sort_order::type order = sort_order::get_enum(selectable.get_value()).value_or(sort_order::type::none);

	if(static_cast<unsigned int>(order) > orders_[column].second.size()) {
		return;
	}

	if(order == sort_order::type::none) {
		order_by(std::less<unsigned>());
	} else {
		order_by(orders_[column].second[static_cast<unsigned int>(order) - 1]);
	}

	if(callback_order_change_ != nullptr) {
		callback_order_change_(column, order);
	}
}

void listbox::order_by(const generator_base::order_func& func)
{
	generator_->set_order(func);

	update_layout();
}

void listbox::set_column_order(unsigned col, const generator_sort_array& func)
{
	if(col >= orders_.size()) {
		orders_.resize(col + 1);
	}

	orders_[col].second = func;
}

void listbox::register_translatable_sorting_option(const int col, translatable_sorter_func_t f)
{
	set_column_order(col, {{
		[f](int lhs, int rhs) { return translation::icompare(f(lhs), f(rhs)) < 0; },
		[f](int lhs, int rhs) { return translation::icompare(f(lhs), f(rhs)) > 0; }
	}});
}

void listbox::set_active_sorting_option(const order_pair& sort_by, const bool select_first)
{
	// TODO: should this be moved to a public header_grid() getter function?
	grid& header_grid = find_widget<grid>(this, "_header_grid", false);

	selectable_item& w = find_widget<selectable_item>(&header_grid, "sort_" + std::to_string(sort_by.first), false);

	// Set the sorting toggle widgets' value (in this case, its state) to the given sorting
	// order. This is necessary since the widget's value is used to determine the order in
	// @ref order_by_column in lieu of a direction being passed directly.
	w.set_value(static_cast<int>(sort_by.second));

	order_by_column(sort_by.first, dynamic_cast<widget&>(w));

	if(select_first && generator_->get_item_count() > 0) {
		select_row_at(0);
	}
}

const listbox::order_pair listbox::get_active_sorting_option()
{
	for(unsigned int column = 0; column < orders_.size(); ++column) {
		selectable_item* w = orders_[column].first;
		sort_order::type sort = sort_order::get_enum(w->get_value()).value_or(sort_order::type::none);

		if(w && sort != sort_order::type::none) {
			return std::pair(column, sort);
		}
	}

	return std::pair(-1, sort_order::type::none);
}

void listbox::mark_as_unsorted()
{
	for(auto& pair : orders_) {
		if(pair.first != nullptr) {
			pair.first->set_value(static_cast<unsigned int>(sort_order::type::none));
		}
	}
}

void listbox::set_content_size(const point& origin, const point& size)
{
	/** @todo This function needs more testing. */
	assert(content_grid());

	const int best_height = content_grid()->get_best_size().y;
	const point s(size.x, size.y < best_height ? size.y : best_height);

	content_grid()->place(origin, s);
}

void listbox::update_layout()
{
	assert(content_grid());

	// If we haven't initialized, or have no content, just return.
	point size = content_grid()->get_size();
	if(size.x <= 0 || size.y <= 0) {
		return;
	}

	content_grid()->place(content_grid()->get_origin(), size);

	const SDL_Rect& visible = content_visible_area_;
	content_grid()->set_visible_rectangle(visible);

	queue_redraw();
}

// }---------- DEFINITION ---------{

listbox_definition::listbox_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing listbox " << id;

	load_resolutions<resolution>(cfg);
}

listbox_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg)
	, grid(nullptr)
{
	// Note the order should be the same as the enum state_t in listbox.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", _("Missing required state for listbox")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", _("Missing required state for listbox")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", _("No grid defined."));
	grid = std::make_shared<builder_grid>(child);
}

namespace implementation
{
static std::vector<widget_data> parse_list_data(const config& data, const unsigned int req_cols)
{
	std::vector<widget_data> list_data;

	for(const auto& row : data.child_range("row")) {
		auto cols = row.child_range("column");

		VALIDATE(static_cast<unsigned>(cols.size()) == req_cols,
			_("'list_data' must have the same number of columns as the 'list_definition'.")
		);

		for(const auto& c : cols) {
			list_data.emplace_back();

			for(const auto& i : c.attribute_range()) {
				list_data.back()[""][i.first] = i.second;
			}

			for(const auto& w : c.child_range("widget")) {
				VALIDATE(w.has_attribute("id"), missing_mandatory_wml_key("[list_data][row][column][widget]", "id"));

				for(const auto& i : w.attribute_range()) {
					list_data.back()[w["id"]][i.first] = i.second;
				}
			}
		}
	}

	return list_data;
}

builder_listbox::builder_listbox(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, header(nullptr)
	, footer(nullptr)
	, list_builder(nullptr)
	, list_data()
	, has_minimum_(cfg["has_minimum"].to_bool(true))
	, has_maximum_(cfg["has_maximum"].to_bool(true))
{
	if(auto h = cfg.optional_child("header")) {
		header = std::make_shared<builder_grid>(*h);
	}

	if(auto f = cfg.optional_child("footer")) {
		footer = std::make_shared<builder_grid>(*f);
	}

	auto l = cfg.optional_child("list_definition");

	VALIDATE(l, _("No list defined."));

	list_builder = std::make_shared<builder_grid>(*l);
	assert(list_builder);

	VALIDATE(list_builder->rows == 1, _("A 'list_definition' should contain one row."));

	if(cfg.has_child("list_data")) {
		list_data = parse_list_data(cfg.mandatory_child("list_data"), list_builder->cols);
	}
}

std::unique_ptr<widget> builder_listbox::build() const
{
	auto widget = std::make_unique<listbox>(*this, generator_base::vertical_list, list_builder);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id << "' with definition '" << definition << "'.";

	const auto conf = widget->cast_config_to<listbox_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);

	auto generator = generator_base::build(has_minimum_, has_maximum_, generator_base::vertical_list, true);
	widget->finalize(std::move(generator), header, footer, list_data);

	return widget;
}

builder_horizontal_listbox::builder_horizontal_listbox(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, list_builder(nullptr)
	, list_data()
	, has_minimum_(cfg["has_minimum"].to_bool(true))
	, has_maximum_(cfg["has_maximum"].to_bool(true))
{
	auto l = cfg.optional_child("list_definition");

	VALIDATE(l, _("No list defined."));

	list_builder = std::make_shared<builder_grid>(*l);
	assert(list_builder);

	VALIDATE(list_builder->rows == 1, _("A 'list_definition' should contain one row."));

	if(cfg.has_child("list_data")) {
		list_data = parse_list_data(cfg.mandatory_child("list_data"), list_builder->cols);
	}
}

std::unique_ptr<widget> builder_horizontal_listbox::build() const
{
	auto widget = std::make_unique<listbox>(*this, generator_base::horizontal_list, list_builder);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id << "' with definition '" << definition << "'.";

	const auto conf = widget->cast_config_to<listbox_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);

	auto generator = generator_base::build(has_minimum_, has_maximum_, generator_base::horizontal_list, true);
	widget->finalize(std::move(generator), nullptr, nullptr, list_data);

	return widget;
}

builder_grid_listbox::builder_grid_listbox(const config& cfg)
	: builder_styled_widget(cfg)
	, vertical_scrollbar_mode(get_scrollbar_mode(cfg["vertical_scrollbar_mode"]))
	, horizontal_scrollbar_mode(get_scrollbar_mode(cfg["horizontal_scrollbar_mode"]))
	, list_builder(nullptr)
	, list_data()
	, has_minimum_(cfg["has_minimum"].to_bool(true))
	, has_maximum_(cfg["has_maximum"].to_bool(true))
{
	auto l = cfg.optional_child("list_definition");

	VALIDATE(l, _("No list defined."));

	list_builder = std::make_shared<builder_grid>(*l);
	assert(list_builder);

	VALIDATE(list_builder->rows == 1, _("A 'list_definition' should contain one row."));

	if(cfg.has_child("list_data")) {
		list_data = parse_list_data(cfg.mandatory_child("list_data"), list_builder->cols);
	}
}

std::unique_ptr<widget> builder_grid_listbox::build() const
{
	auto widget = std::make_unique<listbox>(*this, generator_base::table, list_builder);

	widget->set_vertical_scrollbar_mode(vertical_scrollbar_mode);
	widget->set_horizontal_scrollbar_mode(horizontal_scrollbar_mode);

	DBG_GUI_G << "Window builder: placed listbox '" << id << "' with definition '" << definition << "'.";

	const auto conf = widget->cast_config_to<listbox_definition>();
	assert(conf);

	widget->init_grid(*conf->grid);

	auto generator = generator_base::build(has_minimum_, has_maximum_, generator_base::table, true);
	widget->finalize(std::move(generator), nullptr, nullptr, list_data);

	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
