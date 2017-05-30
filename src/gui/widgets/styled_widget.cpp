/*
   Copyright (C) 2008 - 2018 by Mark de Wever <koraq@xs4all.nl>
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

#include "gui/widgets/styled_widget.hpp"

#include "formatter.hpp"
#include "formula/string_utils.hpp"
#include "gettext.hpp"
#include "gui/auxiliary/iterator/walker_widget.hpp"
#include "gui/core/event/message.hpp"
#include "gui/core/gui_definition.hpp"
#include "gui/core/log.hpp"
#include "gui/dialogs/tooltip.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/window.hpp"
#include "hotkey/hotkey_item.hpp"
#include "sdl/rect.hpp"
#include "wml_exception.hpp"

#include "utils/functional.hpp"

#include <algorithm>
#include <iomanip>

#define LOG_SCOPE_HEADER                                                       \
	"styled_widget(" + get_control_type() + ") [" + id() + "] " + __func__
#define LOG_HEADER LOG_SCOPE_HEADER + ':'

namespace gui2
{

// ------------ WIDGET -----------{

styled_widget::styled_widget(const implementation::builder_styled_widget& builder,
				   const std::string& control_type)
	: widget(builder)
	, definition_(builder.definition)
	, label_(builder.label_string)
	, use_markup_(builder.use_markup)
	, use_tooltip_on_label_overflow_(builder.use_tooltip_on_label_overflow)
	, tooltip_(builder.tooltip)
	, help_message_(builder.help)
	, config_(get_control(control_type, definition_))
	, canvases_(config_->state.size()) // One canvas per state
	, renderer_()
	, text_maximum_width_(0)
	, text_alignment_(PANGO_ALIGN_LEFT)
	, text_ellipse_mode_(PANGO_ELLIPSIZE_END)
	, shrunken_(false)
{
	/*
	 * Fill in each canvas from the widget state definitons.
	 *
	 * Most widgets have a single canvas. However, some widgets such as toggle_panel
	 * and toggle_button have a variable canvas count determined by their definitions.
	 */
	for(unsigned i = 0; i < config_->state.size(); ++i) {
		canvases_[i].set_cfg(config_->state[i].canvas_cfg_);
	}

	// Initialize all the canvas variables.
	update_canvas();

	// Enable hover behavior if a tooltip was provided.
	set_wants_mouse_hover(!tooltip_.empty());

	connect_signal<event::SHOW_TOOLTIP>(std::bind(
			&styled_widget::signal_handler_show_tooltip, this, _2, _3, _5));

	connect_signal<event::SHOW_HELPTIP>(std::bind(
			&styled_widget::signal_handler_show_helptip, this, _2, _3, _5));

	connect_signal<event::NOTIFY_REMOVE_TOOLTIP>(std::bind(
			&styled_widget::signal_handler_notify_remove_tooltip, this, _2, _3));
}

void styled_widget::set_members(const string_map& data)
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

	itor = data.find("text_alignment");
	if(itor != data.end()) {
		set_text_alignment(decode_text_alignment(itor->second));
	}
}

bool styled_widget::disable_click_dismiss() const
{
	return get_visible() == widget::visibility::visible && get_active();
}

iteration::walker_base* styled_widget::create_walker()
{
	return new iteration::walker::widget(*this);
}

point styled_widget::get_config_minimum_size() const
{
	assert(config_);

	point result(config_->min_width, config_->min_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

point styled_widget::get_config_default_size() const
{
	assert(config_);

	point result(config_->default_width, config_->default_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

point styled_widget::get_config_maximum_size() const
{
	assert(config_);

	point result(config_->max_width, config_->max_height);

	DBG_GUI_L << LOG_HEADER << " result " << result << ".\n";
	return result;
}

unsigned styled_widget::get_characters_per_line() const
{
	return 0;
}

bool styled_widget::get_link_aware() const
{
	return false;
}

color_t styled_widget::get_link_color() const
{
	return color_t::from_hex_string("ffff00");
}

void styled_widget::layout_initialize(const bool full_initialization)
{
	// Inherited.
	widget::layout_initialize(full_initialization);

	if(full_initialization) {
		shrunken_ = false;
	}
}

void styled_widget::request_reduce_width(const unsigned maximum_width)
{
	assert(config_);

	if(!label_.empty() && can_wrap()) {

		point size = get_best_text_size(
				point(), point(maximum_width - config_->text_extra_width, 0));

		size.x += config_->text_extra_width;
		size.y += config_->text_extra_height;

		set_layout_size(size);

		DBG_GUI_L << LOG_HEADER << " label '" << debug_truncate(label_)
				  << "' maximum_width " << maximum_width << " result " << size
				  << ".\n";

	} else if(label_.empty() || text_can_shrink()) {
		point size = get_best_size();
		point min_size = get_config_minimum_size();
		size.x = std::min(size.x, std::max<int>(maximum_width, min_size.x));
		set_layout_size(size);

		DBG_GUI_L << LOG_HEADER << " styled_widget " << id()
		          << " maximum_width " << maximum_width << " result " << size
		          << ".\n";
	} else {
		DBG_GUI_L << LOG_HEADER << " label '" << debug_truncate(label_)
				  << "' failed; either no label or wrapping not allowed.\n";
	}
}

void styled_widget::request_reduce_height(const unsigned maximum_height)
{
	if(!label_.empty()) {
		// Do nothing
	} else {
		point size = get_best_size();
		point min_size = get_config_minimum_size();
		size.y = std::min(size.y, std::max<int>(maximum_height, min_size.y));
		set_layout_size(size);

		DBG_GUI_L << LOG_HEADER << " styled_widget " << id()
		          << " maximum_height " << maximum_height << " result " << size
		          << ".\n";
	}
}

point styled_widget::calculate_best_size() const
{
	assert(config_);
	if(label_.empty()) {
		DBG_GUI_L << LOG_HEADER << " empty label return default.\n";
		return get_config_default_size();
	}

	const point minimum = get_config_default_size();
	const point maximum = get_config_maximum_size();

	/**
	 * @todo The value send should subtract the border size
	 * and read it after calculation to get the proper result.
	 */
	point result = get_best_text_size(minimum, maximum);
	DBG_GUI_L << LOG_HEADER << " label '" << debug_truncate(label_)
			  << "' result " << result << ".\n";
	return result;
}

void styled_widget::place(const point& origin, const point& size)
{
	// resize canvasses
	for(auto & canvas : canvases_)
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
	widget::place(origin, size);

	// update the state of the canvas after the sizes have been set.
	update_canvas();
}

widget* styled_widget::find_at(const point& coordinate, const bool must_be_active)
{
	return (widget::find_at(coordinate, must_be_active)
			&& (!must_be_active || get_active()))
				   ? this
				   : nullptr;
}

const widget* styled_widget::find_at(const point& coordinate,
								 const bool must_be_active) const
{
	return (widget::find_at(coordinate, must_be_active)
			&& (!must_be_active || get_active()))
				   ? this
				   : nullptr;
}

widget* styled_widget::find(const std::string& id, const bool must_be_active)
{
	return (widget::find(id, must_be_active)
			&& (!must_be_active || get_active()))
				   ? this
				   : nullptr;
}

const widget* styled_widget::find(const std::string& id, const bool must_be_active)
		const
{
	return (widget::find(id, must_be_active)
			&& (!must_be_active || get_active()))
				   ? this
				   : nullptr;
}

void styled_widget::set_label(const t_string& label)
{
	if(label == label_) {
		return;
	}

	label_ = label;
	set_layout_size(point());
	update_canvas();
}

void styled_widget::set_use_markup(bool use_markup)
{
	if(use_markup == use_markup_) {
		return;
	}

	use_markup_ = use_markup;
	update_canvas();
}

void styled_widget::set_text_alignment(const PangoAlignment text_alignment)
{
	if(text_alignment_ == text_alignment) {
		return;
	}

	text_alignment_ = text_alignment;
	update_canvas();
}

void styled_widget::set_text_ellipse_mode(const PangoEllipsizeMode ellipse_mode)
{
	if(text_ellipse_mode_ == ellipse_mode) {
		return;
	}

	text_ellipse_mode_ = ellipse_mode;
	update_canvas();
}

void styled_widget::update_canvas()
{
	const int max_width = get_text_maximum_width();
	const int max_height = get_text_maximum_height();

	// set label in canvases
	for(auto & canvas : canvases_)
	{
		canvas.set_variable("text", wfl::variant(label_));
		canvas.set_variable("text_markup", wfl::variant(use_markup_));
		canvas.set_variable("text_link_aware", wfl::variant(get_link_aware()));

		// Possible TODO: consider making a formula_callable for colors.
		color_t link_color = get_link_color();
		std::vector<wfl::variant> link_color_as_list {
			wfl::variant(link_color.r),
			wfl::variant(link_color.g),
			wfl::variant(link_color.b),
			wfl::variant(link_color.a)
		};

		canvas.set_variable("text_link_color", wfl::variant(link_color_as_list));
		canvas.set_variable("text_alignment",
							wfl::variant(encode_text_alignment(text_alignment_)));
		canvas.set_variable("text_maximum_width", wfl::variant(max_width));
		canvas.set_variable("text_maximum_height", wfl::variant(max_height));
		canvas.set_variable("text_wrap_mode", wfl::variant(get_text_ellipse_mode()));
		canvas.set_variable("text_characters_per_line",
							wfl::variant(get_characters_per_line()));
	}
}

int styled_widget::get_text_maximum_width() const
{
	assert(config_);

	return text_maximum_width_ != 0 ? text_maximum_width_
									: get_width() - config_->text_extra_width;
}

int styled_widget::get_text_maximum_height() const
{
	assert(config_);

	return get_height() - config_->text_extra_height;
}

void styled_widget::impl_draw_background(surface& frame_buffer,
									int x_offset,
									int y_offset)
{
	DBG_GUI_D << LOG_HEADER << " label '" << debug_truncate(label_) << "' size "
			  << get_rectangle() << ".\n";

	get_canvas(get_state()).blit(frame_buffer,
							 calculate_blitting_rectangle(x_offset, y_offset));
}

void styled_widget::impl_draw_foreground(surface& /*frame_buffer*/
									,
									int /*x_offset*/
									,
									int /*y_offset*/)
{
	/* DO NOTHING */
}

point styled_widget::get_best_text_size(point minimum_size, point maximum_size) const
{
	log_scope2(log_gui_layout, LOG_SCOPE_HEADER);

	assert(!label_.empty());

	// Try with the minimum wanted size.
	const int maximum_width = text_maximum_width_ != 0
		? text_maximum_width_
		: maximum_size.x;

	/*
	 * NOTE: text rendering does *not* happen here. That happens in the text_shape
	 * canvas class. Instead, this just leverages the pango text rendering engine to
	 * calculate the area this widget will need to successfully render its text later.
	 */
	renderer_
		.set_link_aware(get_link_aware())
		.set_link_color(get_link_color())
		.set_family_class(config_->text_font_family)
		.set_font_size(config_->text_font_size)
		.set_font_style(config_->text_font_style)
		.set_alignment(text_alignment_)
		.set_maximum_width(maximum_width)
		.set_ellipse_mode(get_text_ellipse_mode())
		.set_characters_per_line(get_characters_per_line())
		.set_text(label_, use_markup_);

	if(get_characters_per_line() != 0 && !can_wrap()) {
		WRN_GUI_L << LOG_HEADER
			<< " Limited the number of characters per line, "
			<< "but wrapping is not set, output may not be as expected.\n";
	}

	DBG_GUI_L << LOG_HEADER << "\n"
		<< std::boolalpha
		<< "Label: '" << debug_truncate(label_) << "'\n\n"
		<< "Status:\n"
		<< "minimum_size: " << minimum_size << "\n"
		<< "maximum_size: " << maximum_size << "\n"
		<< "text_maximum_width_: " << text_maximum_width_ << "\n"
		<< "can_wrap: " << can_wrap() << "\n"
		<< "characters_per_line: " << get_characters_per_line() << "\n"
		<< "truncated: " << renderer_.is_truncated() << "\n"
		<< "renderer size: " << renderer_.get_size() << "\n\n"
		<< std::noboolalpha;

	const point border(config_->text_extra_width, config_->text_extra_height);

	// If doesn't fit try the maximum.
	if(renderer_.is_truncated() && !can_wrap()) {
		// FIXME if maximum size is defined we should look at that
		// but also we don't adjust for the extra text space yet!!!
		maximum_size = point(config_->max_width, config_->max_height);

		renderer_.set_maximum_width(maximum_size.x ? maximum_size.x - border.x : -1);
	}

	// Get the resulting size.
	point size = renderer_.get_size() + border;

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

void styled_widget::signal_handler_show_tooltip(const event::ui_event event,
										   bool& handled,
										   const point& location)
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

		event::message_show_tooltip message(tip, location, get_rectangle());
		handled = fire(event::MESSAGE_SHOW_TOOLTIP, *this, message);
	}
}

void styled_widget::signal_handler_show_helptip(const event::ui_event event,
										   bool& handled,
										   const point& location)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	if(!help_message_.empty()) {
		event::message_show_helptip message(help_message_, location, get_rectangle());
		handled = fire(event::MESSAGE_SHOW_HELPTIP, *this, message);
	}
}

void styled_widget::signal_handler_notify_remove_tooltip(const event::ui_event event,
													bool& handled)
{
	DBG_GUI_E << LOG_HEADER << ' ' << event << ".\n";

	/*
	 * This makes the class know the tip code rather intimately. An
	 * alternative is to add a message to the window to remove the tip.
	 * Might be done later.
	 */
	dialogs::tip::remove();

	handled = true;
}

std::string styled_widget::get_label_token(const point & position, const char * delim) const
{
	return renderer_.get_token(position, delim);
}

std::string styled_widget::get_label_link(const point & position) const
{
	return renderer_.get_link(position);
}

// }---------- BUILDER -----------{

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 1_widget
 *
 * = Widget =
 * @begin{parent}{name="generic/"}
 * @begin{tag}{name="widget_instance"}{min="0"}{max="-1"}
 * All widgets placed in the cell have some values in common:
 * @begin{table}{config}
 *     id & string & "" &              This value is used for the engine to
 *                                     identify 'special' items. This means that
 *                                     for example a text_box can get the proper
 *                                     initial value. This value should be
 *                                     unique or empty. Those special values are
 *                                     documented at the window definition that
 *                                     uses them. NOTE items starting with an
 *                                     underscore are used for composed widgets
 *                                     and these should be unique per composed
 *                                     widget. $
 *
 *     definition & string & "default" &
 *                                     The id of the widget definition to use.
 *                                     This way it's possible to select a
 *                                     specific version of the widget e.g. a
 *                                     title label when the label is used as
 *                                     title. $
 *
 *     linked_group & string & "" &    The linked group the styled_widget belongs
 *                                     to. $
 *
 *     label & t_string & "" &          Most widgets have some text associated
 *                                     with them, this field contain the value
 *                                     of that text. Some widgets use this value
 *                                     for other purposes, this is documented
 *                                     at the widget. E.g. an image uses the
 *                                     filename in this field. $
 *
 *     tooltip & t_string & "" &        If you hover over a widget a while (the
 *                                     time it takes can differ per widget) a
 *                                     short help can show up.This defines the
 *                                     text of that message. This field may not
 *                                     be empty when 'help' is set. $
 *
 *     help & t_string & "" &           If you hover over a widget and press F10
 *                                     (or the key the user defined for the help
 *                                     tip) a help message can show up. This
 *                                     help message might be the same as the
 *                                     tooltip but in general (if used) this
 *                                     message should show more help. This
 *                                     defines the text of that message. $
 *
 *    use_markup & bool & false &      Whether to format the text using Pango
 *                                     markup. Applies to Labels and
 *                                     other Widgets with text. $
 *
 *    use_tooltip_on_label_overflow & bool & true &
 *                                     If the text on the label is truncated and
 *                                     the tooltip is empty the label can be
 *                                     used for the tooltip. If this variable is
 *                                     set to true this will happen. $
 *
 *   debug_border_mode & unsigned & 0 &
 *                                     The mode for showing the debug border.
 *                                     This border shows the area reserved for
 *                                     a widget. This function is only meant
 *                                     for debugging and might not be
 *                                     available in all Wesnoth binaries.
 *                                     Available modes:
 *                                     @* 0 no border.
 *                                     @* 1 1 pixel border.
 *                                     @* 2 floodfill the widget area. $
 *
 *   debug_border_color & color & "" & The color of the debug border. $
 * @end{table}
 * @end{tag}{name="widget_instance"}
 * @end{parent}{name="generic/"}
 */

namespace implementation
{

builder_styled_widget::builder_styled_widget(const config& cfg)
	: builder_widget(cfg)
	, definition(cfg["definition"])
	, label_string(cfg["label"].t_str())
	, tooltip(cfg["tooltip"].t_str())
	, help(cfg["help"].t_str())
	, use_tooltip_on_label_overflow(true)
	, use_markup(cfg["use_markup"].to_bool(false))
{
	if(definition.empty()) {
		definition = "default";
	}

	VALIDATE_WITH_DEV_MESSAGE(
			help.empty() || !tooltip.empty(),
			_("Found a widget with a helptip and without a tooltip."),
			formatter() << "id '" << id << "' label '" << label_string
						 << "' helptip '" << help << "'.");


	DBG_GUI_P << "Window builder: found styled_widget with id '" << id
			  << "' and definition '" << definition << "'.\n";
}

widget* builder_styled_widget::build(const replacements_map& /*replacements*/) const
{
	return build();
}

} // namespace implementation

// }------------ END --------------

} // namespace gui2
