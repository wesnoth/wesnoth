world_conquest_tek_scenario_res(2, 2, 30)
local generators = {
	wct_map_generator("classic", "2a", 40, 20, 6, 8280, 4, 4, 7),
	wct_map_generator("classic", "2b", 40, 20, 6, 8280, 4, 4, 7),
	wct_map_generator("classic", "2c", 40, 20, 6, 8280, 4, 4, 7),
	wct_map_generator("provinces", "2d", 40, 20, 6, 8280, 4, 4, 7),
	wct_map_generator("paradise", "2e", 40, 20, 6, 8280, 4, 4, 7),
	wct_map_generator("clayey", "2f", 40, 20, 6, 8280, 4, 4, 7),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 135,
		sides = {
			wct_enemy(4, 0, 0, 0, 0, 2, 0),
			wct_enemy(5, 0, 1, 0, 0, 2, 0),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 30, player_gold = 200 }
