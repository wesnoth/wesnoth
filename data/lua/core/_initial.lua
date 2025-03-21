
print("Loading Lua core files...")

--[========[Deprecation Helpers]========]

-- Need a textdomain for translatable deprecation strings.
local _ = wesnoth.textdomain "wesnoth"

-- Marks a function or subtable as deprecated.
---@generic T
---@param elem_name string the full name of the element being deprecated (including the module), to be shown in the deprecation message
---@param replacement_name string the name of the element that will replace it (including the module), to be shown in the deprecation message
--- Can be nil if there is not replacement, though this should be an unlikely situation
---@param level 1|2|3|4 deprecation level (1-4)
---@param version string|nil the version at which the element may be removed (level 2 or 3 only)
--- Set to nil if deprecation level is 1 or 4
--- Will be shown in the deprecation message
---@param elem T Implementation of the compatibility layer, ignored if level is 4.
--- This can be the original, pre-deprecated element, but it does not have to be.
--- It could also be a wrapper that presents a different API, for example.
--- If deprecating a function, that would mean a wrapper function that calls the new API.
--- If deprecating a table, you would need to provide a table with __index and __newindex metamethods that call the new API.
--- This is the only argument that affects the functionality of the resulting deprecation wrapper.
---@param detail_msg? string An optional message to add to the deprecation message
---@return T elem_deprecated #A wrapper around the element, which triggers a deprecation message when used.
--- If it is a function, the message is triggered the first time it is called.
--- If it is a table, the message is triggered when a key is written or read on the table.
function wesnoth.deprecate_api(elem_name, replacement_name, level, version, elem, detail_msg)
	if wesnoth.game_config.strict_lua then return nil end
	local message = detail_msg or ''
	if replacement_name then
		message = message .. " " .. (_"(Note: You should use $replacement instead in new code)"):vformat{replacement = replacement_name}
	end
	if type(level) ~= "number" or level < 1 or level > 4 then
		local err_params = {level = level}
		-- Note: This message is duplicated in src/deprecation.cpp
		-- Any changes should be mirrored there.
		error((_"Invalid deprecation level $level (should be 1-4)"):vformat(err_params))
	end
	local msg_shown = false
	if level == 4 then
		local function show_msg(...)
			if not msg_shown then
				msg_shown = true
				wesnoth.deprecated_message(elem_name, level, version, message)
			end
		end
		return setmetatable({__deprecated = true}, {
			__index = show_msg,
			__newindex = show_msg,
			__call = show_msg,
			__metatable = "removed API",
		})
	elseif type(elem) == "function" or getmetatable(elem) == "function" then
		return setmetatable({__deprecated = true}, {
			__call = function(self, ...)
				if not msg_shown then
					msg_shown = true
					wesnoth.deprecated_message(elem_name, level, version, message)
				end
				return elem(...)
			end,
			__metatable = "function"
		})
	elseif type(elem) == "table" then
		-- Don't clobber the old metatable.
		local old_mt = getmetatable(elem) or {}
		if type(old_mt) ~= "table" then
			-- See https://github.com/wesnoth/wesnoth/issues/4584#issuecomment-555788446
			wesnoth.log('warn', "Attempted to deprecate a table with a masked metatable: " ..
				elem_name .. " -> " .. replacement_name .. ", where getmetatable(" .. elem_name .. ") = " .. tostring(old_mt))
			return elem
		end
		local mt = {}
		for k,v in pairs(old_mt) do
			mt[k] = old_mt[k]
		end
		mt.__index = function(self, key)
			if not msg_shown then
				msg_shown = true
				wesnoth.deprecated_message(elem_name, level, version, message)
			end
			return elem[key]
		end
		mt.__newindex = function(self, key, val)
			if not msg_shown then
				msg_shown = true
				wesnoth.deprecated_message(elem_name, level, version, message)
			end
			elem[key] = val
		end
		return setmetatable({__deprecated = true}, mt)
	else
		wesnoth.log('warn', "Attempted to deprecate something that is not a table or function: " ..
			elem_name .. " -> " .. replacement_name .. ", which is " .. tostring(elem))
	end
	return elem
end

function wesnoth.type(value)
	local t = type(value)
	if t == 'userdata' or t == 'table' then
		local m = getmetatable(value)
		if type(m) == 'string' then
			return m
		elseif type(m) == 'table' and type(m.__name) == 'string' then
			return m.__name
		end
	end
	return t
end

local function compare_versions(a, op, b)
	local V = wesnoth.version
	if op == '==' then
		return V(a) == V(b)
	elseif op == '!=' then
		return V(a) ~= V(b)
	elseif op == '<' then
		return V(a) < V(b)
	elseif op == '<=' then
		return V(a) <= V(b)
	elseif op == '>' then
		return V(a) > V(b)
	elseif op == '>=' then
		return V(a) >= V(b)
	else
		error(string.format('Invalid operator %s for comparing versions', op))
	end
end

unpack = wesnoth.deprecate_api('unpack', 'table.unpack', 3, '1.17', table.unpack)
math.pow = wesnoth.deprecate_api('math.pow', '^', 3, '1.17', function(a,b) return a ^ b end)
wesnoth.get_time_stamp = wesnoth.deprecate_api('wesnoth.get_time_stamp', 'wesnoth.ms_since_init', 1, nil, wesnoth.ms_since_init)
wesnoth.compare_versions = wesnoth.deprecate_api('wesnoth.compare_versions', 'wesnoth.version', 1, nil, compare_versions, 'Use wesnoth.version to construct a version object and compare using the normal Lua comparison operators')

wesnoth.set_next_scenario = wesnoth.deprecate_api('wesnoth.set_next_scenario', 'wesnoth.scenario.next', 1, nil, function(n)
	wesnoth.scenario.next = n
end)
wesnoth.set_end_campaign_credits = wesnoth.deprecate_api('wesnoth.set_end_campaign_credits', 'wesnoth.scenario.show_credits', 1, nil, function(b)
	wesnoth.scenario.show_credits = b
end)
wesnoth.set_end_campaign_text = wesnoth.deprecate_api('wesnoth.set_end_campaign_text', 'wesnoth.scenario.end_text, wesnoth.scenario.end_text_duration', 1, nil, function(t, d)
	wesnoth.scenario.end_text = t
	if type(d) == 'number' then
		wesnoth.scenario.end_text_duration = d
	end
end)

if wesnoth.kernel_type() ~= 'Application Lua Kernel' then
	wesnoth.find_path = wesnoth.deprecate_api('wesnoth.find_path', 'wesnoth.paths.find_path', 1, nil, wesnoth.paths.find_path)
end

if wesnoth.kernel_type() == 'Game Lua Kernel' then
	local function get_time_of_day(...)
		local arg_i, turn = 1, nil
		if type(...) == 'number' then
			turn = ...
			arg_i = arg_i + 1
		end
		local loc, n = wesnoth.map.read_location(select(arg_i, ...))
		local illum = false
		if loc ~= nil then
			local actual_loc = type(select(arg_i, ...))
			if type(actual_loc) == 'table' and type(actual_loc[3]) == 'boolean' then
				illum = actual_loc[3]
			end
			arg_i = arg_i + n
		end
		local final_arg = select(arg_i, ...)
		if type(final_arg) == 'boolean' then
			illum = final_arg
		end
		local get_tod
		if illum then
			get_tod = wesnoth.schedule.get_illumination
		else
			get_tod = wesnoth.schedule.get_time_of_day
		end
		return get_tod(loc, turn)
	end

	local function liminal_bonus(...)
		return wesnoth.current.schedule.liminal_bonus
	end

	wesnoth.get_time_of_day = wesnoth.deprecate_api('wesnoth.get_time_of_day', 'wesnoth.schedule.get_time_of_day or wesnoth.schedule.get_illumination', 1, nil, get_time_of_day, 'The arguments have changed')
	wesnoth.set_time_of_day = wesnoth.deprecate_api('wesnoth.set_time_of_day', 'wesnoth.current.schedule.time_of_day', 4, nil, nil)
	wesnoth.get_max_liminal_bonus = wesnoth.deprecate_api('wesnoth.get_max_liminal_bonus', 'wesnoth.current.schedule.liminal_bonus', 1, nil, liminal_bonus, "It's now a read-write attribute")
	wesnoth.replace_schedule = wesnoth.deprecate_api('wesnoth.replace_schedule', 'wesnoth.schedule.replace', 1, nil, wesnoth.schedule.replace)

	wesnoth.get_traits = wesnoth.deprecate_api('wesnoth.get_traits', 'wesnoth.game_config.global_traits', 1, nil, function() return wesnoth.game_config.global_traits end)
	wesnoth.end_level = wesnoth.deprecate_api('wesnoth.end_level', 'wesnoth.scenario.end_level_data assignment', 1, nil, function(cfg) wesnoth.scenario.end_level_data = cfg end)
	wesnoth.get_end_level_data = wesnoth.deprecate_api('wesnoth.get_end_level_data', 'wesnoth.scenario.end_level_data', 1, nil, function() return wesnoth.scenario.end_level_data end)

	wesnoth.invoke_synced_command = wesnoth.deprecate_api('wesnoth.invoke_synced_command', 'wesnoth.sync.invoke_command', 1, nil, wesnoth.sync.invoke_command)
	wesnoth.unsynced = wesnoth.deprecate_api('wesnoth.unsynced', 'wesnoth.sync.run_unsynced', 1, nil, wesnoth.sync.run_unsynced)
	wesnoth.synchronize_choice = wesnoth.deprecate_api('wesnoth.synchronize_choice', 'wesnoth.sync.evaluate_single', 1, nil, wesnoth.sync.evaluate_single)
	wesnoth.synchronize_choices = wesnoth.deprecate_api('wesnoth.synchronize_choices', 'wesnoth.sync.evaluate_multiple', 1, nil, wesnoth.sync.evaluate_multiple)

	wesnoth.find_cost_map = wesnoth.deprecate_api('wesnoth.find_cost_map', 'wesnoth.paths.find_cost_map', 1, nil, wesnoth.paths.find_cost_map)
	wesnoth.find_reach = wesnoth.deprecate_api('wesnoth.find_reach', 'wesnoth.paths.find_reach', 1, nil, wesnoth.paths.find_reach)
	wesnoth.find_vacant_tile = wesnoth.deprecate_api('wesnoth.find_vacant_tile', 'wesnoth.paths.find_vacant_hex', 1, nil, wesnoth.paths.find_vacant_hex)
	wesnoth.find_vision_range = wesnoth.deprecate_api('wesnoth.find_vision_range', 'wesnoth.paths.find_vision_range', 1, nil, wesnoth.paths.find_vision_range)
end
