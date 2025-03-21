--<<
local _ = wesnoth.textdomain 'wesnoth-wc'
local _wesnoth = wesnoth.textdomain "wesnoth"

local wc2_scenario = {}
local on_event = wesnoth.require("on_event")
local carryover = wesnoth.require("carryover_gold.lua")

function wc2_scenario.is_human_side(side_num)
	return side_num <= wml.variables.wc2_player_count
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

function wc2_scenario.average_gold()
	local total_gold = 0
	local nsides = 0

	for i, s in ipairs(wesnoth.sides) do
		if wc2_scenario.is_human_side(i) then
			nsides = nsides + 1
			total_gold = total_gold + s.gold
		end
	end
	return math.floor(total_gold / nsides + 0.5)
end

-- overwrite parts of the carryover gold implementation.
function carryover.set_side_carryover_gold(side)
	local turns_left = carryover.turns_left()
	-- make the carryover bonus independent of the map generation.
	local num_villages = wml.variables.wc2_nvillages or carryover.get_num_villages()

	local finishing_bonus_per_turn = wml.variables.wc2_early_victory_bonus or num_villages * side.village_gold + side.base_income
	local finishing_bonus =  finishing_bonus_per_turn * turns_left
	local avg_gold = wc2_scenario.average_gold()

	side.carryover_gold = math.ceil((avg_gold + finishing_bonus) * side.carryover_percentage / 100)

	return {
		turns_left = turns_left,
		avg_gold = avg_gold,
		finishing_bonus = finishing_bonus,
		finishing_bonus_per_turn = finishing_bonus_per_turn,
	}
end

---@param side side
---@param info table
---@return string
function carryover.remaining_gold_message(side, info)
	return "<small>\n" .. _wesnoth("Remaining gold: ") .. carryover.half_signed_value(side.gold) .. "</small>"
		.. "<small>\n" .. _("Average remaining gold: ") .. carryover.half_signed_value(info.avg_gold) .. "</small>"
end

---@param side side
---@param info table
---@return string
function carryover.total_gold_message(side, info)
	return "<small>" .. _wesnoth("Total gold: ") .. carryover.half_signed_value(info.avg_gold + info.finishing_bonus) .. "</small>"
end

-- carryover handling: we use a custom carryover machnics that
-- splits the carryover gold evenly to all players
on_event("prestart", function(cx)
	wesnoth.game_events.fire("wc2_start")
end)

-- we need to do this also after difficulty selection.
on_event("wc2_start", function(cx)
	if wml.variables.wc2_scenario == 1 then
		for side_num = 1, wml.variables.wc2_player_count do
			wesnoth.wml_actions.wc2_start_units {
				side = side_num
			}
		end

		if wml.variables.wc2_difficulty.extra_training then
			for side_num = 1, wml.variables.wc2_player_count do
				wesnoth.wml_actions.wc2_give_random_training {
					among="2,3,4,5,6",
					side = side_num,
				}
			end
		end
	end

	local gold = (wml.variables["wc2_difficulty.extra_gold"] or 0)
	for i = 1, wml.variables.wc2_player_count do
		wesnoth.sides[i].gold = wesnoth.sides[i].gold + gold
	end
end)

-- our victory condition
on_event("enemies defeated", function(cx)
	if wml.variables.wc2_scenario > 4 then
		return
	end
	wesnoth.audio.play("ambient/ship.ogg")
	wesnoth.wml_actions.endlevel {
		result = "victory",
		carryover_report = true,
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
