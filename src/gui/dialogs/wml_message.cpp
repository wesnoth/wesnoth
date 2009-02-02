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
	assert(text);

	has_input_ = true;
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

/**
 * @todo This function enables the wml markup for all items, but the interface
 * is a bit hacky. Especially the fiddling in the internals of the listbox is
 * ugly. There needs to be a clean interface to set whether a widget has a
 * markup and what kind of markup. These fixes will be post 1.6.
 */
void twml_message_::pre_show(CVideo& video, twindow& window)
{
	// Inherited.
	tmessage::pre_show(video, window);

	window.canvas(1).set_variable("portrait_image", variant(portrait_));
	window.canvas(1).set_variable("portrait_mirror", variant(mirror_));

	// Set the markup
	tlabel* title =
			dynamic_cast<tlabel*>(window.find_widget("title", false));
	assert(title);
	title->set_markup_mode(tcontrol::WML_MARKUP);

	tcontrol* label =
			dynamic_cast<tcontrol*>(window.find_widget("label", false));
	assert(label);
	label->set_markup_mode(tcontrol::WML_MARKUP);

	// Find the input box related fields.
	tlabel* caption = dynamic_cast<tlabel*>(
			window.find_widget("input_caption", false));
	VALIDATE(caption, missing_widget("input_caption"));

	ttext_box* input = dynamic_cast<ttext_box*>(
			 window.find_widget("input", true));
	VALIDATE(input, missing_widget("input"));

	if(has_input_) {
		caption->set_label(input_caption_);
		caption->set_markup_mode(tcontrol::WML_MARKUP);
		input->set_value(*input_text_);
		input->set_maximum_length(input_maximum_lenght_);
		window.keyboard_capture(input);
		window.set_easy_close(false);
		window.set_escape_disabled(true);
	} else {
		caption->set_visible(twidget::INVISIBLE);
		input->set_visible(twidget::INVISIBLE);
	}

	// Find the option list related fields.
	tlistbox* options = dynamic_cast<tlistbox*>(
			window.find_widget("input_list", true));
	VALIDATE(options, missing_widget("input_list"));

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
			std::string icon = "";
			std::string label = option_list_[i];
			std::string description = "";

			// Handle selection.
			if(!label.empty() && label[0] == '*') {
				// Number of items hasn't been increased yet so i is ok.
				*chosen_option_ = i;
				label.erase(0, 1);
			}

			// Handle the special case with an image.
			if(!label.empty() && label[0] == '&') {
				label.erase(0, 1);

				std::vector<std::string> row_data = utils::split(label, '=');
				label = "";

				assert(!row_data.empty());
				icon = row_data[0];

				if(row_data.size() > 0) {
					label = row_data[1];

					for(size_t j = 2; j < row_data.size(); ++j) {
						description += row_data[j];
						if(j +1  < row_data.size()) {
							description += '=';
						}
					}
				}
			}

			// Add the data.
			data["icon"]["label"] = icon;
			data["label"]["label"] = label;
			data["description"]["label"] = description;
			options->add_row(data);

			// Set the markup flag.
			assert(options->generator_);			
			tgrid& grid = options->generator_->get_item(i);

			tcontrol* control = dynamic_cast<tcontrol*>(
					grid.find_widget("label", false));
			assert(control);
			control->set_markup_mode(tcontrol::WML_MARKUP);

			control = dynamic_cast<tcontrol*>(
					grid.find_widget("description", false));
			assert(control);
			control->set_markup_mode(tcontrol::WML_MARKUP);
		}

		// Avoid negetive and 0 since item 0 is already selected.
		if(*chosen_option_ > 0 
				&& static_cast<size_t>(*chosen_option_) 
				< option_list_.size()) {

			options->select_row(*chosen_option_);
		}

		if(!has_input_) {
			window.keyboard_capture(options);
			window.set_easy_close(false); 
			window.set_escape_disabled(true);
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
	if(has_input_) {
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

