/* $Id$ */
/*
   Copyright (C) 2011 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "gui/dialogs/chat_log.hpp"

#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/slider.hpp"

#include "../../foreach.hpp"
#include "../../gamestatus.hpp"
#include "../../log.hpp"
#include "../../resources.hpp"
#include "../../team.hpp"
#include "../../replay.hpp"

#include <vector>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>

static lg::log_domain log_chat_log("chat_log");
#define DBG_CHAT_LOG LOG_STREAM(debug, log_chat_log)
#define LOG_CHAT_LOG LOG_STREAM(info, log_chat_log)
#define WRN_CHAT_LOG LOG_STREAM(warn, log_chat_log)
#define ERR_CHAT_LOG LOG_STREAM(err, log_chat_log)

namespace gui2 {

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 3_chat_log
 *
 * == Settings manager ==
 *
 * This shows the settings manager
 *
 */


template <class D, class V, void (V::*fptr)(twindow&)>
void dialog_view_callback(twidget* caller)
{
        D* dialog = dynamic_cast<D*>(caller->dialog());
        assert(dialog);
        twindow* window = dynamic_cast<twindow*>(caller->get_window());
        assert(window);
        (*(dialog->get_view()).*fptr)(*window);
}


//The model is an interface defining the data to be displayed or otherwise acted upon in the user interface.
class tchat_log::model {
public:
	model(const vconfig &c, replay *r)
		: cfg(c), chat_log_history(r->build_chat_log()), page(0), page_number(), previous_page(), next_page()
	{
		LOG_CHAT_LOG << "entering tchat_log::model...\n";
		LOG_CHAT_LOG << "finished tchat_log::model...\n";
	}

	vconfig cfg;
	tcontrol *msg_label;
	const std::vector<chat_msg> &chat_log_history;
	int page;
	static const int COUNT_PER_PAGE = 1000;
	tslider *page_number;
	tbutton *previous_page;
	tbutton *next_page;

	void clear_chat_msg_list()
	{
		msg_label->set_label("");
	}

	std::string replace(std::string str, const std::string &src, const std::string &dst)
	{
		std::string::size_type pos = 0;
		while ( (pos = str.find(src, pos)) != std::string::npos ) {
			str.replace( pos, src.size(), dst );
			pos++;
		}
		return str;
	}
	std::string escape(const std::string &str)
	{
		// need pango escape here
		std::string result = replace(str,"&","&amp;");
		result = replace(result,"<","&lt;");
		result = replace(result,">","&gt;");
		return result;
	}

	int count_of_pages()
	{
		int size = chat_log_history.size();
		return (size%COUNT_PER_PAGE==0) ? (size/COUNT_PER_PAGE) : (size/COUNT_PER_PAGE)+1 ;
	}

	void populate_chat_message_list(int first, int last)
	{
		std::stringstream str;
		LOG_CHAT_LOG << "entering tchat_log::model::add_row_to_chat_message_list\n";
		if (first<last) {
			BOOST_FOREACH (const chat_msg &t, make_pair(chat_log_history.begin()+first,chat_log_history.begin()+last))
			{
				std::string prefix("/me");
				bool me = false;
				if (!t.text().compare(0, prefix.size(), prefix))
				{
					me = true;
				}
				const std::string & color = t.color();
				std::string nick_prefix = "<span color=\""+color+"\">";
				std::string nick_suffix ="</span> ";
				std::map<std::string, string_map> data;
				if (me) {
					str << nick_prefix << "&lt;" << escape(t.nick()) << escape(t.text().substr(3))<<"&gt;" <<nick_suffix << std::endl;
				} else {
					str << nick_prefix << "&lt;" << escape(t.nick()) << "&gt;"<< nick_suffix << escape(t.text()) << std::endl;
				}
			}
		}
		msg_label->set_label(str.str());
		LOG_CHAT_LOG << "exited tchat_log::model::add_row_to_chat_message_list\n";
	}

};

//The controller acts upon the model. It retrieves data from repositories, persists it, manipulates it, and determines how it will be displayed in the view.
class tchat_log::controller {
public:
	controller(model &m)
		: model_(m)
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller" << std::endl;
		LOG_CHAT_LOG << "Exiting tchat_log::controller" << std::endl;
	}

	void next_page()
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller::next_page" << std::endl;
		if (model_.page >= model_.count_of_pages()-1) {
			return;
		}
		model_.page++;
		LOG_CHAT_LOG << "Set page to " << model_.page+1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting tchat_log::controller::next_page" << std::endl;
	}

	void previous_page()
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller::previous_page" << std::endl;
		if (model_.page == 0) {
			return;
		}
		model_.page--;
		LOG_CHAT_LOG << "Set page to " << model_.page+1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting tchat_log::controller::previous_page" << std::endl;
	}

	void handle_page_number_changed()
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller::handle_page_number_changed" << std::endl;
		model_.page = model_.page_number->get_value()-1;
		LOG_CHAT_LOG << "Set page to " << model_.page+1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting tchat_log::controller::handle_page_number_changed" << std::endl;
	}

	void update_view_from_model()
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller::update_view_from_model" << std::endl;
		model_.msg_label->set_use_markup(true);
		int size = model_.chat_log_history.size();
		LOG_CHAT_LOG << "Number of chat messages: " << size << std::endl;
		// get page
		int page = model_.page;
		// determine count of pages
		int count_of_pages = model_.count_of_pages();
		if (count_of_pages==0) {
			count_of_pages = 1;
		}
		LOG_CHAT_LOG << "Page: " << page+1 << " of " << count_of_pages << std::endl;
		// determine first
		int first = model_.page*model_.COUNT_PER_PAGE;
		// determine last
		int last = (page<count_of_pages-1) ? first+model_.COUNT_PER_PAGE : size;
		LOG_CHAT_LOG << "First " << first << ", last " << last << std::endl;
		// determine has previous, determine has next
		bool has_next = page+1 < count_of_pages;
		bool has_previous = page > 0;
		model_.previous_page->set_active(has_previous);
		model_.next_page->set_active(has_next);
		model_.populate_chat_message_list(first,last);
		model_.page_number->set_minimum_value(1);
		model_.page_number->set_maximum_value(count_of_pages);
		LOG_CHAT_LOG << "Maximum value of page number slider: " << count_of_pages << std::endl;
		model_.page_number->set_value(page+1);
		LOG_CHAT_LOG << "Exiting tchat_log::controller::update_view_from_model" << std::endl;
	}


private:
	model &model_;
};


//The view is an interface that displays data (the model) and routes user commands to the controller to act upon that data.
class tchat_log::view {
public:
	view(const vconfig &cfg, replay *r)
		: model_(cfg, r),controller_(model_)
	{
	}

	void pre_show(CVideo& /*video*/, twindow& window)
	{
		LOG_CHAT_LOG << "Entering tchat_log::view::pre_show" << std::endl;
		controller_.update_view_from_model();
		window.keyboard_capture(model_.msg_label);
		window.invalidate_layout();//workaround for assertion failure
		LOG_CHAT_LOG << "Exiting tchat_log::view::pre_show" << std::endl;
	}

	void handle_page_number_changed(twindow &window)
	{
		controller_.handle_page_number_changed();
		window.invalidate_layout();//workaround for assertion failure
	}

	void next_page(twindow &window)
	{
		controller_.next_page();
		window.invalidate_layout();//workaround for assertion failure
	}

	void previous_page(twindow &window)
	{
		controller_.previous_page();
		window.invalidate_layout();//workaround for assertion failure
	}

	void bind(twindow &window)
	{
		LOG_CHAT_LOG << "Entering tchat_log::view::bind" << std::endl;
		model_.msg_label = &find_widget<tcontrol>(&window, "msg", false);
		model_.page_number = &find_widget<tslider>(&window, "page_number", false);
		model_.page_number->set_callback_positioner_move(
			dialog_view_callback<tchat_log, tchat_log::view, &tchat_log::view::handle_page_number_changed>);

		model_.previous_page = &find_widget<tbutton>(&window, "previous_page", false);
		model_.previous_page->set_callback_mouse_left_click(
			dialog_view_callback<tchat_log, tchat_log::view, &tchat_log::view::previous_page>);

		model_.next_page = &find_widget<tbutton>(&window, "next_page", false);
		model_.next_page->set_callback_mouse_left_click(
			dialog_view_callback<tchat_log, tchat_log::view, &tchat_log::view::next_page>);


		LOG_CHAT_LOG << "Exiting tchat_log::view::bind" << std::endl;
	}

private:
	model model_;
	controller controller_;
};


tchat_log::tchat_log(const vconfig &cfg, replay *r)
	: view_()
{
	LOG_CHAT_LOG << "Entering tchat_log::tchat_log" << std::endl;
	view_ = boost::shared_ptr<view>(new view(cfg, r));
	LOG_CHAT_LOG << "Exiting tchat_log::tchat_log" << std::endl;
}

boost::shared_ptr<tchat_log::view> tchat_log::get_view()
{
	return view_;
}

twindow* tchat_log::build_window(CVideo& video)
{
	return build(video, get_id(CHAT_LOG));
}

void tchat_log::pre_show(CVideo& video, twindow& window)
{
	LOG_CHAT_LOG << "Entering tchat_log::pre_show" << std::endl;
	view_->bind(window);
	view_->pre_show(video,window);
	LOG_CHAT_LOG << "Exiting tchat_log::pre_show" << std::endl;
}

} //end of namespace gui2
