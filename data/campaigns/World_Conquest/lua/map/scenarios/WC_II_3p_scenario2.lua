world_conquest_tek_scenario_res(3, 2, 28)
local generators = {
	wct_map_generator("classic", "2a", 45, 21, 6, 9000, 4, 5, 7),
	wct_map_generator("classic", "2b", 45, 21, 6, 9000, 4, 5, 7),
	wct_map_generator("classic", "2c", 45, 21, 6, 9000, 4, 5, 7),
	wct_map_generator("provinces", "2d", 45, 21, 6, 9000, 4, 5, 7),
	wct_map_generator("paradise", "2e", 45, 21, 6, 9000, 4, 5, 7),
	wct_map_generator("clayey", "2f", 45, 21, 6, 9000, 4, 5, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 135,
		sides = {
			wct_enemy(4, 0, 0, 0, 0, 3, 0),
			wct_enemy(5, 0, 1, 0, 0, 3, 0),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 28, player_gold = 175 }
