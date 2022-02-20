-- NOTE: the string output from here is intended solely as an aid in debugging; it should never be taken and used as an input to anything else.

-- escaping takes 3/4 of the time, but we can't avoid it...
local function escape(str)
	str = string.gsub(str, "%c", "")
	str = string.gsub(str, "[\\\"]", "\\%0")
	return str
end

local function add_table_key(obj, buffer)
	local _type = type(obj)
	if _type == "string" then
		buffer[#buffer + 1] = escape(obj)
	elseif _type == "number" then
		buffer[#buffer + 1] = obj
	elseif _type == "boolean" then
		buffer[#buffer + 1] = tostring(obj)
	else
		buffer[#buffer + 1] = '???' .. _type .. '???'
	end
end

local function format_any_value(obj, buffer)
	local _type = type(obj)
	if _type == "table" then
		buffer[#buffer + 1] = '{'
		buffer[#buffer + 1] = '"' -- needs to be separate for empty tables {}
		for key, value in pairs(obj) do
			add_table_key(key, buffer)
			buffer[#buffer + 1] = '":'
			format_any_value(value, buffer)
			buffer[#buffer + 1] = ',"'
		end
		buffer[#buffer] = '}' -- note the overwrite
	elseif _type == "string" then
		buffer[#buffer + 1] = '"' .. escape(obj) .. '"'
	elseif _type == "boolean" or _type == "number" then
		buffer[#buffer + 1] = tostring(obj)
	elseif _type == "userdata" then
		buffer[#buffer + 1] = '"' .. escape(tostring(obj)) .. '"'
	else
		buffer[#buffer + 1] = '"???' .. _type .. '???"'
	end
end

local function value_to_text(obj)
	if obj == nil then return "null" else
		local buffer = {}
		format_any_value(obj, buffer)
		return table.concat(buffer)
	end
end

---Convert an arbitrary value (especially a table) to a string for debugging
---@return string
function wesnoth.as_text(...)
	local result = {}
	local n = 1
	for _, v in ipairs({ ... }) do
		result[n] = value_to_text(v)
		n = n + 1
	end
	return table.concat(result, "\t")
end
