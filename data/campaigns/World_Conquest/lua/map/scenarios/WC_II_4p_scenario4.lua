world_conquest_tek_scenario_res(4, 4, 33)
local generators = {
	wct_map_generator("classic", "4a", 64, 25, 8, 14000, 6, 8, 7),
	wct_map_generator("classic", "4b", 64, 25, 8, 14000, 6, 8, 7),
	wct_map_generator("provinces", "4c", 64, 25, 8, 14000, 6, 8, 7),
	wct_map_generator("podzol", "4d", 64, 25, 8, 14000, 6, 8, 7),
	wct_map_generator("wicked", "4e", 64, 25, 8, 19200, 5, 8, 7),
	wct_map_generator("wild", "4f", 64, 25, 8, 14000, 6, 8, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 150,
		sides = {
			wct_enemy(5, 2, 0, 5, 1, 11, 4),
			wct_enemy(6, 0, 1, 0, 0, enemy_power, 3),
			wct_enemy(7, 1, 1, 0, 0, enemy_power, 3),
			wct_enemy(8, 0, 0, 3, 0, enemy_power, 3),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 33, player_gold = 250 }
