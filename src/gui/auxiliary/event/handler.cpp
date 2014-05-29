/*
   Copyright (C) 2009 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/auxiliary/event/handler.hpp"

#include "clipboard.hpp"
#include "gui/auxiliary/event/dispatcher.hpp"
#include "gui/auxiliary/timer.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/helper.hpp"
#include "gui/widgets/widget.hpp"
#include "gui/widgets/window.hpp"
#include "hotkey/hotkey_item.hpp"
#include "utils/foreach.tpp"
#include "video.hpp"

#include <cassert>

/**
 * @todo The items below are not implemented yet.
 *
 * - Tooltips have a fixed short time until showing up.
 * - Tooltips are shown until the widget is exited.
 * - Help messages aren't shown yet.
 *
 * @note it might be that tooltips will be shown independent of a window and in
 * their own window, therefore the code will be cleaned up after that has been
 * determined.
 */

/*
 * At some point in the future this event handler should become the main event
 * handler. This switch controls the experimental switch for that change.
 */
//#define MAIN_EVENT_HANDLER

/* Since this code is still very experimental it's not enabled yet. */
//#define ENABLE

namespace gui2
{

namespace event
{

/***** Static data. *****/
class thandler;
static thandler* handler = NULL;
static events::event_context* event_context = NULL;

#ifdef MAIN_EVENT_HANDLER
static unsigned draw_interval = 0;
static unsigned event_poll_interval = 0;

/***** Static functions. *****/

/**
 * SDL_AddTimer() callback for the draw event.
 *
 * When this callback is called it pushes a new draw event in the event queue.
 *
 * @returns                       The new timer interval, 0 to stop.
 */
static Uint32 timer_sdl_draw_event(Uint32, void*)
{
	// DBG_GUI_E << "Pushing draw event in queue.\n";

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
 * SDL_AddTimer() callback for the poll event.
 *
 * When this callback is called it will run the events in the SDL event queue.
 *
 * @returns                       The new timer interval, 0 to stop.
 */
static Uint32 timer_sdl_poll_events(Uint32, void*)
{
	try
	{
		events::pump();
	}
	catch(CVideo::quit&)
	{
		return 0;
	}
	return event_poll_interval;
}
#endif

/***** thandler class. *****/

/**
 * This singleton class handles all events.
 *
 * It's a new experimental class.
 */
class thandler : public events::handler
{
	friend bool gui2::is_in_dialog();

public:
	thandler();

	~thandler();

	/** Inherited from events::handler. */
	void handle_event(const SDL_Event& event);

	/**
	 * Connects a dispatcher.
	 *
	 * @param dispatcher              The dispatcher to connect.
	 */
	void connect(tdispatcher* dispatcher);

	/**
	 * Disconnects a dispatcher.
	 *
	 * @param dispatcher              The dispatcher to disconnect.
	 */
	void disconnect(tdispatcher* dispatcher);

	/** The dispatcher that captured the mouse focus. */
	tdispatcher* mouse_focus;

private:
	/**
	 * Reinitializes the state of all dispatchers.
	 *
	 * This is needed when the application gets activated, to make sure the
	 * state of mainly the mouse is set properly.
	 */
	void activate();

	/***** Handlers *****/

	/** Fires a draw event. */
	using events::handler::draw;
	void draw(const bool force);

	/**
	 * Fires a video resize event.
	 *
	 * @param new_size               The new size of the window.
	 */
	void video_resize(const tpoint& new_size);

	/**
	 * Fires a generic mouse event.
	 *
	 * @param event                  The event to fire.
	 * @param position               The position of the mouse.
	 */
	void mouse(const tevent event, const tpoint& position);

	/**
	 * Fires a mouse button up event.
	 *
	 * @param position               The position of the mouse.
	 * @param button                 The SDL id of the button that caused the
	 *                               event.
	 */
	void mouse_button_up(const tpoint& position, const Uint8 button);

	/**
	 * Fires a mouse button down event.
	 *
	 * @param position               The position of the mouse.
	 * @param button                 The SDL id of the button that caused the
	 *                               event.
	 */
	void mouse_button_down(const tpoint& position, const Uint8 button);

	/**
	 * Fires a mouse wheel event.
	 *
	 * @param position               The position of the mouse.
	 * @param scrollx                The amount of horizontal scrolling.
	 * @param scrolly                The amount of vertical scrolling.
	 */
	void mouse_wheel(const tpoint& position, int scrollx, int scrolly);

	/**
	 * Gets the dispatcher that wants to receive the keyboard input.
	 *
	 * @returns                   The dispatcher.
	 * @retval NULL               No dispatcher found.
	 */
	tdispatcher* keyboard_dispatcher();

	/**
	 * Handles a hat motion event.
	 *
	 * @param event                  The SDL joystick hat event triggered.
	 */
	void hat_motion(const SDL_JoyHatEvent& event);


	/**
	 * Handles a joystick button down event.
	 *
	 * @param event                  The SDL joystick button event triggered.
	 */
	void button_down(const SDL_JoyButtonEvent& event);


	/**
	 * Fires a key down event.
	 *
	 * @param event                  The SDL keyboard event triggered.
	 */
	void key_down(const SDL_KeyboardEvent& event);

	/**
	 * Handles the pressing of a hotkey.
	 *
	 * @param key                 The hotkey item pressed.
	 *
	 * @returns                   True if the hotkey is handled false otherwise.
	 */
	bool hotkey_pressed(const hotkey::hotkey_item& key);

	/**
	 * Fires a key down event.
	 *
	 * @param key                    The SDL key code of the key pressed.
	 * @param modifier               The SDL key modifiers used.
	 * @param unicode                The unicode value for the key pressed.
	 */
	void
	key_down(const SDLKey key, const SDLMod modifier, const Uint16 unicode);

	/**
	 * Fires a keyboard event which has no parameters.
	 *
	 * This can happen for example when the mouse wheel is used.
	 *
	 * @param event                  The event to fire.
	 */
	void keyboard(const tevent event);

	/**
	 * The dispatchers.
	 *
	 * The order of the items in the list is also the z-order the front item
	 * being the one completely in the background and the back item the one
	 * completely in the foreground.
	 */
	std::vector<tdispatcher*> dispatchers_;

	/**
	 * Needed to determine which dispatcher gets the keyboard events.
	 *
	 * NOTE the keyboard events aren't really wired in yet so doesn't do much.
	 */
	tdispatcher* keyboard_focus_;
	friend void capture_keyboard(tdispatcher*);
};

thandler::thandler()
	: events::handler(false)
	, mouse_focus(NULL)
	, dispatchers_()
	, keyboard_focus_(NULL)
{
	if(SDL_WasInit(SDL_INIT_TIMER) == 0) {
		if(SDL_InitSubSystem(SDL_INIT_TIMER) == -1) {
			assert(false);
		}
	}

// The event context is created now we join it.
#ifdef ENABLE
	join();
#endif
}

thandler::~thandler()
{
#ifdef ENABLE
	leave();
#endif
}

void thandler::handle_event(const SDL_Event& event)
{
	/** No dispatchers drop the event. */
	if(dispatchers_.empty()) {
		return;
	}

	switch(event.type) {
		case SDL_MOUSEMOTION:
			mouse(SDL_MOUSE_MOTION, tpoint(event.motion.x, event.motion.y));
			break;

		case SDL_MOUSEBUTTONDOWN:
			mouse_button_down(tpoint(event.button.x, event.button.y),
							  event.button.button);
			break;

		case SDL_MOUSEBUTTONUP:
			mouse_button_up(tpoint(event.button.x, event.button.y),
							event.button.button);
			break;

#if SDL_VERSION_ATLEAST(2, 0, 0)
		case SDL_MOUSEWHEEL:
			mouse_wheel(get_mouse_position(), event.wheel.x, event.wheel.y);
			break;
#endif

		case SHOW_HELPTIP_EVENT:
			mouse(SHOW_HELPTIP, get_mouse_position());
			break;

		case HOVER_REMOVE_POPUP_EVENT:
			// remove_popup();
			break;

		case DRAW_EVENT:
			draw(false);
			break;

		case TIMER_EVENT:
			execute_timer(reinterpret_cast<size_t>(event.user.data1));
			break;

		case CLOSE_WINDOW_EVENT: {
			/** @todo Convert this to a proper new style event. */
			DBG_GUI_E << "Firing " << CLOSE_WINDOW << ".\n";

			twindow* window = twindow::window_instance(event.user.code);
			if(window) {
				window->set_retval(twindow::AUTO_CLOSE);
			}
		} break;

		case SDL_JOYBUTTONDOWN:
			button_down(event.jbutton);
			break;

		case SDL_JOYBUTTONUP:
			break;

		case SDL_JOYAXISMOTION:
			break;

		case SDL_JOYHATMOTION:
			hat_motion(event.jhat);
			break;

		case SDL_KEYDOWN:
			key_down(event.key);
			break;

#if SDL_VERSION_ATLEAST(2, 0, 0)
		case SDL_WINDOWEVENT:
			switch(event.window.event) {
				case SDL_WINDOWEVENT_EXPOSED:
					draw(true);
					break;

				case SDL_WINDOWEVENT_RESIZED:
					video_resize(
							tpoint(event.window.data1, event.window.data2));
					break;

				case SDL_WINDOWEVENT_ENTER:
				case SDL_WINDOWEVENT_FOCUS_GAINED:
					activate();
					break;
			}

			break;
#else
		case SDL_VIDEOEXPOSE:
			draw(true);
			break;

		case SDL_VIDEORESIZE:
			video_resize(tpoint(event.resize.w, event.resize.h));
			break;

		case SDL_ACTIVEEVENT:
			activate();
			break;
#endif

#if(defined(_X11) && !defined(__APPLE__)) || defined(_WIN32)
		case SDL_SYSWMEVENT:
			/* DO NOTHING */
			break;
#endif

		// Silently ignored events.
		case SDL_KEYUP:
		case DOUBLE_CLICK_EVENT:
			break;

		default:
			WRN_GUI_E << "Unhandled event " << static_cast<Uint32>(event.type)
					  << ".\n";
			break;
	}
}

void thandler::connect(tdispatcher* dispatcher)
{
	assert(std::find(dispatchers_.begin(), dispatchers_.end(), dispatcher)
		   == dispatchers_.end());

	if(dispatchers_.empty()) {
		event_context = new events::event_context();
		join();
	}

	dispatchers_.push_back(dispatcher);
}

void thandler::disconnect(tdispatcher* dispatcher)
{
	/***** Validate pre conditions. *****/
	std::vector<tdispatcher*>::iterator itor
			= std::find(dispatchers_.begin(), dispatchers_.end(), dispatcher);
	assert(itor != dispatchers_.end());

	/***** Remove dispatcher. *****/
	dispatchers_.erase(itor);

	if(dispatcher == mouse_focus) {
		mouse_focus = NULL;
	}
	if(dispatcher == keyboard_focus_) {
		keyboard_focus_ = NULL;
	}

	/***** Set proper state for the other dispatchers. *****/
	FOREACH(AUTO dispatcher, dispatchers_)
	{
		dynamic_cast<twidget&>(*dispatcher).set_is_dirty(true);
	}

	activate();

	/***** Validate post conditions. *****/
	assert(std::find(dispatchers_.begin(), dispatchers_.end(), dispatcher)
		   == dispatchers_.end());

	if(dispatchers_.empty()) {
		leave();
		delete event_context;
		event_context = NULL;
	}
}

void thandler::activate()
{
	FOREACH(AUTO dispatcher, dispatchers_)
	{
		dispatcher->fire(
				SDL_ACTIVATE, dynamic_cast<twidget&>(*dispatcher), NULL);
	}
}

void thandler::draw(const bool force)
{
	// Don't display this event since it floods the screen
	// DBG_GUI_E << "Firing " << DRAW << ".\n";

	/*
	 * In normal draw mode the first window in not forced to be drawn the
	 * others are. So for forced mode we only need to force the first window to
	 * be drawn the others are already handled.
	 */
	bool first = !force;

	/**
	 * @todo Need to evaluate which windows really to redraw.
	 *
	 * For now we use a hack, but would be nice to rewrite it for 1.9/1.11.
	 */
	FOREACH(AUTO dispatcher, dispatchers_)
	{
		if(!first) {
			/*
			 * This leaves glitches on window borders if the window beneath it
			 * has changed, on the other hand invalidating twindown::restorer_
			 * causes black borders around the window. So there's the choice
			 * between two evils.
			 */
			dynamic_cast<twidget&>(*dispatcher).set_is_dirty(true);
		} else {
			first = false;
		}

		dispatcher->fire(DRAW, dynamic_cast<twidget&>(*dispatcher));
	}

	if(!dispatchers_.empty()) {
		CVideo& video = dynamic_cast<twindow&>(*dispatchers_.back()).video();

		surface frame_buffer = video.getSurface();

		cursor::draw(frame_buffer);
		video.flip();
		cursor::undraw(frame_buffer);
	}
}

void thandler::video_resize(const tpoint& new_size)
{
	DBG_GUI_E << "Firing: " << SDL_VIDEO_RESIZE << ".\n";

	FOREACH(AUTO dispatcher, dispatchers_)
	{
		dispatcher->fire(SDL_VIDEO_RESIZE,
						 dynamic_cast<twidget&>(*dispatcher),
						 new_size);
	}
}

void thandler::mouse(const tevent event, const tpoint& position)
{
	DBG_GUI_E << "Firing: " << event << ".\n";

	if(mouse_focus) {
		mouse_focus->fire(
				event, dynamic_cast<twidget&>(*mouse_focus), position);
	} else {

		for(std::vector<tdispatcher*>::reverse_iterator ritor
			= dispatchers_.rbegin();
			ritor != dispatchers_.rend();
			++ritor) {

			if((**ritor).get_mouse_behavior() == tdispatcher::all) {
				(**ritor)
						.fire(event, dynamic_cast<twidget&>(**ritor), position);
				break;
			}
			if((**ritor).get_mouse_behavior() == tdispatcher::none) {
				continue;
			}
			if((**ritor).is_at(position)) {
				(**ritor)
						.fire(event, dynamic_cast<twidget&>(**ritor), position);
				break;
			}
		}
	}
}

void thandler::mouse_button_up(const tpoint& position, const Uint8 button)
{
	switch(button) {
		case SDL_BUTTON_LEFT:
			mouse(SDL_LEFT_BUTTON_UP, position);
			break;
		case SDL_BUTTON_MIDDLE:
			mouse(SDL_MIDDLE_BUTTON_UP, position);
			break;
		case SDL_BUTTON_RIGHT:
			mouse(SDL_RIGHT_BUTTON_UP, position);
			break;

#if !SDL_VERSION_ATLEAST(2, 0, 0)
		case SDL_BUTTON_WHEELLEFT:
			mouse(SDL_WHEEL_LEFT, get_mouse_position());
			break;
		case SDL_BUTTON_WHEELRIGHT:
			mouse(SDL_WHEEL_RIGHT, get_mouse_position());
			break;
		case SDL_BUTTON_WHEELUP:
			mouse(SDL_WHEEL_UP, get_mouse_position());
			break;
		case SDL_BUTTON_WHEELDOWN:
			mouse(SDL_WHEEL_DOWN, get_mouse_position());
			break;
#endif

		default:
			WRN_GUI_E << "Unhandled 'mouse button up' event for button "
					  << static_cast<Uint32>(button) << ".\n";
			break;
	}
}

void thandler::mouse_button_down(const tpoint& position, const Uint8 button)
{
	// The wheel buttons generate and up and down event we handle the
	// up event so ignore the mouse if it's a down event. Handle it
	// here to avoid a warning.
	if(button == SDL_BUTTON_WHEELUP || button == SDL_BUTTON_WHEELDOWN
	   || button == SDL_BUTTON_WHEELLEFT || button == SDL_BUTTON_WHEELRIGHT) {

		return;
	}

	switch(button) {
		case SDL_BUTTON_LEFT:
			mouse(SDL_LEFT_BUTTON_DOWN, position);
			break;
		case SDL_BUTTON_MIDDLE:
			mouse(SDL_MIDDLE_BUTTON_DOWN, position);
			break;
		case SDL_BUTTON_RIGHT:
			mouse(SDL_RIGHT_BUTTON_DOWN, position);
			break;
		default:
			WRN_GUI_E << "Unhandled 'mouse button down' event for button "
					  << static_cast<Uint32>(button) << ".\n";
			break;
	}
}

#if SDL_VERSION_ATLEAST(2, 0, 0)
void thandler::mouse_wheel(const tpoint& position, int x, int y)
{
	if(x > 0) {
		mouse(SDL_WHEEL_RIGHT, position);
	} else if(x < 0) {
		mouse(SDL_WHEEL_LEFT, position);
	}

	if(y < 0) {
		mouse(SDL_WHEEL_DOWN, position);
	} else if(y > 0) {
		mouse(SDL_WHEEL_UP, position);
	}
}
#endif

tdispatcher* thandler::keyboard_dispatcher()
{
	if(keyboard_focus_) {
		return keyboard_focus_;
	}

	for(std::vector<tdispatcher*>::reverse_iterator ritor
		= dispatchers_.rbegin();
		ritor != dispatchers_.rend();
		++ritor) {

		if((**ritor).get_want_keyboard_input()) {
			return *ritor;
		}
	}

	return NULL;
}

void thandler::hat_motion(const SDL_JoyHatEvent& event)
{
	const hotkey::hotkey_item& hk = hotkey::get_hotkey(event);
	bool done = false;
	if(!hk.null()) {
		done = hotkey_pressed(hk);
	}
	if(!done) {
		// TODO fendrin think about handling hat motions that are not bound to a
		// hotkey.
	}
}

void thandler::button_down(const SDL_JoyButtonEvent& event)
{
	const hotkey::hotkey_item& hk = hotkey::get_hotkey(event);
	bool done = false;
	if(!hk.null()) {
		done = hotkey_pressed(hk);
	}
	if(!done) {
		// TODO fendrin think about handling button down events that are not
		// bound to a hotkey.
	}
}

void thandler::key_down(const SDL_KeyboardEvent& event)
{
	const hotkey::hotkey_item& hk = hotkey::get_hotkey(event);
	bool done = false;
	if(!hk.null()) {
		done = hotkey_pressed(hk);
	}
	if(!done) {
#if SDL_VERSION_ATLEAST(2, 0, 0)
		key_down(event.keysym.sym,
				 static_cast<const SDL_Keymod>(event.keysym.mod),
				 static_cast<const Uint16>(event.keysym.scancode));
#else
		key_down(event.keysym.sym, event.keysym.mod, event.keysym.unicode);
#endif
	}
}

bool thandler::hotkey_pressed(const hotkey::hotkey_item& key)
{
	tdispatcher* dispatcher = keyboard_dispatcher();

	if(!dispatcher) {
		return false;
	}

	return dispatcher->execute_hotkey(hotkey::get_id(key.get_command()));
}

void thandler::key_down(const SDLKey key,
						const SDLMod modifier,
						const Uint16 unicode)
{
	DBG_GUI_E << "Firing: " << SDL_KEY_DOWN << ".\n";

	if(tdispatcher* dispatcher = keyboard_dispatcher()) {
		dispatcher->fire(SDL_KEY_DOWN,
						 dynamic_cast<twidget&>(*dispatcher),
						 key,
						 modifier,
						 unicode);
	}
}

void thandler::keyboard(const tevent event)
{
	DBG_GUI_E << "Firing: " << event << ".\n";

	if(tdispatcher* dispatcher = keyboard_dispatcher()) {
		dispatcher->fire(event, dynamic_cast<twidget&>(*dispatcher));
	}
}

/***** tmanager class. *****/

tmanager::tmanager()
{
	handler = new thandler();

#ifdef MAIN_EVENT_HANDLER
	draw_interval = 30;
	SDL_AddTimer(draw_interval, timer_sdl_draw_event, NULL);

	event_poll_interval = 10;
	SDL_AddTimer(event_poll_interval, timer_sdl_poll_events, NULL);
#endif
}

tmanager::~tmanager()
{
	delete handler;
	handler = NULL;

#ifdef MAIN_EVENT_HANDLER
	draw_interval = 0;
	event_poll_interval = 0;
#endif
}

/***** free functions class. *****/

void connect_dispatcher(tdispatcher* dispatcher)
{
	assert(handler);
	assert(dispatcher);
	handler->connect(dispatcher);
}

void disconnect_dispatcher(tdispatcher* dispatcher)
{
	assert(handler);
	assert(dispatcher);
	handler->disconnect(dispatcher);
}

void init_mouse_location()
{
	tpoint mouse = get_mouse_position();

	SDL_Event event;
	event.type = SDL_MOUSEMOTION;
	event.motion.type = SDL_MOUSEMOTION;
	event.motion.x = mouse.x;
	event.motion.y = mouse.y;

	SDL_PushEvent(&event);
}

void capture_mouse(tdispatcher* dispatcher)
{
	assert(handler);
	assert(dispatcher);
	handler->mouse_focus = dispatcher;
}

void release_mouse(tdispatcher* dispatcher)
{
	assert(handler);
	assert(dispatcher);
	if(handler->mouse_focus == dispatcher) {
		handler->mouse_focus = NULL;
	}
}

void capture_keyboard(tdispatcher* dispatcher)
{
	assert(handler);
	assert(dispatcher);
	assert(dispatcher->get_want_keyboard_input());

	handler->keyboard_focus_ = dispatcher;
}

std::ostream& operator<<(std::ostream& stream, const tevent event)
{
	switch(event) {
		case DRAW:
			stream << "draw";
			break;
		case CLOSE_WINDOW:
			stream << "close window";
			break;
		case SDL_VIDEO_RESIZE:
			stream << "SDL video resize";
			break;
		case SDL_MOUSE_MOTION:
			stream << "SDL mouse motion";
			break;
		case MOUSE_ENTER:
			stream << "mouse enter";
			break;
		case MOUSE_LEAVE:
			stream << "mouse leave";
			break;
		case MOUSE_MOTION:
			stream << "mouse motion";
			break;
		case SDL_LEFT_BUTTON_DOWN:
			stream << "SDL left button down";
			break;
		case SDL_LEFT_BUTTON_UP:
			stream << "SDL left button up";
			break;
		case LEFT_BUTTON_DOWN:
			stream << "left button down";
			break;
		case LEFT_BUTTON_UP:
			stream << "left button up";
			break;
		case LEFT_BUTTON_CLICK:
			stream << "left button click";
			break;
		case LEFT_BUTTON_DOUBLE_CLICK:
			stream << "left button double click";
			break;
		case SDL_MIDDLE_BUTTON_DOWN:
			stream << "SDL middle button down";
			break;
		case SDL_MIDDLE_BUTTON_UP:
			stream << "SDL middle button up";
			break;
		case MIDDLE_BUTTON_DOWN:
			stream << "middle button down";
			break;
		case MIDDLE_BUTTON_UP:
			stream << "middle button up";
			break;
		case MIDDLE_BUTTON_CLICK:
			stream << "middle button click";
			break;
		case MIDDLE_BUTTON_DOUBLE_CLICK:
			stream << "middle button double click";
			break;
		case SDL_RIGHT_BUTTON_DOWN:
			stream << "SDL right button down";
			break;
		case SDL_RIGHT_BUTTON_UP:
			stream << "SDL right button up";
			break;
		case RIGHT_BUTTON_DOWN:
			stream << "right button down";
			break;
		case RIGHT_BUTTON_UP:
			stream << "right button up";
			break;
		case RIGHT_BUTTON_CLICK:
			stream << "right button click";
			break;
		case RIGHT_BUTTON_DOUBLE_CLICK:
			stream << "right button double click";
			break;
		case SDL_WHEEL_LEFT:
			stream << "SDL wheel left";
			break;
		case SDL_WHEEL_RIGHT:
			stream << "SDL wheel right";
			break;
		case SDL_WHEEL_UP:
			stream << "SDL wheel up";
			break;
		case SDL_WHEEL_DOWN:
			stream << "SDL wheel down";
			break;
		case SDL_KEY_DOWN:
			stream << "SDL key down";
			break;

		case NOTIFY_REMOVAL:
			stream << "notify removal";
			break;
		case NOTIFY_MODIFIED:
			stream << "notify modified";
			break;
		case RECEIVE_KEYBOARD_FOCUS:
			stream << "receive keyboard focus";
			break;
		case LOSE_KEYBOARD_FOCUS:
			stream << "lose keyboard focus";
			break;
		case SHOW_TOOLTIP:
			stream << "show tooltip";
			break;
		case NOTIFY_REMOVE_TOOLTIP:
			stream << "notify remove tooltip";
			break;
		case SDL_ACTIVATE:
			stream << "SDL activate";
			break;
		case MESSAGE_SHOW_TOOLTIP:
			stream << "message show tooltip";
			break;
		case SHOW_HELPTIP:
			stream << "show helptip";
			break;
		case MESSAGE_SHOW_HELPTIP:
			stream << "message show helptip";
			break;
		case REQUEST_PLACEMENT:
			stream << "request placement";
			break;
	}

	return stream;
}

} // namespace event

bool is_in_dialog()
{
	return event::handler && !event::handler->dispatchers_.empty();
}

} // namespace gui2
