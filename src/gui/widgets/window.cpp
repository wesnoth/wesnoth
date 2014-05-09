/*
   Copyright (C) 2007 - 2014 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

/**
 *  @file
 *  Implementation of window.hpp.
 */

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/widgets/window_private.hpp"

#include "font.hpp"
#include "game_display.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "gui/auxiliary/event/distributor.hpp"
#include "gui/auxiliary/event/message.hpp"
#include "gui/auxiliary/find_widget.tpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/layout_exception.hpp"
#include "gui/auxiliary/window_builder/control.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/tip.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/detail/register.tpp"
#include "gui/widgets/settings.hpp"
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif
#include "preferences.hpp"
#include "preferences_display.hpp"
#include "utils/foreach.tpp"
#include "video.hpp"

#include <boost/bind.hpp>

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

#define LOG_IMPL_SCOPE_HEADER                                                  \
	window.get_control_type() + " [" + window.id() + "] " + __func__
#define LOG_IMPL_HEADER LOG_IMPL_SCOPE_HEADER + ':'

namespace gui2
{

namespace implementation
{
/** @todo See whether this hack can be removed. */
// Needed to fix a compiler error in REGISTER_WIDGET.
class tbuilder_window : public tbuilder_control
{
public:
	tbuilder_window(const config& cfg) : tbuilder_control(cfg)
	{
	}

	using tbuilder_control::build;

	twidget* build() const
	{
		return NULL;
	}
};

} // namespace implementation
REGISTER_WIDGET(window)

unsigned twindow::sunset_ = 0;

namespace
{
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
const unsigned SHOW = tdebug_layout_graph::SHOW;
const unsigned LAYOUT = tdebug_layout_graph::LAYOUT;
#else
// values are irrelavant when DEBUG_WINDOW_LAYOUT_GRAPHS is not defined.
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
 * Adds a SHOW_HELPTIP event to the SDL event queue.
 *
 * The event is used to show the helptip for the currently focussed widget.
 */
static bool helptip()
{
	DBG_GUI_E << "Pushing SHOW_HELPTIP_EVENT event in queue.\n";

	SDL_Event event;
	SDL_UserEvent data;

	data.type = SHOW_HELPTIP_EVENT;
	data.code = 0;
	data.data1 = NULL;
	data.data2 = NULL;

	event.type = SHOW_HELPTIP_EVENT;
	event.user = data;

	SDL_PushEvent(&event);

	return true;
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

tmanager::tmanager() : windows_()
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
		itor != windows_.end();
		++itor) {

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
		itor != windows_.end();
		++itor) {

		if(itor->second == &window) {
			return itor->first;
		}
	}
	assert(false);

	return 0;
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
				 tformula<unsigned> x,
				 tformula<unsigned> y,
				 tformula<unsigned> w,
				 tformula<unsigned> h,
				 tformula<bool> reevaluate_best_size,
				 const game_logic::function_symbol_table& functions,
				 const bool automatic_placement,
				 const unsigned horizontal_placement,
				 const unsigned vertical_placement,
				 const unsigned maximum_width,
				 const unsigned maximum_height,
				 const std::string& definition,
				 const twindow_builder::tresolution::ttip& tooltip,
				 const twindow_builder::tresolution::ttip& helptip)
	: tpanel()
	, cursor::setter(cursor::NORMAL)
	, video_(video)
	, status_(NEW)
	, show_mode_(none)
	, retval_(NONE)
	, owner_(0)
	, need_layout_(true)
	, variables_()
	, invalidate_layout_blocked_(false)
	, suspend_drawing_(true)
	, restorer_()
	, automatic_placement_(automatic_placement)
	, horizontal_placement_(horizontal_placement)
	, vertical_placement_(vertical_placement)
	, maximum_width_(maximum_width)
	, maximum_height_(maximum_height)
	, x_(x)
	, y_(y)
	, w_(w)
	, h_(h)
	, reevaluate_best_size_(reevaluate_best_size)
	, functions_(functions)
	, tooltip_(tooltip)
	, helptip_(helptip)
	, click_dismiss_(false)
	, enter_disabled_(false)
	, escape_disabled_(false)
	, linked_size_()
	, mouse_button_state_(0) /**< Needs to be initialised in @ref show. */
	, dirty_list_()
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, debug_layout_(new tdebug_layout_graph(this))
#endif
	, event_distributor_(
			  new event::tdistributor(*this, event::tdispatcher::front_child))
{
	// We load the config in here as exception.
	// Our caller did update the screen size so no need for us to do that again.
	set_definition(definition);
	load_config();

	tmanager::instance().add(*this);

	connect();

	connect_signal<event::DRAW>(boost::bind(&twindow::draw, this));

	connect_signal<event::SDL_VIDEO_RESIZE>(boost::bind(
			&twindow::signal_handler_sdl_video_resize, this, _2, _3, _5));

	connect_signal<event::SDL_ACTIVATE>(boost::bind(
			&event::tdistributor::initialize_state, event_distributor_));

	connect_signal<event::SDL_LEFT_BUTTON_UP>(
			boost::bind(&twindow::signal_handler_click_dismiss,
						this,
						_2,
						_3,
						_4,
						SDL_BUTTON_LMASK),
			event::tdispatcher::front_child);
	connect_signal<event::SDL_MIDDLE_BUTTON_UP>(
			boost::bind(&twindow::signal_handler_click_dismiss,
						this,
						_2,
						_3,
						_4,
						SDL_BUTTON_MMASK),
			event::tdispatcher::front_child);
	connect_signal<event::SDL_RIGHT_BUTTON_UP>(
			boost::bind(&twindow::signal_handler_click_dismiss,
						this,
						_2,
						_3,
						_4,
						SDL_BUTTON_RMASK),
			event::tdispatcher::front_child);

	connect_signal<event::SDL_KEY_DOWN>(
			boost::bind(
					&twindow::signal_handler_sdl_key_down, this, _2, _3, _5),
			event::tdispatcher::back_pre_child);
	connect_signal<event::SDL_KEY_DOWN>(boost::bind(
			&twindow::signal_handler_sdl_key_down, this, _2, _3, _5));

	connect_signal<event::MESSAGE_SHOW_TOOLTIP>(
			boost::bind(&twindow::signal_handler_message_show_tooltip,
						this,
						_2,
						_3,
						_5),
			event::tdispatcher::back_pre_child);

	connect_signal<event::MESSAGE_SHOW_HELPTIP>(
			boost::bind(&twindow::signal_handler_message_show_helptip,
						this,
						_2,
						_3,
						_5),
			event::tdispatcher::back_pre_child);

	connect_signal<event::REQUEST_PLACEMENT>(
			boost::bind(
					&twindow::signal_handler_request_placement, this, _2, _3),
			event::tdispatcher::back_pre_child);

	register_hotkey(hotkey::GLOBAL__HELPTIP, boost::bind(gui2::helptip));
}

twindow::~twindow()
{
	/*
	 * We need to delete our children here instead of waiting for the grid to
	 * automatically do it. The reason is when the grid deletes its children
	 * they will try to unregister them self from the linked widget list. At
	 * this point the member of twindow are destroyed and we enter UB. (For
	 * some reason the bug didn't trigger on g++ but it does on MSVC.
	 */
	for(unsigned row = 0; row < grid().get_rows(); ++row) {
		for(unsigned col = 0; col < grid().get_cols(); ++col) {
			grid().remove_child(row, col);
		}
	}

	/*
	 * The tip needs to be closed if the window closes and the window is
	 * not a tip. If we don't do that the tip will unrender in the next
	 * window and cause drawing glitches.
	 * Another issue is that on smallgui and an MP game the tooltip not
	 * unrendered properly can capture the mouse and make playing impossible.
	 */
	if(show_mode_ == modal) {
		tip::remove();
	}

	tmanager::instance().remove(*this);

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS

	delete debug_layout_;

#endif
	delete event_distributor_;
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

		display* display = display::get_singleton();
		if(display) {
			const SDL_Rect rect_gm = display->map_outside_area();

			if(rect_gm.w && rect_gm.h) {
				settings::gamemap_width = rect_gm.w;
				settings::gamemap_height = rect_gm.h;
				settings::gamemap_x_offset = rect_gm.x;
			}
		}
	}
}

/*WIKI
 * @page = GUIToolkitWML
 * @order = 3_widget_window_2
 *
 * List if the id's that have generate a return value:
 * * ok confirms the dialog.
 * * cancel cancels the dialog.
 *
 */
twindow::tretval twindow::get_retval_by_id(const std::string& id)
{
	// Note it might change to a map later depending on the number
	// of items.
	if(id == "ok") {
		return OK;
	} else if(id == "cancel") {
		return CANCEL;

		/**
		 * The ones for the title screen.
		 *
		 * This is a kind of hack, but the values are hardcoded in the
		 * titlescreen and don't want to change them at the moment. It would be
		 * a good idea to
		 * add some namespaces to avoid names clashing.
		 */
	} else if(id == "tutorial") {
		return static_cast<tretval>(ttitle_screen::TUTORIAL);
	} else if(id == "editor") {
		return static_cast<tretval>(ttitle_screen::START_MAP_EDITOR);
	} else if(id == "credits") {
		return static_cast<tretval>(ttitle_screen::SHOW_ABOUT);
	} else if(id == "quit") {
		return static_cast<tretval>(ttitle_screen::QUIT_GAME);

		/**
		 * The hacks which are here so the old engine can handle the event. The
		 * new engine can't handle all dialogs yet, so it needs to fall back to
		 * the old engine to make certain things happen.
		 */
	} else if(id == "help") {
		return static_cast<tretval>(ttitle_screen::SHOW_HELP);
	} else if(id == "campaign") {
		return static_cast<tretval>(ttitle_screen::NEW_CAMPAIGN);
	} else if(id == "multiplayer") {
		return static_cast<tretval>(ttitle_screen::MULTIPLAYER);
	} else if(id == "load") {
		return static_cast<tretval>(ttitle_screen::LOAD_GAME);
	} else if(id == "addons") {
		return static_cast<tretval>(ttitle_screen::GET_ADDONS);
	} else if(id == "cores") {
		return static_cast<tretval>(ttitle_screen::CORES);
	} else if(id == "language") {
		return static_cast<tretval>(ttitle_screen::CHANGE_LANGUAGE);
	} else if(id == "preferences") {
		return static_cast<tretval>(ttitle_screen::EDIT_PREFERENCES);

		// default if nothing matched
	} else {
		return NONE;
	}
}

void twindow::show_tooltip(/*const unsigned auto_close_timeout*/)
{
	log_scope2(log_gui_draw, "Window: show as tooltip.");

	generate_dot_file("show", SHOW);

	assert(status_ == NEW);

	set_mouse_behavior(event::tdispatcher::none);
	set_want_keyboard_input(false);

	show_mode_ = tooltip;

	/*
	 * Before show has been called, some functions might have done some testing
	 * on the window and called layout, which can give glitches. So
	 * reinvalidate the window to avoid those glitches.
	 */
	invalidate_layout();
	suspend_drawing_ = false;
}

void twindow::show_non_modal(/*const unsigned auto_close_timeout*/)
{
	log_scope2(log_gui_draw, "Window: show non modal.");

	generate_dot_file("show", SHOW);

	assert(status_ == NEW);

	set_mouse_behavior(event::tdispatcher::hit);

	show_mode_ = modal;

	/*
	 * Before show has been called, some functions might have done some testing
	 * on the window and called layout, which can give glitches. So
	 * reinvalidate the window to avoid those glitches.
	 */
	invalidate_layout();
	suspend_drawing_ = false;
}

int twindow::show(const bool restore, const unsigned auto_close_timeout)
{
	/*
	 * Removes the old tip if one shown. The show_tip doesn't remove
	 * the tip, since it's the tip.
	 */
	tip::remove();

	show_mode_ = modal;

	/**
	 * Helper class to set and restore the drawing interval.
	 *
	 * We need to make sure we restore the value when the function ends, be it
	 * normally or due to an exception.
	 */
	class tdraw_interval_setter
	{
	public:
		tdraw_interval_setter() : interval_(draw_interval)
		{
			if(interval_ == 0) {
				draw_interval = 30;
				SDL_AddTimer(draw_interval, draw_timer, NULL);

				// There might be some time between creation and showing so
				// reupdate the sizes.
				update_screen_size();
			}
		}

		~tdraw_interval_setter()
		{
			draw_interval = interval_;
		}

	private:
		int interval_;
	};

	log_scope2(log_gui_draw, LOG_SCOPE_HEADER);

	generate_dot_file("show", SHOW);

	assert(status_ == NEW);

	tdraw_interval_setter draw_interval_setter;

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


	try
	{
		// Start our loop drawing will happen here as well.
		bool mouse_button_state_initialised = false;
		for(status_ = SHOWING; status_ != REQUEST_CLOSE;) {
			// process installed callback if valid, to allow e.g. network
			// polling
			events::pump();

			if(!mouse_button_state_initialised) {
				/*
				 * The state must be initialize when showing the dialogue.
				 * However when initialized before this point there were random
				 * errors. This only happened when the 'click' was done fast; a
				 * slower click worked properly.
				 *
				 * So it seems the events need to be processed before SDL can
				 * return the proper button state. When initializing here all
				 * works fine.
				 */
				mouse_button_state_ = SDL_GetMouseState(NULL, NULL);
				mouse_button_state_initialised = true;
			}

			// Add a delay so we don't keep spinning if there's no event.
			SDL_Delay(10);
		}
	}
	catch(...)
	{
		/**
		 * @todo Clean up the code duplication.
		 *
		 * In the future the restoring shouldn't be needed so the duplication
		 * doesn't hurt too much but keep this todo as a reminder.
		 */
		suspend_drawing_ = true;

		// restore area
		if(restore) {
			SDL_Rect rect = get_rectangle();
			sdl_blit(restorer_, 0, video_.getSurface(), &rect);
			update_rect(get_rectangle());
			font::undraw_floating_labels(video_.getSurface());
		}
		throw;
	}

	suspend_drawing_ = true;

	// restore area
	if(restore) {
		SDL_Rect rect = get_rectangle();
		sdl_blit(restorer_, 0, video_.getSurface(), &rect);
		update_rect(get_rectangle());
		font::undraw_floating_labels(video_.getSurface());
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
			SDL_Rect rect = get_rectangle();
			sdl_blit(restorer_, 0, frame_buffer, &rect);
			// Since the old area might be bigger as the new one, invalidate
			// it.
			update_rect(rect);
		}

		layout();

		// Get new surface for restoring
		SDL_Rect rect = get_rectangle();
		// We want the labels underneath the window so draw them and use them
		// as restore point.
		font::draw_floating_labels(frame_buffer);
		restorer_ = get_surface_portion(frame_buffer, rect);

		// Need full redraw so only set ourselves dirty.
		dirty_list_.push_back(std::vector<twidget*>(1, this));
	} else {

		// Let widgets update themselves, which might dirty some things.
		layout_children();

		// Now find the widgets that are dirty.
		std::vector<twidget*> call_stack;
		if(!new_widgets) {
			populate_dirty_list(*this, call_stack);
		} else {
			/* Force to update and redraw the entire screen */
			dirty_list_.clear();
			dirty_list_.push_back(std::vector<twidget*>(1, this));
			update_rect(screen_area());
		}
	}

	if(dirty_list_.empty()) {
		if(preferences::use_color_cursors() || sunset_) {
			surface frame_buffer = get_video_surface();

			if(sunset_) {
				/** @todo should probably be moved to event::thandler::draw. */
				static unsigned i = 0;
				if(++i % sunset_ == 0) {
					SDL_Rect r = ::create_rect(
							0, 0, frame_buffer->w, frame_buffer->h);
					const Uint32 color
							= SDL_MapRGBA(frame_buffer->format, 0, 0, 0, 255);

					fill_rect_alpha(r, color, 1, frame_buffer);
					update_rect(r);
				}
			}
		}
		return;
	}

	FOREACH(AUTO & item, dirty_list_)
	{

		assert(!item.empty());

		const SDL_Rect dirty_rect
				= new_widgets ? screen_area()
							  : item.back()->get_dirty_rectangle();

// For testing we disable the clipping rect and force the entire screen to
// update. This way an item rendered at the wrong place is directly visible.
#if 0
		dirty_list_.clear();
		dirty_list_.push_back(std::vector<twidget*>(1, this));
		update_rect(screen_area());
#else
		clip_rect_setter clip(frame_buffer, &dirty_rect);
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
		 *
		 * Before drawing there needs to be determined whether a dirty widget
		 * really needs to be redrawn. If the widget doesn't need to be
		 * redrawing either being not tvisible::visible or has status
		 * twidget::tredraw_action::none. If it's not drawn it's still set not
		 * dirty to avoid it keep getting on the dirty list.
		 */

		for(std::vector<twidget*>::iterator itor = item.begin();
			itor != item.end();
			++itor) {

			if((**itor).get_visible() != twidget::tvisible::visible
			   || (**itor).get_drawing_action()
				  == twidget::tredraw_action::none) {

				for(std::vector<twidget*>::iterator citor = itor;
					citor != item.end();
					++citor) {

					(**citor).set_is_dirty(false);
				}

				item.erase(itor, item.end());
				break;
			}
		}

		// Restore.
		SDL_Rect rect = get_rectangle();
		sdl_blit(restorer_, 0, frame_buffer, &rect);

/**
 * @todo Remove the if an always use the true branch.
 *
 * When removing that code the draw functions with only a frame_buffer
 * as parameter should also be removed since they will be unused from
 * that moment.
 */
#if 0
		if(new_widgets)
#else
		if(true)
#endif
		{
			// Background.
			for(std::vector<twidget*>::iterator itor = item.begin();
				itor != item.end();
				++itor) {

				(**itor).draw_background(frame_buffer, 0, 0);
			}

			// Children.
			if(!item.empty()) {
				item.back()->draw_children(frame_buffer, 0, 0);
			}

			// Foreground.
			for(std::vector<twidget*>::reverse_iterator ritor = item.rbegin();
				ritor != item.rend();
				++ritor) {

				(**ritor).draw_foreground(frame_buffer, 0, 0);
				(**ritor).set_is_dirty(false);
			}
		}
		else
		{
			// Background.
			for(std::vector<twidget*>::iterator itor = item.begin();
				itor != item.end();
				++itor) {

				(**itor).draw_background(frame_buffer);
			}

			// Children.
			if(!item.empty()) {
				item.back()->draw_children(frame_buffer);
			}

			// Foreground.
			for(std::vector<twidget*>::reverse_iterator ritor = item.rbegin();
				ritor != item.rend();
				++ritor) {

				(**ritor).draw_foreground(frame_buffer);
				(**ritor).set_is_dirty(false);
			}
		}

		update_rect(dirty_rect);
	}

	dirty_list_.clear();

	std::vector<twidget*> call_stack;
	populate_dirty_list(*this, call_stack);
	assert(dirty_list_.empty());

	SDL_Rect rect = get_rectangle();
	update_rect(rect);
}

void twindow::undraw()
{
	if(restorer_) {
		SDL_Rect rect = get_rectangle();
		sdl_blit(restorer_, 0, video_.getSurface(), &rect);
		// Since the old area might be bigger as the new one, invalidate
		// it.
		update_rect(rect);
	}
}

twindow::tinvalidate_layout_blocker::tinvalidate_layout_blocker(twindow& window)
	: window_(window)
{
	assert(!window_.invalidate_layout_blocked_);
	window_.invalidate_layout_blocked_ = true;
}

twindow::tinvalidate_layout_blocker::~tinvalidate_layout_blocker()
{
	assert(window_.invalidate_layout_blocked_);
	window_.invalidate_layout_blocked_ = false;
}

void twindow::invalidate_layout()
{
	if(!invalidate_layout_blocked_) {
		need_layout_ = true;
	}
}
twidget* twindow::find_at(const tpoint& coordinate, const bool must_be_active)
{
	return tpanel::find_at(coordinate, must_be_active);
}

const twidget* twindow::find_at(const tpoint& coordinate,
								const bool must_be_active) const
{
	return tpanel::find_at(coordinate, must_be_active);
}

twidget* twindow::find(const std::string& id, const bool must_be_active)
{
	return tcontainer_::find(id, must_be_active);
}

const twidget* twindow::find(const std::string& id, const bool must_be_active)
		const
{
	return tcontainer_::find(id, must_be_active);
}

void twindow::init_linked_size_group(const std::string& id,
									 const bool fixed_width,
									 const bool fixed_height)
{
	assert(fixed_width || fixed_height);
	assert(!has_linked_size_group(id));

	linked_size_[id] = tlinked_size(fixed_width, fixed_height);
}

bool twindow::has_linked_size_group(const std::string& id)
{
	return linked_size_.find(id) != linked_size_.end();
}

void twindow::add_linked_widget(const std::string& id, twidget* widget)
{
	assert(widget);
	assert(has_linked_size_group(id));

	std::vector<twidget*>& widgets = linked_size_[id].widgets;
	if(std::find(widgets.begin(), widgets.end(), widget) == widgets.end()) {
		widgets.push_back(widget);
	}
}

void twindow::remove_linked_widget(const std::string& id, const twidget* widget)
{
	assert(widget);
	assert(has_linked_size_group(id));

	std::vector<twidget*>& widgets = linked_size_[id].widgets;

	std::vector<twidget*>::iterator itor
			= std::find(widgets.begin(), widgets.end(), widget);

	if(itor != widgets.end()) {
		widgets.erase(itor);

		assert(std::find(widgets.begin(), widgets.end(), widget)
			   == widgets.end());
	}
}

void twindow::layout()
{
	/***** Initialize. *****/

	boost::intrusive_ptr<const twindow_definition::tresolution>
	conf = boost::dynamic_pointer_cast<const twindow_definition::tresolution>(
			config());
	assert(conf);

	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	const tpoint mouse = get_mouse_position();
	variables_.add("mouse_x", variant(mouse.x));
	variables_.add("mouse_y", variant(mouse.y));
	variables_.add("window_width", variant(0));
	variables_.add("window_height", variant(0));
	variables_.add("size_request_mode", variant("maximum"));
	get_screen_size_variables(variables_);

	const int maximum_width = automatic_placement_ ? maximum_width_
			? std::min(maximum_width_, settings::screen_width)
			: settings::screen_width
			: w_(variables_, &functions_);

	const int maximum_height = automatic_placement_ ? maximum_height_
			? std::min(maximum_height_, settings::screen_height)
			: settings::screen_height
			: h_(variables_, &functions_);

	/***** Handle click dismiss status. *****/
	tbutton* click_dismiss_button = NULL;
	if((click_dismiss_button
		= find_widget<tbutton>(this, "click_dismiss", false, false))) {

		click_dismiss_button->set_visible(twidget::tvisible::invisible);
	}
	if(click_dismiss_) {
		tbutton* button = find_widget<tbutton>(this, "ok", false, false);
		if(button) {
			button->set_visible(twidget::tvisible::invisible);
			click_dismiss_button = button;
		}
		VALIDATE(click_dismiss_button,
				 _("Click dismiss needs a 'click_dismiss' or 'ok' button."));
	}

	/***** Layout. *****/
	layout_initialise(true);
	generate_dot_file("layout_initialise", LAYOUT);

	layout_linked_widgets();

	try
	{
		twindow_implementation::layout(*this, maximum_width, maximum_height);
	}
	catch(tlayout_exception_resize_failed&)
	{

		/** @todo implement the scrollbars on the window. */

		std::stringstream sstr;
		sstr << __FILE__ << ":" << __LINE__ << " in function '" << __func__
			 << "' found the following problem: Failed to size window;"
			 << " wanted size " << get_best_size() << " available size "
			 << maximum_width << ',' << maximum_height << " screen size "
			 << settings::screen_width << ',' << settings::screen_height << '.';

		throw twml_exception(_("Failed to show a dialog, "
							   "which doesn't fit on the screen."),
							 sstr.str());
	}

	/****** Validate click dismiss status. *****/
	if(click_dismiss_ && disable_click_dismiss()) {
		assert(click_dismiss_button);
		click_dismiss_button->set_visible(twidget::tvisible::visible);

		connect_signal_mouse_left_click(
				*click_dismiss_button,
				boost::bind(&twindow::set_retval, this, OK, true));

		layout_initialise(true);
		generate_dot_file("layout_initialise", LAYOUT);

		layout_linked_widgets();

		try
		{
			twindow_implementation::layout(
					*this, maximum_width, maximum_height);
		}
		catch(tlayout_exception_resize_failed&)
		{

			/** @todo implement the scrollbars on the window. */

			std::stringstream sstr;
			sstr << __FILE__ << ":" << __LINE__ << " in function '" << __func__
				 << "' found the following problem: Failed to size window;"
				 << " wanted size " << get_best_size() << " available size "
				 << maximum_width << ',' << maximum_height << " screen size "
				 << settings::screen_width << ',' << settings::screen_height
				 << '.';

			throw twml_exception(_("Failed to show a dialog, "
								   "which doesn't fit on the screen."),
								 sstr.str());
		}
	}

	/***** Get the best location for the window *****/
	tpoint size = get_best_size();

	assert(size.x <= maximum_width && size.y <= maximum_height);

	tpoint origin(0, 0);

	if(automatic_placement_) {

		switch(horizontal_placement_) {
			case tgrid::HORIZONTAL_ALIGN_LEFT:
				// Do nothing
				break;
			case tgrid::HORIZONTAL_ALIGN_CENTER:
				origin.x = (settings::screen_width - size.x) / 2;
				break;
			case tgrid::HORIZONTAL_ALIGN_RIGHT:
				origin.x = settings::screen_width - size.x;
				break;
			default:
				assert(false);
		}
		switch(vertical_placement_) {
			case tgrid::VERTICAL_ALIGN_TOP:
				// Do nothing
				break;
			case tgrid::VERTICAL_ALIGN_CENTER:
				origin.y = (settings::screen_height - size.y) / 2;
				break;
			case tgrid::VERTICAL_ALIGN_BOTTOM:
				origin.y = settings::screen_height - size.y;
				break;
			default:
				assert(false);
		}
	} else {

		variables_.add("window_width", variant(size.x));
		variables_.add("window_height", variant(size.y));

		while(reevaluate_best_size_(variables_, &functions_)) {
			layout_initialise(true);

			twindow_implementation::layout(*this,
										   w_(variables_, &functions_),
										   h_(variables_, &functions_));

			size = get_best_size();
			variables_.add("window_width", variant(size.x));
			variables_.add("window_height", variant(size.y));
		}

		variables_.add("size_request_mode", variant("size"));

		origin.x = x_(variables_, &functions_);
		origin.y = y_(variables_, &functions_);

		size.x = w_(variables_, &functions_);
		size.y = h_(variables_, &functions_);
	}

	/***** Set the window size *****/
	place(origin, size);

	generate_dot_file("layout_finished", LAYOUT);
	need_layout_ = false;

	event::init_mouse_location();
}

void twindow::layout_linked_widgets()
{
	// evaluate the group sizes
	FOREACH(AUTO & linked_size, linked_size_)
	{

		tpoint max_size(0, 0);

		// Determine the maximum size.
		FOREACH(AUTO widget, linked_size.second.widgets)
		{

			const tpoint size = widget->get_best_size();

			if(size.x > max_size.x) {
				max_size.x = size.x;
			}
			if(size.y > max_size.y) {
				max_size.y = size.y;
			}
		}

		// Set the maximum size.
		FOREACH(AUTO widget, linked_size.second.widgets)
		{

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
}

bool twindow::click_dismiss(const Uint8 mouse_button_mask)
{
	if(does_click_dismiss()) {
		if((mouse_button_state_ & mouse_button_mask) == 0) {
			set_retval(OK);
		} else {
			mouse_button_state_ &= ~mouse_button_mask;
		}
		return true;
	}
	return false;
}

const std::string& twindow::get_control_type() const
{
	static const std::string type = "window";
	return type;
}

void twindow::draw(surface& /*surf*/,
				   const bool /*force*/,
				   const bool /*invalidate_background*/)
{
	assert(false);
}

namespace
{

/**
 * Swaps an item in a grid for another one.*/
void swap_grid(tgrid* grid,
			   tgrid* content_grid,
			   twidget* widget,
			   const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	tgrid* parent_grid = NULL;
	if(grid) {
		parent_grid = find_widget<tgrid>(grid, id, false, false);
	}
	if(!parent_grid) {
		parent_grid = find_widget<tgrid>(content_grid, id, true, false);
		assert(parent_grid);
	}
	if(tgrid* g = dynamic_cast<tgrid*>(parent_grid->parent())) {
		widget = g->swap_child(id, widget, false);
	} else if(tcontainer_* c
			  = dynamic_cast<tcontainer_*>(parent_grid->parent())) {

		widget = c->grid().swap_child(id, widget, true);
	} else {
		assert(false);
	}

	assert(widget);

	delete widget;
}

} // namespace

void twindow::finalize(const boost::intrusive_ptr<tbuilder_grid>& content_grid)
{
	swap_grid(NULL, &grid(), content_grid->build(), "_window_content_grid");
}

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS

void twindow::generate_dot_file(const std::string& generator,
								const unsigned domain)
{
	debug_layout_->generate_dot_file(generator, domain);
}
#endif

void twindow_implementation::layout(twindow& window,
									const unsigned maximum_width,
									const unsigned maximum_height)
{
	log_scope2(log_gui_layout, LOG_IMPL_SCOPE_HEADER);

	/*
	 * For now we return the status, need to test later whether this can
	 * entirely be converted to an exception based system as in 'promised' on
	 * the algorithm page.
	 */

	try
	{
		tpoint size = window.get_best_size();

		DBG_GUI_L << LOG_IMPL_HEADER << " best size : " << size
				  << " maximum size : " << maximum_width << ','
				  << maximum_height << ".\n";
		if(size.x <= static_cast<int>(maximum_width)
		   && size.y <= static_cast<int>(maximum_height)) {

			DBG_GUI_L << LOG_IMPL_HEADER << " Result: Fits, nothing to do.\n";
			return;
		}

		if(size.x > static_cast<int>(maximum_width)) {
			window.reduce_width(maximum_width);

			size = window.get_best_size();
			if(size.x > static_cast<int>(maximum_width)) {
				DBG_GUI_L << LOG_IMPL_HEADER << " Result: Resize width failed."
						  << " Wanted width " << maximum_width
						  << " resulting width " << size.x << ".\n";
				throw tlayout_exception_width_resize_failed();
			}
			DBG_GUI_L << LOG_IMPL_HEADER
					  << " Status: Resize width succeeded.\n";
		}

		if(size.y > static_cast<int>(maximum_height)) {
			window.reduce_height(maximum_height);

			size = window.get_best_size();
			if(size.y > static_cast<int>(maximum_height)) {
				DBG_GUI_L << LOG_IMPL_HEADER << " Result: Resize height failed."
						  << " Wanted height " << maximum_height
						  << " resulting height " << size.y << ".\n";
				throw tlayout_exception_height_resize_failed();
			}
			DBG_GUI_L << LOG_IMPL_HEADER
					  << " Status: Resize height succeeded.\n";
		}

		assert(size.x <= static_cast<int>(maximum_width)
			   && size.y <= static_cast<int>(maximum_height));


		DBG_GUI_L << LOG_IMPL_HEADER << " Result: Resizing succeeded.\n";
		return;
	}
	catch(tlayout_exception_width_modified&)
	{
		DBG_GUI_L << LOG_IMPL_HEADER
				  << " Status: Width has been modified, rerun.\n";

		window.layout_initialise(false);
		window.layout_linked_widgets();
		layout(window, maximum_width, maximum_height);
		return;
	}
}

void twindow::mouse_capture(const bool capture)
{
	assert(event_distributor_);
	event_distributor_->capture_mouse(capture);
}

void twindow::keyboard_capture(twidget* widget)
{
	assert(event_distributor_);
	event_distributor_->keyboard_capture(widget);
}

void twindow::add_to_keyboard_chain(twidget* widget)
{
	assert(event_distributor_);
	event_distributor_->keyboard_add_to_chain(widget);
}

void twindow::remove_from_keyboard_chain(twidget* widget)
{
	assert(event_distributor_);
	event_distributor_->keyboard_remove_from_chain(widget);
}

void twindow::signal_handler_sdl_video_resize(const event::tevent event,
											  bool& handled,
											  const tpoint& new_size)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(new_size.x < preferences::min_allowed_width()
	   || new_size.y < preferences::min_allowed_height()) {

		DBG_GUI_E << LOG_HEADER << " resize aborted, too small.\n";
		return;
	}

	if(new_size.x == static_cast<int>(settings::screen_width)
	   && new_size.y == static_cast<int>(settings::screen_height)) {

		DBG_GUI_E << LOG_HEADER << " resize not needed.\n";
		handled = true;
		return;
	}

	if(!preferences::set_resolution(video_, new_size.x, new_size.y)) {

		LOG_GUI_E << LOG_HEADER << " resize aborted, resize failed.\n";
		return;
	}

	settings::gamemap_width += new_size.x - settings::screen_width;
	settings::gamemap_height += new_size.y - settings::screen_height;
	settings::screen_width = new_size.x;
	settings::screen_height = new_size.y;
	invalidate_layout();

	handled = true;
}

void twindow::signal_handler_click_dismiss(const event::tevent event,
										   bool& handled,
										   bool& halt,
										   const Uint8 mouse_button_mask)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << " mouse_button_mask "
			  << static_cast<unsigned>(mouse_button_mask) << ".\n";

	handled = halt = click_dismiss(mouse_button_mask);
}

void twindow::signal_handler_sdl_key_down(const event::tevent event,
										  bool& handled,
										  SDLKey key)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(!enter_disabled_ && (key == SDLK_KP_ENTER || key == SDLK_RETURN)) {
		set_retval(OK);
		handled = true;
	} else if(key == SDLK_ESCAPE && !escape_disabled_) {
		set_retval(CANCEL);
		handled = true;
	} else if(key == SDLK_SPACE) {
		handled = click_dismiss(0);
	}
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	if(key == SDLK_F12) {
		debug_layout_->generate_dot_file("manual", tdebug_layout_graph::MANUAL);
		handled = true;
	}
#endif
}

void twindow::signal_handler_message_show_tooltip(const event::tevent event,
												  bool& handled,
												  event::tmessage& message)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	event::tmessage_show_tooltip& request
			= dynamic_cast<event::tmessage_show_tooltip&>(message);

	tip::show(video_, tooltip_.id, request.message, request.location);

	handled = true;
}

void twindow::signal_handler_message_show_helptip(const event::tevent event,
												  bool& handled,
												  event::tmessage& message)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	event::tmessage_show_helptip& request
			= dynamic_cast<event::tmessage_show_helptip&>(message);

	tip::show(video_, helptip_.id, request.message, request.location);

	handled = true;
}

void twindow::signal_handler_request_placement(const event::tevent event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	invalidate_layout();

	handled = true;
}

} // namespace gui2


/**
 * @page layout_algorithm Layout algorithm
 *
 * @section introduction Introduction
 *
 * This page describes how the layout engine for the dialogs works. First
 * a global overview of some terms used in this document.
 *
 * - @ref gui2::twidget "Widget"; Any item which can be used in the widget
 *   toolkit. Not all widgets are visible. In general widgets cannot be
 *   sized directly, but this is controlled by a window. A widget has an
 *   internal size cache and if the value in the cache is not equal to 0,0
 *   that value is its best size. This value gets set when the widget can
 *   honor a resize request.  It will be set with the value which honors
 *   the request.
 *
 * - @ref gui2::tgrid "Grid"; A grid is an invisible container which holds
 *   one or more widgets.  Several widgets have a grid in them to hold
 *   multiple widgets eg panels and windows.
 *
 * - @ref gui2::tgrid::tchild "Grid cell"; Every widget which is in a grid is
 *   put in a grid cell. These cells also hold the information about the gaps
 *   between widgets the behavior on growing etc. All grid cells must have a
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
 *   of it doesn't have the state tvisible::invisible. Widgets which have the
 *   state @ref tvisible::hidden are sized properly since when they become
 *   @ref tvisible::visible the layout shouldn't be invalidated. A grid cell
 *   that's invisible has size 0,0.
 *
 * - All resizable grid cells; A grid cell is resizable under the following
 *   conditions:
 *   - The widget is tvisible::visible.
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
 * @section layout_algorithm_window Window
 *
 * Here is the algorithm used to layout the window:
 *
 * - Perform a full initialization
 *   (@ref gui2::twidget::layout_initialise (full_initialisation = true)):
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
 *       (@ref gui2::tgrid::reduce_width):
 *       - Sort the widgets in the row on the resize priority.
 *         - Loop through this priority queue until the row fits
 *           - If priority != 0 try to share the extra width else all
 *             widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by either wrapping or using a
 *             scrollbar (@ref gui2::twidget::request_reduce_width).
 *           - If the row fits in the wanted width this row is done.
 *           - Else try the next priority.
 *         - All priorities done and the width still doesn't fit.
 *         - Loop through this priority queue until the row fits.
 *           - If priority != 0:
 *             - try to share the extra width
 *           -Else:
 *             - All widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by sizing them smaller as really
 *             wanted (@ref gui2::twidget::demand_reduce_width).
 *             For labels, buttons etc. they get ellipsized.
 *           - If the row fits in the wanted width this row is done.
 *           - Else try the next priority.
 *         - All priorities done and the width still doesn't fit.
 *         - Throw a layout width doesn't fit exception.
 *   - If height > maximum_height, optimize the height
 *       (@ref gui2::tgrid::reduce_height):
 *     - For every grid cell in a grid column there will be a resize request:
 *       - Sort the widgets in the column on the resize priority.
 *         - Loop through this priority queue until the column fits:
 *           - If priority != 0 try to share the extra height else all
 *              widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by using a scrollbar
 *             (@ref gui2::twidget::request_reduce_height).
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
 *             wanted (@ref gui2::twidget::demand_reduce_width).
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
 *     (@ref gui2::twidget::layout_initialise (full_initialisation = false))
 *   - Handle shared sizes, since the reinitialization resets that state.
 *   - Goto start layout loop.
 *
 * @section grid Grid
 *
 * This section will be documented later.
 */
