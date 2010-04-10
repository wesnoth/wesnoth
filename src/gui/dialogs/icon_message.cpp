/* $Id: icon_message.cpp 39955 2009-11-26 05:32:48Z fendrin $ */
/*
 Copyright (C) 2009 - 2010 by Fabian Mueller <fabianmueller5@gmx.de>
 Part of the Battle for Wesnoth Project http://www.wesnoth.org/

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License version 2
 or at your option any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY.

 See the COPYING file for more details.
 */

/**
 * @todo This function enables the wml markup for all items, but the interface
 * is a bit hacky. Especially the fiddling in the internals of the listbox is
 * ugly. There needs to be a clean interface to set whether a widget has a
 * markup and what kind of markup. These fixes will be post 1.6.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/icon_message.hpp"

//#include "gettext.hpp"
#include "font.hpp"
#include "unit.hpp"
#include "foreach.hpp"
#include "resources.hpp"
#include "game_display.hpp"
#include "map.hpp"
#include "help.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

namespace gui2 {

/*
 void trecall_message_::recall_pressed(twindow& window) {

 tlistbox& input_list = find_widget<tlistbox> (&window, "unit_list", false);

 game_menu_handler_->do_recall(chosen_unit_);
 input_list.remove_row(input_list.get_selected_row());

 if (unit_list_.empty()) {
 window.close();
 }

 unit_list_.erase(unit_list_.begin() + input_list.get_selected_row());
 chosen_unit_ = &unit_list_[0];
 update_unit_list(window);
 input_list.set_dirty();
 window.set_dirty();
 }
 */

void tunit_message_::profile_pressed() {
	const unit_type& t = *chosen_unit_->type();
	help::show_unit_help(*resources::screen, t.id(), t.hide_help());
}

void tunit_message_::help_pressed() {
	help::show_help(*resources::screen, "recruit_and_recall");
}

void trecall_message_::remove_pressed(CVideo& video, twindow& window) {
	tlistbox& unit_listbox =
			find_widget<tlistbox> (&window, "unit_list", false);

	//If the unit is of level > 1, or is close to advancing,
	//we warn the player about it
	std::stringstream message;
	if (chosen_unit_->loyal()) {
		message
				<< /*_*/("My lord, this unit is loyal and requires no upkeep! ")
				<< (chosen_unit_->gender() == unit_race::MALE ? /*_*/("Do you really want to dismiss him?")
						: /*_*/("Do you really want to dismiss her?"));
	} else if (chosen_unit_->level() > 1) {
		message
				<< /*_*/("My lord, this unit is an experienced one, having advanced levels! ")
				<< (chosen_unit_->gender() == unit_race::MALE ? /*_*/("Do you really want to dismiss him?")
						: /*_*/("Do you really want to dismiss her?"));

	} else if (chosen_unit_->experience() > chosen_unit_->max_experience() / 2) {
		message << /*_*/("My lord, this unit is close to advancing a level! ")
				<< (chosen_unit_->gender() == unit_race::MALE ? /*_*/("Do you really want to dismiss him?")
						: /*_*/("Do you really want to dismiss her?"));
	}

	if (!message.str().empty()) {
		const int res = gui2::show_message(video, "", message.str(),
				tmessage::yes_no_buttons, false, false);
		if (res == gui2::twindow::CANCEL) {
			return;
		}
	}

	game_menu_handler_->do_delete_recall(chosen_unit_);

	//	assert(unit_)
	int selected_row = unit_listbox.get_selected_row();

	unit_listbox.remove_row(selected_row);
	window.invalidate_layout();

	unit_list_.erase(unit_list_.begin() + selected_row);
	if (unit_list_.empty()) {
		window.close();
		return;
	}
	*chosen_unit_ = unit_list_[0];
	unit_listbox.select_row(0, true);
	update_unit_list(window);
	unit_listbox.set_dirty();
	window.set_dirty();
}

void tunit_message_::update_unit_list(twindow& window) {
	tlistbox& unit_listbox =
			find_widget<tlistbox> (&window, "unit_list", false);
	*chosen_unit_ = unit_list_[unit_listbox.get_selected_row()];

	if (resources::game_map->on_board(chosen_unit_->get_location())) {
		resources::screen->highlight_hex(chosen_unit_->get_location());
		//TODO is false better?
		resources::screen->scroll_to_tile(chosen_unit_->get_location(),
				game_display::SCROLL, true);
	} else {

		//TODO
		//		chosen_unit_->draw_report();
	}

	window.canvas(1).set_variable("portrait_image", variant(
			chosen_unit_->transparent()));
	window.set_dirty();
}

void toption_message_::pre_show(CVideo& video, twindow& window) {
	ticon_message_::pre_show(video, window);
	// Find the option list related fields.
	tlistbox& options = find_widget<tlistbox> (&window, "input_list", true);

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

	assert(!option_list_.empty());

	// Avoid negetive and 0 since item 0 is already selected.
	if (*chosen_option_ > 0 && static_cast<size_t> (*chosen_option_)
			< option_list_.size()) {
		options.select_row(*chosen_option_);
	}

	std::map<std::string, string_map> data;
	for (size_t i = 0; i < option_list_.size(); ++i) {
		std::string icon = "";
		std::string label = option_list_[i];
		std::string description = "";

		// Handle selection.
		if (!label.empty() && label[0] == '*') {
			// Number of items hasn't been increased yet so i is ok.
			*chosen_option_ = i;
			label.erase(0, 1);
		}

		// Handle the special case with an image.
		if (label.find('=') != std::string::npos) {
			// We always assume the first item is an image if not it's
			// still handled as an image. This is a hack but the code
			// should be rewritten anyway.
			if (label[0] == '&') {
				label.erase(0, 1);
			}

			std::vector<std::string> row_data = utils::split(label, '=', 0);
			label = "";

			assert(!row_data.empty());
			icon = row_data[0];

			if (row_data.size() > 1) {
				label = row_data[1];

				for (size_t j = 2; j < row_data.size(); ++j) {
					description += row_data[j];
					if (j + 1 < row_data.size()) {
						description += '=';
					}
				}
			}
		}

		// Add the data.
		data["icon"]["label"] = icon;
		data["label"]["label"] = label;
		data["label"]["use_markup"] = "true";
		data["description"]["label"] = description;
		data["description"]["use_markup"] = "true";
		options.add_row(data);
	}

	window.set_click_dismiss(false);
	window.set_escape_disabled(true);
}

void trecall_message_::pre_show(CVideo& video, twindow& window) {
	tunit_message_::pre_show(video, window);

	/*
	 tbutton* recall_button = find_widget<tbutton> (&window, "recall", false,
	 false);
	 recall_button->connect_signal_mouse_left_click(boost::bind(
	 &trecall_message_::recall_pressed, this, boost::ref(window)));
	 */

	tbutton* help_button = find_widget<tbutton> (&window, "help", false, false);
	help_button->connect_signal_mouse_left_click(boost::bind(
			&tunit_message_::help_pressed, this));

	tbutton* remove_button = find_widget<tbutton> (&window, "remove", false,
			false);
	remove_button->connect_signal_mouse_left_click(boost::bind(
			&trecall_message_::remove_pressed, this, boost::ref(video),
			boost::ref(window)));
}

void tunit_message_::pre_show(CVideo& video, twindow& window) {
	ticon_message_::pre_show(video, window);

	tbutton* profile_button = find_widget<tbutton> (&window, "profile", false,
			false);
	profile_button->connect_signal_mouse_left_click(boost::bind(
			&trecall_message_::profile_pressed, this));

	//		 Find the unit list related fields.
	//		 TODO what does the boolean has to be?
	tlistbox& units = find_widget<tlistbox> (&window, "unit_list", true);
	units.set_callback_value_change(dialog_callback<tunit_message_,
			&tunit_message_::update_unit_list> );

	std::map<std::string, string_map> data;
	for (size_t i = 0; i < unit_list_.size(); i++) {
		unit& unit = unit_list_[i];

		bool affordable = unit.cost() <= gold_;

		std::string unit_mod = affordable ? unit.image_mods() : "~GS()";
		std::string icon = (unit.absolute_image() + unit_mod);
		std::string type = unit.type_name();
		std::string name = unit.name();
		std::string traits = unit.traits_description();
		std::ostringstream level;
		std::ostringstream xp;
		xp << unit.experience() << "/" << unit.max_experience();

		// Show units of level (0=gray, 1 normal, 2 bold, 2+ bold&wbright)
		const int level_number = unit.level();
		//TODO enabe the switch construct after replacing the font::NORMAL_TEXT etc.
		//			switch(level_number)
		//			{
		//			case 0: level << "<150,150,150>";
		//			break;
		//			case 1: level << font::NORMAL_TEXT;
		//			break;
		//			case 2: level << font::BOLD_TEXT;
		//			break;
		//			default: level << font::BOLD_TEXT << "<255,255,255>";
		//			}
		level << level_number;

		std::string gold_color;

		gold_color = affordable ? "green" : "red";

		std::ostringstream cost;
		cost << "<span color=\"" << gold_color << "\">" << unit.cost() << " "
				<< /*_*/("Gold") << "</span>";

		// Add the data.
		data["cost"]["label"] = cost.str();
		data["cost"]["use_markup"] = "true";
		data["usage"]["label"] = unit.usage();
		data["icon"]["label"] = icon;
		data["icon"]["use_markup"] = "true";
		data["name"]["label"] = name;
		data["type"]["label"] = type;
		data["label"]["use_markup"] = "true";
		data["level"]["label"] = level.str();
		data["xp"]["label"] = xp.str();
		data["traits"]["label"] = unit.traits_description();
		units.add_row(data);

	}
	update_unit_list(window);
}

void trecruit_message_::pre_show(CVideo& video, twindow& window) {
	tunit_message_::pre_show(video, window);

	tbutton* help_button = find_widget<tbutton> (&window, "help", false, false);
	help_button->connect_signal_mouse_left_click(boost::bind(
			&tunit_message_::help_pressed, this));

	window.set_click_dismiss(false);
}

void tinput_message_::pre_show(CVideo& video, twindow& window) {
	ticon_message_::pre_show(video, window);
	ttext_box& input = find_widget<ttext_box> (&window, "input", true);
	tlabel& caption = find_widget<tlabel> (&window, "input_caption", false);

	caption.set_label(input_caption_);
	caption.set_use_markup(true);
	input.set_value(*input_text_);
	input.set_maximum_length(input_maximum_lenght_);
	window.keyboard_capture(&input);
	window.set_click_dismiss(false);
	window.set_escape_disabled(true);
}

void ticon_message_::pre_show(CVideo& video, twindow& window) {
	window.canvas(1).set_variable("portrait_image", variant(portrait_));
	window.canvas(1).set_variable("portrait_mirror", variant(mirror_));

	// Set the markup
	tlabel& title = find_widget<tlabel> (&window, "title", false);
	title.set_label(title_);
	title.set_use_markup(true);

	tcontrol& message = find_widget<tcontrol> (&window, "message", false);
	message.set_label(message_);
	message.set_use_markup(true);
	// The message label might not always be a scroll_label but the capturing
	// shouldn't hurt.
	window.keyboard_capture(&message);
	window.set_click_dismiss(true);
}

void tinput_message_::post_show(twindow& window) {
	*input_text_ = find_widget<ttext_box> (&window, "input", true).get_value();
}

void toption_message_::post_show(twindow& window) {
	*chosen_option_
			= find_widget<tlistbox> (&window, "input_list", true).get_selected_row();
}

void tunit_message_::post_show(twindow& window) {
	if (!unit_list_.empty()) {
		*chosen_unit_ = unit_list_[find_widget<tlistbox> (&window, "unit_list",
				true).get_selected_row()];
	}
}

void trecruit_message_::post_show(twindow& window) {
	tunit_message_::post_show(window);
	chosen_type_ = chosen_unit_->type_id();
}

REGISTER_WINDOW(icon_message_left)
REGISTER_WINDOW(icon_message_right)
REGISTER_WINDOW(option_message_left)
REGISTER_WINDOW(option_message_right)
REGISTER_WINDOW(input_message_left)
REGISTER_WINDOW(input_message_right)
REGISTER_WINDOW(recall_message_left)
REGISTER_WINDOW(recall_message_right)
REGISTER_WINDOW(recruit_message_right)
REGISTER_WINDOW(recruit_message_left)
REGISTER_WINDOW(unit_message_left)
REGISTER_WINDOW(unit_message_right)

//begin of the wrapper functions
int show_icon_message(const bool left_side, CVideo& video,
		const std::string& title, const std::string& message,
		const std::string& portrait, const bool mirror) {
	std::auto_ptr<ticon_message_> dlg;
	if (left_side) {
		dlg.reset(new ticon_message_left(title, message, portrait, mirror));
	} else {
		dlg.reset(new ticon_message_right(title, message, portrait, mirror));
	}
	assert(dlg.get());
	dlg->show(video);
	return dlg->get_retval();
}

int show_input_message(const bool left_side, CVideo& video,
		const std::string& title, const std::string& message,
		const std::string& portrait, const bool mirror,
		const std::string& input_caption, std::string* input_text,
		const unsigned maximum_length) {
	std::auto_ptr<tinput_message_> dlg;
	if (left_side) {
		dlg.reset(new tinput_message_left(title, message, portrait, mirror,
				input_caption, input_text, maximum_length));
	} else {
		dlg.reset(new tinput_message_right(title, message, portrait, mirror,
				input_caption, input_text, maximum_length));
	}
	assert(dlg.get());
	dlg->show(video);
	return dlg->get_retval();
}

int show_option_message(const bool left_side, CVideo& video,
		const std::string& title, const std::string& message,
		const std::string& portrait, const bool mirror, const std::vector<
				std::string>& option_list, int* chosen_option) {
	std::auto_ptr<toption_message_> dlg;
	if (left_side) {
		dlg.reset(new toption_message_left(title, message, portrait, mirror,
				option_list, chosen_option));
	} else {
		dlg.reset(new toption_message_right(title, message, portrait, mirror,
				option_list, chosen_option));
	}
	assert(dlg.get());
	dlg->show(video);
	return dlg->get_retval();
}

int show_unit_message(const bool left_side, CVideo& video,
		const std::string& title, const std::string& message,
		const std::string& portrait, const bool mirror,
		const std::vector<unit>& unit_list, unit* chosen_unit) {
	std::auto_ptr<tunit_message_> dlg;
	if (left_side) {
		dlg.reset(new tunit_message_left(title, message, portrait, mirror,
				unit_list, chosen_unit));
	} else {
		dlg.reset(new tunit_message_right(title, message, portrait, mirror,
				unit_list, chosen_unit));
	}
	assert(dlg.get());
	dlg->show(video);
	return dlg->get_retval();
}

int show_recall_message(const bool left_side, CVideo& video, const bool mirror,
		const std::vector<unit>& unit_list, unit* chosen_unit,
		events::menu_handler* game_menu_handler) {
	std::auto_ptr<trecall_message_> dlg;
	if (left_side) {
		dlg.reset(new trecall_message_left(mirror, unit_list, chosen_unit,
				game_menu_handler));
	} else {
		dlg.reset(new trecall_message_right(mirror, unit_list, chosen_unit,
				game_menu_handler));
	}
	assert(dlg.get());
	dlg->show(video);
	return dlg->get_retval();
}

int show_recruit_message(const bool left_side, CVideo& video,
		const bool mirror, const std::vector<const unit_type*> type_list,
		int side_num, int gold) {
	std::string chosen_type_id;
	std::auto_ptr<trecruit_message_> dlg;
	if (left_side) {
		dlg.reset(new trecruit_message_left(mirror, type_list, chosen_type_id,
				side_num, gold));
	} else {
		dlg.reset(new trecruit_message_right(mirror, type_list, chosen_type_id,
				side_num, gold));
	}
	assert(dlg.get());
	dlg->show(video);
	return dlg->get_retval();
}

} // namespace gui2
