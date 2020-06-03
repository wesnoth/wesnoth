world_conquest_tek_scenario_res(1, 3, 38)
local generators = {
	wct_map_generator("savannah", "3a", 45, 15, 7, 9680, 5, 4, 7),
	wct_map_generator("wreck", "3b", 45, 15, 7, 9680, 5, 4, 7),
	wct_map_generator("delta", "3c", 45, 14, 7, 9196, 3, 4, 7),
	wct_map_generator("sulfurous", "3d", 45, 15, 7, 9680, 5, 4, 7),
	wct_map_generator("coral", "3e", 45, 15, 7, 9680, 5, 4, 7),
	wct_map_generator("wetland", "3f", 45, 15, 7, 14520, 4, 4, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 140,
		sides = {
			wct_enemy(4, 0, 0, 0, 0, 2, 1),
			wct_enemy(5, 0, 0, 6, 0, 2, 1),
			wct_enemy(6, 1, 1, 0, 0, "$($wc2_difficulty.enemy_power-6)", 1),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 38, player_gold = 300 }
