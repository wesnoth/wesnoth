
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=10
	res.min_lake_height=150
	res.lake_size=125
	res.river_frequency=100
	res.temperature_size=size
	res.roads=20
	res.road_windiness=3

	res.height = {
		-- list of common terrain types which come in at different heights, from highest to lowest
		dr_height(900, "Uh"),
		dr_height(800, "Uu"),
		dr_height(750, "Xu"),
		dr_height(725, "Mm^Xm"),
		dr_height(675, "Mm"),
		dr_height(550, "Hh"),
		dr_height(100, "Gg"),
		dr_height(30, "Ds"),
		dr_height(1, "Ww"),
		dr_height(0, "Wo"),
	}
	res.convert = {
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		-- water at cold becomes ice
		dr_convert(50, 999, 0, 50, "Ww, Wo", "Ai"),
		-- at low temperatures, snow appears
		dr_convert(50, 999, 0, 75, "Gg, Ds", "Aa"),
		-- hills at cold get snow on them
		dr_convert(0, 999, 0, 100, "Hh", "Ha"),
		-- savannah appears on mod temp and high
		dr_convert(250, 400, 500, 800, "Gg", "Gs"),
		-- swamp appears on low land, at mod temp
		dr_convert(0, 200, 400, 700, "Gg", "Ss"),
		-- forest appears at moderate temperatures
		dr_convert(0, 999, 320, 420, "Gg", "Gs^Fp"),
		dr_convert(0, 999, 320, 420, "Hh", "Hh^Fp"),
		-- jungle appears at mod high temperatures
		dr_convert(0, 999, 450, 520, "Gg,Gs", "Gs^Ft"),
		dr_convert(0, 999, 450, 520, "Hh", "Hh^Ft"),
		-- fungus appears at med temp and high
		dr_convert(825, 950, 500, 525, "Uu, Uh", "Uu^Tf"),
		dr_convert(825, 950, 550, 575, "Uu, Uh", "Uu^Tf"),
		dr_convert(825, 950, 600, 625, "Uu, Uh", "Uu^Tf"),
		-- lava appears at extreme temp and height
		dr_convert(800, 999, 850, 999, "Uu, Uh, Uu^Tf", "Ql"),
		-- desert appears at extreme temperatures
		dr_convert(0, 999, 800, 999, "Gg", "Dd"),
		-- dunes at extreme temp and mod elevation
		dr_convert(475, 550, 800, 999, "Ds, Hh", "Hd"),
	}
	res.road_cost = {
		wct_generator_road_cost_classic(),
		dr_road("Gs^Ft", "Re", 30),
		dr_road("Ds", "Re", 25),
		dr_bridge("Ww", "Ww^Bw", "Ce", 50),
	}
	res.village = {
		wct_generator_village(8, 5, 2, 4, 3, 4, 4, 3, 3, 3, 2, 1)
	}
	res.castle = {
		valid_terrain="Gs, Gg, Gs^Fp, Hh",
		min_distance=12,
	}
	return default_generate_map(res)
end
return generate
