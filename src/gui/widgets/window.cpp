/* $Id$ */
/*
   Copyright (C) 2007 - 2009 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file window.cpp
 *  Implementation of window.hpp.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/window_private.hpp"

#include "font.hpp"
#include "foreach.hpp"
#include "game_display.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/layout_exception.hpp"
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif
#include "preferences.hpp"
#include "titlescreen.hpp"
#include "video.hpp"

namespace gui2{

unsigned twindow::sunset_ = 0;

namespace {
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	const unsigned MANUAL = tdebug_layout_graph::MANUAL;
	const unsigned SHOW = tdebug_layout_graph::SHOW;
	const unsigned LAYOUT = tdebug_layout_graph::LAYOUT;
#else
	// values are irrelavant when DEBUG_WINDOW_LAYOUT_GRAPHS is not defined.
	const unsigned MANUAL = 0;
	const unsigned SHOW = 0;
	const unsigned LAYOUT = 0;
#endif

/**
 * The interval between draw events.
 *
 * When the window is shown this value is set, the callback function always
 * uses this value instead of the parameter send, that way the window can stop
 * drawing when it wants.
 */
static int draw_interval = 0;

/**
 * SDL_AddTimer() callback for the draw event.
 *
 * When this callback is called it pushes a new draw event in the event queue.
 *
 * @returns                       The new timer interval, 0 to stop.
 */
static Uint32 draw_timer(Uint32, void*)
{
//	DBG_GUI_E << "Pushing draw event in queue.\n";

	SDL_Event event;
	SDL_UserEvent data;

	data.type = DRAW_EVENT;
	data.code = 0;
	data.data1 = NULL;
	data.data2 = NULL;

	event.type = DRAW_EVENT;
	event.user = data;

	SDL_PushEvent(&event);
	return draw_interval;
}

/**
 * SDL_AddTimer() callback for delay_event.
 *
 * @param event                   The event to push in the event queue.
 *
 * @return                        The new timer interval (always 0).
 */
static Uint32 delay_event_callback(const Uint32, void* event)
{
	SDL_PushEvent(static_cast<SDL_Event*>(event));
	return 0;
}

/**
 * Allows an event to be delayed a certain amount of time.
 *
 * @note the delay is the minimum time, after the time has passed the event
 * will be pushed in the SDL event queue, so it might delay more.
 *
 * @param event                   The event to delay.
 * @param delay                   The number of ms to delay the event.
 */
static void delay_event(const SDL_Event& event, const Uint32 delay)
{
	SDL_AddTimer(delay, delay_event_callback, new SDL_Event(event));
}

/**
 * Small helper class to get an unique id for every window instance.
 *
 * This is used to send event to the proper window, this allows windows to post
 * messages to themselves and let them delay for a certain amount of time.
 */
class tmanager
{
	tmanager();
public:

	static tmanager& instance();

	void add(twindow& window);

	void remove(twindow& window);

	unsigned get_id(twindow& window);

	twindow* window(const unsigned id);

private:

	// The number of active window should be rather small
	// so keep it simple and don't add a reverse lookup map.
	std::map<unsigned, twindow*> windows_;
};

tmanager::tmanager()
	: windows_()
{
}

tmanager& tmanager::instance()
{
	static tmanager window_manager;
	return window_manager;
}

void tmanager::add(twindow& window)
{
	static unsigned id;
	++id;
	windows_[id] = &window;
}

void tmanager::remove(twindow& window)
{
	for(std::map<unsigned, twindow*>::iterator itor = windows_.begin();
			itor != windows_.end(); ++itor) {

		if(itor->second == &window) {
			windows_.erase(itor);
			return;
		}
	}
	assert(false);
}

unsigned tmanager::get_id(twindow& window)
{
	for(std::map<unsigned, twindow*>::iterator itor = windows_.begin();
			itor != windows_.end(); ++itor) {

		if(itor->second == &window) {
			return itor->first;
		}
	}
	assert(false);
}

twindow* tmanager::window(const unsigned id)
{
	std::map<unsigned, twindow*>::iterator itor = windows_.find(id);

	if(itor == windows_.end()) {
		return NULL;
	} else {
		return itor->second;
	}
}

} // namespace

twindow::twindow(CVideo& video,
		tformula<unsigned>x,
		tformula<unsigned>y,
		tformula<unsigned>w,
		tformula<unsigned>h,
		const bool automatic_placement,
		const unsigned horizontal_placement,
		const unsigned vertical_placement,
		const std::string& definition)
	: tpanel()
	, tevent_handler()
	, cursor::setter(cursor::NORMAL)
	, video_(video)
	, status_(NEW)
	, retval_(NONE)
	, owner_(0)
	, need_layout_(true)
	, suspend_drawing_(true)
	, top_level_(false)
	, restorer_()
	, tooltip_()
	, tooltip_restorer_()
	, help_popup_()
	, automatic_placement_(automatic_placement)
	, horizontal_placement_(horizontal_placement)
	, vertical_placement_(vertical_placement)
	, x_(x)
	, y_(y)
	, w_(w)
	, h_(h)
	, easy_close_(false)
	, easy_close_blocker_()
	, escape_disabled_(false)
	, linked_size_()
	, dirty_list_()
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, debug_layout_(new tdebug_layout_graph(this))
#endif
{
	// We load the config in here as exception.
	// Our caller did update the screen size so no need for us to do that again.
	set_definition(definition);
	load_config();

	tooltip_.set_definition("default");
	tooltip_.set_parent(this);
	tooltip_.set_visible(twidget::HIDDEN);

	help_popup_.set_definition("default");
	help_popup_.set_parent(this);
	help_popup_.set_visible(twidget::HIDDEN);

	tmanager::instance().add(*this);
}

twindow::~twindow()
{
	tmanager::instance().remove(*this);

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS

	delete debug_layout_;

#endif
}

twindow* twindow::window_instance(const unsigned handle)
{
	return tmanager::instance().window(handle);
}

void twindow::update_screen_size()
{
	// Only if we're the toplevel window we need to update the size, otherwise
	// it's done in the resize event.
	if(draw_interval == 0) {
		const SDL_Rect rect = screen_area();
		settings::screen_width = rect.w;
		settings::screen_height = rect.h;

		settings::gamemap_width = settings::screen_width;
		settings::gamemap_height = settings::screen_height;

		game_display* display = game_display::get_singleton();
		if(display) {
			const unsigned w = display->map_outside_area().w;
			const unsigned h = display->map_outside_area().h;
			if(w && h) {
				settings::gamemap_width = w;
				settings::gamemap_height = h;
			}
		}
	}
}
twindow::tretval twindow::get_retval_by_id(const std::string& id)
{
/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget_window_2
 *
 * List if the id's that have generate a return value:
 * * ok confirms the dialog.
 * * cancel cancels the dialog.
 *
 */
	// Note it might change to a map later depending on the number
	// of items.
	if(id == "ok") {
		return OK;
	} else if(id == "cancel") {
		return CANCEL;

	/**
	 * The ones for the title screen.
	 *
	 * This is a kind of hack, but the values are hardcoded in the titlescreen
	 * and don't want to change them at the moment. It would be a good idea to
	 * add some namespaces to avoid names clashing.
	 */
	} else if(id == "tutorial") {
		return static_cast<tretval>(gui::TUTORIAL);
#ifndef DISABLE_EDITOR2
	} else if(id == "editor") {
		return static_cast<tretval>(gui::START_MAP_EDITOR);
#endif
	} else if(id == "credits") {
		return static_cast<tretval>(gui::SHOW_ABOUT);
	} else if(id == "quit") {
		return static_cast<tretval>(gui::QUIT_GAME);

	/**
	 * The hacks which are here so the old engine can handle the event. The new
	 * engine can't handle all dialogs yet, so it needs to fall back to the old
	 * engine to make certain things happen.
	 */
	} else if(id == "campaign") {
		return static_cast<tretval>(gui::NEW_CAMPAIGN);
	} else if(id == "multiplayer") {
		return static_cast<tretval>(gui::MULTIPLAYER);
	} else if(id == "load") {
		return static_cast<tretval>(gui::LOAD_GAME);
	} else if(id == "addons") {
		return static_cast<tretval>(gui::GET_ADDONS);
	} else if(id == "preferences") {
		return static_cast<tretval>(gui::EDIT_PREFERENCES);

	// default if nothing matched
	} else {
		return NONE;
	}
}

int twindow::show(const bool restore, const unsigned auto_close_timeout)
{
	log_scope2(log_gui_draw, "Window: show.");

	generate_dot_file("show", SHOW);

	assert(status_ == NEW);

	top_level_ = (draw_interval == 0);
	if(top_level_) {
		draw_interval = 30;
		SDL_AddTimer(draw_interval, draw_timer, NULL);

		// There might be some time between creation and showing so reupdate
		// the sizes.
		update_screen_size();
	}

	/*
	 * Before show has been called, some functions might have done some testing
	 * on the window and called layout, which can give glitches. So
	 * reinvalidate the window to avoid those glitches.
	 */
	invalidate_layout();
	suspend_drawing_ = false;

	if(auto_close_timeout) {
		// Make sure we're drawn before we try to close ourselves, which can
		// happen if the timeout is small.
		draw();

		SDL_Event event;
		SDL_UserEvent data;

		data.type = CLOSE_WINDOW_EVENT;
		data.code = tmanager::instance().get_id(*this);
		data.data1 = NULL;
		data.data2 = NULL;

		event.type = CLOSE_WINDOW_EVENT;
		event.user = data;

		delay_event(event, auto_close_timeout);
	}

	// Start our loop drawing will happen here as well.
	for(status_ = SHOWING; status_ != REQUEST_CLOSE; ) {
		process_events();
		// Add a delay so we don't keep spinning if there's no event.
		SDL_Delay(10);
	}

	suspend_drawing_ = true;

	if(top_level_) {
		draw_interval = 0;
	}

	// restore area
	if(restore) {
		SDL_Rect rect = get_rect();
		SDL_BlitSurface(restorer_, 0, video_.getSurface(), &rect);
		update_rect(get_rect());
	}

	return retval_;
}

void twindow::draw()
{
	/***** ***** ***** ***** Init ***** ***** ***** *****/
	// Prohibited from drawing?
	if(suspend_drawing_) {
		return;
	}

	surface frame_buffer = video_.getSurface();

	/***** ***** Layout and get dirty list ***** *****/
	if(need_layout_) {
		// Restore old surface. In the future this phase will not be needed
		// since all will be redrawn when needed with dirty rects. Since that
		// doesn't work yet we need to undraw the window.
		if(restorer_) {
			SDL_Rect rect = get_rect();
			SDL_BlitSurface(restorer_, 0, frame_buffer, &rect);
			// Since the old area might be bigger as the new one, invalidate
			// it.
			update_rect(rect);
		}

		if(gui2::new_widgets) {
			NEW_layout();
		} else {
			layout();
		}

		// Get new surface for restoring
		SDL_Rect rect = get_rect();
		// We want the labels underneath the window so draw them and use them
		// as restore point.
		font::draw_floating_labels(frame_buffer);
		restorer_ = get_surface_portion(frame_buffer, rect);

		// Need full redraw so only set ourselves dirty.
		dirty_list_.push_back(std::vector<twidget*>(1, this));
	} else {

		// Find the widgets that are dirty.
		std::vector<twidget*> call_stack(1, this);
		populate_dirty_list(*this, call_stack);
	}

	if(tooltip_.is_visible() && tooltip_.get_dirty()) {
		dirty_list_.push_back(std::vector<twidget*>(1, &tooltip_));
	}

	if(dirty_list_.empty()) {
		if(preferences::use_colour_cursors() || sunset_) {
			surface frame_buffer = get_video_surface();

			if(sunset_) {
				static unsigned i = 0;
				if(++i % sunset_ == 0) {
					SDL_Rect r = {0, 0, frame_buffer->w, frame_buffer->h };
					const Uint32 color =
							SDL_MapRGBA(frame_buffer->format,0,0,0,255);

					fill_rect_alpha(r, color, 1, frame_buffer);
					update_rect(r);
				}
			}

			cursor::draw(frame_buffer);
			video_.flip();
			cursor::undraw(frame_buffer);
		}
		return;
	}

	foreach(std::vector<twidget*>& item, dirty_list_) {

		assert(!item.empty());

		const SDL_Rect dirty_rect = item.back()->get_dirty_rect();

// For testing we disable the clipping rect and force the entire screen to
// update. This way an item rendered at the wrong place is directly visible.
#if 0
		dirty_list_.clear();
		dirty_list_.push_back(std::vector<twidget*>(1, this));
		update_rect(screen_area());
#else
		clip_rect_setter clip(frame_buffer, dirty_rect);
#endif

		/*
		 * The actual update routine does the following:
		 * - Restore the background.
		 *
		 * - draw [begin, end) the back ground of all widgets.
		 *
		 * - draw the children of the last item in the list, if this item is
		 *   a container it's children get a full redraw. If it's not a
		 *   container nothing happens.
		 *
		 * - draw [rbegin, rend) the fore ground of all widgets. For items
		 *   which have two layers eg window or panel it draws the foreground
		 *   layer. For other widgets it's a nop.
		 */

		for(std::vector<twidget*>::iterator itor = item.begin();
				itor != item.end(); ++itor) {

			if(!(**itor).is_visible()
					|| (**itor).get_drawing_action() == twidget::NOT_DRAWN) {

				item.erase(itor, item.end());
				break;
			}
		}

		// Restore.
		SDL_Rect rect = get_rect();
		SDL_BlitSurface(restorer_, 0, frame_buffer, &rect);

		// Background.
		for(std::vector<twidget*>::iterator itor = item.begin();
				itor != item.end(); ++itor) {

			(**itor).draw_background(frame_buffer);
		}

		// Children.
		item.back()->draw_children(frame_buffer);

		// Foreground.
		for(std::vector<twidget*>::reverse_iterator ritor = item.rbegin();
				ritor != item.rend(); ++ritor) {

			(**ritor).draw_foreground(frame_buffer);
			(**ritor).set_dirty(false);
		}

		update_rect(dirty_rect);
	}

	dirty_list_.clear();

	SDL_Rect rect = get_rect();
	update_rect(rect);
	cursor::draw(frame_buffer);
	video_.flip();
	cursor::undraw(frame_buffer);
}

void twindow::window_resize(tevent_handler&,
		const unsigned new_width, const unsigned new_height)
{
	settings::gamemap_width += new_width - settings::screen_width ;
	settings::gamemap_height += new_height - settings::screen_height ;
	settings::screen_width = new_width;
	settings::screen_height = new_height;
	invalidate_layout();
}

void twindow::key_press(tevent_handler& /*event_handler*/, bool& handled,
		SDLKey key, SDLMod /*modifier*/, Uint16 /*unicode*/)
{
	if(key == SDLK_KP_ENTER || key == SDLK_RETURN) {
		set_retval(OK);
		handled = true;
	} else if(key == SDLK_ESCAPE && !escape_disabled_) {
		set_retval(CANCEL);
		handled = true;
	} else if(key == SDLK_SPACE) {
		handled = easy_close();
	}
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	if(key == SDLK_F12) {
		debug_layout_->generate_dot_file(
				"manual", tdebug_layout_graph::MANUAL);
		handled = true;
	}
#endif
}

void twindow::add_easy_close_blocker(const std::string& id)
{
	// avoid duplicates.
	remove_easy_close_blocker(id);

	easy_close_blocker_.push_back(id);
}

void twindow::remove_easy_close_blocker(const std::string& id)
{
	easy_close_blocker_.erase(
		std::remove(easy_close_blocker_.begin(), easy_close_blocker_.end(), id),
		easy_close_blocker_.end());
}

void twindow::init_linked_size_group(const std::string& id,
		const bool fixed_width, const bool fixed_height)
{
	assert(fixed_width || fixed_height);
	assert(linked_size_.find(id) == linked_size_.end());

	linked_size_[id] = tlinked_size(fixed_width, fixed_height);
}

void twindow::add_linked_widget(const std::string& id, twidget* widget)
{
	assert(widget);
	assert(linked_size_.find(id) != linked_size_.end());

	linked_size_[id].widgets.push_back(widget);
}

void twindow::layout()
{
	/**** Initialize and get initial size. *****/

	boost::intrusive_ptr<const twindow_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const twindow_definition::tresolution>
		(config());
	assert(conf);

	log_scope2(log_gui_layout, "Window: Recalculate size");

	layout_init();
	generate_dot_file("layout_init", LAYOUT);

	const game_logic::map_formula_callable variables =
		get_screen_size_variables();

	const int maximum_width = automatic_placement_ ?
			settings::screen_width :  w_(variables);

	const int maximum_height = automatic_placement_ ?
			settings::screen_height :  h_(variables);

	/**
	 * @todo Need to document this new feature in the size algorithm. Also the
	 * fixed size only works when a widget doesn't get a second layout phase.
	 */
	// evaluate the group sizes
	typedef std::pair<const std::string, tlinked_size> hack;
	foreach(hack& linked_size, linked_size_) {

		tpoint max_size(0, 0);

		// Determine the maximum size.
		foreach(twidget* widget, linked_size.second.widgets) {

			const tpoint size = widget->get_best_size();

			if(size.x > max_size.x) {
				max_size.x = size.x;
			}
			if(size.y > max_size.y) {
				max_size.y = size.y;
			}
		}

		// Set the maximum size.
		foreach(twidget* widget, linked_size.second.widgets) {

			tpoint size = widget->get_best_size();

			if(linked_size.second.width) {
				size.x = max_size.x;
			}
			if(linked_size.second.height) {
				size.y = max_size.y;
			}

			widget->set_layout_size(size);
		}
	}

	tpoint size = get_best_size();
	generate_dot_file("get_initial_best_size", LAYOUT);

	DBG_GUI_L << "twindow " << __func__ << ": " << size << " maximum size "
			<< maximum_width << ',' << maximum_height << ".\n";

	/***** Does the width fit in the available width? *****/

	// *** wrap (can change height)
	if(size.x > maximum_width && can_wrap()) {
		layout_wrap(maximum_width);
		size = get_best_size();
		generate_dot_file("wrapped", LAYOUT);
	}

	// *** scrollbar (leaves height untouched)
	if(size.x > maximum_width && has_horizontal_scrollbar()) {
		layout_use_horizontal_scrollbar(maximum_width);
		size = get_best_size();
		generate_dot_file("horizontal_scrollbar", LAYOUT);
	}

	// *** failed?
	if(size.x > maximum_width) {
		ERR_GUI_L << "Failed to resize window, wanted width "
			<< size.x << " available width "
			<< maximum_width << ".\n";

		// Fall back to the new layout engine.
		layout2(maximum_width, maximum_height);
		size = get_best_size();
	}

	/***** Does the height fit in the available height? ******/

	// *** scrollbar (leaves width untouched)
	if(size.y > maximum_height && has_vertical_scrollbar()) {
		layout_use_vertical_scrollbar(maximum_height);
		size = get_best_size();
		generate_dot_file("vertical_scrollbar", LAYOUT);
	}

	// *** shrink (can change width)
	if(size.y > maximum_height) {
		layout_shrink_height(maximum_height);
		size = get_best_size();
		generate_dot_file("shrink_height", LAYOUT);
	}

	// *** failed?
	if(size.y > maximum_height) {
		ERR_GUI_L << "Failed to resize window, wanted height "
			<< size.y << " available height "
			<< maximum_height << ".\n";

		// Fall back to the new layout engine.
		layout2(maximum_width, maximum_height);
		size = get_best_size();
	}

	if(size.x > maximum_width || size.y > maximum_height) {

		std::stringstream sstr;
		sstr << __FILE__ << ":" << __LINE__ << " in function '" << __func__
				<< "' found the following problem: Failed to size window;"
				<< " wanted size " << size
				<< " available size "
				<< maximum_width << ',' << maximum_height
				<< " screen size "
				<< settings::screen_width << ',' << settings::screen_height
				<< '.';

		throw twml_exception(_("Failed to show a dialog, "
				"which doesn't fit on the screen."), sstr.str());
	}

	/***** Get the best location for the window *****/

	tpoint origin(0, 0);

	if(automatic_placement_) {

		switch(horizontal_placement_) {
			case tgrid::HORIZONTAL_ALIGN_LEFT :
				// Do nothing
				break;
			case tgrid::HORIZONTAL_ALIGN_CENTER :
				origin.x = (settings::screen_width - size.x) / 2;
				break;
			case tgrid::HORIZONTAL_ALIGN_RIGHT :
				origin.x = settings::screen_width - size.x;
				break;
			default :
				assert(false);
		}
		switch(vertical_placement_) {
			case tgrid::VERTICAL_ALIGN_TOP :
				// Do nothing
				break;
			case tgrid::VERTICAL_ALIGN_CENTER :
				origin.y = (settings::screen_height - size.y) / 2;
				break;
			case tgrid::VERTICAL_ALIGN_BOTTOM :
				origin.y = settings::screen_height - size.y;
				break;
			default :
				assert(false);
		}
	} else {
		origin.x = x_(variables);
		origin.y = y_(variables);

		size.x = w_(variables);
		size.y = h_(variables);
	}

	/***** Set the window size *****/
	set_size(origin, size);

	generate_dot_file("layout_finished", LAYOUT);
	need_layout_ = false;

	// The widgets might have moved so set the mouse location properly.
	init_mouse_location();
}

void twindow::layout2(
		const unsigned maximum_width, const unsigned maximum_height)
{
	/**
	 * @todo The following features are not done yet:
	 * - Linked widget support.
	 */

	DBG_GUI_L << "twindow " << __func__ << ": maximum size "
			<< maximum_width << ',' << maximum_height << ".\n";

	layout_init2(true);

	bool done = false;
	unsigned run = 1;
	while(!done) {
		done = true;

		tpoint size = get_best_size();
		generate_dot_file("get_initial_best_size-"
				+ lexical_cast<std::string>(run), LAYOUT);

		try {

			/***** Does the width fit in the available width? *****/
			if(size.x > static_cast<int>(maximum_width)) {
				layout_fit_width(maximum_width, FORCE);
				size = get_best_size();
				generate_dot_file("layout_fit_width-"
						+ lexical_cast<std::string>(run), LAYOUT);
				assert(size.x <= static_cast<int>(maximum_width));
			}

			/***** Does the height fit in the available height? *****/
			// This part hasn't been implemented yet.
#if 0
			if(size.y > maximum_height) {
				layout_fit_height(maximum_height);
				size = get_best_size();
				generate_dot_file("layout_fit_height-"
						+ lexical_cast<std::string>(run), LAYOUT);
				assert(size.y <= maximum_height);
			}
#endif
		} catch(trelayout_exception&) {
			layout_init2(false);
			done = false;
			++run;
		}
	}
}

void twindow::NEW_layout()
{
	/**** Initialize and get initial size. *****/

	boost::intrusive_ptr<const twindow_definition::tresolution> conf =
		boost::dynamic_pointer_cast<const twindow_definition::tresolution>
		(config());
	assert(conf);

	log_scope2(log_gui_layout, "Window: Recalculate size");

	NEW_layout_init(true);
	generate_dot_file("layout_init", LAYOUT);

	const game_logic::map_formula_callable variables =
		get_screen_size_variables();

	const int maximum_width = automatic_placement_ ?
			settings::screen_width : w_(variables);

	const int maximum_height = automatic_placement_ ?
			settings::screen_height : h_(variables);

	/** @todo Handle linked widgets. */

	try {
		twindow_implementation::NEW_layout(*this, maximum_width, maximum_height);
	} catch(tlayout_exception_resize_failed&) {

		/** @todo implement the scrollbars on the window. */

		std::stringstream sstr;
		sstr << __FILE__ << ":" << __LINE__ << " in function '" << __func__
				<< "' found the following problem: Failed to size window;"
				<< " wanted size " << get_best_size()
				<< " available size "
				<< maximum_width << ',' << maximum_height
				<< " screen size "
				<< settings::screen_width << ',' << settings::screen_height
				<< '.';

		throw twml_exception(_("Failed to show a dialog, "
				"which doesn't fit on the screen."), sstr.str());
	}

	tpoint size = get_best_size();

	assert(size.x <= maximum_width && size.y <= maximum_height);

	/***** Get the best location for the window *****/

	tpoint origin(0, 0);

	if(automatic_placement_) {

		switch(horizontal_placement_) {
			case tgrid::HORIZONTAL_ALIGN_LEFT :
				// Do nothing
				break;
			case tgrid::HORIZONTAL_ALIGN_CENTER :
				origin.x = (settings::screen_width - size.x) / 2;
				break;
			case tgrid::HORIZONTAL_ALIGN_RIGHT :
				origin.x = settings::screen_width - size.x;
				break;
			default :
				assert(false);
		}
		switch(vertical_placement_) {
			case tgrid::VERTICAL_ALIGN_TOP :
				// Do nothing
				break;
			case tgrid::VERTICAL_ALIGN_CENTER :
				origin.y = (settings::screen_height - size.y) / 2;
				break;
			case tgrid::VERTICAL_ALIGN_BOTTOM :
				origin.y = settings::screen_height - size.y;
				break;
			default :
				assert(false);
		}
	} else {
		origin.x = x_(variables);
		origin.y = y_(variables);

		size.x = w_(variables);
		size.y = h_(variables);
	}

	/***** Set the window size *****/
	set_size(origin, size);

	generate_dot_file("layout_finished", LAYOUT);
	need_layout_ = false;

	// The widgets might have moved so set the mouse location properly.
	init_mouse_location();
}

void twindow::do_show_tooltip(const tpoint& location, const t_string& tooltip)
{
	DBG_GUI_G << "Showing tooltip message: '" << tooltip << "'.\n";

	assert(!tooltip.empty());

	twidget* widget = find_widget(location, true);
	assert(widget);

	const SDL_Rect widget_rect = widget->get_rect();
	const SDL_Rect client_rect = get_client_rect();

	tooltip_.set_label(tooltip);
	const tpoint size = tooltip_.get_best_size();

	SDL_Rect tooltip_rect = {
		(settings::screen_width - size.x) / 2
		, settings::screen_height - size.y
		, size.x
		, size.y
		};
#if 0
	// Find the best position to place the widget
	if(widget_rect.y - size.y > 0) {
		// put above
		tooltip_rect.y = widget_rect.y - size.y;
	} else {
		//put below no test
		tooltip_rect.y = widget_rect.y + widget_rect.h;
	}

	if(widget_rect.x + size.x < client_rect.w) {
		// Directly above the mouse
		tooltip_rect.x = widget_rect.x;
	} else {
		// shift left, no test
		tooltip_rect.x = client_rect.w - size.x;
	}
#endif

	tooltip_.set_size(
			tpoint(tooltip_rect.x, tooltip_rect.y),
			tpoint(tooltip_rect.w, tooltip_rect.h));

	tooltip_.set_visible(twidget::VISIBLE);

	tooltip_restorer_= get_surface_portion(video_.getSurface(), tooltip_rect);
}

void twindow::do_remove_tooltip()
{
	SDL_Rect rect = tooltip_.get_rect();
	SDL_BlitSurface(tooltip_restorer_, 0, video_.getSurface(), &rect);
	update_rect(tooltip_.get_rect());

	tooltip_.set_visible(twidget::HIDDEN);
}

void twindow::do_show_help_popup(const tpoint& location, const t_string& help_popup)
{
	// Note copy past of twindow::do_show_tooltip except that the help may be empty.
	DBG_GUI_G << "Showing help message: '" << help_popup << "'.\n";

	if(help_popup.empty()) {
		return;
	}
	twidget* widget = find_widget(location, true);
	assert(widget);

	const SDL_Rect widget_rect = widget->get_rect();
	const SDL_Rect client_rect = get_client_rect();

	help_popup_.set_label(help_popup);
	const tpoint size = help_popup_.get_best_size();

	SDL_Rect help_popup_rect = {0, 0, size.x, size.y};

	// Find the best position to place the widget
	if(widget_rect.y - size.y > 0) {
		// put above
		help_popup_rect.y = widget_rect.y - size.y;
	} else {
		//put below no test
		help_popup_rect.y = widget_rect.y + widget_rect.h;
	}

	if(widget_rect.x + size.x < client_rect.w) {
		// Directly above the mouse
		help_popup_rect.x = widget_rect.x;
	} else {
		// shift left, no test
		help_popup_rect.x = client_rect.w - size.x;
	}

	help_popup_.set_size(
			tpoint(help_popup_rect.w, help_popup_rect.h),
			tpoint(help_popup_rect.x, help_popup_rect.y));

	help_popup_.set_visible(twidget::VISIBLE);
}

bool twindow::easy_close()
{
	if(does_easy_close()) {
		set_retval(OK);
		return true;
	}
	return false;
}

void twindow::draw(surface& /*surf*/, const bool /*force*/,
		const bool /*invalidate_background*/)
{
	assert(false);
}

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS

void twindow::generate_dot_file(const std::string& generator,
		const unsigned domain)
{
	debug_layout_->generate_dot_file(generator, domain);
}
#endif

void twindow_implementation::NEW_layout(twindow& window,
		const unsigned maximum_width, const unsigned maximum_height)
{
	log_scope2(log_gui_layout, std::string("Window: ") + __func__);

	/*
	 * For now we return the status, need to test later whether this can
	 * entirely be converted to an exception based system as in 'promised' on
	 * the algorithm page.
	 */

	try {
		tpoint size = window.get_best_size();

		DBG_GUI_L << "Best size : " << size
				<< " maximum size : " << maximum_width
				<< ',' << maximum_height
				<< ".\n";
		if(size.x <= static_cast<int>(maximum_width)
				&& size.y <= static_cast<int>(maximum_height)) {

			DBG_GUI_L << "Result: Fits, nothing to do.\n";
			return;
		}

		if(size.x > static_cast<int>(maximum_width)) {
			window.NEW_reduce_width(maximum_width);

			size = window.get_best_size();
			if(size.x > static_cast<int>(maximum_width)) {
				DBG_GUI_L << "Result: Resize width failed."
					<< " Wanted width " << maximum_width
					<< " resulting width " << size.x
					<< ".\n";
				throw tlayout_exception_width_resize_failed();
			}
			DBG_GUI_L << "Status: Resize width succeeded.\n";
		}

		if(size.y > static_cast<int>(maximum_height)) {
			window.NEW_reduce_height(maximum_height);

			size = window.get_best_size();
			if(size.y > static_cast<int>(maximum_height)) {
				DBG_GUI_L << "Result: Resize height failed."
					<< " Wanted height " << maximum_height
					<< " resulting height " << size.y
					<< ".\n";
				throw tlayout_exception_height_resize_failed();
			}
			DBG_GUI_L << "Status: Resize height succeeded.\n";
		}

		assert(size.x <= static_cast<int>(maximum_width)
				&& size.y <= static_cast<int>(maximum_height));


		DBG_GUI_L << "Result: Resizing succeeded.\n";
		return;

	} catch (tlayout_exception_width_modified&) {
		DBG_GUI_L << "Status: Width has been modified, rerun.\n";
		window.NEW_layout_init(false);
		NEW_layout(window, maximum_width, maximum_height);
		return;
	}
}

} // namespace gui2


/**
 * @page layout_algorihm Layout algorithm
 *
 * @section introduction Introduction
 *
 * This page describes how the layout engine for the dialogs works. First
 * a global overview of some terms used in this document.
 *
 * - @ref gui2::twidget "Widget"; Any item which can be used in the widget
 *   toolkit. Not all widgets are visible. In general widgets can not be
 *   sized directly, but this is controlled by a window. A widget has an
 *   internal size cache and if the value in the cache is not equal to 0,0
 *   that value is its best size. This value gets set when the widget can
 *   honour a resize request.  It will be set with the value which honours
 *   the request.
 *
 * - @ref gui2::tgrid "Grid"; A grid is an invisible container which holds
 *   one or more widgets.  Several widgets have a grid in them to hold
 *   multiple widgets eg panels and windows.
 *
 * - @ref gui2::tgrid::tchild "Grid cell"; Every widget which is in a grid is
 *   put in a grid cell. These cells also hold the information about the gaps
 *   between widgets the behaviour on growing etc. All grid cells must have a
 *   widget inside them.
 *
 * - @ref gui2::twindow "Window"; A window is a top level item which has a
 *   grid with its children. The window handles the sizing of the window and
 *   makes sure everything fits.
 *
 * - @ref gui2::twindow::tlinked_size "Shared size group"; A shared size
 *   group is a number of widgets which share width and or height. These
 *   widgets are handled separately in the layout algorithm. All grid cells
 *   width such a widget will get the same height and or width and these
 *   widgets won't be resized when there's not enough size. To be sure that
 *   these widgets don't cause trouble for the layout algorithm, they must be
 *   in a container with scrollbars so there will always be a way to properly
 *   layout them. The engine must enforce this restriction so the shared
 *   layout property must be set by the engine after validation.
 *
 * - All visible grid cells; A grid cell is visible when the widget inside
 *   of it doesn't have the state INVISIBLE. Widgets which are HIDDEN are
 *   sized properly since when they become VISIBLE the layout shouldn't be
 *   invalidated. A grid cell that's invisible has size 0,0.
 *
 * - All resizable grid cells; A grid cell is resizable under the following
 *   conditions:
 *   - The widget is VISIBLE.
 *   - The widget is not in a shared size group.
 *
 * There are two layout algorithms with a different purpose.
 *
 * - The Window algorithm; this algorithm's goal is it to make sure all grid
 *   cells fit in the window. Sizing the grid cells depends on the widget
 *   size as well, but this algorithm only sizes the grid cells and doesn't
 *   handle the widgets inside them.
 *
 * - The Grid algorithm; after the Window algorithm made sure that all grid
 *   cells fit this algorithm makes sure the widgets are put in the optimal
 *   state in their grid cell.
 *
 * @section layout_algorihm_window Window
 *
 * Here is the algorithm used to layout the window:
 *
 * - Perform a full initialization
 *   (@ref gui2::twidget::NEW_layout_init (full_initialization = true)):
 *   - Clear the internal best size cache for all widgets.
 *   - For widgets with scrollbars hide them unless the
 *     @ref gui2::tscrollbar_container::tscrollbar_mode "scrollbar_mode" is
 *     always_visible or auto_visible.
 * - Handle shared sizes:
 *   - Height and width:
 *     - Get the best size for all widgets that share height and width.
 *     - Set the maximum of width and height as best size for all these
 *       widgets.
 *   - Width only:
 *     - Get the best width for all widgets which share their width.
 *     - Set the maximum width for all widgets, but keep their own height.
 *   - Height only:
 *     - Get the best height for all widgets which share their height.
 *     - Set the maximum height for all widgets, but keep their own width.
 * - Start layout loop:
 *   - Get best size.
 *   - If width <= maximum_width && height <= maximum_height we're done.
 *   - If width > maximum_width, optimize the width:
 *     - For every grid cell in a grid row there will be a resize request
 *       (@ref gui2::tgrid::NEW_reduce_width):
 *       - Sort the widgets in the row on the resize priority.
 *         - Loop through this priority queue until the row fits
 *           - If priority != 0 try to share the extra width else all
 *             widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by either wrapping or using a
 *             scrollbar (@ref gui2::twidget::NEW_request_reduce_width).
 *           - If the row fits in the wanted width this row is done.
 *           - Else try the next priority.
 *         - All priorities done and the width still doesn't fit.
 *         - Loop through this priority queue until the row fits.
 *           - If priority != 0:
 *             - try to share the extra width
 *           -Else:
 *             - All widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by sizing them smaller as really
 *             wanted (@ref gui2::twidget::NEW_demand_reduce_width).
 *             For labels, buttons etc. they get ellipsized.
 *           - If the row fits in the wanted width this row is done.
 *           - Else try the next priority.
 *         - All priorities done and the width still doesn't fit.
 *         - Throw a layout width doesn't fit exception.
 *   - If height > maximum_height, optimize the height
 *       (@ref gui2::tgrid::NEW_reduce_height):
 *     - For every grid cell in a grid column there will be a resize request:
 *       - Sort the widgets in the column on the resize priority.
 *         - Loop through this priority queue until the column fits:
 *           - If priority != 0 try to share the extra height else all
 *              widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by using a scrollbar
 *             (@ref gui2::twidget::NEW_request_reduce_height).
 *             - If succeeded for a widget the width is influenced and the
 *               width might be invalid.
 *             - Throw a width modified exception.
 *           - If the column fits in the wanted height this column is done.
 *           - Else try the next priority.
 *         - All priorities done and the height still doesn't fit.
 *         - Loop through this priority queue until the column fits.
 *           - If priority != 0 try to share the extra height else all
 *             widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by sizing them smaller as really
 *             wanted (@ref gui2::twidget::NEW_demand_reduce_width).
 *             For labels, buttons etc. they get ellipsized .
 *           - If the column fits in the wanted height this column is done.
 *           - Else try the next priority.
 *         - All priorities done and the height still doesn't fit.
 *         - Throw a layout height doesn't fit exception.
 * - End layout loop.
 *
 * - Catch @ref gui2::tlayout_exception_width_modified "width modified":
 *   - Goto relayout.
 *
 * - Catch
 *   @ref gui2::tlayout_exception_width_resize_failed "width resize failed":
 *   - If the window has a horizontal scrollbar which isn't shown but can be
 *     shown.
 *     - Show the scrollbar.
 *     - goto relayout.
 *   - Else show a layout failure message.
 *
 * - Catch
 *   @ref gui2::tlayout_exception_height_resize_failed "height resize failed":
 *   - If the window has a vertical scrollbar which isn't shown but can be
 *     shown:
 *     - Show the scrollbar.
 *     - goto relayout.
 *   - Else:
 *     - show a layout failure message.
 *
 * - Relayout:
 *   - Initialize all widgets
 *     (@ref gui2::twidget::NEW_layout_init (full_initialization = false))
 *   - Goto start layout loop.
 *
 * @section grid Grid
 *
 * This section will be documented later.
 */
