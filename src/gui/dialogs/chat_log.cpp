/*
   Copyright (C) 2011 - 2014 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "gui/auxiliary/find_widget.tpp"
#include "gui/dialogs/helper.hpp"
#include "gui/widgets/button.hpp"
#ifdef GUI2_EXPERIMENTAL_LISTBOX
#include "gui/widgets/list.hpp"
#else
#include "gui/widgets/listbox.hpp"
#endif
#include "gui/widgets/settings.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "gui/widgets/slider.hpp"
#include "utils/foreach.tpp"

#include "../../clipboard.hpp"
#include "../../game_preferences.hpp"
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

namespace gui2
{

/*WIKI
 * @page = GUIWindowDefinitionWML
 * @order = 3_chat_log
 *
 * == Settings manager ==
 *
 * This shows the settings manager
 *
 */


REGISTER_DIALOG(chat_log)

// The model is an interface defining the data to be displayed or otherwise
// acted upon in the user interface.
class tchat_log::model
{
public:
	model(const vconfig& c, replay* r)
		: cfg(c)
		, msg_label(NULL)
		, chat_log_history(r->build_chat_log())
		, page(0)
		, page_number()
		, previous_page()
		, next_page()
		, filter()
		, copy_button()
	{
		LOG_CHAT_LOG << "entering tchat_log::model...\n";
		LOG_CHAT_LOG << "finished tchat_log::model...\n";
	}

	vconfig cfg;
	tcontrol* msg_label;
	const std::vector<chat_msg>& chat_log_history;
	int page;
	static const int COUNT_PER_PAGE = 100;
	tslider* page_number;
	tbutton* previous_page;
	tbutton* next_page;
	ttext_box* filter;
	tbutton* copy_button;

	void clear_chat_msg_list()
	{
		msg_label->set_label("");
	}

	int count_of_pages()
	{
		int size = chat_log_history.size();
		return (size % COUNT_PER_PAGE == 0) ? (size / COUNT_PER_PAGE)
											: (size / COUNT_PER_PAGE) + 1;
	}

	void stream_log(std::ostringstream& s,
					int first,
					int last,
					bool raw = false)
	{
		if(first >= last) {
			return;
		}

		const std::string& lcfilter = utf8::lowercase(filter->get_value());
		LOG_CHAT_LOG << "entering tchat_log::model::stream_log\n";

		FOREACH(const AUTO & t, make_pair(chat_log_history.begin() + first,
										  chat_log_history.begin() + last))
		{
			const std::string& timestamp
					= preferences::get_chat_timestamp(t.time());

			if(!lcfilter.empty()) {
				const std::string& lcsample = utf8::lowercase(timestamp)
											  + utf8::lowercase(t.nick())
											  + utf8::lowercase(t.text());

				if(lcsample.find(lcfilter) == std::string::npos) {
					continue;
				}
			}

			const std::string me_prefix = "/me";
			const bool is_me = t.text().compare(0, me_prefix.size(),
												me_prefix) == 0;

			std::string nick_prefix, nick_suffix;

			if(!raw) {
				nick_prefix = "<span color=\"" + t.color() + "\">";
				nick_suffix = "</span> ";
			} else {
				nick_suffix = " ";
			}

			const std::string lbracket = raw ? "<" : "&lt;";
			const std::string rbracket = raw ? ">" : "&gt;";

			//
			// Chat line format:
			//
			// is_me == true:  "<[TS] nick message text here>\n"
			// is_me == false: "<[TS] nick> message text here\n"
			//

			s << nick_prefix << lbracket;

			if(raw) {
				s << timestamp
				  << t.nick();
			} else {
				s << font::escape_text(timestamp)
				  << font::escape_text(t.nick());
			}

			if(is_me) {
				if(!raw) {
					s << font::escape_text(t.text().substr(3));
				} else {
					s << t.text().substr(3);
				}
				s << rbracket << nick_suffix;
			} else {
				// <[TS] nick> message text here
				s << rbracket << nick_suffix;
				if(!raw) {
					s << font::escape_text(t.text());
				} else {
					s << t.text();
				}
			}

			s << '\n';
		}
	}

	void populate_chat_message_list(int first, int last)
	{
		std::ostringstream s;
		stream_log(s, first, last);
		msg_label->set_label(s.str());
	}

	void chat_message_list_to_clipboard(int first, int last)
	{
		std::ostringstream s;
		stream_log(s, first, last, true);
		copy_to_clipboard(s.str(), false);
	}
};

// The controller acts upon the model. It retrieves data from repositories,
// persists it, manipulates it, and determines how it will be displayed in the
// view.
class tchat_log::controller
{
public:
	controller(model& m) : model_(m)
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller" << std::endl;
		LOG_CHAT_LOG << "Exiting tchat_log::controller" << std::endl;
	}

	void next_page()
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller::next_page"
					 << std::endl;
		if(model_.page >= model_.count_of_pages() - 1) {
			return;
		}
		model_.page++;
		LOG_CHAT_LOG << "Set page to " << model_.page + 1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting tchat_log::controller::next_page" << std::endl;
	}

	void previous_page()
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller::previous_page"
					 << std::endl;
		if(model_.page == 0) {
			return;
		}
		model_.page--;
		LOG_CHAT_LOG << "Set page to " << model_.page + 1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting tchat_log::controller::previous_page"
					 << std::endl;
	}

	void filter()
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller::filter" << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting tchat_log::controller::filter" << std::endl;
	}

	void handle_page_number_changed()
	{
		LOG_CHAT_LOG
		<< "Entering tchat_log::controller::handle_page_number_changed"
		<< std::endl;
		model_.page = model_.page_number->get_value() - 1;
		LOG_CHAT_LOG << "Set page to " << model_.page + 1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG
		<< "Exiting tchat_log::controller::handle_page_number_changed"
		<< std::endl;
	}

	std::pair<int, int> calculate_log_line_range()
	{
		const int log_size = model_.chat_log_history.size();
		const int page_size = model_.COUNT_PER_PAGE;

		const int page = model_.page;
		const int count_of_pages = std::max(1, model_.count_of_pages());

		LOG_CHAT_LOG << "Page: " << page + 1 << " of " << count_of_pages
		             << '\n';

		const int first = page * page_size;
		const int last = page < (count_of_pages - 1)
						 ? first + page_size
						 : log_size;

		LOG_CHAT_LOG << "First " << first << ", last " << last << '\n';

		return std::make_pair(first, last);
	}

	void update_view_from_model()
	{
		LOG_CHAT_LOG << "Entering tchat_log::controller::update_view_from_model"
					 << std::endl;
		model_.msg_label->set_use_markup(true);
		int size = model_.chat_log_history.size();
		LOG_CHAT_LOG << "Number of chat messages: " << size << std::endl;
		// get page
		const int page = model_.page;
		// determine count of pages
		const int count_of_pages = std::max(1, model_.count_of_pages());
		// determine first and last
		const std::pair<int, int>& range = calculate_log_line_range();
		const int first = range.first;
		const int last = range.second;
		// determine has previous, determine has next
		bool has_next = page + 1 < count_of_pages;
		bool has_previous = page > 0;
		model_.previous_page->set_active(has_previous);
		model_.next_page->set_active(has_next);
		model_.populate_chat_message_list(first, last);
		model_.page_number->set_minimum_value(1);
		model_.page_number->set_maximum_value(count_of_pages);
		model_.page_number->set_active(count_of_pages > 1);
		LOG_CHAT_LOG << "Maximum value of page number slider: "
					 << count_of_pages << std::endl;
		model_.page_number->set_value(page + 1);
		LOG_CHAT_LOG << "Exiting tchat_log::controller::update_view_from_model"
					 << std::endl;
	}

	void handle_copy_button_clicked()
	{
		const std::pair<int, int>& range = calculate_log_line_range();
		model_.chat_message_list_to_clipboard(range.first, range.second);
	}

private:
	model& model_;
};


// The view is an interface that displays data (the model) and routes user
// commands to the controller to act upon that data.
class tchat_log::view
{
public:
	view(const vconfig& cfg, replay* r) : model_(cfg, r), controller_(model_)
	{
	}

	void pre_show(CVideo& /*video*/, twindow& window)
	{
		LOG_CHAT_LOG << "Entering tchat_log::view::pre_show" << std::endl;
		controller_.update_view_from_model();
		window.invalidate_layout(); // workaround for assertion failure
		LOG_CHAT_LOG << "Exiting tchat_log::view::pre_show" << std::endl;
	}

	void handle_page_number_changed(twindow& window)
	{
		controller_.handle_page_number_changed();
		window.invalidate_layout(); // workaround for assertion failure
	}

	void next_page(twindow& window)
	{
		controller_.next_page();
		window.invalidate_layout(); // workaround for assertion failure
	}

	void previous_page(twindow& window)
	{
		controller_.previous_page();
		window.invalidate_layout(); // workaround for assertion failure
	}

	void filter(twindow& window)
	{
		controller_.filter();
		window.invalidate_layout(); // workaround for assertion failure
	}

	void handle_copy_button_clicked(twindow& /*window*/)
	{
		controller_.handle_copy_button_clicked();
	}

	void bind(twindow& window)
	{
		LOG_CHAT_LOG << "Entering tchat_log::view::bind" << std::endl;
		model_.msg_label = &find_widget<tcontrol>(&window, "msg", false);
		model_.page_number
				= &find_widget<tslider>(&window, "page_number", false);
		connect_signal_notify_modified(
				*model_.page_number,
				boost::bind(&view::handle_page_number_changed,
							this,
							boost::ref(window)));

		model_.previous_page
				= &find_widget<tbutton>(&window, "previous_page", false);
		model_.previous_page->connect_click_handler(
				boost::bind(&view::previous_page, this, boost::ref(window)));

		model_.next_page = &find_widget<tbutton>(&window, "next_page", false);
		model_.next_page->connect_click_handler(
				boost::bind(&view::next_page, this, boost::ref(window)));

		model_.filter = &find_widget<ttext_box>(&window, "filter", false);
		model_.filter->set_text_changed_callback(
				boost::bind(&view::filter, this, boost::ref(window)));
		window.keyboard_capture(model_.filter);

		model_.copy_button = &find_widget<tbutton>(&window, "copy", false);
		connect_signal_mouse_left_click(
				*model_.copy_button,
				boost::bind(&view::handle_copy_button_clicked,
							this,
							boost::ref(window)));

		LOG_CHAT_LOG << "Exiting tchat_log::view::bind" << std::endl;
	}

private:
	model model_;
	controller controller_;
};


tchat_log::tchat_log(const vconfig& cfg, replay* r) : view_()
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
	return build(video, window_id());
}

void tchat_log::pre_show(CVideo& video, twindow& window)
{
	LOG_CHAT_LOG << "Entering tchat_log::pre_show" << std::endl;
	view_->bind(window);
	view_->pre_show(video, window);
	LOG_CHAT_LOG << "Exiting tchat_log::pre_show" << std::endl;
}

} // end of namespace gui2
