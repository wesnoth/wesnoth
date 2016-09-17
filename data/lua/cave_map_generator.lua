local helper = wesnoth.require "lua/helper.lua"
local T = helper.set_wml_tag_metatable {}
local random = wesnoth.random

local params = _G.params

local map = {}

local function loc_to_index(x,y)
	return x + 1 + y * params.map_width
end

local function index_to_loc(index)
	return (index -1) % params.map_width, math.floor((index -1) / params.map_width)
end

for i = 1, params.map_width * params.map_height do
	table.insert(map, params.terrain_wall)
end
--positions are 0-based to match the ingame locations.
local function set_tile_index(index, val)
	if index < 1 or index > params.map_width * params.map_height then
		return
	end
	if map[index] == params.terrain_castle or map[index] == params.terrain_keep then
		return
	end
	if val == params.terrain_clear then
		local r = random(1000)
		if r <= params.village_density then
			map[index] = params.terrain_village
			else
			map[index] = params.terrain_clear
		end
		else
		map[index] = val
	end
end

local function set_tile(x, y, val)
	set_tile_index(loc_to_index(x, y), val)
end

local function get_tile(x, y, val)
	return map[loc_to_index(x,y)]
end

local function on_board(x, y)
	return x >= 0 and y >= 0 and x < params.map_width and y < params.map_height
end

local function on_inner_board(x, y)
	return x >= 1 and y >= 1 and x < params.map_width - 1 and y < params.map_height -  1
end

local adjacent_offset = {
	{ {0,-1}, {1,-1}, {1,0}, {0,1}, {-1,0}, {-1,-1} },
	{ {0,-1}, {1,0}, {1,1}, {0,1}, {-1,1}, {-1,0} }
}
local function adjacent_tiles(x, y)
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

local function build_chamber(x, y, locs_set, size, jagged)
	local index = loc_to_index(x, y)
	if locs_set[index] or not on_board(x, y) or size == 0 then
		return
	end
	locs_set[index] = {x, y}
	for xn, yn in adjacent_tiles(x, y) do
		if random(100) <= 100 - jagged then
			build_chamber(xn, yn, locs_set, size - 1, jagged)
		end
	end
end

local chambers = {}
local chambers_by_id = {}
local passages = {}
local starting_positions = {}

for chamber in helper.child_range(params, "chamber") do
	local x = chamber.x
	local y = chamber.y
	local id = chamber.id
	if type(x) == "string" then
		local x_min, x_max = x:match("(%d+)-(%d+)")
		x = random(tonumber(x_min), tonumber(x_max))
	end
	if type(y) == "string" then
		local y_min, y_max = y:match("(%d+)-(%d+)")
		y = random(tonumber(y_min), tonumber(y_max))
	end
	local locs_set = {}
	build_chamber(x, y, locs_set, chambers.size or 3, chambers.size or 0)
	local items = {}
	std_print("before item")
	for item in helper.child_range(chamber, "item") do
		std_print("added item")
		table.insert(items, item)
	end
	table.insert(chambers, {
		center_x = x,
		center_y = y,
		side_num = chamber.side,
		locs_set = locs_set,
		id = id,
		items = items,
		--items = helper.get_child(v, "items")
	})
	chambers_by_id[id] = chambers[#chambers]
	for passage in helper.child_range(chamber, "passage") do
		local dst = chambers_by_id[passage.destination]
		if dst ~= nil then
			table.insert(passages, {
				start_x = x,
				start_y = y,
				dest_x = dst.center_x,
				dest_y = dst.center_y,
				data = passage,
			})
		end
	end
end

for i,v in ipairs(chambers) do
	std_print("processing chamber " .. (v.id or ""))
	local locs_list = {}
	for k2,v2 in pairs(v.locs_set) do
		set_tile_index(k2, params.terrain_clear)
		if on_inner_board (v2[1], v2[2]) then
			table.insert(locs_list, k2)
		end
	end
	for i1, item in ipairs(v.items or {}) do
		local index = random(#locs_list)
		local loc = locs_list[index]
		table.remove(locs_list, index)
		std_print("placed special location " .. (item.id or "") .. " at " .. loc)
		local x, y = index_to_loc(loc)
		
		starting_positions[loc] = item.id
		
		if item.place_castle then
			set_tile_index(loc, params.terrain_keep)
			for x2, y2 in adjacent_tiles(x, y) do
				set_tile(x2, y2, params.terrain_castle)
			end
		end
	end
end

for i,v in ipairs(passages) do
	if random(100) <= (v.data.chance or 100) then
		local windiness = v.data.windiness or 0
		local laziness = math.max(v.data.laziness or 1, 1)
		local width = math.max(v.data.width or 1, 1)
		local jagged = v.data.jagged or 0
		local calc = function(x, y, current_cost)
			local res = 1.0
			if get_tile(x, y) == params.terrain_wall then
				res = laziness
			end
			if windiness > 1 then
				res = res * random(windiness)
			end
			return res
		end
		local path, cost = wesnoth.find_path(v.start_x, v.start_y, v.dest_x, v.dest_y, calc, params.map_width, params.map_height)
		for i, loc in ipairs(path) do
			local locs_set = {}
			build_chamber(loc[1], loc[2], locs_set, width, jagged)
			for k,v in pairs(locs_set) do
				set_tile_index(k, params.terrain_clear)
			end
		end
	end
	
end

local stringbuilder = {}

for y = 0, params.map_height - 1 do
	local stringbuilder_row = {}
	for x = 0, params.map_width - 1 do
		local index = x + 1 + y * params.map_width
		if starting_positions[index] ~= nil then
			table.insert(stringbuilder_row, tostring(starting_positions[index]) .. " " .. map[index])
			else
			table.insert(stringbuilder_row, map[index])
		end
	end
	table.insert(stringbuilder, table.concat(stringbuilder_row, ","))
end
std_print( table.concat(stringbuilder, "\n"))
return table.concat(stringbuilder, "\n")
