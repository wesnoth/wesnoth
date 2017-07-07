/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/addon_list.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/multimenu_button.hpp"
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
#include "serialization/string_utils.hpp"
#include "formula/string_utils.hpp"
#include "image.hpp"
#include "language.hpp"
#include "preferences/general.hpp"
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
		explicit filter_transform(const std::vector<std::string>& filtertext) : filtertext_(filtertext) {}
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

	std::string make_display_dependencies(
		const std::string& addon_id,
		const addons_list& addons_list,
		const addons_tracking_list& addon_states)
	{
		const addon_info& addon = const_at(addon_id, addons_list);
		std::string str;

		const std::set<std::string>& deps = addon.resolve_dependencies(addons_list);

		for(const auto& dep_id : deps) {
			addon_info dep;
			addon_tracking_info depstate;

			addons_list::const_iterator ali = addons_list.find(dep_id);
			addons_tracking_list::const_iterator tli = addon_states.find(dep_id);

			if(ali == addons_list.end()) {
				dep.id = dep_id; // Build dummy addon_info.
			} else {
				dep = ali->second;
			}

			if(tli == addon_states.end()) {
				depstate = get_addon_tracking_info(dep);
			} else {
				depstate = tli->second;
			}

			if(!str.empty()) {
				str += ", ";
			}

			str += addon_list::colorify_addon_state_string(dep.display_title(), depstate.state);
		}

		return str;
	}

	std::string langcode_to_string(const std::string& lcode)
	{
		for(const auto & ld : get_languages())
		{
			if(ld.localename == lcode || ld.localename.substr(0, 2) == lcode) {
				return ld.language;
			}
		}

		return "";
	}
}

REGISTER_DIALOG(addon_manager)

const std::vector<std::pair<ADDON_STATUS_FILTER, std::string>> addon_manager::status_filter_types_{
	{FILTER_ALL,           N_("addons_view^All Add-ons")},
	{FILTER_INSTALLED,     N_("addons_view^Installed")},
	{FILTER_UPGRADABLE,    N_("addons_view^Upgradable")},
	{FILTER_NOT_INSTALLED, N_("addons_view^Not Installed")},
};

const std::vector<std::pair<ADDON_TYPE, std::string>> addon_manager::type_filter_types_{
	{ADDON_SP_CAMPAIGN,    N_("addons_of_type^Campaigns")},
	{ADDON_SP_SCENARIO,    N_("addons_of_type^Scenarios")},
	{ADDON_SP_MP_CAMPAIGN, N_("addons_of_type^SP/MP campaigns")},
	{ADDON_MP_CAMPAIGN,    N_("addons_of_type^MP campaigns")},
	{ADDON_MP_SCENARIO,    N_("addons_of_type^MP scenarios")},
	{ADDON_MP_MAPS,        N_("addons_of_type^MP map-packs")},
	{ADDON_MP_ERA,         N_("addons_of_type^MP eras")},
	{ADDON_MP_FACTION,     N_("addons_of_type^MP factions")},
	{ADDON_MP_MOD,         N_("addons_of_type^MP modifications")},
	{ADDON_CORE,           N_("addons_of_type^Cores")},
	{ADDON_MEDIA,          N_("addons_of_type^Resources")},
	// FIXME: (also in WML) should this and Unknown be a single option in the UI?
	{ADDON_OTHER,          N_("addons_of_type^Other")},
	{ADDON_UNKNOWN,        N_("addons_of_type^Unknown")},
};

addon_manager::addon_manager(addons_client& client)
	: orders_()
	, cfg_()
	, client_(client)
	, addons_()
	, tracking_info_()
	, need_wml_cache_refresh_(false)
{
}

void addon_manager::on_filtertext_changed(text_box_base* textbox, const std::string& text)
{
	apply_filters(*textbox->get_window());
}

static std::string describe_status_verbose(const addon_tracking_info& state)
{
	std::string s;

	utils::string_map i18n_symbols {{"local_version", state.installed_version.str()}};

	switch(state.state) {
		case ADDON_NONE:
			s = !state.can_publish
				? _("addon_state^Not installed")
				: _("addon_state^Published, not installed");
			break;
		case ADDON_INSTALLED:
			s = !state.can_publish
				? _("addon_state^Installed")
				: _("addon_state^Published");
			break;
		case ADDON_NOT_TRACKED:
			s = !state.can_publish
				? _("addon_state^Installed, not tracking local version")
				// Published add-ons often don't have local status information,
				// hence untracked. This should be considered normal.
				: _("addon_state^Published, not tracking local version");
			break;
		case ADDON_INSTALLED_UPGRADABLE: {
			const std::string vstr = !state.can_publish
				? _("addon_state^Installed ($local_version|), upgradable")
				: _("addon_state^Published ($local_version| installed), upgradable");

			s = utils::interpolate_variables_into_string(vstr, &i18n_symbols);
		} break;
		case ADDON_INSTALLED_OUTDATED: {
			const std::string vstr = !state.can_publish
				? _("addon_state^Installed ($local_version|), outdated on server")
				: _("addon_state^Published ($local_version| installed), outdated on server");

			s = utils::interpolate_variables_into_string(vstr, &i18n_symbols);
		} break;
		case ADDON_INSTALLED_LOCAL_ONLY:
			s = !state.can_publish
				? _("addon_state^Installed, not ready to publish")
				: _("addon_state^Ready to publish");
			break;
		case ADDON_INSTALLED_BROKEN:
			s = !state.can_publish
				? _("addon_state^Installed, broken")
				: _("addon_state^Published, broken");
			break;
		default:
			s = _("addon_state^Unknown");
	}

	return addon_list::colorify_addon_state_string(s, state.state);
}

void addon_manager::pre_show(window& window)
{
	addon_list& list = find_widget<addon_list>(&window, "addons", false);

	text_box& filter = find_widget<text_box>(&window, "filter", false);
	filter.set_text_changed_callback(std::bind(&addon_manager::on_filtertext_changed, this, _1, _2));

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	connect_signal_notify_modified(list,
		std::bind(&addon_manager::on_addon_select, *this, std::ref(window)));
#else
	list.set_install_function(std::bind(&addon_manager::install_addon,
		this, std::placeholders::_1, std::ref(window)));
	list.set_uninstall_function(std::bind(&addon_manager::uninstall_addon,
		this, std::placeholders::_1, std::ref(window)));
	list.set_update_function(std::bind(&addon_manager::update_addon,
		this, std::placeholders::_1, std::ref(window)));

	list.set_publish_function(std::bind(&addon_manager::publish_addon,
		this, std::placeholders::_1, std::ref(window)));
	list.set_delete_function(std::bind(&addon_manager::delete_addon,
		this, std::placeholders::_1, std::ref(window)));

	list.set_callback_value_change(
		dialog_callback<addon_manager, &addon_manager::on_addon_select>);
#endif

	load_addon_list(window);

	menu_button& status_filter = find_widget<menu_button>(&window, "install_status_filter", false);

	std::vector<config> status_filter_entries;
	for(const auto& f : status_filter_types_) {
		status_filter_entries.emplace_back(config {"label", t_string(f.second, GETTEXT_DOMAIN)});
	}

	status_filter.set_values(status_filter_entries);
	status_filter.connect_click_handler(std::bind(&addon_manager::apply_filters, this, std::ref(window)));

	multimenu_button& type_filter = find_widget<multimenu_button>(&window, "type_filter", false);

	std::vector<config> type_filter_entries;
	for(const auto& f : type_filter_types_) {
		type_filter_entries.emplace_back(config {"label", t_string(f.second, GETTEXT_DOMAIN), "checkbox", false});
	}

	type_filter.set_values(type_filter_entries);
	type_filter.set_callback_toggle_state_change(std::bind(&addon_manager::apply_filters, this, std::ref(window)));

	button& url_go_button = find_widget<button>(&window, "url_go", false);
	button& url_copy_button = find_widget<button>(&window, "url_copy", false);
	text_box& url_textbox = find_widget<text_box>(&window, "url", false);

	url_textbox.set_active(false);

	if(!desktop::clipboard::available()) {
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
		find_widget<button>(&window, "uninstall", false),
		std::bind(&addon_manager::uninstall_selected_addon, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "update", false),
		std::bind(&addon_manager::update_selected_addon, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "publish", false),
		std::bind(&addon_manager::publish_selected_addon, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "delete", false),
		std::bind(&addon_manager::delete_selected_addon, this, std::ref(window)));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "update_all", false),
		std::bind(&addon_manager::update_all_addons, this, std::ref(window)));

	connect_signal_mouse_left_click(
		url_go_button,
		std::bind(&addon_manager::browse_url_callback, this, std::ref(url_textbox)));

	connect_signal_mouse_left_click(
		url_copy_button,
		std::bind(&addon_manager::copy_url_callback, this, std::ref(url_textbox)));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "show_help", false),
		std::bind(&addon_manager::show_help, this, std::ref(window)));

	if(stacked_widget* stk = find_widget<stacked_widget>(&window, "main_stack", false, false)) {
		button& btn = find_widget<button>(&window, "details_toggle", false);
		connect_signal_mouse_left_click(btn, std::bind(&addon_manager::toggle_details, this, std::ref(btn), std::ref(*stk)));
		stk->select_layer(0);
	}

	on_addon_select(window);

	window.set_enter_disabled(true);

	window.keyboard_capture(&filter);
	list.add_list_to_keyboard_chain();

	// Use handle the special addon_list retval to allow installing addons on double click
	window.set_exit_hook(std::bind(&addon_manager::exit_hook, this, std::ref(window)));
}

void addon_manager::toggle_details(button& btn, stacked_widget& stk)
{
	if(stk.current_layer() == 0) {
		btn.set_label(_("addons^Back to List"));
		stk.select_layer(1);
	} else {
		btn.set_label(_("Addon Details"));
		stk.select_layer(0);
	}
}

void addon_manager::load_addon_list(window& window)
{
	if(need_wml_cache_refresh_) {
		refresh_addon_version_info_cache();
	}

	client_.request_addons_list(cfg_);
	if(!cfg_) {
		show_error_message(window.video(), _("An error occurred while downloading the add-ons list from the server."));
		window.close();
	}

	read_addons_list(cfg_, addons_);

	std::vector<std::string> publishable_addons = available_addons();

	for(std::string id : publishable_addons) {
		if(addons_.find(id) == addons_.end()) {
			// Get a config from the addon's pbl file
			// Note that the name= key is necessary or stuff breaks, since the filter code uses this key
			// to match add-ons in the config list. It also fills in addon_info's id field.
			config pbl_cfg = get_addon_pbl_info(id);
			pbl_cfg["name"] = id;

			// Add the add-on to the list.
			addon_info addon(pbl_cfg);
			addon.local_only = true;
			addons_[id] = addon;

			// Add the addon to the config entry
			cfg_.add_child("campaign", pbl_cfg);
		}
	}

	if(addons_.empty()) {
		show_transient_message(window.video(), _("No Add-ons Available"), _("There are no add-ons available for download from this server."));
		window.close();
	}

	addon_list& list = find_widget<addon_list>(&window, "addons", false);
	list.set_addons(addons_);

	bool has_upgradable_addons = false;
	for(const auto& a : addons_) {
		tracking_info_[a.first] = get_addon_tracking_info(a.second);

		if(tracking_info_[a.first].state == ADDON_INSTALLED_UPGRADABLE) {
			has_upgradable_addons = true;
		}
	}

	find_widget<button>(&window, "update_all", false).set_active(has_upgradable_addons);

	apply_filters(window);
}

void addon_manager::reload_list_and_reselect_item(const std::string id, window& window)
{
	load_addon_list(window);

	// Reselect the add-on.
	find_widget<addon_list>(&window, "addons", false).select_addon(id);
	on_addon_select(window);
}

boost::dynamic_bitset<> addon_manager::get_name_filter_visibility(const window& window) const
{
	const text_box& name_filter = find_widget<const text_box>(&window, "filter", false);
	const std::string& text = name_filter.get_value();

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

	return res;
}

boost::dynamic_bitset<> addon_manager::get_status_filter_visibility(const window& window) const
{
	const menu_button& status_filter = find_widget<const menu_button>(&window, "install_status_filter", false);
	const ADDON_STATUS_FILTER selection = status_filter_types_[status_filter.get_value()].first;

	boost::dynamic_bitset<> res;
	for(const auto& a : addons_) {
		const ADDON_STATUS state = tracking_info_.at(a.second.id).state;

		res.push_back(
			(selection == FILTER_ALL) ||
			(selection == FILTER_INSTALLED     && is_installed_addon_status(state)) ||
			(selection == FILTER_UPGRADABLE    && state == ADDON_INSTALLED_UPGRADABLE) ||
			(selection == FILTER_NOT_INSTALLED && state == ADDON_NONE)
		);
	}

	return res;
}

boost::dynamic_bitset<> addon_manager::get_type_filter_visibility(const window& window) const
{
	const multimenu_button& type_filter = find_widget<const multimenu_button>(&window, "type_filter", false);

	boost::dynamic_bitset<> toggle_states = type_filter.get_toggle_states();
	if(toggle_states.none()) {
		// Nothing selected. It means that *all* add-ons are shown.
		boost::dynamic_bitset<> res_flipped(addons_.size());
		return ~res_flipped;
	} else {
		boost::dynamic_bitset<> res;

		for(const auto& a : addons_) {
			int index = std::find_if(type_filter_types_.begin(), type_filter_types_.end(),
				[&a](const std::pair<ADDON_TYPE, std::string>& entry) {
					return entry.first == a.second.type;
				}) - type_filter_types_.begin();
			res.push_back(toggle_states[index]);
		}

		return res;
	}
}

void addon_manager::apply_filters(window& window)
{
	boost::dynamic_bitset<> res = 
		get_status_filter_visibility(window)
		& get_type_filter_visibility(window)
		& get_name_filter_visibility(window);
	find_widget<addon_list>(&window, "addons", false).set_addon_shown(res);
}

template<void(addon_manager::*fptr)(const addon_info& addon, window& window)>
void addon_manager::execute_action_on_selected_addon(window& window)
{
	addon_list& addons = find_widget<addon_list>(&window, "addons", false);
	const addon_info* addon = addons.get_selected_addon();

	if(addon == nullptr) {
		return;
	}

	(this->*fptr)(*addon, window);
}

void addon_manager::install_addon(const addon_info& addon, window& window)
{
	addons_client::install_result result = client_.install_addon_with_checks(addons_, addon);

 	// Take note if any wml_changes occurred
	need_wml_cache_refresh_ |= result.wml_changed;

	if(result.outcome != addons_client::install_outcome::abort) {
		reload_list_and_reselect_item(addon.id, window);
	}
}

void addon_manager::uninstall_addon(const addon_info& addon, window& window)
{
	if(have_addon_pbl_info(addon.id) || have_addon_in_vcs_tree(addon.id)) {
		show_error_message(window.video(),
			_("The following add-on appears to have publishing or version control information stored locally, and will not be removed:") + " " +
				addon.display_title());
		return;
	}

	bool success = remove_local_addon(addon.id);

	if(!success) {
		show_error_message(window.video(), _("The following add-on could not be deleted properly:") + " " + addon.display_title());
	} else {
		need_wml_cache_refresh_ = true;

		reload_list_and_reselect_item(addon.id, window);
	}
}

void addon_manager::update_addon(const addon_info& addon, window& window)
{
	/* Currently, the install and update codepaths are the same, so this function simply
	 * calls the other. Since this might change in the future, I'm leaving this function
	 * here for now.
	 *
	 * - vultraz, 3/12/17
	 */
	install_addon(addon, window);
}

void addon_manager::update_all_addons(window& window)
{
	for(const auto& a : addons_) {
		if(tracking_info_[a.first].state == ADDON_INSTALLED_UPGRADABLE) {
			// TODO: handle result of this
			client_.install_addon_with_checks(addons_, a.second);
		}
	}

	need_wml_cache_refresh_ = true;
	load_addon_list(window);
}

/** Performs all backend and UI actions for publishing the specified add-on. */
void addon_manager::publish_addon(const addon_info& addon, window& window)
{
	std::string server_msg;

	const std::string addon_id = addon.id;
	config cfg = get_addon_pbl_info(addon_id);

	const version_info& version_to_publish = cfg["version"].str();

	if(version_to_publish <= tracking_info_[addon_id].remote_version) {
		const int res = gui2::show_message(window.video(), _("Warning"),
			_("The remote version of this add-on is greater or equal to the version being uploaded. Do you really wish to continue?"),
			gui2::dialogs::message::yes_no_buttons);

		if(res != gui2::window::OK) {
			return;
		}
	}

	if(!::image::exists(cfg["icon"].str())) {
		gui2::show_error_message(window.video(), _("Invalid icon path. Make sure the path points to a valid image."));
	} else if(!client_.request_distribution_terms(server_msg)) {
		gui2::show_error_message(window.video(),
			_("The server responded with an error:") + "\n" + client_.get_last_server_error());
	} else if(gui2::show_message(window.video(), _("Terms"), server_msg, gui2::dialogs::message::ok_cancel_buttons) == gui2::window::OK) {
		if(!client_.upload_addon(addon_id, server_msg, cfg)) {
			gui2::show_error_message(window.video(),
				_("The server responded with an error:") + "\n" + client_.get_last_server_error());
		} else {
			gui2::show_transient_message(window.video(), _("Response"), server_msg);
		}
	}
}

/** Performs all backend and UI actions for taking down the specified add-on. */
void addon_manager::delete_addon(const addon_info& addon, window& window)
{
	const std::string addon_id = addon.id;
	const std::string& text = vgettext(
		"Deleting '$addon|' will permanently erase its download and upload counts on the add-ons server. Do you really wish to continue?",
		{{"addon", make_addon_title(addon_id)}} // FIXME: need the real title!
	);

	const int res = gui2::show_message(
		window.video(), _("Confirm"), text, gui2::dialogs::message::yes_no_buttons);

	if(res != gui2::window::OK) {
		return;
	}

	std::string server_msg;
	if(!client_.delete_remote_addon(addon_id, server_msg)) {
		gui2::show_error_message(window.video(), _("The server responded with an error:") + "\n" + client_.get_last_server_error());
	} else {
		// FIXME: translation needed!
		gui2::show_transient_message(window.video(), _("Response"), server_msg);
	}
}

/** Called when the player double-clicks an add-on. */
void addon_manager::execute_default_action(const addon_info& addon, window& window)
{
	switch(tracking_info_[addon.id].state) {
		case ADDON_NONE:
			install_addon(addon, window);
			break;
		case ADDON_INSTALLED:
			if(!tracking_info_[addon.id].can_publish) {
				utils::string_map symbols{ { "addon", addon.display_title() } };
				int res = gui2::show_message(window.video(), _("Uninstall add-on"),
					vgettext("Do you want to uninstall '$addon|'?", symbols),
					gui2::dialogs::message::ok_cancel_buttons);
				if(res == gui2::window::OK) {
					uninstall_addon(addon, window);
				}
			}
			break;
		case ADDON_INSTALLED_UPGRADABLE:
			update_addon(addon, window);
			break;
		case ADDON_INSTALLED_LOCAL_ONLY:
		case ADDON_INSTALLED_OUTDATED:
			publish_addon(addon, window);
			break;
		default:
			break;
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

		ss << utils::put_time(std::localtime(&time), format);

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

	widget* parent = &window;
	if(stacked_widget* stk = find_widget<stacked_widget>(&window, "main_stack", false, false)) {
		parent = stk->get_layer_grid(1);
	}

	find_widget<drawing>(parent, "image", false).set_label(info->display_icon());

	find_widget<styled_widget>(parent, "title", false).set_label(info->display_title());
	find_widget<styled_widget>(parent, "description", false).set_label(info->description);
	find_widget<styled_widget>(parent, "version", false).set_label(info->version.str());
	find_widget<styled_widget>(parent, "author", false).set_label(info->author);
	find_widget<styled_widget>(parent, "type", false).set_label(info->display_type());

	styled_widget& status = find_widget<styled_widget>(parent, "status", false);
	status.set_label(describe_status_verbose(tracking_info_[info->id]));
	status.set_use_markup(true);

	find_widget<styled_widget>(parent, "size", false).set_label(size_display_string(info->size));
	find_widget<styled_widget>(parent, "downloads", false).set_label(std::to_string(info->downloads));
	find_widget<styled_widget>(parent, "created", false).set_label(format_addon_time(info->created));
	find_widget<styled_widget>(parent, "updated", false).set_label(format_addon_time(info->updated));

	find_widget<styled_widget>(parent, "dependencies", false).set_label(!info->depends.empty()
		? make_display_dependencies(info->id, addons_, tracking_info_)
		: _("None"));

	std::string languages;

	for(const auto& lc : info->locales) {
		const std::string& langlabel = langcode_to_string(lc);
		if(!langlabel.empty()) {
			if(!languages.empty()) {
				languages += ", ";
			}
			languages += langlabel;
		}
	}

	find_widget<styled_widget>(parent, "translations", false).set_label(!languages.empty() ? languages : _("None"));

	const std::string& feedback_url = info->feedback_url;

	if(!feedback_url.empty()) {
		find_widget<stacked_widget>(parent, "feedback_stack", false).select_layer(1);
		find_widget<text_box>(parent, "url", false).set_value(feedback_url);
	} else {
		find_widget<stacked_widget>(parent, "feedback_stack", false).select_layer(0);
	}

	bool installed = is_installed_addon_status(tracking_info_[info->id].state);
	bool updatable = tracking_info_[info->id].state == ADDON_INSTALLED_UPGRADABLE;

	stacked_widget& action_stack = find_widget<stacked_widget>(parent, "action_stack", false);

	if(!tracking_info_[info->id].can_publish) {
		action_stack.select_layer(0);

		stacked_widget& install_update_stack = find_widget<stacked_widget>(parent, "install_update_stack", false);
		install_update_stack.select_layer(updatable ? 1 : 0);

		if(!updatable) {
			find_widget<button>(parent, "install", false).set_active(!installed);
		} else {
			find_widget<button>(parent, "update", false).set_active(true);
		}

		find_widget<button>(parent, "uninstall", false).set_active(installed);
	} else {
		action_stack.select_layer(1);

		// TODO: are these the right flags to check?
		find_widget<button>(parent, "publish", false).set_active(installed);
		find_widget<button>(parent, "delete", false).set_active(!installed);
	}
}

bool addon_manager::exit_hook(window& window)
{
	if(window.get_retval() == addon_list::DEFAULT_ACTION_RETVAL) {
		execute_default_action_on_selected_addon(window);
		return false;
	}

	return true;
}

} // namespace dialogs
} // namespace gui2
