
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=90
	res.min_lake_height=250
	res.lake_size=60
	res.river_frequency=100
	res.temperature_size=7
	res.roads=25
	res.road_windiness=2

	res.height = {
		-- list of common terrain types which come in at different heights, from highest to lowest
		dr_height(910, "Uh"),
		dr_height(820, "Uu"),
		dr_height(780, "Xu"),
		dr_height(765, "Mm^Xm"),
		dr_height(725, "Mm"),
		dr_height(610, "Hh"),
		dr_height(600, "Gg"),
		dr_height(590, "Hh^Fp"),
		dr_height(580, "Gg"),
		dr_height(570, "Gs^Fp"),
		dr_height(410, "Gg"),
		dr_height(400, "Mm"),
		dr_height(360, "Gg"),
		dr_height(340, "Hh^Fp"),
		dr_height(320, "Gg"),
		dr_height(300, "Gs^Fp"),
		dr_height(240, "Gg"),
		dr_height(220, "Gs^Fp"),
		dr_height(200, "Hh^Fp"),
		dr_height(180, "Hh"),
		dr_height(100, "Gg"),
		dr_height(30, "Ds"),
		dr_height(1, "Wwg"),
		dr_height(0, "Wog"),
	}
	res.convert = {
		wct_fix_river_into_ocean("g", 29),
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		-- low temperatures
		dr_convert(50, 999, 0, 270, "Ww, Wo", "Ai"),
		dr_convert(350, 999, 0, 320, "Gg", "Aa"),
		dr_convert(330, 999, 320, 370, "Gg", "Gd"),
		dr_convert(100, 999, 370, 475, "Gg", "Gs"),
		dr_convert(350, 999, 0, 310, "Gs^Fp", "Aa^Fpa"),
		dr_convert(350, 999, 0, 345, "Hh", "Ha"),
		dr_convert(350, 999, 0, 335, "Hh^Fp", "Ha^Fpa"),
		dr_convert(350, 999, 0, 370, "Mm", "Ms"),
		dr_convert(350, 999, 0, 390, "Mm^Xm", "Ms^Xm"),
		-- swamp appears on low land, at mod temp
		dr_convert(0, 200, 400, 700, "Gg", "Ss"),
		-- pine appears at low temperatures
		dr_convert(150, 999, 320, 420, "Gg", "Gs^Fp"),
		dr_convert(150, 999, 320, 420, "Hh", "Hh^Fp"),
		-- decidius appears at mod temperatures with some heigh
		dr_convert(300, 999, 510, 540, "Gg,Gs", "Gg^Fds"),
		dr_convert(300, 999, 510, 540, "Hh", "Hh^Fds"),
		-- fungus appears at med temp and high
		dr_convert(825, 950, 500, 525, "Uu, Uh", "Uu^Tf"),
		dr_convert(825, 950, 550, 575, "Uu, Uh", "Uu^Tf"),
		dr_convert(825, 950, 600, 625, "Uu, Uh", "Uu^Tf"),
		-- high temperatures
		dr_convert(800, 999, 850, 999, "Uu, Uh, Uu^Tf", "Ql"),
		dr_convert(260, 999, 800, 999, "Gg", "Dd"),
		dr_convert(230, 999, 750, 999, "Gg", "Gd"),
		dr_convert(100, 999, 650, 999, "Gg", "Gs"),
		dr_convert(460, 630, 800, 999, "Ds, Hh", "Hd"),

		-- DR_TEMPERATURE FROM MIN MAX TO),
		-- convert forest at different temperatures
		dr_temperature("Gs^Fp", 420, 475, "Gs^Fdf"),
		dr_temperature("Hh^Fp", 420, 510, "Hh^Fmf"),
		dr_temperature("Gs^Fp", 475, 510, "Gg^Fdf"),
		dr_temperature("Gs^Fp", 510, 540, "Gg^Fds"),
		dr_temperature("Hh^Fp", 510, 540, "Hh^Fds"),
		dr_temperature("Gs^Fp", 540, 650, "Gg^Ftr"),
		dr_temperature("Hh^Fp", 540, 650, "Hh^Fms"),
	}
	res.road_cost = {
		wct_generator_road_cost_classic(),
		dr_road("Gs", "Re", 10),
		dr_road("Gd", "Re", 10),
		dr_road("Gg^Fds", "Re", 20),
		dr_road("Gg^Ftr", "Re", 20),
		dr_road("Gg^Fdf", "Re", 20),
		dr_road("Hh^Fmf", "Re", 20),
		dr_road("Hh^Ft", "Re", 30),
		dr_road("Hh^Fp", "Re", 30),
		dr_road("Hh^Fds", "Re", 20),
		dr_road("Hh^Fms", "Re", 20),
		dr_road("Gs^Ft", "Re", 30),
		dr_road("Ds", "Re", 20),
		dr_bridge("Ww", "Ww^Bsb", "Chw", 35),
		dr_road("Chw", "Chw", 2),
		dr_road_over_bridges("Ww^Bsb", 2),
	}
	res.village = {
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg",
			rating=8
		},
		dr_village {
			terrain = "Gs",
			convert_to="Gs^Vh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gd, Gd, Gs, Gs",
			rating=5
		},
		dr_village {
			terrain = "Gd",
			convert_to="Gd^Vc",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gd, Gd, Gs, Gs, Gd",
			rating=5
		},
		dr_village {
			terrain="Ds",
			convert_to="Dd^Vda",
			adjacent_liked="Gg, Gs, Gd, Wwg, Wwg, Wwg, Re, Re, Hh, Ch, Wog,",
			rating=2
		},
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vud",
			adjacent_liked="Re,Hh,Mm,Uu,Uh,Xu",
			rating=4
		},
		dr_village {
			terrain="Uh",
			convert_to="Uu^Vu",
			adjacent_liked="Re,Hh,Mm,Uu,Uh,Xu",
			rating=3
		},
		dr_village {
			terrain = "Gg^Fds",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gg^Fds",
			rating=4
		},
		dr_village {
			terrain="Gs^Fp",
			convert_to="Gs^Vh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gd, Gd, Gs, Gs, Gs^Fp",
			rating=4
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gd, Gd, Gs, Gs, Gs^Fp",
			rating=4
		},
		dr_village {
			terrain="Hh^Fp",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gd, Gd, Gs, Gs, Gs^Fp",
			rating=4
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vhh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bsb|, Ww^Bsb/, Ww^Bsb\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=4
		},
		-- villages in snow
		dr_village {
			terrain="Aa",
			convert_to="Aa^Vha",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bsb|, Ww^Bsb/, Ww^Bsb\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=3
		},
		dr_village {
			terrain="Aa^Fpa",
			convert_to="Aa^Vea",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bsb|, Ww^Bsb/, Ww^Bsb\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=3
		},
		-- swamp villages
		dr_village {
			terrain="Ss",
			convert_to="Ss^Vhs",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bsb|, Ww^Bsb/, Ww^Bsb\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=2
		},
		-- mermen villages - give them low chance of appearing
		dr_village {
			terrain="Wwg",
			convert_to="Wwg^Vm",
			adjacent_liked="Wwg, Wwg, Ch, Ss, Ds, Ds",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gs, Gg, Gd, Gs^Fp, Gg^Ft, Gg^Fds, Gg^Ftr, Hh, Hh^Fp, Hh^Ft, Hh^Fds, Hh^Fms, Ha^Fpa, Ha, Hh^Fmf, Gg^Fdf",
		min_distance=15,
	}

	return default_generate_map(res)
end
return generate
