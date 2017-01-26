/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/addon/manager.hpp"

#include "addon/info.hpp"
#include "addon/manager.hpp"
#include "addon/state.hpp"

#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"

#include "help/help.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/filter.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/addon_list.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/image.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/pane.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "gui/dialogs/addon/filter_options.hpp"
#include "serialization/string_utils.hpp"
#include "formula/string_utils.hpp"
#include "preferences.hpp"
#include "utils/general.hpp"

#include "config.hpp"

#include "utils/functional.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include "utils/io.hpp"

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_list
 *
 * == Addon list ==
 *
 * This shows the dialog with the addons to install. This dialog is under
 * construction and only used with --new-widgets.
 *
 * @begin{table}{dialog_widgets}
 *
 * addons & & listbox & m &
 *        A listbox that will contain the info about all addons on the server. $
 *
 * -name & & styled_widget & o &
 *        The name of the addon. $
 *
 * -version & & styled_widget & o &
 *        The version number of the addon. $
 *
 * -author & & styled_widget & o &
 *        The author of the addon. $
 *
 * -downloads & & styled_widget & o &
 *        The number of times the addon has been downloaded. $
 *
 * -size & & styled_widget & o &
 *        The size of the addon. $
 *
 * @end{table}
 */

namespace {
	struct filter_transform
	{
		filter_transform(const std::vector<std::string>& filtertext) : filtertext_(filtertext) {}
		bool operator()(const config& cfg) const
		{
			for(const auto& filter : filtertext_)
			{
				bool found = false;
				for(const auto& attribute : cfg.attribute_range())
				{
					std::string val = attribute.second.str();
					if(std::search(val.begin(),
						val.end(),
						filter.begin(),
						filter.end(),
						chars_equal_insensitive)
						!= val.end())
					{
						found = true;
						break;
					}
				}
				if(!found) {
					return false;
				}
			}
			return true;
		}
		const std::vector<std::string> filtertext_;
	};

	/**
	 * Retrieves an element from the given associative container or dies in some
	 * way.
	 *
	 * It fails an @a assert() check or throws an exception if the requested element
	 * does not exist.
	 *
	 * @return An element from the container that is guranteed to have existed
	 *         before running this function.
	 */
	template <typename MapT>
	typename MapT::mapped_type const& const_at(typename MapT::key_type const& key,
										   MapT const& map)
	{
		typename MapT::const_iterator it = map.find(key);
		if(it == map.end()) {
			assert(it != map.end());
			throw std::out_of_range(
					"const_at()"); // Shouldn't get here without disabling assert()
		}
		return it->second;
	}
}

REGISTER_DIALOG(addon_manager)

addon_manager::addon_manager(addons_client& client)
	: orders_()
	, cfg_()
	, cfg_iterators_(cfg_.child_range("campaign"))
	, client_(client)
	, addons_()
	, tracking_info_()
{
}

void addon_manager::on_filtertext_changed(text_box_base* textbox, const std::string& text)
{
	addon_list& addons = find_widget<addon_list>(textbox->get_window(), "addons", true);
	filter_transform filter(utils::split(text, ' '));
	boost::dynamic_bitset<> res;

	const config::const_child_itors& addon_cfgs = cfg_.child_range("campaign");

	for(const auto& a : addons_)
	{
		const config& addon_cfg = *std::find_if(addon_cfgs.begin(), addon_cfgs.end(),
			[&a](const config& cfg)
		{
			return cfg["name"] == a.first;
		});

		res.push_back(filter(addon_cfg));
	}

	addons.set_addon_shown(res);
}

static std::string describe_status_verbose(const addon_tracking_info& state)
{
	std::string s;

	utils::string_map i18n_symbols;
	i18n_symbols["local_version"] = state.installed_version.str();

	switch(state.state) {
		case ADDON_NONE:
			if(!state.can_publish) {
				s = _("addon_state^Not installed");
			} else {
				s = _("addon_state^Published, not installed");
			}
			break;
		case ADDON_INSTALLED:
			if(!state.can_publish) {
				s = _("addon_state^Installed");
			} else {
				s = _("addon_state^Published");
			}
			break;
		case ADDON_NOT_TRACKED:
			if(!state.can_publish) {
				s = _("addon_state^Installed, not tracking local version");
			} else {
				// Published add-ons often don't have local status information,
				// hence untracked. This should be considered normal.
				s = _("addon_state^Published, not tracking local version");
			}
			break;
		case ADDON_INSTALLED_UPGRADABLE: {
			const std::string vstr
					= !state.can_publish
							  ? _("addon_state^Installed ($local_version|), "
								  "upgradable")
							  : _("addon_state^Published ($local_version| "
								  "installed), upgradable");
			s = utils::interpolate_variables_into_string(vstr, &i18n_symbols);
		} break;
		case ADDON_INSTALLED_OUTDATED: {
			const std::string vstr
					= !state.can_publish
							  ? _("addon_state^Installed ($local_version|), "
								  "outdated on server")
							  : _("addon_state^Published ($local_version| "
								  "installed), outdated on server");
			s = utils::interpolate_variables_into_string(vstr, &i18n_symbols);
		} break;
		case ADDON_INSTALLED_BROKEN:
			if(!state.can_publish) {
				s = _("addon_state^Installed, broken");
			} else {
				s = _("addon_state^Published, broken");
			}
			break;
		default:
			s = _("addon_state^Unknown");
	}

	return addon_list::colorify_addon_state_string(s, state.state);
}

void addon_manager::pre_show(window& window)
{
	load_addon_list(window);
	addon_list& list = find_widget<addon_list>(&window, "addons", false);

	find_widget<text_box>(&window, "filter", false).set_text_changed_callback(
		std::bind(&addon_manager::on_filtertext_changed, this, _1, _2));

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(list,
			std::bind(&addon_manager::on_addon_select,
			*this,
			std::ref(window)));
#else
	list.set_callback_value_change(
			dialog_callback<addon_manager, &addon_manager::on_addon_select>);
#endif

	button& url_go_button = find_widget<button>(&window, "url_go", false);
	button& url_copy_button = find_widget<button>(&window, "url_copy", false);
	text_box& url_textbox = find_widget<text_box>(&window, "url", false);

	url_textbox.set_active(false);

	if (!desktop::clipboard::available()) {
		url_copy_button.set_active(false);
		url_copy_button.set_tooltip(_("Clipboard support not found, contact your packager"));
	}

	if(!desktop::open_object_is_supported()) {
		// No point in displaying the button on platforms that can't do
		// open_object().
		url_go_button.set_visible(styled_widget::visibility::invisible);
	}

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "install", false),
			std::bind(&addon_manager::install_selected_addon, this, std::ref(window)));

	connect_signal_mouse_left_click(
			url_go_button,
			std::bind(&addon_manager::browse_url_callback, this, std::ref(url_textbox)));

	connect_signal_mouse_left_click(
			url_copy_button,
			std::bind(&addon_manager::copy_url_callback, this, std::ref(url_textbox)));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "options", false),
			std::bind(&addon_manager::options_button_callback, this, std::ref(window)));

	connect_signal_mouse_left_click(
			find_widget<button>(&window, "show_help", false),
			std::bind(&addon_manager::show_help, this, std::ref(window)));

	on_addon_select(window);
}

void addon_manager::load_addon_list(window& window)
{
	refresh_addon_version_info_cache();

	client_.request_addons_list(cfg_);
	if(!cfg_)
	{
		show_error_message(window.video(),
			_("An error occurred while downloading the "
			"add-ons list from the server."));
		window.close();
	}

	read_addons_list(cfg_, addons_);

	addon_list& list = find_widget<addon_list>(&window, "addons", false);
	list.set_addons(addons_);

	for(const auto& a : addons_)
	{
		tracking_info_[a.first] = get_addon_tracking_info(a.second);
	}

	// TODO: wire up the install buttons in some way
}

void addon_manager::options_button_callback(window& window)
{
	// TODO
	//gui2::addon_filter_options dlg;

	//dlg.set_displayed_status(f_.status);
	//dlg.set_displayed_types(f_.types);
	//dlg.set_sort(f_.sort);
	//dlg.set_direction(f_.direction);

	//dlg.show(window.video());
	UNUSED(window); // Remove this once the code works.
}

void addon_manager::install_selected_addon(window& window)
{
	addon_list& addons = find_widget<addon_list>(&window, "addons", false);
	const addon_info* addon = addons.get_selected_addon();

	if(addon == nullptr) {
		return;
	}

	install_addon(*addon, window);
}

void addon_manager::install_addon(addon_info addon, window& window)
{
	addon_list& addons = find_widget<addon_list>(&window, "addons", false);
	config archive;
	bool download_succeeded = client_.download_addon(archive, addon.id, addon.title);
	if(download_succeeded)
	{
		bool install_succeeded = client_.install_addon(archive, addon);
		if(install_succeeded)
		{
			load_addon_list(window);

			// Reselect the add-on.
			addons.select_addon(addon.id);
			on_addon_select(window);

			return;
		}
	}

	// failure
	const std::string& server_error = client_.get_last_server_error();
	if(server_error != "") {
		show_error_message(window.video(),
			_("The server responded with an error:") + "\n" + server_error);
	}
}

void addon_manager::show_help(window& window)
{
	help::show_help(window.video(), "installing_addons");
}

void addon_manager::browse_url_callback(text_box& url_box)
{
	/* TODO: ask for confirmation */

	desktop::open_object(url_box.get_value());
}

void addon_manager::copy_url_callback(text_box& url_box)
{
	desktop::clipboard::copy_to_clipboard(url_box.get_value(), false);
}

static std::string format_addon_time(time_t time)
{
	if(time) {
		std::ostringstream ss;

		const char* format = preferences::use_twelve_hour_clock_format()
			? "%Y-%m-%d %I:%M %p"
			: "%Y-%m-%d %H:%M";

		ss << util::put_time(std::localtime(&time), format);

		return ss.str();
	}

	return font::unicode_em_dash;
}

void addon_manager::on_addon_select(window& window)
{
	const addon_info* info = find_widget<addon_list>(&window, "addons", false).get_selected_addon();

	if(info == nullptr) {
		return;
	}

	find_widget<drawing>(&window, "image", false).set_label(info->display_icon());

	find_widget<styled_widget>(&window, "title", false).set_label(info->display_title());
	find_widget<styled_widget>(&window, "description", false).set_label(info->description);
	find_widget<styled_widget>(&window, "version", false).set_label(info->version.str());
	find_widget<styled_widget>(&window, "author", false).set_label(info->author);
	find_widget<styled_widget>(&window, "type", false).set_label(info->display_type());

	styled_widget& status = find_widget<styled_widget>(&window, "status", false);
	status.set_label(describe_status_verbose(tracking_info_[info->id]));
	status.set_use_markup(true);

	find_widget<styled_widget>(&window, "size", false).set_label(size_display_string(info->size));
	find_widget<styled_widget>(&window, "downloads", false).set_label(std::to_string(info->downloads));
	find_widget<styled_widget>(&window, "created", false).set_label(format_addon_time(info->created));
	find_widget<styled_widget>(&window, "updated", false).set_label(format_addon_time(info->updated));

	const std::string& feedback_url = info->feedback_url;

	if(!feedback_url.empty()) {
		find_widget<stacked_widget>(&window, "feedback_stack", false).select_layer(1);
		find_widget<text_box>(&window, "url", false).set_value(feedback_url);
	} else {
		find_widget<stacked_widget>(&window, "feedback_stack", false).select_layer(0);
	}

	bool installed = is_installed_addon_status(tracking_info_[info->id].state);

	find_widget<button>(&window, "install", false).set_active(!installed);
	find_widget<button>(&window, "uninstall", false).set_active(installed);
}

} // namespace dialogs
} // namespace gui2
