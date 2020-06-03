world_conquest_tek_scenario_res(2, 6, 50)
local generators = {
	wct_map_generator("classic", "6a", 71, 19, 10, 16660, 8, 8, 7),
	wct_map_generator("maritime", "6b", 71, 19, 10, 16660, 8, 8, 7),
	wct_map_generator("industrial", "6c", 71, 19, 10, 18742, 7, 8, 7),
	wct_map_generator("feudal", "6d", 71, 19, 10, 16660, 8, 8, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 350,
		bonus_gold = 175,
		sides = {
			wct_enemy(4, 3, 9, 2, 0, 18, "$($wc2_difficulty.enemy_power-2)"),
			wct_enemy(5, 2, 8, 0, 0, "$($wc2_difficulty.enemy_power*2-1)", (enemy_power)),
			wct_enemy(6, 3, 1, 7, 0, "$($wc2_difficulty.enemy_power*2-1)", "$($wc2_difficulty.enemy_power-1)"),
			wct_enemy(7, 2, 1, 0, 0, "$($wc2_difficulty.enemy_power*2-1)", "$($wc2_difficulty.enemy_power-1)"),
			wct_enemy(8, 2, 0, 2, 1, "$($wc2_difficulty.enemy_power*2-1)", 9),
			wct_enemy(9, 2, 1, 4, 1, 17, "$($wc2_difficulty.enemy_power-1)"),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 50, player_gold = 400 }
