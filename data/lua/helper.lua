--! #textdomain wesnoth

local helper = {}

local wml_actions = wesnoth.wml_actions

--! Returns an iterator over all the sides matching a given filter that can be used in a for-in loop.
function helper.get_sides(cfg)
	local function f(s)
		local i = s.i
		while i < #wesnoth.sides do
			i = i + 1
			if wesnoth.match_side(i, cfg) then
				s.i = i
				return wesnoth.sides[i], i
			end
		end
	end
	return f, { i = 0 }
end

--! Interrupts the current execution and displays a chat message that looks like a WML error.
function helper.wml_error(m)
	error("~wml:" .. m, 0)
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

--! Modifies all the units satisfying the given @a filter.
--! @param vars key/value pairs that need changing.
--! @note Usable only during WML actions.
function helper.modify_unit(filter, vars)
	wml_actions.store_unit({
		[1] = { "filter", filter },
		variable = "LUA_modify_unit",
		kill = true
	})
	for i = 0, wesnoth.get_variable("LUA_modify_unit.length") - 1 do
		local u = string.format("LUA_modify_unit[%d]", i)
		for k, v in pairs(vars) do
			wesnoth.set_variable(u .. '.' .. k, v)
		end
		wml_actions.unstore_unit({
			variable = u,
			find_vacant = false
		})
	end
	wesnoth.set_variable("LUA_modify_unit")
end

--! Fakes the move of a unit satisfying the given @a filter to position @a x, @a y.
--! @note Usable only during WML actions.
function helper.move_unit_fake(filter, to_x, to_y)
	wml_actions.store_unit({
		[1] = { "filter", filter },
		variable = "LUA_move_unit",
		kill = false
	})
	local from_x = wesnoth.get_variable("LUA_move_unit.x")
	local from_y = wesnoth.get_variable("LUA_move_unit.y")

	wml_actions.scroll_to({ x=from_x, y=from_y })

	if to_x < from_x then
		wesnoth.set_variable("LUA_move_unit.facing", "sw")
	elseif to_x > from_x then
		wesnoth.set_variable("LUA_move_unit.facing", "se")
	end
	wesnoth.set_variable("LUA_move_unit.x", to_x)
	wesnoth.set_variable("LUA_move_unit.y", to_y)

	wml_actions.kill({
		x = from_x,
		y = from_y,
		animate = false,
		fire_event = false
	})

	wml_actions.move_unit_fake({
		type      = "$LUA_move_unit.type",
		gender    = "$LUA_move_unit.gender",
		variation = "$LUA_move_unit.variation",
		side      = "$LUA_move_unit.side",
		x         = from_x .. ',' .. to_x,
		y         = from_y .. ',' .. to_y
	})

	wml_actions.unstore_unit({ variable="LUA_move_unit", find_vacant=true })
	wml_actions.redraw({})
	wesnoth.set_variable("LUA_move_unit")
end

-- Metatable that redirects access to wml.variable.proxy
local proxy_var_mt = {
	__metatable = "WML variables",
	__index    = function(t, k) return wml.variable.proxy[k] end,
	__newindex = function(t, k, v) wml.variable.proxy[k] = v end,
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

--! Displays a WML message box with attributes from table @attr and options
--! from table @options.
--! @return the index of the selected option.
--! @code
--! local result = helper.get_user_choice({ speaker = "narrator" },
--!     { "Choice 1", "Choice 2" })
--! @endcode
function helper.get_user_choice(attr, options)
	local result = 0
	function wesnoth.__user_choice_helper(i)
		result = i
	end
	local msg = {}
	for k,v in pairs(attr) do
		msg[k] = attr[k]
	end
	for k,v in ipairs(options) do
		table.insert(msg, { "option", { message = v,
			{ "command", { { "lua", {
				code = string.format("wesnoth.__user_choice_helper(%d)", k)
			}}}}}})
	end
	wml_actions.message(msg)
	wesnoth.__user_choice_helper = nil
	return result
end

local adjacent_offset = {
	[false] = { {0,-1}, {1,-1}, {1,0}, {0,1}, {-1,0}, {-1,-1} },
	[true] = { {0,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0} }
}

--! Returns an iterator over adjacent locations that can be used in a for-in loop.
-- Not deprecated because, unlike wesnoth.map.get_adjacent_tiles,
-- this verifies that the locations are on the map.
function helper.adjacent_tiles(x, y, with_borders)
	local x1,y1,x2,y2,b = 1,1,wesnoth.get_map_size()
	if with_borders then
		x1 = x1 - b
		y1 = y1 - b
		x2 = x2 + b
		y2 = y2 + b
	end
	local adj = {wesnoth.map.get_adjacent_tiles(x, y)}
	local i = 0
	return function()
		while i < #adj do
			i = i + 1
			local u, v = adj[i][1], adj[i][2]
			if u >= x1 and u <= x2 and v >= y1 and v <= y2 then
				return u, v
			end
		end
		return nil
	end
end

function helper.rand (possible_values, random_func)
	random_func = random_func or wesnoth.random
	assert(type(possible_values) == "table" or type(possible_values) == "string", string.format("helper.rand expects a string or table as parameter, got %s instead", type(possible_values)))

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

function helper.deprecate(msg, f)
	return function(...)
		if msg then
			if not message_shown and wesnoth.game_config.debug then
				wesnoth.message("warning", msg)
			end
			wesnoth.log("warn", msg)
			-- trigger the message only once
			msg = nil
		end
		return f(...)
	end
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

function helper.shuffle( t, random_func)
	random_func = random_func or wesnoth.random
	-- since tables are passed by reference, this is an in-place shuffle
	-- it uses the Fisher-Yates algorithm, also known as Knuth shuffle
	assert( type( t ) == "table", string.format( "helper.shuffle expects a table as parameter, got %s instead", type( t ) ) )
	local length = #t
	for index = length, 2, -1 do
		local random = random_func( 1, index )
		t[index], t[random] = t[random], t[index]
	end
end

-- Compatibility and deprecations

helper.distance_between = helper.deprecate(
	"helper.distance_between is deprecated; use wesnoth.map.distance_between instead",
	wesnoth.map.distance_between)
helper.get_child = helper.deprecate(
	"helper.get_child is deprecated; use wml.get_child instead", wml.get_child)
helper.get_nth_child = helper.deprecate(
	"helper.get_nth_child is deprecated; use wml.get_nth_child instead", wml.get_nth_child)
helper.child_count = helper.deprecate(
	"helper.child_count is deprecated; use wml.child_count instead", wml.child_count)
helper.child_range = helper.deprecate(
	"helper.child_range is deprecated; use wml.child_range instead", wml.child_range)
helper.child_array = helper.deprecate(
	"helper.child_array is deprecated; use wml.child_array instead", wml.child_array)
helper.get_variable_array = helper.deprecate(
	"helper.get_variable_array is deprecated; use wml.variable.get_array instead",
	wml.variable.get_array)
helper.set_variable_array = helper.deprecate(
	"helper.set_variable_array is deprecated; use wml.variable.set_array instead",
	wml.variable.set_array)
helper.get_variable_proxy_array = helper.deprecate(
	"helper.get_variable_proxy_array is deprecated; use wml.variable.get_proxy_array instead",
	wml.variable.get_proxy_array)
helper.literal = helper.deprecate(
	"helper.literal is deprecated; use wml.literal instead", wml.literal)
helper.parsed = helper.deprecate(
	"helper.parsed is deprecated; use wml.parsed instead", wml.parsed)
helper.shallow_literal = helper.deprecate(
	"helper.shallow_literal is deprecated; use wml.shallow_literal instead", wml.shallow_literal)
helper.shallow_parsed = helper.deprecate(
	"helper.shallow_parsed is deprecated; use wml.shallow_parsed instead", wml.shallow_parsed)
helper.set_wml_var_metatable = helper.deprecate(
	"helper.set_wml_var_metatable is deprecated; use wml.variable.proxy instead " ..
	"which has the metatable already set", helper.set_wml_var_metatable)
helper.set_wml_tag_metatable = helper.deprecate(
	"helper.set_wml_tag_metatable is deprecated; use wml.tag instead " ..
	"which has the metatable already set", helper.set_wml_tag_metatable)

wesnoth.get_variable = helper.deprecate(
	"wesnoth.get_variable is deprecated; use wml.variable.get instead", wesnoth.get_variable)
wesnoth.set_variable = helper.deprecate(
	"wesnoth.set_variable is deprecated; use wml.variable.set instead", wesnoth.set_variable)
wesnoth.get_all_vars = helper.deprecate(
	"wesnoth.get_all_vars is deprecated; use wml.variable.get_all instead", wesnoth.get_all_vars)
wesnoth.tovconfig = helper.deprecate(
	"wesnoth.tovconfig is deprecated; use wml.tovconfig instead", wesnoth.tovconfig)
wesnoth.debug = helper.deprecate(
	"wesnoth.debug is deprecated; use wml.tostring instead", wesnoth.debug)

return helper
