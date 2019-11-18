/*
   Copyright (C) 2007 - 2018 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "config.hpp"
#include "cursor.hpp"
#include "events.hpp"
#include "floating_label.hpp"
#include "formula/callable.hpp"
#include "gettext.hpp"
#include "log.hpp"
#include "gui/auxiliary/typed_formula.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/event/distributor.hpp"
#include "gui/core/event/handler.hpp"
#include "gui/core/event/message.hpp"
#include "gui/core/log.hpp"
#include "gui/core/layout_exception.hpp"
#include "sdl/point.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/dialogs/title_screen.hpp"
#include "gui/dialogs/tooltip.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/container_base.hpp"
#include "gui/widgets/text_box_base.hpp"
#include "gui/core/register_widget.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/panel.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
#include "gui/widgets/debug.hpp"
#endif
#include "preferences/general.hpp"
#include "preferences/display.hpp"
#include "sdl/rect.hpp"
#include "sdl/surface.hpp"
#include "formula/variant.hpp"
#include "video.hpp"
#include "wml_exception.hpp"
#include "sdl/userevent.hpp"

#include "utils/functional.hpp"

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace wfl { class function_symbol_table; }
namespace gui2 { class button; }

static lg::log_domain log_gui("gui/layout");
#define ERR_GUI  LOG_STREAM(err, log_gui)

#define LOG_SCOPE_HEADER get_control_type() + " [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

#define LOG_IMPL_SCOPE_HEADER                                                  \
	window.get_control_type() + " [" + window.id() + "] " + __func__
#define LOG_IMPL_HEADER LOG_IMPL_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
/** @todo See whether this hack can be removed. */
// Needed to fix a compiler error in REGISTER_WIDGET.
class builder_window : public builder_styled_widget
{
public:
	builder_window(const config& cfg) : builder_styled_widget(cfg)
	{
	}

	using builder_styled_widget::build;

	widget* build() const
	{
		return nullptr;
	}
};

} // namespace implementation
REGISTER_WIDGET(window)

namespace
{
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
const unsigned SHOW = debug_layout_graph::SHOW;
const unsigned LAYOUT = debug_layout_graph::LAYOUT;
#else
// values are irrelavant when DEBUG_WINDOW_LAYOUT_GRAPHS is not defined.
const unsigned SHOW = 0;
const unsigned LAYOUT = 0;
#endif

/**
 * Pushes a single draw event to the queue. To be used before calling
 * events::pump when drawing windows.
 *
 * @todo: in the future we should simply call draw functions directly
 * from events::pump and do away with the custom drawing events, but
 * that's a 1.15 target. For now, this will have to do.
 */
static void push_draw_event()
{
	//	DBG_GUI_E << "Pushing draw event in queue.\n";

	SDL_Event event;
	sdl::UserEvent data(DRAW_EVENT);

	event.type = DRAW_EVENT;
	event.user = data;

	SDL_PushEvent(&event);
}

/**
 * SDL_AddTimer() callback for delay_event.
 *
 * @param event                   The event to push in the event queue.
 *
 * @return                        The new timer interval (always 0).
 */
static uint32_t delay_event_callback(const uint32_t, void* event)
{
	SDL_PushEvent(static_cast<SDL_Event*>(event));
	delete static_cast<SDL_Event*>(event);
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
static void delay_event(const SDL_Event& event, const uint32_t delay)
{
	SDL_AddTimer(delay, delay_event_callback, new SDL_Event(event));
}

/**
 * Adds a SHOW_HELPTIP event to the SDL event queue.
 *
 * The event is used to show the helptip for the currently focused widget.
 */
static void helptip()
{
	DBG_GUI_E << "Pushing SHOW_HELPTIP_EVENT event in queue.\n";

	SDL_Event event;
	sdl::UserEvent data(SHOW_HELPTIP_EVENT);

	event.type = SHOW_HELPTIP_EVENT;
	event.user = data;

	SDL_PushEvent(&event);
}

/**
 * Small helper class to get an unique id for every window instance.
 *
 * This is used to send event to the proper window, this allows windows to post
 * messages to themselves and let them delay for a certain amount of time.
 */
class manager
{
	manager();

public:
	static manager& instance();

	void add(window& window);

	void remove(window& window);

	unsigned get_id(window& window);

	window* get_window(const unsigned id);

private:
	// The number of active window should be rather small
	// so keep it simple and don't add a reverse lookup map.
	std::map<unsigned, window*> windows_;
};

manager::manager() : windows_()
{
}

manager& manager::instance()
{
	static manager window_manager;
	return window_manager;
}

void manager::add(window& win)
{
	static unsigned id;
	++id;
	windows_[id] = &win;
}

void manager::remove(window& win)
{
	for(std::map<unsigned, window*>::iterator itor = windows_.begin();
		itor != windows_.end();
		++itor) {

		if(itor->second == &win) {
			windows_.erase(itor);
			return;
		}
	}
	assert(false);
}

unsigned manager::get_id(window& win)
{
	for(std::map<unsigned, window*>::iterator itor = windows_.begin();
		itor != windows_.end();
		++itor) {

		if(itor->second == &win) {
			return itor->first;
		}
	}
	assert(false);

	return 0;
}

window* manager::get_window(const unsigned id)
{
	std::map<unsigned, window*>::iterator itor = windows_.find(id);

	if(itor == windows_.end()) {
		return nullptr;
	} else {
		return itor->second;
	}
}

} // namespace

window::window(const builder_window::window_resolution* definition)
	: panel(implementation::builder_window(::config {"definition", definition->definition}), type())
	, video_(CVideo::get_singleton())
	, status_(NEW)
	, show_mode_(none)
	, retval_(retval::NONE)
	, owner_(nullptr)
	, need_layout_(true)
	, variables_()
	, invalidate_layout_blocked_(false)
	, suspend_drawing_(true)
	, restore_(true)
	, is_toplevel_(!is_in_dialog())
	, restorer_()
	, automatic_placement_(definition->automatic_placement)
	, horizontal_placement_(definition->horizontal_placement)
	, vertical_placement_(definition->vertical_placement)
	, maximum_width_(definition->maximum_width)
	, maximum_height_(definition->maximum_height)
	, x_(definition->x)
	, y_(definition->y)
	, w_(definition->width)
	, h_(definition->height)
	, reevaluate_best_size_(definition->reevaluate_best_size)
	, functions_(definition->functions)
	, tooltip_(definition->tooltip)
	, helptip_(definition->helptip)
	, click_dismiss_(false)
	, enter_disabled_(false)
	, escape_disabled_(false)
	, linked_size_()
	, mouse_button_state_(0) /**< Needs to be initialized in @ref show. */
	, dirty_list_()
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	, debug_layout_(new debug_layout_graph(this))
#endif
	, event_distributor_(new event::distributor(*this, event::dispatcher::front_child))
	, exit_hook_([](window&)->bool { return true; })
	, callback_next_draw_(nullptr)
{
	manager::instance().add(*this);

	connect();

	if (!video_.faked())
	{
		connect_signal<event::DRAW>(std::bind(&window::draw, this));
	}

	connect_signal<event::SDL_VIDEO_RESIZE>(std::bind(
			&window::signal_handler_sdl_video_resize, this, _2, _3, _5));

	connect_signal<event::SDL_ACTIVATE>(std::bind(
			&event::distributor::initialize_state, event_distributor_.get()));

	connect_signal<event::SDL_LEFT_BUTTON_UP>(
			std::bind(&window::signal_handler_click_dismiss,
						this,
						_2,
						_3,
						_4,
						SDL_BUTTON_LMASK),
			event::dispatcher::front_child);
	connect_signal<event::SDL_MIDDLE_BUTTON_UP>(
			std::bind(&window::signal_handler_click_dismiss,
						this,
						_2,
						_3,
						_4,
						SDL_BUTTON_MMASK),
			event::dispatcher::front_child);
	connect_signal<event::SDL_RIGHT_BUTTON_UP>(
			std::bind(&window::signal_handler_click_dismiss,
						this,
						_2,
						_3,
						_4,
						SDL_BUTTON_RMASK),
			event::dispatcher::front_child);

	connect_signal<event::SDL_KEY_DOWN>(
			std::bind(
					&window::signal_handler_sdl_key_down, this, _2, _3, _5, _6, true),
			event::dispatcher::back_post_child);
	connect_signal<event::SDL_KEY_DOWN>(std::bind(
			&window::signal_handler_sdl_key_down, this, _2, _3, _5, _6, false));

	connect_signal<event::MESSAGE_SHOW_TOOLTIP>(
			std::bind(&window::signal_handler_message_show_tooltip,
						this,
						_2,
						_3,
						_5),
			event::dispatcher::back_pre_child);

	connect_signal<event::MESSAGE_SHOW_HELPTIP>(
			std::bind(&window::signal_handler_message_show_helptip,
						this,
						_2,
						_3,
						_5),
			event::dispatcher::back_pre_child);

	connect_signal<event::REQUEST_PLACEMENT>(
			std::bind(
					&window::signal_handler_request_placement, this, _2, _3),
			event::dispatcher::back_pre_child);

	connect_signal<event::CLOSE_WINDOW>(std::bind(&window::signal_handler_close_window, this));

	register_hotkey(hotkey::GLOBAL__HELPTIP, std::bind(gui2::helptip));

	/** @todo: should eventally become part of global hotkey handling. */
	register_hotkey(hotkey::HOTKEY_FULLSCREEN,
		std::bind(&CVideo::toggle_fullscreen, std::ref(video_)));
}

window::~window()
{
	/*
	 * We need to delete our children here instead of waiting for the grid to
	 * automatically do it. The reason is when the grid deletes its children
	 * they will try to unregister them self from the linked widget list. At
	 * this point the member of window are destroyed and we enter UB. (For
	 * some reason the bug didn't trigger on g++ but it does on MSVC.
	 */
	for(unsigned row = 0; row < get_grid().get_rows(); ++row) {
		for(unsigned col = 0; col < get_grid().get_cols(); ++col) {
			get_grid().remove_child(row, col);
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
		dialogs::tip::remove();
	}

	manager::instance().remove(*this);

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS

	delete debug_layout_;

#endif
}

window* window::window_instance(const unsigned handle)
{
	return manager::instance().get_window(handle);
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
retval window::get_retval_by_id(const std::string& id)
{
	// Note it might change to a map later depending on the number
	// of items.
	if(id == "ok") {
		return retval::OK;
	} else if(id == "cancel" || id == "quit") {
		return retval::CANCEL;
	} else {
		return retval::NONE;
	}
}

void window::show_tooltip(/*const unsigned auto_close_timeout*/)
{
	log_scope2(log_gui_draw, "Window: show as tooltip.");

	generate_dot_file("show", SHOW);

	assert(status_ == NEW);

	set_mouse_behavior(event::dispatcher::none);
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

void window::show_non_modal(/*const unsigned auto_close_timeout*/)
{
	log_scope2(log_gui_draw, "Window: show non modal.");

	generate_dot_file("show", SHOW);

	assert(status_ == NEW);

	set_mouse_behavior(event::dispatcher::hit);

	show_mode_ = modeless;

	/*
	 * Before show has been called, some functions might have done some testing
	 * on the window and called layout, which can give glitches. So
	 * reinvalidate the window to avoid those glitches.
	 */
	invalidate_layout();
	suspend_drawing_ = false;

	push_draw_event();

	events::pump();
}

int window::show(const bool restore, const unsigned auto_close_timeout)
{
	/*
	 * Removes the old tip if one shown. The show_tip doesn't remove
	 * the tip, since it's the tip.
	 */
	dialogs::tip::remove();

	show_mode_ = modal;
	restore_ = restore;

	log_scope2(log_gui_draw, LOG_SCOPE_HEADER);

	generate_dot_file("show", SHOW);

	assert(status_ == NEW);

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
		sdl::UserEvent data(CLOSE_WINDOW_EVENT, manager::instance().get_id(*this));

		event.type = CLOSE_WINDOW_EVENT;
		event.user = data;

		delay_event(event, auto_close_timeout);
	}


	try
	{
		// Start our loop drawing will happen here as well.
		bool mouse_button_state_initialized = false;
		for(status_ = SHOWING; status_ != CLOSED;) {
			push_draw_event();

			// process installed callback if valid, to allow e.g. network
			// polling
			events::pump();

			if(!mouse_button_state_initialized) {
				/*
				 * The state must be initialize when showing the dialog.
				 * However when initialized before this point there were random
				 * errors. This only happened when the 'click' was done fast; a
				 * slower click worked properly.
				 *
				 * So it seems the events need to be processed before SDL can
				 * return the proper button state. When initializing here all
				 * works fine.
				 */
				mouse_button_state_ = SDL_GetMouseState(nullptr, nullptr);
				mouse_button_state_initialized = true;
			}

			if(status_ == REQUEST_CLOSE) {
				status_ = exit_hook_(*this) ? CLOSED : SHOWING;
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
		if(restore_) {
			SDL_Rect rect = get_rectangle();
			sdl_blit(restorer_, 0, video_.getSurface(), &rect);
			font::undraw_floating_labels(video_.getSurface());
		}
		throw;
	}

	suspend_drawing_ = true;

	// restore area
	if(restore_) {
		SDL_Rect rect = get_rectangle();
		sdl_blit(restorer_, 0, video_.getSurface(), &rect);
		font::undraw_floating_labels(video_.getSurface());
	}

	if(text_box_base* tb = dynamic_cast<text_box_base*>(event_distributor_->keyboard_focus())) {
		tb->interrupt_composition();
	}

	return retval_;
}

void window::draw()
{
	/***** ***** ***** ***** Init ***** ***** ***** *****/
	// Prohibited from drawing?
	if(suspend_drawing_) {
		return;
	}

	surface& frame_buffer = video_.getSurface();

	/***** ***** Layout and get dirty list ***** *****/
	if(need_layout_) {
		// Restore old surface. In the future this phase will not be needed
		// since all will be redrawn when needed with dirty rects. Since that
		// doesn't work yet we need to undraw the window.
		if(restore_ && restorer_) {
			SDL_Rect rect = get_rectangle();
			sdl_blit(restorer_, 0, frame_buffer, &rect);
		}

		layout();

		// Get new surface for restoring
		SDL_Rect rect = get_rectangle();

		// We want the labels underneath the window so draw them and use them
		// as restore point.
		if(is_toplevel_) {
			font::draw_floating_labels(frame_buffer);
		}

		if(restore_) {
			restorer_ = get_surface_portion(frame_buffer, rect);
		}

		// Need full redraw so only set ourselves dirty.
		dirty_list_.emplace_back(1, this);
	} else {

		// Let widgets update themselves, which might dirty some things.
		layout_children();

		// Now find the widgets that are dirty.
		std::vector<widget*> call_stack;
		if(!new_widgets) {
			populate_dirty_list(*this, call_stack);
		} else {
			/* Force to update and redraw the entire screen */
			dirty_list_.clear();
			dirty_list_.emplace_back(1, this);
		}
	}

	if (dirty_list_.empty()) {
		consecutive_changed_frames_ = 0u;
		return;
	}

	++consecutive_changed_frames_;
	if(consecutive_changed_frames_ >= 100u && id_ == "title_screen") {
		/* The title screen has changed in 100 consecutive frames, i.e. every
		frame for two seconds. It looks like the screen is constantly changing
		or at least marking widgets as dirty.

		That's a severe problem. Every time the title screen changes, all
		other GUI windows need to be fully redrawn, with huge CPU usage cost.
		For that reason, this situation is a hard error. */
		throw std::logic_error("The title screen is constantly changing, "
			"which has a huge CPU usage cost. See the code comment.");
	}

	for(auto & item : dirty_list_)
	{

		assert(!item.empty());

		const SDL_Rect dirty_rect
				= new_widgets ? video().screen_area()
							  : item.back()->get_dirty_rectangle();

// For testing we disable the clipping rect and force the entire screen to
// update. This way an item rendered at the wrong place is directly visible.
#if 0
		dirty_list_.clear();
		dirty_list_.emplace_back(1, this);
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
		 * redrawing either being not visibility::visible or has status
		 * widget::redraw_action::none. If it's not drawn it's still set not
		 * dirty to avoid it keep getting on the dirty list.
		 */

		for(std::vector<widget*>::iterator itor = item.begin();
			itor != item.end();
			++itor) {

			if((**itor).get_visible() != widget::visibility::visible
			   || (**itor).get_drawing_action()
				  == widget::redraw_action::none) {

				for(std::vector<widget*>::iterator citor = itor;
					citor != item.end();
					++citor) {

					(**citor).set_is_dirty(false);
				}

				item.erase(itor, item.end());
				break;
			}
		}

		// Restore.
		if(restore_) {
			SDL_Rect rect = get_rectangle();
			sdl_blit(restorer_, 0, frame_buffer, &rect);
		}

		// Background.
		for(std::vector<widget*>::iterator itor = item.begin();
			itor != item.end();
			++itor) {

			(**itor).draw_background(frame_buffer, 0, 0);
		}

		// Children.
		if(!item.empty()) {
			item.back()->draw_children(frame_buffer, 0, 0);
		}

		// Foreground.
		for(std::vector<widget*>::reverse_iterator ritor = item.rbegin();
			ritor != item.rend();
			++ritor) {

			(**ritor).draw_foreground(frame_buffer, 0, 0);
			(**ritor).set_is_dirty(false);
		}
	}

	dirty_list_.clear();

	redraw_windows_on_top();

	std::vector<widget*> call_stack;
	populate_dirty_list(*this, call_stack);
	assert(dirty_list_.empty());

	if(callback_next_draw_ != nullptr) {
		callback_next_draw_();
		callback_next_draw_ = nullptr;
	}
}

void window::undraw()
{
	if(restore_ && restorer_) {
		SDL_Rect rect = get_rectangle();
		sdl_blit(restorer_, 0, video_.getSurface(), &rect);
		// Since the old area might be bigger as the new one, invalidate
		// it.
	}
}

window::invalidate_layout_blocker::invalidate_layout_blocker(window& window)
	: window_(window)
{
	assert(!window_.invalidate_layout_blocked_);
	window_.invalidate_layout_blocked_ = true;
}

window::invalidate_layout_blocker::~invalidate_layout_blocker()
{
	assert(window_.invalidate_layout_blocked_);
	window_.invalidate_layout_blocked_ = false;
}

void window::invalidate_layout()
{
	if(!invalidate_layout_blocked_) {
		need_layout_ = true;
	}
}
widget* window::find_at(const point& coordinate, const bool must_be_active)
{
	return panel::find_at(coordinate, must_be_active);
}

const widget* window::find_at(const point& coordinate,
								const bool must_be_active) const
{
	return panel::find_at(coordinate, must_be_active);
}

widget* window::find(const std::string& id, const bool must_be_active)
{
	return container_base::find(id, must_be_active);
}

const widget* window::find(const std::string& id, const bool must_be_active)
		const
{
	return container_base::find(id, must_be_active);
}

void window::init_linked_size_group(const std::string& id,
									 const bool fixed_width,
									 const bool fixed_height)
{
	assert(fixed_width || fixed_height);
	assert(!has_linked_size_group(id));

	linked_size_[id] = linked_size(fixed_width, fixed_height);
}

bool window::has_linked_size_group(const std::string& id)
{
	return linked_size_.find(id) != linked_size_.end();
}

void window::add_linked_widget(const std::string& id, widget* wgt)
{
	assert(wgt);
	if(!has_linked_size_group(id)) {
		ERR_GUI << "Unknown linked group '" << id << "'; skipping\n";
		return;
	}

	std::vector<widget*>& widgets = linked_size_[id].widgets;
	if(std::find(widgets.begin(), widgets.end(), wgt) == widgets.end()) {
		widgets.push_back(wgt);
	}
}

void window::remove_linked_widget(const std::string& id, const widget* wgt)
{
	assert(wgt);
	if(!has_linked_size_group(id)) {
		return;
	}

	std::vector<widget*>& widgets = linked_size_[id].widgets;

	std::vector<widget*>::iterator itor
			= std::find(widgets.begin(), widgets.end(), wgt);

	if(itor != widgets.end()) {
		widgets.erase(itor);

		assert(std::find(widgets.begin(), widgets.end(), wgt)
			   == widgets.end());
	}
}

void window::layout()
{
	/***** Initialize. *****/

	const auto conf = cast_config_to<window_definition>();
	assert(conf);

	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	point size = get_best_size();
	const point mouse = get_mouse_position();

	variables_.add("mouse_x", wfl::variant(mouse.x));
	variables_.add("mouse_y", wfl::variant(mouse.y));
	variables_.add("window_width", wfl::variant(0));
	variables_.add("window_height", wfl::variant(0));
	variables_.add("best_window_width", wfl::variant(size.x));
	variables_.add("best_window_height", wfl::variant(size.y));
	variables_.add("size_request_mode", wfl::variant("maximum"));

	get_screen_size_variables(variables_);

	int maximum_width = 0;
	int maximum_height = 0;

	if(automatic_placement_) {
		if(maximum_width_ > 0) {
			maximum_width = std::min(maximum_width_, settings::screen_width);
		} else {
			maximum_width = settings::screen_width;
		}

		if(maximum_height_ > 0) {
			maximum_height = std::min(maximum_height_, settings::screen_height);
		} else {
			maximum_height = settings::screen_height;
		}
	} else {
		maximum_width  = w_(variables_, &functions_);
		maximum_height = h_(variables_, &functions_);
	}

	/***** Handle click dismiss status. *****/
	button* click_dismiss_button = nullptr;
	if((click_dismiss_button
		= find_widget<button>(this, "click_dismiss", false, false))) {

		click_dismiss_button->set_visible(widget::visibility::invisible);
	}
	if(click_dismiss_) {
		button* btn = find_widget<button>(this, "ok", false, false);
		if(btn) {
			btn->set_visible(widget::visibility::invisible);
			click_dismiss_button = btn;
		}
		VALIDATE(click_dismiss_button,
				 _("Click dismiss needs a 'click_dismiss' or 'ok' button."));
	}

	/***** Layout. *****/
	layout_initialize(true);
	generate_dot_file("layout_initialize", LAYOUT);

	layout_linked_widgets();

	try
	{
		window_implementation::layout(*this, maximum_width, maximum_height);
	}
	catch(const layout_exception_resize_failed&)
	{

		/** @todo implement the scrollbars on the window. */

		std::stringstream sstr;
		sstr << __FILE__ << ":" << __LINE__ << " in function '" << __func__
			 << "' found the following problem: Failed to size window;"
			 << " wanted size " << get_best_size() << " available size "
			 << maximum_width << ',' << maximum_height << " screen size "
			 << settings::screen_width << ',' << settings::screen_height << '.';

		throw wml_exception(_("Failed to show a dialog, "
							   "which doesn't fit on the screen."),
							 sstr.str());
	}

	/****** Validate click dismiss status. *****/
	if(click_dismiss_ && disable_click_dismiss()) {
		assert(click_dismiss_button);
		click_dismiss_button->set_visible(widget::visibility::visible);

		connect_signal_mouse_left_click(
				*click_dismiss_button,
				std::bind(&window::set_retval, this, retval::OK, true));

		layout_initialize(true);
		generate_dot_file("layout_initialize", LAYOUT);

		layout_linked_widgets();

		try
		{
			window_implementation::layout(
					*this, maximum_width, maximum_height);
		}
		catch(const layout_exception_resize_failed&)
		{

			/** @todo implement the scrollbars on the window. */

			std::stringstream sstr;
			sstr << __FILE__ << ":" << __LINE__ << " in function '" << __func__
				 << "' found the following problem: Failed to size window;"
				 << " wanted size " << get_best_size() << " available size "
				 << maximum_width << ',' << maximum_height << " screen size "
				 << settings::screen_width << ',' << settings::screen_height
				 << '.';

			throw wml_exception(_("Failed to show a dialog, "
								   "which doesn't fit on the screen."),
								 sstr.str());
		}
	}

	/***** Get the best location for the window *****/
	size = get_best_size();
	assert(size.x <= maximum_width && size.y <= maximum_height);

	point origin(0, 0);

	if(automatic_placement_) {

		switch(horizontal_placement_) {
			case grid::HORIZONTAL_ALIGN_LEFT:
				// Do nothing
				break;
			case grid::HORIZONTAL_ALIGN_CENTER:
				origin.x = (settings::screen_width - size.x) / 2;
				break;
			case grid::HORIZONTAL_ALIGN_RIGHT:
				origin.x = settings::screen_width - size.x;
				break;
			default:
				assert(false);
		}
		switch(vertical_placement_) {
			case grid::VERTICAL_ALIGN_TOP:
				// Do nothing
				break;
			case grid::VERTICAL_ALIGN_CENTER:
				origin.y = (settings::screen_height - size.y) / 2;
				break;
			case grid::VERTICAL_ALIGN_BOTTOM:
				origin.y = settings::screen_height - size.y;
				break;
			default:
				assert(false);
		}
	} else {

		variables_.add("window_width", wfl::variant(size.x));
		variables_.add("window_height", wfl::variant(size.y));

		while(reevaluate_best_size_(variables_, &functions_)) {
			layout_initialize(true);

			window_implementation::layout(*this,
										   w_(variables_, &functions_),
										   h_(variables_, &functions_));

			size = get_best_size();
			variables_.add("window_width", wfl::variant(size.x));
			variables_.add("window_height", wfl::variant(size.y));
		}

		variables_.add("size_request_mode", wfl::variant("size"));

		size.x = w_(variables_, &functions_);
		size.y = h_(variables_, &functions_);

		variables_.add("window_width", wfl::variant(size.x));
		variables_.add("window_height", wfl::variant(size.y));

		origin.x = x_(variables_, &functions_);
		origin.y = y_(variables_, &functions_);
	}

	/***** Set the window size *****/
	place(origin, size);

	generate_dot_file("layout_finished", LAYOUT);
	need_layout_ = false;

	event::init_mouse_location();
}

void window::layout_linked_widgets()
{
	// evaluate the group sizes
	for(auto & linked_size : linked_size_)
	{

		point max_size(0, 0);

		// Determine the maximum size.
		for(auto widget : linked_size.second.widgets)
		{

			const point size = widget->get_best_size();

			if(size.x > max_size.x) {
				max_size.x = size.x;
			}
			if(size.y > max_size.y) {
				max_size.y = size.y;
			}
		}
		if(linked_size.second.width != -1) {
			linked_size.second.width = max_size.x;
		}
		if(linked_size.second.height != -1) {
			linked_size.second.height = max_size.y;
		}

		// Set the maximum size.
		for(auto widget : linked_size.second.widgets)
		{

			point size = widget->get_best_size();

			if(linked_size.second.width != -1) {
				size.x = max_size.x;
			}
			if(linked_size.second.height != -1) {
				size.y = max_size.y;
			}

			widget->set_layout_size(size);
		}
	}
}

bool window::click_dismiss(const int mouse_button_mask)
{
	if(does_click_dismiss()) {
		if((mouse_button_state_ & mouse_button_mask) == 0) {
			set_retval(retval::OK);
		} else {
			mouse_button_state_ &= ~mouse_button_mask;
		}
		return true;
	}
	return false;
}

namespace
{

/**
 * Swaps an item in a grid for another one.
 * This differs slightly from the standard swap_grid utility, so it's defined by itself here.
 */
void window_swap_grid(grid* g,
			   grid* content_grid,
			   widget* widget,
			   const std::string& id)
{
	assert(content_grid);
	assert(widget);

	// Make sure the new child has same id.
	widget->set_id(id);

	// Get the container containing the wanted widget.
	grid* parent_grid = nullptr;
	if(g) {
		parent_grid = find_widget<grid>(g, id, false, false);
	}
	if(!parent_grid) {
		parent_grid = find_widget<grid>(content_grid, id, true, false);
		assert(parent_grid);
	}
	if(grid* grandparent_grid = dynamic_cast<grid*>(parent_grid->parent())) {
		grandparent_grid->swap_child(id, widget, false);
	} else if(container_base* c
			  = dynamic_cast<container_base*>(parent_grid->parent())) {

		c->get_grid().swap_child(id, widget, true);
	} else {
		assert(false);
	}
}

} // namespace

void window::redraw_windows_on_top() const
{
	std::vector<dispatcher*>& dispatchers = event::get_all_dispatchers();
	auto me = std::find(dispatchers.begin(), dispatchers.end(), this);

	for(auto it = std::next(me); it != dispatchers.end(); ++it) {
		// Note that setting an entire window dirty like this is expensive.
		dynamic_cast<widget&>(**it).set_is_dirty(true);
	}
}

void window::finalize(const std::shared_ptr<builder_grid>& content_grid)
{
	window_swap_grid(nullptr, &get_grid(), content_grid->build(), "_window_content_grid");
}

#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS

void window::generate_dot_file(const std::string& generator,
								const unsigned domain)
{
	debug_layout_->generate_dot_file(generator, domain);
}
#endif

void window_implementation::layout(window& window,
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
		point size = window.get_best_size();

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
				throw layout_exception_width_resize_failed();
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
				throw layout_exception_height_resize_failed();
			}
			DBG_GUI_L << LOG_IMPL_HEADER
					  << " Status: Resize height succeeded.\n";
		}

		assert(size.x <= static_cast<int>(maximum_width)
			   && size.y <= static_cast<int>(maximum_height));


		DBG_GUI_L << LOG_IMPL_HEADER << " Result: Resizing succeeded.\n";
		return;
	}
	catch(const layout_exception_width_modified&)
	{
		DBG_GUI_L << LOG_IMPL_HEADER
				  << " Status: Width has been modified, rerun.\n";

		window.layout_initialize(false);
		window.layout_linked_widgets();
		layout(window, maximum_width, maximum_height);
		return;
	}
}

void window::mouse_capture(const bool capture)
{
	assert(event_distributor_);
	event_distributor_->capture_mouse(capture);
}

void window::keyboard_capture(widget* widget)
{
	assert(event_distributor_);
	event_distributor_->keyboard_capture(widget);
}

void window::add_to_keyboard_chain(widget* widget)
{
	assert(event_distributor_);
	event_distributor_->keyboard_add_to_chain(widget);
}

void window::remove_from_keyboard_chain(widget* widget)
{
	assert(event_distributor_);
	event_distributor_->keyboard_remove_from_chain(widget);
}

void window::add_to_tab_order(widget* widget, int at)
{
	if(std::find(tab_order.begin(), tab_order.end(), widget) != tab_order.end()) {
		return;
	}
	assert(event_distributor_);
	if(tab_order.empty() && !event_distributor_->keyboard_focus()) {
		keyboard_capture(widget);
	}
	if(at < 0 || at >= static_cast<int>(tab_order.size())) {
		tab_order.push_back(widget);
	} else {
		tab_order.insert(tab_order.begin() + at, widget);
	}
}

void window::signal_handler_sdl_video_resize(const event::ui_event event,
											  bool& handled,
											  const point& new_size)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	settings::gamemap_width += new_size.x - settings::screen_width;
	settings::gamemap_height += new_size.y - settings::screen_height;
	settings::screen_width = new_size.x;
	settings::screen_height = new_size.y;
	invalidate_layout();

	handled = true;
}

void window::signal_handler_click_dismiss(const event::ui_event event,
										   bool& handled,
										   bool& halt,
										   const int mouse_button_mask)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << " mouse_button_mask "
			  << static_cast<unsigned>(mouse_button_mask) << ".\n";

	handled = halt = click_dismiss(mouse_button_mask);
}

static bool is_active(const widget* wgt)
{
	if(const styled_widget* control = dynamic_cast<const styled_widget*>(wgt)) {
		return control->get_active() && control->get_visible() == widget::visibility::visible;
	}
	return false;
}

void window::signal_handler_sdl_key_down(const event::ui_event event,
										  bool& handled,
										  const SDL_Keycode key,
										  const SDL_Keymod mod,
										  bool handle_tab)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(text_box_base* tb = dynamic_cast<text_box_base*>(event_distributor_->keyboard_focus())) {
		if(tb->is_composing()) {
			if(handle_tab && !tab_order.empty() && key == SDLK_TAB) {
				tb->interrupt_composition();
			} else {
				return;
			}
		}
	}
	if(!enter_disabled_ && (key == SDLK_KP_ENTER || key == SDLK_RETURN)) {
		set_retval(retval::OK);
		handled = true;
	} else if(key == SDLK_ESCAPE && !escape_disabled_) {
		set_retval(retval::CANCEL);
		handled = true;
	} else if(key == SDLK_SPACE) {
		handled = click_dismiss(0);
	} else if(handle_tab && !tab_order.empty() && key == SDLK_TAB) {
		assert(event_distributor_);
		widget* focus = event_distributor_->keyboard_focus();
		auto iter = std::find(tab_order.begin(), tab_order.end(), focus);
		do {
			if(mod & KMOD_SHIFT) {
				if(iter == tab_order.begin()) {
					iter = tab_order.end();
				}
				iter--;
			} else {
				if(iter == tab_order.end()) {
					iter = tab_order.begin();
				} else {
					iter++;
					if(iter == tab_order.end()) {
						iter = tab_order.begin();
					}
				}
			}
		} while(!is_active(*iter));
		keyboard_capture(*iter);
		handled = true;
	}
#ifdef DEBUG_WINDOW_LAYOUT_GRAPHS
	if(key == SDLK_F12) {
		debug_layout_->generate_dot_file("manual", debug_layout_graph::MANUAL);
		handled = true;
	}
#endif
}

void window::signal_handler_message_show_tooltip(const event::ui_event event,
												  bool& handled,
												  const event::message& message)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	const event::message_show_tooltip& request
			= dynamic_cast<const event::message_show_tooltip&>(message);

	dialogs::tip::show(tooltip_.id, request.message, request.location, request.source_rect);

	handled = true;
}

void window::signal_handler_message_show_helptip(const event::ui_event event,
												  bool& handled,
												  const event::message& message)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	const event::message_show_helptip& request
			= dynamic_cast<const event::message_show_helptip&>(message);

	dialogs::tip::show(helptip_.id, request.message, request.location, request.source_rect);

	handled = true;
}

void window::signal_handler_request_placement(const event::ui_event event,
											   bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	invalidate_layout();

	handled = true;
}

void window::signal_handler_close_window()
{
	set_retval(retval::AUTO_CLOSE);
}

// }---------- DEFINITION ---------{

/*WIKI
 * @page = GUIWidgetDefinitionWML
 * @order = 1_window
 *
 * == Window ==
 *
 * The definition of a window. A window is a kind of panel see the panel for
 * which fields exist
 *
 * @begin{parent}{name="gui/"}
 * @begin{tag}{name="window_definition"}{min=0}{max=-1}{super="generic/widget_definition"}
 * @begin{tag}{name="resolution"}{min=0}{max=-1}{super="gui/panel_definition/resolution"}
 * @allow{link}{name="gui/window/resolution/grid"}
 * @allow{link}{name="gui/panel_definition/resolution/background"}
 * @allow{link}{name="gui/panel_definition/resolution/foreground"}
 * @end{tag}{name="resolution"}
 * @end{tag}{name="window_definition"}
 * @end{parent}{name="gui/"}
 */
window_definition::window_definition(const config& cfg)
	: styled_widget_definition(cfg)
{
	DBG_GUI_P << "Parsing window " << id << '\n';

	load_resolutions<resolution>(cfg);
}

window_definition::resolution::resolution(const config& cfg)
	: panel_definition::resolution(cfg), grid(nullptr)
{
	const config& child = cfg.child("grid");
	// VALIDATE(child, _("No grid defined."));

	/** @todo Evaluate whether the grid should become mandatory. */
	if(child) {
		grid = std::make_shared<builder_grid>(child);
	}
}

// }------------ END --------------

} // namespace gui2


/**
 * @page layout_algorithm Layout algorithm
 *
 * @section introduction Introduction
 *
 * This page describes how the layout engine for the dialogs works. First
 * a global overview of some terms used in this document.
 *
 * - @ref gui2::widget "Widget"; Any item which can be used in the widget
 *   toolkit. Not all widgets are visible. In general widgets cannot be
 *   sized directly, but this is controlled by a window. A widget has an
 *   internal size cache and if the value in the cache is not equal to 0,0
 *   that value is its best size. This value gets set when the widget can
 *   honor a resize request.  It will be set with the value which honors
 *   the request.
 *
 * - @ref gui2::grid "Grid"; A grid is an invisible container which holds
 *   one or more widgets.  Several widgets have a grid in them to hold
 *   multiple widgets eg panels and windows.
 *
 * - @ref gui2::grid::child "Grid cell"; Every widget which is in a grid is
 *   put in a grid cell. These cells also hold the information about the gaps
 *   between widgets the behavior on growing etc. All grid cells must have a
 *   widget inside them.
 *
 * - @ref gui2::window "Window"; A window is a top level item which has a
 *   grid with its children. The window handles the sizing of the window and
 *   makes sure everything fits.
 *
 * - @ref gui2::window::linked_size "Shared size group"; A shared size
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
 *   of it doesn't have the state visibility::invisible. Widgets which have the
 *   state @ref visibility::hidden are sized properly since when they become
 *   @ref visibility::visible the layout shouldn't be invalidated. A grid cell
 *   that's invisible has size 0,0.
 *
 * - All resizable grid cells; A grid cell is resizable under the following
 *   conditions:
 *   - The widget is visibility::visible.
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
 *   (@ref gui2::widget::layout_initialize (full_initialization = true)):
 *   - Clear the internal best size cache for all widgets.
 *   - For widgets with scrollbars hide them unless the
 *     @ref gui2::scrollbar_container::scrollbar_mode "scrollbar_mode" is
 *     ALWAYS_VISIBLE or AUTO_VISIBLE.
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
 *       (@ref gui2::grid::reduce_width):
 *       - Sort the widgets in the row on the resize priority.
 *         - Loop through this priority queue until the row fits
 *           - If priority != 0 try to share the extra width else all
 *             widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by either wrapping or using a
 *             scrollbar (@ref gui2::widget::request_reduce_width).
 *           - If the row fits in the wanted width this row is done.
 *           - Else try the next priority.
 *         - All priorities done and the width still doesn't fit.
 *         - Loop through this priority queue until the row fits.
 *           - If priority != 0:
 *             - try to share the extra width
 *           -Else:
 *             - All widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by sizing them smaller as really
 *             wanted (@ref gui2::widget::demand_reduce_width).
 *             For labels, buttons etc. they get ellipsized.
 *           - If the row fits in the wanted width this row is done.
 *           - Else try the next priority.
 *         - All priorities done and the width still doesn't fit.
 *         - Throw a layout width doesn't fit exception.
 *   - If height > maximum_height, optimize the height
 *       (@ref gui2::grid::reduce_height):
 *     - For every grid cell in a grid column there will be a resize request:
 *       - Sort the widgets in the column on the resize priority.
 *         - Loop through this priority queue until the column fits:
 *           - If priority != 0 try to share the extra height else all
 *              widgets are tried to reduce the full size.
 *           - Try to shrink the widgets by using a scrollbar
 *             (@ref gui2::widget::request_reduce_height).
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
 *             wanted (@ref gui2::widget::demand_reduce_width).
 *             For labels, buttons etc. they get ellipsized .
 *           - If the column fits in the wanted height this column is done.
 *           - Else try the next priority.
 *         - All priorities done and the height still doesn't fit.
 *         - Throw a layout height doesn't fit exception.
 * - End layout loop.
 *
 * - Catch @ref gui2::layout_exception_width_modified "width modified":
 *   - Goto relayout.
 *
 * - Catch
 *   @ref gui2::layout_exception_width_resize_failed "width resize failed":
 *   - If the window has a horizontal scrollbar which isn't shown but can be
 *     shown.
 *     - Show the scrollbar.
 *     - goto relayout.
 *   - Else show a layout failure message.
 *
 * - Catch
 *   @ref gui2::layout_exception_height_resize_failed "height resize failed":
 *   - If the window has a vertical scrollbar which isn't shown but can be
 *     shown:
 *     - Show the scrollbar.
 *     - goto relayout.
 *   - Else:
 *     - show a layout failure message.
 *
 * - Relayout:
 *   - Initialize all widgets
 *     (@ref gui2::widget::layout_initialize (full_initialization = false))
 *   - Handle shared sizes, since the reinitialization resets that state.
 *   - Goto start layout loop.
 *
 * @section grid Grid
 *
 * This section will be documented later.
 */
