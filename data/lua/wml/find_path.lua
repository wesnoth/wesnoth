local utils = wesnoth.require "wml-utils"

function wesnoth.wml_actions.find_path(cfg)
	local filter_unit = wml.get_child(cfg, "traveler") or wml.error("[find_path] missing required [traveler] tag")
	-- only the first unit matching
	local unit = wesnoth.units.find_on_map(filter_unit)[1] or wml.error("[find_path]'s filter didn't match any unit")
	local filter_location = wml.get_child(cfg, "destination") or wml.error( "[find_path] missing required [destination] tag" )
	-- support for $this_unit
	local this_unit <close> = utils.scoped_var("this_unit")

	wml.variables["this_unit"] = nil -- clearing this_unit
	wml.variables["this_unit"] = unit.__cfg -- cfg field needed

	local variable = cfg.variable or "path"
	local ignore_units = false
	local ignore_teleport = false

	if cfg.check_zoc == false then --if we do not want to check the ZoCs, we must ignore units
		ignore_units = true
	end
	if cfg.check_teleport == false then --if we do not want to check teleport, we must ignore it
		ignore_teleport = true
	end

	local allow_multiple_turns = cfg.allow_multiple_turns
	local ignore_visibility = not cfg.check_visibility

	local nearest_by_cost = true
	local nearest_by_distance = false
	local nearest_by_steps = false
	if (cfg.nearest_by or "movement_cost") == "hexes" then
		nearest_by_cost = false
		nearest_by_distance = true
		nearest_by_steps = false
	elseif (cfg.nearest_by or "movement_cost") == "steps" then
		nearest_by_cost = false
		nearest_by_distance = false
		nearest_by_steps = true
	end

	-- only the first location with the lowest distance and lowest movement cost will match.
	local locations = wesnoth.map.find(filter_location)

	local max_cost = nil
	if not allow_multiple_turns then max_cost = unit.moves end --to avoid wrong calculation on already moved units
	local current_distance, current_cost, current_steps = math.huge, math.huge, math.huge
	local current_location

	for index, location in ipairs(locations) do
		-- we test if location passed to pathfinder is invalid (border);
		-- if it is, do not use it, and continue the cycle
		if not wesnoth.current.map:on_border(location) then
			local distance = wesnoth.map.distance_between ( unit.x, unit.y, location )
			-- if we pass an unreachable location then an empty path and high value cost will be returned
			local path, cost = wesnoth.paths.find_path( unit, location, {
				max_cost = max_cost,
				ignore_units = ignore_units,
				ignore_teleport = ignore_teleport,
				ignore_visibility = ignore_visibility
			} )

			-- it's a reachable hex.
			-- it's not 0, and less than 42424242 which is the high value returned for unwalkable or busy terrains
			if #path ~= 0 and cost < 42424241 then
				local steps = #path

				local is_better = false
				if nearest_by_cost and cost < current_cost then
					is_better = true
				elseif nearest_by_distance and distance < current_distance then
					is_better = true
				elseif nearest_by_steps and steps < current_steps then
					is_better = true
				elseif cost == current_cost and distance == current_distance and steps == current_steps then
					-- the two options are equivalent. Treating this as not-better probably creates a bias for
					-- choosing the north-west option, treating it as better probably biases to south-east.
					-- Choosing false is more likely to match the option that 1.14 would choose.
					is_better = false
				elseif cost <= current_cost and distance <= current_distance and steps <= current_steps then
					is_better = true
				end

				if is_better then
					current_distance = distance
					current_cost = cost
					current_steps = steps
					current_location = location
				end
			end
		end
	end

	if current_location == nil then
		-- either no matching locations, or only inaccessible matching locations (maybe enemy units are there)
		if #locations == 0 then
			wesnoth.interface.add_chat_message("WML warning","[find_path]'s filter didn't match any location")
		end
		wml.variables[tostring(variable)] = { hexes = 0 } -- set only hexes, nil all other values
	else
		local path, cost = wesnoth.paths.find_path(
			unit,
			current_location,
			{
				max_cost = max_cost,
				ignore_units = ignore_units,
				ignore_teleport = ignore_teleport,
				ignore_visibility = ignore_visibility
			})
		local turns

		if cost == 0 then -- if location is the same, of course it doesn't cost any MP
			turns = 0
		else
			turns = math.ceil( ( ( cost - unit.moves ) / unit.max_moves ) + 1 )
		end

		if cost >= 42424241 then -- it's the high value returned for unwalkable or busy terrains
			wml.variables[tostring(variable)] = { hexes = 0 } -- set only length, nil all other values
			return
		end

		if not allow_multiple_turns and turns > 1 then -- location cannot be reached in one turn
			wml.variables[tostring(variable)] = { hexes = 0 }
			return
		end -- skip the cycles below

		wml.variables[tostring( variable )] =
			{
				hexes = current_distance,
				from_x = unit.x, from_y = unit.y,
				to_x = current_location[1], to_y = current_location[2],
				movement_cost = cost,
				required_turns = turns
			}

		for index, path_loc in ipairs(path) do
			local sub_path, sub_cost = wesnoth.paths.find_path(
				unit,
				path_loc,
				{
					max_cost = max_cost,
					ignore_units = ignore_units,
					ignore_teleport = ignore_teleport,
					ignore_visibility = ignore_visibility
				} )
			local sub_turns

			if sub_cost == 0 then
				sub_turns = 0
			else
				sub_turns = math.ceil( ( ( sub_cost - unit.moves ) / unit.max_moves ) + 1 )
			end

			wml.variables[string.format( "%s.step[%d]", variable, index - 1 )] =
				{  -- this structure takes less space in the inspection window
					x = path_loc[1], y = path_loc[2],
					terrain = wesnoth.current.map[path_loc],
					movement_cost = sub_cost,
					required_turns = sub_turns
				}
		end
	end
end
