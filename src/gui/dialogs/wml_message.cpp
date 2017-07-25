/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

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

#include "gui/auxiliary/find_widget.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2
{
namespace dialogs
{

void wml_message_base::set_input(const std::string& caption,
							  std::string* text,
							  const unsigned maximum_length)
{
	assert(text);

	has_input_ = true;
	input_caption_ = caption;
	input_text_ = text;
	input_maximum_length_ = maximum_length;
}

void wml_message_base::set_option_list(const std::vector<wml_message_option>& option_list,
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
void wml_message_base::pre_show(window& window)
{
	set_restore(true);

	window.get_canvas(1).set_variable("portrait_image", wfl::variant(portrait_));
	window.get_canvas(1).set_variable("portrait_mirror", wfl::variant(mirror_));

	// Set the markup
	label& title = find_widget<label>(&window, "title", false);
	title.set_label(title_);
	title.set_use_markup(true);
	title.set_can_wrap(true);

	styled_widget& message = find_widget<styled_widget>(&window, "message", false);
	message.set_label(message_);
	message.set_use_markup(true);
	// The message label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	window.keyboard_capture(&message);

	// Find the input box related fields.
	label& caption = find_widget<label>(&window, "input_caption", false);
	text_box& input = find_widget<text_box>(&window, "input", true);

	if(has_input_) {
		caption.set_label(input_caption_);
		caption.set_use_markup(true);
		input.set_value(*input_text_);
		input.set_maximum_length(input_maximum_length_);
		window.keyboard_capture(&input);
		window.set_click_dismiss(false);
		window.set_escape_disabled(true);
	} else {
		caption.set_visible(widget::visibility::invisible);
		input.set_visible(widget::visibility::invisible);
	}

	// Find the option list related fields.
	listbox& options = find_widget<listbox>(&window, "input_list", true);

	if(!option_list_.empty()) {
		std::map<std::string, string_map> data;
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
		if(*chosen_option_ > 0 && static_cast<size_t>(*chosen_option_)
								  < option_list_.size()) {

			options.select_row(*chosen_option_);
		}

		if(!has_input_) {
			window.keyboard_capture(&options);
			window.set_click_dismiss(false);
			window.set_escape_disabled(true);
		} else {
			window.add_to_keyboard_chain(&options);
			// click_dismiss has been disabled due to the input.
		}
	} else {
		options.set_visible(widget::visibility::invisible);
	}
	window.set_click_dismiss(!has_input_ && option_list_.empty());
}

void wml_message_base::post_show(window& window)
{
	if(has_input_) {
		*input_text_
				= find_widget<text_box>(&window, "input", true).get_value();
	}

	if(!option_list_.empty()) {
		*chosen_option_ = find_widget<listbox>(&window, "input_list", true)
								  .get_selected_row();
	}
}

void wml_message_double::pre_show(window& window)
{
	wml_message_left::pre_show(window);
	window.get_canvas(1).set_variable("second_portrait_image", wfl::variant(second_portrait_));
	window.get_canvas(1).set_variable("second_portrait_mirror", wfl::variant(second_mirror_));
}

REGISTER_DIALOG(wml_message_left)

REGISTER_DIALOG(wml_message_right)

REGISTER_DIALOG(wml_message_double)

int show_wml_message(CVideo& video,
					 const std::string& title,
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

	dlg->show(video);
	return dlg->get_retval();
}

} // namespace dialogs
} // namespace gui2
