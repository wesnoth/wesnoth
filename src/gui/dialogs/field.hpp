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

/**
 * @file
 * Implements some helper classes to ease adding fields to a dialog and hide
 * the synchronization needed. Since some templates are used all is stored in
 * the header.
 *
 */

#ifndef GUI_DIALOGS_FIELD_HPP_INCLUDED
#define GUI_DIALOGS_FIELD_HPP_INCLUDED

#include "gui/dialogs/field-fwd.hpp"
#include "gui/widgets/selectable.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

namespace gui2 {

/**
 * Abstract base class for the fields.
 */
class tfield_
{
public:

	tfield_(const std::string& id, const bool optional) :
		id_(id),
		optional_(optional)
	{
	}

	virtual ~tfield_() {}

	/**
	 * Initializes the widget.
	 *
	 * This routine is called before the dialog is shown and the pre_show() is
	 * called. So the user can override the values set. This routine does the
	 * following:
	 * - If no widget available exit gives feedback it the widget must exist.
	 * - If a getter is defined we use to set value_ and the widget.
	 * - If no setter is defined we use the widget value to set value_.
	 *
	 * The function calls two functions
	 *  - init_generic which is to be used in the template subclass.
	 *  - init_specialized which is to be used in subclasses of the template
	 *     class. This way they can override this function without to use their
	 *     signature to inherit.
	 *
	 * @param window              The window containing the widget.
	 */
	void widget_init(twindow& window)
	{
		init_generic(window);
		init_specialized(window);
	}

	/**
	 * Finalizes the widget.
	 *
	 * This routine is called after the dialog is closed with OK. It's called
	 * before post_show(). This routine does the following:
	 * - if no active widget available exit.
	 * - if a setter is defined the widget value is saved in the setter.
	 * - The widget value is saved in value_.
	 *
	 * Like widget_init it calls two functions with the same purpose.
	 *
	 * @param window              The window containing the widget.
	 */
	void widget_finalize(twindow& window)
	{
		finalize_generic(window);
		finalize_specialized(window);
	}

	/**
	 * Saves a widget.
	 *
	 * It can be a window must be recreated, in that case the state needs to be
	 * saved and restored. This routine does the following:
	 * - if no widget available exit (doesn't look at the active state).
	 * - The widget value is saved in value_.
	 *
	 * @param window              The window containing the widget.
	 */
	virtual void widget_save(twindow& window) = 0;

	/**
	 * Restores a widget.
	 *
	 * See widget_save for more info.
	 *
	 * @param window              The window containing the widget.
	 */
	virtual void widget_restore(twindow& window) = 0;

	/**
	 * Enables a widget.
	 *
	 * @param window              The window containing the widget.
	 * @param enable              If true enables the widget, disables otherwise.
	 * @param sync                If the state is changed do we need to
	 *                            synchronize. Upon disabling, write the value
	 *                            of the widget in the variable value_. Upon
	 *                            enabling write the value of value_ in the
	 *                            widget.
	 */
	void widget_set_enabled(twindow& window, const bool enable, const bool sync)
	{
		tcontrol* widget =
			dynamic_cast<tcontrol*>(window.find(id(), false));

		if(!widget) {
			return;
		}

		const bool widget_state =  widget->get_active();
		if(widget_state == enable) {
			return;
		}

		if(sync) {
			if(enable) {
				widget_restore(window);
			} else {
				widget_save(window);
			}
		}

		widget->set_active(enable);
	}

	/**
	 * Returns the widget associated with the field.
	 *
	 * @param window              The window containing the widget.
	 *
	 * @returns                   The widget NULL if not found and optional.
	 */
	twidget* widget(twindow& window) {

		twidget* widget = dynamic_cast<tcontrol*>(window.find(id(), false));
		VALIDATE(optional_ || widget, missing_widget(id()));

		return widget;
	}

	/***** ***** ***** setters / getters for members ***** ****** *****/

	const std::string& id() const { return id_; }

	bool is_optional() const { return optional_; }

private:
	/** The id field of the widget, should be unique in a window. */
	const std::string id_;

	/** Is the widget optional or mandatory in this window. */
	bool optional_;


	/** See widget_init. */
	virtual void init_generic(twindow& window) = 0;

	/** See widget_init. */
	virtual void init_specialized(twindow& /*window*/) {}


	/** See widget_finalize. */
	virtual void finalize_generic(twindow& window) = 0;

	/** See widget_finalize. */
	virtual void finalize_specialized(twindow& /*window*/) {}
};

/**
 * Template class to implement the generic field implementation.
 *
 * @param T                       The type of the item to show in the widget.
 * @param W                       The type of widget to show, this is not a
 *                                widget class but a behaviour class.
 * @param CT                      The type tp be used in the
 *                                callback_save_value callback. Normally this
 *                                is const T but for example with strings it
 *                                can be const T&. Note the const needs to be
 *                                in the template otherwise compilation on
 *                                GCC-4.3 fails (not sure whether compiler bug
 *                                or not).
 */
template<class T, class W, class CT> class tfield : public tfield_
{
public:

	tfield(const std::string& id,
			const bool optional,
			T (*callback_load_value) (),
			void (*callback_save_value) (CT value)) :
		tfield_(id, optional),
		value_(T()),
		callback_load_value_(callback_load_value),
		callback_save_value_(callback_save_value)
	{
	}

	/** Inherited from tfield_. */
	void widget_restore(twindow& window)
	{
		validate_widget(window);

		restore(window);
	}

	/**
	 * Sets the value of the field.
	 *
	 * This sets the value in both the internal cache value and in the widget
	 * itself.
	 *
	 * @param window              The window containing the widget.
	 * @param value               The new value.
	 */
	void set_widget_value(twindow& window, CT value)
	{
		value_ = value;
		restore(window);
	}

	/**
	 * Sets the value of the field.
	 *
	 * This sets the internal cache value but not the widget value, this can
	 * be used to initialize the field.
	 *
	 * @param value               The new value.
	 */
	void set_cache_value(CT value)
	{
		value_ = value;
	}

	/** Inherited from tfield_. */
	void widget_save(twindow& window)
	{
		save(window, false);
	}

	/**
	 * Gets the value of the field.
	 *
	 * This function gets the value of the widget and stores that in the
	 * internal cache, then that value is returned.
	 *
	 * @param window              The window containing the widget.
	 *
	 * @returns                   The current value of the widget.
	 */
	T get_widget_value(twindow& window)
	{
		save(window, false);
		return value_;
	}

	/**
	 * Gets the value of the field.
	 *
	 * This function returns the value internal cache, this function can be
	 * used after the widget no longer exists. The cache is normally updated
	 * when the window is closed with succes.
	 *
	 * @returns                   The currently value of the internal cache.
	 */
	T get_cache_value()
	{
		return value_;
	}

private:

	/**
	 * The value_ of the widget, this value is also available once the widget
	 * is destroyed.
	 */
	T value_;

	/**
	 * The callback function to load the value.
	 *
	 * This is used to load the initial value of the widget, if defined.
	 */
	T (*callback_load_value_) ();

	/** Inherited from tfield_. */
	void init_generic(twindow& window)
	{
		validate_widget(window);

		if(callback_load_value_) {
			value_ = callback_load_value_();
		}

		restore(window);
	}

	/** Inherited from tfield_. */
	void finalize_generic(twindow& window)
	{
		save(window, true);

		if(callback_save_value_) {
			callback_save_value_(value_);
		}
	}

	/**
	 * The callback function to save the value.
	 *
	 * Once the dialog has been successful this function is used to store the
	 * result of this widget.
	 */
	void (*callback_save_value_) ( CT value);

	/**
	 * Test whether the widget exists if the widget is mandatory.
	 *
	 * @param window              The window containing the widget.
	 */
	void validate_widget(twindow& window)
	{
		if(is_optional()) {
			return;
		}
		find_widget<const W>(&window, id(), false);
	}

	/**
	 * Stores the value in the widget in the interval value_.
	 *
	 * @param window              The window containing the widget.
	 * @param must_be_active      If true only active widgets will store their value.
	 */
	void save(twindow& window, const bool must_be_active)
	{
		const W* widget =
			dynamic_cast<const W*>(window.find(id(), must_be_active));

		if(widget) {
			value_ = widget->get_value();
		}
	}

	/**
	 * Stores the internal value_ in the widget.
	 */
	void restore(twindow& window)
	{
		W* widget = dynamic_cast<W*>(window.find(id(), false));

		if(widget) {
			widget->set_value(value_);
		}
	}
};

/** Specialized field class for boolean. */
class tfield_bool : public tfield<bool, tselectable_>
{
public:
	tfield_bool(const std::string& id,
			const bool optional,
			bool (*callback_load_value) (),
			void (*callback_save_value) (const bool value),
			void (*callback_change) (twidget* widget)) :
		tfield<bool, gui2::tselectable_>
			(id, optional, callback_load_value, callback_save_value),
		callback_change_(callback_change)
		{
		}

private:

	/** Overridden from tfield_. */
	void init_specialized(twindow& window)
	{
		if(callback_change_) {
			tselectable_* widget =
				dynamic_cast<tselectable_*>(window.find(id(), false));

			if(widget) {
				widget->set_callback_state_change(callback_change_);
			}
		}
	}

	void (*callback_change_) (twidget* widget);
};

/** Specialized field class for text. */
class tfield_text : public tfield<std::string, ttext_, const std::string& >
{
public:
	tfield_text(const std::string& id,
			const bool optional,
			std::string (*callback_load_value) (),
			void (*callback_save_value) (const std::string& value)) :
		tfield<std::string, ttext_, const std::string& >
			(id, optional, callback_load_value, callback_save_value)
		{
		}

private:
	/** Overridden from tfield_. */
	void finalize_specialized(twindow& window)
	{
		ttext_box* widget = dynamic_cast<ttext_box*>(window.find(id(), false));

		if(widget) {
			widget->save_to_history();
		}
	}
};

} // namespace gui2

#endif

