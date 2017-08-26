local MG = wesnoth.require "mapgen_helper"
local LS = wesnoth.require "location_set"
local random = mathx.random

local callbacks = {}

function callbacks.generate_map(params)
	local map = MG.create_map(params.map_width, params.map_height, params.terrain_wall)

	local function build_chamber(x, y, locs_set, size, jagged)
		if locs_set:get(x,y) or not map:on_board(x, y) or size == 0 then
			return
		end
		locs_set:insert(x,y)
		for xn, yn in MG.adjacent_tiles(x, y) do
			if random(100) <= 100 - jagged then
				build_chamber(xn, yn, locs_set, size - 1, jagged)
			end
		end
	end

	local function clear_tile(x, y, terrain_clear)
		if not map:on_board(x,y) then
			return
		end
		if map:get_tile(x,y) == params.terrain_castle or map:get_tile(x,y) == params.terrain_keep then
			return
		end
		local r = random(1000)
		if r <= params.village_density then
			map:set_tile(x, y, mathx.random_choice(params.terrain_village))
		else
			map:set_tile(x, y, mathx.random_choice(terrain_clear or params.terrain_clear))
		end
	end

	local function place_road(to_x, to_y, from_x, from_y, road_ops, terrain_clear)
		if not map:on_board(to_x, to_y) then
			return
		end
		if map:get_tile(to_x, to_y) == params.terrain_castle or map:get_tile(to_x, to_y) == params.terrain_keep then
			return
		end
		local tile_op = road_ops[map:get_tile(to_x, to_y)]
		if tile_op then
			if tile_op.convert_to_bridge and from_x and from_y then
				local bridges = {}
				for elem in tile_op.convert_to_bridge:gmatch("[^%s,][^,]*") do
					table.insert(bridges, elem)
				end
				local dir = wesnoth.map.get_relative_dir(from_x, from_y, to_x, to_y)
				if dir == 'n' or dir == 's' then
					map:set_tile(to_x, to_y, bridges[1])
				elseif dir == 'sw' or dir == 'ne' then
					map:set_tile(to_x, to_y, bridges[2])
				elseif dir == 'se' or dir == 'nw' then
					map:set_tile(to_x, to_y, bridges[3])
				end
			elseif tile_op.convert_to then
				local tile = mathx.random_choice(tile_op.convert_to)
				map:set_tile(to_x, to_y, tile)
			end
		else
			local tile = mathx.random_choice(terrain_clear or params.terrain_clear)
			map:set_tile(to_x, to_y, tile)
		end
	end

	local chambers = {}
	local chambers_by_id = {}
	local passages = {}

	for chamber in wml.child_range(params, "chamber") do
		local chance = tonumber(chamber.chance) or 100
		local x, y = MG.random_location(chamber.x, chamber.y)
		local id = chamber.id
		if chance == 0 or random(100) > chance then
			-- Set chance to 0 so that the scenario generator can tell which chambers were used
			params.chance = 0
			goto continue
		end
		if type(chamber.require_player) == "number" and chamber.require_player > params.nplayers then
			params.chance = 0
			goto continue
		end
		-- Ditto, set it to 100
		params.chance = 100
		local locs_set = LS.create()
		build_chamber(x, y, locs_set, chamber.size or 3, chamber.jagged or 0)
		local items = {}
		for item in wml.child_range(chamber, "item_location") do
			table.insert(items, item)
		end
		table.insert(chambers, {
			center_x = x,
			center_y = y,
			side_num = chamber.side,
			locs_set = locs_set,
			id = id,
			items = items,
			data = chamber,
		})
		chambers_by_id[id] = chambers[#chambers]
		for passage in wml.child_range(chamber, "passage") do
			local dst = chambers_by_id[passage.destination]
			if dst ~= nil then
				local road_costs, road_ops = {}, {}
				for road in wml.child_range(passage, "road_cost") do
					road_costs[road.terrain] = road.cost
					road_ops[road.terrain] = road
				end
				table.insert(passages, {
					start_x = x,
					start_y = y,
					dest_x = dst.center_x,
					dest_y = dst.center_y,
					data = passage,
					costs = road_costs,
					roads = road_ops,
				})
			end
		end
		::continue::
	end

	for i,v in ipairs(chambers) do
		local locs_list = {}
		for x, y in v.locs_set:stable_iter() do
			clear_tile(x, y, v.data.terrain_clear)
			if map:on_inner_board(x, y) then
				table.insert(locs_list, {x,y})
			end
		end
		for i1, item in ipairs(v.items or {}) do
			local index = random(#locs_list)
			local loc = locs_list[index]
			table.remove(locs_list, index)
			local x, y = table.unpack(loc)

			if item.id then
				map:add_location(x, y, item.id)
			end

			if item.place_castle then
				map:set_tile(x, y, params.terrain_keep)
				for x2, y2 in MG.adjacent_tiles(x, y) do
					map:set_tile(x2, y2, params.terrain_castle)
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
			local calc = function(x, y)
				if x == 0 or x == params.map_width - 1 or y == 0 or y == params.map_height - 1 then
					-- Map borders are impassable
					return math.huge
				end
				local res = 1.0
				local tile = map:get_tile(x, y)
				if tile == params.terrain_wall then
					res = laziness
				else
					res = v.costs[tile] or 1.0
				end
				if windiness > 1 then
					res = res * random(windiness)
				end
				return res
			end
			local path = wesnoth.paths.find_path(
				v.start_x, v.start_y, v.dest_x, v.dest_y, calc, params.map_width, params.map_height)
			for j, loc in ipairs(path) do
				local locs_set = LS.create()
				build_chamber(loc[1], loc[2], locs_set, width, jagged)
				local prev_x, prev_y
				for x,y in locs_set:stable_iter() do
					local r = 1000
					local ter_to_place
					if v.data.place_villages then r = random(1000) end
					if r <= params.village_density then
						ter_to_place = v.data.terrain_village or params.terrain_village
					else
						ter_to_place = v.data.terrain_clear or params.terrain_clear
					end
					place_road(x, y, prev_x, prev_y, v.roads, ter_to_place)
					prev_x, prev_y = x, y
				end
			end
		end

	end

	if type(params.transform) == "string" then
		local chance = params.transform_chance or 100
		if random(100) <= chance then
			local transforms = {}
			for t in params.transform:gmatch("[^%s,][^,]*") do
				if MG.is_valid_transform(t) then
					table.insert(transforms, t)
				else
					wml.error("Unknown transformation '" .. t .. "'")
				end
			end
			map[transforms[random(#transforms)]](map)
		end
	end

	return tostring(map)
end

function callbacks.generate_scenario(params)
	-- This is more or less backwards compatible with the cave generator syntax
	local scenario = wml.get_child(params, "scenario")
	scenario.map_data = callbacks.generate_map(params)
	for chamber in wml.child_range(params, "chamber") do
		local chamber_items = wml.get_child(chamber, "items")
		if chamber.chance == 100 and chamber_items then
			-- TODO: Should we support [event]same_location_as_previous=yes?
			for i,tag in ipairs(chamber_items) do
				table.insert(scenario, tag)
			end
		end
	end
	return scenario
end

return callbacks
