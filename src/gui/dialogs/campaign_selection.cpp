/*
	Copyright (C) 2009 - 2024
	by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/dialogs/campaign_selection.hpp"

#include "filesystem.hpp"
#include "gui/dialogs/campaign_difficulty.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/multi_page.hpp"
#include "gui/widgets/multimenu_button.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/toggle_panel.hpp"
#include "gui/widgets/tree_view.hpp"
#include "gui/widgets/tree_view_node.hpp"
#include "gui/widgets/window.hpp"
#include "preferences/preferences.hpp"
#include "serialization/markup.hpp"
#include "utils/irdya_datetime.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(campaign_selection)

campaign_selection::campaign_selection(ng::create_engine& eng)
	: modal_dialog(window_id())
	, engine_(eng)
	, choice_(-1)
	, rng_mode_(RNG_DEFAULT)
	, mod_states_()
	, page_ids_()
	, difficulties_()
	, current_difficulty_()
	, current_sorting_(RANK)
	, currently_sorted_asc_(true)
	, mod_ids_()
{
	set_show_even_without_video(true);
	set_allow_plugin_skip(false);
}

void campaign_selection::campaign_selected()
{
	tree_view& tree = find_widget<tree_view>("campaign_tree");
	if(tree.empty()) {
		return;
	}

	assert(tree.selected_item());

	const std::string& campaign_id = tree.selected_item()->id();

	if(!campaign_id.empty()) {
		auto iter = std::find(page_ids_.begin(), page_ids_.end(), campaign_id);

		button& ok_button = find_widget<button>("proceed");
		ok_button.set_active(campaign_id != missing_campaign_);
		ok_button.set_label((campaign_id == addons_) ? _("game^Get Add-ons") : _("game^Play"));

		const int choice = std::distance(page_ids_.begin(), iter);
		if(iter == page_ids_.end()) {
			return;
		}

		multi_page& pages = find_widget<multi_page>("campaign_details");
		pages.select_page(choice);

		engine_.set_current_level(choice);

		styled_widget& background = find_widget<styled_widget>("campaign_background");
		background.set_label(engine_.current_level().data()["background"].str());

		// Rebuild difficulty menu
		difficulties_.clear();

		auto& diff_menu = find_widget<menu_button>("difficulty_menu");

		const auto& diff_config = generate_difficulty_config(engine_.current_level().data());
		diff_menu.set_active(diff_config.child_count("difficulty") > 1);

		if(!diff_config.empty()) {
			std::vector<config> entry_list;
			unsigned n = 0, selection = 0, max_n = diff_config.child_count("difficulty");

			for(const auto& cfg : diff_config.child_range("difficulty")) {
				config entry;

				// FIXME: description may have markup that will display weird on the menu_button proper
				entry["label"] = cfg["label"].str() + " (" + cfg["description"].str() + ")";
				entry["image"] = cfg["image"].str("misc/blank-hex.png");

				if(prefs::get().is_campaign_completed(campaign_id, cfg["define"])) {
					std::string laurel;

					if(n + 1 >= max_n) {
						laurel = game_config::images::victory_laurel_hardest;
					} else if(n == 0) {
						laurel = game_config::images::victory_laurel_easy;
					} else {
						laurel = game_config::images::victory_laurel;
					}

					entry["image"] = laurel + "~BLIT(" + entry["image"].str() + ")";
				}

				if(!cfg["description"].empty()) {
					std::string desc;
					if(cfg["auto_markup"].to_bool(true) == false) {
						desc = cfg["description"].str();
					} else {
						if(!cfg["old_markup"].to_bool()) {
							desc += markup::span_color(font::GRAY_COLOR, "(", cfg["description"].str(), ")");
						} else {
							desc += markup::span_color(font::GRAY_COLOR, cfg["description"].str());
						}
					}

					// Icons get displayed instead of the labels on the dropdown menu itself,
					// so we want to prepend each label to its description here
					desc = cfg["label"].str() + "\n" + desc;

					entry["details"] = std::move(desc);
				}

				entry_list.emplace_back(std::move(entry));
				difficulties_.emplace_back(cfg["define"].str());

				if(cfg["default"].to_bool(false)) {
					selection = n;
				}

				++n;
			}

			diff_menu.set_values(entry_list);
			diff_menu.set_selected(selection);
		}
	}
}

void campaign_selection::difficulty_selected()
{
	const std::size_t selection = find_widget<menu_button>("difficulty_menu").get_value();
	current_difficulty_ = difficulties_.at(std::min(difficulties_.size() - 1, selection));
}

void campaign_selection::sort_campaigns(campaign_selection::CAMPAIGN_ORDER order, bool ascending)
{
	using level_ptr = ng::create_engine::level_ptr;

	auto levels = engine_.get_levels_by_type_unfiltered(level_type::type::sp_campaign);

	switch(order) {
	case RANK: // Already sorted by rank
		// This'll actually never happen, but who knows if that'll ever change...
		if(!ascending) {
			std::reverse(levels.begin(), levels.end());
		}

		break;

	case DATE:
		std::sort(levels.begin(), levels.end(), [ascending](const level_ptr& a, const level_ptr& b) {
			auto cpn_a = std::dynamic_pointer_cast<ng::campaign>(a);
			auto cpn_b = std::dynamic_pointer_cast<ng::campaign>(b);

			if(cpn_b == nullptr) {
				return cpn_a != nullptr;
			}

			if(cpn_a == nullptr) {
				return false;
			}

			return ascending
				? cpn_a->dates().first < cpn_b->dates().first
				: cpn_a->dates().first > cpn_b->dates().first;
		});

		break;

	case NAME:
		std::sort(levels.begin(), levels.end(), [ascending](const level_ptr& a, const level_ptr& b) {
			const int cmp = translation::icompare(a->name(), b->name());
			return ascending ? cmp < 0 : cmp > 0;
		});

		break;
	}

	tree_view& tree = find_widget<tree_view>("campaign_tree");

	// Remember which campaign was selected...
	std::string was_selected;
	if(!tree.empty()) {
		was_selected = tree.selected_item()->id();
		tree.clear();
	}

	boost::dynamic_bitset<> show_items;
	show_items.resize(levels.size(), true);

	if(!last_search_words_.empty()) {
		for(unsigned i = 0; i < levels.size(); ++i) {
			bool found = false;
			for(const auto& word : last_search_words_) {
				found = translation::ci_search(levels[i]->name(), word) ||
						translation::ci_search(levels[i]->data()["name"].t_str().base_str(), word) ||
						translation::ci_search(levels[i]->description(), word) ||
						translation::ci_search(levels[i]->data()["description"].t_str().base_str(), word) ||
						translation::ci_search(levels[i]->data()["abbrev"], word) ||
						translation::ci_search(levels[i]->data()["abbrev"].t_str().base_str(), word);

				if(!found) {
					break;
				}
			}

			show_items[i] = found;
		}
	}

	// List of which options has been selected in the completion filter multimenu_button
	boost::dynamic_bitset<> filter_comp_options = find_widget<multimenu_button>("filter_completion").get_toggle_states();

	bool exists_in_filtered_result = false;
	for(unsigned i = 0; i < levels.size(); ++i) {
		bool completed = prefs::get().is_campaign_completed(levels[i]->data()["id"]);
		config::const_child_itors difficulties = levels[i]->data().child_range("difficulty");
		auto did_complete_at = [](const config& c) { return c["completed_at"].to_bool(); };

		// Check for non-completion on every difficulty save the first.
		const bool only_first_completed = difficulties.size() > 1 &&
			std::none_of(difficulties.begin() + 1, difficulties.end(), did_complete_at);
		const bool completed_easy = only_first_completed && did_complete_at(difficulties.front());
		const bool completed_hardest = !difficulties.empty() && did_complete_at(difficulties.back());
		const bool completed_mid = completed && !completed_hardest && !completed_easy;

		if( show_items[i] && (
					( (!completed) && filter_comp_options[0] )       // Selects all campaigns not finished by player
				 || ( completed && filter_comp_options[4] )          // Selects all campaigns finished by player
				 || ( completed_hardest && filter_comp_options[3] )  // Selects campaigns completed in hardest difficulty
				 || ( completed_easy && filter_comp_options[1] )     // Selects campaigns completed in easiest difficulty
				 || ( completed_mid && filter_comp_options[2])       // Selects campaigns completed in any other difficulty
				 )) {
			add_campaign_to_tree(levels[i]->data());
			if (!exists_in_filtered_result) {
				exists_in_filtered_result = levels[i]->id() == was_selected;
			}
		}
	}

	if(!was_selected.empty() && exists_in_filtered_result) {
		find_widget<tree_view_node>(was_selected).select_node();
	} else {
		campaign_selected();
	}
}

void campaign_selection::toggle_sorting_selection(CAMPAIGN_ORDER order)
{
	static bool force = false;
	if(force) {
		return;
	}

	if(current_sorting_ == order) {
		if(currently_sorted_asc_) {
			currently_sorted_asc_ = false;
		} else {
			currently_sorted_asc_ = true;
			current_sorting_ = RANK;
		}
	} else if(current_sorting_ == RANK) {
		currently_sorted_asc_ = true;
		current_sorting_ = order;
	} else {
		currently_sorted_asc_ = true;
		current_sorting_ = order;

		force = true;

		if(order == NAME) {
			find_widget<toggle_button>("sort_time").set_value(0);
		} else if(order == DATE) {
			find_widget<toggle_button>("sort_name").set_value(0);
		}

		force = false;
	}

	sort_campaigns(current_sorting_, currently_sorted_asc_);
}

void campaign_selection::filter_text_changed(const std::string& text)
{
	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_search_words_) {
		return;
	}

	last_search_words_ = words;
	sort_campaigns(current_sorting_, currently_sorted_asc_);
}

void campaign_selection::pre_show()
{
	text_box* filter = find_widget<text_box>("filter_box", false, true);
	filter->on_modified([this](const auto& box) { filter_text_changed(box.text()); });

	/***** Setup campaign tree. *****/
	tree_view& tree = find_widget<tree_view>("campaign_tree");

	connect_signal_notify_modified(tree,
		std::bind(&campaign_selection::campaign_selected, this));

	toggle_button& sort_name = find_widget<toggle_button>("sort_name");
	toggle_button& sort_time = find_widget<toggle_button>("sort_time");

	connect_signal_notify_modified(sort_name,
		std::bind(&campaign_selection::toggle_sorting_selection, this, NAME));

	connect_signal_notify_modified(sort_time,
		std::bind(&campaign_selection::toggle_sorting_selection, this, DATE));

	connect_signal_mouse_left_click(find_widget<button>("proceed"),
		std::bind(&campaign_selection::proceed, this));

	keyboard_capture(filter);
	add_to_keyboard_chain(&tree);

	/***** Setup campaign details. *****/
	multi_page& pages = find_widget<multi_page>("campaign_details");

	// Setup completion filter
	multimenu_button& filter_comp = find_widget<multimenu_button>("filter_completion");
	connect_signal_notify_modified(filter_comp,
		std::bind(&campaign_selection::sort_campaigns, this, RANK, 1));
	for (unsigned j = 0; j < filter_comp.num_options(); j++) {
		filter_comp.select_option(j);
	}

	// Add campaigns to the list
	for(const auto& level : engine_.get_levels_by_type_unfiltered(level_type::type::sp_campaign)) {
		const config& campaign = level->data();

		/*** Add tree item ***/
		add_campaign_to_tree(campaign);

		/*** Add detail item ***/
		widget_data data;
		widget_item item;

		item["label"] = campaign["description"];
		item["use_markup"] = "true";

		if(!campaign["description_alignment"].empty()) {
			item["text_alignment"] = campaign["description_alignment"];
		}

		data.emplace("description", item);

		item["label"] = campaign["image"];
		data.emplace("image", item);

		pages.add_page(data);
		page_ids_.push_back(campaign["id"]);
	}

	//
	// Addon Manager link
	//
	config addons;
	addons["icon"] = "icons/icon-game.png~BLIT(icons/icon-addon-publish.png)";
	addons["name"] = _("More campaigns...");
	addons["completed"] = false;
	addons["id"] = addons_;

	add_campaign_to_tree(addons);

	widget_data data;
	widget_item item;

	item["label"] = _("In addition to the mainline campaigns, Wesnoth also has an ever-growing list of add-on content created by other players available via the Add-ons server, included but not limited to more single and multiplayer campaigns, multiplayer maps, additional media and various other content! Be sure to give it a try!");
	data.emplace("description", item);
	pages.add_page(data);
	page_ids_.push_back(addons_);

	std::vector<std::string> dirs;
	filesystem::get_files_in_dir(game_config::path + "/data/campaigns", nullptr, &dirs);
	if(dirs.size() <= 15) {
		config missing;
		missing["icon"] = "units/unknown-unit.png";
		missing["name"] = _("Missing Campaigns");
		missing["completed"] = false;
		missing["id"] = missing_campaign_;

		add_campaign_to_tree(missing);

		widget_data data;
		widget_item item;

		// TRANSLATORS: "more than 15" gives a little leeway to add or remove one without changing the translatable text.
		// It's already ambiguous, 1.18 has 19 campaigns, if you include the tutorial and multiplayer-only World Conquest.
		item["label"] = _("Wesnoth normally includes more than 15 mainline campaigns, even before installing any from the add-ons server. If you’ve installed the game via a package manager, there’s probably a separate package to install the complete game data.");
		data.emplace("description", item);

		pages.add_page(data);
		page_ids_.push_back(missing_campaign_);
	}

	//
	// Set up Mods selection dropdown
	//
	multimenu_button& mods_menu = find_widget<multimenu_button>("mods_menu");

	if(!engine_.get_const_extras_by_type(ng::create_engine::MOD).empty()) {
		std::vector<config> mod_menu_values;
		std::vector<std::string> enabled = engine_.active_mods();

		for(const auto& mod : engine_.get_const_extras_by_type(ng::create_engine::MOD)) {
			const bool active = std::find(enabled.begin(), enabled.end(), mod->id) != enabled.end();

			mod_menu_values.emplace_back("label", mod->name, "checkbox", active);

			mod_states_.push_back(active);
			mod_ids_.emplace_back(mod->id);
		}

		mods_menu.set_values(mod_menu_values);
		mods_menu.select_options(mod_states_);

		connect_signal_notify_modified(mods_menu, std::bind(&campaign_selection::mod_toggled, this));
	} else {
		mods_menu.set_active(false);
		mods_menu.set_label(_("active_modifications^None"));
	}

	//
	// Set up Difficulty dropdown
	//
	menu_button& diff_menu = find_widget<menu_button>("difficulty_menu");

	diff_menu.set_use_markup(true);
	connect_signal_notify_modified(diff_menu, std::bind(&campaign_selection::difficulty_selected, this));

	campaign_selected();

	plugins_context_.reset(new plugins_context("Campaign Selection"));
	plugins_context_->set_callback("create", [this](const config&) { set_retval(retval::OK); }, false);
	plugins_context_->set_callback("quit", [this](const config&) { set_retval(retval::CANCEL); }, false);

	plugins_context_->set_accessor("find_level", [this](const config& cfg) {
		const std::string id = cfg["id"].str();
		auto result = engine_.find_level_by_id(id);
		return config {
			"index", result.second,
			"type", level_type::get_string(result.first),
		};
	});

	plugins_context_->set_accessor_int("find_mod", [this](const config& cfg) {
		return engine_.find_extra_by_id(ng::create_engine::MOD, cfg["id"]);
	});

	plugins_context_->set_callback("select_level", [this](const config& cfg) {
		choice_ = cfg["index"].to_int();
		engine_.set_current_level(choice_);
	}, true);
}

void campaign_selection::add_campaign_to_tree(const config& campaign)
{
	tree_view& tree = find_widget<tree_view>("campaign_tree");
	widget_data data;
	widget_item item;

	item["label"] = campaign["icon"];
	data.emplace("icon", item);

	item["label"] = campaign["name"];
	data.emplace("name", item);

	// We completed the campaign! Calculate the appropriate victory laurel.
	if(campaign["completed"].to_bool()) {
		config::const_child_itors difficulties = campaign.child_range("difficulty");

		auto did_complete_at = [](const config& c) { return c["completed_at"].to_bool(); };

		// Check for non-completion on every difficulty save the first.
		const bool only_first_completed = difficulties.size() > 1 &&
			std::none_of(difficulties.begin() + 1, difficulties.end(), did_complete_at);

		/*
		 * Criteria:
		 *
		 * - Use the gold laurel (hardest) for campaigns with only one difficulty OR
		 *   if out of two or more difficulties, the last one has been completed.
		 *
		 * - Use the bronze laurel (easiest) only if the first difficulty out of two
		 *   or more has been completed.
		 *
		 * - Use the silver laurel otherwise.
		 */
		if(!difficulties.empty() && did_complete_at(difficulties.back())) {
			item["label"] = game_config::images::victory_laurel_hardest;
		} else if(only_first_completed && did_complete_at(difficulties.front())) {
			item["label"] = game_config::images::victory_laurel_easy;
		} else {
			item["label"] = game_config::images::victory_laurel;
		}

		data.emplace("victory", item);
	}

	auto& node = tree.add_node("campaign", data);
	node.set_id(campaign["id"]);
	connect_signal_mouse_left_double_click(
		node.find_widget<toggle_panel>("tree_view_node_label"),
		std::bind(&campaign_selection::proceed, this)
	);
}

void campaign_selection::proceed()
{
	tree_view& tree = find_widget<tree_view>("campaign_tree");

	if(tree.empty()) {
		return;
	}

	assert(tree.selected_item());
	const std::string& campaign_id = tree.selected_item()->id();
	if(!campaign_id.empty()) {
		if (campaign_id == addons_) {
			set_retval(OPEN_ADDON_MANAGER);
		} else {
			auto iter = std::find(page_ids_.begin(), page_ids_.end(), campaign_id);
			if(iter != page_ids_.end()) {
				choice_ = std::distance(page_ids_.begin(), iter);
			}
			set_retval(retval::OK);
		}
	}


	rng_mode_ = RNG_MODE(std::clamp<unsigned>(find_widget<menu_button>("rng_menu").get_value(), RNG_DEFAULT, RNG_BIASED));

	prefs::get().set_modifications(engine_.active_mods(), false);
}

void campaign_selection::mod_toggled()
{
	boost::dynamic_bitset<> new_mod_states =
		find_widget<multimenu_button>("mods_menu").get_toggle_states();

	// Get a mask of any mods that were toggled, regardless of new state
	mod_states_ = mod_states_ ^ new_mod_states;

	for(unsigned i = 0; i < mod_states_.size(); i++) {
		if(mod_states_[i]) {
			engine_.toggle_mod(mod_ids_[i]);
		}
	}

	// Save the full toggle states for next time
	mod_states_ = new_mod_states;
}

} // namespace dialogs
