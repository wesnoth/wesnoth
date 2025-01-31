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

#include "addon/validation.hpp"
#include "editor/editor_common.hpp"
#include "filesystem.hpp"
#include "gettext.hpp"
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
#include "gui/widgets/scroll_text.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "serialization/binary_or_text.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "serialization/schema_validator.hpp"


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
	"theme",
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
		find_widget<toggle_button>("forum_auth"), std::bind(&editor_edit_pbl::toggle_auth, this));
	connect_signal_mouse_left_click(find_widget<button>("translations_add"),
		std::bind(&editor_edit_pbl::add_translation, this));
	connect_signal_mouse_left_click(find_widget<button>("translations_delete"),
		std::bind(&editor_edit_pbl::delete_translation, this));
	connect_signal_mouse_left_click(
		find_widget<button>("validate"), std::bind(&editor_edit_pbl::validate, this));
	connect_signal_mouse_left_click(
		find_widget<button>("select_icon"), std::bind(&editor_edit_pbl::select_icon_file, this));
	connect_signal_notify_modified(
		find_widget<text_box>("icon"), std::bind(&editor_edit_pbl::update_icon_preview, this));
	connect_signal_notify_modified(find_widget<text_box>("forum_thread"),
		std::bind(&editor_edit_pbl::update_url_preview, this));
	label& url = find_widget<label>("forum_url");
	url.set_link_aware(true);
	url.set_use_markup(true);
	// not setting this to some value causes the modified signal to not update the label text
	url.set_label("https://r.wesnoth.org/t");
}

void editor_edit_pbl::pre_show()
{
	config pbl;
	if(filesystem::file_exists(pbl_)) {
		try {
			read(pbl, *preprocess_file(pbl_));
		} catch(const config::error& e) {
			ERR_ED << "Caught a config error while parsing file " << pbl_ << "\n" << e.message;
		}
	}

	text_box* name = find_widget<text_box>("name", false, true);
	name->set_value(pbl["title"]);
	keyboard_capture(name);

	find_widget<scroll_text>("description").set_value(pbl["description"]);
	find_widget<text_box>("icon").set_value(pbl["icon"]);
	if(!pbl["icon"].empty()) {
		drawing& img = find_widget<drawing>("preview");
		img.set_label(pbl["icon"]);
	}
	find_widget<text_box>("author").set_value(pbl["author"]);
	find_widget<text_box>("version").set_value(pbl["version"]);

	multimenu_button& dependencies = find_widget<multimenu_button>("dependencies");
	std::vector<config> addons_list;
	filesystem::get_files_in_dir(filesystem::get_addons_dir(), nullptr, &dirs_, filesystem::name_mode::FILE_NAME_ONLY);
	if(dirs_.size() > 0 && std::find(dirs_.begin(), dirs_.end(), current_addon_) != dirs_.end()) {
		utils::erase(dirs_, current_addon_);
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
		find_widget<toggle_button>("forum_auth").set_value(true);
		find_widget<text_box>("primary_authors").set_value(pbl["primary_authors"]);
		find_widget<text_box>("secondary_authors").set_value(pbl["secondary_authors"]);
		find_widget<text_box>("email").set_visible(gui2::widget::visibility::invisible);
		find_widget<label>("email_label").set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>("password").set_visible(gui2::widget::visibility::invisible);
		find_widget<label>("password_label").set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>("primary_authors").set_visible(gui2::widget::visibility::visible);
		find_widget<label>("primary_authors_label").set_visible(gui2::widget::visibility::visible);
		find_widget<text_box>("secondary_authors").set_visible(gui2::widget::visibility::visible);
		find_widget<label>("secondary_authors_label").set_visible(gui2::widget::visibility::visible);
	} else {
		find_widget<text_box>("email").set_value(pbl["email"]);
		find_widget<text_box>("password").set_value(pbl["passphrase"]);
		find_widget<text_box>("primary_authors").set_visible(gui2::widget::visibility::invisible);
		find_widget<label>("primary_authors_label").set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>("secondary_authors").set_visible(gui2::widget::visibility::invisible);
		find_widget<label>("secondary_authors_label").set_visible(gui2::widget::visibility::invisible);
	}

	if(pbl.has_child("feedback")) {
		find_widget<text_box>("forum_thread").set_value(pbl.mandatory_child("feedback")["topic_id"]);
	}

	unsigned selected = 0;
	for(unsigned i = 0; i < type_values.size(); i++) {
		if(type_values[i] == pbl["type"]) {
			selected = i;
			break;
		}
	}

	menu_button& types = find_widget<menu_button>("type");
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
	type_list.emplace_back("label", _("Theme"));
	type_list.emplace_back("label", _("Other"));
	types.set_values(type_list);
	types.set_selected(selected);

	multimenu_button& tags = find_widget<multimenu_button>("tags");
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

	listbox& translations = find_widget<listbox>("translations");
	button& translations_delete = find_widget<button>("translations_delete");

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

void editor_edit_pbl::post_show()
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

	if(const std::string& name = find_widget<text_box>("name").get_value(); !name.empty()) {
		cfg["title"] = name;
	}
	if(const std::string& description = find_widget<scroll_text>("description").get_value(); !description.empty()) {
		cfg["description"] = description;
	}
	if(const std::string& icon = find_widget<text_box>("icon").get_value(); !icon.empty()) {
		cfg["icon"] = icon;
	}
	if(const std::string& author = find_widget<text_box>("author").get_value(); !author.empty()) {
		cfg["author"] = author;
	}
	if(const std::string& version = find_widget<text_box>("version").get_value(); !version.empty()) {
		cfg["version"] = version;
	}

	multimenu_button& dependencies = find_widget<multimenu_button>("dependencies");
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

	if(find_widget<toggle_button>("forum_auth").get_value_bool()) {
		cfg["forum_auth"] = true;

		if(const std::string& primary_authors = find_widget<text_box>("primary_authors").get_value(); !primary_authors.empty()) {
			cfg["primary_authors"] = primary_authors;
		}

		if(const std::string& secondary_authors = find_widget<text_box>("secondary_authors").get_value(); !secondary_authors.empty()) {
			cfg["secondary_authors"] = secondary_authors;
		}
	} else {
		if(const std::string& email = find_widget<text_box>("email").get_value(); !email.empty()) {
			cfg["email"] = email;
		}
		if(const std::string& passphrase = find_widget<text_box>("password").get_value(); !passphrase.empty()) {
			cfg["passphrase"] = passphrase;
		}
	}

	if(const std::string& topic_id = find_widget<text_box>("forum_thread").get_value(); !topic_id.empty()) {
		config& feedback = cfg.add_child("feedback");
		feedback["topic_id"] = topic_id;
	}

	if(unsigned value = find_widget<menu_button>("type").get_value(); value != 0) {
		cfg["type"] = type_values[value];
	}

	multimenu_button& tags = find_widget<multimenu_button>("tags");
	boost::dynamic_bitset<> tag_states = tags.get_toggle_states();
	std::vector<std::string> chosen_tags;
	for(unsigned i = 0; i < tag_states.size(); i++) {
		if(tag_states[i] == 1) {
			chosen_tags.emplace_back(tag_values[i]);
		}
	}
	if(chosen_tags.size() > 0) {
		cfg["tags"] = utils::join(chosen_tags, ",");
	}

	listbox& translations = find_widget<listbox>("translations");
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
	toggle_button& forum_auth = find_widget<toggle_button>("forum_auth");
	if(forum_auth.get_value_bool()) {
		find_widget<text_box>("email").set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>("password").set_visible(gui2::widget::visibility::invisible);
		find_widget<label>("email_label").set_visible(gui2::widget::visibility::invisible);
		find_widget<label>("password_label").set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>("primary_authors").set_visible(gui2::widget::visibility::visible);
		find_widget<label>("primary_authors_label").set_visible(gui2::widget::visibility::visible);
		find_widget<text_box>("secondary_authors").set_visible(gui2::widget::visibility::visible);
		find_widget<label>("secondary_authors_label").set_visible(gui2::widget::visibility::visible);
	} else {
		find_widget<text_box>("email").set_visible(gui2::widget::visibility::visible);
		find_widget<text_box>("password").set_visible(gui2::widget::visibility::visible);
		find_widget<label>("email_label").set_visible(gui2::widget::visibility::visible);
		find_widget<label>("password_label").set_visible(gui2::widget::visibility::visible);
		find_widget<text_box>("primary_authors").set_visible(gui2::widget::visibility::invisible);
		find_widget<label>("primary_authors_label").set_visible(gui2::widget::visibility::invisible);
		find_widget<text_box>("secondary_authors").set_visible(gui2::widget::visibility::invisible);
		find_widget<label>("secondary_authors_label").set_visible(gui2::widget::visibility::invisible);
	}
}

void editor_edit_pbl::add_translation()
{
	std::string language;
	std::string title;
	std::string description;
	editor_edit_pbl_translation::execute(language, title, description);

	if(!language.empty() && !title.empty()) {
		listbox& translations = find_widget<listbox>("translations");
		const widget_data& entry{
			{"translations_language", widget_item{{"label", language}}},
			{"translations_title", widget_item{{"label", title}}},
			{"translations_description", widget_item{{"label", description}}},
		};
		translations.add_row(entry);
		find_widget<button>("translations_delete").set_active(true);
	}
}

void editor_edit_pbl::delete_translation()
{
	listbox& translations = find_widget<listbox>("translations");
	translations.remove_row(translations.get_selected_row());

	button& translations_delete = find_widget<button>("translations_delete");
	if(translations.get_item_count() == 0) {
		translations_delete.set_active(false);
	}
}

void editor_edit_pbl::validate()
{
	std::unique_ptr<schema_validation::schema_validator> validator;
	validator.reset(new schema_validation::schema_validator(filesystem::get_wml_location("schema/pbl.cfg").value()));
	validator->set_create_exceptions(false);

	config temp;
	std::stringstream ss;
	ss << create_cfg();
	read(temp, ss.str(), validator.get());
	if(!validator->get_errors().empty()) {
		gui2::show_error_message(utils::join(validator->get_errors(), "\n"));
	} else if(addon_icon_too_large(temp["icon"].str())) {
		gui2::show_error_message(_("The icon’s file size is too large"));
	} else {
		gui2::show_message(_("Success"), _("No validation errors"), gui2::dialogs::message::button_style::auto_close);
	}
}

void editor_edit_pbl::update_icon_preview()
{
	std::string icon = find_widget<text_box>("icon").get_value();
	if(icon.find(".png") != std::string::npos || icon.find(".jpg") != std::string::npos || icon.find(".webp") != std::string::npos) {
		std::string path = filesystem::get_core_images_dir() + icon;
		drawing& img = find_widget<drawing>("preview");

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
	std::string topic = find_widget<text_box>("forum_thread").get_value();
	find_widget<label>("forum_url").set_label("https://r.wesnoth.org/t" + topic);
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
			find_widget<text_box>("icon").set_value(icon);
			find_widget<drawing>("preview").set_label(icon);
		} else {
			std::string uri = filesystem::read_file_as_data_uri(path);

			if(!uri.empty()) {
				find_widget<text_box>("icon").set_value(uri);
				find_widget<drawing>("preview").set_label(uri);
			}
		}
	}
}

} // namespace gui2::dialogs
