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

local function ensure_config(cfg)
	if type(cfg) == 'table' then
		return true
	end
	if type(cfg) == 'userdata' then
		if getmetatable(cfg) == 'wml object' then return true end
		error("Expected a table or wml object but got " .. getmetatable(cfg), 3)
	else
		error("Expected a table or wml object but got " .. type(cfg), 3)
	end
	return false
end

--! Returns the first subtag of @a cfg with the given @a name.
--! If @a id is not nil, the "id" attribute of the subtag has to match too.
--! The function also returns the index of the subtag in the array.
function helper.get_child(cfg, name, id)
	ensure_config(cfg)
	for i,v in ipairs(cfg) do
		if v[1] == name then
			local w = v[2]
			if not id or w.id == id then return w, i end
		end
	end
end

--! Returns the nth subtag of @a cfg with the given @a name.
--! (Indices start at 1, as always with Lua.)
--! The function also returns the index of the subtag in the array.
function helper.get_nth_child(cfg, name, n)
	ensure_config(cfg)
	for i,v in ipairs(cfg) do
		if v[1] == name then
			n = n - 1
			if n == 0 then return v[2], i end
		end
	end
end

--! Returns the number of subtags of @a with the given @a name.
function helper.child_count(cfg, name)
	ensure_config(cfg)
	local n = 0
	for i,v in ipairs(cfg) do
		if v[1] == name then
			n = n + 1
		end
	end
	return n
end

--! Returns an iterator over all the subtags of @a cfg with the given @a name.
function helper.child_range(cfg, tag)
	ensure_config(cfg)
	local iter, state, i = ipairs(cfg)
	local function f(s)
		local c
		repeat
			i,c = iter(s,i)
			if not c then return end
		until c[1] == tag
		return c[2]
	end
	return f, state
end

--! Returns an array from the subtags of @a cfg with the given @a name
function helper.child_array(cfg, tag)
	ensure_config(cfg)
	local result = {}
	for val in helper.child_range(cfg, tag) do
		table.insert(result, val)
	end
	return result
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

local variable_mt = {
	__metatable = "WML variable proxy"
}

local function get_variable_proxy(k)
	local v = wesnoth.get_variable(k, true)
	if type(v) == "table" then
		v = setmetatable({ __varname = k }, variable_mt)
	end
	return v
end

local function set_variable_proxy(k, v)
	if getmetatable(v) == variable_mt then
		v = wesnoth.get_variable(v.__varname)
	end
	wesnoth.set_variable(k, v)
end

function variable_mt.__index(t, k)
	local i = tonumber(k)
	if i then
		k = t.__varname .. '[' .. i .. ']'
	else
		k = t.__varname .. '.' .. k
	end
	return get_variable_proxy(k)
end

function variable_mt.__newindex(t, k, v)
	local i = tonumber(k)
	if i then
		k = t.__varname .. '[' .. i .. ']'
	else
		k = t.__varname .. '.' .. k
	end
	set_variable_proxy(k, v)
end

local root_variable_mt = {
	__metatable = "WML variables",
	__index    = function(t, k)    return get_variable_proxy(k)    end,
	__newindex = function(t, k, v)
		if type(v) == "function" then
			-- User-friendliness when _G is overloaded early.
			-- FIXME: It should be disabled outside the "preload" event.
			rawset(t, k, v)
		else
			set_variable_proxy(k, v)
		end
	end
}

--! Sets the metatable of @a t so that it can be used to access WML variables.
--! @return @a t.
--! @code
--! helper.set_wml_var_metatable(_G)
--! my_persistent_variable = 42
--! @endcode
function helper.set_wml_var_metatable(t)
	return setmetatable(t, root_variable_mt)
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

local create_tag_mt = {
	__metatable = "WML tag builder",
	__index = function(t, n)
		return function(cfg) return { n, cfg } end
	end
}

--! Sets the metatable of @a t so that it can be used to create subtags with less brackets.
--! @return @a t.
--! @code
--! T = helper.set_wml_tag_metatable {}
--! W.event { name = "new turn", T.message { speaker = "narrator", message = "?" } }
--! @endcode
function helper.set_wml_tag_metatable(t)
	return setmetatable(t, create_tag_mt)
end

--! Fetches all the WML container variables with name @a var.
--! @returns a table containing all the variables (starting at index 1).
function helper.get_variable_array(var)
	local result = {}
	for i = 1, wesnoth.get_variable(var .. ".length") do
		result[i] = wesnoth.get_variable(string.format("%s[%d]", var, i - 1))
	end
	return result
end

--! Puts all the elements of table @a t inside a WML container with name @a var.
function helper.set_variable_array(var, t)
	wesnoth.set_variable(var)
	for i, v in ipairs(t) do
		wesnoth.set_variable(string.format("%s[%d]", var, i - 1), v)
	end
end

--! Creates proxies for all the WML container variables with name @a var.
--! This is similar to helper.get_variable_array, except that the elements
--! can be used for writing too.
--! @returns a table containing all the variable proxies (starting at index 1).
function helper.get_variable_proxy_array(var)
	local result = {}
	for i = 1, wesnoth.get_variable(var .. ".length") do
		result[i] = get_variable_proxy(string.format("%s[%d]", var, i - 1))
	end
	return result
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

--! Returns the distance between two tiles given by their WML coordinates.
function helper.distance_between(...)
	wesnoth.log("warn", "helper.distance_between is deprecated; use wesnoth.map.distance_between instead")
	return wesnoth.map.distance_between(...)
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

function helper.literal(cfg)
	if type(cfg) == "userdata" then
		return cfg.__literal
	else
		return cfg or {}
	end
end

function helper.parsed(cfg)
	if type(cfg) == "userdata" then
		return cfg.__parsed
	else
		return cfg or {}
	end
end

function helper.shallow_literal(cfg)
	if type(cfg) == "userdata" then
		return cfg.__shallow_literal
	else
		return cfg or {}
	end
end

function helper.shallow_parsed(cfg)
	if type(cfg) == "userdata" then
		return cfg.__shallow_parsed
	else
		return cfg or {}
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
			wesnoth.message("warning", msg)
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

return helper
