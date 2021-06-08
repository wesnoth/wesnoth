
print("Loading Lua core files...")

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
	if wesnoth.game_config.strict_lua then return nil end
	local message = detail_msg or ''
	if replacement then
		message = message .. " " .. (_"(Note: You should use $replacement instead in new code)"):vformat{replacement = replacement}
	end
	if type(level) ~= "number" or level < 1 or level > 4 then
		local err_params = {level = level}
		-- Note: This message is duplicated in src/deprecation.cpp
		-- Any changes should be mirrorred there.
		error((_"Invalid deprecation level $level (should be 1-4)"):vformat(err_params))
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
		return setmetatable({}, mt)
	else
		wesnoth.log('warn', "Attempted to deprecate something that is not a table or function: " ..
			elem_name .. " -> " .. replacement .. ", where " .. elem_name .. " = " .. tostring(elem))
	end
	return elem
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
