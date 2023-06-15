/*
	Copyright (C) 2010 - 2023
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

#include "gui/dialogs/editor/edit_pbl.hpp"

#include "editor/editor_common.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/multimenu_button.hpp"
#include "gui/dialogs/editor/edit_pbl_translation.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"

namespace gui2::dialogs
{

REGISTER_DIALOG(editor_edit_pbl)

const std::array type_values = {
	"",
	"core",
	"campaign",
	"campaign_sp_mp",
	"campaign_mp",
	"scenario",
	"scenario_mp",
	"faction",
	"era",
	"map_pack",
	"mod_mp",
	"media",
	"other",
};

const std::array tag_values = {
	"cooperative",
	"cosmetic",
	"difficulty",
	"rng",
	"survival",
	"terraforming",
};

editor_edit_pbl::editor_edit_pbl(const std::string& pbl)
    : modal_dialog(window_id())
	, pbl_(pbl)
{
	connect_signal_mouse_left_click(find_widget<toggle_button>(get_window(), "forum_auth", false), std::bind(&editor_edit_pbl::toggle_auth, this));
	connect_signal_mouse_left_click(find_widget<button>(get_window(), "translations_add", false), std::bind(&editor_edit_pbl::add_translation, this));
	connect_signal_mouse_left_click(find_widget<button>(get_window(), "translations_delete", false), std::bind(&editor_edit_pbl::delete_translation, this));
}

void editor_edit_pbl::pre_show(window& win)
{
	config pbl;
	if(filesystem::file_exists(pbl_)) {
		try {
			// preprocessing needed to strip out comments added by the editor
			read(pbl, *preprocess_file(pbl_));
		} catch(const config::error& e) {
			ERR_ED << "Caught a config error while parsing file " << pbl_ << "\n" << e.message;
		}
	}

	text_box* name = find_widget<text_box>(&win, "name", false, true);
	name->set_value(pbl["title"]);
	win.keyboard_capture(name);

	find_widget<text_box>(&win, "description", false).set_value(pbl["description"]);
	find_widget<text_box>(&win, "icon", false).set_value(pbl["icon"]);
	find_widget<text_box>(&win, "author", false).set_value(pbl["author"]);
	find_widget<text_box>(&win, "version", false).set_value(pbl["version"]);
	find_widget<text_box>(&win, "dependencies", false).set_value(pbl["dependencies"]);

	if(pbl["forum_auth"].to_bool()) {
		find_widget<toggle_button>(&win, "forum_auth", false).set_value(true);
		find_widget<text_box>(&win, "email", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(&win, "email_label", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>(&win, "password", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(&win, "password_label", false).set_visible(gui2::widget::visibility::invisible);
	} else {
		find_widget<text_box>(&win, "email", false).set_value(pbl["email"]);
		find_widget<text_box>(&win, "password", false).set_value(pbl["passphrase"]);
	}

	if(pbl.has_child("feedback")) {
		find_widget<text_box>(&win, "forum_thread", false).set_value(pbl.mandatory_child("feedback")["topic_id"]);
	}

	unsigned selected = 0;
	for(unsigned i = 0; i < type_values.size(); i++) {
		if(type_values[i] == pbl["type"]) {
			selected = i;
			break;
		}
	}

	menu_button& types = find_widget<menu_button>(&win, "type", false);
	std::vector<config> type_list;
	type_list.emplace_back("label", "");
	type_list.emplace_back("label", _("Core"));
	type_list.emplace_back("label", _("Campaign"));
	type_list.emplace_back("label", _("Hybrid Campaign"));
	type_list.emplace_back("label", _("Multiplayer Campaign"));
	type_list.emplace_back("label", _("Scenario"));
	type_list.emplace_back("label", _("Multiplayer Scenario"));
	type_list.emplace_back("label", _("Faction"));
	type_list.emplace_back("label", _("Era"));
	type_list.emplace_back("label", _("Map Pack"));
	type_list.emplace_back("label", _("Modification"));
	type_list.emplace_back("label", _("Media"));
	type_list.emplace_back("label", _("Other"));
	types.set_values(type_list);
	types.set_selected(selected);

	multimenu_button& tags = find_widget<multimenu_button>(&win, "tags", false);
	std::vector<config> tags_list;
	tags_list.emplace_back("label", _("Cooperative"), "checkbox", false);
	tags_list.emplace_back("label", _("Cosmetic"), "checkbox", false);
	tags_list.emplace_back("label", _("Difficulty"), "checkbox", false);
	tags_list.emplace_back("label", _("RNG"), "checkbox", false);
	tags_list.emplace_back("label", _("Survival"), "checkbox", false);
	tags_list.emplace_back("label", _("Terraforming"), "checkbox", false);
	tags.set_values(tags_list);

	std::vector<std::string> chosen_tags = utils::split(pbl["tags"].str(), ',');
	for(unsigned i = 0; i < tag_values.size(); i++) {
		if(std::find(chosen_tags.begin(), chosen_tags.end(), tag_values[i]) != chosen_tags.end()) {
			tags.select_option(i);
		}
	}

	listbox& translations = find_widget<listbox>(&win, "translations", false);
	button& translations_delete = find_widget<button>(&win, "translations_delete", false);

	for(const config& child : pbl.child_range("translation")) {
		const widget_data& entry{
			{ "translations_language",    widget_item{{"label", child["language"].str()}} },
			{ "translations_title",       widget_item{{"label", child["title"].str()}} },
			{ "translations_description", widget_item{{"label", child["description"].str()}} },
		};
		translations.add_row(entry);
	}

	if(translations.get_item_count() == 0) {
		translations_delete.set_active(false);
	}
}

void editor_edit_pbl::post_show(window& win)
{
	if(get_retval() != retval::OK) {
		return;
	}

	config cfg;
	filesystem::scoped_ostream stream = filesystem::ostream_file(pbl_);

	if(const std::string& name = find_widget<text_box>(&win, "name", false).get_value(); !name.empty()) {
		cfg["title"] = name;
	}
	if(const std::string& description = find_widget<text_box>(&win, "description", false).get_value(); !description.empty()) {
		cfg["description"] = description;
	}
	if(const std::string& icon = find_widget<text_box>(&win, "icon", false).get_value(); !icon.empty()) {
		cfg["icon"] = icon;
	}
	if(const std::string& author = find_widget<text_box>(&win, "author", false).get_value(); !author.empty()) {
		cfg["author"] = author;
	}
	if(const std::string& version = find_widget<text_box>(&win, "version", false).get_value(); !version.empty()) {
		cfg["version"] = version;
	}
	if(const std::string& dependencies = find_widget<text_box>(&win, "dependencies", false).get_value(); !dependencies.empty()) {
		cfg["dependencies"] = dependencies;
	}

	if(find_widget<toggle_button>(&win, "forum_auth", false).get_value_bool()) {
		cfg["forum_auth"] = true;
	} else {
		if(const std::string& email = find_widget<text_box>(&win, "email", false).get_value(); !email.empty()) {
			cfg["email"] = email;
		}
		if(const std::string& passphrase = find_widget<text_box>(&win, "password", false).get_value(); !passphrase.empty()) {
			cfg["passphrase"] = passphrase;
		}
	}

	if(const std::string& topic_id = find_widget<text_box>(&win, "forum_thread", false).get_value(); !topic_id.empty()) {
		config& feedback = cfg.add_child("feedback");
		feedback["topic_id"] = topic_id;
	}

	if(unsigned value = find_widget<menu_button>(&win, "type", false).get_value(); value != 0) {
		cfg["type"] = type_values[value];
	}

	multimenu_button& tags = find_widget<multimenu_button>(&win, "tags", false);
	boost::dynamic_bitset<> states = tags.get_toggle_states();
	std::vector<std::string> chosen;
	for(unsigned i = 0; i < states.size(); i++) {
		if(states[i] == 1) {
			chosen.emplace_back(tag_values[i]);
		}
	}
	if(chosen.size() > 0) {
		cfg["tags"] = utils::join(chosen, ",");
	}

	listbox& translations = find_widget<listbox>(&win, "translations", false);
	for(unsigned i = 0; i < translations.get_item_count(); i++) {
		grid* row = translations.get_row_grid(i);
		config& translation = cfg.add_child("translation");

		translation["language"] = dynamic_cast<label*>(row->find("translations_language", false))->get_label();
		translation["title"] = dynamic_cast<label*>(row->find("translations_title", false))->get_label();
		translation["description"] = dynamic_cast<label*>(row->find("translations_description", false))->get_label();
	}

	*stream << cfg;
}

void editor_edit_pbl::toggle_auth()
{
	toggle_button& forum_auth = find_widget<toggle_button>(get_window(), "forum_auth", false);
	if(forum_auth.get_value_bool()) {
		find_widget<text_box>(get_window(), "email", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>(get_window(), "password", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(get_window(), "email_label", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(get_window(), "password_label", false).set_visible(gui2::widget::visibility::invisible);
	} else {
		find_widget<text_box>(get_window(), "email", false).set_visible(gui2::widget::visibility::visible);
		find_widget<text_box>(get_window(), "password", false).set_visible(gui2::widget::visibility::visible);
		find_widget<label>(get_window(), "email_label", false).set_visible(gui2::widget::visibility::visible);
		find_widget<label>(get_window(), "password_label", false).set_visible(gui2::widget::visibility::visible);
	}
}

void editor_edit_pbl::add_translation()
{
	std::string language;
	std::string title;
	std::string description;
	editor_edit_pbl_translation::execute(language, title, description);

	if(!language.empty() && !title.empty()) {
		listbox& translations = find_widget<listbox>(get_window(), "translations", false);
		const widget_data& entry{
			{ "translations_language",    widget_item{{"label", language}} },
			{ "translations_title",       widget_item{{"label", title}} },
			{ "translations_description", widget_item{{"label", description}} },
		};
		translations.add_row(entry);
		find_widget<button>(get_window(), "translations_delete", false).set_active(true);
	}
}

void editor_edit_pbl::delete_translation()
{
	listbox& translations = find_widget<listbox>(get_window(), "translations", false);
	translations.remove_row(translations.get_selected_row());

	button& translations_delete = find_widget<button>(get_window(), "translations_delete", false);
	if(translations.get_item_count() == 0) {
		translations_delete.set_active(false);
	}
}

} // namespace dialogs
