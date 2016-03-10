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

#include "gui/dialogs/addon_list.hpp"

#include "addon/info.hpp"
#include "addon/state.hpp"

#include "desktop/clipboard.hpp"
#include "desktop/open.hpp"

#include "gettext.hpp"
#include "gui/auxiliary/filter.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/helper.hpp"
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
#include "utils/foreach.tpp"
#include "serialization/string_utils.hpp"
#include "formula_string_utils.hpp"
#include "marked-up_text.hpp"
#include "font.hpp"
#include "preferences.hpp"
#include "strftime.hpp"

#include "config.hpp"

#include <boost/bind.hpp>
#include <sstream>

namespace gui2
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
 * -name & & control & o &
 *        The name of the addon. $
 *
 * -version & & control & o &
 *        The version number of the addon. $
 *
 * -author & & control & o &
 *        The author of the addon. $
 *
 * -downloads & & control & o &
 *        The number of times the addon has been downloaded. $
 *
 * -size & & control & o &
 *        The size of the addon. $
 *
 * @end{table}
 */

REGISTER_DIALOG(addon_list)

struct filter_transform
{
	filter_transform(const std::vector<std::string>& filtertext) : filtertext_(filtertext) {}
	bool operator()(const config& cfg) const
	{
			FOREACH(const AUTO& filter, filtertext_)
			{
				bool found = false;
				FOREACH(const AUTO& attribute, cfg.attribute_range())
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
				if(!found)
				{
					return false;
				}
			}
			return true;
	}
	const std::vector<std::string> filtertext_;
};

taddon_list::taddon_list(const config& cfg)
	: orders_()
	, cfg_(cfg)
	, cfg_iterators_(cfg_.child_range("campaign"))
	, addons_()
	, tracking_info_()
	, ids_()
{
	read_addons_list(cfg, addons_);
}

void taddon_list::on_filtertext_changed(ttext_* textbox, const std::string& text)
{
	tlistbox& listbox = find_widget<tlistbox>(textbox->get_window(), "addons", true);
	filter_transform filter(utils::split(text, ' '));
	std::vector<bool> res;
	res.reserve(cfg_.child_count("campaign"));
	FOREACH(const AUTO& child, cfg_.child_range("campaign"))
	{
		res.push_back(filter(child));
	}
	listbox.set_row_shown(res);
}

void taddon_list::on_order_button_click(twindow& window, const tgenerator_::torder_func& up, const tgenerator_::torder_func& down, twidget& w)
{
	tselectable_& selectable = dynamic_cast<tselectable_&>(w);
	FOREACH(AUTO& other, orders_)
	{
		if(other != &selectable) {
			other->set_value(0);
		}
	}
	tlistbox& listbox = find_widget<tlistbox>(&window, "addons", true);
	switch(selectable.get_value())
	{
	case 0:
		listbox.order_by(std::less<unsigned>());
		break;
	case 1:
		listbox.order_by(up);
		break;
	case 2:
		listbox.order_by(down);
		break;
	}
}

void taddon_list::register_sort_button(twindow& window, const std::string& id, const tgenerator_::torder_func& up, const tgenerator_::torder_func& down)
{
	tselectable_& selectable = find_widget<tselectable_>(&window, id, true);
	orders_.push_back(&selectable);
	selectable.set_callback_state_change(boost::bind(&taddon_list::on_order_button_click, this, boost::ref(window), up, down, _1));
}
namespace {
	/// TODO: it would be better if we didnt have to parse the config every time.
	bool str_up(const config* cfg, const std::string& prop_id, unsigned i1, unsigned i2)
	{
		return cfg->child("campaign", i1)[prop_id].str() < cfg->child("campaign", i2)[prop_id].str();
	}
	bool str_down(const config* cfg, const std::string& prop_id, unsigned i1, unsigned i2)
	{
		return cfg->child("campaign", i1)[prop_id].str() > cfg->child("campaign", i2)[prop_id].str();
	}
	bool num_up(const config* cfg, const std::string& prop_id, unsigned i1, unsigned i2)
	{
		return cfg->child("campaign", i1)[prop_id].to_int() < cfg->child("campaign", i2)[prop_id].to_int();
	}
	bool num_down(const config* cfg, const std::string& prop_id, unsigned i1, unsigned i2)
	{
		return cfg->child("campaign", i1)[prop_id].to_int() > cfg->child("campaign", i2)[prop_id].to_int();
	}

	void show_desc_impl(unsigned index, tlistbox& list, tgrid &lower_grid,twidget& w)
	{
		// showing teh description ona  seperate multipage woudo be better specially on large screens.
		tselectable_& selectable = dynamic_cast<tselectable_ &>(w);
		list.select_row(index);
		lower_grid.set_visible(!selectable.get_value_bool() ? twidget::tvisible::invisible : twidget::tvisible::visible);
	}

}
void taddon_list::register_sort_button_alphabetical(twindow& window, const std::string& id, const std::string& prop_id)
{
	register_sort_button(window, id, boost::bind(&str_up, &cfg_, prop_id, _1, _2), boost::bind(&str_down, &cfg_, prop_id, _1, _2));
}

void taddon_list::register_sort_button_numeric(twindow& window, const std::string& id, const std::string& prop_id)
{

	register_sort_button(window, id, boost::bind(&num_up, &cfg_, prop_id, _1, _2), boost::bind(&num_down, &cfg_, prop_id, _1, _2));
}

namespace {
	bool default_sort(const tpane::titem& i1, const tpane::titem& i2)
	{
		return i1.id < i2.id;
	}

	template<typename T>
	void sort_callback(twidget& w, tpane* pane, const std::string& name_prop)
	{
		tselectable_& selectable = dynamic_cast<tselectable_&>(w);
		if(selectable.get_value() == 0) {
			pane->sort(&default_sort);
		}
		else {
			pane->sort(boost::bind(&sort<T>,  _1, _2, name_prop, (selectable.get_value() == 1)));
		}
	}
}

static inline const addon_info& addon_at(const std::string& id, const addons_list& addons)
{
	addons_list::const_iterator it = addons.find(id);
	assert(it != addons.end());
	return it->second;
}

static std::string describe_addon_status(const addon_tracking_info& info)
{
	std::string tc, tx;

	switch(info.state) {
		case ADDON_NONE:
			tc = "#a69275";
			tx = info.can_publish ? _("addon_state^Published, not installed") : _("addon_state^Not installed");
			break;

		case ADDON_INSTALLED:
		case ADDON_NOT_TRACKED:
			// Consider add-ons without version information as installed
			// for the main display. Their Description info should elaborate
			// on their status.
			tc = "#00ff00";
			tx = info.can_publish ? _("addon_state^Published") : _("addon_state^Installed");
			break;

		case ADDON_INSTALLED_UPGRADABLE:
			tc = "#ffff00";
			tx = info.can_publish ? _("addon_state^Published, upgradable") : _("addon_state^Installed, upgradable");
			break;

		case ADDON_INSTALLED_OUTDATED:
			tc = "#ff7f00";
			tx = info.can_publish ? _("addon_state^Published, outdated on server") : _("addon_state^Installed, outdated on server");
			break;

		case ADDON_INSTALLED_BROKEN:
			tc = "#ff0000";
			tx = info.can_publish ? _("addon_state^Published, broken") : _("addon_state^Installed, broken");
			break;

		default:
			tc = "#777777";
			tx = _("addon_state^Unknown");
	}

	return "<span color='" + tc + "'>" + tx + "</span>";
}

static std::string colorify_addon_state_string(const std::string& str,
										const addon_tracking_info& state)
{
	std::string colorname = "";

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

static std::string describe_addon_state_info(const addon_tracking_info& state)
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

void taddon_list::pre_show(CVideo& /*video*/, twindow& window)
{
	if(new_widgets) {

		/***** ***** Init buttons. ***** *****/

		tpane& pane = find_widget<tpane>(&window, "addons", false);

		find_widget<tselectable_>(&window, "sort_name", false).set_callback_state_change(boost::bind(&sort_callback<std::string>, _1, &pane, "name"));
		find_widget<tselectable_>(&window, "sort_size", false).set_callback_state_change(boost::bind(&sort_callback<std::string>, _1, &pane, "size"));


		/***** ***** Init the filter text box. ***** *****/

		ttext_box& filter_box
				= find_widget<ttext_box>(&window, "filter", false);

		tpane::tfilter_functor filter_functor
				= boost::bind(&contains, _1, "filter", boost::cref(filter_box));

		connect_signal_notify_modified(
				filter_box, boost::bind(&tpane::filter, &pane, filter_functor));

		/***** ***** Fill the listbox. ***** *****/

		tbutton* load_button
				= find_widget<tbutton>(&window, "load_campaign", false, false);
		if(load_button) {
			connect_signal_mouse_left_click(
					*load_button,
					boost::bind(&taddon_list::load, this, boost::ref(pane)));
			load(pane);
		} else {
			while(cfg_iterators_.first != cfg_iterators_.second) {
				create_campaign(pane, *cfg_iterators_.first);
				++cfg_iterators_.first;
			}
		}

	} else {
		tlistbox& list = find_widget<tlistbox>(&window, "addons", false);

		FOREACH(const AUTO & c, cfg_.child_range("campaign"))
		{
			ids_.push_back(c["name"]);
			const addon_info& info = addon_at(ids_.back(), addons_);
			tracking_info_[info.id] = get_addon_tracking_info(info);

			std::map<std::string, string_map> data;
			string_map item;

			item["label"] = info.display_icon();
			data.insert(std::make_pair("icon", item));

			item["label"] = info.display_title();
			data.insert(std::make_pair("name", item));

			item["label"] = describe_addon_status(tracking_info_[info.id]);
			item["use_markup"] = "true";
			data.insert(std::make_pair("installation_status", item));

			item["label"] = info.version.str();
			data.insert(std::make_pair("version", item));

			// utf8::truncate(tmp, 16)
			item["label"] = info.author;
			data.insert(std::make_pair("author", item));

			item["label"] = size_display_string(info.size);
			data.insert(std::make_pair("size", item));

			item["label"] = lexical_cast<std::string>(info.downloads);
			data.insert(std::make_pair("downloads", item));

			item["label"] = info.display_type();
			data.insert(std::make_pair("type", item));

			list.add_row(data);
		}
		register_sort_button_alphabetical(window, "sort_name", "name");
		register_sort_button_alphabetical(window, "sort_author", "author");
		register_sort_button_numeric(window, "sort_downloads", "downloads");
		register_sort_button_numeric(window, "sort_size", "size");

		ttext_box& filter_box
				= find_widget<ttext_box>(&window, "filter", false);

		filter_box.set_text_changed_callback(
			boost::bind(&taddon_list::on_filtertext_changed, this, _1, _2));

#ifdef GUI2_EXPERIMENTAL_LISTBOX
		connect_signal_notify_modified(list,
				boost::bind(&taddon_list::on_addon_select,
				*this,
				boost::ref(window)));
#else
		list.set_callback_value_change(
				dialog_callback<taddon_list, &taddon_list::on_addon_select>);
#endif

		tbutton& url_go_button = find_widget<tbutton>(&window, "url_go", false);
		tbutton& url_copy_button = find_widget<tbutton>(&window, "url_copy", false);
		ttext_box& url_textbox = find_widget<ttext_box>(&window, "url", false);

		url_textbox.set_active(false);

		if (!desktop::clipboard::available()) {
			url_copy_button.set_active(false);
			url_copy_button.set_tooltip(_("Clipboard support not found, contact your packager"));
		}

		if(!desktop::open_object_is_supported()) {
			// No point in displaying the button on platforms that can't do
			// open_object().
			url_go_button.set_visible(tcontrol::tvisible::invisible);
		}

		//connect_signal_mouse_left_click(
		//		url_go_button,
		//		boost::bind(&taddon_description::browse_url_callback, this));

		//connect_signal_mouse_left_click(
		//		url_copy_button,
		//		boost::bind(&taddon_description::copy_url_callback, this));

		on_addon_select(window);
	}
}

static std::string format_addon_time(time_t time)
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
static typename MapT::mapped_type const& const_at(typename MapT::key_type const& key,
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

void taddon_list::on_addon_select(twindow& window)
{
	const int index = find_widget<tlistbox>(&window, "addons", false).get_selected_row();

	const addon_info& info = addon_at(ids_[index], addons_);

	find_widget<tdrawing>(&window, "image", false).set_label(info.display_icon());

	find_widget<tcontrol>(&window, "title", false).set_label(info.display_title());
	find_widget<tcontrol>(&window, "description", false).set_label(info.description);
	find_widget<tcontrol>(&window, "version", false).set_label(info.version.str());
	find_widget<tcontrol>(&window, "author", false).set_label(info.author);
	find_widget<tcontrol>(&window, "type", false).set_label(info.display_type());

	tcontrol& status = find_widget<tcontrol>(&window, "status", false);
	status.set_label(describe_addon_state_info(tracking_info_[info.id]));
	status.set_use_markup(true);

	find_widget<tcontrol>(&window, "size", false).set_label(size_display_string(info.size));
	find_widget<tcontrol>(&window, "downloads", false).set_label(lexical_cast<std::string>(info.downloads));
	find_widget<tcontrol>(&window, "created", false).set_label(format_addon_time(info.created));
	find_widget<tcontrol>(&window, "updated", false).set_label(format_addon_time(info.updated));

	const std::string& feedback_url = info.feedback_url;

	if(!feedback_url.empty()) {
		find_widget<tstacked_widget>(&window, "feedback_stack", false).select_layer(1);
		find_widget<ttext_box>(&window, "url", false).set_value(feedback_url);
	} else {
		find_widget<tstacked_widget>(&window, "feedback_stack", false).select_layer(0);
	}
}

void taddon_list::create_campaign(tpane& pane, const config& campaign)
{
	/***** Determine the data for the widgets. *****/

	std::map<std::string, string_map> data;
	string_map item;

	item["label"] = campaign["icon"];
	data.insert(std::make_pair("icon", item));

	utf8::string tmp = campaign["name"];
	item["label"] = utf8::truncate(tmp, 20);
	data.insert(std::make_pair("name", item));

	tmp = campaign["version"].str();
	item["label"] = utf8::truncate(tmp, 12);
	data.insert(std::make_pair("version", item));

	tmp = campaign["author"].str();
	item["label"] = utf8::truncate(tmp, 16);
	data.insert(std::make_pair("author", item));

	item["label"] = utils::si_string(campaign["size"], true, _("unit_byte^B"));
	data.insert(std::make_pair("size", item));

	item["label"] = campaign["downloads"];
	data.insert(std::make_pair("downloads", item));

	item["label"] = campaign["description"];
	data.insert(std::make_pair("description", item));

	/***** Determine the tags for the campaign. *****/

	std::map<std::string, std::string> tags;
	tags.insert(std::make_pair("name", campaign["name"]));
	tags.insert(std::make_pair("size", campaign["size"]));

	std::stringstream filter;
	filter << campaign["version"] << '\n' << campaign["author"] << '\n'
		   << campaign["type"] << '\n' << campaign["description"];

	tags.insert(std::make_pair("filter", utf8::lowercase(filter.str())));

	/***** Add the campaign. *****/

	const unsigned id = pane.create_item(data, tags);

	tgrid* grid = pane.grid(id);
	assert(grid);
}

void taddon_list::load(tpane& pane)
{
	while(cfg_iterators_.first != cfg_iterators_.second) {
		create_campaign(pane, *cfg_iterators_.first);
		++cfg_iterators_.first;
	}

	find_widget<tbutton>(pane.get_window(), "load_campaign", false)
			.set_active(cfg_iterators_.first != cfg_iterators_.second);
}

} // namespace gui2
