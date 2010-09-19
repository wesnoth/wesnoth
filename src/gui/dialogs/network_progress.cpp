/* $Id$ */
/*
   Copyright (C) 2008 - 2010 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/dialogs/network_progress.hpp"

#include "foreach.hpp"
#include "gui/auxiliary/timer.hpp"
#include "gui/dialogs/field.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/password_box.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/toggle_button.hpp"
#include "log.hpp"

#include <boost/bind.hpp>

#include <iomanip>
#include <sstream>

static lg::log_domain log_network("network");
#define ERR_NET LOG_STREAM(err , log_network)
#define LOG_NET LOG_STREAM(info, log_network)

namespace gui2 {

REGISTER_WINDOW(network_progress)

tnetwork_progress::tnetwork_progress()
	: pd_(NULL), poll_progress_timer_(0)
{
}

tnetwork_progress::~tnetwork_progress()
{
        if(poll_progress_timer_) {
        	remove_timer(poll_progress_timer_);
        	poll_progress_timer_ = 0;
	}
}

void tnetwork_progress::set_progress_object(network::progress_data &pd)
{
        pd_ = &pd;
}
                                
void tnetwork_progress::pre_show(CVideo& /*video*/, twindow& window)
{
	assert(pd_);

	// Set view list callback button.
	if(tbutton* button = find_widget<tbutton>(&window, "abort", false, false)) {

		button->connect_click_handler(boost::bind(
				  &tnetwork_progress::abort
				, this
				, boost::ref(window)));
	}
        poll_progress_timer_ = add_timer(100
                                , boost::bind(&tnetwork_progress::poll_progress, this, boost::ref(window))
				, true);                        
}

void tnetwork_progress::post_show(twindow& /*window*/)
{
        if(poll_progress_timer_) {
                remove_timer(poll_progress_timer_);
                poll_progress_timer_ = 0;
        }
}

void tnetwork_progress::poll_progress(twindow& window)
{
	if (!pd_->running()) {
		window.close();
        } else {
                tlabel* progress = dynamic_cast<tlabel*>(window.find("progress", false));
                assert(progress);
                double total = pd_->total();
                double done = pd_->done();
                std::stringstream ss;
                if (total > 0) {
                    double percent = (total > 0) ? (done / total) : 0;
                    percent *= 100;
                    ss << done << "/" << total << "( " << std::setprecision(1) << percent << ")";
                } else {
                    if (done == 0) {
                        ss << "...";
                    } else {
                        ss << done;
                    }
                }
                LOG_NET << ss.str() << "\n";
                progress->set_label(ss.str());
                window.invalidate_layout();
        }

}

void tnetwork_progress::abort(twindow& /*window*/)
{
	pd_->set_abort(true);
}

} // end namespace gui2
