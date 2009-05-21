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

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "control.hpp"

#include "font.hpp"
#include "foreach.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/window.hpp"
#include "marked-up_text.hpp"

#include <iomanip>

namespace gui2 {

tcontrol::tcontrol(const unsigned canvas_count)
	: label_()
	, markup_mode_(NO_MARKUP)
	, use_tooltip_on_label_overflow_(true)
	, tooltip_()
	, help_message_()
	, canvas_(canvas_count)
	, config_(0)
	, renderer_()
	, text_maximum_width_(0)
	, shrunken_(false)
{
}

void tcontrol::set_members(const string_map& data)
{
	string_map::const_iterator itor = data.find("label");
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
}

void tcontrol::set_block_easy_close(const bool block)
{
	twindow* window = get_window();
	if(!window) {
		/*
		 * This can happen in a listbox when the row data is manipulated before
		 * the listbox is finalized. In that case that widget should do set the
		 * state in its finalizer.
		 */
		DBG_GUI_G << "tcontrol(" + get_control_type() + ") " + __func__ + ": "
			"No window set, this might be a bug.\n";
		return;
	}

	if(block) {
		if(id().empty()) {
			set_id(get_uid());
		}
		window->add_easy_close_blocker(id());
	} else if(!id().empty()) {
		// It might never have been enabled so the id might be empty.
		window->remove_easy_close_blocker(id());
	}
}

tpoint tcontrol::get_config_minimum_size() const
{
	assert(config_);

	tpoint result(config_->min_width, config_->min_height);

	DBG_GUI_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
		<< " result " << result
		<< ".\n";
	return result;
}

tpoint tcontrol::get_config_default_size() const
{
	assert(config_);

	tpoint result(config_->default_width, config_->default_height);

	DBG_GUI_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
		<< " result " << result
		<< ".\n";
	return result;
}

tpoint tcontrol::get_config_maximum_size() const
{
	assert(config_);

	tpoint result(config_->max_width, config_->max_height);

	DBG_GUI_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
		<< " result " << result
		<< ".\n";
	return result;
}

void tcontrol::NEW_layout_init(const bool full_initialization)
{
	// Inherited.
	twidget::NEW_layout_init(full_initialization);

	if(full_initialization) {
		shrunken_ = false;
	}
}

void tcontrol::NEW_request_reduce_width(const unsigned maximum_width)
{
	// Inherited.
	twidget::NEW_request_reduce_width(maximum_width);

	assert(config_);

	if(!label_.empty() && can_wrap()) {

		tpoint size = get_best_text_size(
				tpoint(0,0),
				tpoint(maximum_width - config_->text_extra_width, 0));

		size.x += config_->text_extra_width;
		size.y += config_->text_extra_height;

		set_layout_size(size);

		DBG_GUI_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
				<< " maximum_width " << maximum_width
				<< " result " << size
				<< ".\n";

	} else {
		DBG_GUI_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
				<< " failed; either no label or wrapping not allowed.\n";
	}
}

void tcontrol::layout_wrap(const unsigned maximum_width)
{
	// Inherited.
	twidget::layout_wrap(maximum_width);

	assert(config_);

	if(label_.empty()) {
		// FIXME see what to do on an empty label later.
		return;
	} else {

		tpoint size = get_best_text_size(
				tpoint(0,0),
				tpoint(maximum_width - config_->text_extra_width, 0));

		size.x += config_->text_extra_width;
		size.y += config_->text_extra_height;

		set_layout_size(size);

		DBG_GUI_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
				<< " maximum_width " << maximum_width
				<< " result " << size
				<< ".\n";

	}
}

void tcontrol::layout_shrink_width(const unsigned maximum_width)
{
	DBG_GUI_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
			<< " maximum_width " << maximum_width
			<< ".\n";

	/** @todo handle the tooltip properly. */

	shrunken_ = true;
	set_layout_size(tpoint(maximum_width, get_best_size().y));

	assert(static_cast<unsigned>(get_best_size().x) == maximum_width);
}

void tcontrol::layout_fit_width(const unsigned maximum_width,
		const tfit_flags flags)
{
	assert(get_visible() != twidget::INVISIBLE);

	log_scope2(log_gui_layout,
			"tcontrol(" + get_control_type() + ") " + __func__);
	DBG_GUI_L << "maximum_width " << maximum_width
			<< " flags " << flags
			<< ".\n";

	// Already fits.
	if(get_best_size().x <= static_cast<int>(maximum_width)) {
		DBG_GUI_L << "Already fits.\n";
		return;
	}

	// Wrap.
	if((flags & twidget::WRAP) && can_wrap()) {
		layout_wrap(maximum_width);

		if(get_best_size().x <= static_cast<int>(maximum_width)) {
			DBG_GUI_L << "Success: Wrapped.\n";
			return;
		}
	}

	// Horizontal scrollbar.
	if((flags & twidget::SCROLLBAR) && has_horizontal_scrollbar()) {
		layout_use_horizontal_scrollbar(maximum_width);

		if(get_best_size().x <= static_cast<int>(maximum_width)) {
			DBG_GUI_L << "Success: Horizontal scrollbar.\n";
			return;
		}
	}

	// Shrink.
	if((flags & twidget::SHRINK) && can_shrink_width()) {
		layout_shrink_width(maximum_width);
		DBG_GUI_L << "Success: Shrunken.\n";
	}

	DBG_GUI_L << "Failed.\n";
}

tpoint tcontrol::calculate_best_size() const
{
	assert(config_);
	tpoint result(config_->default_width, config_->default_height);
	if(! label_.empty()) {
		// If no label text set we use the predefined value.

		/**
		 * @todo The value send should subtract the border size
		 * and readd it after calculation to get the proper result.
		 */
		result = get_best_text_size(result);
	}

	DBG_GUI_L << "tcontrol(" + get_control_type() + ") " + __func__ + ":"
		<< " empty label " << label_.empty()
		<< " result " << result
		<< ".\n";
	return result;
}

void tcontrol::set_size(const tpoint& origin, const tpoint& size)
{
	// resize canvasses
	foreach(tcanvas& canvas, canvas_) {
		canvas.set_width(size.x);
		canvas.set_height(size.y);
	}

	// Note we assume that the best size has been queried but otherwise it
	// should return false.
	if(renderer_.is_truncated()
			&& use_tooltip_on_label_overflow_ && tooltip_.empty()) {

		 set_tooltip(label_);
	}

	// inherited
	twidget::set_size(origin, size);

	// update the state of the canvas after the sizes have been set.
	update_canvas();
}

void tcontrol::mouse_hover(tevent_handler& event)
{
	DBG_GUI_E << "Control: mouse hover.\n";
	event.show_tooltip(tooltip_, settings::popup_show_time);
}

void tcontrol::help_key(tevent_handler& event)
{
	DBG_GUI_E << "Control: help key.\n";
	event.show_help_popup(help_message_, settings::help_show_time);
}

void tcontrol::load_config()
{
	if(!config()) {
		set_config(get_control(get_control_type(), definition()));

		assert(canvas().size() == config()->state.size());
		for(size_t i = 0; i < canvas().size(); ++i) {
			canvas(i) = config()->state[i].canvas;
		}

		update_canvas();

		load_config_extra();
	}
}

void tcontrol::set_definition(const std::string& definition)
{
	assert(!config());
	twidget::set_definition(definition);
	load_config();
	assert(config());
}

void tcontrol::set_label(const t_string& label)
{
	if(label == label_) {
		return;
	}

	label_ = label;
	update_canvas();
	set_dirty();
}

void tcontrol::set_markup_mode(const tmarkup_mode markup_mode)
{
	if(markup_mode == markup_mode_) {
		return;
	}

	markup_mode_ = markup_mode;
	update_canvas();
	set_dirty();
}

void tcontrol::update_canvas()
{
	const int max_width = get_text_maximum_width();
	const int max_height = get_text_maximum_height();

	// set label in canvases
	foreach(tcanvas& canvas, canvas_) {
		switch(markup_mode_) {
			case NO_MARKUP :
				canvas.set_variable("text", variant(label_));
				canvas.set_variable("text_markup", variant(false));
				break;
			case PANGO_MARKUP :
				canvas.set_variable("text", variant(label_));
				canvas.set_variable("text_markup", variant(true));
				break;
			case WML_MARKUP :
				canvas.set_variable("text", variant(get_pango_markup()));
				canvas.set_variable("text_markup", variant(true));
				break;
			default:
				assert(false);
		}
		canvas.set_variable("text_maximum_width", variant(max_width));
		canvas.set_variable("text_maximum_height", variant(max_height));
		canvas.set_variable("text_wrap_mode", variant(can_wrap()
			? PANGO_ELLIPSIZE_NONE : PANGO_ELLIPSIZE_END));
	}
}

int tcontrol::get_text_maximum_width() const
{
	assert(config_);

	return text_maximum_width_ != 0
		? text_maximum_width_
		: get_width() - config_->text_extra_width;
}

int tcontrol::get_text_maximum_height() const
{
	assert(config_);

	return get_height() - config_->text_extra_height;
}

void tcontrol::impl_draw_background(surface& frame_buffer)
{
#if 0
	std::cerr << "tcontrol(" + get_control_type() + ") " + __func__ + ": "
		<< " id " << id()
		<< " dirty " << get_dirty()
		<< " visible " << is_visible()
		<< ".\n";
#endif
	if(!is_visible()) {
		return;
	}

	canvas(get_state()).draw();
	SDL_Rect rect = get_rect();
	SDL_BlitSurface(canvas(get_state()).surf(), NULL, frame_buffer, &rect);
}

tpoint tcontrol::get_best_text_size(const tpoint& minimum_size, const tpoint& maximum_size) const
{
	log_scope2(log_gui_layout, "tcontrol(" + get_control_type() + ") " + __func__);

	assert(!label_.empty());

	const tpoint border(config_->text_extra_width, config_->text_extra_height);
	tpoint size = minimum_size - border;

	switch(markup_mode_) {
		case NO_MARKUP :
			renderer_.set_text(label_, false);
			break;
		case PANGO_MARKUP :
			renderer_.set_text(label_, true);
			break;
		case WML_MARKUP :
			renderer_.set_text(get_pango_markup(), true);
			break;
		default:
			assert(false);
	}

	renderer_.set_font_size(config_->text_font_size);
	renderer_.set_font_style(config_->text_font_style);

	// Try with the minimum wanted size.
	const int maximum_width =  text_maximum_width_ != 0
		? text_maximum_width_
		: maximum_size.x;

	renderer_.set_maximum_width(maximum_width);

	if(can_wrap()) {
		renderer_.set_ellipse_mode(PANGO_ELLIPSIZE_NONE);
	}

	DBG_GUI_L << "tcontrol(" + get_control_type() + ") status:\n";
	DBG_GUI_L << "minimum_size " << minimum_size
		<< " maximum_size " << maximum_size
		<< " text_maximum_width_ " << text_maximum_width_
		<< " can_wrap " << can_wrap()
		<< " truncated " << renderer_.is_truncated()
		<< " renderer size " << renderer_.get_size()
		<< ".\n";

	// If doesn't fit try the maximum.
	if(renderer_.is_truncated() && !can_wrap()) {
		// FIXME if maximum size is defined we should look at that
		// but also we don't adjust for the extra text space yet!!!
		const tpoint maximum_size(config_->max_width, config_->max_height);
		renderer_.set_maximum_width(maximum_size.x ? maximum_size.x - border.x : -1);
	}

	size = renderer_.get_size() + border;

	if(size.x < minimum_size.x) {
		size.x = minimum_size.x;
	}

	if(size.y < minimum_size.y) {
		size.y = minimum_size.y;
	}

	DBG_GUI_L << "tcontrol(" + get_control_type() + ") result " << size << ".\n";
	return size;
}

namespace {

/** Converts a SDL_Color to a pango colour prefix. */
std::string colour_prefix(const SDL_Color& colour)
{
	std::stringstream result;

	// Cast to unsigned is needed to avoid being interpreted as a char.
	result << "<span foreground=\"#"
			<< std::hex
			<< std::setfill('0') << std::setw(2)
			<< static_cast<unsigned>(colour.r)
			<< std::setfill('0') << std::setw(2)
			<< static_cast<unsigned>(colour.g)
			<< std::setfill('0') << std::setw(2)
			<< static_cast<unsigned>(colour.b)
			<< "\">";

	return result.str();
}

/**
 * Escapes a string to be used in a pango formatted string.
 *
 * This function also changes every "\x" to "x" thus also "\\" to "\". This
 * is used to mimic the old dialog behaviour.
 */
std::string escape_string(std::string str)
{
	struct tconverter
	{
		tconverter(const std::string& string)
			: str(g_markup_escape_text(string.c_str(), -1))
		{
		}

		~tconverter()
		{
			g_free(str);
		}

		gchar* str;
	};

	size_t offset = str.find('\\', 0);
	while (offset != std::string::npos) {
		str.erase(offset, 1);
		++offset;
		offset = str.find('\\', offset);
	}

	tconverter converter(str);
	return std::string(converter.str);
}

} // namespace

std::string tcontrol::get_pango_markup() const
{
	std::vector<std::string> lines = utils::split(label_, '\n', 0);

	foreach(std::string& line, lines) {
		if(line.empty()) {
			continue;
		}

		std::string pre = "";
		std::string post = "";
		bool proceed = true;

		while(!line.empty() && proceed) {
			const char c = line[0];
			// The font 'constants' aren't seen as const so it's not possible
			// to use a switch statement.
			if(c == font::BAD_TEXT) {
				pre += colour_prefix(font::BAD_COLOUR);
				post = "</span>" + post;
				line.erase(0, 1);
			} else if(c == font::GOOD_TEXT) {
				pre += colour_prefix(font::GOOD_COLOUR);
				post = "</span>" + post;
				line.erase(0, 1);
			} else if(c == font::NORMAL_TEXT) {
				pre += colour_prefix(font::NORMAL_COLOUR);
				post = "</span>" + post;
				line.erase(0, 1);
			} else if(c == font::BLACK_TEXT) {
				pre += colour_prefix(font::BLACK_COLOUR);
				post = "</span>" + post;
				line.erase(0, 1);
			} else if(c == font::GRAY_TEXT) {
				pre += colour_prefix(font::GRAY_COLOUR);
				post = "</span>" + post;
				line.erase(0, 1);
			} else if(c == font::LARGE_TEXT) {
				pre += "<big>";
				post = "</big>" + post;
				line.erase(0, 1);
			} else if(c == font::SMALL_TEXT) {
				pre += "<small>";
				post = "</small>" + post;
				line.erase(0, 1);
			} else if(c == font::BOLD_TEXT) {
				pre += "<b>";
				post = "</b>" + post;
				line.erase(0, 1);

			} else if(c == font::COLOR_TEXT) {
				/*
				 * Crude parsing of <1,23,123>.
				 *
				 * The old engine didn't specify the error behaviour so we
				 * define our own.
				 *
				 * If no > found the entire line is discarded.
				 * If not three colours are found the colour part is ignored.
				 * Invalid colour channels are seen as 0.
				 */

				// Find the closing >.
				const size_t pos = line.find('>');
				if(pos == std::string::npos) {
					line.clear();
					continue;
				}

				std::vector<std::string> rgb = utils::split(
						line.substr(1, pos - 1));

				// Remove the colour part of the string it's no longer needed
				// and the next test might fail so we need to clean the
				// string here.
				line.erase(0, pos + 1);

				if(rgb.size() != 3) {
					continue;
				}

				SDL_Color colour;
#if 0
				/** @todo Test why this doesn't work. */
				colour.r = lexical_cast_default<Uint8>(rgb[0]);
				colour.g = lexical_cast_default<Uint8>(rgb[1]);
				colour.b = lexical_cast_default<Uint8>(rgb[2]);
#else
				colour.r = atoi(rgb[0].c_str());
				colour.g = atoi(rgb[1].c_str());
				colour.b = atoi(rgb[2].c_str());
#endif

				pre += colour_prefix(colour);
				post = "</span>" + post;
			} else if(c == font::NULL_MARKUP) {
				line.erase(0, 1);
				proceed = false;
			} else {
				proceed = false;
			}
		}

		line = pre + escape_string(line) + post;
	}
	return utils::join(lines, '\n');
}

} // namespace gui2

