/*
	Copyright (C) 2008 - 2024
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

#include "gui/dialogs/wml_message.hpp"

#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2::dialogs
{

void wml_message_base::set_input(
	const std::string& caption,
	std::string* text,
	const unsigned maximum_length)
{
	assert(text);

	has_input_ = true;
	input_caption_ = caption;
	input_text_ = text;
	input_maximum_length_ = maximum_length;
}

void wml_message_base::set_option_list(
	const std::vector<wml_message_option>& option_list,
	int* chosen_option)
{
	assert(!option_list.empty());
	assert(chosen_option);

	option_list_ = option_list;
	chosen_option_ = chosen_option;
}

/**
 * @todo This function enables the wml markup for all items, but the interface
 * is a bit hacky. Especially the fiddling in the internals of the listbox is
 * ugly. There needs to be a clean interface to set whether a widget has a
 * markup and what kind of markup. These fixes will be post 1.6.
 */
void wml_message_base::pre_show()
{
	get_canvas(1).set_variable("portrait_image", wfl::variant(portrait_));
	get_canvas(1).set_variable("portrait_mirror", wfl::variant(mirror_));

	// Set the markup
	label& title = find_widget<label>("title");
	title.set_label(title_);
	title.set_use_markup(true);
	title.set_can_wrap(true);

	styled_widget& message = find_widget<styled_widget>("message");
	message.set_label(message_);
	message.set_use_markup(true);
	// The message label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	keyboard_capture(&message);

	// Find the input box related fields.
	label& caption = find_widget<label>("input_caption");
	text_box& input = find_widget<text_box>("input", true);

	if(has_input_) {
		caption.set_label(input_caption_);
		caption.set_use_markup(true);
		input.set_value(*input_text_);
		input.set_maximum_length(input_maximum_length_);
		keyboard_capture(&input);
		set_click_dismiss(false);
		set_escape_disabled(true);
	} else {
		caption.set_visible(widget::visibility::invisible);
		input.set_visible(widget::visibility::invisible);
	}

	// Find the option list related fields.
	listbox& options = find_widget<listbox>("input_list", true);

	if(!option_list_.empty()) {
		widget_data data;
		for(const wml_message_option& item : option_list_) {
			// Add the data.
			data["icon"]["label"] = item.image();
			data["label"]["label"] = item.label();
			data["label"]["use_markup"] = "true";
			data["description"]["label"] = item.description();
			data["description"]["use_markup"] = "true";
			options.add_row(data);
		}

		// Avoid negative and 0 since item 0 is already selected.
		if(*chosen_option_ > 0 && static_cast<std::size_t>(*chosen_option_)
								  < option_list_.size()) {

			options.select_row(*chosen_option_);
		}

		if(!has_input_) {
			keyboard_capture(&options);
			set_click_dismiss(false);
			set_escape_disabled(true);
		} else {
			add_to_keyboard_chain(&options);
			// click_dismiss has been disabled due to the input.
		}
	} else {
		options.set_visible(widget::visibility::invisible);
	}
	set_click_dismiss(!has_input_ && option_list_.empty());
}

void wml_message_base::post_show()
{
	if(has_input_) {
		*input_text_
				= find_widget<text_box>("input", true).get_value();
	}

	if(!option_list_.empty()) {
		*chosen_option_ = find_widget<listbox>("input_list", true)
								  .get_selected_row();
	}
}

void wml_message_double::pre_show()
{
	wml_message_left::pre_show();
	get_canvas(1).set_variable("second_portrait_image", wfl::variant(second_portrait_));
	get_canvas(1).set_variable("second_portrait_mirror", wfl::variant(second_mirror_));
}

REGISTER_DIALOG(wml_message_left)

REGISTER_DIALOG(wml_message_right)

REGISTER_DIALOG(wml_message_double)

int show_wml_message(const std::string& title,
					 const std::string& message,
					 const wml_message_portrait* left,
					 const wml_message_portrait* right,
					 const wml_message_options& options,
					 const wml_message_input& input)
{
	std::shared_ptr<wml_message_base> dlg;
	if(left && !right) {
		dlg.reset(new wml_message_left(title, message, left->portrait, left->mirror));
	} else if(!left && right) {
		dlg.reset(new wml_message_right(title, message, right->portrait, right->mirror));
	} else if(right && left) {
		dlg.reset(new wml_message_double(title, message, left->portrait, left->mirror, right->portrait, right->mirror));
	}
	assert(dlg.get());

	if(input.text_input_was_specified) {
		dlg->set_input(input.caption, &input.text, input.maximum_length);
	}

	if(!options.option_list.empty()) {
		dlg->set_option_list(options.option_list, &options.chosen_option);
	}

	dlg->show();
	return dlg->get_retval();
}

} // namespace dialogs
