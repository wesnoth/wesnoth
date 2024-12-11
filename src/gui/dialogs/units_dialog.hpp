/*
	Copyright (C) 2016 - 2024
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "gui/dialogs/modal_dialog.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/group.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "team.hpp"
#include "units/ptr.hpp"
#include "units/types.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <string>
#include <vector>

class team;

namespace gui2::dialogs
{

class units_dialog : public modal_dialog
{
public:
	units_dialog();

	int get_selected_index() const
	{
		return selected_index_;
	}

	bool is_selected() const
	{
		return selected_index_ != -1;
	}

	/** Gender choice from the user. */
	unit_race::GENDER gender() const
	{
		return gender_;
	}

	/** Variation choice from the user. */
	std::string variation() const
	{
		return variation_;
	}

	units_dialog& set_title(const std::string& title)
	{
		title_ = title;
		return *this;
	}

	units_dialog& set_ok_label(const std::string& ok_label)
	{
		ok_label_ = ok_label;
		return *this;
	}

	units_dialog& set_cancel_label(const std::string& cancel_label)
	{
		cancel_label_ = cancel_label;
		return *this;
	}

	units_dialog& show_variations(const bool show_variation)
	{
		show_variation_grid_ = show_variation;
		return *this;
	}

	units_dialog& show_gender(const bool show_gender)
	{
		show_gender_grid_ = show_gender;
		return *this;
	}

	units_dialog& show_dismiss_option(const bool show_dismiss)
	{
		show_dismiss_ = show_dismiss;
		return *this;
	}

	units_dialog& show_rename_option(const bool show_rename)
	{
		show_rename_ = show_rename;
		return *this;
	}

	units_dialog& show_all_headers()
	{
		show_header_ = true;
		return *this;
	}

	units_dialog& hide_all_headers()
	{
		show_header_ = false;
		return *this;
	}

	units_dialog& show_header(std::string_view id, const bool visible = true)
	{
		find_widget<toggle_button>(id).set_visible(
			visible ? widget::visibility::visible : widget::visibility::invisible);
		return *this;
	}

	units_dialog& set_help_topic(const std::string& topic_id)
	{
		topic_id_ = topic_id;
		return *this;
	}

	units_dialog& set_team(const team* team)
	{
		team_ = team;
		return *this;
	}

	units_dialog& set_types(const std::vector<const unit_type*>& types)
	{
		unit_type_list_ = types;
		return *this;
	}

	units_dialog& set_units(const std::vector<unit_const_ptr>& units)
	{
		unit_list_ = units;
		return *this;
	}

	units_dialog& set_row_num(const std::size_t row_num)
	{
		row_num_ = row_num;
		return *this;
	}

	/**
	 * Corresponding to each widget in the row with id 'id', there is a lambda that generates
	 * the corresponding label to set to that widget. This method sets those generator functions.
	 * The 'id's must be the same as those defined for the various widgets
	 * in the [list_definition] in the WML file.
	 */
	template<typename Value, typename Generator = std::function<std::string(const Value&)>>
	units_dialog& set_column(
		std::string_view id,
		const std::vector<Value>& container,
		const Generator& generator,
		const bool use_as_sorter = false)
	{
		column_generators_.try_emplace(id, [&container, generator](std::size_t index) { return generator(container[index]); });
		// use the generator function also as sorter function
		if (use_as_sorter) {
			find_widget<gui2::listbox>("main_list").set_single_sorter(
				id, [&container, generator](std::size_t index) { return generator(container[index]); });
		}
		return *this;
	}

	template<typename Value, typename Generator = std::function<std::string(const Value&)>
	, typename Sorter = std::function<std::string(const Value&)>>
	units_dialog& set_column(
		std::string_view id,
		const std::vector<Value>& container,
		const Generator& generator,
		const Sorter& sorter)
	{
		column_generators_.try_emplace(id, [&container, generator](std::size_t index) { return generator(container[index]); });
		find_widget<gui2::listbox>("main_list").set_single_sorter(
			id, [&container, sorter](std::size_t index) { return sorter(container[index]); });
		return *this;
	}

	/**
	 * Sets the generator function for the tooltips
	 */
	template<typename Value, typename Generator = std::function<std::string(const Value&)>>
	units_dialog& set_tooltip_generator(
		const std::vector<Value>& container, const Generator& generator)
	{
		tooltip_gen_ = [&container, generator](std::size_t index) { return generator(container[index]); };
		return *this;
	}

	// } -------------------- BUILDERS -------------------- {
	units_dialog& build_create_dialog(const std::vector<const unit_type*>& types_list);
	units_dialog& build_unit_list_dialog(const std::vector<unit_const_ptr>& units_list);
	units_dialog& build_recruit_dialog(
		const std::vector<const unit_type*>& recruit_list, const team& team);
	units_dialog& build_recall_dialog(
		const std::vector<unit_const_ptr>& recall_list,	const team& team);

private:
	std::vector<const unit_type*> unit_type_list_;
	std::vector<unit_const_ptr> unit_list_;

	const team* team_;

	int selected_index_;
	std::size_t row_num_;

	std::string title_;
	std::string ok_label_;
	std::string cancel_label_;
	std::string topic_id_;
	bool show_header_;
	bool show_variation_grid_;
	bool show_gender_grid_;
	bool show_dismiss_;
	bool show_rename_;

	std::map<std::string_view, std::function<std::string(std::size_t)>> column_generators_;
	std::function<std::string(std::size_t)> tooltip_gen_;

	unit_race::GENDER gender_;
	std::string variation_;

	std::vector<std::string> filter_options_;
	std::vector<std::string> last_words_;
	group<unit_race::GENDER> gender_toggle_;

	/** Callbacks */
	void list_item_clicked();
	void filter_text_changed();
	void rename_unit();

	// FIXME only thing needing team
	void dismiss_unit(const team& team);

	void show_list(listbox& list);
	void show_help() const;

	unit_type update_gender_and_variations(int selected_row);

	void gender_toggle_callback(const unit_race::GENDER val);
	void variation_menu_callback();

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;
	virtual void post_show() override;

};

struct unit_view_builder {

};

} // namespace gui2::dialogs
