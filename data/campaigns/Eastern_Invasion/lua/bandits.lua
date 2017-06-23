-- Used for the bandit villages in S5. Much more specific than the village spawn implementations elsewhere,
-- since there are a lot more specific things needed (mostly the boss mechanics and village spreading)

local helper = wesnoth.require "helper"
local wml_actions = wesnoth.wml_actions
local _ = wesnoth.textdomain "wesnoth-ei"
local T = helper.set_wml_tag_metatable {}

function wml_actions.spread_bandit_villages(cfg)
	local x = cfg.x or helper.wml_error("[spread_bandit_villages] missing required x= attribute.")
	local y = cfg.y or helper.wml_error("[spread_bandit_villages] missing required y= attribute.")
	local count = cfg.count or helper.wml_error("[spread_bandit_villages] missing required count= attribute.")
	local types = cfg.types or helper.wml_error("[spread_bandit_villages] missing required types= attribute.")

	wesnoth.set_variable("villages_visited", 0)
	wesnoth.set_variable("boss_found", false)

	wesnoth.set_variable("bandit_types", types)

	local villages = wesnoth.get_villages(cfg)

	-- Shouldn't happen in the scenario, but a failsafe is always nice.
	if count > #villages then count = #villages end

	local village_i

	for i = 0, (count - 1) do
		village_i = helper.rand("1.."..#villages)

		wesnoth.set_variable(string.format("bandit_villages[%d].x", i), villages[village_i][1])
		wesnoth.set_variable(string.format("bandit_villages[%d].y", i), villages[village_i][2])
		table.remove(villages, village_i)
	end
end

local function bandits_found(x,y)
	local bandit_types = wesnoth.get_variable("bandit_types")
	local bandit_villages = helper.get_variable_array("bandit_villages")
	local boss_found = wesnoth.get_variable("boss_found")
	local visited = wesnoth.get_variable("villages_visited")
	local rand1 = helper.rand("3,4")
	local rand2 = helper.rand("2.."..rand1)

	for i=1,rand2 do
		local radius = 1
		local locs
		repeat
			locs = wesnoth.get_locations({T["not"] { T.filter {} } , T["and"] { x = x, y = y, radius = radius } })
			radius = radius + 1
		until locs[1]

		local bandit = helper.rand(bandit_types)
		local loc_i = helper.rand("1.."..#locs)

		wml_actions.move_unit_fake({x = string.format("%d,%d", x, locs[loc_i][1]), y = string.format("%d,%d", y, locs[loc_i][2]), type = bandit, side = "4"})
		wesnoth.put_unit(locs[loc_i][1], locs[loc_i][2], { type = bandit, side = "4", random_traits = "yes", generate_name = "yes", upkeep = "loyal" })
	end

	if not boss_found and visited > 2 then
		local boss_chance = (100 / #bandit_villages)
		local rand3 = helper.rand("1..100")

		if rand3 <= boss_chance or #bandit_villages < 3 then
			wesnoth.set_variable("boss_found", true)
			local loc = wesnoth.get_locations({T["not"] { T.filter {} } , T["and"] { x = x, y = y, radius = 2 } })[1]
			wesnoth.fire_event("boss_found", x, y, loc[1], loc[2])
		end
	end
end

function wml_actions.bandit_village_capture(cfg)
	local bandit_villages = helper.get_variable_proxy_array("bandit_villages")
	local x = cfg.x or helper.wml_error("[bandit_village_capture] missing required x= attribute.")
	local y = cfg.y or helper.wml_error("[bandit_village_capture] missing required y= attribute.")

	for i=1,#bandit_villages do
		if bandit_villages[i].x == x and bandit_villages[i].y == y then
			wesnoth.set_variable(string.format("bandit_villages[%d]", i - 1))

			local visited = wesnoth.get_variable("villages_visited")
			wesnoth.set_variable("villages_visited", visited + 1)

			wesnoth.fire("message" , { x = x , y = y , message = _"They're here!"})

			bandits_found(x,y)
			return
		end
	end

	wesnoth.fire("message" , { x = x , y = y , message = _"No outlaws in this village."})
end
