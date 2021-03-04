local utils = wesnoth.require "wml-utils"

wesnoth.wml_actions.random_placement = function(cfg)
	local parsed = wml.shallow_parsed(cfg)
	-- TODO: In most cases this tag is used to place units, so maybe make include_borders=no the default for [filter_location]?
	local filter = wml.get_child(parsed, "filter_location") or {}
	local command = wml.get_child(parsed, "command") or wml.error("[random_placement] missing required [command] subtag")
	local distance = cfg.min_distance or 0
	local num_items = cfg.num_items or wml.error("[random_placement] missing required 'num_items' attribute")
	local variable = cfg.variable or wml.error("[random_placement] missing required 'variable' attribute")
	local allow_less = cfg.allow_less == true
	local variable_previous <close> = utils.scoped_var(variable)
	local math_abs = math.abs
	local locs = wesnoth.map.find(filter)
	if type(num_items) == "string" then
		if num_items:match('^%s*%(.*%)%s*$') then
			local params = {size = #locs}
			local result = wesnoth.eval_formula(num_items, params)
			num_items = math.floor(tonumber(result))
		elseif num_items:match('^%d+%%$') then
			num_items = tonumber(num_items:sub(1,-2))
		else
			wesnoth.deprecated_message('num_items=lua', 2, '1.17.0', 'Use of Lua for [random_placement]num_items is deprecated. Use WFL instead and enclose the whole thing in parentheses.')
			num_items = math.floor(load("local size = " .. #locs .. "; return " .. num_items)())
			print("num_items=" .. num_items .. ", #locs=" .. #locs)
		end
	end
	local size = #locs
	for i = 1, num_items do
		if size == 0 then
			if allow_less then
				print("placed only " .. i .. " items")
				return
			else
				wml.error("[random_placement] failed to place items. only " .. i .. " items were placed")
			end
		end
		local index = wesnoth.random(size)
		local point = locs[index]
		wml.variables[variable .. ".x"] = point[1]
		wml.variables[variable .. ".y"] = point[2]
		wml.variables[variable .. ".n"] = i
		wml.variables[variable .. ".terrain"] = wesnoth.current.map[point]
		if distance < 0 then
			-- optimisation: nothing to do for distance < 0
		elseif distance == 0 then
			-- optimisation: for distance = 0 we just need to remove the element at index
			-- optimisation: swapping elements and storing size in an extra variable is faster than table.remove(locs, j)
			locs[index] = locs[size]
			size = size - 1
		else
			-- the default case and the main reason why this was implemented.
			for j = size, 1, -1 do
				local x1 = locs[j][1]
				local y1 = locs[j][2]
				local x2 = point[1]
				local y2 = point[2]
				-- optimisation: same effect as "if wesnoth.map.distance_between(x1,y1,x2,y2) <= distance then goto continue; end" but faster.
				local d_x = math_abs(x1-x2)
				if d_x > distance then
					goto continue
				end
				if d_x % 2 ~= 0 then
					if x1 % 2 == 0 then
						y2 = y2 - 0.5
					else
						y2 = y2 + 0.5
					end
				end
				local d_y = math_abs(y1-y2)
				if d_x + 2*d_y > 2*distance then
					goto continue
				end
				-- optimisation: swapping elements and storing size in an extra variable is faster than table.remove(locs, j)
				locs[j] = locs[size]
				size = size - 1
				::continue::
			end
		end
		-- TODO: This should really be "do" but is kept as "command" for compatibility
		for do_child in wml.child_range(cfg, "command") do
			local action = utils.handle_event_commands(do_child, "loop")
			if action == "break" then
				utils.set_exiting("none")
				return
			elseif action == "continue" then
				utils.set_exiting("none")
				break
			elseif action ~= "none" then
				return
			end
		end
	end
end
