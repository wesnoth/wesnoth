--! #textdomain wesnoth

local helper = {}

local wml_actions = wesnoth.wml_actions

--! Returns an iterator over all the sides matching a given filter that can be used in a for-in loop.
function helper.get_sides(cfg)
	local function f(s)
		local i = s.i
		while i < #wesnoth.sides do
			i = i + 1
			if wesnoth.sides.matches(i, cfg) then
				s.i = i
				return wesnoth.sides[i], i
			end
		end
	end
	return f, { i = 0 }
end

--! Returns an iterator over teams that can be used in a for-in loop.
function helper.all_teams()
	local function f(s)
		local i = s.i
		local team = wesnoth.sides[i]
		s.i = i + 1
		return team
	end
	return f, { i = 1 }
end

-- Metatable that redirects access to wml.variables_proxy
local proxy_var_mt = {
	__metatable = "WML variables",
	__index    = function(t, k) return wml.get_variable_proxy(k) end,
	__newindex = function(t, k, v) wml.set_variable_proxy(k, v) end,
}

function helper.set_wml_var_metatable(t)
	return setmetatable(t, proxy_var_mt)
end

local fire_action_mt = {
	__metatable = "WML actions",
	__index = function(t, n)
		return function(cfg) wesnoth.fire(n, cfg) end
	end
}

--! Sets the metatable of @a t so that it can be used to fire WML actions.
--! @return @a t.
--! @code
--! W = helper.set_wml_action_metatable {}
--! W.message { speaker = "narrator", message = "?" }
--! @endcode
function helper.set_wml_action_metatable(t)
	return setmetatable(t, fire_action_mt)
end

-- Metatable that redirects to wml.tag
local proxy_tag_mt = {
	__metatable = "WML tag builder",
	__index = function(t, n) return wml.tag[n] end
}

function helper.set_wml_tag_metatable(t)
	return setmetatable(t, proxy_tag_mt)
end

--! Returns an iterator over adjacent locations that can be used in a for-in loop.
-- Not deprecated because, unlike wesnoth.map.get_adjacent_tiles,
-- this verifies that the locations are on the map.
function helper.adjacent_tiles(x, y, with_borders)
	local adj = {wesnoth.map.get_adjacent_hexes(x, y)}
	local i = 0
	return function()
		while i < #adj do
			i = i + 1
			local u, v = adj[i][1], adj[i][2]
			if wesnoth.current.map:on_board(u, v, with_borders) then
				return u, v
			end
		end
		return nil
	end
end

-- Compatibility and deprecations
helper.distance_between = wesnoth.deprecate_api('helper.distance_between', 'wesnoth.map.distance_between', 1, nil, wesnoth.map.distance_between)
helper.get_child = wesnoth.deprecate_api('helper.get_child', 'wml.get_child', 1, nil, wml.get_child)
helper.get_nth_child = wesnoth.deprecate_api('helper.get_nth_child', 'wml.get_nth_child', 1, nil, wml.get_nth_child)
helper.child_count = wesnoth.deprecate_api('helper.child_count', 'wml.child_count', 1, nil, wml.child_count)
helper.child_range = wesnoth.deprecate_api('helper.child_range', 'wml.child_range', 1, nil, wml.child_range)
helper.child_array = wesnoth.deprecate_api('helper.child_array', 'wml.child_array', 1, nil, wml.child_array)
if wesnoth.kernel_type() == "Game Lua Kernel" then
	helper.get_variable_array = wesnoth.deprecate_api('helper.get_variable_array', ' wml.array_access.get', 1, nil, wml.array_access.get)
	helper.set_variable_array = wesnoth.deprecate_api('helper.set_variable_array', 'wml.array_access.set', 1, nil, wml.array_access.set)
	helper.get_variable_proxy_array = wesnoth.deprecate_api('helper.get_variable_proxy_array', 'wml.array_access.get_proxy', 1, nil, wml.array_access.get_proxy)
	helper.wml_error = wesnoth.deprecate_api('helper.wml_error', 'wml.error', 1, nil, wml.error)
	helper.move_unit_fake = wesnoth.deprecate_api('helper.move_unit_fake', 'wesnoth.interface.move_unit_fake', 1, nil, wesnoth.interface.move_unit_fake)
	helper.modify_unit = wesnoth.deprecate_api('helper.modify_unit', 'wesnoth.units.modify', 1, nil, wesnoth.units.modify)
	helper.find_attack = wesnoth.deprecate_api('helper.find_attack', 'wesnoth.units.find_attack', 1, nil, wesnoth.units.find_attack)
end
helper.literal = wesnoth.deprecate_api('helper.literal', 'wml.literal', 1, nil, wml.literal)
helper.parsed = wesnoth.deprecate_api('helper.parsed', 'wml.parsed', 1, nil, wml.parsed)
helper.shallow_literal = wesnoth.deprecate_api('helper.shallow_literal', 'wml.shallow_literal', 1, nil, wml.shallow_literal)
helper.shallow_parsed = wesnoth.deprecate_api('helper.shallow_parsed', 'wml.shallow_parsed', 1, nil, wml.shallow_parsed)
helper.set_wml_var_metatable = wesnoth.deprecate_api('helper.set_wml_var_metatable', 'wml.variable.proxy', 2, nil, helper.set_wml_var_metatable)
helper.set_wml_tag_metatable = wesnoth.deprecate_api('helper.set_wml_tag_metatable', 'wml.tag', 2, nil, helper.set_wml_tag_metatable)
helper.get_user_choice = wesnoth.deprecate_api('helper.get_user_choice', 'gui.get_user_choice', 1, nil, gui.get_user_choice)
helper.rand = wesnoth.deprecate_api('helper.rand', 'mathx.random_choice', 1, nil, mathx.random_choice)
helper.round = wesnoth.deprecate_api('helper.round', 'mathx.round', 1, nil, mathx.round)
helper.shuffle = wesnoth.deprecate_api('helper.shuffle', 'mathx.shuffle', 1, nil, mathx.shuffle)

return helper
