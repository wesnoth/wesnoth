
----------------------------------------------------------
---- The code that generates the maps, basicially     ----
---- each map is generated in two steps, first        ----
---- the default mapgenerator is used then, one of    ----
---- the codes in ,/postgeneration is run             ----
----------------------------------------------------------

Distmap = wesnoth.dofile("./distmap.lua")
wesnoth.dofile("./postgeneration_utils/engine.lua")

local postgenerators = {}
for i, v in ipairs(filesystem.read_file("./postgeneration")) do
	local code = string.match(v, "^(%d%a).*")
	if code then
		postgenerators[string.lower(code)] = v
	end
end

function wct_map_enemy_themed(race, pet, castle, village, chance)
	table.insert(prestart_event, wml.tag.wc2_enemy_themed {
		race = race,
		pet = pet,
		castle = castle,
		village = village,
		chance = chance
	})
end

local function run_postgeneration(map_data, id, scenario_content, nplayers, nhumanplayer)
	local player_list = {}
	for i = 1, nplayers do--nhumanplayer
		player_list[i] = i
	end
	local postgen_starttime = wesnoth.ms_since_init()
	wesnoth.dofile("./postgeneration_utils/utilities.lua")
	wesnoth.dofile("./postgeneration_utils/events.lua")
	wesnoth.dofile("./postgeneration_utils/snow.lua")
	wesnoth.dofile("./postgeneration_utils/noise.lua")
	local postgenfile = postgenerators[id] or id .. "./lua"
	--local postgenfile = postgenerators["2f"] or id .. "./lua"
	_G.scenario_data = {
		nplayers = nplayers,
		nhumanplayers = nhumanplayer,
		scenario = scenario_content,
	}
	_G.map = wesnoth.map.create(map_data)
	_G.total_tiles = _G.map.width * _G.map.height
	_G.prestart_event = scenario_content.event[1]
	_G.print_time = function(msg)
		wesnoth.log("info", msg .. " time: " .. (wesnoth.ms_since_init() - postgen_starttime))
	end
	--the only reason why we do this here an not in mian.lua is that it needs a map object.
	shuffle_special_locations(map, player_list)
	local fun = wesnoth.dofile(string.format("./postgeneration/%s", postgenfile))
	fun()
	print_time("postegen end")
	wct_fix_impassible_item_spawn(_G.map)
	local map = _G.map.data
	_G.map = nil
	_G.total_tiles = nil
	_G.prestart_event = nil
	_G.scenario_data = nil
	return map
end

function wct_map_generator(default_id, postgen_id, length, villages, castle, iterations, hill_size, players, island)
	return function(scenario, nhumanplayer)
		wesnoth.log("debug", "wct_map_generator " .. default_id .. " " .. postgen_id)
		local generatorfile = "./generator/" .. default_id .. ".lua"
		local generate1 = wesnoth.dofile(generatorfile)
		wesnoth.log("debug", "running generate")
		local map_data =generate1(length, villages, castle, iterations, hill_size, players, island)
		map_data = run_postgeneration(map_data, postgen_id, scenario, players, nhumanplayer)
		scenario.map_data = map_data
	end
end

function world_conquest_tek_scenario_res()
end
