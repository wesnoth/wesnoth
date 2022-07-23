local LS = wesnoth.require "location_set"

---@class mapgen_helper
local mapgen_helper, map_mt = {}, {__index = {}}

---@class map_wrapper
---@field __map terrain_map

---Create a map.
---@deprecated Use wesnoth.map.create instead
---@param width integer
---@param height integer
---@param default_terrain string
---@return map_wrapper
local function create_map(width,height,default_terrain)
	return setmetatable({__map = wesnoth.map.create(width, height, default_terrain)}, map_mt)
end

local function rand_from_ranges(list)
	if type(list) == 'number' then return math.tointeger(list) end
	local choices = {}
	for n in stringx.iter_ranges(list) do
		table.insert(choices, n)
	end
	return math.tointeger(mathx.random_choice(choices)) or 0
end

---Select a random location from lists of coordinates.
---@param x_list string
---@param y_list string
---@return integer
---@return integer
function mapgen_helper.random_location(x_list, y_list)
	return rand_from_ranges(x_list), rand_from_ranges(y_list)
end

local valid_transforms = {
	flip_x = true,
	flip_y = true,
	flip_xy = true,
}

---Test whether a string is a valid transform
---@param t string
---@return boolean
function mapgen_helper.is_valid_transform(t)
	return valid_transforms[t]
end

---Set the tile at the specified location
---@param map map_wrapper
---@param x integer
---@param y integer
---@param val string
function map_mt.__index.set_tile(map, x, y, val)
	map.__map[{x, y}] = val
end

---Set the tile at the specified location
---@param map map_wrapper
---@param x integer
---@param y integer
---@return string
function map_mt.__index.get_tile(map, x, y)
	return map.__map[{x, y}]
end

---Test if a tile is on the board (including the border)
---@param map map_wrapper
---@param x integer
---@param y integer
---@return boolean
function map_mt.__index.on_board(map, x, y)
	return map.__map:on_board(x, y, true)
end

---Test if a tile is on the board (excluding the border)
---@param map map_wrapper
---@param x integer
---@param y integer
---@return boolean
function map_mt.__index.on_inner_board(map, x, y)
	return map.__map:on_board(x, y, false)
end

---Add a named location at the given coordinates
---@param map map_wrapper
---@param x integer
---@param y integer
---@param name string
function map_mt.__index.add_location(map, x, y, name)
	map.__map.special_locations[name] = {x, y}
end

---Flip the map horizontally
---@param map terrain_map
function mapgen_helper.flip_x(map)
	for x, y in map:iter(true) do
		if x <= map.width / 2 or y <= map.height / 2 then
			local x_opp = map.width - x - 1
			map[{x,y}], map[{x_opp,y}] = map[{x_opp,y}], map[{x,y}]
		end
	end
end

---Flip the map vertically
---@param map terrain_map
function mapgen_helper.flip_y(map)
	for x, y in map:iter(true) do
		if x <= map.width / 2 or y <= map.height / 2 then
			local y_opp = map.height - y - 1
			map[{x,y}], map[{x,y_opp}] = map[{x,y_opp}], map[{x,y}]
		end
	end
end

---Flip the map diagonally
---@param map terrain_map
function mapgen_helper.flip_xy(map)
	for x, y in map:iter(true) do
		if x <= map.width / 2 or y <= map.height / 2 then
			local x_opp = map.width - x - 1
			local y_opp = map.height - y - 1
			map[{x,y}], map[{x_opp,y_opp}] = map[{x_opp,y_opp}], map[{x,y}]
		end
	end
end

---Convert to string
---@param map map_wrapper
---@return string
function map_mt.__tostring(map)
	return map.__map.data
end

local adjacent_offset = {
	{ {0,-1}, {1,-1}, {1,0}, {0,1}, {-1,0}, {-1,-1} },
	{ {0,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0} }
}
---Iterates over adjacent tiles
---@param x integer
---@param y integer
---@return fun():integer,integer
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

---@alias relative_anchor
---|'center'
---|'top-left'
---|'top-middle'
---|'top-right'
---|'bottom-left'
---|'bottom-middle'
---|'bottom-right'
---|'middle-left'
---|'middle-right'

---@class map_chamber : WMLTable
---@field ignore boolean
---@field id string
---@field x string
---@field y string
---@field terrain_clear string
---@field size integer
---@field jagged integer
---@field chance integer
---@field side integer
---@field relative_to relative_anchor
---@field require_player integer

---@class map_passage : WMLTable
---@field ignore boolean
---@field id string
---@field place_villages boolean
---@field destination string
---@field terrain_clear string
---@field windiness integer
---@field laziness integer
---@field width integer
---@field jagged integer
---@field chance integer

---Get a chamber by ID or index from the map generator settings.
---@param params WMLTable
---@param id_or_idx string|integer
---@return map_chamber?
function mapgen_helper.get_chamber(params, id_or_idx)
	if type(id_or_idx) == 'number' then
		local cfg, i = wml.get_nth_child(params, 'chamber', id_or_idx)
		if not cfg then return nil end
		return params[i].contents
	elseif type(id_or_idx) == 'string' then
		local cfg, i = wml.get_child(params, 'chamber', id_or_idx)
		if not cfg then return nil end
		return params[i].contents
	end
end

---Get a passage by ID or index from the map generator settings.
---@param chamber map_chamber
---@param id_or_idx string|integer
---@return map_passage?
function mapgen_helper.get_passage(chamber, id_or_idx)
	if type(id_or_idx) == 'number' then
		local cfg, i = wml.get_nth_child(chamber, 'passage', id_or_idx)
		if not cfg then return nil end
		return chamber[i].contents
	elseif type(id_or_idx) == 'string' then
		local cfg, i = wml.get_child(chamber, 'passage', id_or_idx)
		if not cfg then return nil end
		return chamber[i].contents
	end
end

mapgen_helper.create_map = wesnoth.deprecate_api('mapgen_helper.create_map', 'wesnoth.map.create', 1, nil, create_map)
mapgen_helper.adjacent_tiles = wesnoth.deprecate_api('mapgen_helper.adjacent_tiles', 'wesnoth.map.iter_adjacent', 1, nil, mapgen_helper.adjacent_tiles)
map_mt.__index.set_tile = wesnoth.deprecate_api('oldmap:set_tile(x,y,ter)', 'map[{x,y}]=ter', 1, nil, map_mt.__index.set_tile)
map_mt.__index.get_tile = wesnoth.deprecate_api('oldmap:get_tile(x,y)', 'map[{x,y}]', 1, nil, map_mt.__index.get_tile)
map_mt.__index.on_board = wesnoth.deprecate_api('oldmap:on_board(x,y)', 'map:on_board(x,y,true)', 1, nil, map_mt.__index.on_board)
map_mt.__index.on_inner_board = wesnoth.deprecate_api('oldmap:on_inner_board(x,y)', 'map:on_board(x,y,false)', 1, nil, map_mt.__index.on_inner_board)
map_mt.__index.add_location = wesnoth.deprecate_api('oldmap:add_location(x,y,name)', 'map.special_locations[name]={x,y}', 1, nil, map_mt.__index.add_location)
map_mt.__index.flip_x = wesnoth.deprecate_api('oldmap:flip_x()', 'mapgen_helper.flip_x(map)', 1, nil, function(m) mapgen_helper.flip_x(m) end)
map_mt.__index.flip_y = wesnoth.deprecate_api('oldmap:flip_y()', 'mapgen_helper.flip_y(map)', 1, nil, function(m) mapgen_helper.flip_y(m) end)
map_mt.__index.flip_xy = wesnoth.deprecate_api('oldmap:flip_xy()', 'mapgen_helper.flip_xy(map)', 1, nil, function(m) mapgen_helper.flip_xy(m) end)
map_mt.__tostring = wesnoth.deprecate_api('tostring(map)', 'map.data', 1, nil, map_mt.__tostring)

return mapgen_helper
