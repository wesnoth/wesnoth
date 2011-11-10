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

--! Returns the first subtag of @a cfg with the given @a name.
--! If @a id is not nil, the "id" attribute of the subtag has to match too.
--! The function also returns the index of the subtag in the array.
function helper.get_child(cfg, name, id)
	-- ipairs cannot be used on a vconfig object
	for i = 1, #cfg do
		local v = cfg[i]
		if v[1] == name then
			local w = v[2]
			if not id or w.id == id then return w, i end
		end
	end
end

--! Returns an iterator over all the subtags of @a cfg with the given @a name.
function helper.child_range(cfg, tag)
	local function f(s)
		local c
		repeat
			local i = s.i
			c = cfg[i]
			if not c then return end
			s.i = i + 1
		until c[1] == tag
		return c[2]
	end
	return f, { i = 1 }
end

--! Modifies all the units satisfying the given @a filter.
--! @param vars key/value pairs that need changing.
--! @note Usable only during WML actions.
function helper.modify_unit(filter, vars)
	wesnoth.fire("store_unit", {
		[1] = { "filter", filter },
		variable = "LUA_modify_unit",
		kill = true
	})
	for i = 0, wesnoth.get_variable("LUA_modify_unit.length") - 1 do
		local u = "LUA_modify_unit[" .. i .. "]"
		for k, v in pairs(vars) do
			wesnoth.set_variable(u .. '.' .. k, v)
		end
		wesnoth.fire("unstore_unit", {
			variable = u,
			find_vacant = false
		})
	end
	wesnoth.set_variable("LUA_modify_unit")
end

--! Fakes the move of a unit satisfying the given @a filter to position @a x, @a y.
--! @note Usable only during WML actions.
function helper.move_unit_fake(filter, to_x, to_y)
	wesnoth.fire("store_unit", {
		[1] = { "filter", filter },
		variable = "LUA_move_unit",
		kill = false
	})
	local from_x = wesnoth.get_variable("LUA_move_unit.x")
	local from_y = wesnoth.get_variable("LUA_move_unit.y")

	wesnoth.fire("scroll_to", { x=from_x, y=from_y })

	if to_x < from_x then
		wesnoth.set_variable("LUA_move_unit.facing", "sw")
	elseif to_x > from_x then
		wesnoth.set_variable("LUA_move_unit.facing", "se")
	end
	wesnoth.set_variable("LUA_move_unit.x", to_x)
	wesnoth.set_variable("LUA_move_unit.y", to_y)

	wesnoth.fire("kill", {
		x = from_x,
		y = from_y,
		animate = false,
		fire_event = false
	})

	wesnoth.fire("move_unit_fake", {
		type      = "$LUA_move_unit.type",
		gender    = "$LUA_move_unit.gender",
		variation = "$LUA_move_unit.variation",
		side      = "$LUA_move_unit.side",
		x         = from_x .. ',' .. to_x,
		y         = from_y .. ',' .. to_y
	})

	wesnoth.fire("unstore_unit", { variable="LUA_move_unit", find_vacant=true })
	wesnoth.fire("redraw")
	wesnoth.set_variable("LUA_move_unit")
end

local variable_mt = {}

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

--! Sets the metable of @a t so that it can be used to access WML variables.
--! @return @a t.
--! @code
--! helper.set_wml_var_metatable(_G)
--! my_persistent_variable = 42
--! @endcode
function helper.set_wml_var_metatable(t)
	return setmetatable(t, root_variable_mt)
end

local fire_action_mt = {
	__index = function(t, n)
		return function(cfg) wesnoth.fire(n, cfg) end
	end
}

--! Sets the metable of @a t so that it can be used to fire WML actions.
--! @return @a t.
--! @code
--! W = helper.set_wml_action_metatable {}
--! W.message { speaker = "narrator", message = "?" }
--! @endcode
function helper.set_wml_action_metatable(t)
	return setmetatable(t, fire_action_mt)
end

local create_tag_mt = {
	__index = function(t, n)
		return function(cfg) return { n, cfg } end
	end
}

--! Sets the metable of @a t so that it can be used to create subtags with less brackets.
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
	wesnoth.fire("message", msg)
	wesnoth.__user_choice_helper = nil
	return result
end

local function is_even(v) return v % 2 == 0 end

--! Returns the distance between two tiles given by their WML coordinates.
function helper.distance_between(x1, y1, x2, y2)
	local hdist = math.abs(x1 - x2)
	local vdist = math.abs(y1 - y2)
	if (y1 < y2 and not is_even(x1) and is_even(x2)) or
	   (y2 < y1 and not is_even(x2) and is_even(x1))
	then vdist = vdist + 1 end
	return math.max(hdist, vdist + math.floor(hdist / 2))
end

local adjacent_offset = {
	[false] = { {0,-1}, {1,-1}, {1,0}, {0,1}, {-1,0}, {-1,-1} },
	[true] = { {0,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0} }
}

--! Returns an iterator over adjacent locations that can be used in a for-in loop.
function helper.adjacent_tiles(x, y, with_borders)
	local x1,y1,x2,y2,b = 1,1,wesnoth.get_map_size()
	if with_borders then
		x1 = x1 - b
		y1 = y1 - b
		x2 = x2 + b
		y2 = y2 + b
	end
	local offset = adjacent_offset[is_even(x)]
	local i = 1
	return function()
		while i <= 6 do
			local o = offset[i]
			i = i + 1
			local u, v = o[1] + x, o[2] + y
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

function helper.rand (possible_values)
	wml_actions.set_variable({ name = "LUA_rand", rand = possible_values })
	local result = wesnoth.get_variable("LUA_rand")
	wesnoth.set_variable("LUA_rand")
	return result
end

function helper.deprecate(msg, f)
	return function(...)
		if msg then
			wesnoth.message("WML warning", msg)
			-- trigger the message only once
			msg = nil
		end
		return f(...)
	end
end

return helper
