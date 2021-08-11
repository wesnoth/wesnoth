
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=30
	res.min_lake_height=170
	res.lake_size=75
	res.river_frequency=80
	res.temperature_size=9
	res.roads=11
	res.road_windiness=8

	res.height = {
		dr_height(990, "Qxu"),
		dr_height(960, "Uu"),
		dr_height(900, "Uh"),
		dr_height(825, "Uu"),
		dr_height(775, "Xu"),
		dr_height(750, "Mm^Xm"),
		dr_height(720, "Mm"),
		dr_height(670, "Hhd"),
		dr_height(650, "Hh^Fp"),
		dr_height(630, "Hhd^Tf"),
		dr_height(610, "Gll^Fp"),
		dr_height(590, "Gll^Tf"),
		dr_height(580, "Hhd"),
		dr_height(400, "Gll"),
		dr_height(395, "Hh"),
		dr_height(360, "Ss"),
		dr_height(340, "Gll^Ftr"),
		dr_height(330, "Hh"),
		dr_height(320, "Hh^Tf"),
		dr_height(300, "Hh^Fp"),
		dr_height(280, "Ss"),
		dr_height(270, "Hh"),
		dr_height(250, "Gll^Fp"),
		dr_height(225, "Gll^Tf"),
		dr_height(175, "Gll"),
		dr_height(70, "Ds"),
		dr_height(65, "Wwrt"),
		dr_height(35, "Wwt"),
		dr_height(30, "Wwrt"),
		dr_height(1, "Wwt"),
		dr_height(0, "Wot"),
	}
	res.convert = {
		wct_fix_river_into_ocean("t", 65),

		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		dr_convert(360, 999, 10, 400, "Ww", "Ai"),
		dr_convert(475, 750, 10, 415, "Gll^Fp", "Aa^Fpa"),
		dr_convert(450, 750, 10, 425, "Gll", "Aa"),
		dr_convert(410, 750, 10, 465, "Gll,Gll^Tf", "Gd"),
		dr_convert(650, 750, 10, 540, "Hh,Hhd", "Ha"),
		dr_convert(650, 750, 10, 510, "Hh^Fp", "Ha^Fpa"),
		dr_convert(650, 750, 10, 415, "Hh^Tf,Hhd^Tf", "Ha^Tf"),
		dr_convert(375, 750, 10, 390, "Ss", "Ai"),
		dr_convert(450, 750, 650, 675, "Gll", "Gll^Fet"),
		dr_convert(100, 300, 800, 850, "Gll", "Gll^Em"),
		dr_convert(400, 750, 680, 700, "Gll", "Gll^Efm"),
		dr_convert(475, 505, 725, 750, "Gll", "Ce"),
		dr_convert(585, 595, 10, 440, "Aa,Gll^Tf", "Ms"),
		dr_convert(575, 585, 10, 440, "Aa", "Ha"),
		dr_convert(475, 600, 870, 900, "Gll,Gll^Tf", "Ql"),

		-- DR_TEMPERATURE FROM MIN MAX TO),
		dr_temperature("Mm", 10, 570, "Ms"),
		dr_temperature("Mm^Xm", 10, 585, "Ms^Xm"),
		dr_temperature("Ds", 870, 900, "Ds^Edpp"),
		dr_temperature("Uu", 100, 300, "Ai"),
		dr_temperature("Uu", 650, 700, "Uu^Tf"),
		dr_temperature("Uh", 675, 725, "Uh^Tf"),
		dr_temperature("(Uu,Uh,Uu^Tf)", 850, 999, "Ql"),
		dr_temperature("Qxu", 800, 999, "Ql"),
	}
	res.road_cost = {
		dr_road("Gll", "Rb", 10),
		dr_road("Ce", "Rb", 20),
		dr_road("Gll^Fp", "Rb", 20),
		dr_road("Gll^Ftr", "Rb", 25),
		dr_road("Gll^Fet", "Rb", 15),
		dr_road("Ds", "Rb", 40),
		dr_road("Ww", "Wwf", 50),
		dr_road("Gll^Tf", "Rb", 15),
		dr_road("Hh^Tf", "Rb", 15),
		dr_road("Hhd^Tf", "Rb", 15),
		dr_road("Ss", "Rb", 15),
		dr_road("Aa", "Rb", 10),
		dr_road("Hh", "Rb", 20),
		dr_road("Hhd", "Rb", 20),
		dr_road("Hh^Fp", "Rb", 25),
		dr_road("Ha", "Rb", 25),
		dr_road("Ha^Fpa", "Rb", 30),
		dr_road("Mm", "Rb", 50),
		dr_road("Ms", "Rb", 55),
		dr_bridge("Ql", "Ql^Bsb", "Cud", 45),
		dr_road("Uu", "Rb", 35),
		dr_road("Uu^Tf", "Rb", 40),
		dr_road("Uh", "Rb", 45),
		dr_road("Xu", "Rb", 60),
		dr_road("Rb", "Rb", 2),
		dr_road_over_bridges("Ql^Bsb", 2),
	}
	res.village = {
		dr_village {
			terrain = "Gll",
			convert_to="Gll^Vh",
			adjacent_liked="Gll, Ce, Ww, Ww, Ww, Ww, Ww, Wwt, Wwf, Wwf, Rb, Rb, Rb, Rb, Gll, Gll, Hh, Gll^Fp",
			rating=5
		},
		dr_village {
			terrain = "Gd",
			convert_to="Gd^Vc",
			adjacent_liked="Gll, Gll^Fet, Ww, Ww, Ww, Ww, Wwt, Wwf, Wwf, Rb, Rb, Ce, Gd, Gd, Gll, Hh, Hh^Fp, Gll^Fp",
			rating=5
		},
		dr_village {
			terrain="Gll^Fp",
			convert_to="Gll^Ve",
			adjacent_liked="Gll, Gll^Fet, Ww, Ww, Ww, Ww, Ww, Wwt, Wwf, Wwf, Rb, Rb, Rb, Ce, Gd, Gll, Hh, Hh^Fp, Gll^Fp",
			rating=5
		},
		dr_village {
			terrain="Gll^Fet",
			convert_to="Gll^Ve",
			adjacent_liked="Gll, Gll^Fet, Ww, Ww, Ww, Ww, Ww, Wwt, Wwf, Wwf, Rb, Rb, Rb, Ce, Gd, Gll, Hh, Hh^Fp, Gll^Fp, Gll^Fp, Gll^Fet, Gll^Fet",
			rating=7
		},
		dr_village {
			terrain="Ds",
			convert_to="Ds^Vda",
			adjacent_liked="Gll, Gll, Wwt, Wwt, Wwt, Wwt, Wwt, Rb, Rb, Rb, Rb, Gll, Gll, Hh, Hh^Fp, Gll^Fp, Gll^Fet, Ds",
			rating=4
		},
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vo",
			adjacent_liked="Ai,Hh,Mm,Uu,Uh,Uu^Tf,Xu,Rb",
			rating=4
		},
		dr_village {
			terrain="Uh",
			convert_to="Uh^Vud",
			adjacent_liked="Ai,Hh,Mm,Uu,Uh,Uu^Tf,Xu,Rb",
			rating=3
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vhh",
			adjacent_liked="Gll, Gll^Fet, Ww, Ww, Ww, Ww, Ww, Wwt, Wwf, Wwf, Rb, Rb, Rb, Ce, Gd, Gll, Hh, Hh^Fp, Gll^Fp, Gll^Fp, Gll^Fet, Gll^Fet",
			rating=4
		},
		dr_village {
			terrain="Hhd",
			convert_to="Hh^Vhh",
			adjacent_liked="Gll, Gll^Fet, Ww, Ww, Ww, Ww, Ww, Ww, Wwf, Wwf, Rb, Rb, Rb, Ce, Gd, Gll, Hhd, Hh^Fp, Gll^Fp, Gll^Fp, Gll^Fet, Gll^Fet",
			rating=4
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vhh",
			adjacent_liked="Gll, Gll^Fet, Mm, Mm^Xm, Ms, Ha, Ha^Fpa, Ww, Wwf, Wwf, Rb, Rb, Ce, Gd, Gd, Gll, Hh, Hh^Fp, Gll^Fp, Gll^Fp, Gll^Fet, Gll^Fet",
			rating=3
		},
		-- villages in snow
		dr_village {
			terrain="Aa",
			convert_to="Aa^Vha",
			adjacent_liked="Gll, Gll^Fet, Aa, Ai, Ha, Mm, Ms, Ww, Wwf, Wwf, Rb, Rb, Ce, Gd, Gd, Gll, Hh, Hh^Fp, Gll^Fp",
			rating=3
		},
		dr_village {
			terrain="Aa^Fpa",
			convert_to="Aa^Vea",
			adjacent_liked="Gll, Gll^Fet, Aa, Ai, Ha, Mm, Ms, Ww, Wwf, Wwf, Rb, Rb, Ce, Gd, Gd, Gll, Hh, Hh^Fp, Gll^Fp",
			rating=3
		},
		dr_village {
			terrain="Ha",
			convert_to="Ha^Vhha",
			adjacent_liked="Gll, Gll^Fet, Aa, Ai, Ha, Mm, Ms, Ww, Wwf, Wwf, Rb, Rb, Ce, Gd, Gd, Gll, Hh, Hh^Fp, Gll^Fp",
			rating=2
		},
		dr_village {
			terrain="Ms",
			convert_to="Ms^Vhha",
			adjacent_liked="Gll, Gll^Fet, Aa, Ai, Ha, Mm, Ms, Ww, Wwf, Wwf, Rb, Rb, Ce, Gd, Gd, Gll, Hh, Hh^Fp, Gll^Fp",
			rating=1
		},
		dr_village {
			terrain="Ss",
			convert_to="Ss^Vhs",
			adjacent_liked="Gll, Ss, Ce, Ww, Ww, Ww, Ww, Wwt, Wwf, Wwf, Rb, Rb, Rb, Rb, Gll, Gll, Hh, Gll^Fp",
			rating=3
		},
		dr_village {
			terrain="Ww",
			convert_to="Ww^Vm",
			adjacent_liked="Ww, Ww",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gll, Gd, Hh, Gll^Ft, Gll^Efm, Gll^Fet, Gll^Ftr, Ss, Ai, Gll^Tf, Hh^Tf, Hh^Fp, Aa, Ha, Ha^Fpa, Mm, Ms",
		min_distance=13,
	}

	return default_generate_map(res)
end
return generate
