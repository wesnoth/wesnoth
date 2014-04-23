/*
   Copyright (C) 2012 - 2014 by Boldizsár Lipka <lipkab@zoho.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include "mp_options.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/window_builder.hpp"
#include "gui/dialogs/transient_message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/slider.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "widgets/slider.hpp"
#include "widgets/textbox.hpp"
#include <boost/foreach.hpp>
#include <boost/bind.hpp>

static lg::log_domain log_mp_create_options("mp/create/options");
#define DBG_MP LOG_STREAM(debug, log_mp_create_options)

namespace mp
{

namespace options
{

config to_event(const config& cfg)
{
	config result;

	if (!cfg) {
		return result;
	}

	result["name"] = "prestart";

	BOOST_FOREACH (const config& c, cfg.child_range("option")) {
		config ev;
		ev["name"] = c["id"];
		ev["value"] = c["value"];
		result.add_child("set_variable", ev);
	}

	return result;
}

void manager::init_info(const config& cfg, const std::string& key)
{
	BOOST_FOREACH (const config& comp, cfg.child_range(key)) {
		config entry;
		entry["id"] = comp["id"];
		entry["name"] = comp["name"];

		if (comp.has_child("options") && comp["allow_new_game"].to_bool(true)) {
			const config& options = comp.child("options");

			BOOST_FOREACH (const config::any_child& c,
								options.all_children_range()) {
				entry.add_child(c.key, c.cfg);
			}

		}

		// We need to store components even if they don't have any options in
		// order to have set_xxx_by_index work properly
		options_info_.add_child(key, entry);
	}
}

void manager::init_widgets()
{
	BOOST_FOREACH(option_display* od, widgets_ordered_) {
		delete od;
	}

	widgets_.clear();
	widgets_ordered_.clear();

	BOOST_FOREACH (const config::any_child& comp, options_info_.all_children_range()) {
		if (comp.cfg.all_children_count() == 0 || !is_active(comp.cfg["id"])) {
			continue;
		}

		widgets_ordered_.push_back(new title_display(display_.video(), comp.cfg["name"]));
		BOOST_FOREACH (const config::any_child& c, comp.cfg.all_children_range()) {
			const std::string id = c.cfg["id"];
			if (c.key == "slider") {
				widgets_ordered_.push_back(new slider_display(display_.video(), c.cfg));
			} else if (c.key == "entry") {
				widgets_ordered_.push_back(new entry_display(display_.video(), c.cfg));
			} else if (c.key == "checkbox") {
				widgets_ordered_.push_back(new checkbox_display(display_.video(), c.cfg));
			} else if (c.key == "combo") {
				widgets_ordered_.push_back(new combo_display(display_, c.cfg));
			}
			widgets_ordered_.back()->set_value(get_stored_value(id));
			widgets_[id] = widgets_ordered_.back();
		}
	}
}

manager::manager(const config &gamecfg, game_display &display, gui::scrollpane *pane, const config &values)
	: options_info_()
	, values_(values)
	, display_(display)
	, pane_(pane)
	, era_()
	, scenario_()
	, modifications_()
	, widgets_()
	, widgets_ordered_()
{
	DBG_MP << "Initializing the options manager" << std::endl;
	init_info(gamecfg, "modification");
	init_info(gamecfg, "era");
	init_info(gamecfg, "multiplayer");

	BOOST_FOREACH (const config::any_child& i,
				   options_info_.all_children_range())
	{
		BOOST_FOREACH (const config::any_child& j, i.cfg.all_children_range())
		{
			if (is_valid_option(j.key, j.cfg)) {
				config& value = get_value_cfg(j.cfg["id"]);
				value["value"] = get_stored_value(j.cfg["id"]);
			}
		}
	}

	init_widgets();
}

manager::~manager()
{
	BOOST_FOREACH(option_display* od, widgets_ordered_)
	{
		delete od;
	}
}


void manager::set_values(const config& c)
{
	values_ = c;
}

void manager::set_era(const std::string& era)
{
	era_ = era;
}

void manager::set_era_by_index(int index)
{
	era_ = options_info_.child("era", index)["id"].str();
}

void manager::set_scenario(const std::string& scenario)
{
	scenario_ = scenario;
}

void manager::set_scenario_by_index(int index)
{
	scenario_ = options_info_.child("multiplayer", index - 1)["id"].str();
}

void manager::set_modifications(const std::vector<std::string>& modifications)
{
	modifications_ = modifications;
}

void manager::insert_element(elem_type type, const config& data, int pos)
{
	switch (type)
	{
	case SCENARIO:
		options_info_.add_child_at("multiplayer", data, pos);
		break;
	case ERA:
		options_info_.add_child_at("era", data, pos);
		break;
	case MODIFICATION:
		options_info_.add_child_at("modification", data, pos);
		break;
	}
}

void manager::layout_widgets(int startx, int starty)
{
	int ypos = starty;
	int border_size = 3;
	BOOST_FOREACH(option_display* od, widgets_ordered_)
	{
		od->layout(startx, ypos, border_size, pane_);
		ypos += border_size;
	}
}

void manager::process_event()
{
	for (std::map<std::string, option_display*>::iterator i = widgets_.begin();
		 i != widgets_.end(); ++i)
	{
		i->second->process_event();
	}
}

void manager::hide_children(bool hide)
{
	for (std::map<std::string, option_display*>::iterator i = widgets_.begin();
		 i != widgets_.end(); ++i)
	{
		i->second->hide_children(hide);
	}
}


const config &manager::get_values()
{
	update_values();
	return values_;
}

config& manager::get_value_cfg(const std::string& id)
{
	{
		const config& value_cfg = get_value_cfg_or_empty(id);
		if (!value_cfg.empty()) {
			return const_cast<config&>(value_cfg);
		}
	}

	config::any_child info = get_option_parent(id);
	config* parent_cfg;
	if (!values_.find_child(info.key, "id", info.cfg["id"])) {
		parent_cfg = &values_.add_child(info.key);
		(*parent_cfg)["id"] = info.cfg["id"];
	} else {
		parent_cfg = &values_.find_child(info.key, "id", info.cfg["id"]);
	}

	config& value_cfg = parent_cfg->add_child("option");
	value_cfg["id"] = id;

	return value_cfg;
}

const config& manager::get_value_cfg_or_empty(const std::string& id) const
{
	static const config empty;

	BOOST_FOREACH (const config::any_child& i, values_.all_children_range()) {
		BOOST_FOREACH (const config& j, i.cfg.child_range("option")) {
			if (j["id"] == id) {
				return j;
			}
		}
	}

	return empty;
}

config::any_child manager::get_option_parent(const std::string& id) const
{
	static const config empty;
	static const std::string empty_key = "";
	static config::any_child not_found(&empty_key, &empty);

	BOOST_FOREACH (const config::any_child& i,
				   options_info_.all_children_range()) {
		BOOST_FOREACH (const config::any_child& j, i.cfg.all_children_range()) {
			if (j.cfg["id"] == id) {
				return i;
			}
		}
	}

	return not_found;
}

const config& manager::get_option_info_cfg(const std::string& id) const
{
	static const config empty;

	BOOST_FOREACH (const config::any_child& i,
				   options_info_.all_children_range()) {
		BOOST_FOREACH (const config::any_child& j, i.cfg.all_children_range()) {
			if (j.cfg["id"] == id) {
				return j.cfg;
			}
		}
	}

	return empty;
}


config::attribute_value manager::get_stored_value(const std::string& id) const
{
	const config& valcfg = get_value_cfg_or_empty(id);

	if (!valcfg["value"].empty()) {
		// There's a saved value for this option
		return valcfg["value"];
	}

	// Fall back to the option's default
	return get_default_value(id);
}

config::attribute_value manager::get_default_value(const std::string& id) const
{
	const config& optinfo = get_option_info_cfg(id);

	return optinfo["default"];
}

void manager::extract_values(const std::string& key, const std::string& id)
{
	BOOST_FOREACH (const config::any_child& c,
				   options_info_.find_child(key, "id", id).all_children_range())
	{
		if (!is_valid_option(c.key, c.cfg)) {
			continue;
		}

		config& out = get_value_cfg(c.cfg["id"].str());
		out["value"] = widgets_[c.cfg["id"]]->get_value();
	}
}

void manager::update_values()
{
	extract_values("era", era_);
	extract_values("multiplayer", scenario_);
	BOOST_FOREACH(const std::string& str, modifications_) {
		extract_values("modification", str);
	}
}

bool manager::is_valid_option(const std::string& key, const config& option)
{
	return (key == "slider" || key == "entry" || key == "checkbox" || key == "combo") &&
		   (!option["id"].empty());
}

void manager::restore_defaults(manager* m)
{
	const config& era = m->options_info_.find_child("era", "id", m->era_);
	restore_defaults_for_component(era, m);

	const config& scen = m->options_info_.find_child("multiplayer", "id",
													 m->scenario_);
	restore_defaults_for_component(scen, m);

	BOOST_FOREACH (const std::string& id, m->modifications_) {
		const config& mod = m->options_info_.find_child("modification", "id",
														id);
		restore_defaults_for_component(mod, m);
	}
}

void manager::restore_defaults_for_component(const config& c, manager* m)
{
	BOOST_FOREACH (const config::any_child& i, c.all_children_range()) {
		if (!is_valid_option(i.key, i.cfg)) {
			continue;
		}

		const std::string id = i.cfg["id"].str();

		m->widgets_[id]->set_value(m->get_default_value(id));
	}
}

bool manager::is_active(const std::string &id) const
{
	return (era_ == id) || (scenario_ == id) ||
			(std::find(modifications_.begin(), modifications_.end(), id) != modifications_.end());
}

entry_display::entry_display(CVideo &video, const config &cfg) :
	entry_(new gui::textbox(video, 150, cfg["default"])),
	label_(new gui::label(video, cfg["name"]))
{
	entry_->set_help_string(cfg["description"]);
}

entry_display::~entry_display()
{
	delete entry_;
	delete label_;
}

void entry_display::layout(int &xpos, int &ypos, int border_size, gui::scrollpane *pane)
{
	pane->add_widget(label_, xpos, ypos);
	pane->add_widget(entry_, xpos + label_->width() + border_size, ypos);
	ypos += std::max(label_->height(), entry_->height()) + border_size;
}

void entry_display::set_value(const config::attribute_value &val)
{
	entry_->set_text(val);
}

config::attribute_value entry_display::get_value() const
{
	config::attribute_value res;
	res = entry_->text();
	return res;
}

void entry_display::hide_children(bool hide)
{
	label_->hide(hide);
	entry_->hide(hide);
}

slider_display::slider_display(CVideo &video, const config &cfg) :
	slider_(new gui::slider(video)),
	label_(new gui::label(video, cfg["name"], font::SIZE_SMALL)),
	last_value_(cfg["default"].to_int()),
	label_text_(cfg["name"])
{
	slider_->set_min(cfg["min"].to_int());
	slider_->set_max(cfg["max"].to_int());
	slider_->set_increment(cfg["step"].to_int());
	slider_->set_width(150);
	slider_->set_value(cfg["default"].to_int());

	slider_->set_help_string(cfg["description"]);

	update_label();
}

slider_display::~slider_display()
{
	delete slider_;
	delete label_;
}

void slider_display::layout(int &xpos, int &ypos, int border_size, gui::scrollpane *pane)
{
	pane->add_widget(label_, xpos, ypos);
	ypos += label_->height() + border_size;
	pane->add_widget(slider_, xpos, ypos);
	ypos += slider_->height() + border_size;
}

void slider_display::set_value(const config::attribute_value &val)
{
	slider_->set_value(val.to_int());
}

config::attribute_value slider_display::get_value() const
{
	config::attribute_value res;
	res = slider_->value();
	return res;
}

void slider_display::process_event()
{
	if (slider_->value() != last_value_) {
		last_value_ = slider_->value();
		update_label();
	}
}

void slider_display::hide_children(bool hide)
{
	label_->hide(hide);
	slider_->hide(hide);
}

void slider_display::update_label()
{
	std::stringstream ss;
	ss << label_text_ << ' ' << last_value_;
	label_->set_text(ss.str());
}

checkbox_display::checkbox_display(CVideo &video, const config &cfg) :
	checkbox_(new gui::button(video, cfg["name"], gui::button::TYPE_CHECK))
{
	checkbox_->set_check(cfg["default"].to_bool());
	checkbox_->set_help_string(cfg["description"]);
}

checkbox_display::~checkbox_display()
{
	delete checkbox_;
}

void checkbox_display::layout(int &xpos, int &ypos, int border_size, gui::scrollpane *pane)
{
	pane->add_widget(checkbox_, xpos, ypos);
	ypos += checkbox_->height() + border_size;
}

void checkbox_display::set_value(const config::attribute_value &val)
{
	checkbox_->set_check(val.to_bool());
}

config::attribute_value checkbox_display::get_value() const
{
	config::attribute_value res;
	res = checkbox_->checked();
	return res;
}

void checkbox_display::hide_children(bool hide)
{
	checkbox_->hide(hide);
}

title_display::title_display(CVideo &video, const std::string &label) :
	title_(new gui::label(video, "`~" + label, font::SIZE_PLUS, font::LOBBY_COLOR))
{}

title_display::~title_display()
{
	delete title_;
}

void title_display::layout(int &xpos, int &ypos, int border_size, gui::scrollpane *pane)
{
	ypos += 4*border_size;
	pane->add_widget(title_, xpos, ypos);
	ypos += title_->height() + 2*border_size;
}

void title_display::hide_children(bool hide)
{
	title_->hide(hide);
}

combo_display::combo_display(game_display &display, const config &cfg) :
	label_(new gui::label(display.video(), cfg["name"])),
	combo_(new gui::combo(display, std::vector<std::string>())),
	values_()
{
	std::vector<std::string> items;
	BOOST_FOREACH(const config& item, cfg.child_range("item")) {
		items.push_back(item["name"]);
		values_.push_back(item["value"]);
	}

	combo_->set_items(items);
	combo_->set_help_string(cfg["description"]);
	set_value(cfg["default"]);
}

combo_display::~combo_display()
{
	delete label_;
	delete combo_;
}

void combo_display::layout(int &xpos, int &ypos, int border_size, gui::scrollpane *pane)
{
	pane->add_widget(label_, xpos, ypos);
	pane->add_widget(combo_, xpos + label_->width() + border_size, ypos);
	ypos += std::max(label_->height(), combo_->height()) + border_size;
}

void combo_display::set_value(const config::attribute_value &val)
{
	const std::string value = val;
	for (size_t i = 0; i<values_.size(); i++) {
		if (value == values_[i]) {
			combo_->set_selected(i);
			break;
		}
	}
}

config::attribute_value combo_display::get_value() const
{
	config::attribute_value res;
	res = values_[combo_->selected()];
	return res;
}

void combo_display::hide_children(bool hide)
{
	label_->hide(hide);
	combo_->hide(hide);
}

}	// namespace options

}	// namespace mp

