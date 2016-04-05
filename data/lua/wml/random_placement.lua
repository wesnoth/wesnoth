local helper = wesnoth.require "lua/helper.lua"
local utils = wesnoth.require "lua/wml-utils.lua"

wesnoth.wml_actions.random_placement = function(cfg)
	local dist_le = nil
	
	local parsed = helper.shallow_parsed(cfg)
	-- TODO: In most cases this tag is used to place units, so maybe make include_borders=no the default for [filter_location]?
	local filter = helper.get_child(parsed, "filter_location") or {}
	local command = helper.get_child(parsed, "command") or helper.wml_error("[random_placement] missing required [command] subtag")
	local distance = cfg.min_distance or 0
	local num_items = cfg.num_items or helper.wml_error("[random_placement] missing required 'num_items' attribute")
	local variable = cfg.variable or helper.wml_error("[random_placement] missing required 'variable' attribute")
	local allow_less = cfg.allow_less == true
	local variable_previous = utils.start_var_scope(variable)

	if distance < 0 then
		-- optimisation for distance = -1
		dist_le = function() return false end
	elseif distance == 0 then
		-- optimisation for distance = 0
		dist_le = function(x1,y1,x2,y2) return x1 == x2 and y1 == y2 end
	else 
		-- optimisation: cloasure is faster than string lookups.
		local math_abs = math.abs
		-- same effect as helper.distance_between(x1,y1,x2,y2) <= distance but faster.
		dist_le = function(x1,y1,x2,y2)
			local d_x = math_abs(x1-x2)
			if d_x > distance then
				return false
			end
			if d_x % 2 ~= 0 then
				if x1 % 2 == 0 then
					y2 = y2 - 0.5
				else
					y2 = y2 + 0.5
				end
			end
			local d_y = math_abs(y1-y2)
			return d_x + 2*d_y <= 2*distance
		end
	end
	
	local locs = wesnoth.get_locations(filter)
	if type(num_items) == "string" then		
		num_items = math.floor(loadstring("local size = " .. #locs .. "; return " .. num_items)())
		print("num_items=" .. num_items .. ", #locs=" .. #locs)
	end
	local size = #locs
	for i = 1, num_items do
		if size == 0 then
			if allow_less then
				print("placed only " .. i .. " items")
				return
			else
				helper.wml_error("[random_placement] failed to place items. only " .. i .. " items were placed")
			end
		end
		local index = wesnoth.random(size)
		local point = locs[index]
		wesnoth.set_variable(variable .. ".x", point[1])
		wesnoth.set_variable(variable .. ".y", point[2])
		wesnoth.set_variable(variable .. ".n", i)
		for j = size, 1, -1 do
			if dist_le(locs[j][1], locs[j][2], point[1], point[2]) then
				-- optimisation: swapping elements and storing size in an extra variable is faster than table.remove(locs, j)
				locs[j] = locs[size]
				size = size - 1
			end
		end
		wesnoth.wml_actions.command (command)
	end
	utils.end_var_scope(variable, variable_previous)

end