-- Ensure the package is shared between all its users.
-- TODO: consider using the "package" module instead, if this code structure gets common.
if wesnoth.package then
	local helper = wesnoth.package.helper
	if helper then
		return helper
	end
else
	wesnoth.package = {}
end

local helper = {}
wesnoth.package.helper = helper

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
		return function(...) wesnoth.fire(n, ...) end
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

return helper
