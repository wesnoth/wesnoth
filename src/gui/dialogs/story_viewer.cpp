/*
   Copyright (C) 2017 by Charles Dang <exodia339@gmail.com>
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

#include "gui/dialogs/story_viewer.hpp"

#include "formula/variant.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/core/point.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/window.hpp"

namespace gui2
{
namespace dialogs
{

// TODO: change the internal part handling to either use PangoAlignment or plain strings
//       once the GUI1 screen is dropped.
static PangoAlignment storyscreen_alignment_to_pango(const storyscreen::part::TEXT_ALIGNMENT alignment)
{
	PangoAlignment text_alignment = PANGO_ALIGN_LEFT;

	switch(alignment) {
		case storyscreen::part::TEXT_CENTERED:
			text_alignment = PANGO_ALIGN_CENTER;
			break;
		case storyscreen::part::TEXT_RIGHT:
			text_alignment = PANGO_ALIGN_RIGHT;
			break;
		default:
			break; // already set before
	}

	return text_alignment;
}

// Helper function to get the canvas shape data for the shading under the title area until
// I can figure out how to ensure it always stays on top of the canvas stack.
static config get_title_area_decor_config()
{
	static config cfg;
	cfg["x"] = 0;
	cfg["y"] = 0;
	cfg["w"] = "(screen_width)";
	cfg["h"] = "(image_original_height * 2)";
	cfg["name"] = "dialogs/story_title_decor.png";

	return cfg;
}

#if 0
static panel* get_new_panel(const std::string& definition)
{
	panel* panel_ptr = new panel();
	panel_ptr->set_definition(definition);
	panel_ptr->set_id("text_panel");

	return panel_ptr;
}
#endif

REGISTER_DIALOG(story_viewer)

story_viewer::story_viewer(storyscreen::controller& controller)
	: controller_(controller)
	, part_index_(0)
	, current_part_(nullptr)
	, timer_id_(0)
{
	update_current_part_ptr();
}

story_viewer::~story_viewer()
{
	if(timer_id_ != 0) {
		remove_timer(timer_id_);
		timer_id_ = 0;
	}
}

void story_viewer::pre_show(window& window)
{
	window.set_enter_disabled(true);

	// Special callback handle key presses
	connect_signal_pre_key_press(window, std::bind(&story_viewer::key_press_callback, this, std::ref(window), _5));

	connect_signal_mouse_left_click(find_widget<button>(&window, "next", false),
		std::bind(&story_viewer::nav_button_callback, this, std::ref(window), DIR_FORWARD));

	connect_signal_mouse_left_click(find_widget<button>(&window, "back", false),
		std::bind(&story_viewer::nav_button_callback, this, std::ref(window), DIR_BACKWARDS));

	display_part(window);
}

void story_viewer::update_current_part_ptr()
{
	current_part_ = controller_.get_part(part_index_);
}

void story_viewer::display_part(window& window)
{
	// Update Back button state. Doing this here so it gets called in pre_show too.
	find_widget<button>(&window, "back", false).set_active(part_index_ != 0);

	config cfg, image;

	//
	// Background images
	//
	bool has_background = false;

	for(const auto& layer : current_part_->get_background_layers()) {
		has_background |= !layer.file().empty();

		const bool preserve_ratio = layer.keep_aspect_ratio();

		// By default, no scaling will be applied.
		std::string width_formula  = "(image_original_width)";
		std::string height_formula = "(image_original_height)";

		// If scale width is true, the image will be stretched to screen width.
		if(layer.scale_horizontally()) {
			width_formula = "(width)";

			// Override height formula to preserve ratio, if applicable.
			if(preserve_ratio) {
				height_formula = "(image_original_height * width / image_original_width)";
			}
		}

		// If scale height is true, the image will be stretched to screen height.
		if(layer.scale_vertically()) {
			height_formula = "(height)";

			// Override width formula to preserve ratio, if applicable.
			if(preserve_ratio) {
				width_formula = "(image_original_width * height / image_original_height)";
			}
		}

		// Background layers are always centered.
		image["x"] = "(max(pos, 0) where pos = (width  / 2 - image_width  / 2))";
		image["y"] = "(max(pos, 0) where pos = (height / 2 - image_height / 2))";
		image["w"] = width_formula;
		image["h"] = height_formula;
		image["name"] = layer.file();

		cfg.add_child("image", image);
	}

	cfg.add_child("image", get_title_area_decor_config());

	canvas& window_canvas = window.get_canvas(0);

	window_canvas.set_cfg(cfg);

	// Needed to make the background redraw correctly.
	window_canvas.set_is_dirty(true);
	window.set_is_dirty(true);

	//
	// Title
	//
	std::string title;
	if(current_part_->show_title() && !current_part_->title().empty()) {
		title = current_part_->title();
	} else {
		// FIXME: seems if the label doesn't have *something* in it each time its set, the label will never show up.
		title = " ";
	}

	PangoAlignment title_text_alignment = storyscreen_alignment_to_pango(current_part_->title_text_alignment());

	label& title_label = find_widget<label>(&window, "title", false);

	title_label.set_text_alignment(title_text_alignment);
	title_label.set_label(title);

	//
	// Story text
	//
	stacked_widget& text_stack = find_widget<stacked_widget>(&window, "text_and_control_stack", false);

	//std::string new_definition;
	gui2::point new_origin;

	switch(current_part_->story_text_location()) {
		case storyscreen::part::BLOCK_TOP:
			//new_definition = "wml_message_top";
			// Default 0,0 origin value is correct.
			break;
		case storyscreen::part::BLOCK_MIDDLE:
			//new_definition = "wml_message_center";
			new_origin.y = (window.get_size().y / 2) - (text_stack.get_size().y / 2);
			break;
		case storyscreen::part::BLOCK_BOTTOM:
			//new_definition = "wml_message";
			new_origin.y = window.get_size().y - text_stack.get_size().y;
			break;
	}

	// FIXME: sometimes the text won't appear after a move...
	text_stack.set_origin(new_origin);

	// TODO
	//delete text_stack.get_layer_grid(0)->swap_child("text_panel", get_new_panel(new_definition), true);

	const std::string& part_text = current_part_->text();

	if(part_text.empty() || !has_background) {
		// No text or no background for this part, hide the background layer.
		text_stack.select_layer(1);
	} else if(text_stack.current_layer() != -1)  {
		// If the background layer was previously hidden, re-show it.
		text_stack.select_layer(-1);
	}

	// Convert the story part text alignment types into the Pango equivalents
	PangoAlignment story_text_alignment = storyscreen_alignment_to_pango(current_part_->story_text_alignment());

	scroll_label& text_label = find_widget<scroll_label>(&window, "part_text", false);

	text_label.set_text_alignment(story_text_alignment);
	text_label.set_label(part_text);

	//
	// Floating images (handle this last)
	//
	const auto& floating_images = current_part_->get_floating_images();

	// If we have images to draw, draw the first one now. A new non-repeating timer is added
	// after every draw to schedule the next one after the specified interval.
	if(!floating_images.empty()) {
		draw_foreground_image(window, floating_images.begin(), part_index_);
	}
}

void story_viewer::draw_foreground_image(window& window, floating_image_list::const_iterator image_iter, int this_part_index)
{
	const auto& images = current_part_->get_floating_images();

	// If the current part has changed or we're out of images to draw, exit the draw loop.
	if((this_part_index != part_index_) || (image_iter == images.end())) {
		timer_id_ = 0;
		return;
	}

	const auto& floating_image = *image_iter;

	config cfg, image;

	// FIXME: these aren't the proper locations - they need to be relative to a base rect!
	image["x"] = floating_image.ref_x();
	image["y"] = floating_image.ref_y();
	image["w"] = "(image_width)";
	image["h"] = "(image_height)";
	image["name"] = floating_image.file();

	cfg.add_child("image", image);

	canvas& window_canvas = window.get_canvas(0);

	window_canvas.append_cfg(cfg);
	window_canvas.set_is_dirty(true);

	window.set_is_dirty(true);

	// Schedule the next image draw. This *must* be a non-repeating timer!
	timer_id_ = add_timer(floating_image.display_delay(),
		std::bind(&story_viewer::draw_foreground_image, this, std::ref(window), ++image_iter, this_part_index), false);
}

void story_viewer::nav_button_callback(window& window, NAV_DIRECTION direction)
{
	part_index_ = (direction == DIR_FORWARD ? part_index_ + 1 : part_index_ -1);

	// If we've viewed all the parts, close the dialog.
	if(part_index_ == controller_.max_parts()) {
		window.close();
		return;
	}

	if(part_index_ < 0) {
		part_index_ = 0;
	}

	update_current_part_ptr();

	display_part(window);
}

void story_viewer::key_press_callback(window& window, const SDL_Keycode key)
{
	const bool next_keydown =
		   key == SDLK_SPACE
		|| key == SDLK_RETURN
		|| key == SDLK_KP_ENTER
		|| key == SDLK_RIGHT;

	const bool back_keydown =
		   key == SDLK_BACKSPACE
		|| key == SDLK_LEFT;

	if(next_keydown) {
		nav_button_callback(window, DIR_FORWARD);
	} else if(back_keydown) {
		nav_button_callback(window, DIR_BACKWARDS);
	}
}

} // namespace dialogs
} // namespace gui2
