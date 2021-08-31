--<<
local wc2_scenario = {}
local on_event = wesnoth.require("on_event")

function wc2_scenario.is_human_side(side_num)
	return side_num == 1 or side_num == 2 or side_num == 3
end

function wc2_scenario.scenario_num()
	return wml.variables["wc2_scenario"] or 1
end

function wc2_scenario.experience_penalty()
	return  wml.tag.effect {
		apply_to = "max_experience",
		increase = wml.variables["wc2_difficulty.experience_penalty"] .. "%",
	}
end

-- happens before training events.
on_event("recruit", 1, function(ec)
	local u = wesnoth.units.get(ec.x1, ec.y1)
	if (not u) or (not wc2_scenario.is_human_side(u.side)) then
		return
	end
	u:add_modification("advancement", { wc2_scenario.experience_penalty() })
end)

function wesnoth.wml_actions.wc2_start_units(cfg)
	local u = wesnoth.units.find_on_map({ side = cfg.side, canrecruit = true })[1]
	if not u then error("[wc2_start_units] no leader found") end
	u:add_modification("advancement", { wc2_scenario.experience_penalty() })
	u:add_modification("trait", wc2_heroes.trait_heroic )
	u.hitpoints = u.max_hitpoints
	u.moves = u.max_moves
	for i = 1, wml.variables["wc2_difficulty.heroes"] do
		wesnoth.wml_actions.wc2_random_hero {
			x = u.x,
			y = u.y,
			side = u.side,
		}
	end
end

function wesnoth.wml_actions.wc2_store_carryover(cfg)
	local human_sides = wesnoth.sides.find(wml.get_child(cfg, "sides"))
	--use an the average amount of villages for this scenario to stay independent of map generator results.
	local nvillages = cfg.nvillages
	local turns_left = math.max(wesnoth.scenario.turns - wesnoth.current.turn, 0)
	local player_gold = 0

	for side_num, side in ipairs(human_sides) do
		player_gold = player_gold + side.gold
	end
	local player_gold = math.max(player_gold / #human_sides, 0)
	wml.variables.wc2_carryover = math.ceil( (nvillages*turns_left + player_gold) * 0.15)
end

-- carryover handling: we use a custom carryover machnics that 
-- splits the carryover gold evenly to all players
on_event("prestart", function(cx)
	wesnoth.fire_event("wc2_start")
end)

-- we need to do this also after difficulty selection.
-- NOTE: this is a bit fragile, in particualr it breaks if difficulty_selection happens before the prestart event above.
on_event("wc2_start", function(cx)
	if wml.variables.wc2_scenario == 1 then
		for side_num = 1, wml.variables.wc2_player_count do
			wesnoth.wml_actions.wc2_start_units {
				side = side_num
			}
		end
	end

	if wml.variables.wc2_difficulty.extra_training then
		for side_num = 1, wml.variables.wc2_player_count do
			wesnoth.wml_actions.wc2_give_random_training {
				among="2,3,4,5,6",
				side = side_num,
			}
		end
	end
	
	local gold = (wml.variables.wc2_carryover or 0) + (wml.variables["wc2_difficulty.extra_gold"] or 0)
	for i = 1, wml.variables.wc2_player_count do
		wesnoth.sides[i].gold = wesnoth.sides[i].gold + gold
	end
	wml.variables.wc2_carryover = nil
end)

-- our victory condition
on_event("enemies defeated", function(cx)
	if wml.variables.wc2_scenario > 4 then
		return
	end
	wesnoth.audio.play("ambient/ship.ogg")
	wesnoth.wml_actions.endlevel {
		result = "victory",
		carryover_percentage = 0,
		carryover_add = false,
		carryover_report = false,
	}
end)

on_event("victory", function(cx)
	if wml.variables.wc2_scenario > 5 then
		return
	end
	wesnoth.wml_actions.wc2_set_recall_cost { }
	--{CLEAR_VARIABLE bonus.theme,bonus.point,items}
	wml.variables.wc2_scenario = (wml.variables.wc2_scenario or 1) + 1
end)

on_event("start", function(cx)
	wesnoth.wml_actions.wc2_objectives({})
end)

return wc2_scenario
-->>
