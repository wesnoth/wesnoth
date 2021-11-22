world_conquest_tek_scenario_res(4, 2, 26)
local generators = {
	wct_map_generator("classic", "2a", 50, 22, 6, 10000, 4, 6, 7),
	wct_map_generator("classic", "2b", 50, 22, 6, 10000, 4, 6, 7),
	wct_map_generator("classic", "2c", 50, 22, 6, 10000, 4, 6, 7),
	wct_map_generator("provinces", "2d", 50, 22, 6, 10000, 4, 6, 7),
	wct_map_generator("paradise", "2e", 50, 22, 6, 10000, 4, 6, 7),
	wct_map_generator("clayey", "2f", 50, 22, 6, 10000, 4, 6, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 135,
		sides = {
			wct_enemy(5, 0, 0, 0, 0, 4, 0),
			wct_enemy(6, 0, 1, 0, 0, 4, 0),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 26, player_gold = 150 }
