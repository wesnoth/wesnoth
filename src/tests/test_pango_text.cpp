/*
	Copyright (C) 2026
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

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

#include "tests/utils/game_config_manager_tests.hpp"

#include "font/font_options.hpp"
#include "font/text.hpp"
#include "sdl/point.hpp"

#include <string>
#include <vector>


struct pango_text_fixture
{
	pango_text_fixture()
	{
		test_utils::get_test_config_ref();
	}
};

BOOST_FIXTURE_TEST_SUITE(pango_text, pango_text_fixture)

BOOST_AUTO_TEST_CASE(pango_text_wraps_at_maximum_width)
{
	const std::string text = "Alle Spieler erhalten das gleiche Einkommen in regelmäßigen Abständen. Alles andere Einkommen ist deaktiviert.\n\nInspiriert vom gleichnamigen 'Earth 2150' Spielmodus.";

	font::pango_text renderer;
	BOOST_REQUIRE(renderer.set_text(text, true));

	const auto configure = [&](int width) {
		renderer.clear_attributes();
		renderer
			.set_family_class(font::family_class::sans_serif)
			.set_font_size(15)
			.set_font_style(font::pango_text::STYLE_NORMAL)
			.set_alignment(PANGO_ALIGN_LEFT)
			.set_maximum_width(width)
			.set_maximum_height(-1, true)
			.set_ellipse_mode(PANGO_ELLIPSIZE_NONE)
			.set_characters_per_line(0);
	};

	const auto measure_and_get_size = [&](int width) {
		configure(width);
		return renderer.get_size();
	};

	const auto lines_at = [&](int width) {
		configure(width);
		return renderer.get_lines();
	};

	for(int width : {420, 400, 399, 398}) {
		const auto size = measure_and_get_size(width);
		const auto wrapped_at_width = lines_at(width);
		const auto wrapped_at_size = lines_at(size.x); // feed the resulting size/width back and expect the same layout/lines
		BOOST_TEST(wrapped_at_size == wrapped_at_width, "wrap at " << width << " changes at " << size.x);
	}
}

BOOST_AUTO_TEST_SUITE_END()
