
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=10
	res.min_lake_height=150
	res.lake_size=125
	res.river_frequency=100
	res.temperature_size=8
	res.roads=12
	res.road_windiness=10

	res.height = {
		dr_height(900, "Uh"),
		dr_height(800, "Uu"),
		dr_height(750, "Xu"),
		dr_height(725, "Mm^Xm"),
		dr_height(675, "Mm"),
		dr_height(550, "Hh"),
		dr_height(175, "Gs"),
		dr_height(30, "Ds"),
		dr_height(1, "Ww"),
		dr_height(0, "Wo"),
	}
	res.convert = {
		-- swamp appears on low land, at moderate temperatures
		dr_convert(nil, 300, 300, 700, "Gg,Gs", "Ss"),
		-- jungle appears at moderately high temperatures
		dr_temperature("Gg,Gs", 380, 430, "Gs^Ft"),
		dr_temperature("Gg,Gs", 460, 520, "Gs^Ft"),
		-- fungus appears at medium temperatures and extremely high elevation
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		dr_convert(825, 950, 500, 525, "Uu, Uh", "Uu^Tf"),
		dr_convert(825, 950, 550, 575, "Uu, Uh", "Uu^Tf"),
		dr_convert(825, 950, 600, 625, "Uu, Uh", "Uu^Tf"),
		-- lava appears at extreme temperatures and elevation
		dr_convert(800, nil, 900, nil, "Uu, Uh, Uu^Tf", "Ql"),
		-- desert appears at high temperatures
		dr_temperature("Gs", 475, 500, "Dd"),
		dr_temperature("Gs", 600, 650, "Dd"),
		dr_temperature("Gs", 725, 775, "Dd"),
		dr_temperature("Gs", 800, 999, "Dd"),
		-- dunes appear at extreme temperatures
		dr_temperature("Hh", 625, 650, "Hd"),
		dr_temperature("Hh", 750, 775, "Hd"),
		dr_temperature("Hh", 825, 999, "Hd"),
	}
	res.road_cost = {
		wct_generator_road_cost_classic(),
		dr_road("Ds", "Re", 15),
		dr_road("Dd", "Re", 15),
		dr_road("Gs^Ft", "Re", 20),
		dr_bridge("Ww", "Ww^Bw", "Ce", 50),
	}
	res.village = {
		wct_generator_village(2, 8, 8, 4, 3, 2, 6, 3, 3, 3, 5, 1)
	}
	res.castle = {
		valid_terrain="Gs, Gg, Gs^Fp, Hh, Ds, Dd",
		min_distance=12,
	}

	return default_generate_map(res)
end
return generate
