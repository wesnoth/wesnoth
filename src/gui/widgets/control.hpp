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

#ifndef GUI_WIDGETS_CONTROL_HPP_INCLUDED
#define GUI_WIDGETS_CONTROL_HPP_INCLUDED

#include "gui/core/widget_definition.hpp"
#include "gui/core/window_builder.hpp"
#include "gui/widgets/widget.hpp"
#include "../../text.hpp" // We want the file in src/

namespace gui2
{

// ------------ WIDGET -----------{

namespace implementation
{
struct tbuilder_control;
} // namespace implementation

/** Base class for all visible items. */
class tcontrol : public twidget
{
	friend class tdebug_layout_graph;

public:
	/** @deprecated Used the second overload. */
	explicit tcontrol(const unsigned canvas_count);

	/**
	 * Constructor.
	 *
	 * @param builder             The builder object with the settings for the
	 *                            object.
	 *
	 * @param canvas_count        The number of canvasses in the control.
	 */
	tcontrol(const implementation::tbuilder_control& builder,
			 const unsigned canvas_count,
			 const std::string& control_type);

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
	 * @param data                Map with the key value pairs to set the
	 *                            members.
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
	 * See @ref twidget::disable_click_dismiss.
	 *
	 * The default behavior is that a widget blocks easy close, if not it
	 * should override this function.
	 */
	bool disable_click_dismiss() const OVERRIDE;

	/** See @ref twidget::create_walker. */
	virtual iterator::twalker_* create_walker() OVERRIDE;

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

	/**
	 * Returns the number of characters per line.
	 *
	 * This value is used to call @ref ttext::set_characters_per_line
	 * (indirectly).
	 *
	 * @returns                   The characters per line. This implementation
	 *                            always returns 0.
	 */
	virtual unsigned get_characters_per_line() const;

	/**
	 * Returns whether the label should be link_aware, in
	 * in rendering and in searching for links with get_link.
	 *
	 * This value is used to call @ref ttext::set_link_aware
	 * (indirectly).
	 *
	 * @returns		      The link aware status. This impl always
	 *			      always returns false.
	 */
	virtual bool get_link_aware() const;

	/**
	 * Returns the color string to be used with links.
	 *
	 * This value is used to call @ref ttext::set_link_color
	 * (indirectly).
	 *
	 * @returns		      The link color string. This impl returns "#ffff00".
	 *
	 */
	virtual std::string get_link_color() const;

	/**
	 * See @ref twidget::layout_initialise.
	 *
	 * @todo Also handle the tooltip state.
	 * Handle if shrunken_ && use_tooltip_on_label_overflow_.
	 */
	virtual void layout_initialise(const bool full_initialisation) OVERRIDE;

	/** See @ref twidget::request_reduce_width. */
	virtual void request_reduce_width(const unsigned maximum_width) OVERRIDE;

protected:
	/** See @ref twidget::calculate_best_size. */
	virtual tpoint calculate_best_size() const OVERRIDE;

public:
	/** See @ref twidget::place. */
	virtual void place(const tpoint& origin, const tpoint& size) OVERRIDE;

	/***** ***** ***** ***** Inherited ***** ***** ***** *****/

private:
	/**
	 * Loads the configuration of the widget.
	 *
	 * Controls have their definition stored in a definition object. In order to
	 * determine sizes and drawing the widget this definition needs to be
	 * loaded. The member definition_ contains the name of the definition and
	 * function load the proper configuration.
	 *
	 * @deprecated @ref definition_load_configuration() is the replacement.
	 */
	void load_config();

	/**
	 * Uses the load function.
	 *
	 * @note This doesn't look really clean, but the final goal is refactor
	 * more code and call load_config in the ctor, removing the use case for
	 * the window. That however is a longer termine refactoring.
	 */
	friend class twindow;

public:
	/** See @ref twidget::find_at. */
	virtual twidget* find_at(const tpoint& coordinate,
							 const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find_at. */
	virtual const twidget* find_at(const tpoint& coordinate,
								   const bool must_be_active) const OVERRIDE;

	/** See @ref twidget::find. */
	twidget* find(const std::string& id, const bool must_be_active) OVERRIDE;

	/** See @ref twidget::find. */
	const twidget* find(const std::string& id,
						const bool must_be_active) const OVERRIDE;

	/**
	 * Sets the definition.
	 *
	 * This function sets the definition of a control and should be called soon
	 * after creating the object since a lot of internal functions depend on the
	 * definition.
	 *
	 * This function should be called one time only!!!
	 */
	void set_definition(const std::string& definition);

	/***** ***** ***** setters / getters for members ***** ****** *****/
	bool get_use_tooltip_on_label_overflow() const
	{
		return use_tooltip_on_label_overflow_;
	}
	void set_use_tooltip_on_label_overflow(const bool use_tooltip = true)
	{
		use_tooltip_on_label_overflow_ = use_tooltip;
	}

	const t_string& label() const
	{
		return label_;
	}
	virtual void set_label(const t_string& label);

	virtual void set_use_markup(bool use_markup);
	bool get_use_markup() const
	{
		return use_markup_;
	}

	const t_string& tooltip() const
	{
		return tooltip_;
	}
	// Note setting the tooltip_ doesn't dirty an object.
	void set_tooltip(const t_string& tooltip)
	{
		tooltip_ = tooltip;
		set_wants_mouse_hover(!tooltip_.empty());
	}

	const t_string& help_message() const
	{
		return help_message_;
	}
	// Note setting the help_message_ doesn't dirty an object.
	void set_help_message(const t_string& help_message)
	{
		help_message_ = help_message;
	}

	// const versions will be added when needed
	std::vector<tcanvas>& canvas()
	{
		return canvas_;
	}
	tcanvas& canvas(const unsigned index)
	{
		assert(index < canvas_.size());
		return canvas_[index];
	}

	void set_text_alignment(const PangoAlignment text_alignment);
	PangoAlignment get_text_alignment() const
	{
		return text_alignment_;
	}

protected:
	tresolution_definition_ptr config()
	{
		return config_;
	}
	tresolution_definition_const_ptr config() const
	{
		return config_;
	}

	void set_config(tresolution_definition_ptr config)
	{
		config_ = config;
	}

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
	/**
	 * The definition is the id of that widget class.
	 *
	 * Eg for a button it [button_definition]id. A button can have multiple
	 * definitions which all look different but for the engine still is a
	 * button.
	 */
	std::string definition_;

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
	 * pointer to the definition. The definition is resolution dependent, where
	 * the resolution is the size of the Wesnoth application window. Depending
	 * on the resolution widgets can look different, use different fonts.
	 * Windows can use extra scrollbars use abbreviations as text etc.
	 */
	tresolution_definition_ptr config_;

	/**
	 * Load class dependent config settings.
	 *
	 * load_config will call this method after loading the config, by default it
	 * does nothing but classes can override it to implement custom behavior.
	 */
	virtual void load_config_extra()
	{
	}

	/**
	 * Loads the configuration of the widget.
	 *
	 * Controls have their definition stored in a definition object. In order to
	 * determine sizes and drawing the widget this definition needs to be
	 * loaded. The member definition_ contains the name of the definition and
	 * function load the proper configuration.
	 */
	void definition_load_configuration(const std::string& control_type);

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
	/** See @ref twidget::impl_draw_background. */
	virtual void impl_draw_background(surface& frame_buffer,
									  int x_offset,
									  int y_offset) OVERRIDE;

	/** See @ref twidget::impl_draw_foreground. */
	virtual void impl_draw_foreground(surface& frame_buffer,
									  int x_offset,
									  int y_offset) OVERRIDE;

	/** Exposes font::ttext::get_token, for the text label of this control */
	std::string get_label_token(const gui2::tpoint & position, const char * delimiters = " \n\r\t") const;

	std::string get_label_link(const gui2::tpoint & position) const;

private:
#ifdef GUI2_EXPERIMENTAL_LISTBOX
	/**
	 * Initializes the control.
	 *
	 * Not everything can be code in the constructor since virtual functions
	 * can't be used. So after construction this function needs to be called and
	 * only once, this happens when set_definition is called.
	 */
	virtual void init()
	{
	}
#endif

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
							  const tpoint& maximum_size = tpoint(0, 0)) const;

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

	/** The alignment of the text in a control. */
	PangoAlignment text_alignment_;

	/** Is the widget smaller as it's best size? */
	bool shrunken_;

	/***** ***** ***** signal handlers ***** ****** *****/

	void signal_handler_show_tooltip(const event::tevent event,
									 bool& handled,
									 const tpoint& location);

	void signal_handler_show_helptip(const event::tevent event,
									 bool& handled,
									 const tpoint& location);

	void signal_handler_notify_remove_tooltip(const event::tevent event,
											  bool& handled);
};

// }---------- BUILDER -----------{

class tcontrol;

namespace implementation
{

struct tbuilder_control : public tbuilder_widget
{
public:
	tbuilder_control(const config& cfg);

	using tbuilder_widget::build;

	virtual twidget* build(const treplacements& replacements) const OVERRIDE;

	/** @deprecated The control can initialize itself. */
	void init_control(tcontrol* control) const;

	/** Parameters for the control. */
	std::string definition;
	t_string label;
	t_string tooltip;
	t_string help;
	bool use_tooltip_on_label_overflow;
};

} // namespace implementation

// }------------ END --------------

} // namespace gui2

#endif
