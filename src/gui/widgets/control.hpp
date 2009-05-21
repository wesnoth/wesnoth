/* $Id$ */
/*
   Copyright (C) 2008 - 2009 by Mark de Wever <koraq@xs4all.nl>
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
#include "gui/widgets/settings.hpp"
#include "gui/widgets/widget.hpp"
#include "../../text.hpp"
#include "tstring.hpp"

#include <cassert>

namespace gui2 {

/** Base class for all visible items. */
class tcontrol : public virtual twidget
{
	friend class tdebug_layout_graph;
public:

	tcontrol(const unsigned canvas_count);

	virtual ~tcontrol() {}

	/**
	 * The markup mode of the label.
	 *
	 * The markup could have been a boolean but since we have the old
	 * wml markup as well we need to convert that markup to the markup used
	 * by pango.
	 *
	 * @todo once the wml markup is phased out this enum can be removed.
	 */
	enum tmarkup_mode {
		NO_MARKUP,    /**< The control doesn't use markup for its text. */
		PANGO_MARKUP, /**< The control uses the pango markup feature. */
		WML_MARKUP   /**<
					    * The control uses the wml_markup feature.
					    * The engine can only handle pango markup and this
					    * markup will be converted to pango markup before
					    * sending to the layout engine.
					    */
	};

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
	 * Adds or removes a widget to/from the easy close block list.
	 *
	 * For adding to the block list an id is required, if not set it will get
	 * one.
	 *
	 * @param block               If true it's added to the blocklist, removed
	 *                            otherwise.
	 */
	void set_block_easy_close(const bool block = true);

	/**
	 * Does the widget block the easy close feature?
	 *
	 * NOTE widgets that return true here, probably also need to make
	 * modifications to their set_state() function. Easy close blocking _must_
	 * be disabled when the widget is disabled or not visible. (The tcontrol
	 * class handles the visibility part.)
	 *
	 * @returns                   Whether or not it blocks.
	 */
	virtual bool does_block_easy_close() const = 0;

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
	void NEW_layout_init(const bool full_initialization);

	/** Inherited from twidget. */
	void NEW_request_reduce_width(const unsigned maximum_width);

	/** Inherited from twidget. */
	void layout_wrap(const unsigned maximum_width);

protected:
	/** Inherited from twidget. */
	tpoint calculate_best_size() const;
public:

	/** Inherited from twidget. */
	void set_size(const tpoint& origin, const tpoint& size);

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

	/** Inherted from tevent_executor. */
	void mouse_hover(tevent_handler& event);

	/** Inherted from tevent_executor. */
	void help_key(tevent_handler& event);

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
	twidget* find_widget(const tpoint& coordinate, const bool must_be_active)
	{
		return (twidget::find_widget(coordinate, must_be_active)
			&& (!must_be_active || get_active())) ? this : 0;
	}

	/** Inherited from twidget. */
	const twidget* find_widget(const tpoint& coordinate,
			const bool must_be_active) const
	{
		return (twidget::find_widget(coordinate, must_be_active)
			&& (!must_be_active || get_active())) ? this : 0;
	}

	/** Inherited from twidget.*/
	twidget* find_widget(const std::string& id, const bool must_be_active)
	{
		return (twidget::find_widget(id, must_be_active)
			&& (!must_be_active || get_active())) ? this : 0;
	}

	/** Inherited from twidget.*/
	const twidget* find_widget(const std::string& id,
			const bool must_be_active) const
	{
		return (twidget::find_widget(id, must_be_active)
			&& (!must_be_active || get_active())) ? this : 0;
	}

	/** Import overloaded versions. */
	using twidget::find_widget;

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

	virtual void set_markup_mode(const tmarkup_mode markup_mode);
	tmarkup_mode get_markup_mode() const { return markup_mode_; }

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

	/** The markup mode of the label_. */
	tmarkup_mode markup_mode_;

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
protected:
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

	/** Converts the label_ to a pango markup string. */
	std::string get_pango_markup() const;
};

} // namespace gui2

#endif

