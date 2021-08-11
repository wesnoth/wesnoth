
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=35
	res.min_lake_height=250
	res.lake_size=100
	res.river_frequency=100
	res.temperature_size=size
	res.roads=0
	res.road_windiness=1
	res.height = {
		-- list of common terrain types which come in at different heights, from highest to lowest
		dr_height(955, "Uh"),
		dr_height(945, "Uu^Tf"),
		dr_height(900, "Uu"),
		dr_height(855, "Uh"),
		dr_height(845, "Uu^Tf"),
		dr_height(825, "Uu"),
		dr_height(775, "Xu"),
		dr_height(750, "Mm^Xm"),
		dr_height(700, "Mm"),
		dr_height(690, "Hh^Tf"),
		dr_height(660, "Hh^Fp"),
		dr_height(625, "Hh"),
		-- most rough terrain is added post generation
		dr_height(115, "Gg"),
		dr_height(30, "Ds"),
		dr_height(1, "Wwg"),
		dr_height(0, "Wog"),
	}
	res.convert = {
		wct_fix_river_into_ocean("g", 29),
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		-- lava appears at extreme temp and height
		dr_convert(800, 999, 850, 999, "Uu, Uh, Uu^Tf", "Ql"),
		-- DR_TEMPERATURE FROM MIN MAX TO),
		-- less wet flat as higher temperature
		dr_temperature("Gg", 720, 999, "Dd"),
		dr_temperature("Gg", 655, 720, "Rd"),
		dr_temperature("Gg", 570, 655, "Re"),
		dr_temperature("Gg", 435, 570, "Rb"),
		dr_temperature("Gg", 340, 435, "Gs"),
	}
	res.village = {
		-- flat villages
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vht",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs",
			rating=8
		},
		dr_village {
			terrain = "Gs",
			convert_to="Gs^Vc",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs",
			rating=7
		},
		dr_village {
			terrain = "Rb",
			convert_to="Rb^Vda",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs, Dd",
			rating=6
		},
		dr_village {
			terrain = "Re",
			convert_to="Re^Vda",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs, Dd",
			rating=6
		},
		dr_village {
			terrain = "Rd",
			convert_to="Rd^Vda",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs, Dd, Dd",
			rating=5
		},
		dr_village {
			terrain = "Dd",
			convert_to="Dd^Vda",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs, Dd, Dd",
			rating=4
		},
		dr_village {
			terrain = "Ds",
			convert_to="Ds^Vda",
			adjacent_liked="Gg, Gs, Wwg, Wwg, Ds, Ds, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs",
			rating=1
		},
		-- rough villages
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs, Hh, Hh^Fp, Hh^Tf, Mm",
			rating=5
		},
		dr_village {
			terrain="Hh^Fp",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs, Hh, Hh^Fp, Hh^Tf, Mm",
			rating=4
		},
		dr_village {
			terrain="Hh^Tf",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs, Hh, Hh^Fp, Hh^Tf, Mm",
			rating=3
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vhh",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Re, Rd, Rb, Gg, Gs, Re, Rd, Rb, Gg, Gs, Hh, Hh^Fp, Hh^Tf, Mm",
			rating=3
		},
		-- cave villages
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vu",
			adjacent_liked="Hh,Hh^Fp,Mm,Uu,Uh,Hh^Tf,Xu,Uu^Tf,Mm^Xm,Ww",
			rating=4
		},
		dr_village {
			terrain="Uh",
			convert_to="Uu^Vu",
			adjacent_liked="Hh,Hh^Fp,Mm,Uu,Uh,Hh^Tf,Xu,Uu^Tf,Mm^Xm,Ww",
			rating=4
		},
		-- water villages
		dr_village {
			terrain="Wwg",
			convert_to="Wwg^Vm",
			adjacent_liked="Wwg, Wwg, Ds",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gs, Gg, Re, Rb, Rd",
		min_distance=14,
	}

	return default_generate_map(res)
end
return generate
