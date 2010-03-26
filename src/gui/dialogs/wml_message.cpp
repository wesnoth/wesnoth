/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/wml_message.hpp"

#include "foreach.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

void twml_message_::set_input(const std::string& caption,
		std::string* text, const unsigned maximum_length)
{
	assert(text);

	has_input_ = true;
	input_caption_ = caption;
	input_text_ = text;
	input_maximum_lenght_ = maximum_length;
}

void twml_message_::set_option_list(
		const std::vector<std::string>& option_list, int* choosen_option)
{
	assert(!option_list.empty());
	assert(choosen_option);

	option_list_ = option_list;
	chosen_option_ = choosen_option;
}

/**
 * @todo This function enables the wml markup for all items, but the interface
 * is a bit hacky. Especially the fiddling in the internals of the listbox is
 * ugly. There needs to be a clean interface to set whether a widget has a
 * markup and what kind of markup. These fixes will be post 1.6.
 */
void twml_message_::pre_show(CVideo& /*video*/, twindow& window)
{
	window.canvas(1).set_variable("portrait_image", variant(portrait_));
	window.canvas(1).set_variable("portrait_mirror", variant(mirror_));

	// Set the markup
	tlabel& title = find_widget<tlabel>(&window, "title", false);
	title.set_label(title_);
	title.set_use_markup(true);

	tcontrol& message = find_widget<tcontrol>(&window, "message", false);
	message.set_label(message_);
	message.set_use_markup(true);
	// The message label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	window.keyboard_capture(&message);

	// Find the input box related fields.
	tlabel& caption = find_widget<tlabel>(&window, "input_caption", false);
	ttext_box& input = find_widget<ttext_box>(&window, "input", true);

	if(has_input_) {
		caption.set_label(input_caption_);
		caption.set_use_markup(true);
		input.set_value(*input_text_);
		input.set_maximum_length(input_maximum_lenght_);
		window.keyboard_capture(&input);
		window.set_click_dismiss(false);
		window.set_escape_disabled(true);
	} else {
		caption.set_visible(twidget::INVISIBLE);
		input.set_visible(twidget::INVISIBLE);
	}

	// Find the option list related fields.
	tlistbox& options = find_widget<tlistbox>(&window, "input_list", true);

	/*
	 * The options have some special markup:
	 * A line starting with a * means select that line.
	 * A line starting with a & means more special markup.
	 * - The part until the = is the name of an image.
	 * - The part until the second = is the first column.
	 * - The rest is the third column (the wiki only specifies two columns
	 *   so only implement two of them).
	 */
	/**
	 * @todo This syntax looks like a bad hack, it would be nice to write
	 * a new syntax which doesn't use those hacks (also avoids the problem
	 * with special meanings for certain characters.
	 */
	if(!option_list_.empty()) {
		std::map<std::string, string_map> data;
		for(size_t i = 0; i < option_list_.size(); ++i) {
			std::string icon;
			std::string label = option_list_[i];
			std::string description;

			// Handle selection.
			if(!label.empty() && label[0] == '*') {
				// Number of items hasn't been increased yet so i is ok.
				*chosen_option_ = i;
				label.erase(0, 1);
			}

			// Handle the special case with an image.
			std::string::size_type pos = label.find('=');
			if (pos != std::string::npos && (label[0] == '&' || pos == 0)) {
				if (pos) icon = label.substr(1, pos - 1);
				label.erase(0, pos + 1);
			}

			// Search for an '=' symbol that is not inside markup.
			std::string::size_type prev = 0;
			bool open = false;
			while ((pos = label.find('=', prev)) != std::string::npos) {
				for (std::string::size_type i = prev; i != pos; ++i) {
					switch (label[i]) {
					case '<': open = true; break;
					case '>': open = false; break;
					}
				}
				if (!open) break;
				prev = pos + 1;
			}
			if (pos != std::string::npos) {
				description = label.substr(pos + 1);
				label.erase(pos);
			}

			// Add the data.
			data["icon"]["label"] = icon;
			data["label"]["label"] = label;
			data["label"]["use_markup"] = "true";
			data["description"]["label"] = description;
			data["description"]["use_markup"] = "true";
			options.add_row(data);
		}

		// Avoid negetive and 0 since item 0 is already selected.
		if(*chosen_option_ > 0
				&& static_cast<size_t>(*chosen_option_)
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
		options.set_visible(twidget::INVISIBLE);
	}
	window.set_click_dismiss(!has_input_ && option_list_.empty());
}

void twml_message_::post_show(twindow& window)
{
	if(has_input_) {
		*input_text_ =
				find_widget<ttext_box>(&window, "input", true).get_value();
	}

	if(!option_list_.empty()) {
		*chosen_option_ = find_widget<tlistbox>(
				&window, "input_list", true).get_selected_row();
	}
}

REGISTER_WINDOW(wml_message_left)

REGISTER_WINDOW(wml_message_right)

int show_wml_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
		, const bool has_input
		, const std::string& input_caption
		, std::string* input_text
		, const unsigned maximum_length
		, const std::vector<std::string>& option_list
		, int* chosen_option)
{
	std::auto_ptr<twml_message_> dlg;
	if(left_side) {
		dlg.reset(new twml_message_left(title, message, portrait, mirror));
	} else {
		dlg.reset(new twml_message_right(title, message, portrait, mirror));
	}
	assert(dlg.get());

	if(has_input) {
		dlg->set_input(input_caption, input_text, maximum_length);
	}

	if(!option_list.empty()) {
		dlg->set_option_list(option_list, chosen_option);
	}

	dlg->show(video);
	return dlg->get_retval();
}

} // namespace gui2

