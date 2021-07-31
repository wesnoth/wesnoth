
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=0
	res.min_lake_height=999
	res.lake_size=0
	res.river_frequency=0
	res.temperature_size=size
	res.roads=30
	res.road_windiness=3

	res.height = {
		dr_height(990, "Uh^Tf"),
		dr_height(970, "Qxu"),
		dr_height(950, "Uh"),
		dr_height(940, "Uu^Tf"),
		dr_height(920, "Uu"),
		dr_height(910, "Qxu"),
		dr_height(890, "Uh"),
		dr_height(880, "Uu^Tf"),
		dr_height(860, "Uu"),
		dr_height(840, "Uh"),
		dr_height(830, "Uu^Tf"),
		dr_height(810, "Uu"),
		dr_height(765, "Xu"),
		dr_height(740, "Mm^Xm"),
		dr_height(700, "Mm"),
		dr_height(690, "Hh^Tf"),
		dr_height(655, "Hh"),
		dr_height(650, "Hh^Tf"),
		dr_height(600, "Hh"),
		dr_height(110, "Gg"),
		dr_height(30, "Ds"),
		dr_height(1, "Ww"),
		dr_height(0, "Wo"),
	}
	res.convert = {
		-- DR_TEMPERATURE FROM MIN MAX TO),
		-- cold
		dr_temperature("Mm^Xm", 1, 405, "Ms^Xm"),
		dr_temperature("Mm", 1, 385, "Ms"),
		dr_temperature("Hh,Hh^Tf", 1, 345, "Ha"),
		dr_temperature("Gg", 1, 305, "Aa"),
		dr_temperature("Ds", 1, 260, "Ai"),
		dr_temperature("Gg", 305, 500, "Gs"),
		-- hot
		dr_temperature("Hh,Hh^Tf", 585, 760, "Hhd"),
		dr_temperature("Hh,Hh^Tf", 760, 999, "Hd"),
		dr_temperature("Gg", 720, 999, "Dd"),
		dr_temperature("Qxu", 850, 999, "Ql"),
	}
	res.road_cost = {
		dr_road("Gg", "Re", 10),
		dr_road("Gs", "Re", 10),
		dr_road("Hh", "Re", 30),
		dr_road("Hhd", "Re", 30),
		dr_road("Mm", "Re", 40),
		dr_road("Mm^Xm", "Re", 75),
		dr_road("Hd", "Re", 30),
		dr_road("Dd", "Re", 20),
		dr_bridge("Ql", "Ql^Bs", "Re", 100),
		dr_bridge("Qxu", "Qxu^Bs", "Re", 100),
		dr_road("Uu", "Re", 10),
		dr_road("Uh", "Re", 40),
		dr_road("Xu", "Re", 90),
		dr_road("Aa", "Re", 20),
		dr_road("Ha", "Re", 40),
		dr_road("Re", "Re", 2),
		dr_road("Ch", "Ch", 2),
	}
	res.village = {
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Gg, Gg, Gg, Ww, Ww, Gs, Gs, Gs, Re, Re, Re, Re, Hh",
			rating=8
		},
		dr_village {
			terrain = "Gs",
			convert_to="Gs^Vh",
			adjacent_liked="Gg, Gg, Gg, Gs, Ww, Ww, Gs, Gs, Gs, Re, Re, Re, Re, Hh",
			rating=8
		},
		-- villages in snow and dessert, give them grass rating
		dr_village {
			terrain="Aa",
			convert_to="Aa^Vha",
			adjacent_liked="Gg, Gg, Aa, Aa, Gs, Aa, Aa, Gs, Gs, Gs, Re, Re, Re, Ha, Ha, Ms",
			rating=8
		},
		dr_village {
			terrain = "Dd",
			convert_to="Dd^Vd",
			adjacent_liked="Gg, Gg, Dd, Dd, Gg, Ww, Dd, Dd, Re, Re, Re, Hd, Hd, Gs",
			rating=8
		},
		dr_village {
			terrain="Ds",
			convert_to="Ds^Vda",
			adjacent_liked="Gg, Gg, Gg, Gs, Gs, Gs, Ww, Ww, Ww, Ds, Ds, Ds, Re, Hh",
			rating=3
		},
		-- cave villages
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vud",
			adjacent_liked="Re,Hh,Mm,Uu,Uh,Xu,Qxu,Uu,Uu,Uh",
			rating=5
		},
		dr_village {
			terrain="Uh",
			convert_to="Uu^Vud",
			adjacent_liked="Re,Hh,Mm,Uu,Uh,Xu,Qxu,Uu,Uu,Uh",
			rating=5
		},
		dr_village {
			terrain="Uu^Tf",
			convert_to="Uu^Vu",
			adjacent_liked="Hh^Tf,Hh,Mm,Uu,Uh,Xu,Qxu,Uu,Uu^Tf,Uu^Tf,Uh^Tf",
			rating=5
		},
		dr_village {
			terrain="Uh^Tf",
			convert_to="Uh^Vu",
			adjacent_liked="Hh^Tf,Hh,Mm,Uu,Uh,Xu,Qxu,Uu,Uu^Tf,Uh^Tf,Uh^Tf",
			rating=5
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gg, Gg, Hh, Ww, Hh, Hh, Gs, Gs, Gs, Re, Re, Mm, Hh",
			rating=6
		},
		dr_village {
			terrain="Ha",
			convert_to="Ha^Vhha",
			adjacent_liked="Gg, Aa, Aa, Hh, Aa, Hh, Hh, Gs, Gs, Gs, Ha, Ha, Mm, Ha, Ms",
			rating=6
		},
		dr_village {
			terrain="Hhd",
			convert_to="Hhd^Vd",
			adjacent_liked="Gg, Gg, Gg, Hh, Ww, Hh, Hh, Gs, Gs, Gs, Re, Re, Mm, Hh",
			rating=6
		},
		dr_village {
			terrain="Hd",
			convert_to="Hd^Vct",
			adjacent_liked="Gg, Gg, Gg, Hh, Ww, Hhd, Hh, Dd, Dd, Gs, Re, Re, Mm, Hd",
			rating=6
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vhh",
			adjacent_liked="Gg, Gg, Gg, Hh, Gs, Gs, Gs, Hh, Hh, Mm, Mm, Re, Mm, Hh",
			rating=5
		},
		-- mermen villages - give them low chance of appearing
		dr_village {
			terrain="Ww",
			convert_to="Ww^Vm",
			adjacent_liked="Ww, Ww, Ds",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gg,Gs",
		min_distance=14,
	}

	return default_generate_map(res)
end
return generate
