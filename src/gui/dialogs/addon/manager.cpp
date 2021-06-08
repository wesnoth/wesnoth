/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/dialogs/addon/license_prompt.hpp"
#include "gui/dialogs/addon/addon_auth.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/multimenu_button.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/pane.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "serialization/string_utils.hpp"
#include "formula/string_utils.hpp"
#include "picture.hpp"
#include "language.hpp"
#include "preferences/general.hpp"
#include "utils/general.hpp"

#include "config.hpp"

#include <functional>

#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace gui2::dialogs
{

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
						utils::chars_equal_insensitive)
						!= val.end())
					{
						found = true;
						break;
					}
				}
				for(const config& child : cfg.child_range("translation")) {
					for(const auto& attribute : child.attribute_range()) {
						std::string val = attribute.second.str();
						if(translation::ci_search(val, filter)) {
							found = true;
							break;
						}
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

	std::string make_display_dependencies(
		const std::string& addon_id,
		const addons_list& addons_list,
		const addons_tracking_list& addon_states)
	{
		const addon_info& addon = addons_list.at(addon_id);
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

			str += addon_list::colorize_addon_state_string(dep.display_title_translated_or_original(), depstate.state);
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
	{FILTER_PUBLISHABLE,   N_("addons_view^Publishable")},
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
	{ADDON_MOD,            N_("addons_of_type^Modifications")},
	{ADDON_CORE,           N_("addons_of_type^Cores")},
	{ADDON_MEDIA,          N_("addons_of_type^Resources")},
	// FIXME: (also in WML) should this and Unknown be a single option in the UI?
	{ADDON_OTHER,          N_("addons_of_type^Other")},
	{ADDON_UNKNOWN,        N_("addons_of_type^Unknown")},
};

const std::vector<addon_manager::addon_order> addon_manager::all_orders_{
	{N_("addons_order^Name ($order)"), "name", 0,
	[](const addon_info& a, const addon_info& b) { return a.title < b.title; },
	[](const addon_info& a, const addon_info& b) { return a.title > b.title; }},
	{N_("addons_order^Author ($order)"), "author", 1,
	[](const addon_info& a, const addon_info& b) { return a.author < b.author; },
	[](const addon_info& a, const addon_info& b) { return a.author > b.author; }},
	{N_("addons_order^Size ($order)"), "size", 2,
	[](const addon_info& a, const addon_info& b) { return a.size < b.size; },
	[](const addon_info& a, const addon_info& b) { return a.size > b.size; }},
	{N_("addons_order^Downloads ($order)"), "downloads", 3,
	[](const addon_info& a, const addon_info& b) { return a.downloads < b.downloads; },
	[](const addon_info& a, const addon_info& b) { return a.downloads > b.downloads; }},
	{N_("addons_order^Type ($order)"), "type", 4,
	[](const addon_info& a, const addon_info& b) { return a.display_type() < b.display_type(); },
	[](const addon_info& a, const addon_info& b) { return a.display_type() > b.display_type(); }},
	{N_("addons_order^Last updated ($datelike_order)"), "last_updated", -1,
	[](const addon_info& a, const addon_info& b) { return a.updated < b.updated; },
	[](const addon_info& a, const addon_info& b) { return a.updated > b.updated; }},
	{N_("addons_order^First uploaded ($datelike_order)"), "first_uploaded", -1,
	[](const addon_info& a, const addon_info& b) { return a.created < b.created; },
	[](const addon_info& a, const addon_info& b) { return a.created > b.created; }}
};

namespace
{
struct addon_tag
{
	/** Text to match against addon_info.tags() */
	std::string id;
	/** What to show in the filter's drop-down list */
	std::string label;
	/** Shown when hovering over an entry in the filter's drop-down list */
	std::string tooltip;
};

const std::vector<addon_tag> tag_filter_types_{
	{"cooperative", N_("addon_tag^Cooperative"),
		// TRANSLATORS: tooltip in the drop-down menu for filtering add-ons
		N_("addon_tag^All human players are on the same team, versus the AI")},
	{"cosmetic", N_("addon_tag^Cosmetic"),
		// TRANSLATORS: tooltip in the drop-down menu for filtering add-ons
		N_("addon_tag^These make the game look different, without changing gameplay")},
	{"difficulty", N_("addon_tag^Difficulty"),
		// TRANSLATORS: tooltip in the drop-down menu for filtering add-ons
		N_("addon_tag^Can make campaigns easier or harder")},
	{"rng", N_("addon_tag^RNG"),
		// TRANSLATORS: tooltip in the drop-down menu for filtering add-ons
		N_("addon_tag^Modify the randomness in the combat mechanics, or remove it entirely")},
	{"survival", N_("addon_tag^Survival"),
		// TRANSLATORS: tooltip in the drop-down menu for filtering add-ons
		N_("addon_tag^Fight against waves of enemies")},
	{"terraforming", N_("addon_tag^Terraforming"),
		// TRANSLATORS: tooltip in the drop-down menu for filtering add-ons
		N_("addon_tag^Players can change the terrain")},
};
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

	return addon_list::colorize_addon_state_string(s, state.state);
}

void addon_manager::pre_show(window& window)
{
	window.set_escape_disabled(true);

	addon_list& list = find_widget<addon_list>(&window, "addons", false);

	text_box& filter = find_widget<text_box>(&window, "filter", false);
	filter.set_text_changed_callback(std::bind(&addon_manager::apply_filters, this));

	list.set_install_function(std::bind(&addon_manager::install_addon,
		this, std::placeholders::_1));
	list.set_uninstall_function(std::bind(&addon_manager::uninstall_addon,
		this, std::placeholders::_1));
	list.set_update_function(std::bind(&addon_manager::update_addon,
		this, std::placeholders::_1));

	list.set_publish_function(std::bind(&addon_manager::publish_addon,
		this, std::placeholders::_1));
	list.set_delete_function(std::bind(&addon_manager::delete_addon,
		this, std::placeholders::_1));

	list.set_modified_signal_handler([this]() { on_addon_select(); });

	fetch_addons_list();
	load_addon_list();

	menu_button& status_filter = find_widget<menu_button>(&window, "install_status_filter", false);

	std::vector<config> status_filter_entries;
	for(const auto& f : status_filter_types_) {
		status_filter_entries.emplace_back("label", t_string(f.second, GETTEXT_DOMAIN));
	}

	status_filter.set_values(status_filter_entries);

	connect_signal_notify_modified(status_filter,
		std::bind(&addon_manager::apply_filters, this));

	// The tag filter
	auto& tag_filter = find_widget<multimenu_button>(&window, "tag_filter", false);

	std::vector<config> tag_filter_entries;
	for(const auto& f : tag_filter_types_) {
		tag_filter_entries.emplace_back("label", t_string(f.label, GETTEXT_DOMAIN), "checkbox", false);
		if(!f.tooltip.empty()) {
			tag_filter_entries.back()["tooltip"] = t_string(f.tooltip, GETTEXT_DOMAIN);
		}
	}

	tag_filter.set_values(tag_filter_entries);

	connect_signal_notify_modified(tag_filter, std::bind(&addon_manager::apply_filters, this));

	// The type filter
	multimenu_button& type_filter = find_widget<multimenu_button>(&window, "type_filter", false);

	std::vector<config> type_filter_entries;
	for(const auto& f : type_filter_types_) {
		type_filter_entries.emplace_back("label", t_string(f.second, GETTEXT_DOMAIN), "checkbox", false);
	}

	type_filter.set_values(type_filter_entries);

	connect_signal_notify_modified(type_filter,
		std::bind(&addon_manager::apply_filters, this));

	// Sorting order
	menu_button& order_dropdown = find_widget<menu_button>(&window, "order_dropdown", false);

	std::vector<config> order_dropdown_entries;
	for(const auto& f : all_orders_) {
		utils::string_map symbols;

		symbols["order"] = _("ascending");
		// TRANSLATORS: Sorting order of dates, oldest first
		symbols["datelike_order"] = _("oldest to newest");
		config entry{"label", VGETTEXT(f.label.c_str(), symbols)};
		order_dropdown_entries.push_back(entry);
		symbols["order"] = _("descending");
		// TRANSLATORS: Sorting order of dates, newest first
		symbols["datelike_order"] = _("newest to oldest");
		entry["label"] = VGETTEXT(f.label.c_str(), symbols);
		order_dropdown_entries.push_back(entry);
	}

	order_dropdown.set_values(order_dropdown_entries);
	{
		const std::string saved_order_name = preferences::addon_manager_saved_order_name();
		const preferences::SORT_ORDER saved_order_direction =
			static_cast<preferences::SORT_ORDER>(preferences::addon_manager_saved_order_direction());

		if(!saved_order_name.empty()) {
			auto order_it = std::find_if(all_orders_.begin(), all_orders_.end(),
				[&saved_order_name](const addon_order& order) {return order.as_preference == saved_order_name;});
			if(order_it != all_orders_.end()) {
				int index = 2 * (std::distance(all_orders_.begin(), order_it));
				addon_list::addon_sort_func func;
				if(saved_order_direction == preferences::SORT_ORDER::ASCENDING) {
					func = order_it->sort_func_asc;
				} else {
					func = order_it->sort_func_desc;
					++index;
				}
				find_widget<menu_button>(&window, "order_dropdown", false).set_value(index, false);
				auto& addons = find_widget<addon_list>(&window, "addons", false);
				addons.set_addon_order(func);
				addons.select_first_addon();
			}
		}
	}

	connect_signal_notify_modified(order_dropdown,
		std::bind(&addon_manager::order_addons, this));

	label& url_label = find_widget<label>(&window, "url", false);

	url_label.set_use_markup(true);
	url_label.set_link_aware(true);

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "install", false),
		std::bind(&addon_manager::install_selected_addon, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "uninstall", false),
		std::bind(&addon_manager::uninstall_selected_addon, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "update", false),
		std::bind(&addon_manager::update_selected_addon, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "publish", false),
		std::bind(&addon_manager::publish_selected_addon, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "delete", false),
		std::bind(&addon_manager::delete_selected_addon, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "update_all", false),
		std::bind(&addon_manager::update_all_addons, this));

	connect_signal_mouse_left_click(
		find_widget<button>(&window, "show_help", false),
		std::bind(&addon_manager::show_help, this));

	if(stacked_widget* stk = find_widget<stacked_widget>(&window, "main_stack", false, false)) {
		button& btn = find_widget<button>(&window, "details_toggle", false);
		connect_signal_mouse_left_click(btn, std::bind(&addon_manager::toggle_details, this, std::ref(btn), std::ref(*stk)));
		stk->select_layer(0);
	}

	widget* version_filter_parent = &window;
	if(stacked_widget* stk = find_widget<stacked_widget>(&window, "main_stack", false, false)) {
		version_filter_parent = stk->get_layer_grid(1);
	}

	menu_button& version_filter = find_widget<menu_button>(version_filter_parent, "version_filter", false);
	connect_signal_notify_modified(version_filter,
		std::bind(&addon_manager::on_selected_version_change, this));

	on_addon_select();

	window.set_enter_disabled(true);

	window.keyboard_capture(&filter);
	list.add_list_to_keyboard_chain();

	list.set_callback_order_change(std::bind(&addon_manager::on_order_changed, this,
		std::placeholders::_1, std::placeholders::_2));

	// Use handle the special addon_list retval to allow installing addons on double click
	window.set_exit_hook(std::bind(&addon_manager::exit_hook, this, std::placeholders::_1));
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

void addon_manager::fetch_addons_list()
{
	client_.request_addons_list(cfg_);
	if(!cfg_) {
		gui2::show_error_message(_("An error occurred while downloading the add-ons list from the server."));
		get_window()->close();
	}
}

void addon_manager::load_addon_list()
{
	if(need_wml_cache_refresh_) {
		refresh_addon_version_info_cache();
	}

	read_addons_list(cfg_, addons_);

	std::vector<std::string> publishable_addons = available_addons();

	for(std::string id : publishable_addons) {
		if(addons_.find(id) == addons_.end()) {
			// Get a config from the addon's pbl file
			// Note that the name= key is necessary or stuff breaks, since the filter code uses this key
			// to match add-ons in the config list. It also fills in addon_info's id field. It's also
			// neccessay to set local_only here so that flag can be properly set after addons_ is cleared
			// and recreated by read_addons_list.
			config pbl_cfg = get_addon_pbl_info(id);
			pbl_cfg["name"] = id;
			pbl_cfg["local_only"] = true;

			// Add the add-on to the list.
			addon_info addon(pbl_cfg);
			addons_[id] = addon;

			// Add the addon to the config entry
			cfg_.add_child("campaign", std::move(pbl_cfg));
		}
	}

	if(addons_.empty()) {
		show_transient_message(_("No Add-ons Available"), _("There are no add-ons available for download from this server."));
	}

	addon_list& list = find_widget<addon_list>(get_window(), "addons", false);
	list.set_addons(addons_);

	bool has_upgradable_addons = false;
	for(const auto& a : addons_) {
		tracking_info_[a.first] = get_addon_tracking_info(a.second);

		if(tracking_info_[a.first].state == ADDON_INSTALLED_UPGRADABLE) {
			has_upgradable_addons = true;
		}
	}

	find_widget<button>(get_window(), "update_all", false).set_active(has_upgradable_addons);

	apply_filters();
}

void addon_manager::reload_list_and_reselect_item(const std::string id)
{
	load_addon_list();

	// Reselect the add-on.
	find_widget<addon_list>(get_window(), "addons", false).select_addon(id);
	on_addon_select();
}

boost::dynamic_bitset<> addon_manager::get_name_filter_visibility() const
{
	const text_box& name_filter = find_widget<const text_box>(get_window(), "filter", false);
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

boost::dynamic_bitset<> addon_manager::get_status_filter_visibility() const
{
	const menu_button& status_filter = find_widget<const menu_button>(get_window(), "install_status_filter", false);
	const ADDON_STATUS_FILTER selection = status_filter_types_[status_filter.get_value()].first;

	boost::dynamic_bitset<> res;
	for(const auto& a : addons_) {
		const addon_tracking_info& info = tracking_info_.at(a.second.id);

		res.push_back(
			(selection == FILTER_ALL) ||
			(selection == FILTER_INSTALLED     && is_installed_addon_status(info.state)) ||
			(selection == FILTER_UPGRADABLE    && info.state == ADDON_INSTALLED_UPGRADABLE) ||
			(selection == FILTER_PUBLISHABLE   && info.can_publish == true) ||
			(selection == FILTER_NOT_INSTALLED && info.state == ADDON_NONE)
		);
	}

	return res;
}

boost::dynamic_bitset<> addon_manager::get_tag_filter_visibility() const
{
	const auto& tag_filter = find_widget<const multimenu_button>(get_window(), "tag_filter", false);
	const auto toggle_states = tag_filter.get_toggle_states();
	if(toggle_states.none()) {
		// Nothing selected. It means that all add-ons are shown.
		boost::dynamic_bitset<> res_flipped(addons_.size());
		return ~res_flipped;
	}

	std::vector<std::string> selected_tags;
	for(std::size_t i = 0; i < tag_filter_types_.size(); ++i) {
		if(toggle_states[i]) {
			selected_tags.push_back(tag_filter_types_[i].id);
		}
	}

	boost::dynamic_bitset<> res;
	for(const auto& a : addons_) {
		bool matched_tag = false;
		for(const auto& id : selected_tags) {
			if(utils::contains(a.second.tags, id)) {
				matched_tag = true;
				break;
			}
		}
		res.push_back(matched_tag);
	}

	return res;
}

boost::dynamic_bitset<> addon_manager::get_type_filter_visibility() const
{
	const multimenu_button& type_filter = find_widget<const multimenu_button>(get_window(), "type_filter", false);

	boost::dynamic_bitset<> toggle_states = type_filter.get_toggle_states();
	if(toggle_states.none()) {
		// Nothing selected. It means that *all* add-ons are shown.
		boost::dynamic_bitset<> res_flipped(addons_.size());
		return ~res_flipped;
	} else {
		boost::dynamic_bitset<> res;

		for(const auto& a : addons_) {
			int index = std::distance(type_filter_types_.begin(),
				std::find_if(type_filter_types_.begin(), type_filter_types_.end(),
					[&a](const std::pair<ADDON_TYPE, std::string>& entry) {
						return entry.first == a.second.type;
					})
				);
			res.push_back(toggle_states[index]);
		}

		return res;
	}
}

void addon_manager::apply_filters()
{
	// In the small-screen layout, the text_box for the filter keeps keyboard focus even when the
	// details panel is visible, which means this can be called when the list isn't visible. That
	// causes problems both because find_widget can throw exceptions, but also because changing the
	// filters can trigger on_addon_select and thus affect which add-on's details are shown.
	//
	// Quick workaround is to not process the new filter if the list isn't visible.
	auto list = find_widget<addon_list>(get_window(), "addons", false, false);
	if(!list) {
		return;
	}

	boost::dynamic_bitset<> res =
		get_status_filter_visibility()
		& get_tag_filter_visibility()
		& get_type_filter_visibility()
		& get_name_filter_visibility();
	list->set_addon_shown(res);
}

void addon_manager::order_addons()
{
	const menu_button& order_menu = find_widget<const menu_button>(get_window(), "order_dropdown", false);
	const addon_order& order_struct = all_orders_.at(order_menu.get_value() / 2);
	preferences::SORT_ORDER order = order_menu.get_value() % 2 == 0 ? preferences::SORT_ORDER::ASCENDING : preferences::SORT_ORDER::DESCENDING;
	addon_list::addon_sort_func func;
	if(order == preferences::SORT_ORDER::ASCENDING) {
		func = order_struct.sort_func_asc;
	} else {
		func = order_struct.sort_func_desc;
	}

	find_widget<addon_list>(get_window(), "addons", false).set_addon_order(func);
	preferences::set_addon_manager_saved_order_name(order_struct.as_preference);
	preferences::set_addon_manager_saved_order_direction(order);
}

void addon_manager::on_order_changed(unsigned int sort_column, preferences::SORT_ORDER order)
{
	menu_button& order_menu = find_widget<menu_button>(get_window(), "order_dropdown", false);
	auto order_it = std::find_if(all_orders_.begin(), all_orders_.end(),
		[sort_column](const addon_order& order) {return order.column_index == static_cast<int>(sort_column);});
	int index = 2 * (std::distance(all_orders_.begin(), order_it));
	if(order == preferences::SORT_ORDER::DESCENDING) {
		++index;
	}
	order_menu.set_value(index);
	preferences::set_addon_manager_saved_order_name(order_it->as_preference);
	preferences::set_addon_manager_saved_order_direction(order);
}

template<void(addon_manager::*fptr)(const addon_info& addon)>
void addon_manager::execute_action_on_selected_addon()
{
	// Explicitly return to the main page if we're in low-res mode so the list is visible.
	if(stacked_widget* stk = find_widget<stacked_widget>(get_window(), "main_stack", false, false)) {
		stk->select_layer(0);
		find_widget<button>(get_window(), "details_toggle", false).set_label(_("Addon Details"));
	}

	addon_list& addons = find_widget<addon_list>(get_window(), "addons", false);
	const addon_info* addon = addons.get_selected_addon();

	if(addon == nullptr) {
		return;
	}

	try {
		(this->*fptr)(*addon);
	} catch(const addons_client::user_exit&) {
		// User canceled the op.
	}
}

void addon_manager::install_addon(const addon_info& addon)
{
	addon_info versioned_addon = addon;
	widget* parent = get_window();
	if(stacked_widget* stk = find_widget<stacked_widget>(get_window(), "main_stack", false, false)) {
		parent = stk->get_layer_grid(1);
	}
	if(addon.id == find_widget<addon_list>(get_window(), "addons", false).get_selected_addon()->id) {
		versioned_addon.current_version = find_widget<menu_button>(parent, "version_filter", false).get_value_string();
	}

	addons_client::install_result result = client_.install_addon_with_checks(addons_, versioned_addon);

	// Take note if any wml_changes occurred
	need_wml_cache_refresh_ |= result.wml_changed;

	if(result.outcome != addons_client::install_outcome::abort) {
		reload_list_and_reselect_item(addon.id);
	}
}

void addon_manager::uninstall_addon(const addon_info& addon)
{
	if(have_addon_pbl_info(addon.id) || have_addon_in_vcs_tree(addon.id)) {
		show_error_message(
			_("The following add-on appears to have publishing or version control information stored locally, and will not be removed:") + " " +
				addon.display_title_full());
		return;
	}

	bool success = remove_local_addon(addon.id);

	if(!success) {
		gui2::show_error_message(_("The following add-on could not be deleted properly:") + " " + addon.display_title_full());
	} else {
		need_wml_cache_refresh_ = true;

		reload_list_and_reselect_item(addon.id);
	}
}

void addon_manager::update_addon(const addon_info& addon)
{
	/* Currently, the install and update codepaths are the same, so this function simply
	 * calls the other. Since this might change in the future, I'm leaving this function
	 * here for now.
	 *
	 * - vultraz, 2017-03-12
	 */
	install_addon(addon);
}

void addon_manager::update_all_addons()
{
	for(const auto& a : addons_) {
		if(tracking_info_[a.first].state == ADDON_INSTALLED_UPGRADABLE) {
			addons_client::install_result result = client_.install_addon_with_checks(addons_, a.second);

			if(result.wml_changed) {
				// Updating an add-on may have resulted in its dependencies being updated
				// as well, so we need to reread version info blocks afterwards to make sure
				// we don't try to re-download newly updated add-ons.
				refresh_addon_version_info_cache();
			}

			// Take note if any wml_changes occurred
			need_wml_cache_refresh_ |= result.wml_changed;
		}
	}

	if(need_wml_cache_refresh_) {
		load_addon_list();
	}
}

/** Performs all backend and UI actions for publishing the specified add-on. */
void addon_manager::publish_addon(const addon_info& addon)
{
	std::string server_msg;

	const std::string addon_id = addon.id;
	config cfg = get_addon_pbl_info(addon_id);

	const version_info& version_to_publish = cfg["version"].str();

	if(version_to_publish <= tracking_info_[addon_id].remote_version) {
		const int res = gui2::show_message(_("Warning"),
			_("The remote version of this add-on is greater or equal to the version being uploaded. Do you really wish to continue?"),
			gui2::dialogs::message::yes_no_buttons);

		if(res != gui2::retval::OK) {
			return;
		}
	}

	// the passphrase isn't provided, prompt for it
	if(cfg["passphrase"].empty()) {
		if(!gui2::dialogs::addon_auth::execute(cfg)) {
			return;
		}
	}

	if(!::image::exists(cfg["icon"].str())) {
		gui2::show_error_message(_("Invalid icon path. Make sure the path points to a valid image."));
	} else if(!client_.request_distribution_terms(server_msg)) {
		gui2::show_error_message(
			_("The server responded with an error:") + "\n" + client_.get_last_server_error());
	} else if(gui2::dialogs::addon_license_prompt::execute(server_msg)) {
		if(!client_.upload_addon(addon_id, server_msg, cfg, tracking_info_[addon_id].state == ADDON_INSTALLED_LOCAL_ONLY)) {
			const std::string& msg = _("The add-on was rejected by the server:") +
			                         "\n\n" + client_.get_last_server_error();
			const std::string& extra_data = client_.get_last_server_error_data();
			if (!extra_data.empty()) {
				// TODO: Allow user to copy the extra data portion to clipboard
				//       or something, maybe display it in a dialog similar to
				//       the WML load errors report in a monospace font and
				//       stuff (having a scroll container is especially
				//       important since a long list can cause the dialog to
				//       overflow).
				gui2::show_error_message(msg + "\n\n" + extra_data, true);
			} else {
				gui2::show_error_message(msg, true);
			}
		} else {
			gui2::show_transient_message(_("Response"), server_msg);
			fetch_addons_list();
			reload_list_and_reselect_item(addon_id);
		}
	}
}

/** Performs all backend and UI actions for taking down the specified add-on. */
void addon_manager::delete_addon(const addon_info& addon)
{
	const std::string addon_id = addon.id;
	const std::string& text = VGETTEXT(
		"Deleting '$addon|' will permanently erase its download and upload counts on the add-ons server. Do you really wish to continue?",
		{{"addon", make_addon_title(addon_id)}} // FIXME: need the real title!
	);

	const int res = gui2::show_message(_("Confirm"), text, gui2::dialogs::message::yes_no_buttons);

	if(res != gui2::retval::OK) {
		return;
	}

	std::string server_msg;
	if(!client_.delete_remote_addon(addon_id, server_msg)) {
		gui2::show_error_message(_("The server responded with an error:") + "\n" + client_.get_last_server_error());
	} else {
		// FIXME: translation needed!
		gui2::show_transient_message(_("Response"), server_msg);
		fetch_addons_list();
		reload_list_and_reselect_item(addon_id);
	}
}

/** Called when the player double-clicks an add-on. */
void addon_manager::execute_default_action(const addon_info& addon)
{
	switch(tracking_info_[addon.id].state) {
		case ADDON_NONE:
			install_addon(addon);
			break;
		case ADDON_INSTALLED:
			if(!tracking_info_[addon.id].can_publish) {
				utils::string_map symbols{ { "addon", addon.display_title_full() } };
				int res = gui2::show_message(_("Uninstall add-on"),
					VGETTEXT("Do you want to uninstall '$addon|'?", symbols),
					gui2::dialogs::message::ok_cancel_buttons);
				if(res == gui2::retval::OK) {
					uninstall_addon(addon);
				}
			}
			break;
		case ADDON_INSTALLED_UPGRADABLE:
			update_addon(addon);
			break;
		case ADDON_INSTALLED_LOCAL_ONLY:
		case ADDON_INSTALLED_OUTDATED:
			publish_addon(addon);
			break;
		default:
			break;
	}
}

void addon_manager::show_help()
{
	help::show_help("installing_addons");
}

static std::string format_addon_time(std::time_t time)
{
	if(time) {
		std::ostringstream ss;

		const char* format = preferences::use_twelve_hour_clock_format()
			? "%Y-%m-%d %I:%M %p"
			: "%Y-%m-%d %H:%M";

		ss << std::put_time(std::localtime(&time), format);

		return ss.str();
	}

	return font::unicode_em_dash;
}

void addon_manager::on_addon_select()
{
	const addon_info* info = find_widget<addon_list>(get_window(), "addons", false).get_selected_addon();

	if(info == nullptr) {
		return;
	}

	widget* parent = get_window();
	if(stacked_widget* stk = find_widget<stacked_widget>(get_window(), "main_stack", false, false)) {
		parent = stk->get_layer_grid(1);
	}

	find_widget<drawing>(parent, "image", false).set_label(info->display_icon());

	find_widget<styled_widget>(parent, "title", false).set_label(info->display_title_translated_or_original());
	find_widget<styled_widget>(parent, "description", false).set_label(info->description_translated());
	menu_button& version_filter = find_widget<menu_button>(parent, "version_filter", false);
	find_widget<styled_widget>(parent, "author", false).set_label(info->author);
	find_widget<styled_widget>(parent, "type", false).set_label(info->display_type());

	styled_widget& status = find_widget<styled_widget>(parent, "status", false);
	status.set_label(describe_status_verbose(tracking_info_[info->id]));
	status.set_use_markup(true);

	find_widget<styled_widget>(parent, "size", false).set_label(size_display_string(info->size));
	find_widget<styled_widget>(parent, "downloads", false).set_label(std::to_string(info->downloads));
	find_widget<styled_widget>(parent, "created", false).set_label(format_addon_time(info->created));
	find_widget<styled_widget>(parent, "updated", false).set_label(format_addon_time(info->updated));

	// Although this is a user-visible string, use utils::join instead of format_conjunct_list
	// because "x, y, z" is clearer than "x, y and z" in this context.
	find_widget<styled_widget>(parent, "tags", false).set_label(utils::join(info->tags, ", "));

	find_widget<styled_widget>(parent, "dependencies", false).set_label(!info->depends.empty()
		? make_display_dependencies(info->id, addons_, tracking_info_)
		: _("addon_dependencies^None"));

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

	find_widget<styled_widget>(parent, "translations", false).set_label(!languages.empty() ? languages : _("translations^None"));

	const std::string& feedback_url = info->feedback_url;
	find_widget<label>(parent, "url", false).set_label(!feedback_url.empty() ? feedback_url : _("url^None"));

	bool installed = is_installed_addon_status(tracking_info_[info->id].state);
	bool updatable = tracking_info_[info->id].state == ADDON_INSTALLED_UPGRADABLE;

	stacked_widget& action_stack = find_widget<stacked_widget>(parent, "action_stack", false);
	// #TODO: Add tooltips with upload time and pack size
	std::vector<config> version_filter_entries;

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

		for(const auto& f : info->versions) {
			version_filter_entries.emplace_back("label", f.str());
		}
		version_filter.set_active(true);
	} else {
		action_stack.select_layer(1);

		// Always enable the publish button, but disable the delete button if not yet published.
		find_widget<button>(parent, "publish", false).set_active(true);
		find_widget<button>(parent, "delete", false).set_active(!info->local_only);

		// Show only the version to be published
		version_filter_entries.emplace_back("label", info->current_version.str());
		version_filter.set_active(false);
	}
	version_filter.set_values(version_filter_entries);
}

void addon_manager::on_selected_version_change()
{
	widget* parent = get_window();
	widget* parent_of_addons_list = parent;
	if(stacked_widget* stk = find_widget<stacked_widget>(get_window(), "main_stack", false, false)) {
		parent = stk->get_layer_grid(1);
		parent_of_addons_list = stk->get_layer_grid(0);
	}

	const addon_info* info = find_widget<addon_list>(parent_of_addons_list, "addons", false).get_selected_addon();

	if(info == nullptr) {
		return;
	}

	if(!tracking_info_[info->id].can_publish && is_installed_addon_status(tracking_info_[info->id].state)) {
		bool updatable = tracking_info_[info->id].installed_version
						 != find_widget<menu_button>(parent, "version_filter", false).get_value_string();
		stacked_widget& action_stack = find_widget<stacked_widget>(parent, "action_stack", false);
		action_stack.select_layer(0);

		stacked_widget& install_update_stack = find_widget<stacked_widget>(parent, "install_update_stack", false);
		install_update_stack.select_layer(1);
		find_widget<button>(parent, "update", false).set_active(updatable);
	}
}

bool addon_manager::exit_hook(window& window)
{
	if(window.get_retval() == addon_list::DEFAULT_ACTION_RETVAL) {
		execute_default_action_on_selected_addon();
		return false;
	}

	return true;
}

} // namespace dialogs
