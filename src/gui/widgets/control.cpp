/*
   Copyright (C) 2008 - 2014 by Mark de Wever <koraq@xs4all.nl>
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

#include "control.hpp"

#include "font.hpp"
#include "formula_string_utils.hpp"
#include "gui/auxiliary/iterator/walker_widget.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/auxiliary/event/message.hpp"
#include "gui/dialogs/tip.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "gui/auxiliary/window_builder/control.hpp"
#include "marked-up_text.hpp"
#include "utils/foreach.tpp"
#include "hotkey/hotkey_item.hpp"

#include <boost/bind.hpp>

#include <iomanip>

#define LOG_SCOPE_HEADER                                                       \
	"tcontrol(" + get_control_type() + ") [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

tcontrol::tcontrol(const unsigned canvas_count)
	: definition_("default")
	, label_()
	, use_markup_(false)
	, use_tooltip_on_label_overflow_(true)
	, tooltip_()
	, help_message_()
	, canvas_(canvas_count)
	, config_(NULL)
	, renderer_()
	, text_maximum_width_(0)
	, text_alignment_(PANGO_ALIGN_LEFT)
	, shrunken_(false)
{
	connect_signal<event::SHOW_TOOLTIP>(boost::bind(
			&tcontrol::signal_handler_show_tooltip, this, _2, _3, _5));

	connect_signal<event::SHOW_HELPTIP>(boost::bind(
			&tcontrol::signal_handler_show_helptip, this, _2, _3, _5));

	connect_signal<event::NOTIFY_REMOVE_TOOLTIP>(boost::bind(
			&tcontrol::signal_handler_notify_remove_tooltip, this, _2, _3));
}

tcontrol::tcontrol(const implementation::tbuilder_control& builder,
				   const unsigned canvas_count,
				   const std::string& control_type)
	: twidget(builder)
	, definition_(builder.definition)
	, label_(builder.label)
	, use_markup_(false)
	, use_tooltip_on_label_overflow_(builder.use_tooltip_on_label_overflow)
	, tooltip_(builder.tooltip)
	, help_message_(builder.help)
	, canvas_(canvas_count)
	, config_(NULL)
	, renderer_()
	, text_maximum_width_(0)
	, text_alignment_(PANGO_ALIGN_LEFT)
	, shrunken_(false)
{
	definition_load_configuration(control_type);

	connect_signal<event::SHOW_TOOLTIP>(boost::bind(
			&tcontrol::signal_handler_show_tooltip, this, _2, _3, _5));

	connect_signal<event::SHOW_HELPTIP>(boost::bind(
			&tcontrol::signal_handler_show_helptip, this, _2, _3, _5));

	connect_signal<event::NOTIFY_REMOVE_TOOLTIP>(boost::bind(
			&tcontrol::signal_handler_notify_remove_tooltip, this, _2, _3));
}

void tcontrol::set_members(const string_map& data)
{
	/** @todo document this feature on the wiki. */
	/** @todo do we need to add the debug colors here as well? */
	string_map::const_iterator itor = data.find("id");
	if(itor != data.end()) {
		set_id(itor->second);
	}

	itor = data.find("linked_group");
	if(itor != data.end()) {
		set_linked_group(itor->second);
	}

	itor = data.find("label");
	if(itor != data.end()) {
		set_label(itor->second);
	}

	itor = data.find("tooltip");
	if(itor != data.end()) {
		set_tooltip(itor->second);
	}

	itor = data.find("help");
	if(itor != data.end()) {
		set_help_message(itor->second);
	}

	itor = data.find("use_markup");
	if(itor != data.end()) {
		set_use_markup(utils::string_bool(itor->second));
	}
}

bool tcontrol::disable_click_dismiss() const
{
	return get_visible() == twidget::tvisible::visible && get_active();
}

iterator::twalker_* tcontrol::create_walker()
{
	return new iterator::walker::twidget(*this);
}

tpoint tcontrol::get_config_minimum_size() const
{
	assert(config_);

	tpoint result(config_->min_width, config_->min_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

tpoint tcontrol::get_config_default_size() const
{
	assert(config_);

	tpoint result(config_->default_width, config_->default_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

tpoint tcontrol::get_config_maximum_size() const
{
	assert(config_);

	tpoint result(config_->max_width, config_->max_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

unsigned tcontrol::get_characters_per_line() const
{
	return 0;
}

void tcontrol::layout_initialise(const bool full_initialisation)
{
	// Inherited.
	twidget::layout_initialise(full_initialisation);

	if(full_initialisation) {
		shrunken_ = false;
	}
}

void tcontrol::request_reduce_width(const unsigned maximum_width)
{
	assert(config_);

	if(!label_.empty() && can_wrap()) {

		tpoint size = get_best_text_size(
				tpoint(0, 0),
				tpoint(maximum_width - config_->text_extra_width, 0));

		size.x += config_->text_extra_width;
		size.y += config_->text_extra_height;

		set_layout_size(size);

		DBG_GUI_L << LOG_HEADER << " label '" << debug_truncate(label_)
				  << "' maximum_width " << maximum_width << " result " << size
				  << ".\n";

	} else {
		DBG_GUI_L << LOG_HEADER << " label '" << debug_truncate(label_)
				  << "' failed; either no label or wrapping not allowed.\n";
	}
}

tpoint tcontrol::calculate_best_size() const
{
	assert(config_);
	if(label_.empty()) {
		DBG_GUI_L << LOG_HEADER << " empty label return default.\n";
		return get_config_default_size();
	}

	const tpoint minimum = get_config_default_size();
	const tpoint maximum = get_config_maximum_size();

	/**
	 * @todo The value send should subtract the border size
	 * and read it after calculation to get the proper result.
	 */
	tpoint result = get_best_text_size(minimum, maximum);
	DBG_GUI_L << LOG_HEADER << " label '" << debug_truncate(label_)
			  << "' result " << result << ".\n";
	return result;
}

void tcontrol::place(const tpoint& origin, const tpoint& size)
{
	// resize canvasses
	FOREACH(AUTO & canvas, canvas_)
	{
		canvas.set_width(size.x);
		canvas.set_height(size.y);
	}

	// Note we assume that the best size has been queried but otherwise it
	// should return false.
	if(renderer_.is_truncated() && use_tooltip_on_label_overflow_
	   && tooltip_.empty()) {

		set_tooltip(label_);
	}

	// inherited
	twidget::place(origin, size);

	// update the state of the canvas after the sizes have been set.
	update_canvas();
}

void tcontrol::load_config()
{
	if(!config()) {

		definition_load_configuration(get_control_type());

		load_config_extra();
	}
}

twidget* tcontrol::find_at(const tpoint& coordinate, const bool must_be_active)
{
	return (twidget::find_at(coordinate, must_be_active)
			&& (!must_be_active || get_active()))
				   ? this
				   : NULL;
}

const twidget* tcontrol::find_at(const tpoint& coordinate,
								 const bool must_be_active) const
{
	return (twidget::find_at(coordinate, must_be_active)
			&& (!must_be_active || get_active()))
				   ? this
				   : NULL;
}

twidget* tcontrol::find(const std::string& id, const bool must_be_active)
{
	return (twidget::find(id, must_be_active)
			&& (!must_be_active || get_active()))
				   ? this
				   : NULL;
}

const twidget* tcontrol::find(const std::string& id, const bool must_be_active)
		const
{
	return (twidget::find(id, must_be_active)
			&& (!must_be_active || get_active()))
				   ? this
				   : NULL;
}

void tcontrol::set_definition(const std::string& definition)
{
	assert(!config());
	definition_ = definition;
	load_config();
	assert(config());

#ifdef GUI2_EXPERIMENTAL_LISTBOX
	init();
#endif
}

void tcontrol::set_label(const t_string& label)
{
	if(label == label_) {
		return;
	}

	label_ = label;
	set_layout_size(tpoint(0, 0));
	update_canvas();
	set_is_dirty(true);
}

void tcontrol::set_use_markup(bool use_markup)
{
	if(use_markup == use_markup_) {
		return;
	}

	use_markup_ = use_markup;
	update_canvas();
	set_is_dirty(true);
}

void tcontrol::set_text_alignment(const PangoAlignment text_alignment)
{
	if(text_alignment_ == text_alignment) {
		return;
	}

	text_alignment_ = text_alignment;
	update_canvas();
	set_is_dirty(true);
}

void tcontrol::update_canvas()
{
	const int max_width = get_text_maximum_width();
	const int max_height = get_text_maximum_height();

	// set label in canvases
	FOREACH(AUTO & canvas, canvas_)
	{
		canvas.set_variable("text", variant(label_));
		canvas.set_variable("text_markup", variant(use_markup_));
		canvas.set_variable("text_alignment",
							variant(encode_text_alignment(text_alignment_)));
		canvas.set_variable("text_maximum_width", variant(max_width));
		canvas.set_variable("text_maximum_height", variant(max_height));
		canvas.set_variable("text_wrap_mode",
							variant(can_wrap() ? PANGO_ELLIPSIZE_NONE
											   : PANGO_ELLIPSIZE_END));
		canvas.set_variable("text_characters_per_line",
							variant(get_characters_per_line()));
	}
}

int tcontrol::get_text_maximum_width() const
{
	assert(config_);

	return text_maximum_width_ != 0 ? text_maximum_width_
									: get_width() - config_->text_extra_width;
}

int tcontrol::get_text_maximum_height() const
{
	assert(config_);

	return get_height() - config_->text_extra_height;
}

void tcontrol::impl_draw_background(surface& frame_buffer)
{
	DBG_GUI_D << LOG_HEADER << " label '" << debug_truncate(label_) << "' size "
			  << get_rectangle() << ".\n";

	canvas(get_state()).blit(frame_buffer, get_rectangle());
}

void tcontrol::impl_draw_background(surface& frame_buffer,
									int x_offset,
									int y_offset)
{
	DBG_GUI_D << LOG_HEADER << " label '" << debug_truncate(label_) << "' size "
			  << get_rectangle() << ".\n";

	canvas(get_state()).blit(frame_buffer,
							 calculate_blitting_rectangle(x_offset, y_offset));
}

void tcontrol::impl_draw_foreground(surface& /*frame_buffer*/)
{
	/* DO NOTHING */
}

void tcontrol::impl_draw_foreground(surface& /*frame_buffer*/
									,
									int /*x_offset*/
									,
									int /*y_offset*/)
{
	/* DO NOTHING */
}

void tcontrol::definition_load_configuration(const std::string& control_type)
{
	assert(!config());

	set_config(get_control(control_type, definition_));

	assert(canvas().size() == config()->state.size());
	for(size_t i = 0; i < canvas().size(); ++i) {
		canvas(i) = config()->state[i].canvas;
	}

	update_canvas();
}

tpoint tcontrol::get_best_text_size(const tpoint& minimum_size,
									const tpoint& maximum_size) const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	assert(!label_.empty());

	const tpoint border(config_->text_extra_width, config_->text_extra_height);
	tpoint size = minimum_size - border;

	renderer_.set_text(label_, use_markup_);

	renderer_.set_font_size(config_->text_font_size);
	renderer_.set_font_style(config_->text_font_style);
	renderer_.set_alignment(text_alignment_);

	// Try with the minimum wanted size.
	const int maximum_width = text_maximum_width_ != 0 ? text_maximum_width_
													   : maximum_size.x;

	renderer_.set_maximum_width(maximum_width);

	if(can_wrap()) {
		renderer_.set_ellipse_mode(PANGO_ELLIPSIZE_NONE);
	}

	renderer_.set_characters_per_line(get_characters_per_line());
	if(get_characters_per_line() != 0 && !can_wrap()) {
		WRN_GUI_L
		<< LOG_HEADER << " Limited the number of characters per line, "
		<< "but wrapping is not set, output may not be as expected.\n";
	}

	DBG_GUI_L << LOG_HEADER << " label '" << debug_truncate(label_)
			  << "' status: "
			  << " minimum_size " << minimum_size << " maximum_size "
			  << maximum_size << " text_maximum_width_ " << text_maximum_width_
			  << " can_wrap " << can_wrap() << " characters_per_line "
			  << get_characters_per_line() << " truncated "
			  << renderer_.is_truncated() << " renderer size "
			  << renderer_.get_size() << ".\n";

	// If doesn't fit try the maximum.
	if(renderer_.is_truncated() && !can_wrap()) {
		// FIXME if maximum size is defined we should look at that
		// but also we don't adjust for the extra text space yet!!!
		const tpoint maximum_size(config_->max_width, config_->max_height);
		renderer_.set_maximum_width(maximum_size.x ? maximum_size.x - border.x
												   : -1);
	}

	size = renderer_.get_size() + border;

	if(size.x < minimum_size.x) {
		size.x = minimum_size.x;
	}

	if(size.y < minimum_size.y) {
		size.y = minimum_size.y;
	}

	DBG_GUI_L << LOG_HEADER << " label '" << debug_truncate(label_)
			  << "' result " << size << ".\n";
	return size;
}

void tcontrol::signal_handler_show_tooltip(const event::tevent event,
										   bool& handled,
										   const tpoint& location)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(!tooltip_.empty()) {
		std::string tip = tooltip_;
		if(!help_message_.empty()) {
			utils::string_map symbols;
			symbols["hotkey"] = hotkey::get_names(
					hotkey::hotkey_command::get_command_by_command(
							hotkey::GLOBAL__HELPTIP).command);

			tip = tooltip_ + utils::interpolate_variables_into_string(
									 settings::has_helptip_message, &symbols);
		}

		event::tmessage_show_tooltip message(tip, location);
		handled = fire(event::MESSAGE_SHOW_TOOLTIP, *this, message);
	}
}

void tcontrol::signal_handler_show_helptip(const event::tevent event,
										   bool& handled,
										   const tpoint& location)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(!help_message_.empty()) {
		event::tmessage_show_helptip message(help_message_, location);
		handled = fire(event::MESSAGE_SHOW_HELPTIP, *this, message);
	}
}

void tcontrol::signal_handler_notify_remove_tooltip(const event::tevent event,
													bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	/*
	 * This makes the class know the tip code rather intimately. An
	 * alternative is to add a message to the window to remove the tip.
	 * Might be done later.
	 */
	tip::remove();

	handled = true;
}

} // namespace gui2
