world_conquest_tek_scenario_res(1, 1, 26)
local generators = {
	wct_map_generator("classic", "1a", 25, 18, 5, 6400, 3, 2, 2),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 150,
		sides = {
			wct_enemy(4, 1, 0, 0, 0, 1, 0),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 26, player_gold = 200 }
