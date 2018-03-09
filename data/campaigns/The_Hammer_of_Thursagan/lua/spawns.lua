-- Used for the bandit spawns in scenario 5

local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"
local wml_actions = wesnoth.wml_actions
local T = helper.set_wml_tag_metatable {}

function wml_actions.spawn_units(cfg)
	local x = cfg.x or helper.wml_error("[spawn_units] missing required x= attribute.")
	local y = cfg.y or helper.wml_error("[spawn_units] missing required y= attribute.")
	local types = cfg.types or helper.wml_error("[spawn_units] missing required types= attribute.")
	local count = cfg.count or helper.wml_error("[spawn_units] missing required count= attribute.")
	local side = cfg.side or helper.wml_error("[spawn_units] missing required side= attribute.")

	local done = 0
	for i=1,count do
		local locs = wesnoth.get_locations({T["not"] { T.filter {} } , T["and"] { x = x, y = y, radius = 1 } })
		if #locs == 0 then locs = wesnoth.get_locations({T["not"] { T.filter {} } , T["and"] { x = x, y = y, radius = 2 } }) end
		if #locs == 0 then break end

		done = done + 1

		local unit_type = helper.rand(types)
		local loc_i = helper.rand("1.."..#locs)

		wml_actions.move_unit_fake({x = string.format("%d,%d", x, locs[loc_i][1]) , y = string.format("%d,%d", y, locs[loc_i][2]) , type = unit_type , side = side})
		wesnoth.put_unit({ type = unit_type , side = side, random_traits = "yes", generate_name = "yes" , upkeep = "loyal" }, locs[loc_i][1], locs[loc_i][2])
	end

	if done > 0 then
		for then_child in helper.child_range(cfg, "then") do
			local action = utils.handle_event_commands(then_child, "conditional")
			if action ~= "none" then return end
		end
	end
end
