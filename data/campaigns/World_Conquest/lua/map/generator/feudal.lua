
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=40
	res.min_lake_height=450
	res.lake_size=35
	res.river_frequency=80
	res.temperature_size=size
	res.roads=20
	res.road_windiness=4

	res.height = {
		dr_height(800, "Uu"),
		dr_height(750, "Xu"),
		dr_height(725, "Mm^Xm"),
		dr_height(690, "Mm"),
		dr_height(580, "Hh"),
		dr_height(200, "Gg"),
		dr_height(100, "Wot"), -- mark anti castle generation
		dr_height(35, "Ds"),
		dr_height(1, "Wwg"),
		dr_height(0, "Wog"),
	}
	res.convert = {
		wct_fix_river_into_ocean("g", 33),
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		-- low temperatures
		dr_convert(100, 999, 0, 400, "Ww", "Ai"),
		dr_convert(500, 999, 0, 420, "Ww", "Ai"),
		dr_convert(300, 999, 0, 410, "Gg", "Aa"),
		dr_convert(420, 999, 0, 425, "Gg", "Aa"),
		dr_convert(580, 999, 0, 430, "Hh", "Ha"),
		dr_convert(580, 999, 0, 435, "Mm", "Ms"),
		dr_convert(580, 999, 0, 440, "Mm^Xm", "Ms^Xm"),
		-- desert appears at extreme temperatures and medium altitude
		dr_convert(250, 530, 800, 999, "Gg", "Dd"),
		-- swamps
		dr_convert(450, 500, 500, 575, "Gg", "Ss"),
		dr_convert(300, 350, 475, 525, "Gg", "Ss"),
		dr_convert(100, 200, 450, 500, "Gg,Wot", "Ss"),

		dr_temperature("Gg", 0, 460, "Gs"),
		dr_temperature("Hh", 470, 999, "Hhd"), -- mark for forst type
	}
	res.road_cost = {
		dr_road("Gg", "Re", 8),
		dr_road("Gs", "Re", 9),
		dr_road("Ss", "Ce", 20),
		dr_road("Hh", "Re", 20),
		dr_road("Hhd", "Re", 20),
		dr_road("Mm", "Re", 35),
		dr_road("Mm^Xm", "Re", 50),
		dr_road("Uu", "Re", 10),
		dr_road("Xu", "Re", 50),
		dr_road("Aa", "Coa", 20),
		dr_road("Ha", "Re", 20),
		dr_road("Dd", "Cd", 20),
		dr_road("Hd", "Re", 20),
		dr_bridge("Ww", "Ww^Bw", "Ce", 45),
		dr_road("Re", "Re", 2),
		dr_road_over_bridges("Ww^Bw", 2),
		dr_road("Ch", "Ch", 2),
		dr_road("Ce", "Ce", 2),
		dr_road("Cd", "Cd", 2),
		dr_road("Coa", "Coa", 2),
	}
	res.village = {
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vc",
			adjacent_liked="Gg, Gg, Gg, Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Hh, Hhd",
			rating=8
		},
		dr_village {
			terrain = "Gs",
			convert_to="Gg^Vc",
			adjacent_liked="Gg, Gg, Gs, Gs, Gs, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Hh, Hhd",
			rating=8
		},
		dr_village {
			terrain="Ds",
			convert_to="Ds^Vda",
			adjacent_liked="Gg, Ds, Ds, Wwg, Wwg, Wwg, Ww, Hhd",
			rating=2
		},
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vud",
			adjacent_liked="Re,Hh,Hhd,Mm,Uu,Uu,Uu,Xu",
			rating=4
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vhh",
			adjacent_liked="Gs, Gs, Gs, Gs, Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Hh, Hh, Hhd",
			rating=4
		},
		dr_village {
			terrain="Hhd",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gg, Gg, Gg, Gs, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Hhd, Hhd, Hhd",
			rating=5
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vhh",
			adjacent_liked="Gg, Gg, Gg, Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Hh, Hh, Hhd, Hhd",
			rating=3
		},
		-- villages in snow
		dr_village {
			terrain="Aa",
			convert_to="Aa^Voa",
			adjacent_liked="Gs, Gs, Aa, Aa, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Hh, Ha, Ha",
			rating=3
		},
		-- villages in dessert
		dr_village {
			terrain="Dd",
			convert_to="Dd^Vda",
			adjacent_liked="Gg, Gg, Dd, Dd, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Hhd, Hhd",
			rating=3
		},
		-- swamp villages
		dr_village {
			terrain="Ss",
			convert_to="Ss^Vhs",
			adjacent_liked="Gg, Gg, Gs, Gs, Ss, Ss, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Hh, Hh, Hhd, Hhd",
			rating=2
		},
		-- mermen villages - give them low chance of appearing
		dr_village {
			terrain="Wwg",
			convert_to="Wwg^Vm",
			adjacent_liked="Wwg, Wwg",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gs, Gg, Hh, Hhd, Mm, Aa, Ha",
		min_distance=13,
	}

	return default_generate_map(res)
end
return generate
