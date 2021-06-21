-- the main file for the WC2 mp campaign

T = wml.tag
on_event = wesnoth.require("on_event")


wesnoth.dofile("./game_mechanics/_load.lua")

wc2_era = wesnoth.require("./era/era.lua")

wc2_enemy = wesnoth.dofile("./campaign/enemy.lua")

wc2_scenario = wesnoth.dofile("./campaign/scenario.lua")
wesnoth.dofile("./campaign/autorecall.lua")
wesnoth.dofile("./campaign/objectives.lua")
wesnoth.dofile("./campaign/enemy_themed.lua")

on_event("prestart", function(cx)
	wesnoth.wml_actions.wc2_fix_colors {
		wml.tag.player_sides {
			side="1,2,3",
			wml.tag.has_unit {
				canrecruit = true,
			}
		}
	}
end)
