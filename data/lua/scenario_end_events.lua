local carryover = wesnoth.require("carryover_gold.lua")

wesnoth.game_events.add {
	name = "scenario_end",
	id = "carryover_gold",
	first_time_only = false,
	priority = -1000,
	action = function()
		carryover.do_carryover_gold()
	end
}
