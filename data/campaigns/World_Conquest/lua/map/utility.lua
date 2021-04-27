
globals = {}
setmetatable(globals, {
	["__index"] = function(t, k)
		return rawget(_G, k)
	end,
	["__newindex"] = function(t, k, v)
		_G[k] = v
	end,
})

function wct_enemy(side, com, item, train, sup, l2, l3)
	return {
		commander=com,
		have_item=item,
		trained=train,
		supply=sup,
		recall_level2 = l2,
		recall_level3 = l3,
	}
end

function Set (list)
	local set = {}
	for _, l in ipairs(list) do set[l] = true end
	return set
end

function shuffle_special_locations(map, loc_ids)
	local locs = {}
	for i , v in ipairs(loc_ids) do
		-- this tostring fixes a problem becasue  map.special_locations
		-- is actually a table with map at index 1 so map.special_locations[1]
		-- would just return 1.
		locs[i] = map.special_locations[tostring(v)]
	end
	assert(#locs == #loc_ids)
	mathx.shuffle(locs)
	for i , v in ipairs(loc_ids) do
		map.special_locations[tostring(v)] = locs[i]
	end
end

function shallow_copy(t)
	local res = {}
	for i, v in pairs(t) do
		res[i] = v
	end
	return res
end

wesnoth.dofile("./generator/utilities.lua")
