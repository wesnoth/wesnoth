world_conquest_tek_scenario_res(2, 3, 35)
local generators = {
	wct_map_generator("savannah", "3a", 50, 19, 7, 10340, 5, 5, 7),
	wct_map_generator("wreck", "3b", 50, 19, 7, 10340, 5, 5, 7),
	wct_map_generator("delta", "3c", 50, 18, 7, 9823, 3, 5, 7),
	wct_map_generator("sulfurous", "3d", 50, 19, 7, 10340, 5, 5, 7),
	wct_map_generator("coral", "3e", 50, 19, 7, 10340, 5, 5, 7),
	wct_map_generator("wetland", "3f", 50, 19, 7, 15510, 4, 5, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 140,
		sides = {
			wct_enemy(4, 0, 0, 0, 0, 4, 1),
			wct_enemy(5, 0, 0, 6, 0, 4, 1),
			wct_enemy(6, 1, 1, 0, 0, "$($wc2_difficulty.enemy_power-5)", 2),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 35, player_gold = 250 }
