world_conquest_tek_scenario_res(1, 4, 44)
local generators = {
	wct_map_generator("classic", "4a", 51, 14, 8, 10560, 6, 5, 7),
	wct_map_generator("classic", "4b", 51, 14, 8, 10560, 6, 5, 7),
	wct_map_generator("provinces", "4c", 51, 14, 8, 10560, 6, 5, 7),
	wct_map_generator("podzol", "4d", 51, 14, 8, 10560, 6, 5, 7),
	wct_map_generator("wicked", "4e", 51, 14, 8, 14784, 5, 5, 7),
	wct_map_generator("wild", "4f", 51, 14, 8, 10560, 6, 5, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 150,
		sides = {
			wct_enemy(4, 2, 0, 5, 1, 5, 1),
			wct_enemy(5, 0, 1, 0, 0, "$($wc2_difficulty.enemy_power-4)", 1),
			wct_enemy(6, 1, 1, 0, 0, "$($wc2_difficulty.enemy_power-5)", 1),
			wct_enemy(7, 0, 0, 3, 0, "$($wc2_difficulty.enemy_power-4)", 1),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 44, player_gold = 350 }
