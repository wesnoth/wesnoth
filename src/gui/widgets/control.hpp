/* $Id$ */
/*
   copyright (c) 2008 by mark de wever <koraq@xs4all.nl>
   part of the battle for wesnoth project http://www.wesnoth.org/

   this program is free software; you can redistribute it and/or modify
   it under the terms of the gnu general public license version 2
   or at your option any later version.
   this program is distributed in the hope that it will be useful,
   but without any warranty.

   see the copying file for more details.
*/

#ifndef __GUI_WIDGETS_CONTROL_HPP_INCLUDED__
#define __GUI_WIDGETS_CONTROL_HPP_INCLUDED__

#include "gui/widgets/canvas.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/widget.hpp"
#include "tstring.hpp"

namespace gui2 {

//! Base class for all visible items.
class tcontrol : public virtual twidget
{

	tcontrol();
public:

	tcontrol(const unsigned canvas_count);
	virtual ~tcontrol() {}

	//! Inherted from twidget.
	void mouse_hover(tevent_handler& event);

	//! Inherted from twidget.
	void help_key(tevent_handler& event);

	//! Inherited from twidget.
	void set_size(const SDL_Rect& rect);

	void set_visible(const bool visible = true) 
		{ if(visible_ != visible) { visible_ = visible; set_dirty();} }
	bool get_visible() const { return visible_; }

	void set_multiline_label(const bool multiline = true) 
		{ if(multiline != multiline_label_) { multiline_label_ = multiline; set_dirty(); } }

	void set_label(const t_string& label);

	const t_string& label() const { return label_; }

	// Note setting the tooltip_ doesn't dirty an object.
	void set_tooltip(const t_string& tooltip) 
		{ tooltip_ = tooltip; set_wants_mouse_hover(!tooltip_.empty()); }
	const t_string& tooltip() const { return tooltip_; }

	// Note setting the help_message_ doesn't dirty an object.
	void set_help_message(const t_string& help_message) { help_message_ = help_message; }
	const t_string& help_message() const { return help_message_; }

	std::vector<tcanvas>& canvas() { return canvas_; }
	tcanvas& canvas(const unsigned index) { return canvas_[index]; } // FIXME an assert would be nice

	//! Inherited from twidget.
	void draw(surface& surface);

	//! Sets the control in the active state, when inactive a control can't be
	//! used and doesn't react to events. (Note read-only for a ttext_ is a
	//! different state.)
	virtual void set_active(const bool active) = 0;

	//! Gets the active state of the control.
	virtual bool get_active() const = 0;

	// note we should check whether the label fits in the button
	// Inherited from twidget.
	tpoint get_minimum_size() const;
	tpoint get_best_size() const;
	tpoint get_maximum_size() const;

	//! Inherited from twidget.
	void load_config();

private:
	//! Helpers
	tpoint get_single_line_best_size(const tpoint& config_size) const;
	tpoint get_multi_line_best_size(const tpoint& config_size) const;

public:
protected:

	//! Returns the id of the state, which is also the index for the canvas.
	virtual unsigned get_state() const = 0;

	//! Does the widget need to restore the surface before (re)painting?
	virtual bool full_redraw() const;

	//! Sets the text variable for the canvases.
	virtual void set_canvas_text();

	tresolution_definition_* config() { return config_; }
	const tresolution_definition_* config() const { return config_; }

	void set_config(tresolution_definition_* config) { config_ = config; }

private:

	//! Visible state of the control, invisible isn't drawn.
	bool visible_;

	//! Can the label contain multiple lines.
	bool multiline_label_;

	//! If multiline we need to have a version of the label wrapped to fit
	//! in the widget. This value is cached so changes to the label or widget
	//! size should clear us.
	std::string wrapped_label_;

	//! The label associated with a lot of widgets to show a (non editable) text.
	t_string label_;

	//! When hovering a tooltip with extra information can show up. (FIXME implement)
	t_string tooltip_;

	//! When the user presses help a tooltip with even more info can be shown. (FIXME implement)
	t_string help_message_;

	//! Holds all canvas objects for a control. 
	std::vector<tcanvas> canvas_;

	//! Holds a copy of the original background which can be used before
	//! redrawing. This is needed for semi-tranparent items, the user
	//! defines whether it's required or not.
	surface restorer_;

	//! Saves the portion of the background.
	void save_background(const surface& src);

	//! Restores a portion of the background.
	void restore_background(surface& dst);

	//! Contains a pointer to the configuration of this button at the 
	//! current resolution.
 	tresolution_definition_* config_;

	//! Once the config is loaded this function is called.
	virtual void load_config_extra() {}

	//! The control_type parameter for tgui_definition::get_control()
	//! To keep the code more generic this type is required so the
	//! controls need to return the proper string here.
	//! Might be used at other parts as well the get the type of 
	//! control involved.
	virtual const std::string& get_control_type() const = 0;
};

} // namespace gui2

#endif

