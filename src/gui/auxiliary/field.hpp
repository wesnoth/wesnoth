/*
   Copyright (C) 2008 - 2017 by Mark de Wever <koraq@xs4all.nl>
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

#pragma once

#include "gui/auxiliary/find_widget.hpp"
#include "gui/auxiliary/field-fwd.hpp"
#include "gui/widgets/styled_widget.hpp"
#include "gui/widgets/selectable_item.hpp"
#include "gui/widgets/text_box.hpp"
#include "gui/widgets/window.hpp"
#include "wml_exception.hpp"

namespace gui2
{

/**
 * Abstract base class for the fields.
 *
 * @note In this context a widget is a @ref gui2::styled_widget and not a @ref
 * gui2::widget. This name widget is a generic name and fits, however some
 * functions used are first declared in a styled_widget.
 */
class field_base
{
public:
	/**
	 * Constructor.
	 *
	 * @param id                  The id of the widget to connect to the window.
	 *                            A widget can only be connected once.
	 * @param mandatory           Is the widget mandatory
	 */
	field_base(const std::string& id, const bool mandatory)
		: id_(id), mandatory_(mandatory), widget_(nullptr)
	{
	}

	virtual ~field_base()
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
	 * @pre widget_ == nullptr
	 *
	 * @param window               The window to be attached to.
	 */
	void attach_to_window(window& window)
	{
		assert(!widget_);
		widget_ = find_widget<styled_widget>(&window, id(), false, mandatory_);
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
	void widget_init(window& window)
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
	void widget_finalize(window& window)
	{
		finalize_generic(window);
		finalize_specialized(window);
	}

	/**
	 * Detaches the field from a window.
	 *
	 * @pre widget_ != nullptr || !mandatory_
	 */
	void detach_from_window()
	{
		assert(!mandatory_ || widget_);
		widget_ = nullptr;
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
	virtual void widget_save(window& window) = 0;

	/**
	 * Restores a widget.
	 *
	 * See widget_save for more info.
	 *
	 * @param window              The window containing the widget.
	 */
	virtual void widget_restore(window& window) = 0;

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
	void widget_set_enabled(window& window, const bool enable, const bool sync)
	{
		styled_widget* widget = dynamic_cast<styled_widget*>(window.find(id(), false));

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

	styled_widget* get_widget()
	{
		return widget_;
	}

	const styled_widget* get_widget() const
	{
		return widget_;
	}

private:
	/** The id field of the widget, should be unique in a window. */
	const std::string id_;

	/** Is the widget optional or mandatory in this window. */
	const bool mandatory_;

	/** The widget attached to the field. */
	styled_widget* widget_;

	/** See widget_init. */
	virtual void init_generic(window& window) = 0;

	/** See widget_init. */
	virtual void init_specialized(window& /*window*/)
	{
	}


	/** See widget_finalize. */
	virtual void finalize_generic(window& window) = 0;

	/** See widget_finalize. */
	virtual void finalize_specialized(window& /*window*/)
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
class field : public field_base
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
	field(const std::string& id,
		   const bool mandatory,
		   const std::function<T()>& callback_load_value,
		   const std::function<void(CT)>& callback_save_value)
		: field_base(id, mandatory)
		, value_(T())
		, link_(value_)
		, callback_load_value_(callback_load_value)
		, callback_save_value_(callback_save_value)
	{
		static_assert((!std::is_same<styled_widget, W>::value), "Second template argument cannot be styled_widget");
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
	field(const std::string& id, const bool mandatory, T& linked_variable)
		: field_base(id, mandatory)
		, value_(T())
		, link_(linked_variable)
		, callback_load_value_(std::function<T()>())
		, callback_save_value_(std::function<void(CT)>())
	{
		static_assert((!std::is_same<styled_widget, W>::value), "Second template argument cannot be styled_widget");
	}

	/**
	 * Constructor.
	 *
	 * This version is used for read only variables.
	 *
	 * @note The difference between this constructor and the one above is the
	 * sending of the third parameter as const ref instead of a non-const ref.
	 * So it feels a bit tricky. Since this constructor is only used for a
	 * the @ref styled_widget class and the other constructors not the issue is
	 * solved by using static asserts to test whether the proper constructor
	 * is used.
	 *
	 * @param mandatory            Is the widget mandatory?
	 * @param id                  The id of the widget to connect to the window.
	 *                            A widget can only be connected once.
	 * @param value               The value of the widget.
	 */
	field(const std::string& id, const bool mandatory, const T& value)
		: field_base(id, mandatory)
		, value_(value)
		, link_(value_)
		, callback_load_value_(std::function<T()>())
		, callback_save_value_(std::function<void(CT)>())
	{
		static_assert((std::is_same<styled_widget, W>::value), "Second template argument must be styled_widget");
	}

	/** Inherited from field_base. */
	void widget_restore(window& window)
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
	void set_widget_value(window& window, CT value)
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

	/** Inherited from field_base. */
	void widget_save(window& window)
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
	T get_widget_value(window& window)
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
	std::function<T()> callback_load_value_;

	/** Inherited from field_base. */
	void init_generic(window& window)
	{
		validate_widget(window);

		if(callback_load_value_) {
			value_ = callback_load_value_();
		} else {
			value_ = link_;
		}

		restore(window);
	}

	/** Inherited from field_base. */
	void finalize_generic(window& window)
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
	std::function<void(CT)> callback_save_value_;

	/**
	 * Test whether the widget exists if the widget is mandatory.
	 *
	 * @param window              The window containing the widget.
	 */
	void validate_widget(window& window)
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
	void save(window& window, const bool must_be_active);

	/**
	 * Stores the internal value_ in the widget.
	 *
	 * @param window              The window containing the widget.
	 */
	void restore(window& window);
};

template <class T, class W, class CT>
void field<T, W, CT>::save(window& window, const bool must_be_active)
{
	const W* widget
			= find_widget<const W>(&window, id(), must_be_active, false);

	if(widget) {
		value_ = widget->get_value();
	}
}

template <>
inline void field<bool, selectable_item>::save(
		window& window, const bool must_be_active)
{
	const selectable_item* selectable
			= find_widget<const selectable_item>(&window, id(), must_be_active, false);

	if(selectable) {
		value_ = selectable->get_value_bool();
	}
}

template <>
inline void field<std::string, styled_widget, const std::string&>::save(
		window& window, const bool must_be_active)
{
	const styled_widget* control
			= find_widget<const styled_widget>(&window, id(), must_be_active, false);

	if(control) {
		value_ = control->get_label();
	}
}

template <class T, class W, class CT>
void field<T, W, CT>::restore(window& window)
{
	W* widget = find_widget<W>(&window, id(), false, false);

	if(widget) {
		widget->set_value(value_);
	}
}

template <>
inline void
field<std::string, styled_widget, const std::string&>::restore(window& window)
{
	styled_widget* control = find_widget<styled_widget>(&window, id(), false, false);

	if(control) {
		control->set_label(value_);
	}
}

/** Specialized field class for boolean. */
class field_bool : public field<bool, selectable_item>
{
public:
	field_bool(const std::string& id,
				const bool mandatory,
				const std::function<bool()>& callback_load_value,
				const std::function<void(const bool)>& callback_save_value,
				const std::function<void(widget&)>& callback_change,
				const bool initial_fire)
		: field<bool, gui2::selectable_item>(
				  id, mandatory, callback_load_value, callback_save_value)
		, callback_change_(callback_change)
		, initial_fire_(initial_fire)
	{
	}

	field_bool(const std::string& id,
				const bool mandatory,
				bool& linked_variable,
				const std::function<void(widget&)>& callback_change,
				const bool initial_fire)
		: field<bool, gui2::selectable_item>(id, mandatory, linked_variable)
		, callback_change_(callback_change)
		, initial_fire_(initial_fire)
	{
	}

private:
	/** Overridden from field_base. */
	void init_specialized(window& window)
	{
		if(callback_change_) {
			if(widget* widget = window.find(id(), false)) {
				if(initial_fire_) {
					callback_change_(*widget);
				}

				dynamic_cast<selectable_item*>(widget)->set_callback_state_change(callback_change_);
			}
		}
	}

	std::function<void(widget&)> callback_change_;

	const bool initial_fire_;
};

/** Specialized field class for text. */
class field_text : public field<std::string, text_box_base, const std::string&>
{
public:
	field_text(const std::string& id,
				const bool mandatory,
				const std::function<std::string()>& callback_load_value,
				const std::function<void(const std::string&)>&
						callback_save_value)
		: field<std::string, text_box_base, const std::string&>(
				  id, mandatory, callback_load_value, callback_save_value)
	{
	}

	field_text(const std::string& id,
				const bool mandatory,
				std::string& linked_variable)
		: field<std::string, text_box_base, const std::string&>(
				  id, mandatory, linked_variable)
	{
	}

private:
	/** Overridden from field_base. */
	void finalize_specialized(window& window)
	{
		text_box* widget = dynamic_cast<text_box*>(window.find(id(), false));

		if(widget) {
			widget->save_to_history();
		}
	}
};

/** Specialized field class for a styled_widget, used for labels and images. */
class field_label : public field<std::string, styled_widget, const std::string&>
{
public:
	field_label(const std::string& id,
				 const bool mandatory,
				 const std::string& text,
				 const bool use_markup)
		: field<std::string, styled_widget, const std::string&>(id, mandatory, text)
		, use_markup_(use_markup)

	{
	}

private:
	/** Whether or not the label uses markup. */
	bool use_markup_;

	/** Overridden from field_base. */
	void init_specialized(window& window)
	{
		find_widget<styled_widget>(&window, id(), false).set_use_markup(use_markup_);
	}
};

} // namespace gui2
