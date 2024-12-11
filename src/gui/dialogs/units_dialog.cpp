/*
	Copyright (C) 2024
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

#include "gui/dialogs/units_dialog.hpp"

#include "formatter.hpp"
#include "game_board.hpp"
#include "gettext.hpp"
#include "gui/dialogs/edit_text.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/menu_button.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "help/help.hpp"
#include "log.hpp"
#include "replay_helper.hpp"
#include "play_controller.hpp"
#include "resources.hpp"
#include "serialization/markup.hpp"
#include "synced_context.hpp"
#include "team.hpp"
#include "units/helper.hpp"
#include "units/unit.hpp"
#include "units/ptr.hpp"
#include "units/types.hpp"
#include "whiteboard/manager.hpp"

#include <functional>
#include <string>

static lg::log_domain log_display("display");
#define LOG_DP LOG_STREAM(info, log_display)

namespace gui2::dialogs
{

namespace
{
// Index 2 is by-level
static std::pair sort_default{ std::string{"unit_name"}, sort_order::type::descending };
static utils::optional<decltype(sort_default)> sort_last;
}

REGISTER_DIALOG(units_dialog)

units_dialog::units_dialog()
	: modal_dialog(window_id())
	, unit_type_list_()
	, unit_list_()
	, team_(nullptr)
	, selected_index_(-1)
	, row_num_(0)
	, ok_label_(_("OK"))
	, cancel_label_(_("Cancel"))
	, show_header_(true)
	, show_variation_grid_(false)
	, show_gender_grid_(false)
	, show_dismiss_(false)
	, show_rename_(false)
	, gender_(unit_race::GENDER::MALE)
	, variation_()
	, filter_options_()
	, last_words_()
{
}

namespace {

template<typename T>
void dump_recall_list_to_console(const T& units)
{
	log_scope2(log_display, "dump_recall_list_to_console()")

	LOG_DP << "size: " << units.size();

	std::size_t idx = 0;
	for(const auto& u_ptr : units) {
		LOG_DP << "\tunit[" << (idx++) << "]: " << u_ptr->id() << " name = '" << u_ptr->name() << "'";
	}
}

std::string get_title_suffix(int side_num)
{
	if(!resources::gameboard) {
		return "";
	}

	unit_map& units = resources::gameboard->units();

	int controlled_recruiters = 0;
	for(const auto& team : resources::gameboard->teams()) {
		if(team.is_local_human() && !team.recruits().empty() && units.find_leader(team.side()) !=units.end()) {
			++controlled_recruiters;
		}
	}

	std::stringstream msg;
	if(controlled_recruiters >= 2) {
		unit_map::const_iterator leader = resources::gameboard->units().find_leader(side_num);
		if(leader != resources::gameboard->units().end() && !leader->name().empty()) {
			msg << " (" << leader->name() << ")";
		}
	}

	return msg.str();
}
}

void units_dialog::pre_show()
{
	text_box& filter = find_widget<text_box>("filter_box");
	connect_signal_notify_modified(filter, std::bind(&units_dialog::filter_text_changed, this));

	listbox& list = find_widget<listbox>("main_list");
	connect_signal_notify_modified(list, std::bind(&units_dialog::list_item_clicked, this));

	connect_signal_mouse_left_click(
		find_widget<button>("show_help"),
		std::bind(&units_dialog::show_help, this));

	list.clear();

	keyboard_capture(&filter);
	add_to_keyboard_chain(&list);

	if (show_gender_grid_) {
		toggle_button& male_toggle = find_widget<toggle_button>("male_toggle");
		toggle_button& female_toggle = find_widget<toggle_button>("female_toggle");

		gender_toggle_.add_member(&male_toggle, unit_race::MALE);
		gender_toggle_.add_member(&female_toggle, unit_race::FEMALE);
		gender_toggle_.set_member_states(unit_race::MALE);
		gender_toggle_.set_callback_on_value_change(
			std::bind(&units_dialog::gender_toggle_callback, this, std::placeholders::_2));
	}

	if (show_variation_grid_) {
		menu_button& var_box = find_widget<menu_button>("variation_box");
		connect_signal_notify_modified(var_box, std::bind(&units_dialog::variation_menu_callback, this));
	}

	if (show_rename_) {
		connect_signal_mouse_left_click(
		find_widget<button>("rename"),
		std::bind(&units_dialog::rename_unit, this));
	}

	if (show_dismiss_ && team_) {
		connect_signal_mouse_left_click(
			find_widget<button>("dismiss"),
			std::bind(&units_dialog::dismiss_unit, this, *team_));
	}

	show_list(list);

	find_widget<label>("title").set_label(title_);
	find_widget<button>("ok").set_label(ok_label_);
	find_widget<button>("cancel").set_label(cancel_label_);
	find_widget<button>("dismiss").set_visible(
		show_dismiss_ ? widget::visibility::visible : widget::visibility::invisible);
	find_widget<button>("rename").set_visible(
		show_rename_ ? widget::visibility::visible : widget::visibility::invisible);
	find_widget<grid>("variation_gender_grid").set_visible(
		show_gender_grid_ ? widget::visibility::visible : widget::visibility::invisible);
	find_widget<grid>("_header_grid").set_visible(
		show_header_ ? widget::visibility::visible : widget::visibility::invisible);


	list_item_clicked();
}

void units_dialog::show_list(listbox& list)
{
	if (row_num_ == 0) {
		return;
	}

	for(std::size_t i = 0; i < row_num_; i++) {
		widget_data row_data;
		widget_item column;
		formatter filter_fmt;
		// generate tooltip for ith row
		if (tooltip_gen_) {
			column["tooltip"] = tooltip_gen_(i);
		}

		for (const auto& [id, gen] : column_generators_) {
			column["use_markup"] = "true";
			// generate label for ith row and column with 'id'
			column["label"] = gen(i);
			if (id != "unit_image") {
				filter_fmt << column["label"];
			}
			row_data.emplace(id, column);
		}

		filter_options_.push_back(filter_fmt.str());
		list.add_row(row_data);
	}

	const auto [sorter_id, order] = sort_last.value_or(sort_default);
	list.set_active_sorter(sorter_id, order, true);
}

void units_dialog::rename_unit()
{
	listbox& list = find_widget<listbox>("main_list");

	const int index = list.get_selected_row();
	if (index == -1) {
		return;
	}

	unit& selected_unit = const_cast<unit&>(*unit_list_[index].get());

	std::string name = selected_unit.name();
	const std::string dialog_title(_("Rename Unit"));
	const std::string dialog_label(_("Name:"));

	if(gui2::dialogs::edit_text::execute(dialog_title, dialog_label, name)) {
		selected_unit.rename(name);

		list.get_row_grid(index)->find_widget<label>("unit_name").set_label(name);

		filter_options_.erase(filter_options_.begin() + index);
		std::ostringstream filter_text;
		filter_text << selected_unit.type_name() << " " << name << " " << std::to_string(selected_unit.level());
		for(const std::string& trait : selected_unit.trait_names()) {
			filter_text << " " << trait;
		}
		filter_options_.insert(filter_options_.begin() + index, filter_text.str());

		list_item_clicked();
		get_window()->invalidate_layout();
	}
}

void units_dialog::dismiss_unit(const team& team)
{
	LOG_DP << "Recall list units:"; dump_recall_list_to_console(unit_list_);

	listbox& list = find_widget<listbox>("main_list");
	const int index = list.get_selected_row();
	if (index == -1) {
		return;
	}

	const unit& u = *unit_list_[index].get();

	// If the unit is of level > 1, or is close to advancing, we warn the player about it
	std::stringstream message;
	if(u.loyal()) {
		message << _("This unit is loyal and requires no upkeep.") << " " << (u.gender() == unit_race::MALE
		         ? _("Do you really want to dismiss him?")
		         : _("Do you really want to dismiss her?"));

	} else if(u.level() > 1) {
		message << _("This unit is an experienced one, having advanced levels.") << " " << (u.gender() == unit_race::MALE
		         ? _("Do you really want to dismiss him?")
		         : _("Do you really want to dismiss her?"));

	} else if(u.experience() > u.max_experience()/2) {
		message << _("This unit is close to advancing a level.") << " " << (u.gender() == unit_race::MALE
		         ? _("Do you really want to dismiss him?")
		         : _("Do you really want to dismiss her?"));
	}

	if(!message.str().empty()) {
		const int res = gui2::show_message(_("Dismiss Unit"), message.str(), message::yes_no_buttons);
		if(res != gui2::retval::OK) {
			return;
		}
	}

	unit_list_.erase(unit_list_.begin() + index);

	// Remove the entry from the dialog list
	list.remove_row(index);
	list_item_clicked();

	// Remove the entry from the filter list
	filter_options_.erase(filter_options_.begin() + index);
	assert(filter_options_.size() == list.get_item_count());

	LOG_DP << "Dismissing a unit, side = " << u.side() << ", id = '" << u.id() << "'";
	LOG_DP << "That side's recall list:";
	dump_recall_list_to_console(team.recall_list());


	// Find the unit in the recall list.
	unit_const_ptr dismissed_unit = team.recall_list().find_if_matches_id(u.id());
	assert(dismissed_unit);

	// Record the dismissal, then delete the unit.
	synced_context::run_and_throw("disband", replay_helper::get_disband(dismissed_unit->id()));

	// Close the dialog if all units are dismissed
	if(list.get_item_count() == 0) {
		set_retval(retval::CANCEL);
	}
}

void units_dialog::show_help() const
{
	if (!topic_id_.empty()) {
		help::show_help(topic_id_);
	}
}

void units_dialog::list_item_clicked()
{
	const int selected_row
		= find_widget<listbox>("main_list").get_selected_row();

	if (selected_row == -1) {
		return;
	}

	auto& unit_preview = find_widget<unit_preview_pane>("unit_details");

	if (!unit_list_.empty()) {
		const unit& selected_unit = *unit_list_[selected_row].get();
		unit_preview.set_displayed_unit(selected_unit);
		find_widget<button>("rename").set_active(!selected_unit.unrenamable());
	} else if (!unit_type_list_.empty()) {
		unit_preview.set_displayed_type(update_gender_and_variations(selected_row));
	}
}

unit_type units_dialog::update_gender_and_variations(int selected_row)
{
	const unit_type* ut = unit_type_list_[selected_row];

	if (!show_gender_grid_) {
		return *ut;
	}

	gender_toggle_.set_members_enabled([&](const unit_race::GENDER& gender)->bool {
		return unit_type_list_[selected_row]->has_gender_variation(gender);
	});
	ut = &ut->get_gender_unit_type(gender_);

	if (!show_variation_grid_) {
		return *ut;
	}

	if(!variation_.empty()) {
		// This effectively translates to `ut = ut` if somehow variation_ does
		// not refer to a variation that the unit type supports.
		ut = &ut->get_variation(variation_);
	}

	menu_button& var_box = find_widget<menu_button>("variation_box");
	std::vector<config> var_box_values;
	var_box_values.emplace_back("label", _("unit_variation^Default Variation"), "variation_id", "");

	const auto& uvars = ut->variation_types();

	var_box.set_active(!uvars.empty());

	unsigned n = 0, selection = 0;

	for(const auto& pair : uvars) {
		++n;

		const std::string& uv_id = pair.first;
		const unit_type& uv = pair.second;

		std::string uv_label;
		if(!uv.variation_name().empty()) {
			uv_label = uv.variation_name() + " (" + uv_id + ")";
		} else if(!uv.type_name().empty() && uv.type_name() != ut->type_name()) {
			uv_label = uv.type_name() + " (" + uv_id + ")";
		} else {
			uv_label = uv_id;
		}

		var_box_values.emplace_back("label", uv_label, "variation_id", uv_id);

		if(uv_id == variation_) {
			selection = n;
		}
	}

	// If we didn't find the variation selection again then the new selected
	// unit type doesn't have that variation id.
	if(!selection) {
		variation_.clear();
	}

	var_box.set_values(var_box_values, selection);

	return *ut;
}

void units_dialog::post_show()
{
	listbox& list = find_widget<listbox>("main_list");
	if(const auto [sorter, order] = list.get_active_sorter(); sorter) {
		sort_last.emplace(sorter->id(), order);
	} else {
		sort_last.reset();
	}

	if(get_retval() == retval::OK) {
		selected_index_ = list.get_selected_row();
	}
}

void units_dialog::filter_text_changed()
{
	const std::string& text = find_widget<text_box>("filter_box").get_value();
	listbox& list = find_widget<listbox>("main_list");

	const std::vector<std::string> words = utils::split(text, ' ');

	if(words == last_words_) {
		return;
	}
	last_words_ = words;

	boost::dynamic_bitset<> show_items;
	show_items.resize(list.get_item_count(), true);

	if(!text.empty()) {
		for(unsigned int i = 0; i < list.get_item_count(); i++) {
			bool found = false;

			for(const auto & word : words) {
				found = translation::ci_search(filter_options_[i], word);

				if(!found) {
					// one word doesn't match, we don't reach words.end()
					break;
				}
			}

			show_items[i] = found;
		}
	}

	list.set_row_shown(show_items);

	// Disable rename and dismiss buttons if no units are shown
	const bool any_shown = list.any_rows_shown();
	find_widget<button>("rename").set_active(any_shown);
	find_widget<button>("dismiss").set_active(any_shown);
}

void units_dialog::gender_toggle_callback(const unit_race::GENDER val)
{
	gender_ = val;

	const int selected_row = find_widget<listbox>("main_list").get_selected_row();
	if(selected_row == -1) {
		return;
	}

	auto& unit_preview = find_widget<unit_preview_pane>("unit_details");
	const unit_type* ut = &unit_type_list_[selected_row]->get_gender_unit_type(gender_);
	unit_preview.set_displayed_type(*ut);
}

void units_dialog::variation_menu_callback()
{
	menu_button& var_box = find_widget<menu_button>("variation_box");
	variation_ = var_box.get_value_config()["variation_id"].str();

	const int selected_row = find_widget<listbox>("main_list").get_selected_row();
	if(selected_row == -1) {
		return;
	}

	if(!variation_.empty()) {
		auto& unit_preview = find_widget<unit_preview_pane>("unit_details");
		unit_preview.set_displayed_type(unit_type_list_[selected_row]->get_variation(variation_));
	}
}

// } -------------------- BUILDERS -------------------- {
units_dialog& units_dialog::build_create_dialog(const std::vector<const unit_type*>& types_list)
{
	const auto type_gen = [](const auto& type) {
		std::string type_name = type->type_name();
		if(type_name != type->id()) {
			type_name += " (" + type->id() + ")";
		}
		return type_name;
	};

	const auto race_gen = [](const auto& type) {
		return type->race()->plural_name();
	};

	set_title(_("Create Unit"));
	set_ok_label(_("Create"));
	set_help_topic("..units");
	show_gender(true);
	show_variations(true);
	set_types(types_list);
	set_row_num(types_list.size());
	hide_all_headers();
	set_column("unit_name", types_list, type_gen, true);
	set_column("unit_details", types_list, race_gen, true);

	return *this;
}

units_dialog& units_dialog::build_unit_list_dialog(const std::vector<unit_const_ptr>& unit_list)
{
	set_title(_("Unit List"));
	set_ok_label(_("Scroll To"));
	show_rename_option(true);
	set_help_topic("..units");
	set_units(unit_list);
	set_row_num(unit_list.size());
	set_column("unit_name", unit_list, [&](const auto& unit) {
		return !unit->name().empty() ? unit->name().str() : font::unicode_en_dash;
	}, true);

	set_column("unit_details", unit_list, [&](const auto& unit) {
		return unit->type_name().str();
	}, true);

	set_column("unit_level", unit_list,
		[&](const auto& unit) {
			return unit_helper::format_level_string(unit->level(), true);
		},
		[&](const auto& u) {
			return std::tuple(u->level(), -static_cast<int>(u->experience_to_advance()));
		});

	set_column("unit_moves", unit_list,
		[&](const auto& unit) {
			return unit_helper::format_movement_string(unit->movement_left(), unit->total_movement());
		},
		[&](const auto& u) {
			return u->movement_left();
		});

	set_column("unit_hp", unit_list,
		[&](const auto& unit) {
			return markup::span_color(unit->hp_color(), unit->hitpoints(), "/", unit->max_hitpoints());
		},
	 	[&](const auto& u) {
			return u->hitpoints();
		});

	set_column("unit_xp",  unit_list,
		[&](const auto& unit) {
			std::stringstream exp_str;
			if(unit->can_advance()) {
				exp_str << unit->experience() << "/" << unit->max_experience();
			} else {
				exp_str << font::unicode_en_dash;
			}
			return markup::span_color(unit->xp_color(), exp_str.str());
		},
		[&](const auto& u) {
			// this allows 0/35, 0/100 etc to be sorted
			// also sorts 23/35 before 0/35, after which 0/100 comes
			return u->experience() + u->max_experience();
		});
	set_column("unit_status", unit_list, [&](const auto& unit) {
		// Status
		if(unit->incapacitated()) {
			return "misc/petrified.png";
		}

		if(unit->poisoned()) {
			return "misc/poisoned.png";
		}

		if(unit->slowed()) {
			return "misc/slowed.png";
		}

		if(unit->invisible(unit->get_location(), false)) {
			return "misc/invisible.png";
		}

		return "";
	});
	set_column("unit_traits",  unit_list, [&](const auto& unit) {
		return utils::join(unit->trait_names(), ", ");
	}, true);

	return *this;
}

units_dialog& units_dialog::build_recruit_dialog(
	const std::vector<const unit_type*>& recruit_list,
	const team& team)
{
	set_title(_("Recruit Unit") + get_title_suffix(team.side()));
	set_ok_label(_("Recruit"));
	set_help_topic("recruit_and_recall");
	set_types(recruit_list);
	set_row_num(recruit_list.size());
	set_team(&team);
	hide_all_headers();
	set_column("unit_image", recruit_list, [&](const auto& recruit) {
		std::string image_string = recruit->image();
		image_string += "~RC(" + recruit->flag_rgb() + ">" + team.color() + ")";
		return image_string;
	});
	set_column("unit_details", recruit_list, [&](const auto& recruit) {
		return recruit->type_name() + unit_helper::format_cost_string(recruit->cost());
	}, true);
	return *this;
}

units_dialog& units_dialog::build_recall_dialog(
	const std::vector<unit_const_ptr>& recall_list,
	const team& team)
{
	int wb_gold = 0;
	if(resources::controller && resources::controller->get_whiteboard()) {
		wb::future_map future; // So gold takes into account planned spending
		wb_gold = resources::controller->get_whiteboard()->get_spent_gold_for(team.side());
	}

	// Lambda to check if a unit is recallable
	const auto& recallable = [&](const unit_const_ptr& unit) {
		// Note: Our callers apply [filter_recall], but leave it to us
		// to apply cost-based filtering.
		const int recall_cost =
			(unit->recall_cost() > -1 ? unit->recall_cost() : team.recall_cost());

		return (recall_cost <= team.gold() - wb_gold);
	};

	set_title(_("Recall Unit") + get_title_suffix(team.side()));
	set_ok_label(_("Recall"));
	set_help_topic("recruit_and_recall");
	set_units(recall_list);
	set_team(&team);
	set_row_num(recall_list.size());
	show_rename_option(true);
	show_dismiss_option(true);

	set_column("unit_image", recall_list, [&, recallable](const auto& unit) {
		std::string mods = unit->image_mods();
		if(unit->can_recruit()) { mods += "~BLIT(" + unit::leader_crown() + ")"; }
		for(const std::string& overlay : unit->overlays()) {
			mods += "~BLIT(" + overlay + ")";
		}
		if(!recallable(unit)) { mods += "~GS()"; }
		return unit->absolute_image() + mods;
	});

	set_column("unit_name", recall_list,
		[&, recallable](const auto& unit) {
			const std::string& name = !unit->name().empty() ? unit->name().str() : font::unicode_en_dash;
			return unit_helper::maybe_inactive(name, recallable(unit));
		},
		[&](const auto& unit) { return unit->name().str(); });

	set_column("unit_details", recall_list,
		[&, recallable](const auto& unit) {
			std::stringstream details;
			details << unit_helper::maybe_inactive(unit->type_name().str(), recallable(unit));
			details << unit_helper::format_cost_string(unit->recall_cost(), team.recall_cost());
			return details.str();
		},
		[&](const auto& unit) { return unit->type_name().str(); });

	set_column("unit_moves", recall_list,
		[&](const auto& unit) {
			return unit_helper::format_movement_string(unit->movement_left(), unit->total_movement());
		},
		[&](const auto& recall) { return recall->movement_left(); });

	set_column("unit_level", recall_list,
		[&, recallable](const auto& unit) {
			return unit_helper::format_level_string(unit->level(), recallable(unit));
		},
		[&](const auto& recall) {
			return std::tuple(recall->level(), -static_cast<int>(recall->experience_to_advance()));
		});

	set_column("unit_hp", recall_list,
		[&](const auto& unit) {
			return markup::span_color(unit->hp_color(), unit->hitpoints(), "/", unit->max_hitpoints());
		},
		[&](const auto& recall) { return recall->hitpoints(); });

	set_column("unit_xp", recall_list,
		[&](const auto& unit) {
			std::stringstream exp_str;
			if(unit->can_advance()) {
				exp_str << unit->experience() << "/" << unit->max_experience();
			} else {
				exp_str << font::unicode_en_dash;
			}
			return markup::span_color(unit->xp_color(), exp_str.str());
		},
		[&](const auto& recall) {
			// this allows 0/35, 0/100 etc to be sorted
			// also sorts 23/35 before 0/35, after which 0/100 comes
			return recall->experience() + recall->max_experience();
		});

	set_column("unit_traits", recall_list,
		[&, recallable](const auto& unit) {
			std::string traits;
			for(const std::string& trait : unit->trait_names()) {
				traits += (traits.empty() ? "" : "\n") + trait;
			}
			return unit_helper::maybe_inactive((!traits.empty() ? traits : font::unicode_en_dash), recallable(unit));
		},
		[&](const auto& recall) {
			return !recall->trait_names().empty() ? recall->trait_names().front().str() : "";
		});

	show_header("unit_status", false);

	set_tooltip_generator(recall_list, [&](const auto& recall) {
		if (!recallable(recall)) {
			// Just set the tooltip on every single element in this row.
			if(wb_gold > 0) {
				return _("This unit cannot be recalled because you will not have enough gold at this point in your plan.");
			} else {
				return _("This unit cannot be recalled because you do not have enough gold.");
			}
		} else {
			return std::string();
		}
	});

	return *this;
}



} // namespace dialogs
