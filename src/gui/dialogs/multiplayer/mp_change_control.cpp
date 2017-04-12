/*
   Copyright (C) 2011 - 2017 by Lukasz Dobrogowski
   <lukasz.dobrogowski@gmail.com>
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

#include "gui/dialogs/multiplayer/mp_change_control.hpp"

#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"

#include "font/text_formatting.hpp"
#include "formula/string_utils.hpp"
#include "game_board.hpp"
#include "game_display.hpp"
#include "game_preferences.hpp"
#include "log.hpp"
#include "resources.hpp"
#include "team.hpp"

#include <vector>
#include "utils/functional.hpp"

static lg::log_domain log_gui("gui/dialogs/mp_change_control");
#define ERR_GUI LOG_STREAM(err, log_gui)
#define WRN_GUI LOG_STREAM(warn, log_gui)
#define LOG_GUI LOG_STREAM(info, log_gui)
#define DBG_GUI LOG_STREAM(debug, log_gui)

namespace gui2
{
namespace dialogs
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 2_mp_change_control
 *
 * == Change control dialog ==
 *
 * This shows the multiplayer change control dialog.
 *
 * @begin{table}{dialog_widgets}
 * sides_list & & listbox & m &
 *         List of sides participating in the MP game. $
 *
 * nicks_list & & listbox & m &
 *         List of nicks of all clients playing or observing the MP game. $
 *
 * @end{table}
 *
 */

template <class D, class V, void (V::*fptr)(window&)>
void dialog_view_callback(widget& caller)
{
	D* dialog = dynamic_cast<D*>(caller.dialog());
	assert(dialog);
	window* win = dynamic_cast<window*>(caller.get_window());
	assert(win);
	(*(dialog->get_view()).*fptr)(*win);
}

/**
 * The model is an interface defining the data to be displayed or otherwise
 * acted upon in the user interface.
 */
class mp_change_control::model
{
public:
	model() : sides_list(nullptr), nicks_list(nullptr), sides(), nicks()
	{
	}

	listbox* sides_list;
	listbox* nicks_list;

	// contains the mapping from listbox labels to actual sides
	// (note that due to hidden= attribute nth list item doesn't have to be nth
	// side)
	std::vector<int> sides;
	// contains the mapping from listbox labels to actual nicks
	std::vector<std::string> nicks;

	void clear_sides()
	{
		DBG_GUI << "Sides list: clearing\n";
		sides_list->clear();
		sides.clear();
	}

	void add_side(int side_num, const std::string& label)
	{
		sides.push_back(side_num);
		DBG_GUI << "Sides list: adding item (side_num: \"" << side_num
				<< "\" label: \"" << label << "\")\n";
		std::map<std::string, string_map> data;
		string_map item;
		item["id"] = std::string("side_") + std::to_string(side_num);
		item["label"] = label;
		item["use_markup"] = "true";
		data.emplace("side", item);
		sides_list->add_row(data);
	}

	void clear_nicks()
	{
		DBG_GUI << "Nicks list: clearing\n";
		nicks_list->clear();
		nicks.clear();
	}

	void add_nick(const std::string& nick, const std::string& label)
	{
		DBG_GUI << "Nicks list: adding item (nick: \"" << nick << "\" label: \""
				<< label << "\")\n";
		nicks.push_back(nick);
		std::map<std::string, string_map> data;
		string_map item;
		item["id"] = nick;
		item["label"] = label;
		item["use_markup"] = "true";
		data.emplace("nick", item);
		nicks_list->add_row(data);
	}
};

class side_controller
{
public:
	side_controller(const std::string& name,
					mp_change_control::model& m,
					int side_number)
		: model_(m), name_(name), side_number_(side_number)
	{
	}

	~side_controller()
	{
	}

	std::string name() const
	{
		return name_;
	}

	int side_number() const
	{
		return side_number_;
	}

	void show_nicks_list()
	{
		DBG_GUI << "Nicks list: showing for side " << side_number_ << '\n';
		// model_.selected_side = side_number_;
		model_.clear_nicks();

		std::set<std::string> nicks;
		for(std::vector<team>::const_iterator it = resources::gameboard->teams().begin();
			it != resources::gameboard->teams().end();
			++it) {
			if(!it->is_local_ai() && !it->is_network_ai() && !it->is_idle()
			   && !it->is_empty() && !it->current_player().empty())
				nicks.insert(it->current_player());
		}

		const std::set<std::string>& observers = resources::screen->observers();

		nicks.insert(observers.begin(), observers.end());
		nicks.insert(preferences::login()); // in case we are an observer, it
											// isn't in the observers set then
		// and has to be added manually

		int i = 0; // because we need to know which row contains the controlling
				   // player

		for(const auto & nick : nicks)
		{
			if(side_number_ <= static_cast<int>(resources::gameboard->teams().size())
			   && resources::gameboard->teams().at(side_number_ - 1).current_player()
				  == nick) {
				std::string label_str = "<b>" + nick + "</b>";
				model_.add_nick(nick, label_str);
				model_.nicks_list->select_row(i);
			} else
				model_.add_nick(nick, nick);
			++i;
		}
	}
	void handle_nicks_list_selection()
	{
		int selected = model_.nicks_list->get_selected_row();
		DBG_GUI << "Nicks list: row " << selected << " selected, it contains "
				<< model_.nicks[selected] << '\n';
	}
	void update_view_from_model()
	{
		show_nicks_list();
	}

private:
	mp_change_control::model& model_;
	const std::string name_;
	int side_number_;
};

/**
 * The controller acts upon the model.
 *
 * It retrieves data from repositories, persists it, manipulates it, and
 * determines how it will be displayed in the view.
 */
class mp_change_control::controller
{
public:
	typedef std::vector<std::shared_ptr<side_controller> >
	side_controller_ptr_vector;
	controller(model& m) : model_(m), side_controllers_()
	{
	}

	void show_sides_list()
	{
		DBG_GUI << "Sides list: filling\n";
		model_.clear_sides();
		int sides = resources::gameboard
							? static_cast<int>(resources::gameboard->teams().size())
							: 0;
		for(int side = 1; side <= sides; ++side) {
			if(!resources::gameboard->teams().at(side - 1).hidden()) {
				string_map symbols;
				symbols["side"] = std::to_string(side);
				std::string side_str = vgettext("Side $side", symbols);
				side_str = font::span_color(team::get_side_color(side))
						   + side_str + "</span>";
				model_.add_side(side, side_str);
				side_controllers_.emplace_back(std::make_shared<side_controller>(
						side_str, model_, side));
			}
		}
	}

	std::shared_ptr<side_controller> get_side_controller()
	{
		int selected = model_.sides_list->get_selected_row();
		if(selected < 0 || selected
						   >= static_cast<int>(side_controllers_.size()))
			return std::shared_ptr<side_controller>(); // null pointer
		else
			return side_controllers_.at(selected);
	}

	void handle_sides_list_item_clicked()
	{
		int selected = model_.sides_list->get_selected_row();
		DBG_GUI << "Sides list: selected row: " << selected << " for side "
				<< model_.sides[selected] << '\n';
		if(get_side_controller())
			get_side_controller()->update_view_from_model();
	}

	void handle_nicks_list_item_clicked()
	{
		int selected = model_.sides_list->get_selected_row();
		DBG_GUI << "Nicks list: selected row: " << selected << " with nick "
				<< model_.nicks[selected] << '\n';
		if(get_side_controller())
			get_side_controller()->handle_nicks_list_selection();
	}

	void update_view_from_model()
	{
		if(get_side_controller())
			get_side_controller()->update_view_from_model();
	}

	void change_control(events::menu_handler* mh)
	{
		int selected_side = model_.sides_list->get_selected_row();
		int selected_nick = model_.nicks_list->get_selected_row();
		DBG_GUI << "Main: changing control of side "
				<< model_.sides[selected_side] << " to nick "
				<< model_.nicks[selected_nick] << '\n';
		if(mh) // since in unit tests we pass a null pointer to it
			mh->request_control_change(model_.sides[selected_side],
									   model_.nicks[selected_nick]);
	}

private:
	model& model_;
	side_controller_ptr_vector side_controllers_;
};


/**
 * The view is an interface that displays data (the model) and routes user
 * commands to the controller to act upon that data.
 */
class mp_change_control::view
{
public:
	view() : model_(), controller_(model_)
	{
	}

	void pre_show(window& window)
	{
		model_.clear_sides();
		controller_.show_sides_list();
		model_.clear_nicks();
		controller_.update_view_from_model();
		window.invalidate_layout(); // workaround for assertion failure
	}

	void handle_sides_list_item_clicked(window& window)
	{
		controller_.handle_sides_list_item_clicked();
		window.invalidate_layout(); // workaround for assertion failure
	}

	void handle_nicks_list_item_clicked(window& window)
	{
		controller_.handle_nicks_list_item_clicked();
		window.invalidate_layout(); // workaround for assertion failure
	}

	void bind(window& window)
	{
		DBG_GUI << "Main: Binding widgets and callbacks\n";
		model_.sides_list
				= &find_widget<listbox>(&window, "sides_list", false);
		model_.nicks_list
				= &find_widget<listbox>(&window, "nicks_list", false);

#ifdef GUI2_EXPERIMENTAL_LISTBOX
		connect_signal_notify_modified(
				*model_.sides_list,
				std::bind(&mp_change_control::view::
									 handle_sides_list_item_clicked,
							this,
							std::ref(window)));

		connect_signal_notify_modified(
				*model_.nicks_list,
				std::bind(&mp_change_control::view::
									 handle_nicks_list_item_clicked,
							this,
							std::ref(window)));
#else
		model_.sides_list->set_callback_value_change(
				dialog_view_callback<mp_change_control,
									 mp_change_control::view,
									 &mp_change_control::view::
											  handle_sides_list_item_clicked>);

		model_.nicks_list->set_callback_value_change(
				dialog_view_callback<mp_change_control,
									 mp_change_control::view,
									 &mp_change_control::view::
											  handle_nicks_list_item_clicked>);
#endif
	}

	void post_show(int retval, events::menu_handler* mh)
	{
		if(retval == window::OK) {
			controller_.change_control(mh);
		}
	}

private:
	model model_;
	controller controller_;
};

REGISTER_DIALOG(mp_change_control)

mp_change_control::mp_change_control(events::menu_handler* mh)
	: menu_handler_(mh), view_(new view)
{
}

std::shared_ptr<mp_change_control::view> mp_change_control::get_view()
{
	return view_;
}

void mp_change_control::pre_show(window& window)
{
	view_->bind(window);
	view_->pre_show(window);
}

void mp_change_control::post_show(window& /*window*/)
{
	view_->post_show(get_retval(), menu_handler_);
}

} // namespace dialogs
} // namespace gui2
