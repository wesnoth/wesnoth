local helper = wesnoth.require "lua/helper.lua"
local T = helper.set_wml_tag_metatable {}
local random = wesnoth.random

local passage_chance = function(chance, dest, width, windiness, jagged)
	return T.passage {
		chance = chance,
		destination = dest,
		width = width,
		windiness = windiness,
		jagged = jagged,
	}
end

local passage_normal = function(dest, width, windiness, jagged)
	return passage_chance(nil, dest, width, windiness, jagged)
end

local param_chambers = { 
	{
		id = "player",
		x = "15-35",
		y = 68,
		size = 8,
		jagged = 50,
		side = 1,
	},
	{
		id = "antechamber_1",
		x = "10-25",
		y = "50-60",
		size = 7,
		jagged = 5,
		side = 2,
		passage_normal("player", 2, 10, 10),
	},
	{
		id="antechamber_2",
		x="25-40",
		y="50-60",
		size=7,
		jagged=2,
		passage_normal("player", 2, 3, 1),
		passage_chance(40, "antechamber_1", 1, 9, 9),
		side=3,
	},
	{
		id="center",
		x="25-26",
		y="35-36",
		size=2,
		jagged=1,
		passage_normal("antechamber_1", 1, 20, 3),
		passage_normal("antechamber_2", 1, 20, 3),
	},
	{
		id="mini_1",
		x="10-16",
		y="36-40",
		size=5,
		jagged=2,
		passage_normal("center", 1, 5, 2),
		passage_normal("antechamber_1", 2, 5, 2),
	},
	{
		id="mini_2",
		x="8-20",
		y="17-26",
		size=5,
		jagged=3,
		passage_normal("center", 1, 5, 2),
		passage_normal("mini_1", 1, 5, 2),
	},
	{
		id="mini_3",
		x="6-44",
		y="14-30",
		size=3,
		jagged=4,
		passage_normal("center", 1, 5, 2),
		passage_normal("mini_2", 2, 5, 2),
	},
	{
		id="mini_4",
		x="30-42",
		y="17-26",
		size=5,
		jagged=5,
		passage_normal("center", 1, 5, 2),
		passage_normal("mini_3", 2, 5, 2),
	},
	{
		id="mini_5",
		x="34-40",
		y="36-40",
		size=5,
		jagged=5,
		passage_normal("mini_4", 2, 5, 2),
		passage_normal("center", 1, 5, 2),
		passage_normal("antechamber_2", 2, 5, 2),
	},
	{
		id="exit",
		x=25,
		y=1,
		size=2,
		jagged=1,
		passage_normal("mini_2", 1, 5, 2),
		passage_normal("mini_3", 1, 5, 2),
		passage_normal("mini_4", 1, 5, 2),
	},
	{
		id="enemy_1",
		x="6-15",
		y="45-55",
		size=5,
		jagged=3,
		passage_normal("mini_1", 2, 5, 2),
		passage_chance(60, "mini_2", 1, 5, 2),
		passage_chance(40, "antechamber_2", 1, 5, 2),
		side=4,
	},
	{
		id="enemy_2",
		x="6-15",
		y="1-35",
		size=5,
		jagged=3,
		passage_normal("mini_2", 2, 5, 2),
		passage_normal("enemy_1", 2, 5, 5),
		passage_chance(60, "mini_1", 1, 5, 2),
		passage_chance(40, "mini_3", 1, 5, 2),
		side=5,
	},
	{
		id="enemy_3",
		x="35-45",
		y="1-35",
		size=5,
		jagged=3,
		passage_normal("mini_4", 2, 5, 2),
		passage_chance(60, "mini_5", 1, 5, 2),
		passage_chance(40, "mini_3", 1, 5, 2),
		side=6,
	},
	{
		id="enemy_4",
		x="35-45",
		y="45-55",
		size=5,
		jagged=3,
		passage_normal("mini_5", 2, 5, 2),
		passage_normal("enemy_3", 2, 5, 5),
		passage_chance(60, "mini_4", 1, 5, 2),
		passage_chance(40, "antechamber_2", 1, 5, 2),
		side=7,
	}
}

local size_x = 50
local size_y = 70
local terrain_wall = "Xu"
local terrain_clear = "Re"
local terrain_keep = "Kud"
local terrain_castle = "Cud"
local terrain_village = "Uu^Vud"

local map = {}

local function loc_to_index(x,y)
	return x + 1 + (y) * size_x
end
for i = 1, size_x * size_y do
	table.insert(map, terrain_wall)
end
--positions are 0-based to match the ingame locations.
local function set_tile_index(index, val)
	if index < 1 or index > size_x * size_y then
		return
	end
	if map[index] == terrain_castle or map[index] == terrain_keep then
		return
	end
	if val == terrain_clear then
		local r = random(100)
		if r <= 5 then
			map[index] = "Uu^Vud"
		elseif r <= 20 then
			map[index] = "Re"
		elseif r <= 40 then
			map[index] = "Uh"
		elseif r <= 65 then
			map[index] = "Uu^Uf"
		else
			map[index] = "Uu"
		end
	else
		map[index] = val
	end
	end
end

local function set_tile(x, y, val)
	set_tile_index(loc_to_index(x, y), val)
end

local function get_tile(x, y, val)
	return map[loc_to_index(x,y)]
end

local function on_board(x, y)
	return x >= 0 and y >= 0 and x < size_x and y < size_y
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
for i,v in ipairs(param_chambers) do
	local x = v.x
	local y = v.y
	local id = v.id
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
	table.insert(chambers, {
		center_x = x,
		center_y = y,
		side_num = v.side,
		locs_set = locs_set,
		id = id,
		items = helper.get_child(v, "items")
	})
	chambers_by_id[id] = chambers[#chambers]
	for passage in helper.child_range(v, "passage") do
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
	local locs_list = {}
	for k2,v2 in pairs(v.locs_set) do
		set_tile_index(k2, terrain_clear)
		table.insert(locs_list, v2)
	end
	if v.side_num then
		starting_positions[v.side_num] = { v.center_x, v.center_y }
		set_tile(v.center_x, v.center_y, terrain_keep)
		for x, y in adjacent_tiles(v.center_x, v.center_y) do
			set_tile(x, y, terrain_castle)
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
			if get_tile(x, y) == terrain_wall then
				res = laziness
			end
			if windiness > 1 then
				res = res * random(windiness)
			end
			return res
		end
		local path, cost = wesnoth.find_path(v.start_x, v.start_y, v.dest_x, v.dest_y, calc, size_x, size_y)
		for i, loc in ipairs(path) do
			local locs_set = {}
			build_chamber(loc[1], loc[2], locs_set, width, jagged)
			for k,v in pairs(locs_set) do
				set_tile_index(k, terrain_clear)
			end
		end
	end

end

local starting_pos_by_tile = {}
for k,v in ipairs(starting_positions) do
	starting_pos_by_tile[v[1] + 1 + v[2] * size_x] = k
end

local stringbuilder = {}
for y = 0, size_y - 1 do
	local stringbuilder_row = {}
	for x = 0, size_x - 1 do
		local index = x + 1 + y * size_x
		if starting_pos_by_tile[index] ~= nil then
			table.insert(stringbuilder_row, tostring(starting_pos_by_tile[index]) .. " " .. map[index])
		else
			table.insert(stringbuilder_row, map[index])
		end
	end
	table.insert(stringbuilder, table.concat(stringbuilder_row, ","))
end

return table.concat(stringbuilder, "\n")
