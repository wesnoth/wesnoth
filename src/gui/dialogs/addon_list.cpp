/*
   Copyright (C) 2008 - 2015 by Mark de Wever <koraq@xs4all.nl>
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

#include "gettext.hpp"
#include "gui/auxiliary/filter.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
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

#include <boost/bind.hpp>

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


void taddon_list::expand(tgrid& grid, twidget& w)
{
	tselectable_& selectable = dynamic_cast<tselectable_&>(w);
	if(selectable.get_value_bool())
	{
		find_widget<tlabel>(&grid, "description", false)
			.set_visible(twidget::tvisible::visible);
	}
	else
	{
		find_widget<tlabel>(&grid, "description", false)
			.set_visible(twidget::tvisible::invisible);
	}
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

		/**
		 * @todo do we really want to keep the length limit for the various
		 * items?
		 */
		FOREACH(const AUTO & c, cfg_.child_range("campaign"))
		{
			std::map<std::string, string_map> data;
			string_map item;

			item["label"] = c["icon"];
			data.insert(std::make_pair("icon", item));

			utf8::string tmp = c["name"];
			item["label"] = utf8::truncate(tmp, 20);
			data.insert(std::make_pair("name", item));

			tmp = c["version"].str();
			item["label"] = utf8::truncate(tmp, 12);
			data.insert(std::make_pair("version", item));

			tmp = c["author"].str();
			item["label"] = utf8::truncate(tmp, 16);
			data.insert(std::make_pair("author", item));

			item["label"] = c["downloads"];
			data.insert(std::make_pair("downloads", item));

			item["label"] = c["size"];
			data.insert(std::make_pair("size", item));

			item["label"] = c["description"];
			data.insert(std::make_pair("description", item));

			list.add_row(data);
			unsigned index = list.get_item_count() - 1;
			ttoggle_button& button = find_widget<ttoggle_button>(list.get_row_grid(list.get_item_count() - 1), "expand", true);
			tgrid& lower_grid = find_widget<tgrid>(list.get_row_grid(list.get_item_count() - 1), "description_grid", false);
			lower_grid.set_visible(twidget::tvisible::invisible);
			button.set_callback_state_change(boost::bind(show_desc_impl, index, boost::ref(list), boost::ref(lower_grid), _1));
		}
		register_sort_button_alphabetical(window, "sort_name", "name");
		register_sort_button_alphabetical(window, "sort_author", "author");
		register_sort_button_numeric(window, "sort_downloads", "downloads");
		register_sort_button_numeric(window, "sort_size", "size");

		ttext_box& filter_box
				= find_widget<ttext_box>(&window, "filter", false);

		filter_box.set_text_changed_callback(
			boost::bind(&taddon_list::on_filtertext_changed, this, _1, _2));
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

	item["label"] = campaign["downloads"];
	data.insert(std::make_pair("downloads", item));

	item["label"] = utils::si_string(campaign["size"], true, _("unit_byte^B"));

	data.insert(std::make_pair("size", item));

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

	ttoggle_button* expand
			= find_widget<ttoggle_button>(grid, "expand", false, false);

	if(expand) {
		expand->set_callback_state_change(
				boost::bind(&taddon_list::expand, this, boost::ref(*grid), _1));
		find_widget<tlabel>(grid, "description", false)
				.set_visible(twidget::tvisible::invisible);
	}
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
