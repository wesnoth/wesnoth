/*
   Copyright (C) 2008 - 2018 by Pauli Nieminen <paniemin@cc.hut.fi>
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

#include "tests/utils/fake_display.hpp"

#include "game_board.hpp"
#include "game_display.hpp"
#include "terrain/type_data.hpp"
#include "reports.hpp"

namespace wb {
	class manager;
}

namespace test_utils {

	class fake_display_manager {
		static fake_display_manager* manager_;

		CVideo video_;
		config dummy_cfg_;
		config dummy_cfg2_;
		game_board dummy_board_;
		reports dummy_reports;
		const events::event_context main_event_context_;


		game_display disp_;

		public:
		static fake_display_manager* get_manager();
		game_display& get_display();

		fake_display_manager();
//		~fake_display_manager();
	};

	fake_display_manager* fake_display_manager::manager_ = 0;

	fake_display_manager* fake_display_manager::get_manager()
	{
		if (!manager_)
		{
			manager_ = new fake_display_manager();
		}
		return manager_;
	}

	fake_display_manager::fake_display_manager() :
	   	video_(CVideo::FAKE_TEST),
		dummy_cfg_(),
		dummy_cfg2_(),
		dummy_board_(std::make_shared<terrain_type_data>(dummy_cfg_), dummy_cfg2_),
		main_event_context_(),
		disp_(dummy_board_, std::shared_ptr<wb::manager> (), dummy_reports, dummy_cfg_, dummy_cfg_)
	{
	}

	game_display& fake_display_manager::get_display()
	{
		return disp_;
	}

	game_display& get_fake_display(const int width, const int height)
	{
		game_display& display =
				fake_display_manager::get_manager()->get_display();

		if(width >= 0 && height >= 0) {
			display.video().make_test_fake(width, height);
		}

		return display;
	}


}
