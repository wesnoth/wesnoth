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
		visible_headers_.set();
		return *this;
	}

	units_dialog& hide_all_headers()
	{
		visible_headers_.reset();
		return *this;
	}

	units_dialog& show_header(const int col_num, const bool visible = true)
	{
		visible_headers_[col_num] = visible;
		return *this;
	}

	units_dialog& set_help_topic(const std::string& topic_id)
	{
		topic_id_ = topic_id;
		return *this;
	}

	units_dialog& set_team(team* team)
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

	units_dialog& set_row_num(const size_t row_num)
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
	units_dialog& set_column_generator(
		const std::string& id,
		const std::vector<Value>& container,
		const Generator& generator,
		const bool use_as_sorter = false)
	{
		column_generators_.try_emplace(id, [&container, generator](size_t index) { return generator(container[index]); });
		// use the generator function also as sorter function
		if (use_as_sorter) {
			find_widget<gui2::listbox>("recall_list").register_translatable_sorting_option(
				column_generators_.size()-1,
				[&container, generator](size_t index) { return generator(container[index]); });
		}
		return *this;
	}

	/**
	 * Sets the generator function for the tooltips
	 */
	units_dialog& set_tooltip_generator(const std::function<std::string(const int)>& generator)
	{
		tooltip_gen_ = generator;
		return *this;
	}

	template<typename Func>
	units_dialog& set_sorter(int col, const Func& sorter)
	{
		find_widget<gui2::listbox>("recall_list").register_sorting_option(col, sorter);
		return *this;
	}

	template<typename Value, typename Generator = std::function<std::string(const Value&)>>
	units_dialog& set_translatable_sorter(
		const int col, const std::vector<Value>& container, const Generator& generator)
	{
		find_widget<gui2::listbox>("recall_list").register_translatable_sorting_option(
			col, [&container, generator](size_t index) { return generator(container[index]); });
		return *this;
	}

	// } -------------------- BUILDERS -------------------- {
	units_dialog& build_create_dialog(const std::vector<const unit_type*>& types_list);

private:
	std::vector<const unit_type*> unit_type_list_;
	std::vector<unit_const_ptr> unit_list_;

	team* team_;

	int selected_index_;
	size_t row_num_;

	std::string title_;
	std::string ok_label_;
	std::string topic_id_;
	bool show_variation_grid_;
	bool show_gender_grid_;
	bool show_dismiss_;
	bool show_rename_;

	boost::dynamic_bitset<> visible_headers_;
	std::map<std::string, std::function<std::string(size_t)>> column_generators_;
	std::function<std::string(size_t)> tooltip_gen_;

	unit_race::GENDER gender_;
	std::string variation_;

	std::vector<std::string> filter_options_;
	std::vector<std::string> last_words_;
	group<unit_race::GENDER> gender_toggle_;

	/** Callbacks */
	void list_item_clicked();
	void filter_text_changed();
	void rename_unit();
	void dismiss_unit();

	void show_list(listbox& list);
	void show_help() const;
	void update_gender_and_variations(unit_preview_pane& preview, int selected_row);

	void gender_toggle_callback(const unit_race::GENDER val);
	void variation_menu_callback();

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;
	virtual void post_show() override;

};

} // namespace gui2::dialogs
