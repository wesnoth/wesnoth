/*
	Copyright (C) 2017 - 2024
	by Charles Dang <exodia339@gmail.com>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "display.hpp"
#include "formula/variant.hpp"
#include "sdl/point.hpp"
#include "gui/core/timer.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/grid.hpp"
#include "gui/widgets/image.hpp"
#include "gui/widgets/label.hpp"
#include "gui/widgets/scroll_label.hpp"
#include "gui/widgets/settings.hpp"
#include "gui/widgets/stacked_widget.hpp"
#include "gui/widgets/window.hpp"
#include "sound.hpp"
#include "variable.hpp"

namespace gui2::dialogs
{

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

// Stacked widget layer constants for the text stack.
static const unsigned int LAYER_BACKGROUND = 1;
static const unsigned int LAYER_TEXT = 2;

REGISTER_DIALOG(story_viewer)

story_viewer::story_viewer(const std::string& scenario_name, const config& cfg_parsed)
	: modal_dialog(window_id())
	, controller_(vconfig(cfg_parsed, true), scenario_name)
	, part_index_(0)
	, current_part_(nullptr)
	, timer_id_(0)
	, next_draw_(0)
	, fade_step_(0)
	, fade_state_(NOT_FADING)
{
	update_current_part_ptr();
}

void story_viewer::clear_image_timer()
{
	if(timer_id_ != 0) {
		remove_timer(timer_id_);
		timer_id_ = 0;
	}
}

story_viewer::~story_viewer()
{
	clear_image_timer();
}

void story_viewer::pre_show()
{
	set_enter_disabled(true);

	// Special callback handle key presses
	connect_signal_pre_key_press(*this, std::bind(&story_viewer::key_press_callback, this, std::placeholders::_5));

	connect_signal_mouse_left_click(find_widget<button>("next"),
		std::bind(&story_viewer::nav_button_callback, this, DIR_FORWARD));
	connect_signal_mouse_left_click(find_widget<button>("prev"),
		std::bind(&story_viewer::nav_button_callback, this, DIR_BACKWARDS));

	find_widget<scroll_label>("part_text")
		.connect_signal<event::LEFT_BUTTON_CLICK>(
			std::bind(&story_viewer::nav_button_callback, this, DIR_FORWARD), queue_position::front_pre_child);

	// Tell the game display not to draw
	game_was_already_hidden_ = display::get_singleton()->get_prevent_draw();
	display::get_singleton()->set_prevent_draw(true);

	display_part();
}

void story_viewer::post_show()
{
	// Bring the game display back again, if appropriate
	display::get_singleton()->set_prevent_draw(game_was_already_hidden_);
}

void story_viewer::update_current_part_ptr()
{
	current_part_ = controller_.get_part(part_index_);
}

void story_viewer::display_part()
{
	static const int VOICE_SOUND_SOURCE_ID = 255;
	// Update Back button state. Doing this here so it gets called in pre_show too.
	find_widget<button>("prev").set_active(part_index_ != 0);

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

	sound::stop_sound(VOICE_SOUND_SOURCE_ID);
	if(!current_part_->voice().empty()) {
		sound::play_sound_positioned(current_part_->voice(), VOICE_SOUND_SOURCE_ID, 0, 0);
	}

	config cfg, image;

	//
	// Background images
	//
	bool has_background = false;
	config* base_layer = nullptr;

	for(const auto& layer : current_part_->get_background_layers()) {
		has_background |= !layer.file().empty();

		const bool preserve_ratio = layer.keep_aspect_ratio();
		const bool tile_h = layer.tile_horizontally();
		const bool tile_v = layer.tile_vertically();

		// By default, no scaling will be applied.
		std::string width_formula  = "(image_original_width)";
		std::string height_formula = "(image_original_height)";

		// Background layers are almost always centered. In case of tiling, we want the full
		// area in the horizontal or vertical direction, so set the origin to 0 for that axis.
		// The resize mode will center the original image in the available area first/
		std::string x_formula;
		std::string y_formula;

		if(tile_h) {
			x_formula = "0";
		} else {
			x_formula = "(max(pos, 0) where pos = (width  / 2 - image_width  / 2))";
		}

		if(tile_v) {
			y_formula = "0";
		} else {
			y_formula = "(max(pos, 0) where pos = (height / 2 - image_height / 2))";
		}

		if(layer.scale_horizontally() && preserve_ratio) {
			height_formula = "(min((image_original_height * width  / image_original_width), height))";
		} else if(layer.scale_vertically() || tile_v) {
			height_formula = "(height)";
		}

		if(layer.scale_vertically() && preserve_ratio) {
			width_formula  = "(min((image_original_width  * height / image_original_height), width))";
		} else if(layer.scale_horizontally() || tile_h) {
			width_formula  = "(width)";
		}

		image["x"] = x_formula;
		image["y"] = y_formula;
		image["w"] = width_formula;
		image["h"] = height_formula;
		image["name"] = layer.file();
		image["resize_mode"] = (tile_h || tile_v) ? "tile_center" : "scale";

		config& layer_image = cfg.add_child("image", image);

		if(base_layer == nullptr || layer.is_base_layer()) {
			base_layer = &layer_image;
		}
	}

	canvas& window_canvas = get_canvas(0);

	/* In order to avoid manually loading the image and calculating the scaling factor, we instead
	 * delegate the task of setting the necessary variables to the canvas once the calculations
	 * have been made internally.
	 *
	 * This sets the necessary values with the data for "this" image when its drawn. If no base
	 * layer was found (which would be the case if no backgrounds were provided at all), simply set
	 * some sane defaults directly.
	 */
	if(base_layer != nullptr) {
		(*base_layer)["actions"] = R"((
			[
				set_var('base_scale_x', as_decimal(image_width)  / as_decimal(image_original_width)),
				set_var('base_scale_y', as_decimal(image_height) / as_decimal(image_original_height)),
				set_var('base_origin_x', clip_x),
				set_var('base_origin_y', clip_y)
			]
		))";
	} else {
		window_canvas.set_variable("base_scale_x", wfl::variant(1));
		window_canvas.set_variable("base_scale_y", wfl::variant(1));
		window_canvas.set_variable("base_origin_x", wfl::variant(0));
		window_canvas.set_variable("base_origin_y", wfl::variant(0));
	}

	cfg.add_child("image", get_title_area_decor_config());

	window_canvas.set_cfg(cfg);

	// Needed to make the background redraw correctly.
	window_canvas.update_size_variables();
	queue_redraw();

	//
	// Title
	//
	label& title_label = find_widget<label>("title");

	std::string title_text = current_part_->title();
	bool showing_title;

	if(current_part_->show_title() && !title_text.empty()) {
		showing_title = true;

		PangoAlignment title_text_alignment = decode_text_alignment(current_part_->title_text_alignment());

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
	stacked_widget& text_stack = find_widget<stacked_widget>("text_and_control_stack");

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
	canvas& panel_canvas = text_stack.get_layer_grid(LAYER_BACKGROUND)->find_widget<panel>("text_panel").get_canvas(0);

	panel_canvas.set_variable("panel_position", wfl::variant(new_panel_mode));
	panel_canvas.set_variable("title_present", wfl::variant(static_cast<int>(showing_title))); // cast to 0/1

	const std::string& part_text = current_part_->text();

	if(part_text.empty() || !has_background) {
		// No text or no background for this part, hide the background layer.
		text_stack.select_layer(LAYER_TEXT);
	} else if(text_stack.current_layer() != -1)  {
		// If the background layer was previously hidden, re-show it.
		text_stack.select_layer(-1);
	}

	// Convert the story part text alignment types into the Pango equivalents
	PangoAlignment story_text_alignment = decode_text_alignment(current_part_->story_text_alignment());

	scroll_label& text_label = find_widget<scroll_label>("part_text");
	text_label.set_text_alignment(story_text_alignment);
	text_label.set_text_alpha(0);
	text_label.set_label(part_text);

	// Regenerate any background blur texture
	panel_canvas.queue_reblur();

	begin_fade_draw(true);
	// if the previous page was skipped, it is possible that we already have a timer running.
	clear_image_timer();
	//
	// Floating images (handle this last)
	//
	const auto& floating_images = current_part_->get_floating_images();

	// If we have images to draw, draw the first one now. A new non-repeating timer is added
	// after every draw to schedule the next one after the specified interval.
	//
	// TODO: in the old GUI1 dialog, floating images delayed the appearance of the story panel until
	//       drawing was finished. Might be worth looking into restoring that.
	if(!floating_images.empty()) {
		draw_floating_image(floating_images.begin(), part_index_);
	}
}

void story_viewer::draw_floating_image(floating_image_list::const_iterator image_iter, int this_part_index)
{
	const auto& images = current_part_->get_floating_images();
	canvas& window_canvas = get_canvas(0);

	// If the current part has changed or we're out of images to draw, exit the draw loop.
	while((this_part_index == part_index_) && (image_iter != images.end())) {
		const auto& floating_image = *image_iter;
		++image_iter;

		std::ostringstream x_ss;
		std::ostringstream y_ss;

		// Floating images' locations are scaled by the same factor as the background.
		x_ss << "(trunc(" << floating_image.ref_x() << " * base_scale_x) + base_origin_x";
		y_ss << "(trunc(" << floating_image.ref_y() << " * base_scale_y) + base_origin_y";

		if(floating_image.centered()) {
			x_ss << " - (image_width  / 2)";
			y_ss << " - (image_height / 2)";
		}

		x_ss << ")";
		y_ss << ")";

		config image;
		image["x"] = x_ss.str();
		image["y"] = y_ss.str();

		// Width and height don't need to be set unless the image needs to be scaled.
		if(floating_image.resize_with_background()) {
			image["w"] = "(image_original_width * base_scale_x)";
			image["h"] = "(image_original_height * base_scale_y)";
		}

		image["name"] = floating_image.file();
		config cfg{"image", std::move(image)};

		window_canvas.append_cfg(cfg);

		// Needed to make the background redraw correctly.
		window_canvas.update_size_variables();
		queue_redraw();

		// If a delay is specified, schedule the next image draw and break out of the loop.
		const auto& draw_delay = floating_image.display_delay();
		if(draw_delay != std::chrono::milliseconds{0}) {
			// This must be a non-repeating timer
			timer_id_ = add_timer(draw_delay, std::bind(&story_viewer::draw_floating_image, this, image_iter, this_part_index), false);
			return;
		}
	}

	timer_id_ = 0;
}

void story_viewer::nav_button_callback(NAV_DIRECTION direction)
{
	// If a button is pressed while fading in, abort and set alpha to full opaque.
	if(fade_state_ == FADING_IN) {
		halt_fade_draw();

		// Only set full alpha if Forward was pressed.
		if(direction == DIR_FORWARD) {
			find_widget<scroll_label>("part_text").set_text_alpha(ALPHA_OPAQUE);
			flag_stack_as_dirty();
			return;
		}
	}

	// If a button is pressed while fading out, skip and show next part.
	if(fade_state_ == FADING_OUT) {
		display_part();
		return;
	}

	assert(fade_state_ == NOT_FADING);

	part_index_ = (direction == DIR_FORWARD ? part_index_ + 1 : part_index_ -1);

	// If we've viewed all the parts, close the dialog.
	if(part_index_ >= controller_.max_parts()) {
		close();
		return;
	}

	if(part_index_ < 0) {
		part_index_ = 0;
	}

	update_current_part_ptr();

	begin_fade_draw(false);
}

void story_viewer::key_press_callback(const SDL_Keycode key)
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
		nav_button_callback(DIR_FORWARD);
	} else if(back_keydown) {
		nav_button_callback(DIR_BACKWARDS);
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
	fade_state_ = fade_in ? FADING_IN : FADING_OUT;
}

void story_viewer::halt_fade_draw()
{
	next_draw_ = 0;
	fade_step_ = -1;
	fade_state_ = NOT_FADING;
}

void story_viewer::update()
{
	modal_dialog::update();

	if(next_draw_ && SDL_GetTicks() < next_draw_) {
		return;
	}

	if(fade_state_ == NOT_FADING) {
		return;
	}

	// If we've faded fully in...
	if(fade_state_ == FADING_IN && fade_step_ > 10) {
		halt_fade_draw();
		return;
	}

	// If we've faded fully out...
	if(fade_state_ == FADING_OUT && fade_step_ < 0) {
		halt_fade_draw();

		display_part();
		return;
	}

	unsigned short new_alpha = std::clamp<short>(fade_step_ * 25.5, 0, ALPHA_OPAQUE);
	find_widget<scroll_label>("part_text").set_text_alpha(new_alpha);

	// The text stack also needs to be marked dirty so the background panel redraws correctly.
	flag_stack_as_dirty();

	if(fade_state_ == FADING_IN) {
		fade_step_ ++;
	} else if(fade_state_ == FADING_OUT) {
		fade_step_ --;
	}

	set_next_draw();
}

void story_viewer::flag_stack_as_dirty()
{
	find_widget<stacked_widget>("text_and_control_stack").queue_redraw();
}

} // namespace dialogs
