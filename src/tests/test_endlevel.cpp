/* $Id$ */
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#include <boost/test/auto_unit_test.hpp>

#include "key.hpp"
#include "unit_types.hpp"
#include "dialogs.hpp"
#include "gamestatus.hpp"
//

#include "tests/utils/fake_event_source.hpp"


// Linker workarounds start here
#define LD_WA

#ifdef LD_WA
#include "config.hpp"
#include "serialization/parser.hpp"
#include "serialization/preprocessor.hpp"
#include "filesystem.hpp"
#include "sdl_utils.hpp"
#include "game_events.hpp"
#include "network.hpp"
// to load libwesnoth_extras
WML_HANDLER_FUNCTION(test_sources, , , )
{
	// To load libwesnoth_core
	network::get_pending_stats();
	// To load libwesnoth_sdl
	SDL_Color color = int_to_color(255);
	std::cerr << "Fooled you\n";
	{
		config cfg;
		scoped_istream stream = preprocess_file("data/hardwired/language.cfg");
		read(cfg, *stream);
	}
}
#endif
// Linker workarounds end here

namespace test {

	struct endlevel_fixture {
		test_utils::fake_event_source source;
		SDL_Event event;
	};

	BOOST_FIXTURE_TEST_SUITE( endlevel , endlevel_fixture)

		BOOST_AUTO_TEST_CASE( test_fake_input )
		{

			BOOST_MESSAGE("Starting endlevel test!");

			event.type = SDL_KEYDOWN;
			event.key.state = SDL_PRESSED;
			event.key.keysym.sym = SDLK_ESCAPE;
			event.key.keysym.scancode = 65;
			event.key.keysym.mod = KMOD_NONE;
			event.key.keysym.unicode = 0;

			test_utils::event_node_ptr new_keypress(new test_utils::event_node_keyboard(3, event));
			event.type = SDL_KEYUP;
			event.key.state = SDL_RELEASED;
			test_utils::event_node_ptr new_keyrelease(new test_utils::event_node_keyboard(5, event));
			source.add_event(new_keypress);
			source.add_event(new_keyrelease);


			CKey key;
			while(true)
			{
				events::pump();

				BOOST_CHECK_EQUAL(key[SDLK_ESCAPE], new_keypress->is_fired());
				if (key[SDLK_ESCAPE])
					break;
			}	
			while(true)
			{	
				events::pump();
				BOOST_CHECK_EQUAL(key[SDLK_ESCAPE], !new_keyrelease->is_fired());
				if (!key[SDLK_ESCAPE])
					break;
			}
		}

		BOOST_AUTO_TEST_CASE( test_dialog_get_save_name_enter_pressed )
		{
			// fill in events to be used in test
			test_utils::event_node_ptr press_return_before = source.press_key(0, SDLK_RETURN);
			test_utils::event_node_ptr release_return_before = source.release_key(3, SDLK_RETURN);
			test_utils::event_node_ptr press_return_after = source.press_key(6, SDLK_RETURN);
			test_utils::event_node_ptr release_return_after = source.release_key(9, SDLK_RETURN);
		
			CVideo video_;
			video_.make_test_fake();
			unit_map dummy_umap;
			config dummy_cfg;
			gamemap dummy_map(dummy_cfg, "");
			gamestatus dummy_status(dummy_cfg, 0);
			std::vector<team> dummy_teams;
			const events::event_context main_event_context_;

			game_display disp(dummy_umap, video_, dummy_map, dummy_status,
						dummy_teams, dummy_cfg, dummy_cfg, dummy_cfg);

			std::string fname("press_enter");

			// Start test (set ticks start time)
			source.start();
			// Activated enter press
			events::pump();

			BOOST_CHECK_MESSAGE(press_return_before->is_fired(), "Enter wasn't activated");
			BOOST_CHECK_MESSAGE(!release_return_before->is_fired(), "Enter was released before test");

			BOOST_CHECK_EQUAL(dialogs::get_save_name(disp, "Save game?", "file", &fname,gui::OK_CANCEL, "Save game", false, true), 0);

			BOOST_CHECK_MESSAGE(release_return_before->is_fired(), "get_save_name returned before releasing first enter.");
			BOOST_CHECK_MESSAGE(press_return_after->is_fired(), "get_save_name returned before 2nd enter event was sent");
			BOOST_CHECK_MESSAGE(!release_return_after->is_fired(), "get_save_name returned after 2nd release event was sent");

		}

	BOOST_AUTO_TEST_SUITE_END()
}
