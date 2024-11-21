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

#pragma once

#include "gui/dialogs/modal_dialog.hpp"

namespace gui2::dialogs
{

/**
 *  Helper class for message options
 */
class wml_message_option {
public:
	explicit wml_message_option(std::string label, std::string description = "", std::string image = "")
		: label_(label)
		, description_(description)
		, image_(image)
	{}
	std::string label() const {return label_;}
	std::string description() const {return description_;}
	std::string image() const {return image_;}
private:
	std::string label_, description_, image_;
};

/**
 * Base class for the wml generated messages.
 *
 * We have a separate sub class for left and right images.
 */
class wml_message_base : public modal_dialog
{
public:
	wml_message_base(
			const std::string& window_id,
			const std::string& title,
			const std::string& message,
			const std::string& portrait,
			const bool mirror)
		: modal_dialog(window_id)
		, title_(title)
		, image_("")
		, message_(message)
		, portrait_(portrait)
		, mirror_(mirror)
		, has_input_(false)
		, input_caption_("")
		, input_text_(nullptr)
		, input_maximum_length_(0)
		, option_list_()
		, chosen_option_(nullptr)
	{
	}

	/**
	 * Sets the input text variables.
	 *
	 * @param caption             The caption for the label.
	 * @param [in,out] text       The initial text, after showing the final
	 *                            text.
	 * @param maximum_length      The maximum length of the text.
	 */
	void set_input(const std::string& caption,
				   std::string* text,
				   const unsigned maximum_length);
	/**
	 * Sets the option list
	 *
	 * @param option_list            The list of options to display.
	 * @param [in,out] chosen_option Pointer to the index of the initially
	 *                               selected option; after showing, the
	 *                               chosen option.
	 */
	void set_option_list(const std::vector<wml_message_option>& option_list,
						 int* chosen_option);

private:
	/** The title for the dialog. */
	std::string title_;

	/**
	 * The image which is shown in the dialog.
	 *
	 * This image can be an icon or portrait or any other image.
	 */
	std::string image_;

	/** The message to show to the user. */
	std::string message_;
	/** Filename of the portrait. */
	std::string portrait_;

	/** Mirror the portrait? */
	bool mirror_;

	/** Do we need to show an input box? */
	bool has_input_;

	/** The caption to show for the input text. */
	std::string input_caption_;

	/** The text input. */
	std::string* input_text_;

	/** The maximum length of the input text. */
	unsigned input_maximum_length_;

	/** The list of options the user can choose. */
	std::vector<wml_message_option> option_list_;

	/** The chosen option. */
	int* chosen_option_;

protected:
	virtual void pre_show() override;

private:
	virtual void post_show() override;
};

/** Shows a dialog with the portrait on the left side. */
class wml_message_left : public wml_message_base
{
public:
	wml_message_left(const std::string& title,
					  const std::string& message,
					  const std::string& portrait,
					  const bool mirror)
		: wml_message_base(window_id(), title, message, portrait, mirror)
	{
	}

private:
	virtual const std::string& window_id() const override;
};

/** Shows a dialog with the portrait on the right side. */
class wml_message_right : public wml_message_base
{
public:
	wml_message_right(const std::string& title,
					   const std::string& message,
					   const std::string& portrait,
					   const bool mirror)
		: wml_message_base(window_id(), title, message, portrait, mirror)
	{
	}

private:
	virtual const std::string& window_id() const override;
};

/** Shows a dialog with two portraits, one on each side. */
class wml_message_double : public wml_message_left
{
public:
	wml_message_double(const std::string& title,
					   const std::string& message,
					   const std::string& portrait,
					   const bool mirror,
					   const std::string& second_portrait,
					   const bool second_mirror)
		: wml_message_left(title, message, portrait, mirror)
		, second_portrait_(second_portrait)
		, second_mirror_(second_mirror)
	{
	}

private:
	virtual const std::string& window_id() const override;

	virtual void pre_show() override;

	std::string second_portrait_;

	bool second_mirror_;
};

/**
 * Parameter pack for message list input options
 */
struct wml_message_options
{
	/** A list of options to select in the dialog. */
	std::vector<wml_message_option> option_list;
	/**
	 * The initially chosen option.
	 * Will be set to the chosen option when the dialog closes.
	 */
	mutable int chosen_option;
};

/**
 * Parameter pack for message text input options
 */
struct wml_message_input
{
	/**
	 * The caption for the optional input text box.
	 * If empty, there is no input box.
	 */
	std::string caption;
	/**
	 * The initial text value.
	 * Will be set to the result.
	 */
	mutable std::string text;
	/** The maximum length of the text. */
	unsigned maximum_length;
	/** True when [text_input] appeared in [message] */
	bool text_input_was_specified;
};

/**
 * Parameter pack for message portrait
 */
struct wml_message_portrait
{
	/** Filename of the portrait. */
	std::string portrait;
	/** Does the portrait need to be mirrored? */
	bool mirror;
};

/**
 *  Helper function to show a portrait.
 *
 *  @param title                  The title of the dialog.
 *  @param message                The message to show.
 *  @param left                   Portrait to show on the left.
 *  @param right                  Portrait to show on the right.
 *  @param options                Options to offer.
 *  @param input                  Info on text input.
 */
int show_wml_message(const std::string& title,
					 const std::string& message,
					 const wml_message_portrait* left,
					 const wml_message_portrait* right,
					 const wml_message_options& options,
					 const wml_message_input& input);

} // namespace dialogs
