/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#include "gui/dialogs/message.hpp"

#include "gui/widgets/button.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/window.hpp"

namespace gui2 {

void tmessage::pre_show(CVideo& /*video*/, twindow& window)
{
	if(!title_.empty()) {
		tlabel* title = 
			dynamic_cast<tlabel*>(window.find_widget("title", false));
		VALIDATE(title, missing_widget("title"));

		title->set_label(title_);
	}

	if(!image_.empty()) {
		timage* image = 
			dynamic_cast<timage*>(window.find_widget("image", false));
		VALIDATE(image, missing_widget("image"));

		image->set_label(image_);
	}

	tcontrol* label = 
		dynamic_cast<tcontrol*>(window.find_widget("label", false));
	VALIDATE(label, missing_widget("label"));

	label->set_label(message_);

	if(auto_close_) {
		/*
		 * Hide the buttton and do the layout, if window.does_easy_close() is
		 * false the scroll_label has a scrollbar so we need to show the
		 * button. When the button is hidden the text for the label is bigger
		 * and thus not need a scrollbar. Also when the button is visible
		 * easy_close will always return false.
		 */
		/** @todo The space for invisible items is always reserved. Look about
		 * how to change that. (Maybe get_best_size() in twidget can do that by
		 * returning 0,0 when called upon invisible items. Or the tgrid::tchild
		 * should do that since an item with 0,0 might get a border.)
		 */
		tbutton* button = 
			dynamic_cast<tbutton*>(window.find_widget("ok", false));
		VALIDATE(button, missing_widget("ok"));

		button->set_visible(false);

		window.layout();

		if(! window.does_easy_close()) {
			button->set_visible();
		}
	}
}

twindow tmessage::build_window(CVideo& video)
{
	return build(video, get_id(MESSAGE));
}

/** @todo the caption is ignored. */
void show_message(CVideo& video, const std::string& title, 
	const std::string& message, const std::string& /*button_caption*/,
	const bool auto_close)
{
	tmessage(title, message, auto_close).show(video);
}

} // namespace gui2

