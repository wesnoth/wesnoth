world_conquest_tek_scenario_res(3, 4, 36)
local generators = {
	wct_map_generator("classic", "4a", 60, 22, 8, 13000, 6, 7, 7),
	wct_map_generator("classic", "4b", 60, 22, 8, 13000, 6, 7, 7),
	wct_map_generator("provinces", "4c", 60, 22, 8, 13000, 6, 7, 7),
	wct_map_generator("podzol", "4d", 60, 22, 8, 13000, 6, 7, 7),
	wct_map_generator("wicked", "4e", 60, 22, 8, 18200, 5, 7, 7),
	wct_map_generator("wild", "4f", 60, 22, 8, 13000, 6, 7, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 150,
		sides = {
			wct_enemy(4, 2, 0, 5, 1, 9, 3),
			wct_enemy(5, 0, 1, 0, 0, enemy_power, 3),
			wct_enemy(6, 1, 1, 0, 0, enemy_power, 3),
			wct_enemy(7, 0, 0, 3, 0, enemy_power, 3),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 36, player_gold = 275 }
