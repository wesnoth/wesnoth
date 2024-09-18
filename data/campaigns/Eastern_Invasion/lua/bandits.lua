-- Used for the bandit villages in S5. Much more specific than the village spawn implementations elsewhere,
-- since there are a lot more specific things needed (mostly the boss mechanics and village spreading)

local wml_actions = wesnoth.wml_actions
local _ = wesnoth.textdomain "wesnoth-ei"
local T = wml.tag
local vars = wml.variables

function wml_actions.spread_bandit_villages(cfg)
	local x = cfg.x or wml.error("[spread_bandit_villages] missing required x= attribute.")
	local y = cfg.y or wml.error("[spread_bandit_villages] missing required y= attribute.")
	local count = cfg.count or wml.error("[spread_bandit_villages] missing required count= attribute.")
	local types = cfg.types or wml.error("[spread_bandit_villages] missing required types= attribute.")

	vars.villages_visited = 0
	vars.boss_found = false
	vars.bandit_types = types

	local villages = wesnoth.map.find{gives_income = true, wml.tag['and'](cfg)}

	-- Shouldn't happen in the scenario, but a failsafe is always nice.
	if count > #villages then count = #villages end

	local village_i

	for i = 0, (count - 1) do
		village_i = mathx.random_choice("1.."..#villages)

		vars[string.format("bandit_villages[%d].x", i)] = villages[village_i][1]
		vars[string.format("bandit_villages[%d].y", i)] = villages[village_i][2]
		table.remove(villages, village_i)
	end
end

local function bandits_found(x,y)
	local bandit_types = vars.bandit_types
	local bandit_villages = wml.array_variables["bandit_villages"]
	local boss_found = vars.boss_found
	local visited = vars.villages_visited
	local rand1 = mathx.random_choice("3,4")
	local rand2 = mathx.random_choice("2.."..rand1)

	for i=1,rand2 do
		local radius = 1
		local locs
		repeat
			locs = wesnoth.map.find({T["not"] { T.filter {} } , T["and"] { x = x, y = y, radius = radius } })
			radius = radius + 1
		until locs[1]

		local bandit = mathx.random_choice(bandit_types)
		local loc_i = mathx.random_choice("1.."..#locs)

		wml_actions.move_unit_fake({x = string.format("%d,%d", x, locs[loc_i][1]), y = string.format("%d,%d", y, locs[loc_i][2]), type = bandit, side = "4"})
		wesnoth.units.to_map({ type = bandit, side = "4", random_traits = "yes", generate_name = "yes", upkeep = "loyal" }, locs[loc_i][1], locs[loc_i][2])
	end

	if not boss_found and visited > 2 then
		local boss_chance = (100 / #bandit_villages)
		local rand3 = mathx.random_choice("1..100")

		if rand3 <= boss_chance or #bandit_villages < 3 then
			vars.boss_found = true
			local loc = wesnoth.map.find({T["not"] { T.filter {} } , T["and"] { x = x, y = y, radius = 2 } })[1]
			wesnoth.game_events.fire("boss_found", x, y, loc[1], loc[2])
		end
	end
end

function wml_actions.bandit_village_capture(cfg)
	local bandit_villages = wml.array_access.get_proxy("bandit_villages")
	local x    = cfg.x or wml.error("[bandit_village_capture] missing required x= attribute.")
	local y    = cfg.y or wml.error("[bandit_village_capture] missing required y= attribute.")
	local unit = cfg.unit or wml.error("[bandit_village_capture] missing required unit= attribute.")

	for i=1,#bandit_villages do
		if bandit_villages[i].x == x and bandit_villages[i].y == y then
			vars[string.format("bandit_villages[%d]", i - 1)] = nil

			local visited = vars.villages_visited
			vars.villages_visited = visited + 1

			wesnoth.game_events.fire("addogin_advice", x, y, unit);
			wml.fire("message" , { x = x , y = y , message = _"They’re here!"})

			bandits_found(x,y)
			return
		end
	end

	wml.fire("message" , { x = x , y = y , message = _"No outlaws in this village."})
end
