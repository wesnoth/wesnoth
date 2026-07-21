/*
	Copyright (C) 2024 - 2025
	by Subhraman Sarkar (babaissarkar) <sbmskmm@protonmail.com>
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

#include "gui/widgets/tab_container.hpp"

#include "gui/core/log.hpp"
#include "gettext.hpp"
#include "gui/core/window_builder/helper.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

#include <functional>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

REGISTER_WIDGET(tab_container)

struct tab_container_implementation
{
	template<typename W>
	static W* find(utils::const_clone_ref<tab_container, W> stack,
			const std::string_view id,
			const bool must_be_active)
	{
		for(unsigned i = 0; i < stack.get_tab_count(); ++i) {
			auto* tab_grid = stack.get_tab_grid(i);
			if(!tab_grid) {
				continue;
			}

			if(W* res = tab_grid->find(id, must_be_active)) {
				return res;
			}
		}

		return stack.container_base::find(id, must_be_active);
	}
};

tab_container::tab_container(const implementation::builder_tab_container& builder)
	: container_base(builder, type())
	, state_(ENABLED)
	, tab_count_(0)
	, generator_(nullptr)
{
	const auto conf = cast_config_to<tab_container_definition>();
	assert(conf);

	init_grid(*conf->grid);

	auto generator = generator_base::build(true, true, generator_base::independent, false);

	generator_ = generator.get();
	assert(generator_);

	const widget_item empty_data;
	for(const auto& builder_entry : builder.builders) {
		generator->create_item(-1, *builder_entry, empty_data, nullptr);
	}

	grid* parent_grid = find_widget<grid>("_content_grid", false, true);
	if(parent_grid) {
		parent_grid->swap_child("_page", std::move(generator), false);
	}

	for (const widget_data& row : builder.list_items) {
		add_tab_entry(row);
	}

	tab_count_ = builder.builders.size();

	get_internal_list().connect_signal<event::NOTIFY_MODIFIED>(std::bind(&tab_container::change_selection, this));
}

void tab_container::set_self_active(const bool active)
{
	state_ = active ? ENABLED : DISABLED;
}

bool tab_container::get_active() const
{
	return state_ != DISABLED;
}

unsigned tab_container::get_state() const
{
	return state_;
}

bool tab_container::can_wrap() const
{
	return true;
}

listbox& tab_container::get_internal_list()
{
	return get_grid().find_widget<listbox>("_tab_list", false);
}

void tab_container::add_tab_entry(const widget_data& row)
{
	listbox& list = get_internal_list();
	list.add_row(row);
}

void tab_container::select_tab(unsigned index)
{
	if (index < get_tab_count()) {
		get_internal_list().select_row(index);
		generator_->select_item(index, true);
	}
}

void tab_container::change_selection() {
	select_tab(get_active_tab_index());
	place(get_origin(), get_size());
	queue_redraw();

	fire(event::NOTIFY_MODIFIED, *this, nullptr);
}

widget* tab_container::find(const std::string_view id, const bool must_be_active)
{
	return tab_container_implementation::find<widget>(*this, id, must_be_active);
}

const widget* tab_container::find(const std::string_view id, const bool must_be_active) const
{
	return tab_container_implementation::find<const widget>(*this, id, must_be_active);
}

// }---------- DEFINITION ---------{

tab_container_definition::tab_container_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing tab_container " << id;

	load_resolutions<resolution>(cfg);
}

tab_container_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Note the order should be the same as the enum state_t is tab_container.hpp.
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_enabled", missing_mandatory_wml_tag("tab_container_definition][resolution", "state_enabled")));
	state.emplace_back(VALIDATE_WML_CHILD(cfg, "state_disabled", missing_mandatory_wml_tag("tab_container_definition][resolution", "state_disabled")));

	auto child = VALIDATE_WML_CHILD(cfg, "grid", _("No grid defined for tab container control"));
	grid = std::make_shared<builder_grid>(child);
}

// }---------- BUILDER -----------{

namespace implementation
{

builder_tab_container::builder_tab_container(const config& cfg)
	: implementation::builder_styled_widget(cfg)
{
	if(cfg.has_child("tab")) {
		for(const config& tab : cfg.child_range("tab")) {
			list_items.emplace_back(widget_data{
				{ "image", {{"label", tab["image"].str()}} },
				{ "name", {{"label", tab["name"].t_str()}} }
			});

			if(tab.has_child("data")) {
				auto builder = std::make_shared<builder_grid>(tab.mandatory_child("data"));
				builders.push_back(builder);
			}
		}
	}
}

std::unique_ptr<widget> builder_tab_container::build() const
{
	auto widget = std::make_unique<tab_container>(*this);
	DBG_GUI_G << "Window builder: placed tab_container '" << id
			  << "' with definition '" << definition << "'.";
	return widget;
}

} // namespace implementation

// }------------ END --------------

} //
