/*
   Copyright (C) 2008 - 2016 by Mark de Wever <koraq@xs4all.nl>
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
 * @file
 * Implements some helper classes to ease adding fields to a dialog and hide
 * the synchronization needed. Since some templates are used all is stored in
 * the header.
 *
 */

#ifndef GUI_DIALOGS_FIELD_HPP_INCLUDED
#define GUI_DIALOGS_FIELD_HPP_INCLUDED

#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/field-fwd.hpp"
#include "gui/widgets/control.hpp"
#include "gui/widgets/selectable.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

#include <boost/static_assert.hpp>

namespace gui2
{

/**
 * Abstract base class for the fields.
 *
 * @note In this context a widget is a @ref gui2::tcontrol and not a @ref
 * gui2::twidget. This name widget is a generic name and fits, however some
 * functions used are first declared in a control.
 */
class tfield_
{
public:
	/**
	 * Constructor.
	 *
	 * @param id                  The id of the widget to connect to the window.
	 *                            A widget can only be connected once.
	 * @param mandatory           Is the widget mandatory
	 */
	tfield_(const std::string& id, const bool mandatory)
		: id_(id), mandatory_(mandatory), widget_(NULL)
	{
	}

	virtual ~tfield_()
	{
	}

	/**
	 * Attaches the field to a window.
	 *
	 * When attached the widget which we're a wrapper around is stored linked
	 * in here.
	 *
	 * @warning After attaching the window must remain a valid. Before the
	 * window is destroyed the @ref detach_from_window function must be called.
	 *
	 * @todo Most functions that have a window parameter only use it to get the
	 * widget. Evaluate and remove the window parameter where applicable.
	 *
	 * @pre widget_ == NULL
	 *
	 * @param window               The window to be attached to.
	 */
	void attach_to_window(twindow& window)
	{
		assert(!widget_);
		widget_ = find_widget<tcontrol>(&window, id(), false, mandatory_);
	}

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
	 * Detaches the field from a window.
	 *
	 * @pre widget_ != NULL || !mandatory_
	 */
	void detach_from_window()
	{
		assert(!mandatory_ || widget_);
		widget_ = NULL;
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
	 * @param enable              If true enables the widget, disables
	 *                            otherwise.
	 * @param sync                If the state is changed do we need to
	 *                            synchronize. Upon disabling, write the value
	 *                            of the widget in the variable value_. Upon
	 *                            enabling write the value of value_ in the
	 *                            widget.
	 */
	void widget_set_enabled(twindow& window, const bool enable, const bool sync)
	{
		tcontrol* widget = dynamic_cast<tcontrol*>(window.find(id(), false));

		if(!widget) {
			return;
		}

		const bool widget_state = widget->get_active();
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

	/***** ***** ***** setters / getters for members ***** ****** *****/

	const std::string& id() const
	{
		return id_;
	}

	bool is_mandatory() const
	{
		return mandatory_;
	}

	tcontrol* widget()
	{
		return widget_;
	}

	const tcontrol* widget() const
	{
		return widget_;
	}

private:
	/** The id field of the widget, should be unique in a window. */
	const std::string id_;

	/** Is the widget optional or mandatory in this window. */
	const bool mandatory_;

	/** The widget attached to the field. */
	tcontrol* widget_;

	/** See widget_init. */
	virtual void init_generic(twindow& window) = 0;

	/** See widget_init. */
	virtual void init_specialized(twindow& /*window*/)
	{
	}


	/** See widget_finalize. */
	virtual void finalize_generic(twindow& window) = 0;

	/** See widget_finalize. */
	virtual void finalize_specialized(twindow& /*window*/)
	{
	}
};

/**
 * Template class to implement the generic field implementation.
 *
 * @tparam T                      The type of the item to show in the widget.
 * @tparam W                      The type of widget to show, this is not a
 *                                widget class but a behavior class.
 * @tparam CT                     The type tp be used in the
 *                                callback_save_value callback. Normally this
 *                                is const T but for example with strings it
 *                                can be const T&. Note the const needs to be
 *                                in the template otherwise compilation on
 *                                GCC-4.3 fails (not sure whether compiler bug
 *                                or not).
 */
template <class T, class W, class CT>
class tfield : public tfield_
{
public:
	/**
	 * Constructor.
	 *
	 * @param id                  The id of the widget to connect to the window.
	 *                            A widget can only be connected once.
	 * @param mandatory           Is the widget mandatory?
	 * @param callback_load_value A callback function which is called when the
	 *                            window is shown. This callback returns the
	 *                            initial value of the field.
	 * @param callback_save_value A callback function which is called when the
	 *                            window closed with the OK button. The
	 *                            callback is executed with the new value of
	 *                            the field. It's meant to set the value of
	 *                            some variable in the engine after the window
	 *                            is closed with OK.
	 */
	tfield(const std::string& id,
		   const bool mandatory,
		   const boost::function<T()>& callback_load_value,
		   const boost::function<void(CT)>& callback_save_value)
		: tfield_(id, mandatory)
		, value_(T())
		, link_(value_)
		, callback_load_value_(callback_load_value)
		, callback_save_value_(callback_save_value)
	{
		BOOST_STATIC_ASSERT((!boost::is_same<tcontrol, W>::value));
	}

	/**
	 * Constructor.
	 *
	 * @param id                  The id of the widget to connect to the window.
	 *                            A widget can only be connected once.
	 * @param mandatory           Is the widget mandatory?
	 * @param linked_variable     The variable which is linked to the field.
	 *                            * Upon loading its value is used as initial
	 *                              value of the widget.
	 *                            * Upon closing:
	 *                              * with OK its value is set to the value of
	 *                                the widget.
	 *                              * else, its value is undefined.
	 */
	tfield(const std::string& id, const bool mandatory, T& linked_variable)
		: tfield_(id, mandatory)
		, value_(T())
		, link_(linked_variable)
		, callback_load_value_(boost::function<T()>())
		, callback_save_value_(boost::function<void(CT)>())
	{
		BOOST_STATIC_ASSERT((!boost::is_same<tcontrol, W>::value));
	}

	/**
	 * Constructor.
	 *
	 * This version is used for read only variables.
	 *
	 * @note The difference between this constructor and the one above is the
	 * sending of the third parameter as const ref instead of a non-const ref.
	 * So it feels a bit tricky. Since this constructor is only used for a
	 * the @ref tcontrol class and the other constructors not the issue is
	 * solved by using static asserts to test whether the proper constructor
	 * is used.
	 *
	 * @param mandatory            Is the widget mandatory?
	 * @param id                  The id of the widget to connect to the window.
	 *                            A widget can only be connected once.
	 * @param value               The value of the widget.
	 */
	tfield(const std::string& id, const bool mandatory, const T& value)
		: tfield_(id, mandatory)
		, value_(value)
		, link_(value_)
		, callback_load_value_(boost::function<T()>())
		, callback_save_value_(boost::function<void(CT)>())
	{
		BOOST_STATIC_ASSERT((boost::is_same<tcontrol, W>::value));
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
	 * @deprecated Use references to a variable instead.
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

private:
	/**
	 * The value_ of the widget, this value is also available once the widget
	 * is destroyed.
	 */
	T value_;

	/**
	 * The variable linked to the field.
	 *
	 * When set determines the initial value and the final value is stored here
	 * in the finallizer.
	 */
	T& link_;

	/**
	 * The callback function to load the value.
	 *
	 * This is used to load the initial value of the widget, if defined.
	 */
	boost::function<T()> callback_load_value_;

	/** Inherited from tfield_. */
	void init_generic(twindow& window)
	{
		validate_widget(window);

		if(callback_load_value_) {
			value_ = callback_load_value_();
		} else {
			value_ = link_;
		}

		restore(window);
	}

	/** Inherited from tfield_. */
	void finalize_generic(twindow& window)
	{
		save(window, true);

		if(callback_save_value_) {
			callback_save_value_(value_);
		} else {
			link_ = value_;
		}
	}

	/**
	 * The callback function to save the value.
	 *
	 * Once the dialog has been successful this function is used to store the
	 * result of this widget.
	 */
	boost::function<void(CT)> callback_save_value_;

	/**
	 * Test whether the widget exists if the widget is mandatory.
	 *
	 * @param window              The window containing the widget.
	 */
	void validate_widget(twindow& window)
	{
		if(!is_mandatory()) {
			return;
		}
		find_widget<const W>(&window, id(), false);
	}

	/**
	 * Stores the value in the widget in the interval value_.
	 *
	 * @param window              The window containing the widget.
	 * @param must_be_active      If true only active widgets will store their
	 *value.
	 */
	void save(twindow& window, const bool must_be_active);

	/**
	 * Stores the internal value_ in the widget.
	 *
	 * @param window              The window containing the widget.
	 */
	void restore(twindow& window);
};

template <class T, class W, class CT>
void tfield<T, W, CT>::save(twindow& window, const bool must_be_active)
{
	const W* widget
			= find_widget<const W>(&window, id(), must_be_active, false);

	if(widget) {
		value_ = widget->get_value();
	}
}

template <>
inline void tfield<bool, tselectable_>::save(
		twindow& window, const bool must_be_active)
{
	const tselectable_* selectable
			= find_widget<const tselectable_>(&window, id(), must_be_active, false);

	if(selectable) {
		value_ = selectable->get_value_bool();
	}
}

template <>
inline void tfield<std::string, tcontrol, const std::string&>::save(
		twindow& window, const bool must_be_active)
{
	const tcontrol* control
			= find_widget<const tcontrol>(&window, id(), must_be_active, false);

	if(control) {
		value_ = control->label();
	}
}

template <class T, class W, class CT>
void tfield<T, W, CT>::restore(twindow& window)
{
	W* widget = find_widget<W>(&window, id(), false, false);

	if(widget) {
		widget->set_value(value_);
	}
}

template <>
inline void
tfield<std::string, tcontrol, const std::string&>::restore(twindow& window)
{
	tcontrol* control = find_widget<tcontrol>(&window, id(), false, false);

	if(control) {
		control->set_label(value_);
	}
}

/** Specialized field class for boolean. */
class tfield_bool : public tfield<bool, tselectable_>
{
public:
	tfield_bool(const std::string& id,
				const bool mandatory,
				const boost::function<bool()>& callback_load_value,
				const boost::function<void(const bool)>& callback_save_value,
				const boost::function<void(twidget&)>& callback_change)
		: tfield<bool, gui2::tselectable_>(
				  id, mandatory, callback_load_value, callback_save_value)
		, callback_change_(callback_change)
	{
	}

	tfield_bool(const std::string& id,
				const bool mandatory,
				bool& linked_variable,
				const boost::function<void(twidget&)>& callback_change)
		: tfield<bool, gui2::tselectable_>(id, mandatory, linked_variable)
		, callback_change_(callback_change)
	{
	}

private:
	/** Overridden from tfield_. */
	void init_specialized(twindow& window)
	{
		if(callback_change_) {
			tselectable_* widget
					= dynamic_cast<tselectable_*>(window.find(id(), false));

			if(widget) {
				widget->set_callback_state_change(callback_change_);
			}
		}
	}

	boost::function<void(twidget&)> callback_change_;
};

/** Specialized field class for text. */
class tfield_text : public tfield<std::string, ttext_, const std::string&>
{
public:
	tfield_text(const std::string& id,
				const bool mandatory,
				const boost::function<std::string()>& callback_load_value,
				const boost::function<void(const std::string&)>&
						callback_save_value)
		: tfield<std::string, ttext_, const std::string&>(
				  id, mandatory, callback_load_value, callback_save_value)
	{
	}

	tfield_text(const std::string& id,
				const bool mandatory,
				std::string& linked_variable)
		: tfield<std::string, ttext_, const std::string&>(
				  id, mandatory, linked_variable)
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

/** Specialized field class for a control, used for labels and images. */
class tfield_label : public tfield<std::string, tcontrol, const std::string&>
{
public:
	tfield_label(const std::string& id,
				 const bool mandatory,
				 const std::string& text,
				 const bool use_markup)
		: tfield<std::string, tcontrol, const std::string&>(id, mandatory, text)
		, use_markup_(use_markup)

	{
	}

private:
	/** Whether or not the label uses markup. */
	bool use_markup_;

	/** Overridden from tfield_. */
	void init_specialized(twindow& window)
	{
		find_widget<tcontrol>(&window, id(), false).set_use_markup(use_markup_);
	}
};

} // namespace gui2

#endif
