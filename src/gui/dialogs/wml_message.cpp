/* $Id$ */
/*
   copyright (c) 2008 - 2009 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/wml_message.hpp"

#include "foreach.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

void twml_message_::set_input(const std::string& caption,
		std::string* text, const unsigned maximum_length)
{
	assert(!caption.empty());
	assert(text);

	input_caption_ = caption;
	input_text_ = text;
	input_maximum_lenght_ = maximum_length;

	set_auto_close(false);
}

void twml_message_::set_option_list(
		const std::vector<std::string>& option_list, int* choosen_option)
{
	assert(!option_list.empty());
	assert(choosen_option);

	option_list_ = option_list;
	chosen_option_ = choosen_option;

	set_auto_close(false);
}

void twml_message_::pre_show(CVideo& video, twindow& window)
{
	// Inherited.
	tmessage::pre_show(video, window);

	window.canvas(1).set_variable("portrait_image", variant(portrait_));
	window.canvas(1).set_variable("portrait_mirror", variant(mirror_));

	// Find the input box related fields.
	tlabel* caption = dynamic_cast<tlabel*>(
			window.find_widget("input_caption", false));
	VALIDATE(caption, missing_widget("input_caption"));

	ttext_box* input = dynamic_cast<ttext_box*>(
			 window.find_widget("input", true));
	VALIDATE(input, missing_widget("input"));

	if(has_input()) {
		caption->set_label(input_caption_);
		input->set_value(*input_text_);
		input->set_maximum_length(input_maximum_lenght_);
		window.keyboard_capture(input);
		window.set_easy_close(false);
	} else {
		caption->set_visible(twidget::INVISIBLE);
		input->set_visible(twidget::INVISIBLE);
	}

	// Find the option list related fields.
	tlistbox* options = dynamic_cast<tlistbox*>(
			window.find_widget("input_list", true));
	VALIDATE(options, missing_widget("input_list"));

	if(!option_list_.empty()) {
		string_map row_data;
		foreach(const std::string& option, option_list_) {
			row_data["label"] = option;
			options->add_row(row_data);
		}
		// Avoid negetive and 0 since item 0 is already selected.
		if(*chosen_option_ > 0 
				&& static_cast<size_t>(*chosen_option_) 
				< option_list_.size()) {

			options->select_row(*chosen_option_);
		}

		if(!has_input()) {
			window.keyboard_capture(options);
			window.set_easy_close(false); 
		} else {
			window.add_to_keyboard_chain(options);
			// easy_close has been disabled due to the input.
		}
	} else {
		options->set_visible(twidget::INVISIBLE);
	}
}

void twml_message_::post_show(twindow& window)
{
	if(has_input()) {
		ttext_box* input = dynamic_cast<ttext_box*>(
				 window.find_widget("input", true));
		VALIDATE(input, missing_widget("input"));

		*input_text_ = input->get_value();
	}

	if(!option_list_.empty()) {
		tlistbox* options = dynamic_cast<tlistbox*>(
				window.find_widget("input_list", true));
		VALIDATE(options, missing_widget("input_list"));

		*chosen_option_ = options->get_selected_row();
	}
}

twindow* twml_message_left::build_window(CVideo& video)
{
	return build(video, get_id(WML_MESSAGE_LEFT));
}

twindow* twml_message_right::build_window(CVideo& video)
{
	return build(video, get_id(WML_MESSAGE_RIGHT));
}

int show_wml_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
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

	if(!input_caption.empty()) {
		dlg->set_input(input_caption, input_text, maximum_length);
	}

	if(!option_list.empty()) {
		dlg->set_option_list(option_list, chosen_option);
	}
	
	dlg->show(video);
	return dlg->get_retval();
}

} // namespace gui2

