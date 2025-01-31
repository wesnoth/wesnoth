--[========[Config Manipulation Functions]========]
---@diagnostic disable: deprecated
print("Loading WML module...")

local function ensure_config(cfg)
	if type(cfg) == 'table' then
		return wml.valid(cfg)
	end
	if type(cfg) == 'userdata' then
		if getmetatable(cfg) == 'wml object' then return true end
		if getmetatable(cfg) == 'mp settings' then return true end
		error("Expected a table or wml object but got " .. getmetatable(cfg), 3)
	else
		error("Expected a table or wml object but got " .. type(cfg), 3)
	end
	return false
end

-- Returns the first subtag of a WML table with the given name
-- If id is not nil, the "id" attribute of the subtag has to match too.
-- The function also returns the index of the subtag in the array.
-- Returns nil if no matching subtag is found
---@param cfg WML The WML table to search
---@param name string The tag name to search for
---@param id? string An optional id value to match
---@return WML? #The tag contents
---@return integer? #The index of the tag amongst _all_ child tags
function wml.get_child(cfg, name, id)
	ensure_config(cfg)
	for i,v in ipairs(cfg) do
		if v[1] == name then
			local w = v[2]
			if not id or w.id == id then return w, i end
		end
	end
end

-- Returns the nth subtag of a WML table with the given name.
-- (Indices start at 1, as always with Lua.)
-- The function also returns the index of the subtag in the array.
-- Returns nil if no matching subtag is found
---@param cfg WML The WML table to search
---@param name string The tag name to search for
---@param n integer The index of the child
---@return WML? #The tag contents
---@return integer? #The index of the tag amongst _all_ child tags
function wml.get_nth_child(cfg, name, n)
	ensure_config(cfg)
	for i,v in ipairs(cfg) do
		if v[1] == name then
			n = n - 1
			if n == 0 then return v[2], i end
		end
	end
end

-- Returns the first subtag of a WML table with the given name that matches the filter.
-- If the name is omitted, any subtag can match regardless of its name.
-- The function also returns the index of the subtag in the array.
-- Returns nil if no matching subtag is found
---@overload fun(cfg:WML, filter:WML)
---@param cfg WML The WML table to search
---@param name? string Tag to search for.
---@param filter WML A WML filter to match against
---@return WML? #The WML table of the child tag
---@return integer? #The overall index of the child tag
function wml.find_child(cfg, name, filter)
	ensure_config(cfg)
	if filter == nil and type(name) == 'table' then
		filter = name
		name = nil
	end
	for i,v in ipairs(cfg) do
		if name == nil or v.tag == name then
			if wml.matches_filter(v.contents, filter) then return v.contents, i end
		end
	end
end

-- Returns the number of attributes of the config
---@param cfg WML
---@return integer
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

-- Returns the number of subtags of a WML table with the given name.
---@param cfg WML
---@param name string
---@return integer
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

-- Returns an iterator over all the subtags of a WML table with the given name.
---@param cfg WML
---@param tag string
---@return fun(state:any):WML
---@return any state
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

-- Returns an array from the subtags of a WML table with the given name
---@param cfg WML
---@param tag string
---@return WML[]
function wml.child_array(cfg, tag)
	ensure_config(cfg)
	local result = {}
	for val in wml.child_range(cfg, tag) do
		table.insert(result, val)
	end
	return result
end

-- Removes the first matching child tag from a WML table
---@param cfg WML The WML table to modify
---@param tag string Name of the tag to remove
function wml.remove_child(cfg, tag)
	ensure_config(cfg)
	for i,v in ipairs(cfg) do
		if v[1] == tag then
			table.remove(cfg, i)
			return
		end
	end
end

-- Removes all matching child tags from a WML table
---@param cfg WML The WML table to modify
---@vararg string
function wml.remove_children(cfg, ...)
	for i = #cfg, 1, -1 do
		for j = 1, select('#', ...) do
			if cfg[i] and cfg[i][1] == select(j, ...) then
				table.remove(cfg, i)
			end
		end
	end
end

--[========[WML Tag Creation Table]========]

local create_tag_mt = {
	__metatable = "WML tag builder",
	__index = function(self, n)
		return function(cfg)
			return wesnoth.named_tuple({ n, cfg }, {"tag", "contents"})
		end
	end
}

---@type table<string, fun(cfg:WML):WMLTag>
wml.tag = setmetatable({}, create_tag_mt)

--[========[Config / Vconfig Unified Handling]========]

-- These are slated to be moved to game kernel only

---Returns the underlying WML table from a vconfig, without parsing.
---If passed a WML table, returns the same table unchanged.
---Returns an empty table if passed nil.
---@param cfg WML
---@return WMLTable
function wml.literal(cfg)
	if type(cfg) == "table" then
		return cfg or {}
	else
		return cfg.__literal
	end
end

---Returns a WML table from a vconfig, after parsing all variables.
---If passed a WML table, returns the same table unchanged.
---Returns an empty table if passed nil.
---@param cfg WML
---@return WMLTable
function wml.parsed(cfg)
	if type(cfg) == "table" then
		return cfg or {}
	else
		return cfg.__parsed
	end
end

---Returns WML table from a vconfig, without parsing, but with child tags still as vconfigs.
---If passed a WML table, returns the same table unchanged.
---Returns an empty table if passed nil.
---@param cfg WML
---@return WMLTable
function wml.shallow_literal(cfg)
	if type(cfg) == "table" then
		return cfg or {}
	else
		return cfg.__shallow_literal
	end
end

---Returns a WML table from a vconfig, after parsing all top-level variables and with child tags still as vconfigs.
---Top-level [insert_tag] will also be expanded.
---If passed a WML table, returns the same table unchanged.
---Returns an empty table if passed nil.
---@param cfg WML
---@return WMLTable
function wml.shallow_parsed(cfg)
	if type(cfg) == "table" then
		return cfg or {}
	else
		return cfg.__shallow_parsed
	end
end

if wesnoth.kernel_type() == "Game Lua Kernel" then
	--[========[WML error helper]========]

	-- Interrupts the current execution and displays a chat message that looks like a WML error.
	---@param m string The error message
	function wml.error(m)
		error("~wml:" .. m, 0)
	end

	--- Calling wml.fire isn't the same as calling wesnoth.wml_actions[name] due to the passed vconfig userdata
	--- which also provides "constness" of the passed wml object from the point of view of the caller.
	--- So please don't remove since it's not deprecated.
	---@type fun(name:string, cfg?:WML)
	---@type table<string, fun(cfg:WML)>
	wml.fire = setmetatable({}, {
		__metatable = "WML Actions",
		__call = function(_, name, cfg)
			wesnoth.wml_actions[name](wml.tovconfig(cfg or {}))
		end,
		__index = function(self, tag)
			return function(cfg) self(tag, cfg) end
		end,
		__newindex = function(_, tag)
			error('cannot assign to wml.fire.' .. tag)
		end,
	})
end

if wesnoth.kernel_type() ~= "Application Lua Kernel" then
	--[========[Basic variable access]========]

	-- Get all variables via wml.all_variables (read-only)
	local get_all_vars_local = wml.get_all_vars
	setmetatable(wml, {
		__metatable = "WML module",
		__dir = function(_, keys)
			table.insert(keys, 'all_variables')
			return keys
		end,
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

	local get_variable_local = wml.get_variable
	local set_variable_local = wml.set_variable or function()
		error("Variables are read-only during map generation", 3)
	end
	local function dir_vars(include_attributes)
		return function()
			local vars = get_all_vars_local()
			local keys = {}
			for k,v in pairs(vars) do
				if type(k) == 'number' then
					-- It's a tag; add the tag name
					table.insert(keys, v[1])
				elseif type(k) == 'string' and include_attributes then
					-- It's an attribute, add the key
					table.insert(keys, k)
				end
			end
			return keys
		end
	end

	-- Get and set variables via wml.variables[variable_path]
	---@type WMLVariableProxy
	wml.variables = setmetatable({}, {
		__metatable = "WML variables",
		__dir = dir_vars(true),
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
		__dir = dir_vars(true),
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

	---@type table<string, WMLVariableProxy>
	wml.variables_proxy = setmetatable({}, root_variable_mt)

	--[========[Variable Array Access]========]

	local function resolve_variable_context(ctx, err_hint)
		if ctx == nil then
			return {get = get_variable_local, set = set_variable_local}
		elseif type(ctx) == 'number' and ctx > 0 and ctx <= #wesnoth.sides then
			return resolve_variable_context(wesnoth.sides[ctx])
		elseif type(ctx) == 'string' then
			-- Treat it as a namespace for a global (persistent) variable
			return {
				get = function(path) return wesnoth.experimental.wml.global_vars[ctx][path] end,
				set = function(path, val) wesnoth.experimental.wml.global_vars[ctx][path] = val end,
			}
		elseif getmetatable(ctx) == "unit" then
			return {
				get = function(path) return ctx.variables[path] end,
				set = function(path, val) ctx.variables[path] = val end,
			}
		elseif getmetatable(ctx) == "side" then
			return {
				get = function(path) return wesnoth.sides[ctx.side].variables[path] end,
				set = function(path, val) wesnoth.sides[ctx.side].variables[path] = val end,
			}
		elseif getmetatable(ctx) == "unit variables" or getmetatable(ctx) == "side variables" then
			return {
				get = function(path) return ctx[path] end,
				set = function(path, val) ctx[path] = val end,
			}
		end
		-- TODO: Once the global variables API is no longer experimental, add it as a supported context type in this error message.
		error(string.format("Invalid context for %s: expected nil, side, or unit", err_hint), 3)
	end

	---@alias WMLVariableContext nil|number|string|unit|side|table<string, WMLTable>
	wml.array_access = {}

	-- Fetches all the WML container variables with name var.
	---@param var string Name of the variable to fetch
	---@param context? WMLVariableContext Where to fetch the variable from
	---@return WML[] #A table containing all the variables (starting at index 1).
	function wml.array_access.get(var, context)
		context = resolve_variable_context(context, "get_variable_array")
		local result = {}
		for i = 1, context.get(var .. ".length") do
			result[i] = context.get(string.format("%s[%d]", var, i - 1))
		end
		return result
	end

	-- Puts all the elements of the array inside a WML container with the given name.
	---@param var string Name of the variable to store
	---@param t WML[] An array of WML tables
	---@param context? WMLVariableContext Where to store the variable
	function wml.array_access.set(var, t, context)
		context = resolve_variable_context(context, "set_variable_array")
		context.set(var)
		for i, v in ipairs(t) do
			context.set(string.format("%s[%d]", var, i - 1), v)
		end
	end

	-- Creates proxies for all the WML container variables with name @a var.
	-- This is similar to wml.array_access.get, except that the elements
	-- can be used for writing too.
	---@param var string Name of the variable to fetch
	---@return WMLVariableProxy[] #A table containing all the variable proxies (starting at index 1).
	function wml.array_access.get_proxy(var)
		local result = {}
		for i = 1, wml.variables[var .. ".length"] do
			result[i] = get_variable_proxy(string.format("%s[%d]", var, i - 1))
		end
		return result
	end

	-- More convenient when accessing global variables
	---@type table<string, WML[]>
	wml.array_variables = setmetatable({}, {
		__metatable = "WML variables",
		__dir = dir_vars(false),
		__index = function(_, key)
			return wml.array_access.get(key)
		end,
		__newindex = function(_, key, value)
			wml.array_access.set(key, value)
		end
	})

	--[========[Global persistent variables]========]
	local ns_key, global_temp = '$ns$', "lua_global_variable"
	local global_vars_ns = {}
	local global_vars_mt = {
		__metatable = 'global variables',
		__index = function(self, namespace)
			local ns = setmetatable({
				__metatable = string.format('global variables[%s]', namespace)
			}, {
				__index = global_vars_ns
			})
			return setmetatable({[ns_key] = namespace}, ns)
		end
	}

	function global_vars_ns.__index(self, name)
		local U = wesnoth.require "wml-utils"
		local var <close> = U.scoped_var(global_temp)
		wesnoth.sync.run_unsynced(function()
			wesnoth.wml_actions.get_global_variable {
				namespace = self[ns_key],
				to_local = global_temp,
				from_global = name,
				immediate = true,
			}
		end)
		local res = var:get()
		if res == "" then
			return nil
		end
		return res
	end

	function global_vars_ns.__newindex(self, name, val)
		local U = wesnoth.require "wml-utils"
		local var <close> = U.scoped_var(global_temp)
		var:set(val)
		wesnoth.sync.run_unsynced(function()
			wesnoth.wml_actions.set_global_variable {
				namespace = self[ns_key],
				from_local = global_temp,
				to_global = name,
				immediate = true,
			}
		end)
	end

	-- Make sure wesnoth.experimental.wml actually exists
	-- It's done this way so it doesn't break if we later need to add things here from C++
	wesnoth.experimental = wesnoth.experimental or {}
	wesnoth.experimental.wml = wesnoth.experimental.wml or {}

	---@type table<string, table<string, WML>>
	wesnoth.experimental.wml.global_vars = setmetatable({}, global_vars_mt)
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

	wesnoth.tovconfig = wesnoth.deprecate_api('wesnoth.tovconfig', 'wml.valid or wml.interpolate', 1, nil, tovconfig_fake, 'tovconfig is now deprecated in plugin or map generation contexts; if you need to check whether a table is valid as a WML object, use wml.valid instead, and use wml.interpolate if you need to substitute variables into a WML object.')
	wml.tovconfig = wesnoth.deprecate_api('wml.tovconfig', 'wml.valid', 1, nil, tovconfig_fake, 'tovconfig is now deprecated in plugin or map generation contexts; if you need to check whether a table is valid as a WML object, use wml.valid instead.')
	wml.literal = wesnoth.deprecate_api('wml.literal', '(no replacement)', 1, nil, wml.literal, 'Since vconfigs are not supported outside of the game kernel, this function is redundant and will be removed from plugin and map generation contexts. It will continue to work in the game kernel.')
	wml.parsed = wesnoth.deprecate_api('wml.parsed', '(no replacement)', 1, nil, wml.parsed, 'Since vconfigs are not supported outside of the game kernel, this function is redundant and will be removed from plugin and map generation contexts. It will continue to work in the game kernel.')
	wml.shallow_literal = wesnoth.deprecate_api('wml.shallow_literal', '(no replacement)', 1, nil, wml.shallow_literal, 'Since vconfigs are not supported outside of the game kernel, this function is redundant and will be removed from plugin and map generation contexts. It will continue to work in the game kernel.')
	wml.shallow_parsed = wesnoth.deprecate_api('wml.shallow_parsed', '(no replacement)', 1, nil, wml.shallow_parsed, 'Since vconfigs are not supported outside of the game kernel, this function is redundant and will be removed from plugin and map generation contexts. It will continue to work in the game kernel.')
end

wesnoth.tovconfig = wesnoth.deprecate_api('wesnoth.tovconfig', 'wml.tovconfig', 1, nil, wml.tovconfig)
wesnoth.debug = wesnoth.deprecate_api('wesnoth.debug', 'wml.tostring', 1, nil, wml.tostring)
wesnoth.wml_matches_filter = wesnoth.deprecate_api('wesnoth.wml_matches_filter', 'wml.matches_filter', 1, nil, wml.matches_filter)

if wesnoth.kernel_type() ~= "Application Lua Kernel" then
	wesnoth.get_variable = wesnoth.deprecate_api('wesnoth.get_variable', 'wml.variables', 1, nil, wml.get_variable)
	wesnoth.get_all_vars = wesnoth.deprecate_api('wesnoth.get_all_vars', 'wml.all_variables', 1, nil, wml.get_all_vars)
end

if wesnoth.kernel_type() == "Game Lua Kernel" then
	wesnoth.set_variable = wesnoth.deprecate_api('wesnoth.set_variable', 'wml.variables', 1, nil, wml.set_variable)
	wesnoth.fire = wesnoth.deprecate_api('wesnoth.fire', 'wml.fire', 1, nil, function(name, cfg)
		wesnoth.wml_actions[name](wml.tovconfig(cfg or {}))
	end)
	wesnoth.eval_conditional = wesnoth.deprecate_api('wesnoth.eval_conditional', 'wml.eval_conditional', 1, nil, wml.eval_conditional)
end
