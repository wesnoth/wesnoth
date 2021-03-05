--[========[Sides module]========]

if wesnoth.kernel_type() == "Game Lua Kernel" then
	print("Loading sides module...")

	local sides_mt = {
		__metatable = "sides",
		__index = function(_, key)
			-- Only called if the key doesn't exist, so return nil if it's not a number
			if type(key) == 'number' then
				return wesnoth.sides.get(key)
			end
		end,
		__len = function(_)
			return #wesnoth.sides.find{}
		end
	}
	setmetatable(wesnoth.sides, sides_mt)

	-- Deprecated functions
	function wesnoth.set_side_variable(side, var, val)
		wesnoth.sides[side].variables[var] = val
	end
	function wesnoth.get_side_variable(side, var)
		return wesnoth.sides[side].variables[var]
	end
	function wesnoth.get_starting_location(side)
		local side = side
		if type(side) == 'number' then side = wesnoth.sides[side] end
		return side.starting_location
	end

	wesnoth.get_side_variable = wesnoth.deprecate_api('wesnoth.get_side_variable', 'wesnoth.sides[].variables', 1, nil, wesnoth.get_side_variable)
	wesnoth.set_side_variable = wesnoth.deprecate_api('wesnoth.set_side_variable', 'wesnoth.sides[].variables', 1, nil, wesnoth.set_side_variable)
	wesnoth.get_starting_location = wesnoth.deprecate_api('wesnoth.get_starting_location', 'wesnoth.sides[].starting_location', 1, nil, wesnoth.get_starting_location)
	wesnoth.is_enemy = wesnoth.deprecate_api('wesnoth.is_enemy', 'wesnoth.sides.is_enemy', 1, nil, wesnoth.sides.is_enemy)
	wesnoth.match_side = wesnoth.deprecate_api('wesnoth.match_side', 'wesnoth.sides.matches', 1, nil, wesnoth.sides.matches)
	wesnoth.set_side_id = wesnoth.deprecate_api('wesnoth.set_side_id', 'wesnoth.sides.set_id', 1, nil, wesnoth.sides.set_id)
	wesnoth.append_ai = wesnoth.deprecate_api('wesnoth.append_ai', 'wesnoth.sides.append_ai', 1, nil, wesnoth.sides.append_ai)
	wesnoth.debug_ai = wesnoth.deprecate_api('wesnoth.debug_ai', 'wesnoth.sides.debug_ai', 1, nil, wesnoth.sides.debug_ai)
	wesnoth.switch_ai = wesnoth.deprecate_api('wesnoth.switch_ai', 'wesnoth.sides.switch_ai', 1, nil, wesnoth.sides.switch_ai)
	wesnoth.add_ai_component = wesnoth.deprecate_api('wesnoth.add_ai_component', 'wesnoth.sides.add_ai_component', 1, nil, wesnoth.sides.add_ai_component)
	wesnoth.delete_ai_component = wesnoth.deprecate_api('wesnoth.delete_ai_component', 'wesnoth.sides.delete_ai_component', 1, nil, wesnoth.sides.delete_ai_component)
	wesnoth.change_ai_component = wesnoth.deprecate_api('wesnoth.change_ai_component', 'wesnoth.sides.change_ai_component', 1, nil, wesnoth.sides.change_ai_component)
	wesnoth.get_sides = wesnoth.deprecate_api('wesnoth.get_sides', 'wesnoth.sides.find', 1, nil, wesnoth.sides.find)
	wesnoth.create_side = wesnoth.deprecate_api('wesnoth.create_side', 'wesnoth.sides.create', 1, nil, wesnoth.sides.create)
	wesnoth.modify_ai = wesnoth.deprecate_api('wesnoth.modify_ai', 'wesnoth.sides.add|delete|change_ai_component', 1, nil, wesnoth.modify_ai)
end