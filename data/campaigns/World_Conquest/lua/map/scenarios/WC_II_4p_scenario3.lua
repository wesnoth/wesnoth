world_conquest_tek_scenario_res(4, 3, 25)
local generators = {
	wct_map_generator("savannah", "3a", 60, 23, 7, 12000, 5, 7, 7),
	wct_map_generator("wreck", "3b", 60, 23, 7, 12000, 5, 7, 7),
	wct_map_generator("delta", "3c", 60, 23, 7, 12000, 3, 7, 7),
	wct_map_generator("sulfurous", "3d", 60, 23, 7, 12000, 5, 7, 7),
	wct_map_generator("coral", "3e", 60, 23, 7, 12000, 5, 7, 7),
	wct_map_generator("wetland", "3f", 60, 23, 7, 17500, 4, 7, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 140,
		sides = {
			wct_enemy(5, 0, 0, 0, 0, 6, 1),
			wct_enemy(6, 0, 0, 6, 0, 6, 1),
			wct_enemy(7, 1, 1, 0, 0, "$($wc2_difficulty.enemy_power-3)", 4),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 30, player_gold = 200 }
