/*
   Copyright (C) 2011 - 2017 by Yurii Chernyi <terraninfo@terraninfo.net>
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

#include "gui/auxiliary/find_widget.hpp"
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
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/slider.hpp"

#include "font/pango/escape.hpp"
#include "desktop/clipboard.hpp"
#include "serialization/unicode.hpp"
#include "preferences/game.hpp"
#include "log.hpp"
#include "replay.hpp"
#include "gettext.hpp"

#include "utils/functional.hpp"
#include "utils/iterable_pair.hpp"

#include <vector>

static lg::log_domain log_chat_log("chat_log");
#define DBG_CHAT_LOG LOG_STREAM(debug, log_chat_log)
#define LOG_CHAT_LOG LOG_STREAM(info, log_chat_log)
#define WRN_CHAT_LOG LOG_STREAM(warn, log_chat_log)
#define ERR_CHAT_LOG LOG_STREAM(err, log_chat_log)

namespace gui2
{
namespace dialogs
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
class chat_log::model
{
public:
	model(const vconfig& c, const replay& r)
		: cfg(c)
		, msg_label(nullptr)
		, chat_log_history(r.build_chat_log())
		, page(0)
		, page_number()
		, page_label()
		, previous_page()
		, next_page()
		, filter()
		, copy_button()
	{
		LOG_CHAT_LOG << "entering chat_log::model...\n";
		LOG_CHAT_LOG << "finished chat_log::model...\n";
	}

	vconfig cfg;
	styled_widget* msg_label;
	const std::vector<chat_msg>& chat_log_history;
	int page;
	static const int COUNT_PER_PAGE = 100;
	slider* page_number;
	styled_widget* page_label;
	button* previous_page;
	button* next_page;
	text_box* filter;
	button* copy_button;

	void clear_chat_msg_list()
	{
		msg_label->set_label("");
	}

	int count_of_pages() const
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
		LOG_CHAT_LOG << "entering chat_log::model::stream_log\n";

		for(const auto & t : make_pair(chat_log_history.begin() + first,
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

		// It makes sense to always scroll to the bottom, since the newest messages are there.
		// The only time this might not be desired is tabbing forward through the pages, since
		// one might want to continue reading the conversation in order.
		//
		// TODO: look into implementing the above suggestion
		dynamic_cast<scroll_label&>(*msg_label).scroll_vertical_scrollbar(scrollbar_base::END);
	}

	void chat_message_list_to_clipboard(int first, int last)
	{
		std::ostringstream s;
		stream_log(s, first, last, true);
		desktop::clipboard::copy_to_clipboard(s.str(), false);
	}
};

// The controller acts upon the model. It retrieves data from repositories,
// persists it, manipulates it, and determines how it will be displayed in the
// view.
class chat_log::controller
{
public:
	controller(model& m) : model_(m)
	{
		LOG_CHAT_LOG << "Entering chat_log::controller" << std::endl;
		LOG_CHAT_LOG << "Exiting chat_log::controller" << std::endl;
	}

	void next_page()
	{
		LOG_CHAT_LOG << "Entering chat_log::controller::next_page"
					 << std::endl;
		if(model_.page >= model_.count_of_pages() - 1) {
			return;
		}
		model_.page++;
		LOG_CHAT_LOG << "Set page to " << model_.page + 1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting chat_log::controller::next_page" << std::endl;
	}

	void previous_page()
	{
		LOG_CHAT_LOG << "Entering chat_log::controller::previous_page"
					 << std::endl;
		if(model_.page == 0) {
			return;
		}
		model_.page--;
		LOG_CHAT_LOG << "Set page to " << model_.page + 1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting chat_log::controller::previous_page"
					 << std::endl;
	}

	void filter()
	{
		LOG_CHAT_LOG << "Entering chat_log::controller::filter" << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG << "Exiting chat_log::controller::filter" << std::endl;
	}

	void handle_page_number_changed()
	{
		LOG_CHAT_LOG
		<< "Entering chat_log::controller::handle_page_number_changed"
		<< std::endl;
		model_.page = model_.page_number->get_value() - 1;
		LOG_CHAT_LOG << "Set page to " << model_.page + 1 << std::endl;
		update_view_from_model();
		LOG_CHAT_LOG
		<< "Exiting chat_log::controller::handle_page_number_changed"
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

	void update_view_from_model(bool select_last_page = false)
	{
		LOG_CHAT_LOG << "Entering chat_log::controller::update_view_from_model"
					 << std::endl;
		model_.msg_label->set_use_markup(true);
		int size = model_.chat_log_history.size();
		LOG_CHAT_LOG << "Number of chat messages: " << size << std::endl;
		// determine count of pages
		const int count_of_pages = std::max(1, model_.count_of_pages());
		if(select_last_page) {
			model_.page = count_of_pages - 1;
		}
		// get page
		const int page = model_.page;
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
		model_.page_number->set_value_range(1, count_of_pages);
		model_.page_number->set_active(count_of_pages > 1);
		LOG_CHAT_LOG << "Maximum value of page number slider: "
					 << count_of_pages << std::endl;
		model_.page_number->set_value(page + 1);

		std::ostringstream cur_page_text;
		cur_page_text << (page + 1) << '/' << std::max(1, count_of_pages);
		model_.page_label->set_label(cur_page_text.str());

		LOG_CHAT_LOG << "Exiting chat_log::controller::update_view_from_model"
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
class chat_log::view
{
public:
	view(const vconfig& cfg, const replay& r) : model_(cfg, r), controller_(model_)
	{
	}

	void pre_show()
	{
		LOG_CHAT_LOG << "Entering chat_log::view::pre_show" << std::endl;
		controller_.update_view_from_model(true);
		LOG_CHAT_LOG << "Exiting chat_log::view::pre_show" << std::endl;
	}

	void handle_page_number_changed()
	{
		controller_.handle_page_number_changed();
	}

	void next_page()
	{
		controller_.next_page();
	}

	void previous_page()
	{
		controller_.previous_page();
	}

	void filter()
	{
		controller_.filter();
	}

	void handle_copy_button_clicked(window& /*window*/)
	{
		controller_.handle_copy_button_clicked();
	}

	void bind(window& window)
	{
		LOG_CHAT_LOG << "Entering chat_log::view::bind" << std::endl;
		model_.msg_label = find_widget<styled_widget>(&window, "msg", false, true);
		model_.page_number
				= find_widget<slider>(&window, "page_number", false, true);
		connect_signal_notify_modified(
				*model_.page_number,
				std::bind(&view::handle_page_number_changed, this));

		model_.previous_page
				= find_widget<button>(&window, "previous_page", false, true);
		model_.previous_page->connect_click_handler(
				std::bind(&view::previous_page, this));

		model_.next_page = find_widget<button>(&window, "next_page", false, true);
		model_.next_page->connect_click_handler(
				std::bind(&view::next_page, this));

		model_.filter = find_widget<text_box>(&window, "filter", false, true);
		model_.filter->set_text_changed_callback(
				std::bind(&view::filter, this));
		window.keyboard_capture(model_.filter);

		model_.copy_button = find_widget<button>(&window, "copy", false, true);
		connect_signal_mouse_left_click(
				*model_.copy_button,
				std::bind(&view::handle_copy_button_clicked,
							this,
							std::ref(window)));
		if (!desktop::clipboard::available()) {
			model_.copy_button->set_active(false);
			model_.copy_button->set_tooltip(_("Clipboard support not found, contact your packager"));
		}

		model_.page_label = find_widget<styled_widget>(&window, "page_label", false, true);

		LOG_CHAT_LOG << "Exiting chat_log::view::bind" << std::endl;
	}

private:
	model model_;
	controller controller_;
};


chat_log::chat_log(const vconfig& cfg, const replay& r) : view_()
{
	LOG_CHAT_LOG << "Entering chat_log::chat_log" << std::endl;
	view_ = std::make_shared<view>(cfg, r);
	LOG_CHAT_LOG << "Exiting chat_log::chat_log" << std::endl;
}

std::shared_ptr<chat_log::view> chat_log::get_view()
{
	return view_;
}

void chat_log::pre_show(window& window)
{
	LOG_CHAT_LOG << "Entering chat_log::pre_show" << std::endl;
	view_->bind(window);
	view_->pre_show();
	LOG_CHAT_LOG << "Exiting chat_log::pre_show" << std::endl;
}

} // namespace dialogs
} // namespace gui2
