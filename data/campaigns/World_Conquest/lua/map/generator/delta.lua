
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=10
	res.min_lake_height=150
	res.lake_size=125
	res.river_frequency=100
	res.temperature_size=4
	res.roads=10
	res.road_windiness=7

	res.height = {
		dr_height(960, "Uue^Dr"),
		dr_height(910, "Uue"),
		dr_height(870, "Uue^Dr"),
		dr_height(800, "Uue"),
		dr_height(700, "Xuce"),
		dr_height(625, "Mm"),
		dr_height(475, "Hh"),
		dr_height(310, "Gg"),
		dr_height(300, "Ds"),
		dr_height(200, "Ww"),
		dr_height(0, "Wo"),
	}
	res.convert = {
		-- sand
		dr_convert(75, nil, nil, 200, "Ww,Wo", "Dd^Do"),
		dr_convert(180, nil, nil, 300, "Gg,Ds", "Dd"),
		dr_convert(500, nil, nil, 425, "Hh", "Hd"),
		dr_convert(nil, nil, 900, nil, "Gg", "Ds"),
		-- swamp
		dr_convert(nil, 200, 600, 900, "Gg", "Ss"),
		-- forest
		dr_convert(nil, nil, 240, 320, "Dd, Gs", "Ds^Ftd"),
		dr_convert(nil, nil, 350, 420, "Gg", "Gs^Fp"),
		-- fungus
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		dr_convert(825, 950, 500, 525, "Uue, Uue^Dr", "Uue^Tf"),
		dr_convert(825, 950, 550, 575, "Uue, Uue^Dr", "Uue^Tf"),
		dr_convert(825, 950, 600, 625, "Uue, Uue^Dr", "Uue^Tf"),
		-- lava
		dr_convert(800, nil, 850, nil, "Uue, Uue^Dr, Uue^Tf", "Ql"),
	}
	res.road_cost = {
		dr_road("Gg", "Re", 10),
		dr_road("Gs^Fp", "Re", 20),
		dr_road("Hh", "Re", 30),
		dr_road("Mm", "Re", 40),
		dr_road("Xuce", "Re", 80),
		dr_road("Uue", "Re", 10),
		dr_road("Uue^Dr", "Re", 40),
		dr_road("Ds", "Re", 25),
		dr_bridge("Ww", "Ww^Bw", "Ce", 50),
		dr_road("Re", "Re", 2),
		dr_road("Ce", "Ce", 2),
		dr_road_over_bridges("Ww^Bw", 2),
	}
	res.village = {
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Ds^Ftd, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Hh, Gs^Fp",
			rating=5
		},
		dr_village {
			terrain="Ds",
			convert_to="Ds^Vda",
			adjacent_liked="Gg, Ds^Ftd, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Hh, Gs^Fp",
			rating=4
		},
		dr_village {
			terrain="Dd",
			convert_to="Dd^Vda",
			adjacent_liked="Gg, Ds^Ftd, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Hh, Gs^Fp, Dd, Dd, Dd^Do",
			rating=4
		},
		dr_village {
			terrain="Ds^Ftd",
			convert_to="Ds^Vda",
			adjacent_liked="Gg, Ds^Ftd, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Hh, Gs^Fp, Dd, Dd, Dd^Do",
			rating=3
		},
		dr_village {
			terrain="Dd^Do",
			convert_to="Ds^Vdt",
			adjacent_liked="Gg, Ds^Ftd, Ds^Ftd, Re, Re, Hh, Gs^Fp, Dd, Dd, Dd^Do, Dd^Do, Ds",
			rating=1
		},
		dr_village {
			terrain="Uue",
			convert_to="Uue^Vu",
			adjacent_liked="Re,Hh,Mm,Uue,Uue^Dr,Xuce",
			rating=4
		},
		dr_village {
			terrain="Uue^Dr",
			convert_to="Uue^Vu",
			adjacent_liked="Re,Hh,Mm,Uue,Uue^Dr,Xuce",
			rating=3
		},
		dr_village {
			terrain="Gs^Fp",
			convert_to="Gg^Ve",
			adjacent_liked="Gg, Ds^Ftd, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Hh, Gs^Fp, Gs^Fp",
			rating=4
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vh",
			adjacent_liked="Gg, Ds^Ftd, Hh, Hh, Mm, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Hh, Gs^Fp, Hh",
			rating=8
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vd",
			adjacent_liked="Gg, Ds^Ftd, Hh, Hh, Mm, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Hh, Gs^Fp, Hh",
			rating=3
		},
		dr_village {
			terrain="Ss",
			convert_to="Ss^Vhs",
			adjacent_liked="Gg, Ds^Ftd, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Hh, Gs^Fp",
			rating=2
		},
		dr_village {
			terrain="Ww",
			convert_to="Ww^Vm",
			adjacent_liked="Ww, Ww",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gg, Gs^Fp, Hh, Dd, Hd, Mm, Mm^Xm, Ds, Ss",
		min_distance=13,
	}

	return default_generate_map(res)
end
return generate
