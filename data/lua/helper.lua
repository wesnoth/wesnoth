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
	local adj = {wesnoth.map.get_adjacent_tiles(x, y)}
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

function helper.rand (possible_values, random_func)
	random_func = random_func or wesnoth.random
	assert(type(possible_values) == "table" or type(possible_values) == "string",
		string.format("helper.rand expects a string or table as parameter, got %s instead",
		type(possible_values)))

	local items = {}
	local num_choices = 0

	if type(possible_values) == "string" then
		-- split on commas
		for word in possible_values:gmatch("[^,]+") do
			-- does the word contain two dots? If yes, that's a range
			local dots_start, dots_end = word:find("%.%.")
			if dots_start then
				-- split on the dots if so and cast as numbers
				local low = tonumber(word:sub(1, dots_start-1))
				local high = tonumber(word:sub(dots_end+1))
				-- perhaps someone passed a string as part of the range, intercept the issue
				if not (low and high) then
					wesnoth.message("Malformed range: " .. possible_values)
					table.insert(items, word)
					num_choices = num_choices + 1
				else
					if low > high then
						-- low is greater than high, swap them
						low, high = high, low
					end

					-- if both ends represent the same number, then just use that number
					if low == high then
						table.insert(items, low)
						num_choices = num_choices + 1
					else
						-- insert a table representing the range
						table.insert(items, {low, high})
						-- how many items does the range contain? Increase difference by 1 because we include both ends
						num_choices = num_choices + (high - low) + 1
					end
				end
			else
				-- handle as a string
				table.insert(items, word)
				num_choices = num_choices + 1
			end
		end
	else
		num_choices = #possible_values
		items = possible_values
		-- We need to parse ranges separately anyway
		for i, val in ipairs(possible_values) do
			if type(val) == "table" then
				assert(#val == 2 and type(val[1]) == "number" and type(val[2]) == "number", "Malformed range for helper.rand")
				if val[1] > val[2] then
					val = {val[2], val[1]}
				end
				num_choices = num_choices + (val[2] - val[1])
			end
		end
	end

	local idx = random_func(1, num_choices)

	for i, item in ipairs(items) do
		if type(item) == "table" then -- that's a range
			local elems = item[2] - item[1] + 1 -- amount of elements in the range, both ends included
			if elems >= idx then
				return item[1] + elems - idx
			else
				idx = idx - elems
			end
		else -- that's a single element
			idx = idx - 1
			if idx == 0 then
				return item
			end
		end
	end

	return nil
end

function helper.round( number )
	-- code converted from util.hpp, round_portable function
	-- round half away from zero method
	if number >= 0 then
		number = math.floor( number + 0.5 )
	else
		number = math.ceil ( number - 0.5 )
	end

	return number
end

function helper.shuffle( t, random_func )
	random_func = random_func or wesnoth.random
	-- since tables are passed by reference, this is an in-place shuffle
	-- it uses the Fisher-Yates algorithm, also known as Knuth shuffle
	assert(
		type( t ) == "table",
		string.format( "helper.shuffle expects a table as parameter, got %s instead", type( t ) ) )
	local length = #t
	for index = length, 2, -1 do
		local random = random_func( 1, index )
		t[index], t[random] = t[random], t[index]
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

return helper
