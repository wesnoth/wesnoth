#define GETTEXT_DOMAIN "wesnoth-lib" /*@ \label{progress_bar.cpp:textdomain} @*/

#include "gui/widgets/progress_bar.hpp"

#include "gui/auxiliary/widget_definition/progress_bar.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/settings.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__ /*@ \label{progress_bar.cpp:logheader} @*/
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2 {

REGISTER_WIDGET(progress_bar) /*@ \label{progress_bar.cpp:register} @*/

void tprogress_bar::set_percentage(const unsigned percentage)
{
	assert(percentage <= 100);

	if(percentage_ != percentage) {
		percentage_ = percentage;

		foreach(tcanvas& c, canvas()) {
			c.set_variable("percentage", variant(percentage));
		}

		set_dirty();
	}
}

const std::string& tprogress_bar::get_control_type() const /*@ \label{progress_bar.cpp:get_control_type} @*/
{
	static const std::string type = "progress bar";
	return type;
}

} // namespace gui2

