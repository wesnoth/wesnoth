local helper = wesnoth.require "helper"
local utils = wesnoth.require "wml-utils"

function wesnoth.wml_actions.find_path(cfg)
	local filter_unit = helper.get_child(cfg, "traveler") or helper.wml_error("[find_path] missing required [traveler] tag")
	-- only the first unit matching
	local unit = wesnoth.get_units(filter_unit)[1] or helper.wml_error("[find_path]'s filter didn't match any unit")
	local filter_location = helper.get_child(cfg, "destination") or helper.wml_error( "[find_path] missing required [destination] tag" )
	-- support for $this_unit
	local this_unit = utils.start_var_scope("this_unit")

	wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
	wesnoth.set_variable("this_unit", unit.__cfg) -- cfg field needed

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
	local viewing_side

	if not cfg.check_visibility then viewing_side = 0 end -- if check_visiblity then shroud is taken in account

	local locations = wesnoth.get_locations(filter_location) -- only the location with the lowest distance and lowest movement cost will match. If there will still be more than 1, only the 1st maching one.
	local max_cost = nil
	if not allow_multiple_turns then max_cost = unit.moves end --to avoid wrong calculation on already moved units
	local current_distance, current_cost = math.huge, math.huge
	local current_location = {}

	local width,heigth,border = wesnoth.get_map_size() -- data for test below

	for index, location in ipairs(locations) do
		-- we test if location passed to pathfinder is invalid (border); if is, do nothing, do not return and continue the cycle
		if location[1] == 0 or location[1] == ( width + 1 ) or location[2] == 0 or location[2] == ( heigth + 1 ) then
		else
			local distance = helper.distance_between ( unit.x, unit.y, location[1], location[2] )
			-- if we pass an unreachable locations an high value will be returned
			local path, cost = wesnoth.find_path( unit, location[1], location[2], { max_cost = max_cost, ignore_units = ignore_units, ignore_teleport = ignore_teleport, viewing_side = viewing_side } )

			if ( distance < current_distance and cost <= current_cost ) or ( cost < current_cost and distance <= current_distance ) then -- to avoid changing the hex with one with less distance and more cost, or vice versa
				current_distance = distance
				current_cost = cost
				current_location = location
			end
		end
	end

	if #current_location == 0 then wesnoth.message("WML warning","[find_path]'s filter didn't match any location")
	else
		local path, cost = wesnoth.find_path( unit, current_location[1], current_location[2], { max_cost = max_cost, ignore_units = ignore_units, ignore_teleport = ignore_teleport, viewing_side = viewing_side } )
		local turns

		if cost == 0 then -- if location is the same, of course it doesn't cost any MP
			turns = 0
		else
			turns = math.ceil( ( ( cost - unit.moves ) / unit.max_moves ) + 1 )
		end

		if cost >= 42424242 then -- it's the high value returned for unwalkable or busy terrains
			wesnoth.set_variable ( string.format("%s", variable), { hexes = 0 } ) -- set only length, nil all other values
			-- support for $this_unit
			wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
			utils.end_var_scope("this_unit", this_unit)
		return end

		if not allow_multiple_turns and turns > 1 then -- location cannot be reached in one turn
			wesnoth.set_variable ( string.format("%s", variable), { hexes = 0 } )
			-- support for $this_unit
			wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
			utils.end_var_scope("this_unit", this_unit)
		return end -- skip the cycles below

		wesnoth.set_variable ( string.format( "%s", variable ), { hexes = current_distance, from_x = unit.x, from_y = unit.y, to_x = current_location[1], to_y = current_location[2], movement_cost = cost, required_turns = turns } )

		for index, path_loc in ipairs(path) do
			local sub_path, sub_cost = wesnoth.find_path( unit, path_loc[1], path_loc[2], { max_cost = max_cost, ignore_units = ignore_units, ignore_teleport = ignore_teleport, viewing_side = viewing_side } )
			local sub_turns

			if sub_cost == 0 then
				sub_turns = 0
			else
				sub_turns = math.ceil( ( ( sub_cost - unit.moves ) / unit.max_moves ) + 1 )
			end

			wesnoth.set_variable ( string.format( "%s.step[%d]", variable, index - 1 ), { x = path_loc[1], y = path_loc[2], terrain = wesnoth.get_terrain( path_loc[1], path_loc[2] ), movement_cost = sub_cost, required_turns = sub_turns } ) -- this structure takes less space in the inspection window
		end
	end

	-- support for $this_unit
	wesnoth.set_variable ( "this_unit" ) -- clearing this_unit
	utils.end_var_scope("this_unit", this_unit)
end