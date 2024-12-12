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

#include "gui/widgets/multi_page.hpp"

#include "gui/core/register_widget.hpp"
#include "gui/widgets/widget_helpers.hpp"
#include "gui/widgets/generator.hpp"

#include "gettext.hpp"
#include "wml_exception.hpp"

#include <functional>

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(multi_page)

multi_page::multi_page(const implementation::builder_multi_page& builder)
	: container_base(builder, type())
	, generator_(nullptr)
	, page_builders_(builder.builders)
{
	const auto conf = cast_config_to<multi_page_definition>();
	assert(conf);

	init_grid(*conf->grid);

	auto generator = generator_base::build(true, true, generator_base::independent, false);

	// Save our *non-owning* pointer before this gets moved into the grid.
	generator_ = generator.get();
	assert(generator_);

	generator->create_items(-1, *page_builders_.begin()->second, builder.data, nullptr);

	// TODO: can we use the replacements system here?
	swap_grid(nullptr, &get_grid(), std::move(generator), "_content_grid");
}

grid& multi_page::add_page(const widget_item& item)
{
	assert(generator_);
	grid& page = generator_->create_item(-1, *page_builders_.begin()->second, item, nullptr);

	return page;
}

grid& multi_page::add_page(const std::string& type, int insert_pos, const widget_item& item)
{
	assert(generator_);
	auto it_builder = page_builders_.find(type);
	VALIDATE(it_builder != page_builders_.end(), "invalid page type '" + type + "'");
	return generator_->create_item(insert_pos, *it_builder->second, item, nullptr);
}

grid& multi_page::add_page(const widget_data& data)
{
	assert(generator_);
	grid& page = generator_->create_item(-1, *page_builders_.begin()->second, data, nullptr);

	return page;
}

grid& multi_page::add_page(const std::string& type, int insert_pos, const widget_data& data)
{
	assert(generator_);
	auto it_builder = page_builders_.find(type);
	VALIDATE(it_builder != page_builders_.end(), "invalid page type '" + type + "'");
	return generator_->create_item(insert_pos, *it_builder->second, data, nullptr);
}

void multi_page::remove_page(const unsigned page, unsigned count)
{
	assert(generator_);

	if(page >= get_page_count()) {
		return;
	}

	if(!count || (page + count) > get_page_count()) {
		count = get_page_count() - page;
	}

	for(; count; --count) {
		generator_->delete_item(page);
	}
}

void multi_page::clear()
{
	assert(generator_);
	generator_->clear();
}

unsigned multi_page::get_page_count() const
{
	assert(generator_);
	return generator_->get_item_count();
}

void multi_page::select_page(const unsigned page, const bool select)
{
	if(page >= get_page_count()) {
		throw std::invalid_argument("invalid page index");
	}
	assert(generator_);
	generator_->select_item(page, select);
}

int multi_page::get_selected_page() const
{
	assert(generator_);
	return generator_->get_selected_item();
}

const grid& multi_page::page_grid(const unsigned page) const
{
	assert(generator_);
	return generator_->item(page);
}

grid& multi_page::page_grid(const unsigned page)
{
	assert(generator_);
	return generator_->item(page);
}

bool multi_page::get_active() const
{
	return true;
}

unsigned multi_page::get_state() const
{
	return 0;
}

bool multi_page::impl_draw_background()
{
	/* DO NOTHING */
	return true;
}

void multi_page::set_self_active(const bool /*active*/)
{
	/* DO NOTHING */
}

// }---------- DEFINITION ---------{

multi_page_definition::multi_page_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing multipage " << id;

	load_resolutions<resolution>(cfg);
}

multi_page_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	auto child = cfg.optional_child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(*child);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_multi_page::builder_multi_page(const config& cfg)
	: implementation::builder_styled_widget(cfg), builders(), data()
{
	for (const config& page : cfg.child_range("page_definition"))
	{
		auto builder = std::make_shared<builder_grid>(page);
		assert(builder);
		builders[page["id"]] = builder;
	}
	VALIDATE(!builders.empty(), _("No page defined."));

	/** @todo This part is untested. */
	auto d = cfg.optional_child("page_data");
	if(!d) {
		return;
	}

	auto builder = builders.begin()->second;
	for(const auto & row : d->child_range("row"))
	{
		unsigned col = 0;

		for(const auto & column : row.child_range("column"))
		{
			data.emplace_back();
			for(const auto& [key, value] : column.attribute_range())
			{
				data.back()[key] = value;
			}
			++col;
		}

		VALIDATE(col == builder->cols,
				 _("‘list_data’ must have "
				   "the same number of columns as the ‘list_definition’."));
	}
}

std::unique_ptr<widget> builder_multi_page::build() const
{
	auto widget = std::make_unique<multi_page>(*this);
	DBG_GUI_G << "Window builder: placed multi_page '" << id << "' with definition '" << definition << "'.";
	return widget;
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
