/*
	Copyright (C) 2023 - 2024
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
#include "gui/dialogs/editor/edit_pbl_translation.hpp"
#include "gui/dialogs/file_dialog.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/drawing.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/multimenu_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "serialization/base64.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/schema_validator.hpp"

#include <boost/algorithm/string/replace.hpp>

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

editor_edit_pbl::editor_edit_pbl(const std::string& pbl, const std::string& current_addon)
	: modal_dialog(window_id())
	, pbl_(pbl)
	, current_addon_(current_addon)
	, dirs_()
{
	connect_signal_mouse_left_click(
		find_widget<toggle_button>(get_window(), "forum_auth", false), std::bind(&editor_edit_pbl::toggle_auth, this));
	connect_signal_mouse_left_click(find_widget<button>(get_window(), "translations_add", false),
		std::bind(&editor_edit_pbl::add_translation, this));
	connect_signal_mouse_left_click(find_widget<button>(get_window(), "translations_delete", false),
		std::bind(&editor_edit_pbl::delete_translation, this));
	connect_signal_mouse_left_click(
		find_widget<button>(get_window(), "validate", false), std::bind(&editor_edit_pbl::validate, this));
	connect_signal_mouse_left_click(
		find_widget<button>(get_window(), "select_icon", false), std::bind(&editor_edit_pbl::select_icon_file, this));
	connect_signal_notify_modified(
		find_widget<text_box>(get_window(), "icon", false), std::bind(&editor_edit_pbl::update_icon_preview, this));
	connect_signal_notify_modified(find_widget<text_box>(get_window(), "forum_thread", false),
		std::bind(&editor_edit_pbl::update_url_preview, this));
	label& url = find_widget<label>(get_window(), "forum_url", false);
	url.set_link_aware(true);
	url.set_use_markup(true);
	// not setting this to some value causes the modified signal to not update the label text
	url.set_label("https://r.wesnoth.org/t");
}

void editor_edit_pbl::pre_show(window& win)
{
	config pbl;
	if(filesystem::file_exists(pbl_)) {
		try {
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
	if(!pbl["icon"].empty()) {
		drawing& img = find_widget<drawing>(&win, "preview", false);
		img.set_label(pbl["icon"]);
	}
	find_widget<text_box>(&win, "author", false).set_value(pbl["author"]);
	find_widget<text_box>(&win, "version", false).set_value(pbl["version"]);

	multimenu_button& dependencies = find_widget<multimenu_button>(&win, "dependencies", false);
	std::vector<config> addons_list;
	filesystem::get_files_in_dir(filesystem::get_addons_dir(), nullptr, &dirs_, filesystem::name_mode::FILE_NAME_ONLY);
	if(dirs_.size() > 0 && std::find(dirs_.begin(), dirs_.end(), current_addon_) != dirs_.end()) {
		dirs_.erase(std::remove(dirs_.begin(), dirs_.end(), current_addon_));
	}

	for(const std::string& dir : dirs_) {
		addons_list.emplace_back("label", dir, "checkbox", false);
	}
	dependencies.set_values(addons_list);

	std::vector<std::string> existing_dependencies = utils::split(pbl["dependencies"].str(), ',');
	for(unsigned i = 0; i < dirs_.size(); i++) {
		if(std::find(existing_dependencies.begin(), existing_dependencies.end(), dirs_[i]) != existing_dependencies.end()) {
			dependencies.select_option(i);
		}
	}

	if(pbl["forum_auth"].to_bool()) {
		find_widget<toggle_button>(&win, "forum_auth", false).set_value(true);
		find_widget<text_box>(&win, "email", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(&win, "email_label", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>(&win, "password", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(&win, "password_label", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>(&win, "secondary_authors", false).set_visible(gui2::widget::visibility::visible);
		find_widget<label>(&win, "secondary_authors_label", false).set_visible(gui2::widget::visibility::visible);
	} else {
		find_widget<text_box>(&win, "email", false).set_value(pbl["email"]);
		find_widget<text_box>(&win, "password", false).set_value(pbl["passphrase"]);
		find_widget<text_box>(&win, "secondary_authors", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(&win, "secondary_authors_label", false).set_visible(gui2::widget::visibility::invisible);
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
			{"translations_language", widget_item{{"label", child["language"].str()}}},
			{"translations_title", widget_item{{"label", child["title"].str()}}},
			{"translations_description", widget_item{{"label", child["description"].str()}}},
		};
		translations.add_row(entry);
	}

	if(translations.get_item_count() == 0) {
		translations_delete.set_active(false);
	}
}

void editor_edit_pbl::post_show(window&)
{
	if(get_retval() != retval::OK) {
		return;
	}

	std::stringstream wml_stream;
	config_writer out(wml_stream, false);
	out.write(create_cfg());
	filesystem::write_file(pbl_, wml_stream.str());
}

config editor_edit_pbl::create_cfg()
{
	config cfg;

	if(const std::string& name = find_widget<text_box>(get_window(), "name", false).get_value(); !name.empty()) {
		cfg["title"] = name;
	}
	if(const std::string& description = find_widget<text_box>(get_window(), "description", false).get_value(); !description.empty()) {
		cfg["description"] = description;
	}
	if(const std::string& icon = find_widget<text_box>(get_window(), "icon", false).get_value(); !icon.empty()) {
		cfg["icon"] = icon;
	}
	if(const std::string& author = find_widget<text_box>(get_window(), "author", false).get_value(); !author.empty()) {
		cfg["author"] = author;
	}
	if(const std::string& version = find_widget<text_box>(get_window(), "version", false).get_value(); !version.empty()) {
		cfg["version"] = version;
	}

	multimenu_button& dependencies = find_widget<multimenu_button>(get_window(), "dependencies", false);
	boost::dynamic_bitset<> dep_states = dependencies.get_toggle_states();
	std::vector<std::string> chosen_deps;
	for(unsigned i = 0; i < dep_states.size(); i++) {
		if(dep_states[i] == 1) {
			chosen_deps.emplace_back(dirs_[i]);
		}
	}
	if(chosen_deps.size() > 0) {
		cfg["dependencies"] = utils::join(chosen_deps, ",");
	}

	if(find_widget<toggle_button>(get_window(), "forum_auth", false).get_value_bool()) {
		cfg["forum_auth"] = true;
	} else {
		if(const std::string& email = find_widget<text_box>(get_window(), "email", false).get_value(); !email.empty()) {
			cfg["email"] = email;
		}
		if(const std::string& passphrase = find_widget<text_box>(get_window(), "password", false).get_value(); !passphrase.empty()) {
			cfg["passphrase"] = passphrase;
		}
	}

	if(const std::string& topic_id = find_widget<text_box>(get_window(), "forum_thread", false).get_value(); !topic_id.empty()) {
		config& feedback = cfg.add_child("feedback");
		feedback["topic_id"] = topic_id;
	}

	if(unsigned value = find_widget<menu_button>(get_window(), "type", false).get_value(); value != 0) {
		cfg["type"] = type_values[value];
	}

	multimenu_button& tags = find_widget<multimenu_button>(get_window(), "tags", false);
	boost::dynamic_bitset<> tag_states = tags.get_toggle_states();
	std::vector<std::string> chosen_tags;
	for(unsigned i = 0; i < tag_states.size(); i++) {
		if(tag_states[i] == 1) {
			chosen_tags.emplace_back(dirs_[i]);
		}
	}
	if(chosen_tags.size() > 0) {
		cfg["tags"] = utils::join(chosen_tags, ",");
	}

	listbox& translations = find_widget<listbox>(get_window(), "translations", false);
	for(unsigned i = 0; i < translations.get_item_count(); i++) {
		grid* row = translations.get_row_grid(i);
		config& translation = cfg.add_child("translation");

		translation["language"] = dynamic_cast<label*>(row->find("translations_language", false))->get_label();
		translation["title"] = dynamic_cast<label*>(row->find("translations_title", false))->get_label();
		translation["description"] = dynamic_cast<label*>(row->find("translations_description", false))->get_label();
	}

	return cfg;
}

void editor_edit_pbl::toggle_auth()
{
	toggle_button& forum_auth = find_widget<toggle_button>(get_window(), "forum_auth", false);
	if(forum_auth.get_value_bool()) {
		find_widget<text_box>(get_window(), "email", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>(get_window(), "password", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(get_window(), "email_label", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(get_window(), "password_label", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>(get_window(), "secondary_authors", false).set_visible(gui2::widget::visibility::visible);
		find_widget<label>(get_window(), "secondary_authors_label", false).set_visible(gui2::widget::visibility::visible);
	} else {
		find_widget<text_box>(get_window(), "email", false).set_visible(gui2::widget::visibility::visible);
		find_widget<text_box>(get_window(), "password", false).set_visible(gui2::widget::visibility::visible);
		find_widget<label>(get_window(), "email_label", false).set_visible(gui2::widget::visibility::visible);
		find_widget<label>(get_window(), "password_label", false).set_visible(gui2::widget::visibility::visible);
		find_widget<text_box>(get_window(), "secondary_authors", false).set_visible(gui2::widget::visibility::invisible);
		find_widget<label>(get_window(), "secondary_authors_label", false).set_visible(gui2::widget::visibility::invisible);
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
			{"translations_language", widget_item{{"label", language}}},
			{"translations_title", widget_item{{"label", title}}},
			{"translations_description", widget_item{{"label", description}}},
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

void editor_edit_pbl::validate()
{
	std::unique_ptr<schema_validation::schema_validator> validator;
	validator.reset(new schema_validation::schema_validator(filesystem::get_wml_location("schema/pbl.cfg")));
	validator->set_create_exceptions(false);

	config temp;
	std::stringstream ss;
	ss << create_cfg();
	read(temp, ss.str(), validator.get());
	if(!validator->get_errors().empty()) {
		gui2::show_error_message(utils::join(validator->get_errors(), "\n"));
	} else {
		gui2::show_message(_("Success"), _("No validation errors"), gui2::dialogs::message::button_style::auto_close);
	}
}

void editor_edit_pbl::update_icon_preview()
{
	std::string icon = find_widget<text_box>(get_window(), "icon", false).get_value();
	if(icon.find(".png") != std::string::npos || icon.find(".jpg") != std::string::npos || icon.find(".webp") != std::string::npos) {
		std::string path = filesystem::get_core_images_dir() + icon;
		drawing& img = find_widget<drawing>(get_window(), "preview", false);

		if(filesystem::file_exists(path) || icon.find("data:image") != std::string::npos) {
			img.set_label(icon);
		} else {
			img.set_label("");
			ERR_ED << "Failed to find icon file: " << path;
		}
	}
}

void editor_edit_pbl::update_url_preview()
{
	std::string topic = find_widget<text_box>(get_window(), "forum_thread", false).get_value();
	find_widget<label>(get_window(), "forum_url", false).set_label("https://r.wesnoth.org/t" + topic);
}

void editor_edit_pbl::select_icon_file()
{
	gui2::dialogs::file_dialog dlg;

	dlg.set_title(_("Choose an icon")).set_path(filesystem::get_core_images_dir() + "/icons/");

	if(dlg.show()) {
		std::string path = dlg.path();
		if(path.find(filesystem::get_core_images_dir()) == 0) {
			std::string icon = path.substr(filesystem::get_core_images_dir().length() + 1);
			// setting this programmatically doesn't seem to trigger connect_signal_notify_modified()
			find_widget<text_box>(get_window(), "icon", false).set_value(icon);
			find_widget<drawing>(get_window(), "preview", false).set_label(icon);
		} else {
			std::string uri = filesystem::read_file_as_data_uri(path);

			if(!uri.empty()) {
				find_widget<text_box>(get_window(), "icon", false).set_value(uri);
				find_widget<drawing>(get_window(), "preview", false).set_label(uri);
			}
		}
	}
}

} // namespace gui2::dialogs
