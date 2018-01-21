local LS = wesnoth.require "location_set"

local mapgen_helper, map_mt = {}, {__index = {}}

function mapgen_helper.create_map(width,height,default_terrain)
	local map = setmetatable({w = width, h = height}, map_mt)
	for i = 1, width * height do
		table.insert(map, default_terrain or 'Gg')
	end
	return map
end

local valid_transforms = {
	flip_x = true,
	flip_y = true,
	flip_xy = true,
}

function mapgen_helper.is_valid_transform(t)
	return valid_transforms[t]
end

local function loc_to_index(map,x,y)
	return x + 1 + y * map.w
end

function map_mt.__index.set_tile(map, x, y, val)
	map[loc_to_index(map, x, y)] = val
end

function map_mt.__index.get_tile(map, x, y)
	return map[loc_to_index(map,x,y)]
end

function map_mt.__index.on_board(map, x, y)
	return x >= 0 and y >= 0 and x < map.w and y < map.h
end

function map_mt.__index.on_inner_board(map, x, y)
	return x >= 1 and y >= 1 and x < map.w - 1 and y < map.h -  1
end

function map_mt.__index.add_location(map, x, y, name)
	if not map.locations then
		map.locations = LS.create()
	end
	if map.locations:get(x, y) then
		table.insert(map.locations:get(x, y), name)
	else
		map.locations:insert(x, y, {name})
	end
end

function map_mt.__index.flip_x(map)
	for y = 0, map.h - 1 do
		for x = 0, map.w - 1 do
			local i = loc_to_index(map, x, y)
			local j = loc_to_index(map, map.w - x, y)
			map[i], map[j] = map[j], map[i]
		end
	end
end

function map_mt.__index.flip_y(map)
	for x = 0, map.w - 1 do
		for y = 0, map.h - 1 do
			local i = loc_to_index(map, x, y)
			local j = loc_to_index(map, x, map.h - y)
			map[i], map[j] = map[j], map[i]
		end
	end
end

function map_mt.__index.flip_xy(map)
	map:flip_x()
	map:flip_y()
end

function map_mt.__tostring(map)
	local map_builder = {}
	-- The coordinates are 0-based to match the in-game coordinates
	for y = 0, map.h - 1 do
		local string_builder = {}
		for x = 0, map.w - 1 do
			local tile_string = map:get_tile(x, y)
			if map.locations and map.locations:get(x,y) then
				for i,v in ipairs(map.locations:get(x,y)) do
					tile_string = v .. ' ' .. tile_string
				end
			end
			table.insert(string_builder, tile_string)
		end
		assert(#string_builder == map.w)
		table.insert(map_builder, table.concat(string_builder, ', '))
	end
	assert(#map_builder == map.h)
	return table.concat(map_builder, '\n')
end

local adjacent_offset = {
	{ {0,-1}, {1,-1}, {1,0}, {0,1}, {-1,0}, {-1,-1} },
	{ {0,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0} }
}
function mapgen_helper.adjacent_tiles(x, y)
	local offset = adjacent_offset[2 - (x % 2)]
	local i = 0
	return function()
		i = i + 1
		if i <= 6 then
			return offset[i][1] + x, offset[i][2] + y
		else
			return nil
		end
	end
end

return mapgen_helper
