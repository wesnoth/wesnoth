/* $Id: icon_message.hpp 39955 2009-11-26 05:32:48Z fendrin $ */
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

#ifndef GUI_DIALOGS_ICON_MESSAGE_HPP_INCLUDED
#define GUI_DIALOGS_ICON_MESSAGE_HPP_INCLUDED

//#include "gettext.hpp"
#include "gui/dialogs/dialog.hpp"
#include "unit.hpp"
#include "menu_events.hpp"
//TODO remove after the constructor of unit moved to the cpp file.
#include "resources.hpp"

namespace gui2 {

/**
 * Base class for messages that also display an image or portrait.
 *
 * We have a separate sub class for left and right images.
 */
class ticon_message_
	: public tdialog
{
public:
	ticon_message_(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror)
		: title_(title)
		, image_("")
		, message_(message)
		, portrait_(portrait)
		, mirror_(mirror)
	{
	}

protected:

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

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

	/**
	 * Inherited from tdialog.
	 *
	 * The subclasses need to implement the left or right definition.
	 */
	twindow* build_window(CVideo& /*video*/) = 0;

};

/** Shows a dialog with the portrait on the left side. */
class ticon_message_left : public ticon_message_
{
public:
	ticon_message_left(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror)
		: ticon_message_(title, message, portrait, mirror)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

/** Shows a dialog with the portrait on the right side. */
class ticon_message_right : public ticon_message_
{
public:
	ticon_message_right(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror)
		: ticon_message_(title, message, portrait, mirror)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

class toption_message_	: public ticon_message_
{
public:
	toption_message_(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror,
			const std::vector<std::string>& option_list,
			int* chosen_option)
		: ticon_message_(title, message, portrait, mirror)
		  , option_list_(option_list)
		  , chosen_option_(chosen_option)
	{
	}

private:
	/** The list of options the user can choose. */
	std::vector<std::string> option_list_;

	/** The chosen option. */
	int *chosen_option_;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

};

class toption_message_left : public toption_message_
{
public:
	toption_message_left(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror,
			const std::vector<std::string>& option_list,
			int* chosen_option)
		: toption_message_(title, message, portrait, mirror, option_list, chosen_option)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

/** Shows a dialog with the portrait on the right side. */
class toption_message_right : public toption_message_
{
public:
	toption_message_right(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror,
			const std::vector<std::string>& option_list,
			int* chosen_option)
		: toption_message_(title, message, portrait, mirror, option_list, chosen_option)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};



class tinput_message_ : public ticon_message_
{
public:
	tinput_message_(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror
			, const std::string& input_caption
			, std::string* input_text
			, const unsigned maximum_length)
		: ticon_message_(title, message, portrait, mirror)
		  , input_caption_(input_caption)
		  , input_text_(input_text)
		  , input_maximum_lenght_(maximum_length)
	{
	}
private:

	/** The caption to show for the input text. */
	std::string input_caption_;

	/** The text input. */
	std::string* input_text_;

	/** The maximum length of the input text. */
	unsigned input_maximum_lenght_;

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);
};

class tinput_message_left : public tinput_message_
{
public:
	tinput_message_left(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror
			, const std::string& input_caption
			, std::string* input_text
			, const unsigned maximum_length)
		: tinput_message_(title, message, portrait, mirror, input_caption, input_text, maximum_length)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

/** Shows a dialog with the portrait on the right side. */
class tinput_message_right : public tinput_message_
{
public:
	tinput_message_right(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror
			, const std::string& input_caption
			, std::string* input_text
			, const unsigned maximum_length)
		: tinput_message_(title, message, portrait, mirror, input_caption, input_text, maximum_length)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

class tunit_message_ : public ticon_message_
{
public:
	tunit_message_(const std::string& title,
			const std::string& message,
			const bool mirror,
			const std::vector<unit>& unit_list,
			unit* chosen_unit)
			: ticon_message_(title, message, chosen_unit->transparent(), mirror)
			  , unit_list_(unit_list) , chosen_unit_(chosen_unit), gold_(10000)
	{
	}

	tunit_message_(const std::string& title,
			const std::string& message,
			const bool mirror,
			const std::vector<const unit_type*> type_list,
			unit* chosen_unit, int side_num, int gold)
				: ticon_message_(title, message, "", mirror)
				  , chosen_unit_(chosen_unit)
				  , gold_(gold)
				  {
		std::vector<const unit_type*>::const_iterator it;
		it = type_list.begin();

		for (it = type_list.begin(); it != type_list.end(); it++)
		{
			unit new_unit(resources::units, *it, side_num, false);
			unit_list_.push_back(new_unit);
		}
				  }

	/** Handler for the profile button event. */
	void profile_pressed();

	void help_pressed();

protected:
	/** The list of units the user can choose. */
	std::vector<unit> unit_list_;

	/** The chosen unit. */
	unit* chosen_unit_;

	/** Handler for changed unit selection. */
	void update_unit_list(twindow& window);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

private:
	int gold_;

};

class trecruit_message_ : public tunit_message_
{
public:
	trecruit_message_(const bool mirror,
			const std::vector<const unit_type*> type_list,
			std::string& chosen_type,
			int side_num, int gold)
			: tunit_message_(/*_*/("Recruit")
					, /*_*/("Select unit:")
					, mirror
					, type_list, &unit_, side_num, gold )
			  , chosen_type_(chosen_type)
			  , unit_(resources::units, type_list.front(), false, side_num)
	{
	}

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void post_show(twindow& window);

private:
//	std::vector<const unit_type*> type_list_;
	std::string& chosen_type_;
	unit unit_;
	void update_type_list(twindow& window);

};

class trecruit_message_left : public trecruit_message_
{
public:
	trecruit_message_left(const bool mirror,
			const std::vector<const unit_type*> type_list,
			std::string& chosen_type, int side_num, int gold)
		: trecruit_message_(mirror, type_list, chosen_type, side_num, gold)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

class trecruit_message_right : public trecruit_message_
{
public:
	trecruit_message_right(const bool mirror,
			const std::vector<const unit_type*> type_list,
			std::string& chosen_type, int side_num, int gold)
		: trecruit_message_(mirror, type_list, chosen_type, side_num, gold)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};


class tunit_message_left : public tunit_message_
{
public:
	tunit_message_left(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror,
			const std::vector<unit>& unit_list, unit* chosen_unit)
		: tunit_message_(title, message, mirror, unit_list, chosen_unit)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

class tunit_message_right : public tunit_message_
{
public:
	tunit_message_right(const std::string& title, const std::string& message,
			const std::string& portrait, const bool mirror,
			const std::vector<unit>& unit_list, unit* chosen_unit)
		: tunit_message_(title, message, mirror, unit_list, chosen_unit)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

class trecall_message_ : public tunit_message_
{
public:
	trecall_message_(const bool mirror,
			const std::vector<unit>& unit_list,
			unit* chosen_unit,
			events::menu_handler* game_menu_handler)
		: tunit_message_(/*_*/("Recall"),/*_*/("Select Unit:"), mirror, unit_list, chosen_unit)
		  , game_menu_handler_(game_menu_handler)
	{
	}
private:
	/** The menu event manager that called us. */
	events::menu_handler* game_menu_handler_;

	/** Handler for changed unit selection. */
	void update_type_list(twindow& window);

	/** Handler for the recall button event. */
	void recall_pressed(twindow& window);

	/** Handler for the remove button event. */
	void remove_pressed(CVideo& video, twindow& window);

	/** Inherited from tdialog. */
	void pre_show(CVideo& video, twindow& window);

};

class trecall_message_left : public trecall_message_
{
public:
	trecall_message_left(const bool mirror,
			const std::vector<unit>& unit_list,
			unit* chosen_unit,
			events::menu_handler* game_menu_handler)
		: trecall_message_(mirror, unit_list, chosen_unit, game_menu_handler)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};

class trecall_message_right : public trecall_message_
{
public:
	trecall_message_right(const bool mirror,
			const std::vector<unit>& unit_list,
			unit* chosen_unit,
			events::menu_handler* game_menu_handler)
		: trecall_message_(mirror, unit_list, chosen_unit, game_menu_handler)
	{
	}
private:
	/** Inherited from twml_message_. */
	twindow* build_window(CVideo& video);
};



//TODO find a way to reference to these docu in the other methods.
/**
 *  Helper function to show a message with speaker portrait.
 *
 *  @param left_side              If true the portrait is shown on the left,
 *                                on the right side otherwise.
 *  @param video                  The display variable.
 *  @param title                  The title of the dialog.
 *  @param message                The message to show.
 *  @param portrait               Filename of the portrait.
 *  @param mirror                 Does the portrait need to be mirrored?
 */
int show_icon_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror);

/**
*
*  @param input_caption          The caption for the optional input text
*                                box. If this value != "" there is an input
*                                and the input text parameter is mandatory.
*  @param input_text             Pointer to the initial text value will be
*                                set to the result.
*  @param maximum_length         The maximum length of the text.
*
*/
int show_input_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
		, const std::string& input_caption
		, std::string* input_text
	    , const unsigned maximum_length);

/*
*  @param option_list            A list of options to select in the dialog.
*  @param chosen_option          Pointer to the initially chosen option.
*                                Will be set to the chosen_option when the
*                                dialog closes.
*/
int show_option_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
		, const std::vector<std::string>& option_list
		, int* chosen_option);

int show_unit_message(const bool left_side
		, CVideo& video
		, const std::string& title
		, const std::string& message
		, const std::string& portrait
		, const bool mirror
	    , const std::vector<unit>& unit_list
	    , unit* chosen_unit);

int show_recall_message(const bool left_side
		, CVideo& video
		, const bool mirror
	    , const std::vector<unit>& unit_list
	    , unit* chosen_unit
		, events::menu_handler* menu_handler_);

int show_recruit_message(const bool left_side
		, CVideo& video
		, const bool mirror
	    , const std::vector<const unit_type*> type_list
	//    , std::string& chosen_type
	    , int side_num
	    , int gold);

} // namespace gui2

#endif

