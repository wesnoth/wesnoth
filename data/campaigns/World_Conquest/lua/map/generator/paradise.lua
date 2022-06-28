
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=5
	res.min_lake_height=900
	res.lake_size=1
	res.river_frequency=1
	res.temperature_size=size
	res.roads=25
	res.road_windiness=4

	res.height = {
		-- list of common terrain types which come in at different heights, from highest to lowest
		dr_height(990, "Rr"),
		dr_height(980, "Ww"),
		dr_height(960, "Rr"),
		dr_height(950, "Xos"),
		dr_height(940, "Rr^Fet"),
		dr_height(910, "Xos"),
		dr_height(870, "Rr"),
		dr_height(855, "Rr^Fet"),
		dr_height(850, "Ww"),
		dr_height(840, "Hh^Fp"),
		dr_height(830, "Hh"),
		dr_height(820, "Mm"),
		dr_height(810, "Gg"),
		dr_height(785, "Gg^Fp"),
		dr_height(775, "Gg"),
		dr_height(760, "Hh^Fp"),
		dr_height(750, "Gg"),
		dr_height(730, "Hh"),
		dr_height(720, "Mm"),
		dr_height(685, "Gg"),
		dr_height(670, "Gg^Fp"),
		dr_height(620, "Gg"),
		dr_height(610, "Hh^Fp"),
		dr_height(560, "Gg"),
		dr_height(550, "Hh"),
		dr_height(500, "Gg"),
		dr_height(485, "Mm"),
		dr_height(450, "Hh"),
		dr_height(400, "Hh^Fp"),
		dr_height(350, "Gg^Fp"),
		dr_height(230, "Gg"),
		dr_height(220, "Gg^Fp"),
		dr_height(85, "Gg"),
		dr_height(30, "Ds"),
		dr_height(20, "Ww"),
		dr_height(17, "Wwr"),
		dr_height(1, "Ww"),
		dr_height(0, "Wo"),
	}
	res.convert = {
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		dr_convert(70, 150, 10, 300, "Gg", "Ss"),
		dr_convert(550, 680, 350, 370, "Gg", "Hh^Tf"),
		dr_convert(550, 680, 620, 640, "Gg", "Hh^Tf"),

		-- DR_TEMPERATURE FROM MIN MAX TO),
		-- convert forest at different temperatures
		dr_temperature("Gs^Fp", 700, 990, "Gg^Ft"),
		dr_temperature("Hh^Fp", 700, 990, "Hh^Ft"),
		dr_temperature("Gg^Fp", 400, 700, "Gg^Ftp"),
		dr_temperature("Hh^Fp", 400, 700, "Hh^Ftp"),
		dr_temperature("Gg^Fp", 250, 400, "Gg^Ftr"),
		dr_temperature("Hh^Fp", 250, 400, "Hh^Ftd"),
		dr_temperature("Gg^Fp", 125, 250, "Gg^Fds"),
		dr_temperature("Hh^Fp", 125, 250, "Hh^Fds"),
		dr_temperature("Gg^Fp", 10, 125, "Gg^Fms"),
		dr_temperature("Hh^Fp", 10, 125, "Hh^Fms"),
	}
	res.road_cost = {
		-- try to conect citadels
		dr_bridge("Rr", "Ww^Bsb", "Rr", 1),
		dr_road("Ww^Bsb", "Ww^Bsb", 3),
		dr_road("Rr^Fet", "Rr^Fet", 1),
		dr_road("Xos", "Xos", 1),
		-- flat
		dr_road("Ch", "Ch", 2),
		dr_road("Re", "Re", 2),
		dr_road("Gg", "Re", 10),
		-- forest
		dr_road("Gg^Ftp", "Re", 20),
		dr_road("Gg^Fms", "Re", 20),
		dr_road("Gg^Fds", "Re", 20),
		dr_road("Gg^Ft", "Re", 20),
		dr_road("Gg^Ftr", "Re", 20),
		dr_road("Hh^Ftp", "Re", 20),
		dr_road("Hh^Fms", "Re", 20),
		dr_road("Hh^Fds", "Re", 20),
		dr_road("Hh^Ft", "Re", 20),
		dr_road("Hh^Ftd", "Re", 20),
		-- rough
		dr_road("Hh", "Re", 20),
		dr_road("Mm", "Re", 40),
		dr_road("Ds", "Re", 25),
		dr_bridge("Ww", "Ww^Bw", "Ch", 50),
	}
	res.village = {
		dr_village {
			terrain = "Rr",
			convert_to="Rr^Vhc",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=10
		},
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=5
		},
		dr_village {
			terrain = "Hh^Fms",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Rr, Rr, Re, Re, Rr^Fet, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=2
		},
		dr_village {
			terrain = "Hh^Fds",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Rr, Rr, Re, Re, Rr^Fet, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=2
		},
		dr_village {
			terrain = "Hh^Ftp",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Rr, Rr, Re, Re, Rr^Fet, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=2
		},
		dr_village {
			terrain = "Hh^Ft",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Rr, Rr, Re, Re, Rr^Fet, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=2
		},
		dr_village {
			terrain = "Hh^Ftd",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Rr, Rr, Re, Re, Rr^Fet, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=3
		},
		dr_village {
			terrain="Ds",
			convert_to="Ds^Vc",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Hh, Rr^Fet, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=2
		},
		dr_village {
			terrain="Gg^Fms",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=3
		},
		dr_village {
			terrain="Gg^Fds",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=3
		},
		dr_village {
			terrain="Gg^Ft",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=3
		},
		dr_village {
			terrain="Gg^Ftp",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=3
		},
		dr_village {
			terrain="Gg^Ftr",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=3
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=4
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vhh",
			adjacent_liked="Gg, Gg^Ftr, Gg^Ftp, Gg^Fds, Gg^Fms, Gg^Ftd, Gg^Ft, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Rr^Fet, Gg^Vh, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Xos",
			rating=3
		},
		dr_village {
			terrain="Ss",
			convert_to="Ss^Vhs",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=1
		},
		dr_village {
			terrain="Ww",
			convert_to="Ww^Vm",
			adjacent_liked="Ww, Ww",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gg, Gg^Fp, Gg^Ft, Gg^Ftr, Gg^Ftp, Gg^Ftd, Gg^Fds, Gg^Fms, Hh, Hh^Ft, Hh^Ftr, Hh^Ftp, Hh^Ftd, Hh^Fds, Hh^Fms, Mm, Hh^Fp",
		min_distance=14,
	}

	return default_generate_map(res)
end
return generate
