world_conquest_tek_scenario_res(3, 3, 32)
local generators = {
	wct_map_generator("savannah", "3a", 55, 21, 7, 11000, 5, 6, 7),
	wct_map_generator("wreck", "3b", 55, 21, 7, 11000, 5, 6, 7),
	wct_map_generator("delta", "3c", 55, 21, 7, 11000, 3, 6, 7),
	wct_map_generator("sulfurous", "3d", 55, 21, 7, 11000, 5, 6, 7),
	wct_map_generator("coral", "3e", 55, 21, 7, 11000, 5, 6, 7),
	wct_map_generator("wetland", "3f", 55, 21, 7, 16500, 4, 6, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 140,
		sides = {
			wct_enemy(4, 0, 0, 0, 0, 6, 1),
			wct_enemy(5, 0, 0, 6, 0, 6, 1),
			wct_enemy(6, 1, 1, 0, 0, "$($wc2_difficulty.enemy_power-4)", 3),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 32, player_gold = 225 }
