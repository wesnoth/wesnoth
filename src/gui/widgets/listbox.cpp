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
#include "gui/core/log.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/widget_helpers.hpp"
#include "gui/widgets/window.hpp"
#include "sdl/rect.hpp"
#include "wml_exception.hpp"
#include <functional>
#include "utils/optional_fwd.hpp"

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{
// ------------ WIDGET -----------{

REGISTER_WIDGET(listbox)
REGISTER_WIDGET3(listbox_definition, horizontal_listbox, nullptr)
REGISTER_WIDGET3(listbox_definition, grid_listbox, nullptr)

listbox::listbox(const implementation::builder_listbox_base& builder)
	: scrollbar_container(builder, type())
	, generator_(nullptr)
	, placement_(builder.placement)
	, list_builder_(builder.list_builder)
	, orders_()
	, callback_order_change_()
{
	const auto conf = cast_config_to<listbox_definition>();
	assert(conf);

	// FIXME: replacements (rightfully) clobber IDs. Is there a way around that?
	builder_widget::replacements_map replacements;

	if(auto header = builder.header) {
		header->id = "_header_grid";
		replacements.try_emplace("_header_grid_placeholder", std::move(header));
	}

	if(auto footer = builder.footer) {
		footer->id = "_footer_grid";
		replacements.try_emplace("_footer_grid_placeholder", std::move(footer));
	}

	conf->grid->build(get_grid(), replacements);

	// "Inherited."
	scrollbar_container::finalize_setup();

	auto generator = generator_base::build(
		builder.has_minimum,
		builder.has_maximum,
		builder.placement,
		builder.allow_selection);

	// Save our *non-owning* pointer before this gets moved into the grid.
	generator_ = generator.get();
	assert(generator_);

	generator->create_items(-1, *list_builder_, builder.list_data,
		std::bind(&listbox::list_item_clicked, this, std::placeholders::_1));

	// TODO: can we use the replacements system here?
	swap_grid(nullptr, content_grid(), std::move(generator), "_list_grid");
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

	const bool is_horizontal = placement_ == generator_base::horizontal_list;

	// TODO: Fix this for horizontal listboxes
	// Note the we have to use content_grid_ and cannot use "_list_grid" which is what generator_ uses.
	int row_pos_y = is_horizontal ? -1 : generator_->item(row).get_y() - content_grid_->get_y();
	int row_pos_x = is_horizontal ? -1 : 0;

	for(; count; --count) {
		if(generator_->item(row).get_visible() != visibility::invisible) {
			if(is_horizontal) {
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

		resize_needed = !content_resize_request(true);
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

std::size_t listbox::filter_rows_by(const std::function<bool(std::size_t)>& filter)
{
	boost::dynamic_bitset<> mask;
	mask.resize(get_item_count(), true);

	for(std::size_t i = 0; i < mask.size(); ++i) {
		mask[i] = std::invoke(filter, i);
	}

	set_row_shown(mask);
	return mask.count();
}

boost::dynamic_bitset<> listbox::get_rows_shown() const
{
	return generator_->get_items_shown();
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
	utils::optional<unsigned> vertical_scrollbar_position, horizontal_scrollbar_position;

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

	if(const grid* header = get_grid().find_widget<const grid>("_header_grid", false, false)) {
		if(header->get_visible() != widget::visibility::invisible) {
			result.y += header->get_best_size().y;
		}
	}

	if(const grid* footer = get_grid().find_widget<const grid>("_footer_grid", false, false)) {
		if(footer->get_visible() != widget::visibility::invisible) {
			result.y += footer->get_best_size().y;
		}
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

void listbox::initialize_sorter(std::string_view id, generator_sort_array&& array)
{
	auto header = find_widget<grid>("_header_grid", false, false);
	if(!header) return;

	auto toggle = header->find_widget<selectable_item>(id, false, false);
	if(!toggle) return;

	const std::size_t i = orders_.size();
	orders_.emplace_back(toggle, std::move(array));

	widget& w = dynamic_cast<widget&>(*toggle);

	// If the toggle was hidden previously, show it
	w.set_visible(widget::visibility::visible);

	// TODO: we can bind the pair directly if we remove the on-order callback
	connect_signal_notify_modified(w,
		std::bind(&listbox::order_by_column, this, i, std::placeholders::_1));
}

void listbox::order_by_column(unsigned column, widget& widget)
{
	selectable_item& selectable = dynamic_cast<selectable_item&>(widget);

	for(auto& [w, _] : orders_) {
		if(w && w != &selectable) {
			w->set_value(utils::to_underlying(sort_order::type::none));
		}
	}

	auto order = sort_order::get_enum(selectable.get_value()).value_or(sort_order::type::none);
	if(static_cast<unsigned>(order) > orders_[column].second.size()) {
		return;
	}

	if(order == sort_order::type::none) {
		order_by(std::less<unsigned>());
	} else {
		order_by(orders_[column].second[utils::to_underlying(order) - 1]);
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

bool listbox::sort_helper::less(const t_string& lhs, const t_string& rhs)
{
	return translation::icompare(lhs, rhs) < 0;
}

bool listbox::sort_helper::more(const t_string& lhs, const t_string& rhs)
{
	return translation::icompare(lhs, rhs) > 0;
}

void listbox::set_active_sorter(std::string_view id, sort_order::type order, bool select_first)
{
	for(auto& [w, _] : orders_) {
		if(!w || dynamic_cast<widget*>(w)->id() != id) continue;

		// Set the state and fire a modified event to handle updating the list
		w->set_value(utils::to_underlying(order), true);

		if(select_first && generator_->get_item_count() > 0) {
			select_row_at(0);
		}
	}
}

std::pair<widget*, sort_order::type> listbox::get_active_sorter() const
{
	for(const auto& [w, _] : orders_) {
		if(!w) continue;

		auto sort = sort_order::get_enum(w->get_value()).value_or(sort_order::type::none);
		if(sort != sort_order::type::none) {
			return { dynamic_cast<widget*>(w), sort };
		}
	}

	return { nullptr, sort_order::type::none };
}

void listbox::mark_as_unsorted()
{
	for(auto& [w, _] : orders_) {
		if(w) {
			w->set_value(utils::to_underlying(sort_order::type::none));
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
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("listbox_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("listbox_definition][resolution", "state_disabled")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", missing_mandatory_wml_tag("listbox_definition][resolution", "grid"));
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
			_("‘list_data’ must have the same number of columns as the ‘list_definition’.")
		);

		for(const auto& c : cols) {
			list_data.emplace_back();

			for(const auto& [key, value] : c.attribute_range()) {
				list_data.back()[""][key] = value;
			}

			for(const auto& w : c.child_range("widget")) {
				VALIDATE(w.has_attribute("id"), missing_mandatory_wml_key("[list_data][row][column][widget]", "id"));

				for(const auto& [key, value] : w.attribute_range()) {
					list_data.back()[w["id"]][key] = value;
				}
			}
		}
	}

	return list_data;
}

builder_listbox_base::builder_listbox_base(const config& cfg, const generator_base::placement placement)
	: builder_scrollbar_container(cfg)
	, placement(placement)
	, header(nullptr)
	, footer(nullptr)
	, list_builder(nullptr)
	, list_data()
	, has_minimum(cfg["has_minimum"].to_bool(true))
	, has_maximum(cfg["has_maximum"].to_bool(true))
	, allow_selection(cfg["allow_selection"].to_bool(true))
{
	auto l = cfg.optional_child("list_definition");

	VALIDATE(l, _("No list defined."));

	list_builder = std::make_shared<builder_grid>(*l);
	assert(list_builder);

	VALIDATE(list_builder->rows == 1, _("A ‘list_definition’ should contain one row."));

	if(cfg.has_child("list_data")) {
		list_data = parse_list_data(cfg.mandatory_child("list_data"), list_builder->cols);
	}
}

std::unique_ptr<widget> builder_listbox_base::build() const
{
	auto widget = std::make_unique<listbox>(*this);
	DBG_GUI_G << "Window builder: placed listbox '" << id << "' with definition '" << definition << "'.";
	return widget;
}

builder_listbox::builder_listbox(const config& cfg)
	: builder_listbox_base(cfg, generator_base::vertical_list)
{
	if(auto h = cfg.optional_child("header")) {
		header = std::make_shared<builder_grid>(*h);
	}

	if(auto f = cfg.optional_child("footer")) {
		footer = std::make_shared<builder_grid>(*f);
	}
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
