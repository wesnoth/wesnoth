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

#ifndef GUI_DIALOGS_DIALOG_HPP_INCLUDED
#define GUI_DIALOGS_DIALOG_HPP_INCLUDED

#include "gui/dialogs/field-fwd.hpp"

#include <string>
#include <vector>

class CVideo;

namespace gui2 {

/**
 * Registers a window.
 *
 * Call this function to register a window. In the header of the class add the
 * following code:
 *@code
 *  // Inherited from tdialog, implemented by REGISTER_WINDOW.
 *	virtual const std::string& id() const;
 *@endcode
 * Then use this macro in the implementation, inside the gui2 namespace.
 *
 * @note When the window_id is "foo" and the type tfoo it's easier to use
 * REGISTER_WINDOW(foo).
 *
 * @param type                    Class type of the window to register.
 * @param widget_id               Id of the window, multiple dialogs can use
 *                                the same window so the id doesn't need to be
 *                                unique.
 */
#define REGISTER_WINDOW2(                                                  \
		  type                                                             \
		, id)                                                              \
namespace {                                                                \
                                                                           \
	namespace ns_##type {                                                  \
                                                                           \
		struct tregister_helper {                                          \
			tregister_helper()                                             \
			{                                                              \
				register_window(id);                                       \
			}                                                              \
		};                                                                 \
                                                                           \
		tregister_helper register_helper;                                  \
	}                                                                      \
}                                                                          \
                                                                           \
const std::string&                                                         \
type::window_id() const                                                    \
{                                                                          \
	static const std::string result(id);                                   \
	return result;                                                         \
}

/**
 * Wrapper for REGISTER_WINDOW2.
 *
 * "Calls" REGISTER_WINDOW2(twindow_id, "window_id")
 */
#define REGISTER_WINDOW(window_id) REGISTER_WINDOW2(t##window_id, #window_id)

/**
 * Abstract base class for all dialogs.
 *
 * A dialog shows a certain window instance to the user. The subclasses of this
 * class will hold the parameters used for a certain window, eg a server
 * connection dialog will hold the name of the selected server as parameter that
 * way the caller doesn't need to know about the 'contents' of the window.
 */
class tdialog
{
public:
	tdialog() :
		retval_(0),
		fields_(),
		restore_(true)
	{}

	virtual ~tdialog();

	/**
	 * Shows the window.
	 *
	 * @param video               The video which contains the surface to draw
	 *                            upon.
	 * @param auto_close_time     The time in ms after which the dialog will
	 *                            automatically close, if 0 it doesn't close.
	 *                            @note the timeout is a minimum time and
	 *                            there's no quarantee about how fast it closes
	 *                            after the minimum.
	 */
	void show(CVideo& video, const unsigned auto_close_time = 0);

	/***** ***** ***** setters / getters for members ***** ****** *****/

	int get_retval() const { return retval_; }

	void set_restore(const bool restore) { restore_ = restore; }

protected:

	/**
	 * Creates a new boolean field.
	 *
	 * The field created is owned by tdialog, the returned pointer can be used
	 * in the child classes as access to a field.
	 *
	 * @param id                  Id of the widget, same value as in WML.
	 * @param optional            Is the widget mandatory or optional.
	 * @param callback_load_value The callback function to set the initial value
	 *                            of the widget.
	 * @param callback_save_value The callback function to write the resulting
	 *                            value of the widget. Saving will only happen
	 *                            if the widget is enabled and the window closed
	 *                            with ok.
	 * @param callback_change     When the value of the widget changes this
	 *                            callback is called.
	 *
	 * @returns                   Pointer to the created widget.
	 */
	tfield_bool* register_bool(const std::string& id,
		const bool optional = false,
		bool (*callback_load_value) () = NULL,
		void (*callback_save_value) (const bool value) = NULL,
		void (*callback_change) (twidget* widget) = NULL);

	/**
	 * Creates a new integer field.
	 *
	 * See register_bool for more info.
	 */
	tfield_integer* register_integer(const std::string& id,
		const bool optional = false,
		int (*callback_load_value) () = NULL,
		void (*callback_save_value) (const int value) = NULL);

	/**
	 * Creates a new text field.
	 *
	 * See register_bool for more info.
	 */
	tfield_text* register_text(const std::string& id,
		const bool optional = false,
		std::string (*callback_load_value) () = NULL,
		void (*callback_save_value) (const std::string& value) = NULL);
private:
	/** Returns the window exit status, 0 means not shown. */
	int retval_;

	/**
	 * Contains the automatically managed fields.
	 *
	 * Since the fields are automatically managed and there are no search
	 * functions defined we don't offer access to the vector. If access is
	 * needed the creator should store a copy of the pointer.
	 */
	std::vector<tfield_*> fields_;

	/**
	 * Restore the screen after showing?
	 *
	 * Most windows should restore the display after showing so this value
	 * defaults to true. Toplevel windows (like the titlescreen don't want this
	 * behaviour so they can change it in pre_show().
	 */
	bool restore_;

	/** The id of the window to build. */
	virtual const std::string& window_id() const = 0;

	/**
	 * Builds the window.
	 *
	 * Every dialog shows it's own kind of window, this function should return
	 * the window to show.
	 *
	 * @param video               The video which contains the surface to draw
	 *                            upon.
	 * @returns                   The window to show.
	 */
	twindow* build_window(CVideo& video) const;

	/**
	 * Actions to be taken directly after the window is build.
	 *
	 * @param video               The video which contains the surface to draw
	 *                            upon.
	 * @param window              The window just created.
	 */
	virtual void post_build(CVideo& /*video*/, twindow& /*window*/) {}

	/**
	 * Actions to be taken before showing the window.
	 *
	 * @param video               The video which contains the surface to draw
	 *                            upon.
	 * @param window              The window to be shown.
	 */
	virtual void pre_show(CVideo& /*video*/, twindow& /*window*/) {}

	/**
	 * Actions to be taken after the window has been shown.
	 *
	 * @param window              The window which has been shown.
	 */
	virtual void post_show(twindow& /*window*/) {}

	/**
	 * Initializes all fields in the dialog.
	 *
	 * @param window              The window which has been shown.
	 */
	virtual void init_fields(twindow& window);

	/**
	 * When the dialog is closed with the OK status saves all fields.
	 *
	 * Saving only happens if a callback handler is installed.
	 *
	 * @param window              The window which has been shown.
	 */
	virtual void finalize_fields(twindow& window);
};

} // namespace gui2

#endif

