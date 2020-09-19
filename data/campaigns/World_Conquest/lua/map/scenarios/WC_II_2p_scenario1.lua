world_conquest_tek_scenario_res(2, 1, 25)
local generators = {
	wct_map_generator("classic", "1a", 30, 20, 5, 7200, 3, 3, 2),
}

local function get_enemy_data(enemy_power)
	return {
		gold = 300,
		bonus_gold = 150,
		sides = {
			wct_enemy(4, 1, 0, 0, 0, 2, 0),
		}
	}
end
return { generators = generators, get_enemy_data = get_enemy_data, turns = 25, player_gold = 150 }
