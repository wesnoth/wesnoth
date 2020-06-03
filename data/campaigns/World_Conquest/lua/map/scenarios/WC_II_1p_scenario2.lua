world_conquest_tek_scenario_res(1, 2, 32)
local generators = {
	wct_map_generator("classic", "2a", 35, 16, 6, 7560, 4, 3, 7),
	wct_map_generator("classic", "2b", 35, 16, 6, 7560, 4, 3, 7),
	wct_map_generator("classic", "2c", 35, 16, 6, 7560, 4, 3, 7),
	wct_map_generator("provinces", "2d", 35, 16, 6, 7560, 4, 3, 7),
	wct_map_generator("paradise", "2e", 35, 15, 6, 7560, 4, 3, 7),
	wct_map_generator("clayey", "2f", 35, 16, 6, 7560, 4, 3, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 135,
		sides = {
			wct_enemy(4, 0, 0, 0, 0, 1, 0),
			wct_enemy(5, 0, 1, 0, 0, 1, 0),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 32, player_gold = 250 }
