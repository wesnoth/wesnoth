--[========[Additional string support functions]========]
print("Loading stringx module...")

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

wesnoth.format = wesnoth.deprecate_api('wesnoth.format', 'stringx.vformat', 1, nil, stringx.vformat)
wesnoth.format_conjunct_list = wesnoth.deprecate_api('wesnoth.format_conjunct_list', 'stringx.format_conjunct_list', 1, nil, stringx.format_conjunct_list)
wesnoth.format_disjunct_list = wesnoth.deprecate_api('wesnoth.format_disjunct_list', 'stringx.format_disjunct_list', 1, nil, stringx.format_disjunct_list)
