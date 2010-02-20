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

#ifndef GUI_WIDGETS_CONTROL_HPP_INCLUDED
#define GUI_WIDGETS_CONTROL_HPP_INCLUDED

#include "gui/auxiliary/canvas.hpp"
#include "gui/auxiliary/widget_definition.hpp"
#include "gui/widgets/widget.hpp"
#include "../../text.hpp"
#include "tstring.hpp"

#include <cassert>

typedef std::map<std::string,t_string> string_map;

namespace gui2 {

/** Base class for all visible items. */
class tcontrol : public virtual twidget
{
	friend class tdebug_layout_graph;
public:

	tcontrol(const unsigned canvas_count);

	virtual ~tcontrol() {}

	/**
	 * Sets the members of the control.
	 *
	 * The map contains named members it can set, controls inheriting from us
	 * can add additional members to set by this function. The following
	 * members can by the following key:
	 *  * label_                  label
	 *  * tooltip_                tooltip
	 *  * help_message_           help
	 *
	 *
	 * @param data                Map with the key value pairs to set the members.
	 */
	virtual void set_members(const string_map& data);

	/***** ***** ***** ***** State handling ***** ***** ***** *****/

	/**
	 * Sets the control's state.
	 *
	 *  Sets the control in the active state, when inactive a control can't be
	 *  used and doesn't react to events. (Note read-only for a ttext_ is a
	 *  different state.)
	 */
	virtual void set_active(const bool active) = 0;

	/** Gets the active state of the control. */
	virtual bool get_active() const = 0;

protected:
	/** Returns the id of the state.
	 *
	 * The current state is also the index canvas_.
	 */
	virtual unsigned get_state() const = 0;

public:

	/***** ***** ***** ***** Easy close handling ***** ***** ***** *****/

	/**
	 * Inherited from twidget.
	 *
	 * The default behavious is that a widget blocks easy close, if not it
	 * hould override this function.
	 */
	bool disable_click_dismiss() const;

	/***** ***** ***** ***** layout functions ***** ***** ***** *****/

	/**
	 * Gets the minimum size as defined in the config.
	 *
	 * @pre                       config_ !=  NULL
	 *
	 * @returns                   The size.
	 */
	tpoint get_config_minimum_size() const;

	/**
	 * Gets the default size as defined in the config.
	 *
	 * @pre                       config_ !=  NULL
	 *
	 * @returns                   The size.
	 */
	tpoint get_config_default_size() const;

	/**
	 * Gets the best size as defined in the config.
	 *
	 * @pre                       config_ !=  NULL
	 *
	 * @returns                   The size.
	 */
	tpoint get_config_maximum_size() const;

	/** Inherited from twidget. */
	/** @todo Also handle the tooltip state if shrunken_ &&
	 * use_tooltip_on_label_overflow_. */
	void layout_init(const bool full_initialization);

	/** Inherited from twidget. */
	void request_reduce_width(const unsigned maximum_width);

protected:
	/** Inherited from twidget. */
	tpoint calculate_best_size() const;
public:

	/** Inherited from twidget. */
	void place(const tpoint& origin, const tpoint& size);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/**
	 * Inherited from twidget.
	 *
	 * This function shouldn't be called directly it's called by set_definition().
	 * All classes which use this class as base should call this function in
	 * their constructor. Abstract classes shouldn't call this routine. The
	 *
	 * classes which call this routine should also define get_control_type().
	 */
	void load_config();

	/** Inherited from twidget. */
	twidget* find_at(const tpoint& coordinate, const bool must_be_active)
	{
		return (twidget::find_at(coordinate, must_be_active)
			&& (!must_be_active || get_active())) ? this : 0;
	}

	/** Inherited from twidget. */
	const twidget* find_at(const tpoint& coordinate,
			const bool must_be_active) const
	{
		return (twidget::find_at(coordinate, must_be_active)
			&& (!must_be_active || get_active())) ? this : 0;
	}

	/** Inherited from twidget.*/
	twidget* find(const std::string& id, const bool must_be_active)
	{
		return (twidget::find(id, must_be_active)
			&& (!must_be_active || get_active())) ? this : 0;
	}

	/** Inherited from twidget.*/
	const twidget* find(const std::string& id,
			const bool must_be_active) const
	{
		return (twidget::find(id, must_be_active)
			&& (!must_be_active || get_active())) ? this : 0;
	}

	/**
	 * Inherited from twidget.
	 *
	 * This function sets the defintion of a control and should be called soon
	 * after creating the object since a lot of internal functions depend on the
	 * definition.
	 *
	 * This function should be called one time only!!!
	 */
	void set_definition(const std::string& definition);

	/***** ***** ***** setters / getters for members ***** ****** *****/
	bool get_use_tooltip_on_label_overflow() const { return use_tooltip_on_label_overflow_; }
	void set_use_tooltip_on_label_overflow(const bool use_tooltip = true)
		{ use_tooltip_on_label_overflow_ = use_tooltip; }

	const t_string& label() const { return label_; }
	virtual void set_label(const t_string& label);

	virtual void set_use_markup(bool use_markup);
	bool get_use_markup() const { return use_markup_; }

	const t_string& tooltip() const { return tooltip_; }
	// Note setting the tooltip_ doesn't dirty an object.
	void set_tooltip(const t_string& tooltip)
		{ tooltip_ = tooltip; set_wants_mouse_hover(!tooltip_.empty()); }

	const t_string& help_message() const { return help_message_; }
	// Note setting the help_message_ doesn't dirty an object.
	void set_help_message(const t_string& help_message) { help_message_ = help_message; }

	// const versions will be added when needed
	std::vector<tcanvas>& canvas() { return canvas_; }
	tcanvas& canvas(const unsigned index)
		{ assert(index < canvas_.size()); return canvas_[index]; }

protected:
	tresolution_definition_ptr config() { return config_; }
	tresolution_definition_const_ptr config() const { return config_; }

	void set_config(tresolution_definition_ptr config) { config_ = config; }

	/***** ***** ***** ***** miscellaneous ***** ***** ***** *****/

	/**
	 * Updates the canvas(ses).
	 *
	 * This function should be called if either the size of the widget changes
	 * or the text on the widget changes.
	 */
	virtual void update_canvas();

	/**
	 * Returns the maximum width available for the text.
	 *
	 * This value makes sense after the widget has been given a size, since the
	 * maximum width is based on the width of the widget.
	 */
	int get_text_maximum_width() const;

	/**
	 * Returns the maximum height available for the text.
	 *
	 * This value makes sense after the widget has been given a size, since the
	 * maximum height is based on the height of the widget.
	 */
	int get_text_maximum_height() const;

private:
	/** Contain the non-editable text associated with control. */
	t_string label_;

	/** Use markup for the label? */
	bool use_markup_;

	/**
	 * If the text doesn't fit on the label should the text be used as tooltip?
	 *
	 * This only happens if the tooltip is empty.
	 */
	bool use_tooltip_on_label_overflow_;

	/**
	 * Tooltip text.
	 *
	 * The hovering event can cause a small tooltip to be shown, this is the
	 * text to be shown. At the moment the tooltip is a single line of text.
	 */
	t_string tooltip_;

	/**
	 * Tooltip text.
	 *
	 * The help event can cause a tooltip to be shown, this is the text to be
	 * shown. At the moment the tooltip is a single line of text.
	 */
	t_string help_message_;

	/**
	 * Holds all canvas objects for a control.
	 *
	 * A control can have multiple states, which are defined in the classes
	 * inheriting from us. For every state there is a separate canvas, which is
	 * stored here. When drawing the state is determined and that canvas is
	 * drawn.
	 */
	std::vector<tcanvas> canvas_;

	/**
	 * Contains the pointer to the configuration.
	 *
	 * Every control has a definition of how it should look, this contains a
	 * pointer to the definition. The definition is resolution dependant, where
	 * the resolution is the size of the Wesnoth application window. Depending
	 * on the resolution widgets can look different, use different fonts.
	 * Windows can use extra scrollbars use abbreviations as text etc.
	 */
	tresolution_definition_ptr config_;

	/**
	 * Load class dependant config settings.
	 *
	 * load_config will call this method after loading the config, by default it
	 * does nothing but classes can override it to implement custom behaviour.
	 */
	virtual void load_config_extra() {}
public:
	/**
	 * Returns the control_type of the control.
	 *
	 * The control_type parameter for tgui_definition::get_control() To keep the
	 * code more generic this type is required so the controls need to return
	 * the proper string here.  Might be used at other parts as well the get the
	 * type of
	 * control involved.
	 */
	virtual const std::string& get_control_type() const = 0;

protected:
	/** Inherited from twidget. */
	void impl_draw_background(surface& frame_buffer);

	/** Inherited from twidget. */
	void impl_draw_foreground(surface& /*frame_buffer*/) { /* do nothing */ }
private:

	/**
	 * Gets the best size for a text.
	 *
	 * @param minimum_size        The minimum size of the text.
	 * @param maximum_size        The wanted maximum size of the text, if not
	 *                            possible it's ignored. A value of 0 means
	 *                            that it's ignored as well.
	 *
	 * @returns                   The best size.
	 */
	tpoint get_best_text_size(const tpoint& minimum_size,
		const tpoint& maximum_size = tpoint(0,0)) const;

	/**
	 * Contains a helper cache for the rendering.
	 *
	 * Creating a ttext object is quite expensive and is done on various
	 * occasions so it's cached here.
	 *
	 * @todo Maybe if still too slow we might also copy this cache to the
	 * canvas so it can reuse our results, but for now it seems fast enough.
	 * Unfortunately that would make the dependency between the classes bigger
	 * as wanted.
	 */
	mutable font::ttext renderer_;

	/** The maximum width for the text in a control. */
	int text_maximum_width_;

	/** Is the widget smaller as it's best size? */
	bool shrunken_;
};

} // namespace gui2

#endif

