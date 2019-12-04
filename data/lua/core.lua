-- Note: This file is loaded automatically by the engine.

function gui.alert(title, msg)
	if not msg then
		msg = title;
		title = "";
	end
	gui.show_prompt(title, msg, "ok", true)
end

function gui.confirm(title, msg)
	if not msg then
		msg = title;
		title = "";
	end
	return gui.show_prompt(title, msg, "yes_no", true)
end

--[========[Additional string support functions]========]

function stringx.escaped_split(str, sep, esc)
	esc = esc or '\\'
	return stringx.split(str, sep, {escape = esc, strip_spaces = true, remove_empty = true})
end

function stringx.quoted_split(str, sep, left, right)
	right = right or left
	if left == nil and right == nil then
		left = '('
		right = ')'
	end
	return stringx.split(str, sep, {quote_left = left, quote_right = right, strip_spaces = true, remove_empty = true})
end

function stringx.anim_split(str, sep)
	return stringx.split(str, sep, {expand_anim = true, strip_spaces = true, remove_empty = true});
end

function stringx.iter_range(str)
	return coroutine.wrap(function()
		local lo, hi = stringx.parse_range(str)
		for i = lo, hi do
			coroutine.yield(i)
		end
	end)
end

function stringx.iter_ranges(str)
	return coroutine.wrap(function()
		local split = str:split()
		for _,s in ipairs(split) do
			local lo, hi = s:parse_range()
			for i = lo, hi do
				coroutine.yield(i)
			end
		end
	end)
end

--[========[Config Manipulation Functions]========]

local function ensure_config(cfg)
	if type(cfg) == 'table' then
		return wml.valid(cfg)
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
--! Returns nil if no matching subtag is found
function wml.get_child(cfg, name, id)
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
--! Returns nil if no matching subtag is found
function wml.get_nth_child(cfg, name, n)
	ensure_config(cfg)
	for i,v in ipairs(cfg) do
		if v[1] == name then
			n = n - 1
			if n == 0 then return v[2], i end
		end
	end
end

--! Returns the first subtag of @a cfg with the given @a name that matches the @a filter.
--! If @a name is omitted, any subtag can match regardless of its name.
--! The function also returns the index of the subtag in the array.
--! Returns nil if no matching subtag is found
function wml.find_child(cfg, name, filter)
	ensure_config(cfg)
	if filter == nil then
		filter = name
		name = nil
	end
	for i,v in ipairs(cfg) do
		if name == nil or v[1] == name then
			local w = v[2]
			if wml.matches_filter(w, filter) then return w, i end
		end
	end
end

--! Returns the number of attributes of the config
function wml.attribute_count(cfg)
	ensure_config(cfg)
	local count = 0
	for k,v in pairs(cfg) do
		if type(k) == 'string' then
			count = count + 1
		end
	end
	return count
end

--! Returns the number of subtags of @a with the given @a name.
function wml.child_count(cfg, name)
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
function wml.child_range(cfg, tag)
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
function wml.child_array(cfg, tag)
	ensure_config(cfg)
	local result = {}
	for val in wml.child_range(cfg, tag) do
		table.insert(result, val)
	end
	return result
end

--! Removes the first matching child tag from @a cfg
function wml.remove_child(cfg, tag)
	ensure_config(cfg)
	for i,v in ipairs(cfg) do
		if v[1] == tag then
			table.remove(cfg, i)
			return
		end
	end
end

--! Removes all matching child tags from @a cfg
function wml.remove_children(cfg, ...)
	for i = #cfg, 1, -1 do
		for _,v in ipairs(...) do
			if cfg[i] == v then
				table.remove(cfg, i)
			end
		end
	end
end

--[========[WML Tag Creation Table]========]

local create_tag_mt = {
	__metatable = "WML tag builder",
	__index = function(self, n)
		return function(cfg) return { n, cfg } end
	end
}

wml.tag = setmetatable({}, create_tag_mt)

--[========[Config / Vconfig Unified Handling]========]

-- These are slated to be moved to game kernel only

function wml.literal(cfg)
	if type(cfg) == "userdata" then
		return cfg.__literal
	else
		return cfg or {}
	end
end

function wml.parsed(cfg)
	if type(cfg) == "userdata" then
		return cfg.__parsed
	else
		return cfg or {}
	end
end

function wml.shallow_literal(cfg)
	if type(cfg) == "userdata" then
		return cfg.__shallow_literal
	else
		return cfg or {}
	end
end

function wml.shallow_parsed(cfg)
	if type(cfg) == "userdata" then
		return cfg.__shallow_parsed
	else
		return cfg or {}
	end
end

--[========[Deprecation Helpers]========]

-- Need a textdomain for translatable deprecation strings.
local _ = wesnoth.textdomain "wesnoth"

-- Marks a function or subtable as deprecated.
-- Parameters:
---- elem_name: the full name of the element being deprecated (including the module)
---- replacement: the name of the element that will replace it (including the module)
---- level: deprecation level (1-4)
---- version: the version at which the element may be removed (level 2 or 3 only)
---- Set to nil if deprecation level is 1 or 4
---- elem: The actual element being deprecated
---- detail_msg: An optional message to add to the deprecation message
function wesnoth.deprecate_api(elem_name, replacement, level, version, elem, detail_msg)
	local message = detail_msg or ''
	if replacement then
		message = message .. " " .. (_"(Note: You should use $replacement instead in new code)"):format{replacement = replacement}
	end
	if type(level) ~= "number" or level < 1 or level > 4 then
		local err_params = {level = level}
		-- Note: This message is duplicated in src/deprecation.cpp
		-- Any changes should be mirrorred there.
		error((_"Invalid deprecation level $level (should be 1-4)"):format(err_params))
	end
	local msg_shown = false
	if type(elem) == "function" or getmetatable(elem) == "function" then
		return function(...)
			if not msg_shown then
				msg_shown = true
				wesnoth.deprecated_message(elem_name, level, version, message)
			end
			return elem(...)
		end
	elseif type(elem) == "table" then
		-- Don't clobber the old metatable.
		local old_mt = getmetatable(elem) or {}
		if type(old_mt) ~= "table" then
			-- See https://github.com/wesnoth/wesnoth/issues/4584#issuecomment-555788446
			wesnoth.log('warn', "Attempted to deprecate a table with a masked metatable: " ..
				elem_name .. " -> " .. replacement .. ", where getmetatable(" .. elem_name .. ") = " .. tostring(old_mt))
			return elem
		end
		local mt = {}
		for k,v in pairs(old_mt) do
			mt[k] = old_mt[v]
		end
		mt.__index = function(self, key)
			if not msg_shown then
				msg_shown = true
				wesnoth.deprecated_message(elem_name, level, version, message)
			end
			local val = elem[key]
			if val then return val end
			if type(old_mt) == "table" then
				if type(old_mt.__index) == 'function' then
					return old_mt.__index(self, key)
				elseif type(old_mt.__index) == 'table' then
					return old_mt.__index[key]
				else
					-- As of 2019, __index must be either a function or a table. If you ever run into this error,
					-- add an elseif branch for wrapping old_mt.__index appropriately. 
					wml.error('The wrapped __index metamethod of a deprecated object is neither a function nor a table')
				end
			end
		end
		mt.__newindex = function(self, key, val)
			if not msg_shown then
				msg_shown = true
				wesnoth.deprecated_message(elem_name, level, version, message)
			end
			if type(old_mt) == "table" and old_mt.__newindex ~= nil then
				return old_mt.__newindex(self, key, val)
			end
			elem[key] = val
		end
		return setmetatable({}, mt)
	else
		wesnoth.log('warn', "Attempted to deprecate something that is not a table or function: " ..
			elem_name .. " -> " .. replacement .. ", where " .. elem_name .. " = " .. tostring(elem))
	end
	return elem
end

if wesnoth.kernel_type() == "Game Lua Kernel" then
	--[========[WML error helper]========]

	--! Interrupts the current execution and displays a chat message that looks like a WML error.
	function wml.error(m)
		error("~wml:" .. m, 0)
	end

	--[========[Basic variable access]========]

	-- Get all variables via wml.all_variables (read-only)
	local get_all_vars_local = wesnoth.get_all_vars
	setmetatable(wml, {
		__metatable = "WML module",
		__index = function(self, key)
			if key == 'all_variables' then
				return get_all_vars_local()
			end
			return rawget(self, key)
		end,
		__newindex = function(self, key, value)
			if key == 'all_variables' then
				error("all_variables is read-only")
				-- TODO Implement writing?
			end
			rawset(self, key, value)
		end
	})

	-- So that definition of wml.variables does not cause deprecation warnings:
	local get_variable_local = wesnoth.get_variable
	local set_variable_local = wesnoth.set_variable

	-- Get and set variables via wml.variables[variable_path]
	wml.variables = setmetatable({}, {
		__metatable = "WML variables",
		__index = function(_, key)
			return get_variable_local(key)
		end,
		__newindex = function(_, key, value)
			set_variable_local(key, value)
		end
	})

	--[========[Variable Proxy Table]========]

	local variable_mt = {
		__metatable = "WML variable proxy"
	}

	local function get_variable_proxy(k)
		local v = wml.variables[k]
		if type(v) == "table" then
			v = setmetatable({ __varname = k }, variable_mt)
		end
		return v
	end

	local function set_variable_proxy(k, v)
		if getmetatable(v) == "WML variable proxy" then
			v = wml.variables[v.__varname]
		end
		wml.variables[k] = v
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
		__metatable = "WML variables proxy",
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

	wml.variables_proxy = setmetatable({}, root_variable_mt)

	--[========[Variable Array Access]========]

	local function resolve_variable_context(ctx, err_hint)
		if ctx == nil then
			return {get = get_variable_local, set = set_variable_local}
		elseif type(ctx) == 'number' and ctx > 0 and ctx <= #wesnoth.sides then
			return resolve_variable_context(wesnoth.sides[ctx])
		elseif type(ctx) == 'string' then
			-- TODO: Treat it as a namespace for a global (persistent) variable
			-- (Need Lua API for accessing them first, though.)
		elseif getmetatable(ctx) == "unit" then
			return {
				get = function(path) return ctx.variables[path] end,
				set = function(path, val) ctx.variables[path] = val end,
			}
		elseif getmetatable(ctx) == "side" then
			return {
				get = function(path) return wesnoth.get_side_variable(ctx.side, path) end,
				set = function(path, val) wesnoth.set_side_variable(ctx.side, path, val) end,
			}
		elseif getmetatable(ctx) == "unit variables" or getmetatable(ctx) == "side variables" then
			return {
				get = function(path) return ctx[path] end,
				set = function(path, val) ctx[path] = val end,
			}
		end
		error(string.format("Invalid context for %s: expected nil, side, or unit", err_hint), 3)
	end

	wml.array_access = {}

	--! Fetches all the WML container variables with name @a var.
	--! @returns a table containing all the variables (starting at index 1).
	function wml.array_access.get(var, context)
		context = resolve_variable_context(context, "get_variable_array")
		local result = {}
		for i = 1, context.get(var .. ".length") do
			result[i] = context.get(string.format("%s[%d]", var, i - 1))
		end
		return result
	end

	--! Puts all the elements of table @a t inside a WML container with name @a var.
	function wml.array_access.set(var, t, context)
		context = resolve_variable_context(context, "set_variable_array")
		context.set(var)
		for i, v in ipairs(t) do
			context.set(string.format("%s[%d]", var, i - 1), v)
		end
	end

	--! Creates proxies for all the WML container variables with name @a var.
	--! This is similar to wml.array_access.get, except that the elements
	--! can be used for writing too.
	--! @returns a table containing all the variable proxies (starting at index 1).
	function wml.array_access.get_proxy(var)
		local result = {}
		for i = 1, wml.variables[var .. ".length"] do
			result[i] = get_variable_proxy(string.format("%s[%d]", var, i - 1))
		end
		return result
	end

	-- More convenient when accessing global variables
	wml.array_variables = setmetatable({}, {
		__metatable = "WML variables",
		__index = function(_, key)
			return wml.array_access.get(key)
		end,
		__newindex = function(_, key, value)
			wml.array_access.set(key, value)
		end
	})

	wesnoth.persistent_tags = setmetatable({}, {
		-- This just makes assignment of the read/write funtions more convenient
		__index = function(t,k)
			rawset(t,k,{})
			return t[k]
		end
	})

	-- Note: We don't save the old on_load and on_save here.
	-- It's not necessary because we know this will be the first one registered.
	function wesnoth.game_events.on_load(cfg)
		local warned_tags = {}
		for i = 1, #cfg do
			local name = cfg[i][1]
			-- Use rawget so as not to trigger the auto-adding mechanism
			local tag = rawget(wesnoth.persistent_tags, name)
			if type(tag) == 'table' and type(tag.read) == 'function' then
				tag.read(cfg[i][2])
			elseif tag ~= nil and not warned_tags[name] then
				error(string.format("Invalid persistent tag [%s], should be a table containing read and write functions.", name))
				warned_tags[name] = true
			else
				error(string.format("[%s] not supported at scenario toplevel", name))
				warned_tags[name] = true
			end
		end
	end

	function wesnoth.game_events.on_save()
		local data_to_save = {}
		for name, tag in pairs(wesnoth.persistent_tags) do
			if type(tag) == 'table' and type(tag.write) == 'function' then
				local function add(data)
					table.insert(data_to_save, wml.tag[name](data))
				end
				tag.write(add)
			end
		end
		return data_to_save
	end

	--[========[Game Interface Control]========]

	wesnoth.interface.select_unit = wesnoth.units.select

	--! Fakes the move of a unit satisfying the given @a filter to position @a x, @a y.
	--! @note Usable only during WML actions.
	function wesnoth.interface.move_unit_fake(filter, to_x, to_y)
		local moving_unit = wesnoth.units.find_on_map(filter)[1]
		local from_x, from_y = moving_unit.x, moving_unit.y

		wesnoth.interface.scroll_to_hex(from_x, from_y)
		to_x, to_y = wesnoth.find_vacant_tile(x, y, moving_unit)

		if to_x < from_x then
			moving_unit.facing = "sw"
		elseif to_x > from_x then
			moving_unit.facing = "se"
		end
		moving_unit:extract()

		wml_actions.move_unit_fake{
			type      = moving_unit.type,
			gender    = moving_unit.gender,
			variation = moving.variation,
			side      = moving_unit.side,
			x         = from_x .. ',' .. to_x,
			y         = from_y .. ',' .. to_y
		}

		moving_unit:to_map(to_x, to_y)
		wml_actions.redraw{}
	end

	--[========[Units module]========]
	
	wesnoth.units.scroll_to = wesnoth.interface.scroll_to_hex

	--! Modifies all the units satisfying the given @a filter.
	--! @param vars key/value pairs that need changing.
	--! @note Usable only during WML actions.
	function wesnoth.units.modify(filter, vars)
		local units = wesnoth.units.find(filter)
		for u in pairs(units) do
			for k, v in pairs(vars) do
				-- Minor TODO: What if you want to change values of subtags?
				-- Previously would've been possible with eg {['variables.some_var'] = 'some_value'}
				-- With this implementation, it's not possible.
				u[k] = v
			end
		end
	end

	function wesnoth.units.find_attack(unit, filter)
		for i, atk in ipairs(unit.attacks) do
			if atk:matches(filter) then return atk end
		end
	end

	-- gets map and recalllist units.
	function wesnoth.units.find(filter)
		local res = wesnoth.units.find_on_map(filter)
		for i, u in ipairs(wesnoth.units.find_on_recall(filter)) do
			table.insert(res, u)
		end
		return res
	end
	
	function wesnoth.units.chance_to_be_hit(...)
		return 100 - wesnoth.units.defense_on(...)
	end
	
	-- Deprecated function
	function wesnoth.units.resistance(...)
		return 100 - wesnoth.units.resistance_against(...)
	end

	--[========[Sides module]========]
	
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
else
	--[========[Backwards compatibility for wml.tovconfig]========]
	local fake_vconfig_mt = {
		__index = function(self, key)
			if key == '__literal' or key == '__parsed' or key == '__shallow_literal' or key == '__shallow_parsed' then
				return self
			end
			return self[key]
		end
	}
	
	local function tovconfig_fake(cfg)
		ensure_config(cfg)
		return setmetatable(cfg, fake_vconfig_mt)
	end
	
	wesnoth.tovconfig = wesnoth.deprecate_api('wesnoth.tovconfig', 'wml.valid or wml.interpolate', 1, null, tovconfig_fake, 'tovconfig is now deprecated in plugin or map generation contexts; if you need to check whether a table is valid as a WML object, use wml.valid instead, and use wml.interpolate if you need to substitute variables into a WML object.')
	wml.tovconfig = wesnoth.deprecate_api('wml.tovconfig', 'wml.valid', 1, null, tovconfig_fake, 'tovconfig is now deprecated in plugin or map generation contexts; if you need to check whether a table is valid as a WML object, use wml.valid instead.')
	wml.literal = wesnoth.deprecate_api('wml.literal', '(no replacement)', 1, null, wml.literal, 'Since vconfigs are not supported outside of the game kernel, this function is redundant and will be removed from plugin and map generation contexts. It will continue to work in the game kernel.')
	wml.parsed = wesnoth.deprecate_api('wml.parsed', '(no replacement)', 1, null, wml.parsed, 'Since vconfigs are not supported outside of the game kernel, this function is redundant and will be removed from plugin and map generation contexts. It will continue to work in the game kernel.')
	wml.shallow_literal = wesnoth.deprecate_api('wml.shallow_literal', '(no replacement)', 1, null, wml.shallow_literal, 'Since vconfigs are not supported outside of the game kernel, this function is redundant and will be removed from plugin and map generation contexts. It will continue to work in the game kernel.')
	wml.shallow_parsed = wesnoth.deprecate_api('wml.shallow_parsed', '(no replacement)', 1, null, wml.shallow_parsed, 'Since vconfigs are not supported outside of the game kernel, this function is redundant and will be removed from plugin and map generation contexts. It will continue to work in the game kernel.')
end

--[========[GUI2 Dialog Manipulations]========]

--! Displays a WML message box with attributes from table @attr and options
--! from table @options.
--! @return the index of the selected option.
--! @code
--! local result = gui.get_user_choice({ speaker = "narrator" },
--!     { "Choice 1", "Choice 2" })
--! @endcode
function gui.get_user_choice(attr, options)
	local result = 0
	function gui.__user_choice_helper(i)
		result = i
	end
	local msg = {}
	for k,v in pairs(attr) do
		msg[k] = attr[k]
	end
	for k,v in ipairs(options) do
		table.insert(msg, wml.tag.option, { message = v,
			wml.tag.command, { wml.tag.lua, {
				code = string.format("gui.__user_choice_helper(%d)", k)
			}}})
	end
	wml_actions.message(msg)
	gui.__user_choice_helper = nil
	return result
end

-- Some C++ functions are deprecated; apply the messages here.
-- Note: It must happen AFTER the C++ functions are reassigned above to their new location.
-- These deprecated functions will probably never be removed.
wesnoth.wml_matches_filter = wesnoth.deprecate_api('wesnoth.wml_matches_filter', 'wml.matches_filter', 1, nil, wml.matches_filter)
if wesnoth.kernel_type() == "Game Lua Kernel" then
	wesnoth.get_variable = wesnoth.deprecate_api('wesnoth.get_variable', 'wml.variables', 1, nil, wesnoth.get_variable)
	wesnoth.set_variable = wesnoth.deprecate_api('wesnoth.set_variable', 'wml.variables', 1, nil, wesnoth.set_variable)
	wesnoth.get_all_vars = wesnoth.deprecate_api('wesnoth.get_all_vars', 'wml.all_variables', 1, nil, wesnoth.get_all_vars)
	-- Interface module
	wesnoth.delay = wesnoth.deprecate_api('wesnoth.delay', 'wesnoth.interface.delay', 1, nil, wesnoth.interface.delay)
	wesnoth.float_label = wesnoth.deprecate_api('wesnoth.float_label', 'wesnoth.interface.float_label', 1, nil, wesnoth.interface.float_label)
	wesnoth.highlight_hex = wesnoth.deprecate_api('wesnoth.highlight_hex', 'wesnoth.interface.highlight_hex', 1, nil, wesnoth.interface.highlight_hex)
	wesnoth.deselect_hex = wesnoth.deprecate_api('wesnoth.deselect_hex', 'wesnoth.interface.deselect_hex', 1, nil, wesnoth.interface.deselect_hex)
	wesnoth.get_selected_tile = wesnoth.deprecate_api('wesnoth.get_selected_tile', 'wesnoth.interface.get_selected_hex', 1, nil, wesnoth.interface.get_selected_hex)
	wesnoth.get_mouseover_tile = wesnoth.deprecate_api('wesnoth.get_mouseover_tile', 'wesnoth.interface.get_hovered_hex', 1, nil, wesnoth.interface.get_hovered_hex)
	wesnoth.scroll_to_tile = wesnoth.deprecate_api('wesnoth.scroll_to_tile', 'wesnot.interface.scroll_to_hex', 1, nil, wesnoth.interface.scroll_to_hex)
	wesnoth.scroll = wesnoth.deprecate_api('wesnoth.scroll', 'wesnot.interface.scroll', 1, nil, wesnoth.interface.scroll)
	wesnoth.lock_view = wesnoth.deprecate_api('wesnoth.lock_view', 'wesnoth.interface.lock', 1, nil, wesnoth.interface.lock)
	wesnoth.view_locked = wesnoth.deprecate_api('wesnoth.view_locked', 'wesnoth.interface.is_locked', 1, nil, wesnoth.interface.is_locked)
	wesnoth.is_skipping_messages = wesnoth.deprecate_api('wesnoth.is_skipping_messages', 'wesnoth.interface.is_skipping_messages', 1, nil, wesnoth.interface.is_skipping_messages)
	wesnoth.skip_messages = wesnoth.deprecate_api('wesnoth.skip_messages', 'wesnoth.interface.skip_messages', 1, nil, wesnoth.interface.skip_messages)
	wesnoth.add_tile_overlay = wesnoth.deprecate_api('wesnoth.add_tile_overlay', 'wesnoth.interface.add_hex_overlay', 1, nil, wesnoth.interface.add_hex_overlay)
	wesnoth.remove_tile_overlay = wesnoth.deprecate_api('wesnoth.remove_tile_overlay', 'wesnoth.interface.remove_hex_overlay', 1, nil, wesnoth.interface.remove_hex_overlay)
	wesnoth.theme_items = wesnoth.deprecate_api('wesnoth.theme_items', 'wesnoth.interface.game_display', 1, nil, wesnoth.interface.game_display)
	wesnoth.get_displayed_unit = wesnoth.deprecate_api('wesnoth.get_displayed_unit', 'wesnoth.interface.get_displayed_unit', 1, nil, wesnoth.interface.get_displayed_unit)
	wesnoth.zoom = wesnoth.deprecate_api('wesnoth.zoom', 'wesnoth.interface.zoom', 1, nil, wesnoth.interface.zoom)
	wesnoth.gamestate_inspector = wesnoth.deprecate_api('wesnoth.gamestate_inspector', 'gui.show_inspector', 1, nil, gui.show_inspector)
	-- No deprecation for these since since they're not actually public API yet
	wesnoth.color_adjust = wesnoth.interface.color_adjust
	wesnoth.set_menu_item = wesnoth.interface.set_menu_item
	wesnoth.clear_menu_item = wesnoth.interface.clear_menu_item 
	-- Units module
	wesnoth.match_unit = wesnoth.deprecate_api('wesnoth.match_unit', 'wesnoth.units.matches', 1, nil, wesnoth.units.matches)
	wesnoth.put_recall_unit = wesnoth.deprecate_api('wesnoth.put_recall_unit', 'wesnoth.units.to_recall', 1, nil, wesnoth.units.to_recall)
	wesnoth.put_unit = wesnoth.deprecate_api('wesnoth.put_unit', 'wesnoth.units.to_map', 1, nil, wesnoth.units.to_map)
	wesnoth.erase_unit = wesnoth.deprecate_api('wesnoth.erase_unit', 'wesnoth.units.erase', 1, nil, wesnoth.units.erase)
	wesnoth.copy_unit = wesnoth.deprecate_api('wesnoth.copy_unit', 'wesnoth.units.clone', 1, nil, wesnoth.units.clone)
	wesnoth.extract_unit = wesnoth.deprecate_api('wesnoth.extract_unit', 'wesnoth.units.extract', 1, nil, wesnoth.units.extract)
	wesnoth.advance_unit = wesnoth.deprecate_api('wesnoth.advance_unit', 'wesnoth.units.advance', 1, nil, wesnoth.units.advance)
	wesnoth.add_modification = wesnoth.deprecate_api('wesnoth.add_modification', 'wesnoth.units.add_modification', 1, nil, wesnoth.units.add_modification)
	wesnoth.remove_modifications = wesnoth.deprecate_api('wesnoth.remove_modifications', 'wesnoth.units.remove_modifications', 1, nil, wesnoth.units.remove_modifications)
	wesnoth.unit_resistance = wesnoth.deprecate_api('wesnoth.unit_resistance', 'wesnoth.units.resistance_against', 1, nil, wesnoth.units.resistance)
	wesnoth.unit_defense = wesnoth.deprecate_api('wesnoth.unit_defense', 'wesnoth.units.defense_on', 1, nil, wesnoth.units.chance_to_be_hit)
	wesnoth.unit_movement_cost = wesnoth.deprecate_api('wesnoth.unit_movement_cost', 'wesnoth.units.movement_on', 1, nil, wesnoth.units.movement_on)
	wesnoth.unit_vision_cost = wesnoth.deprecate_api('wesnoth.unit_vision_cost', 'wesnoth.units.vision_on', 1, nil, wesnoth.units.vision_on)
	wesnoth.unit_jamming_cost = wesnoth.deprecate_api('wesnoth.unit_jamming_cost', 'wesnoth.units.jamming_on', 1, nil, wesnoth.units.jamming_on)
	wesnoth.units.resistance = wesnoth.deprecate_api('wesnoth.units.resistance', 'wesnoth.units.resistance_against', 1, nil, wesnoth.units.resistance)
	wesnoth.units.defense = wesnoth.deprecate_api('wesnoth.units.defense', 'wesnoth.units.defense_on', 1, nil, wesnoth.units.chance_to_be_hit)
	wesnoth.units.movement = wesnoth.deprecate_api('wesnoth.units.movement', 'wesnoth.units.movement_on', 1, nil, wesnoth.units.movement_on)
	wesnoth.units.vision = wesnoth.deprecate_api('wesnoth.units.vision', 'wesnoth.units.vision_on', 1, nil, wesnoth.units.vision_on)
	wesnoth.units.jamming = wesnoth.deprecate_api('wesnoth.units.jamming', 'wesnoth.units.jamming_on', 1, nil, wesnoth.units.jamming_on)
	wesnoth.unit_ability = wesnoth.deprecate_api('wesnoth.unit_ability', 'wesnoth.units.ability', 1, nil, wesnoth.units.ability)
	wesnoth.transform_unit = wesnoth.deprecate_api('wesnoth.transform_unit', 'wesnoth.units.transform', 1, nil, wesnoth.units.transform)
	wesnoth.select_unit = wesnoth.deprecate_api('wesnoth.select_unit', 'wesnoth.units.select', 1, nil, wesnoth.units.select)
	wesnoth.create_unit = wesnoth.deprecate_api('wesnoth.create_unit', 'wesnoth.units.create', 1, nil, wesnoth.units.create)
	wesnoth.get_unit = wesnoth.deprecate_api('wesnoth.get_unit', 'wesnoth.units.get', 1, nil, wesnoth.units.get)
	wesnoth.get_units = wesnoth.deprecate_api('wesnoth.get_units', 'wesnoth.units.find_on_map', 1, nil, wesnoth.units.find_on_map)
	wesnoth.get_recall_units = wesnoth.deprecate_api('wesnoth.get_units', 'wesnoth.units.find_on_recall', 1, nil, wesnoth.units.find_on_recall)
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
end
wesnoth.tovconfig = wesnoth.deprecate_api('wesnoth.tovconfig', 'wml.tovconfig', 1, nil, wml.tovconfig)
wesnoth.debug = wesnoth.deprecate_api('wesnoth.debug', 'wml.tostring', 1, nil, wml.tostring)
-- GUI module
wesnoth.show_menu = wesnoth.deprecate_api('wesnoth.show_menu', 'gui.show_menu', 1, nil, gui.show_menu)
wesnoth.show_message_dialog = wesnoth.deprecate_api('wesnoth.show_message_dialog', 'gui.show_narration', 1, nil, gui.show_narration)
wesnoth.show_popup_dialog = wesnoth.deprecate_api('wesnoth.show_popup_dialog', 'gui.show_popup', 1, nil, gui.show_popup)
wesnoth.show_story = wesnoth.deprecate_api('wesnoth.show_story', 'gui.show_story', 1, nil, gui.show_story)
wesnoth.show_message_box = wesnoth.deprecate_api('wesnoth.show_message_box', 'gui.show_prompt', 1, nil, gui.show_prompt)
wesnoth.alert = wesnoth.deprecate_api('wesnoth.alert', 'gui.alert', 1, nil, gui.alert)
wesnoth.confirm = wesnoth.deprecate_api('wesnoth.confirm', 'gui.confirm', 1, nil, gui.confirm)
wesnoth.show_lua_console = wesnoth.deprecate_api('wesnoth.show_lua_console', 'gui.show_lua_console', 1, nil, gui.show_lua_console)
wesnoth.add_widget_definition = wesnoth.deprecate_api('wesnoth.add_widget_definition', 'gui.add_widget_definition', 1, nil, gui.add_widget_definition)
-- StringX module
wesnoth.format = wesnoth.deprecate_api('wesnoth.format', 'stringx.vformat', 1, nil, stringx.vformat)
wesnoth.format_conjunct_list = wesnoth.deprecate_api('wesnoth.format_conjunct_list', 'stringx.format_conjunct_list', 1, nil, stringx.format_conjunct_list)
wesnoth.format_disjunct_list = wesnoth.deprecate_api('wesnoth.format_disjunct_list', 'stringx.format_disjunct_list', 1, nil, stringx.format_disjunct_list)
