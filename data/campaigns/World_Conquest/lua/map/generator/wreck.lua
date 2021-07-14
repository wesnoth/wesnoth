
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments(length, villages, castle, iterations, size, players, island)
	res.max_lakes=50
	res.min_lake_height=150
	res.lake_size=125
	res.river_frequency=100
	res.temperature_size=9
	res.roads=12
	res.road_windiness=10

	res.height = {
		dr_height(900, "Uh"),
		dr_height(800, "Uu"),
		dr_height(750, "Xu"),
		dr_height(725, "Mm^Xm"),
		dr_height(675, "Mm"),
		dr_height(550, "Hh"),
		dr_height(300, "Gs"),
		dr_height(250, "Ds"),
		dr_height(200, "Ss"),
		dr_height(150, "Ds"),
		dr_height(100, "Ss"),
		dr_height(30, "Ds"),
		dr_height(1, "Ww"),
		dr_height(0, "Wo"),
	}
	res.convert = {
		-- swamp appears on low land, at moderate temperatures
		dr_convert(nil, 375, 300, 700, "Gs", "Ss"),
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		dr_convert(425, 475, 300, 700, "Gs", "Ss"),
		-- jungle appears at moderate temperatures
		dr_convert(nil, nil, 200, 700, "Gg,Gs", "Gs^Ft"),
		-- fungus appears at medium temperatures and extremely high elevation
		dr_convert(825, 950, 475, 525, "Uu, Uh", "Uu^Tf"),
		dr_convert(825, 950, 550, 600, "Uu, Uh", "Uu^Tf"),
		dr_convert(825, 950, 625, 675, "Uu, Uh", "Uu^Tf"),
		-- lava appears at extreme temperatures and elevation
		dr_convert(800, nil, 850, nil, "Uu, Uh, Uu^Tf", "Ql"),
		-- desert appears at extreme temperatures
		dr_convert(nil, nil, 800, nil, "Gg", "Ds"),
		-- dunes appear at extreme temperatures and moderate elevation
		dr_convert(475, 550, 800, nil, "Ds, Hh", "Hd"),
	}

	res.road_cost = {
		wct_generator_road_cost_classic(),
		dr_road("Gs^Ft", "Re", 20),
		dr_road("Ds", "Re", 25),
		dr_bridge("Ww", "Ww^Bw", "Chs", 50),
		dr_bridge("Ss", "Ss^Bw", "Chs", 10),
	}
	res.village = {
		wct_generator_village(5, 8, 4, 4, 3, 5, 4, 3, 3, 3, 8, 1)
	}
	res.castle = {
		valid_terrain="Gs, Gg, Gs^Fp, Hh, Gs^Ft, Ss",
		min_distance=12,
	}
	return default_generate_map(res)
end
return generate
