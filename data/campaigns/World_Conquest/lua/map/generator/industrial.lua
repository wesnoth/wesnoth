
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=115
	res.min_lake_height=320
	res.lake_size=60
	res.river_frequency=95
	res.temperature_size=7
	res.roads=11
	res.road_windiness=8

	res.height = {
		-- list of common terrain types which come in at different heights, from highest to lowest
		dr_height(950, "Uh"),
		dr_height(910, "Uu"),
		dr_height(870, "Uh"),
		dr_height(820, "Uu"),
		dr_height(780, "Xu"),
		dr_height(765, "Mm^Xm"),
		dr_height(725, "Mm"),
		dr_height(715, "Hh"),
		dr_height(710, "Hh^Fp"),
		dr_height(685, "Hh"),
		dr_height(675, "Hh^Tf"),
		dr_height(650, "Hh"),
		dr_height(645, "Hh^Fp"),
		dr_height(610, "Hh"),
		dr_height(600, "Gg"),
		dr_height(590, "Hh^Fp"),
		dr_height(580, "Gg"),
		dr_height(570, "Gs^Fp"),
		dr_height(425, "Gg"),
		dr_height(420, "Hh^Fp"),
		dr_height(410, "Gg"),
		dr_height(400, "Mm"),
		dr_height(395, "Gs^Tf"),
		dr_height(380, "Ss"),
		dr_height(375, "Gs^Tf"),
		dr_height(360, "Gg"),
		dr_height(340, "Hh^Fp"),
		dr_height(320, "Gg"),
		dr_height(300, "Gs^Fp"),
		dr_height(260, "Gg"),
		dr_height(240, "Ss"),
		dr_height(220, "Gs^Fp"),
		dr_height(200, "Hh^Fp"),
		dr_height(125, "Gg"),
		dr_height(50, "Ds"),
		dr_height(1, "Wwt"),
		dr_height(0, "Wot"),
	}
	res.convert = {
		wct_fix_river_into_ocean("t", 46),

		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		-- low temperatures
		dr_convert(80, 999, 0, 375, "Gg, Gs^Tf", "Aa"),
		dr_convert(250, 999, 0, 375, "Ss", "Ai"),
		dr_convert(80, 999, 370, 425, "Gg", "Gs"),
		dr_convert(80, 999, 0, 375, "Gs^Fp", "Aa^Fpa"),
		dr_convert(80, 999, 375, 425, "Gs^Fp", "Gs^Fmw"),
		dr_convert(0, 999, 0, 400, "Hh^Fp, Hh^Tf", "Ha^Fpa"),
		dr_convert(80, 999, 400, 450, "Hh^Fp", "Hh^Fmw"),
		dr_convert(0, 999, 0, 425, "Hh", "Ha"),
		dr_convert(0, 999, 0, 450, "Mm", "Ms"),
		dr_convert(750, 999, 0, 460, "Mm^Xm", "Ms^Xm"),
		-- fungus
		dr_convert(850, 950, 500, 525, "Uu, Uh", "Uu^Tf"),
		dr_convert(850, 950, 550, 575, "Uu, Uh", "Uu^Tf"),
		dr_convert(850, 950, 600, 625, "Uu, Uh", "Uu^Tf"),
		-- high temperatures
		dr_convert(825, 999, 850, 999, "Uu, Uh, Uu^Tf", "Ql"),
		dr_convert(0, 999, 800, 999, "Gg", "Dd"),
		dr_convert(250, 999, 800, 999, "Ss", "Dd^Do"),
		dr_convert(80, 999, 750, 800, "Gg", "Gs"),
		dr_convert(80, 999, 730, 760, "Gs^Fp", "Gs^Fet"),
		-- moderate temperatures
		dr_convert(0, 999, 450, 575, "Hh^Fp", "Hh^Fmf"),
		dr_convert(0, 999, 430, 575, "Gs^Fp", "Gg^Fmf"),
		dr_convert(0, 999, 575, 725, "Hh^Fp", "Hh^Fms"),
		dr_convert(0, 999, 575, 725, "Gs^Fp", "Gg^Fms"),
	}
	res.road_cost = {
		dr_road("Gg", "Urb", 10),
		dr_road("Gs^Fp", "Urb", 20),
		dr_road("Gs^Fmw", "Urb", 20),
		dr_road("Gs^Fet", "Urb", 20),
		dr_road("Gg^Fmf", "Urb", 20),
		dr_road("Gg^Fms", "Urb", 20),
		dr_road("Hh", "Urb", 25),
		dr_road("Hh^Fp", "Urb", 30),
		dr_road("Hh^Fmf", "Urb", 30),
		dr_road("Hh^Fmw", "Urb", 30),
		dr_road("Hh^Fms", "Urb", 30),
		dr_road("Hh^Tf", "Urb", 25),
		dr_road("Mm", "Urb", 40),
		dr_road("Mm^Xm", "Urb", 75),
		dr_bridge("Ql", "Ql^Bs", "Urb", 100),
		dr_bridge("Qxu", "Qxu^Bs", "Urb", 100),
		dr_road("Uu", "Urb", 10),
		dr_road("Uh", "Urb", 35),
		dr_road("Xu", "Urb", 80),
		dr_road("Aa", "Urb", 15),
		dr_road("Ha", "Urb", 20),
		dr_road("Ha^Fpa", "Urb", 25),
		dr_road("Aa^Fpa", "Urb", 20),
		dr_bridge("Ww", "Ww^Bcx", "Ce", 50),
		dr_road("Urb", "Urb", 2),
		dr_road_over_bridges("Ww^Bcx", 2),
		dr_road("Ch", "Ch", 2),
		dr_road("Ce", "Ce", 2),
		dr_road("Dd", "Urb", 25),
		dr_road("Ds", "Urb", 35),
	}
	res.village = {
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Hh, Gs^Fp, Gg^Fms",
			rating=8
		},
		dr_village {
			terrain = "Gs",
			convert_to="Gs^Vh",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Hh, Ha, Gs^Fp, Gg^Fms",
			rating=5
		},
		dr_village {
			terrain="Ds",
			convert_to="Ds^Vda",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Hh, Gs^Fp",
			rating=2
		},
		dr_village {
			terrain="Dd",
			convert_to="Dd^Vda",
			adjacent_liked="Gg, Gs, Dd, Dd, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Ww, Ww, Urb, Urb, Hh, Gs^Fp, Mm",
			rating=4
		},
		-- villages in cave are orcish
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vo",
			adjacent_liked="Urb,Hh,Mm,Uu,Uh,Xu,Mm^Xm,Uu^Tf,Uu,Uh,Ww,Ww",
			rating=5
		},
		dr_village {
			terrain="Uu^Tf",
			convert_to="Uh^Vo",
			adjacent_liked="Urb,Hh,Mm,Uu,Uh,Xu,Mm^Xm,Uu^Tf,Uu,Uh,Ww,Ww",
			rating=5
		},
		dr_village {
			terrain="Uh",
			convert_to="Uu^Vo",
			adjacent_liked="Urb,Hh,Mm,Uu,Uh,Xu,Mm^Xm,Uu^Tf,Uu,Uh,Ww,Ww",
			rating=5
		},
		-- villages in forest
		dr_village {
			terrain="Gs^Fp",
			convert_to="Gs^Vh",
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Hh, Gs^Fp, Gs^Fp, Gg^Fms",
			rating=4
		},
		dr_village {
			terrain="Gg^Fms",
			convert_to="Gg^Vh",
			adjacent_liked="Gg, Gs, Gg, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Hh, Gs^Fp, Gg^Fms, Gg^Fms",
			rating=4
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vh",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=4
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vd",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=3
		},
		-- villages in snow
		dr_village {
			terrain="Aa",
			convert_to="Aa^Vha",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=3
		},
		dr_village {
			terrain="Aa^Fpa",
			convert_to="Aa^Vha",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=3
		},
		dr_village {
			terrain="Ha",
			convert_to="Ha^Vha",
			adjacent_liked="Gg, Gs, Gs, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Hh, Ha, Mm, Gs^Fp",
			rating=3
		},
		-- swamp villages
		dr_village {
			terrain="Ss",
			convert_to="Ss^Vhs",
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bcx|, Ww^Bcx/, Ww^Bcx\\, Urb, Urb, Urb, Urb, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
			rating=2
		},
		-- mermen villages
		dr_village {
			terrain="Wwt",
			convert_to="Wwt^Vm",
			adjacent_liked="Wwt, Wwt",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gs, Gg, Hh, Ha, Hh^Tf, Gs^Tf, Ss, Dd, Aa",
		min_distance=16,
	}

	return default_generate_map(res)
end
return generate
