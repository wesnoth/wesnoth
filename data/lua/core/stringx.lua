--[========[Additional string support functions]========]
print("Loading stringx module...")

---Split a string on a separator, skipping separators that have been escaped
---@param str string string to split
---@param sep? string separator character
---@param esc? string escape character
---@return string[]
function stringx.escaped_split(str, sep, esc)
	esc = esc or '\\'
	return stringx.split(str, sep, {escape = esc, strip_spaces = true, remove_empty = true})
end

---Split a string on a separator, skipping separators that are enclosed in quotes
---@param str string string to split
---@param sep? string separator character
---@param left? string left quote characters
---@param right? string right quote characters
---@return string[]
function stringx.quoted_split(str, sep, left, right)
	right = right or left
	if left == nil and right == nil then
		left = '('
		right = ')'
	end
	return stringx.split(str, sep, {quote_left = left, quote_right = right, strip_spaces = true, remove_empty = true})
end

---Split a progressive string on a separator
---@param str string string to split
---@param sep string separator character
---@return string[]
function stringx.anim_split(str, sep)
	return stringx.split(str, sep, {expand_anim = true, strip_spaces = true, remove_empty = true});
end

---Parse and iterate over an integer range
---@param str string
---@return fun():function,number
function stringx.iter_range(str)
	return coroutine.wrap(function()
		local lo, hi = stringx.parse_range(str)
		for i = lo, hi do
			coroutine.yield(i)
		end
	end)
end

---Parse and interate over a sequence of integer ranges
---@param str string
---@return fun():function,number
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

---Test if a string begins with a specified prefix
---@param s string The string to test
---@param p string The prefix to check
---@return boolean
function stringx.starts_with(s, p)
	return s:sub(0, #p) == p
end

---Test if a string end with a specified suffix
---@param s string The string to test
---@param p string The suffix to check
---@return boolean
function stringx.ends_with(s, p)
	return s:sub(-#p) == p
end

wesnoth.format = wesnoth.deprecate_api('wesnoth.format', 'stringx.vformat', 1, nil, stringx.vformat)
wesnoth.format_conjunct_list = wesnoth.deprecate_api('wesnoth.format_conjunct_list', 'stringx.format_conjunct_list', 1, nil, stringx.format_conjunct_list)
wesnoth.format_disjunct_list = wesnoth.deprecate_api('wesnoth.format_disjunct_list', 'stringx.format_disjunct_list', 1, nil, stringx.format_disjunct_list)
