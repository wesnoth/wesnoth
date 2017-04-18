/*
   Copyright (C) 2016 - 2017 by the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "color.hpp"
#include "font/text_formatting.hpp"
#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/event/dispatcher.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

#include <algorithm>

namespace gui2
{

REGISTER_WIDGET(addon_list)

static color_t color_outdated {255, 127, 0};

std::string addon_list::colorify_addon_state_string(const std::string& str, ADDON_STATUS state, bool verbose)
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

	return font::span_color(colorname) + str + "</span>";
}

std::string addon_list::describe_status(const addon_tracking_info& info)
{
	std::string tc, tx;

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

	return colorify_addon_state_string(tx, info.state, true);
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

		std::map<std::string, string_map> data;
		string_map item;

		item["use_markup"] = "true";

		if(!tracking_info.can_publish) {
			item["label"] = addon.display_icon();
			data.emplace("icon", item);

			item["label"] = addon.display_title();
			data.emplace("name", item);
		} else {
			item["label"] = addon.display_icon() + "~BLIT(icons/icon-addon-publish.png)";
			data.emplace("icon", item);

			const std::string publish_name = formatter()
				<< "<span color='#00ff00'>" // GOOD_COLOR
				<< addon.display_title()
				<< "</span>";

			item["label"] = publish_name;
			data.emplace("name", item);
		}

		item["label"] = describe_status(tracking_info);
		data.emplace("installation_status", item);

		item["label"] = addon.version.str();
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
		find_widget<toggle_panel>(row_grid, "list_panel", false).set_retval(DEFAULT_ACTION_RETVAL);

		grid* control_grid = find_widget<grid>(row_grid, "single_install_buttons", false, false);
		if(!control_grid) {
			continue;
		}
		stacked_widget& install_update_stack = find_widget<stacked_widget>(row_grid, "install_update_stack", false);

		if(!tracking_info.can_publish) {
			const bool is_updatable = tracking_info.state == ADDON_INSTALLED_UPGRADABLE;
			const bool is_installed =
				tracking_info.state == ADDON_INSTALLED || tracking_info.state == ADDON_INSTALLED_UPGRADABLE;

			install_update_stack.select_layer(static_cast<int>(is_updatable));

			if(!is_updatable) {
				find_widget<button>(control_grid, "single_install", false).set_active(!is_installed);
				if(install_function_ != nullptr) {
					gui2::event::connect_signal_mouse_left_click(
						find_widget<button>(control_grid, "single_install", false),
						[this, addon](gui2::event::dispatcher&, const gui2::event::ui_event, bool& handled, bool& halt)
					{
						install_function_(addon);
						handled = true;
						halt = true;
					});
				}
			} else {
				find_widget<button>(control_grid, "single_update", false).set_active(true);
				if(update_function_ != nullptr) {
					gui2::event::connect_signal_mouse_left_click(
						find_widget<button>(control_grid, "single_update", false),
						[this, addon](gui2::event::dispatcher&, const gui2::event::ui_event, bool& handled, bool& halt)
					{
						update_function_(addon);
						handled = true;
						halt = true;
					});
				}
			}

			if(is_installed) {
				if(uninstall_function_ != nullptr) {
					gui2::event::connect_signal_mouse_left_click(
						find_widget<button>(control_grid, "single_uninstall", false),
						[this, addon](gui2::event::dispatcher&, const gui2::event::ui_event, bool& handled, bool& halt)
					{
						uninstall_function_(addon);
						handled = true;
						halt = true;
					});
				}
			}

			find_widget<button>(control_grid, "single_uninstall", false).set_active(is_installed);

			find_widget<grid>(control_grid, "single_install_buttons", false).set_visible(install_buttons_visibility_);
			find_widget<label>(row_grid, "installation_status", false).set_visible(install_status_visibility_);
		} else {
			const bool is_updatable = tracking_info.state == ADDON_INSTALLED_OUTDATED;

			button& install_button = find_widget<button>(control_grid, "single_install", false);
			button& update_button = find_widget<button>(control_grid, "single_update", false);
			button& uninstall_button = find_widget<button>(control_grid, "single_uninstall", false);

			install_button.set_active(true);
			update_button.set_active(true);
			uninstall_button.set_active(tracking_info.state == ADDON_INSTALLED);

			if(true) {
				gui2::event::connect_signal_mouse_left_click(
					install_button,
					[this, addon](gui2::event::dispatcher&, const gui2::event::ui_event, bool& handled, bool& halt)
				{
					publish_function_(addon);
					handled = true;
					halt = true;
				});

				install_button.set_tooltip(_("Publish add-on"));
			}

			if(is_updatable) {
				gui2::event::connect_signal_mouse_left_click(
					update_button,
					[this, addon](gui2::event::dispatcher&, const gui2::event::ui_event, bool& handled, bool& halt)
				{
					publish_function_(addon);
					handled = true;
					halt = true;
				});

				update_button.set_tooltip(_("Send new version to server"));
			}

			if(tracking_info.state == ADDON_INSTALLED) {
				gui2::event::connect_signal_mouse_left_click(
					uninstall_button,
					[this, addon](gui2::event::dispatcher&, const gui2::event::ui_event, bool& handled, bool& halt)
				{
					delete_function_(addon);
					handled = true;
					halt = true;
				});

				uninstall_button.set_tooltip(_("Delete add-on from server"));
			}

			install_update_stack.select_layer(static_cast<int>(is_updatable));
		}
	}
}

const addon_info* addon_list::get_selected_addon() const
{
	const listbox& list = find_widget<const listbox>(&get_grid(), "addons", false);

	const int index = list.get_selected_row();
	if(index == -1 || index >= static_cast<int>(addon_vector_.size())) {
		return nullptr;
	}

	return addon_vector_.at(index);
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

	const addon_info& info = **std::find_if(addon_vector_.begin(), addon_vector_.end(),
		[&id](const addon_info* a)
	{
		return a->id == id;
	});

	for(unsigned int i = 0u; i < list.get_item_count(); ++i)
	{
		grid* row = list.get_row_grid(i);
		const label& name_label = find_widget<label>(row, "name", false);
		if(name_label.get_label().base_str() == info.display_title())
		{
			list.select_row(i);
		}
	}
}

listbox& addon_list::get_listbox()
{
	return find_widget<listbox>(&get_grid(), "addons", false);
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

	list.register_sorting_option(0, [this](const int i) { return addon_vector_[i]->title; });
	list.register_sorting_option(1, [this](const int i) { return addon_vector_[i]->author; });
	list.register_sorting_option(2, [this](const int i) { return addon_vector_[i]->size; });
	list.register_sorting_option(3, [this](const int i) { return addon_vector_[i]->downloads; });
	list.register_sorting_option(4, [this](const int i) { return addon_vector_[i]->display_type(); });
}

addon_list_definition::addon_list_definition(const config& cfg) :
	styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing add-on list " << id << "\n";

	load_resolutions<resolution>(cfg);
}

addon_list_definition::resolution::resolution(const config& cfg) :
	resolution_definition(cfg), grid(nullptr)
{
	// Add a dummy state since every widget needs a state.
	static config dummy("draw");
	state.emplace_back(dummy);

	const config& child = cfg.child("grid");
	VALIDATE(child, _("No grid defined."));

	grid = std::make_shared<builder_grid>(child);
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

builder_addon_list::builder_addon_list(const config& cfg) :
	builder_styled_widget(cfg),
	install_status_visibility_(widget::visibility::visible),
	install_buttons_visibility_(widget::visibility::invisible)
{
	if(cfg.has_attribute("install_status_visibility")) {
		install_status_visibility_ = parse_visibility(cfg["install_status_visibility"]);
	}
	if(cfg.has_attribute("install_buttons_visibility")) {
		install_buttons_visibility_ = parse_visibility(cfg["install_buttons_visibility"]);
	}
}

widget* builder_addon_list::build() const
{
	addon_list* widget = new addon_list();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed add-on list '" << id <<
		"' with definition '" << definition << "'.\n";

	auto conf = std::static_pointer_cast<const addon_list_definition::resolution>(widget->config());
	assert(conf != nullptr);

	widget->init_grid(conf->grid);

	widget->set_install_status_visibility(install_status_visibility_);
	widget->set_install_buttons_visibility(install_buttons_visibility_);

	widget->finalize_setup();

	return widget;
}

}

}
