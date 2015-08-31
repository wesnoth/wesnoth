
#include "quit_confirmation.hpp"
#include "gettext.hpp"
#include "display.hpp"
#include "gui/dialogs/message.hpp"
#include "gui/widgets/window.hpp"
#include "resources.hpp"

int quit_confirmation::count_ = 0;
bool quit_confirmation::open_ = false;
void quit_confirmation::quit()
{
	if(count_ != 0 && display::get_singleton() && !open_)
	{
		quit(display::get_singleton()->video());
	}
	else
	{
		throw CVideo::quit();
	}
}

void quit_confirmation::quit(CVideo& video)
{
	assert(!open_);
	open_ = true;
	const int res = gui2::show_message(video, _("Quit"),
		_("Do you really want to quit?"), gui2::tmessage::yes_no_buttons);
	open_ = false;
	if(res != gui2::twindow::CANCEL) {
		throw CVideo::quit();
	} else {
		return;
	}
}
