/*
   Copyright (C) 2011 - 2016 by Karol Kozub <karol.alt@gmail.com>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-test"

#include <boost/test/unit_test.hpp>
#include <sstream>

#include "game_config.hpp"
#include "config_cache.hpp"
#include "config.hpp"
#include "color_range.hpp"
#include "image.hpp"
#include "image_modifications.hpp"
#include "log.hpp"
#include "filesystem.hpp"

using namespace image;

namespace {
/// Sets up the environment for every test
class environment_setup
{
public:
	environment_setup()
		// redirects the output to an ignored stream
		: ignored_stream_()
		, output_redirect_(ignored_stream_)
		, paths_manager_()
	{
		set_up_color_info();
		set_up_team_colors();
		set_up_image_paths();
	}

private:
	/** Sets up color_info configuration
	 *
	 * This is required by the RC modifications
	 */
	void set_up_color_info()
	{
		config cfg;
		cfg.add_child("color_range",
			       create_color_range("red",
						  "FF0000,FFFFFF,000000,FF0000",
						  "Red"));
		cfg.add_child("color_range",
			       create_color_range("blue",
						  "2E419B,FFFFFF,0F0F0F,0000FF",
						  "Blue"));

		game_config::add_color_info(cfg);
	}

	/** Sets up team color mapping
	 *
	 * This is required by TC modification
	 */
	static void set_up_team_colors()
	{
		std::vector<std::string> tc;

		tc.push_back("red");
		tc.push_back("blue");

		image::set_team_colors(&tc);
	}

	/** Sets up the paths later used to load images
	 *
	 * This is required by all the modifications that use image::get_image
	 * to load images from disk
	 */
	void set_up_image_paths()
	{
		config cfg;

		cfg.add_child("binary_path",
			      create_path_config("data/core"));

		paths_manager_.set_paths(cfg);
	}

	static config create_color_range(const std::string& id,
				  const std::string& rgb,
				  const std::string& name)
	{
		config cfg;

		cfg["id"] = id;
		cfg["rgb"] = rgb;
		cfg["name"] = name;

		return cfg;
	}

	static config create_path_config(const std::string& path)
	{
		config cfg;

		cfg["path"] = path;

		return cfg;
	}

	std::stringstream ignored_stream_;
	lg::redirect_output_setter output_redirect_;
	filesystem::binary_paths_manager paths_manager_;
};
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(image_modification_parsing)

/** Tests if modifications with a higher priority are placed before the others
 *
 * The RC modification has a higher priority than other modifications and has
 * to be applied before all the others. This test checks if that order is taken
 * care of by the queue.
 */
BOOST_AUTO_TEST_CASE(test_modificaiton_queue_order)
{
	environment_setup env_setup;

	modification_queue queue;
	modification* low_priority_mod = new fl_modification();
	modification* high_priority_mod = new rc_modification();

	queue.push(low_priority_mod);
	queue.push(high_priority_mod);

	BOOST_REQUIRE_EQUAL(queue.size(), 2);

	BOOST_CHECK_EQUAL(queue.top(), high_priority_mod);
	queue.pop();
	BOOST_CHECK_EQUAL(queue.top(), low_priority_mod);
	queue.pop();

	low_priority_mod = new fl_modification();
	high_priority_mod = new rc_modification();

	// reverse insertion order now
	queue.push(high_priority_mod);
	queue.push(low_priority_mod);

	BOOST_REQUIRE_EQUAL(queue.size(), 2);

	BOOST_CHECK_EQUAL(queue.top(), high_priority_mod);
	queue.pop();
	BOOST_CHECK_EQUAL(queue.top(), low_priority_mod);
	queue.pop();
}

/// Tests if the TC modification is correctly decoded
BOOST_AUTO_TEST_CASE(test_tc_modification_decoding)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~TC(1,blue)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	rc_modification* mod = dynamic_cast<rc_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	const std::vector<color_t>& old_color = game_config::tc_info("blue");
	// The first team color is red
	const color_range& new_color = game_config::color_info("red");
	color_range_map expected = recolor_range(new_color, old_color);

	BOOST_CHECK(expected == mod->map());
}

/// Tests if the TC modification with invalid arguments is ignored
BOOST_AUTO_TEST_CASE(test_tc_modification_decoding_invalid_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~TC()~TC(1)~TC(0,blue)");

	BOOST_REQUIRE_EQUAL(queue.size(), 0);
}

/// Tests if the RC modification is correctly decoded
BOOST_AUTO_TEST_CASE(test_rc_modification_decoding)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~RC(red>blue)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	rc_modification* mod = dynamic_cast<rc_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	const std::vector<color_t>& old_color = game_config::tc_info("red");
	const color_range& new_color = game_config::color_info("blue");
	color_range_map expected = recolor_range(new_color, old_color);

	BOOST_CHECK(expected == mod->map());
}

/// Tests if the RC modification with invalid arguments is ignored
BOOST_AUTO_TEST_CASE(test_rc_modification_decoding_invalid_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~RC()~RC(blue)~RC(>)");

	BOOST_REQUIRE_EQUAL(queue.size(), 0);
}

/// Tests if the PAL modification is correctly decoded
BOOST_AUTO_TEST_CASE(test_pal_modification_decoding)
{
	environment_setup env_setup;

	modification_queue queue =
		modification::decode("~PAL(000000,005000 > FFFFFF,FF00FF)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	rc_modification* mod = dynamic_cast<rc_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	std::vector<color_t> const& old_palette = game_config::tc_info("000000,005000");
	std::vector<color_t> const& new_palette = game_config::tc_info("FFFFFF,FF00FF");
	color_range_map expected;

	for(size_t i = 0; i < old_palette.size() && i < new_palette.size(); ++i) {
	environment_setup env_setup;

		expected[old_palette[i]] = new_palette[i];
	}

	BOOST_CHECK(expected == mod->map());
}

/// Tests if the PAL modification with invalid arguments is ignored
BOOST_AUTO_TEST_CASE(test_pal_modification_decoding_invalid_args)
{
	environment_setup env_setup;

	modification_queue queue =
		modification::decode("~PAL()~PAL(>)");

	BOOST_REQUIRE_EQUAL(queue.size(), 0);
}

/// Tests if the FL modification is correctly decoded without arguments
BOOST_AUTO_TEST_CASE(test_fl_modification_decoding_default)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~FL()");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	fl_modification* mod = dynamic_cast<fl_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(mod->get_horiz());
	BOOST_CHECK(!mod->get_vert());
}

/// Tests if the FL modification is correctly decoded with the horiz argument
BOOST_AUTO_TEST_CASE(test_fl_modification_decoding_horiz)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~FL(horiz)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	fl_modification* mod = dynamic_cast<fl_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(mod->get_horiz());
	BOOST_CHECK(!mod->get_vert());
}

/// Tests if the FL modification is correctly decoded with the vert argument
BOOST_AUTO_TEST_CASE(test_fl_modification_decoding_vert)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~FL(vert)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	fl_modification* mod = dynamic_cast<fl_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(!mod->get_horiz());
	BOOST_CHECK(mod->get_vert());
}

/// Tests if the FL modification is correctly decoded with both horiz and vert
BOOST_AUTO_TEST_CASE(test_fl_modification_decoding_horiz_and_vert)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~FL(horiz,vert)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	fl_modification* mod = dynamic_cast<fl_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(mod->get_horiz());
	BOOST_CHECK(mod->get_vert());
}

/// Tests if the GS modification is correctly decoded
BOOST_AUTO_TEST_CASE(test_gs_modification_decoding)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~GS()");

	BOOST_REQUIRE(queue.size() == 1);

	gs_modification* mod = dynamic_cast<gs_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_CHECK(mod != nullptr);
}

/// Tests if the CROP modification without arguments is ignored
BOOST_AUTO_TEST_CASE(test_crop_modification_decoding_no_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~CROP()");

	BOOST_REQUIRE_EQUAL(queue.size(), 0);
}

/// Tests if the CROP modification is correctly decoded when given one argument
BOOST_AUTO_TEST_CASE(test_crop_modification_decoding_1_arg)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~CROP(1)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	crop_modification* mod = dynamic_cast<crop_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	const SDL_Rect& slice = mod->get_slice();

	BOOST_CHECK_EQUAL(slice.x, 1);
	BOOST_CHECK_EQUAL(slice.y, 0);
	BOOST_CHECK_EQUAL(slice.w, 0);
	BOOST_CHECK_EQUAL(slice.h, 0);
}

/// Tests if the CROP modification is correctly decoded when given two args
BOOST_AUTO_TEST_CASE(test_crop_modification_decoding_2_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~CROP(1,2)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	crop_modification* mod = dynamic_cast<crop_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	const SDL_Rect& slice = mod->get_slice();

	BOOST_CHECK_EQUAL(slice.x, 1);
	BOOST_CHECK_EQUAL(slice.y, 2);
	BOOST_CHECK_EQUAL(slice.w, 0);
	BOOST_CHECK_EQUAL(slice.h, 0);
}

/// Tests if the CROP modification is correctly decoded when given three args
BOOST_AUTO_TEST_CASE(test_crop_modification_decoding_3_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~CROP(1,2,3)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	crop_modification* mod = dynamic_cast<crop_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	const SDL_Rect& slice = mod->get_slice();

	BOOST_CHECK_EQUAL(slice.x, 1);
	BOOST_CHECK_EQUAL(slice.y, 2);
	BOOST_CHECK_EQUAL(slice.w, 3);
	BOOST_CHECK_EQUAL(slice.h, 0);
}

/// Tests if the CROP modification is correctly decoded when given four args
BOOST_AUTO_TEST_CASE(test_crop_modification_decoding_4_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~CROP(1,2,3,4)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	crop_modification* mod = dynamic_cast<crop_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	const SDL_Rect& slice = mod->get_slice();

	BOOST_CHECK_EQUAL(slice.x, 1);
	BOOST_CHECK_EQUAL(slice.y, 2);
	BOOST_CHECK_EQUAL(slice.w, 3);
	BOOST_CHECK_EQUAL(slice.h, 4);
}

/** Tests if the BLIT modification with one argument is correctly decoded
 *
 * @todo check if the surface is correct
 */
BOOST_AUTO_TEST_CASE(test_blit_modification_decoding_1_arg)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BLIT(wesnoth-icon.png)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	blit_modification* mod = dynamic_cast<blit_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(!mod->get_surface().null());
	BOOST_CHECK_EQUAL(mod->get_x(), 0);
	BOOST_CHECK_EQUAL(mod->get_y(), 0);
}

/** Tests if the BLIT modification with three arguments is correctly decoded
 *
 * @todo check if the surface is correct
 */
BOOST_AUTO_TEST_CASE(test_blit_modification_decoding_3_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BLIT(wesnoth-icon.png,1,2)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	blit_modification* mod = dynamic_cast<blit_modification*>(queue.top());

	BOOST_REQUIRE(mod != nullptr);
	// The dynamic_cast returns nullptr if the argument doesn't match the type

	BOOST_CHECK(!mod->get_surface().null());
	BOOST_CHECK_EQUAL(mod->get_x(), 1);
	BOOST_CHECK_EQUAL(mod->get_y(), 2);
}

/// Tests if the BLIT modification with invalid arguments is ignored
BOOST_AUTO_TEST_CASE(test_blit_modification_decoding_invalid_args)
{
	environment_setup env_setup;

	modification_queue queue =
		modification::decode("~BLIT()"
				     "~BLIT(wesnoth-icon.png,1,-2)"
				     "~BLIT(wesnoth-icon.png,-1,2)"
				     "~BLIT(wesnoth-icon.png,-1,-2)");

	BOOST_CHECK_EQUAL(queue.size(), 0);
}

/** Tests if the MASK modification with one argument is correctly decoded
 *
 * @todo check if the surface is correct
 */
BOOST_AUTO_TEST_CASE(test_mask_modification_decoding_1_arg)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~MASK(wesnoth-icon.png)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	mask_modification* mod = dynamic_cast<mask_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(!mod->get_mask().null());
	BOOST_CHECK_EQUAL(mod->get_x(), 0);
	BOOST_CHECK_EQUAL(mod->get_y(), 0);
}

/** Tests if the MASK modification with three arguments is correctly decoded
 *
 * @todo check if the surface is correct
 */
BOOST_AUTO_TEST_CASE(test_mask_modification_decoding_3_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~MASK(wesnoth-icon.png,3,4)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	mask_modification* mod = dynamic_cast<mask_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(!mod->get_mask().null());
	BOOST_CHECK_EQUAL(mod->get_x(), 3);
	BOOST_CHECK_EQUAL(mod->get_y(), 4);
}

/// Tests if the MASK modification with invalid arguments is ignored
BOOST_AUTO_TEST_CASE(test_mask_modification_decoding_invalid_args)
{
	environment_setup env_setup;

	modification_queue queue =
		modification::decode("~MASK()"
				     "~MASK(wesnoth-icon.png,3,-4)"
				     "~MASK(wesnoth-icon.png,-3,4)"
				     "~MASK(wesnoth-icon.png,-3,-4)");

	BOOST_CHECK_EQUAL(queue.size(), 0);
}

/// Tests if the L modification without arguments is ignored
BOOST_AUTO_TEST_CASE(test_l_modification_decoding_no_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~L()");

	BOOST_CHECK_EQUAL(queue.size(), 0);
}

/** Tests if the L modification with one argument is correctly decoded
 *
 * @todo check if the surface is correct
 */
BOOST_AUTO_TEST_CASE(test_l_modification_decoding_1_arg)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~L(wesnoth-icon.png)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	light_modification* mod = dynamic_cast<light_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(!mod->get_surface().null());
}

/// Tests if the SCALE modification without arguments is ignored
BOOST_AUTO_TEST_CASE(test_scale_modification_decoding_no_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~SCALE()");

	BOOST_CHECK_EQUAL(queue.size(), 0);
}

/// Tests if the SCALE modification with one argument is correctly decoded
BOOST_AUTO_TEST_CASE(test_scale_modification_decoding_1_arg)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~SCALE(3)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	scale_modification* mod = dynamic_cast<scale_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_w(), 3);
	BOOST_CHECK_EQUAL(mod->get_h(), 0);
}

/// Tests if the SCALE modification with two arguments is correctly decoded
BOOST_AUTO_TEST_CASE(test_scale_modification_decoding_2_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~SCALE(4,5)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	scale_modification* mod = dynamic_cast<scale_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_w(), 4);
	BOOST_CHECK_EQUAL(mod->get_h(), 5);
}

/// Tests if the O modification with a percent argument is correctly decoded
BOOST_AUTO_TEST_CASE(test_o_modification_decoding_percent_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~O(45%)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	o_modification* mod = dynamic_cast<o_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(mod->get_opacity() > 0.44f);
	BOOST_CHECK(mod->get_opacity() < 0.46f);
}

/// Tests if the O modification with a fraction argument is correctly decoded
BOOST_AUTO_TEST_CASE(test_o_modification_decoding_fraction_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~O(0.34)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	o_modification* mod = dynamic_cast<o_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK(mod->get_opacity() > 0.33f);
	BOOST_CHECK(mod->get_opacity() < 0.35f);
}

/// Tests if the BL modification without arguments is correctly decoded
BOOST_AUTO_TEST_CASE(test_bl_modification_decoding_no_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BL()");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	bl_modification* mod = dynamic_cast<bl_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_depth(), 0);
}

/// Tests if the BL modification with one argument is correctly decoded
BOOST_AUTO_TEST_CASE(test_bl_modification_decoding)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BL(2)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	bl_modification* mod = dynamic_cast<bl_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_depth(), 2);
}

/// Tests if the R, G and B modifications without args are correctly decoded
BOOST_AUTO_TEST_CASE(test_rgb_modification_decoding_no_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~R()~G()~B()");

	BOOST_REQUIRE_EQUAL(queue.size(), 3);

	for(int i = 0; i < 3; i++) {
	environment_setup env_setup;

		cs_modification* mod = dynamic_cast<cs_modification*>(queue.top());

		// The dynamic_cast returns nullptr if the argument doesn't match the type
		BOOST_REQUIRE(mod != nullptr);

		BOOST_CHECK_EQUAL(mod->get_r(), 0);
		BOOST_CHECK_EQUAL(mod->get_g(), 0);
		BOOST_CHECK_EQUAL(mod->get_b(), 0);

		queue.pop();
	}
}

/// Tests if the R modification with one argument is correctly decoded
BOOST_AUTO_TEST_CASE(test_r_modification_decoding)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~R(123)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	cs_modification* mod = dynamic_cast<cs_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_r(), 123);
	BOOST_CHECK_EQUAL(mod->get_g(), 0);
	BOOST_CHECK_EQUAL(mod->get_b(), 0);
}

/// Tests if the G modification with one argument is correctly decoded
BOOST_AUTO_TEST_CASE(test_g_modification_decoding)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~G(132)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	cs_modification* mod = dynamic_cast<cs_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_r(), 0);
	BOOST_CHECK_EQUAL(mod->get_g(), 132);
	BOOST_CHECK_EQUAL(mod->get_b(), 0);
}

/// Tests if the B modification with one argument is correctly decoded
BOOST_AUTO_TEST_CASE(test_b_modification_decoding)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~B(312)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	cs_modification* mod = dynamic_cast<cs_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_r(), 0);
	BOOST_CHECK_EQUAL(mod->get_g(), 0);
	BOOST_CHECK_EQUAL(mod->get_b(), 312);
}

/// Tests if the BG modification without arguments is correctly decoded
BOOST_AUTO_TEST_CASE(test_bg_modification_decoding_no_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BG()");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	background_modification* mod = dynamic_cast<background_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_color().r, 0);
	BOOST_CHECK_EQUAL(mod->get_color().g, 0);
	BOOST_CHECK_EQUAL(mod->get_color().b, 0);
	BOOST_CHECK_EQUAL(mod->get_color().a, SDL_ALPHA_OPAQUE);
}

/// Tests if the BG modification with one argument is correctly decoded
BOOST_AUTO_TEST_CASE(test_bg_modification_decoding_1_arg)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BG(1)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	background_modification* mod = dynamic_cast<background_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_color().r, 1);
	BOOST_CHECK_EQUAL(mod->get_color().g, 0);
	BOOST_CHECK_EQUAL(mod->get_color().b, 0);
	BOOST_CHECK_EQUAL(mod->get_color().a, SDL_ALPHA_OPAQUE);
}

/// Tests if the BG modification with two arguments is correctly decoded
BOOST_AUTO_TEST_CASE(test_bg_modification_decoding_2_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BG(1,2)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	background_modification* mod = dynamic_cast<background_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_color().r, 1);
	BOOST_CHECK_EQUAL(mod->get_color().g, 2);
	BOOST_CHECK_EQUAL(mod->get_color().b, 0);
	BOOST_CHECK_EQUAL(mod->get_color().a, SDL_ALPHA_OPAQUE);
}

/// Tests if the BG modification with three arguments is correctly decoded
BOOST_AUTO_TEST_CASE(test_bg_modification_decoding_3_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BG(1,2,3)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	background_modification* mod = dynamic_cast<background_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_color().r, 1);
	BOOST_CHECK_EQUAL(mod->get_color().g, 2);
	BOOST_CHECK_EQUAL(mod->get_color().b, 3);
	BOOST_CHECK_EQUAL(mod->get_color().a, SDL_ALPHA_OPAQUE);
}

/// Tests if the BG modification with four arguments is correctly decoded
BOOST_AUTO_TEST_CASE(test_bg_modification_decoding_4_args)
{
	environment_setup env_setup;

	modification_queue queue = modification::decode("~BG(1,2,3,4)");

	BOOST_REQUIRE_EQUAL(queue.size(), 1);

	background_modification* mod = dynamic_cast<background_modification*>(queue.top());

	// The dynamic_cast returns nullptr if the argument doesn't match the type
	BOOST_REQUIRE(mod != nullptr);

	BOOST_CHECK_EQUAL(mod->get_color().r, 1);
	BOOST_CHECK_EQUAL(mod->get_color().g, 2);
	BOOST_CHECK_EQUAL(mod->get_color().b, 3);
	BOOST_CHECK_EQUAL(mod->get_color().a, 4);
}

BOOST_AUTO_TEST_SUITE_END()
