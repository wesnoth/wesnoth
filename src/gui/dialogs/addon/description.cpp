/*
   Copyright (C) 2010 - 2014 by Ignacio R. Morelle <shadowm2006@gmail.com>
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

#include "gui/dialogs/addon/description.hpp"

#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"
#include "formula_string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "language.hpp"
#include "preferences.hpp"
#include "strftime.hpp"
#include "utils/foreach.tpp"

#include <boost/bind.hpp>

namespace
{
std::string format_addon_time(time_t time)
{
	if(time) {
		char buf[1024] = { 0 };
		struct std::tm* const t = std::localtime(&time);

		const char* format = preferences::use_twelve_hour_clock_format()
									 ? "%Y-%m-%d %I:%M %p"
									 : "%Y-%m-%d %H:%M";

		if(util::strftime(buf, sizeof(buf), format, t)) {
			return buf;
		}
	}

	return utils::unicode_em_dash;
}

std::string langcode_to_string(const std::string& lcode)
{
	FOREACH(const AUTO & ld, get_languages())
	{
		if(ld.localename == lcode || ld.localename.substr(0, 2) == lcode) {
			return ld.language;
		}
	}

	return "";
}

std::string colorify_addon_state_string(const std::string& str,
										const addon_tracking_info& state)
{
	std::string colorname = "";

	// NOTE: these Pango color names must match the colors used
	// in describe_addon_status() for GUI1.

	switch(state.state) {
		case ADDON_NONE:
			return str;
		case ADDON_INSTALLED:
		case ADDON_NOT_TRACKED:
			colorname = "#00ff00"; // GOOD_COLOR
			break;
		case ADDON_INSTALLED_UPGRADABLE:
			colorname = "#ffff00"; // YELLOW_COLOR/color_upgradable
			break;
		case ADDON_INSTALLED_OUTDATED:
			colorname = "#ff7f00"; // <255,127,0>/color_outdated
			break;
		case ADDON_INSTALLED_BROKEN:
			colorname = "#ff0000"; // BAD_COLOR
			break;
		default:
			colorname = "#777777"; // GRAY_COLOR
			break;
	}

	return "<span color='" + colorname + "'>" + str + "</span>";
}

std::string describe_addon_state_info(const addon_tracking_info& state)
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

	return colorify_addon_state_string(s, state);
}

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

std::string make_display_dependencies(const std::string& addon_id,
									  const addons_list& addons_list,
									  const addons_tracking_list& addon_states)
{
	const addon_info& addon = const_at(addon_id, addons_list);
	std::string str;

	const std::set<std::string>& deps = addon.resolve_dependencies(addons_list);

	FOREACH(const AUTO & dep_id, deps)
	{
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

		str += colorify_addon_state_string(dep.display_title(), depstate);
	}

	return str;
}
}

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_addon_description
 *
 * == Add-on description ==
 *
 * Add-on description and details for the add-ons manager interface.
 *
 * @begin{table}{dialog_widgets}
 *
 * image & & control & m &
 *         Label for displaying the add-on icon, in a 72x72 area. $
 *
 * title & & control & m &
 *         Dialog title label, corresponding to the add-on name. $
 *
 * type & & control & m &
 *         Label for displaying the add-on's type. $
 *
 * version & & control & m &
 *         Label for displaying the add-on version number. $
 *
 * status & & control & m &
 *         Label for displaying the current installation/upgradability status. $
 *
 * author & & control & m &
 *         Label for displaying the add-on author/maintainer name. $
 *
 * size & & control & m &
 *         Label for displaying the add-on package size. $
 *
 * downloads & & control & m &
 *         Label for displaying the add-on's download count. $
 *
 * description & & control & m &
 *         Text label for displaying the add-on's description. The control can
 *         be given a text, this text is shown when the addon has no
 *         description. If the addon has a description this field shows the
 *         description of the addon. $
 *
 * translations & & control & m &
 *         Label for displaying a list of translations provided by the add-on.
 *         Like the ''description'' it can also show a default text if no
 *         translations are available. $
 *
 * dependencies & & control & m &
 *         Label for displaying a list of dependencies of the add-on. Like the
 *         ''description'' it can also show a default text if no dependencies
 *         are defined. $
 *
 * updated & & control & m &
 *         Label displaying the add-on's last upload date. $
 *
 * created & & control & m &
 *         Label displaying the add-on's first upload date. $
 *
 * url & & text_box & m &
 *         Textbox displaying the add-on's feedback page URL if provided by
 *         the server. $
 *
 * url_go & & button & m &
 *         Button for launching a web browser to visit the add-on's feedback
 *         page URL if provided by the server. $
 *
 * url_copy & & button & m &
 *         Button for copying the add-on's feedback page URL to clipboard if
 *         provided by the server. $
 *
 * url_none & & control & m &
 *         Label displayed instead of the other url_* widgets when no URL is
 *         provided by the server.
 *
 * @end{table}
 */

REGISTER_DIALOG(addon_description)

taddon_description::taddon_description(const std::string& addon_id,
									   const addons_list& addons_list,
									   const addons_tracking_list& addon_states)
	: feedback_url_()
{
	const addon_info& addon = const_at(addon_id, addons_list);
	const addon_tracking_info& state = const_at(addon_id, addon_states);

	const std::string& created_text = format_addon_time(addon.created);
	const std::string& updated_text = format_addon_time(addon.updated);

	register_label("image", true, addon.display_icon());
	register_label("title", true, addon.title);
	register_label("version", true, addon.version);
	register_label("status", true, describe_addon_state_info(state), true);
	register_label("author", true, addon.author);
	register_label("type", true, addon.display_type());
	register_label("size", true, size_display_string(addon.size));
	register_label("downloads", true, str_cast(addon.downloads));
	register_label("created", true, created_text);
	register_label("updated", true, updated_text);
	if(!addon.description.empty()) {
		register_label("description", true, addon.description);
	}
	if(!addon.depends.empty()) {
		register_label(
				"dependencies",
				true,
				make_display_dependencies(addon_id, addons_list, addon_states),
				true);
	}

	feedback_url_ = addon.feedback_url;

	std::string languages;

	FOREACH(const AUTO & lc, addon.locales)
	{
		const std::string& langlabel = langcode_to_string(lc);
		if(!langlabel.empty()) {
			if(!languages.empty()) {
				languages += ", ";
			}
			languages += langlabel;
		}
	}

	if(!languages.empty()) {
		register_label("translations", true, languages);
	}
}

void taddon_description::browse_url_callback()
{
	/* TODO: ask for confirmation */

	desktop::open_object(feedback_url_);
}

void taddon_description::copy_url_callback()
{
	desktop::clipboard::copy_to_clipboard(feedback_url_, false);
}

void taddon_description::pre_show(CVideo& /*video*/, twindow& window)
{
	tcontrol& url_none = find_widget<tcontrol>(&window, "url_none", false);
	tbutton& url_go_button = find_widget<tbutton>(&window, "url_go", false);
	tbutton& url_copy_button = find_widget<tbutton>(&window, "url_copy", false);
	ttext_box& url_textbox = find_widget<ttext_box>(&window, "url", false);

	url_textbox.set_value(feedback_url_);
	url_textbox.set_active(false);

	if(!feedback_url_.empty()) {
		url_none.set_visible(tcontrol::tvisible::invisible);

		connect_signal_mouse_left_click(
				url_go_button,
				boost::bind(&taddon_description::browse_url_callback, this));

		connect_signal_mouse_left_click(
				url_copy_button,
				boost::bind(&taddon_description::copy_url_callback, this));

		if (!desktop::clipboard::available()) {
			url_copy_button.set_active(false);
			url_copy_button.set_tooltip(_("Clipboard support not found, contact your packager."));
		}
	} else {
		url_go_button.set_active(false);
		url_copy_button.set_active(false);

		url_go_button.set_visible(tcontrol::tvisible::invisible);
		url_copy_button.set_visible(tcontrol::tvisible::invisible);
		url_textbox.set_visible(tcontrol::tvisible::invisible);
	}

	if(!desktop::open_object_is_supported()) {
		// No point in displaying the button on platforms that can't do
		// open_object().
		url_go_button.set_visible(tcontrol::tvisible::invisible);
	}
}

} // namespace  gui2
