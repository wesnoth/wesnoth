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

	void clear_variation()
	{
		variation_.clear();
	}

	// } --------------- BUILDER HELPERS -------------- {

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

	units_dialog& set_show_rename(bool show = true)
	{
		show_rename_ = show;
		return *this;
	}

	units_dialog& set_show_dismiss(bool show = true)
	{
		show_dismiss_ = show;
		return *this;
	}

	units_dialog& set_show_variations(bool show = true)
	{
		show_variations_ = show;
		return *this;
	}

	units_dialog& set_help_topic(const std::string& topic_id)
	{
		topic_id_ = topic_id;
		return *this;
	}

	units_dialog& set_row_num(const std::size_t row_num)
	{
		num_rows_ = row_num;
		return *this;
	}

	/**
	 * Registers an function which will fired on NOTIFY_MODIFIED dialog events.
	 *
	 * @param f     The update function. This should return a const reference to the
	 *              appropriate unit or unit type to display in the sidebar.
	 */
	template<typename Func>
	void on_modified(const Func& f)
	{
		connect_signal<event::NOTIFY_MODIFIED>([this, f](widget& w, auto&&...) {
			w.find_widget<unit_preview_pane>("unit_details").set_display_data(f(get_selected_index()));
		});
	}

	template<typename Value, typename Func>
	static auto make_index_wrapper(const std::vector<Value>& list, const Func& func)
	{
		return [&list, func](std::size_t index) { return func(list[index]); };
	}

	template<typename Sorter>
	void set_sorter(std::string_view id, const Sorter& sorter)
	{
		find_widget<gui2::listbox>("main_list").set_single_sorter(id, sorter);
	}

	/** Controls the sort behavior for functions returned by @ref make_column_builder. */
	enum class sort_type { generator, none };

	/**
	 * Creates a generator function which registers secondary generator and sorter
	 * functions for the list column associated with the given ID.
	 *
	 * @param list     A list of values to associate with the generator functions.
	 *                 These will be used to populate the dialog's listbox.
	 *
	 * @returns        A function which takes takes the following arguments:
	 *                 - The widget ID whose value will be generated.
	 *                 - The function which returns said widget's display value.
	 *                 - A @ref sort_type flag or a function used to order the list.
	 *                   If sort_type::generator is specified, the second argument
	 *                   will be used as the sorter.
	 */
	template<typename Value>
	auto make_column_builder(const std::vector<Value>& list)
	{
		return [this, &list](std::string_view id, const auto& generator, const auto& sorter) {
			auto wrapper = make_index_wrapper(list, generator);
			column_generators_.try_emplace(id, wrapper);

			if constexpr(std::is_invocable_v<decltype(sorter), const Value&>) {
				set_sorter(id, make_index_wrapper(list, sorter));
			} else {
				if(sorter != sort_type::generator) return;
				set_sorter(id, wrapper);
			}
		};
	}

	/** Sets the generator function for the tooltips. */
	template<typename Generator>
	units_dialog& set_tooltip_generator(const Generator& generator)
	{
		tooltip_gen_ = generator;
		return *this;
	}

	// } -------------------- BUILDERS -------------------- {

	static std::unique_ptr<units_dialog> build_create_dialog(const std::vector<const unit_type*>& types_list);
	static std::unique_ptr<units_dialog> build_recruit_dialog(const std::vector<const unit_type*>& recruit_list, const team& team);
	static std::unique_ptr<units_dialog> build_recall_dialog(std::vector<unit_const_ptr>& recall_list, const team& team);
	static std::unique_ptr<units_dialog> build_unit_list_dialog(std::vector<unit_const_ptr>& units_list);

private:
	int selected_index_;
	std::size_t num_rows_;

	std::string title_;
	std::string ok_label_;
	std::string cancel_label_;
	std::string topic_id_;

	bool show_rename_;
	bool show_dismiss_;
	bool show_variations_;

	std::map<std::string_view, std::function<std::string(std::size_t)>> column_generators_;
	std::function<std::string(std::size_t)> tooltip_gen_;

	unit_race::GENDER gender_;
	std::string variation_;

	std::vector<std::string> filter_options_;
	group<unit_race::GENDER> gender_toggle_;

	group<unit_race::GENDER>& get_toggle() {
		return gender_toggle_;
	}

	/** Callbacks */
	void list_item_clicked();
	void filter_text_changed(const std::string& text);

	// FIXME only thing needing team
	void dismiss_unit(std::vector<unit_const_ptr>& unit_list, const team& team);
	void rename_unit(std::vector<unit_const_ptr>& unit_list);

	void show_list(listbox& list);
	void show_help() const;

	void update_gender(const unit_race::GENDER val);
	void update_variation();

	virtual const std::string& window_id() const override;

	virtual void pre_show() override;
	virtual void post_show() override;
};

} // namespace gui2::dialogs
