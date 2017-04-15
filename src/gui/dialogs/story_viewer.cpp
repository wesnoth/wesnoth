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
#include "sound.hpp"

BOOST_TRIBOOL_THIRD_STATE(not_fading)

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
	cfg["name"] = "dialogs/story_title_decor.png~O(75%)";

	return cfg;
}

REGISTER_DIALOG(story_viewer)

story_viewer::story_viewer(storyscreen::controller& controller)
	: controller_(controller)
	, part_index_(0)
	, current_part_(nullptr)
	, timer_id_(0)
	, next_draw_(0)
	, fade_step_(0)
	, fading_in_(not_fading)
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

	window.connect_signal<event::DRAW>(
		std::bind(&story_viewer::draw_callback, this, std::ref(window)), event::dispatcher::front_child);

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

	//
	// Music and sound
	//
	if(!current_part_->music().empty()) {
		config music_config;
		music_config["name"] = current_part_->music();
		music_config["ms_after"] = 2000;
		music_config["immediate"] = true;

		sound::play_music_config(music_config);
	}

	if(!current_part_->sound().empty()) {
		sound::play_sound(current_part_->sound());
	}

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

		if(layer.scale_horizontally() && preserve_ratio) {
			height_formula = "(min((image_original_height * screen_width  / image_original_width), height))";
		} else if(layer.scale_vertically()) {
			height_formula = "(height)";
		}

		if(layer.scale_vertically() && preserve_ratio) {
			width_formula  = "(min((image_original_width  * screen_height / image_original_height), width))";
		} else if(layer.scale_horizontally()) {
			width_formula  = "(width)";
		}

		// Background layers are always centered.
		image["x"] = "(max(pos, 0) where pos = (width  / 2 - image_width  / 2))";
		image["y"] = "(max(pos, 0) where pos = (height / 2 - image_height / 2))";
		image["w"] = width_formula;
		image["h"] = height_formula;
		image["name"] = layer.file();

		/* In order to avoid manually loading the image and calculating the scaling factor, we instead
		 * delegate the task of setting the necessary variables to the canvas once the calculations
		 * have been made internally.
		 *
		 * This sets the necessary values with the data for "this" image when its drawn.
		 */
		if(layer.is_base_layer()) {
			image["actions"] = R"((
				[
					set_var('base_scale_x', as_decimal(image_width)  / as_decimal(image_original_width)),
					set_var('base_scale_y', as_decimal(image_height) / as_decimal(image_original_height)),
					set_var('base_origin', loc(clip_x, clip_y))
				]
			))";
		}

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
	label& title_label = find_widget<label>(&window, "title", false);

	std::string title_text = current_part_->title();
	bool showing_title;

	if(current_part_->show_title() && !title_text.empty()) {
		showing_title = true;

		PangoAlignment title_text_alignment = storyscreen_alignment_to_pango(current_part_->title_text_alignment());

		title_label.set_visible(widget::visibility::visible);
		title_label.set_text_alignment(title_text_alignment);
		title_label.set_label(title_text);
	} else {
		showing_title = false;

		title_label.set_visible(widget::visibility::invisible);
	}

	//
	// Story text
	//
	stacked_widget& text_stack = find_widget<stacked_widget>(&window, "text_and_control_stack", false);

	std::string new_panel_mode;

	switch(current_part_->story_text_location()) {
		case storyscreen::part::BLOCK_TOP:
			new_panel_mode = "top";
			break;
		case storyscreen::part::BLOCK_MIDDLE:
			new_panel_mode = "center";
			break;
		case storyscreen::part::BLOCK_BOTTOM:
			new_panel_mode = "bottom";
			break;
	}

	text_stack.set_vertical_alignment(new_panel_mode);

	/* Set the panel mode control variables.
	 *
	 * We use get_layer_grid here to ensure the widget is always found regardless of
	 * whether the background is visible or not.
	 */
	canvas& panel_canvas = find_widget<panel>(text_stack.get_layer_grid(1), "text_panel", false).get_canvas(0);

	panel_canvas.set_variable("panel_position", wfl::variant(new_panel_mode));
	panel_canvas.set_variable("title_present", wfl::variant(static_cast<int>(showing_title))); // cast to 0/1

	const std::string& part_text = current_part_->text();

	if(part_text.empty() || !has_background) {
		// No text or no background for this part, hide the background layer.
		text_stack.select_layer(2);
	} else if(text_stack.current_layer() != -1)  {
		// If the background layer was previously hidden, re-show it.
		text_stack.select_layer(-1);
	}

	// Convert the story part text alignment types into the Pango equivalents
	PangoAlignment story_text_alignment = storyscreen_alignment_to_pango(current_part_->story_text_alignment());

	scroll_label& text_label = find_widget<scroll_label>(&window, "part_text", false);

	text_label.set_text_alignment(story_text_alignment);
	text_label.set_text_alpha(0);
	text_label.set_label(part_text);

	begin_fade_draw(true);

	//
	// Floating images (handle this last)
	//
	const auto& floating_images = current_part_->get_floating_images();

	// If we have images to draw, draw the first one now. A new non-repeating timer is added
	// after every draw to schedule the next one after the specified interval.
	if(!floating_images.empty()) {
		draw_floating_image(window, floating_images.begin(), part_index_);
	}
}

void story_viewer::draw_floating_image(window& window, floating_image_list::const_iterator image_iter, int this_part_index)
{
	const auto& images = current_part_->get_floating_images();

	// If the current part has changed or we're out of images to draw, exit the draw loop.
	if((this_part_index != part_index_) || (image_iter == images.end())) {
		timer_id_ = 0;
		return;
	}

	const auto& floating_image = *image_iter;

	std::ostringstream x_ss;
	std::ostringstream y_ss;

	// Floating images are scaled by the same factor as the background.
	x_ss << "(trunc(fi_ref_x * base_scale_x) + base_origin.x";
	y_ss << "(trunc(fi_ref_y * base_scale_y) + base_origin.y";

	if(floating_image.centered()) {
		x_ss << " - (image_original_width  / 2)";
		y_ss << " - (image_original_height / 2)";
	}

	x_ss << " where fi_ref_x = " << floating_image.ref_x() << ")";
	y_ss << " where fi_ref_y = " << floating_image.ref_y() << ")";

	config cfg, image;

	image["x"] = x_ss.str();
	image["y"] = y_ss.str();
	image["w"] = floating_image.autoscale() ? "(width)"  : "(image_width)";
	image["h"] = floating_image.autoscale() ? "(height)" : "(image_height)";
	image["name"] = floating_image.file();

	cfg.add_child("image", image);

	canvas& window_canvas = window.get_canvas(0);

	// Needed to make the background redraw correctly.
	window_canvas.append_cfg(cfg);
	window_canvas.set_is_dirty(true);

	window.set_is_dirty(true);

	++image_iter;

	// If a delay is specified, schedule the next image draw. This *must* be a non-repeating timer!
	// Else draw the next image immediately.
	const unsigned int draw_delay = floating_image.display_delay();

	if(draw_delay != 0) {
		timer_id_ = add_timer(draw_delay,
			std::bind(&story_viewer::draw_floating_image, this, std::ref(window), image_iter, this_part_index), false);
	} else {
		draw_floating_image(window, image_iter, this_part_index);
	}
}

void story_viewer::nav_button_callback(window& window, NAV_DIRECTION direction)
{
	// If a button is pressed while fading in, abort and set alpha to full opaque.
	if(fading_in_) {
		halt_fade_draw();

		find_widget<scroll_label>(&window, "part_text", false).set_text_alpha(ALPHA_OPAQUE);
		return;
	}

	// If a button is pressed while fading out, skip and show next part.
	if(!fading_in_) {
		display_part(window);
		return;
	}

	assert(not_fading(fading_in_));

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

	begin_fade_draw(false);
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

void story_viewer::set_next_draw()
{
	next_draw_ = SDL_GetTicks() + 20;
}

void story_viewer::begin_fade_draw(bool fade_in)
{
	set_next_draw();

	fade_step_ = fade_in ? 0 : 10;
	fading_in_ = fade_in;
}

void story_viewer::halt_fade_draw()
{
	next_draw_ = 0;
	fade_step_ = -1;
	fading_in_ = not_fading;
}

void story_viewer::draw_callback(window& window)
{
	if(SDL_GetTicks() < next_draw_) {
		return;
	}

	// If we've faded fully in...
	if(fading_in_ && fade_step_ > 10) {
		halt_fade_draw();
		return;
	}

	// If we've faded fully out...
	if(!fading_in_ && fade_step_ < 0) {
		halt_fade_draw();

		display_part(window);
		return;
	}

	unsigned short new_alpha = std::min<unsigned short>(255, fade_step_ * 25.5);
	find_widget<scroll_label>(&window, "part_text", false).set_text_alpha(new_alpha);

	// The text stack also needs to be marked dirty so the background panel redraws correctly.
	find_widget<stacked_widget>(&window, "text_and_control_stack", false).set_is_dirty(true);

	if(fading_in_) {
		fade_step_ ++;
	} else if(!fading_in_) {
		fade_step_ --;
	}

	set_next_draw();
}

} // namespace dialogs
} // namespace gui2
