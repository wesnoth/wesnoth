/*
	Copyright (C) 2016 - 2024
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

#include "gui/widgets/addon_list.hpp"

#include "addon/client.hpp"
#include "color.hpp"
#include "formatter.hpp"
#include "gettext.hpp"
#include "gui/core/event/dispatcher.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/generator.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/markup.hpp"
#include "wml_exception.hpp"

#include <algorithm>

namespace gui2
{
namespace
{
const color_t color_outdated {255, 127, 0};

const unsigned CONTROL_STACK_LAYER_INSTALL = 0;
const unsigned CONTROL_STACK_LAYER_UPDATE  = 1;
const unsigned CONTROL_STACK_LAYER_PUBLISH = 2;

} // end anon namespace

REGISTER_WIDGET(addon_list)

addon_list::addon_list(const implementation::builder_addon_list& builder)
	: container_base(builder, type())
	, addon_vector_()
	, install_status_visibility_(builder.install_status_visibility)
	, install_buttons_visibility_(builder.install_buttons_visibility)
	, install_function_()
	, uninstall_function_()
	, publish_function_()
	, delete_function_()
{
}

std::string addon_list::colorize_addon_state_string(const std::string& str, ADDON_STATUS state, bool verbose)
{
	color_t colorname = font::NORMAL_COLOR;

	switch(state) {
	case ADDON_NONE:
		if(!verbose) {
			return str;
		}
		colorname = font::weapon_details_color;
		break;
	case ADDON_INSTALLED:
	case ADDON_INSTALLED_LOCAL_ONLY:
	case ADDON_NOT_TRACKED:
		colorname = font::GOOD_COLOR;
		break;
	case ADDON_INSTALLED_UPGRADABLE:
		colorname = font::YELLOW_COLOR;
		break;
	case ADDON_INSTALLED_OUTDATED:
		colorname = color_outdated;
		break;
	case ADDON_INSTALLED_BROKEN:
		colorname = font::BAD_COLOR;
		break;
	default:
		colorname = font::GRAY_COLOR;
		break;
	}

	return markup::span_color(colorname, str);
}

std::string addon_list::describe_status(const addon_tracking_info& info)
{
	std::string tx;

	switch(info.state) {
	case ADDON_NONE:
		tx = info.can_publish ? _("addon_state^Published, not installed") : _("addon_state^Not installed");
		break;

	case ADDON_INSTALLED:
	case ADDON_NOT_TRACKED:
		// Consider add-ons without version information as installed
		// for the main display. Their Description info should elaborate
		// on their status.
		tx = info.can_publish ? _("addon_state^Published") : _("addon_state^Installed");
		break;
	case ADDON_INSTALLED_LOCAL_ONLY:
		tx = info.can_publish ? _("addon_state^Ready to publish") : _("addon_state^Installed, not ready to publish");
		break;

	case ADDON_INSTALLED_UPGRADABLE:
		tx = info.can_publish ? _("addon_state^Published, upgradable") : _("addon_state^Installed, upgradable");
		break;

	case ADDON_INSTALLED_OUTDATED:
		tx = info.can_publish ? _("addon_state^Published, outdated on server") : _("addon_state^Installed, outdated on server");
		break;

	case ADDON_INSTALLED_BROKEN:
		tx = info.can_publish ? _("addon_state^Published, broken") : _("addon_state^Installed, broken");
		break;

	default:
		tx = _("addon_state^Unknown");
	}

	return colorize_addon_state_string(tx, info.state, true);
}

void addon_list::addon_action_wrapper(addon_op_func_t& func, const addon_info& addon, bool& handled, bool& halt)
{
	try {
		func(addon);

		handled = halt = true;
	} catch(const addons_client::user_exit&) {
		// User canceled the op.
	}
}

const std::string addon_list::display_title_full_shift(const addon_info& addon) const
{
	const std::string& local_title = addon.display_title_translated();
	const std::string& display_title = addon.display_title();
	if(local_title.empty()) {
		return display_title;
	}
	return local_title + "\n" + markup::tag("small", "(", display_title, ")");
}

void addon_list::set_addons(const addons_list& addons)
{
	listbox& list = get_listbox();
	list.clear();

	addon_vector_.clear();

	for(const auto& a : addons) {
		const addon_info& addon = a.second;
		addon_tracking_info tracking_info = get_addon_tracking_info(addon);

		addon_vector_.push_back(&addon);

		widget_data data;
		widget_item item;

		if(!tracking_info.can_publish) {
			item["label"] = addon.display_icon();
			data.emplace("icon", item);

			item["label"] = display_title_full_shift(addon);
			data.emplace("name", item);
		} else {
			item["label"] = addon.display_icon() + "~SCALE(72,72)~BLIT(icons/icon-addon-publish.png,8,8)";
			data.emplace("icon", item);

			const std::string publish_name = markup::span_color(font::GOOD_COLOR, display_title_full_shift(addon));

			item["label"] = publish_name;
			data.emplace("name", item);
		}

		item["label"] = describe_status(tracking_info);
		data.emplace("installation_status", item);

		// If the addon is upgradable or ourdated on server, we display the two relevant
		// versions directly in the list for convenience.
		const bool special_version_display =
			tracking_info.state == ADDON_INSTALLED_UPGRADABLE ||
			tracking_info.state == ADDON_INSTALLED_OUTDATED;

		std::ostringstream ss;
		if(special_version_display) {
			ss << tracking_info.installed_version.str() << "\n";
		}

		ss << (*addon.versions.begin()).str();

		if(special_version_display) {
			ss.str(colorize_addon_state_string(ss.str(), tracking_info.state, false));
		}

		item["label"] = ss.str();
		data.emplace("version", item);

		item["label"] = addon.author;
		data.emplace("author", item);

		item["label"] = size_display_string(addon.size);
		data.emplace("size", item);

		item["label"] = std::to_string(addon.downloads);
		data.emplace("downloads", item);

		item["label"] = addon.display_type();
		data.emplace("type", item);

		grid* row_grid = &list.add_row(data);

		// Set special retval for the toggle panels
		row_grid->find_widget<toggle_panel>("list_panel").set_retval(DEFAULT_ACTION_RETVAL);

		// The control button grid is excluded on lower resolutions.
		grid* control_grid = row_grid->find_widget<grid>("single_install_buttons", false, false);
		if(!control_grid) {
			continue;
		}

		//
		// Set up the inline control buttons.
		//
		stacked_widget& install_update_stack = control_grid->find_widget<stacked_widget>("install_update_stack");

		// These three buttons are in the install_update_stack. Only one is shown depending on the addon's state.
		button& install_button   = control_grid->find_widget<button>("single_install");
		button& update_button    = control_grid->find_widget<button>("single_update");
		button& publish_button   = control_grid->find_widget<button>("single_publish");

		// This button is always shown.
		button& uninstall_button = control_grid->find_widget<button>("single_uninstall");

		const bool is_installed = is_installed_addon_status(tracking_info.state);
		const bool is_local = tracking_info.state == ADDON_INSTALLED_LOCAL_ONLY;

		// Select the right button layer and set its callback.
		if(tracking_info.can_publish) {
			install_update_stack.select_layer(CONTROL_STACK_LAYER_PUBLISH);

			publish_button.set_active(true);

			if(publish_function_ != nullptr) {
				connect_signal_mouse_left_click(publish_button,
					std::bind(&addon_list::addon_action_wrapper, this, publish_function_, std::ref(addon), std::placeholders::_3, std::placeholders::_4));

				install_button.set_tooltip(_("Publish add-on"));
			}
		} else if(tracking_info.state == ADDON_INSTALLED_UPGRADABLE) {
			install_update_stack.select_layer(CONTROL_STACK_LAYER_UPDATE);

			update_button.set_active(true);

			if(update_function_ != nullptr) {
				connect_signal_mouse_left_click(update_button,
					std::bind(&addon_list::addon_action_wrapper, this, update_function_, std::ref(addon), std::placeholders::_3, std::placeholders::_4));
			}
		} else {
			install_update_stack.select_layer(CONTROL_STACK_LAYER_INSTALL);

			install_button.set_active(!is_installed);

			if(install_function_ != nullptr) {
				connect_signal_mouse_left_click(install_button,
					std::bind(&addon_list::addon_action_wrapper, this, install_function_, std::ref(addon), std::placeholders::_3, std::placeholders::_4));
			}
		}

		// Set up the Uninstall button.
		if(tracking_info.can_publish) {
			// Use the uninstall button as a delete-from-server button if the addon's already been published...
			uninstall_button.set_active(!is_local);

			if(!is_local && delete_function_ != nullptr) {
				connect_signal_mouse_left_click(uninstall_button,
					std::bind(&addon_list::addon_action_wrapper, this, delete_function_, std::ref(addon), std::placeholders::_3, std::placeholders::_4));

				uninstall_button.set_tooltip(_("Delete add-on from server"));
			}
		} else {
			// ... else it functions as normal.
			uninstall_button.set_active(is_installed);

			if(is_installed && uninstall_function_ != nullptr) {
				connect_signal_mouse_left_click(uninstall_button,
					std::bind(&addon_list::addon_action_wrapper, this, uninstall_function_, std::ref(addon), std::placeholders::_3, std::placeholders::_4));
			}
		}

		control_grid->set_visible(install_buttons_visibility_);
		row_grid->find_widget<label>("installation_status").set_visible(install_status_visibility_);
	}

	select_first_addon();
}

const addon_info* addon_list::get_selected_addon() const
{
	const listbox& list = get_grid().find_widget<const listbox>("addons");

	try {
		return addon_vector_.at(list.get_selected_row());
	} catch(const std::out_of_range&) {
		return nullptr;
	}
}

std::string addon_list::get_remote_addon_id()
{
	const addon_info* addon = get_selected_addon();
	if(addon == nullptr || !get_addon_tracking_info(*addon).can_publish) {
		return "";
	} else {
		return addon->id;
	}
}

void addon_list::select_addon(const std::string& id)
{
	listbox& list = get_listbox();

	auto iter = std::find_if(addon_vector_.begin(), addon_vector_.end(),
		[&id](const addon_info* a) { return a->id == id; }
	);

	// Corner case: if you publish an addon with an out-of-folder .pbl file and
	// delete it locally before deleting it from the server, the game will try
	// to reselect an addon that now doesn't exist.
	//
	// I don't anticipate this check will match very often, but in the case that
	// some other weird corner case crops up, it's probably better to just exit
	// silently than asserting, as was the pervious behavior.
	if(iter == addon_vector_.end()) {
		return;
	}

	const addon_info& info = **iter;

	for(unsigned int i = 0u; i < list.get_item_count(); ++i) {
		grid* row = list.get_row_grid(i);

		const label& name_label = row->find_widget<label>("name");
		if(name_label.get_label().base_str() == display_title_full_shift(info)) {
			list.select_row(i);
		}
	}
}

listbox& addon_list::get_listbox()
{
	return get_grid().find_widget<listbox>("addons");
}

void addon_list::add_list_to_keyboard_chain()
{
	if(window* window = get_window()) {
		window->add_to_keyboard_chain(&get_listbox());
	}
}

void addon_list::finalize_setup()
{
	listbox& list = get_listbox();

	list.set_sorters(
		[this](const std::size_t i) { return t_string(addon_vector_[i]->display_title_full()); },
		[this](const std::size_t i) { return addon_vector_[i]->author; },
		[this](const std::size_t i) { return addon_vector_[i]->size; },
		[this](const std::size_t i) { return addon_vector_[i]->downloads; },
		[this](const std::size_t i) { return t_string(addon_vector_[i]->display_type()); }
	);

	list.set_active_sorter("sort_0", sort_order::type::ascending);
}

void addon_list::set_addon_order(const addon_sort_func& func)
{
	listbox& list = get_listbox();

	generator_base::order_func generator_func = [this, func](unsigned a, unsigned b)
	{
		return func(*addon_vector_[a], *addon_vector_[b]);
	};

	list.mark_as_unsorted();
	list.order_by(generator_func);
}

void addon_list::select_first_addon()
{
	if(addon_vector_.empty()) {
		// Happens in the dialog unit test.
		return;
	}
	get_listbox().select_row_at(0);
}

addon_list_definition::addon_list_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing add-on list " << id;

	load_resolutions<resolution>(cfg);
}

addon_list_definition::resolution::resolution(const config& cfg)
	: resolution_definition(cfg), grid(nullptr)
{
	// Add a dummy state since every widget needs a state.
	static config dummy("draw");
	state.emplace_back(dummy);

	auto child = cfg.optional_child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(*child);
}

namespace implementation
{

static widget::visibility parse_visibility(const std::string& str)
{
	if(str == "visible") {
		return widget::visibility::visible;
	} else if(str == "hidden") {
		return widget::visibility::hidden;
	} else if(str == "invisible") {
		return widget::visibility::invisible;
	} else {
		FAIL("Invalid visibility value");
	}
}

builder_addon_list::builder_addon_list(const config& cfg)
	: builder_styled_widget(cfg)
	, install_status_visibility(widget::visibility::visible)
	, install_buttons_visibility(widget::visibility::invisible)
{
	if(cfg.has_attribute("install_status_visibility")) {
		install_status_visibility = parse_visibility(cfg["install_status_visibility"]);
	}

	if(cfg.has_attribute("install_buttons_visibility")) {
		install_buttons_visibility = parse_visibility(cfg["install_buttons_visibility"]);
	}
}

std::unique_ptr<widget> builder_addon_list::build() const
{
	auto widget = std::make_unique<addon_list>(*this);

	DBG_GUI_G << "Window builder: placed add-on list '" << id <<
		"' with definition '" << definition << "'.";

	const auto conf = widget->cast_config_to<addon_list_definition>();
	assert(conf != nullptr);

	widget->init_grid(*conf->grid);
	widget->finalize_setup();

	return widget;
}

} // end namespace implementation

} // end namespace gui2
