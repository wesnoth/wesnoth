--[========[Map module]========]
print("Loading map module...")

function wesnoth.map.split_terrain_code(code)
	return table.unpack(code:split('^', {remove_empty = false}))
end

function wesnoth.map.read_location(...)
	local x, y = ...
	if y == nil then
		if type(x.x) == 'number' and type(x.y) == 'number' then
			x, y = x.x, x.y
		elseif type(x[1]) == 'number' and type(x[2]) == 'number' then
			x, y = table.unpack(x)
		else
			return nil, 0
		end
		return {x = x, y = y}, 1
	elseif type(x) == 'number' or type(y) == 'number' then
		return {x = x, y = y}, 2
	end
	return nil, 0
end

if wesnoth.kernel_type() ~= "Application Lua Kernel" then
	-- possible terrain string inputs:
	-- A        A^       A^B      ^        ^B
	-- implied mode:
	-- both     base     both     overlay  overlay
	function wesnoth.map.replace_base(code)
		local base, overlay = wesnoth.map.split_terrain_code(code)
		if base == nil then -- ^ or ^B
			-- There's no base to replace with, so do nothing
			return ''
		else
			-- Use the specified base but ignore the overlay
			return base .. '^'
		end
	end
	
	function wesnoth.map.replace_overlay(code)
		local base, overlay = wesnoth.map.split_terrain_code(code)
		if overlay == nil or overlay == '' then -- A or A^
			-- No overlay was specified, so we want to clear the overlay without touching the base
			return '^'
		else
			-- An overlay was specified, so use that and ignore the base
			return '^' .. overlay
		end
	end
	
	function wesnoth.map.replace_both(code)
		local base, overlay = wesnoth.map.split_terrain_code(code)
		if base == '' then -- ^ or ^B
			-- There's no way to find a base to replace with in this case.
			-- Could use the existing base, but that's not really replacing both, is it?
			error('replace_both: no base terrain specified')
		elseif overlay == '' then -- A^
			-- This would normally mean replace base while preserving overlay,
			-- but we actually want to replace base and clear overlay.
			return base
		else
			-- It's already going to replace both, so return unchanged
			return code
		end
	end
end