
local function generate(length, villages, castle, iterations, size, players, island)

	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=1
	res.min_lake_height=150
	res.lake_size=5
	res.river_frequency=10
	res.temperature_size=8
	res.roads=15
	res.road_windiness=4

	res.height = {
		dr_height(925, "Uh"),
		dr_height(820, "Uu"),
		dr_height(765, "Xu"),
		dr_height(750, "Mm^Xm"),
		dr_height(700, "Mm"),
		dr_height(680, "Hh^Tf"),
		dr_height(650, "Gs^Fp"),
		dr_height(625, "Hh^Fp"),
		dr_height(575, "Hh"),
		dr_height(400, "Gs"),
		dr_height(375, "Gs^Fp"),
		dr_height(325, "Gs"),
		dr_height(300, "Hh^Fp"),
		dr_height(275, "Hh"),
		dr_height(175, "Gs"),
		dr_height(30, "Ds"),
		dr_height(1, "Wwt"),
		dr_height(0, "Wot"),
	}
	res.convert = {
		-- DR_CONVERT MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
		dr_convert(450, 525, 925, 999, "Gs", "Md^Xm"),
		dr_convert(450, 525, 875, 925, "Gs", "Md"),
		dr_convert(450, 525, 775, 875, "Gs", "Qlf"),
		dr_convert(450, 525, 770, 780, "Gs", "Hhd"),
		dr_convert(450, 525, 525, 770, "Gs", "Dd"),
		dr_convert(450, 525, 425, 525, "Gs", "Sm"),
		dr_convert(450, 490, 375, 425, "Gs", "Wwg"),
		dr_convert(490, 525, 375, 425, "Gs", "Wwrg"),
		dr_convert(450, 525, 225, 375, "Gs", "Gs^Tf"),
		dr_convert(450, 525, 10, 225, "Gs", "Gg^Fet"),
		dr_convert(100, 999, 560, 999, "Gs", "Gd"),
		dr_convert(100, 999, 10, 385, "Gs", "Gg"),
		dr_convert(75, 999, 600, 999, "Ds", "Dd"),
		dr_convert(200, 999, 600, 999, "Hh", "Hhd"),
		dr_convert(200, 999, 650, 999, "Hh^Tf", "Hhd^Tf"),
		dr_convert(200, 999, 600, 999, "Mm", "Md"),

		-- DR_TEMPERATURE FROM MIN MAX TO),
		-- convert forest at different temperatures
		dr_temperature("Gs^Fp", 900, 999, "Gd^Fdw"),
		dr_temperature("Hh^Fp", 900, 999, "Hhd^Fdw"),
		dr_temperature("Gs^Fp", 775, 900, "Gd^Fetd"),
		dr_temperature("Hh^Fp", 800, 900, "Hhd^Fdf"),
		dr_temperature("Gs^Fp", 600, 775, "Gd^Fdf"),
		dr_temperature("Hh^Fp", 700, 800, "Hhd^Fmf"),
		dr_temperature("Gs^Fp", 560, 600, "Gs^Fet"),
		dr_temperature("Gs^Fp", 500, 560, "Gs^Ftp"),
		dr_temperature("Gs^Fp", 450, 500, "Gs^Ftr"),
		dr_temperature("Hh^Fp", 450, 700, "Hh^Ftp"),
		dr_temperature("Gs^Fp", 1, 450, "Gg^Ft"),
		dr_temperature("Hh^Fp", 1, 450, "Hh^Ft"),
		-- convert cave at different temperatures
		dr_temperature("Uu,Uh", 850, 999, "Ql"),
		dr_temperature("Uu", 725, 850, "Uue"),
		dr_temperature("Uh", 725, 850, "Uue^Dr"),
		dr_temperature("Uu", 700, 725, "Uue^Tf"),
		dr_temperature("Uu", 575, 600, "Uu^Tf"),
	}
	res.road_cost = {
		dr_road("Gg", "Re", 10),
		dr_road("Gg^Ft", "Re", 20),
		dr_road("Hh", "Re", 30),
		dr_road("Hhd", "Re", 30),
		dr_road("Hh^Ftp", "Re", 30),
		dr_road("Hhd^Fmf", "Re", 30),
		dr_road("Mm", "Re", 40),
		dr_road("Md", "Re", 40),
		dr_road("Mm^Xm", "Re", 75),
		dr_bridge("Qlf", "Qlf^Bsb", "Cud", 55),
		dr_bridge("Sm", "Sm^Bw", "Co", 45),
		dr_road("Uu", "Re", 10),
		dr_road("Uue", "Re", 10),
		dr_road("Uh", "Re", 40),
		dr_road("Xu", "Re", 90),
		dr_road("Re", "Re", 2),
		dr_road("Ch", "Ch", 2),
		dr_road("Dd", "Re", 15),
		dr_road("Gs", "Re", 9),
		dr_road("Gd", "Re", 10),
		dr_road("Gs^Tf", "Re", 12),
		dr_road("Gs^Ftr", "Re", 20),
		dr_road("Gs^Ftp", "Re", 20),
		dr_road("Gs^Fet", "Re", 20),
		dr_road("Gd^Fdf", "Re", 20),
		dr_road("Gd^Fetd", "Re", 20),
	}
	res.village = {
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vht",
			adjacent_liked="Gg, Gs, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Hh, Mm, Hh^Ft, Hh^Ftp",
			rating=5
		},
		dr_village {
			terrain = "Gs",
			convert_to="Gs^Vht",
			adjacent_liked="Gg, Gs, Gd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd",
			rating=4
		},
		dr_village {
			terrain = "Gs^Tf",
			convert_to="Gs^Vh",
			adjacent_liked="Gg, Gs, Gs^Tf, Gs^Tf, Gs^Tf, Gd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd",
			rating=8
		},
		dr_village {
			terrain = "Gd",
			convert_to="Gd^Vo",
			adjacent_liked="Gg, Gs, Gd, Dd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Md, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd, Qlf, Cud, Qlf, Qlf^Bsb|, Qlf^Bsb/, Qlf^Bsb\\",
			rating=4
		},
		dr_village {
			terrain="Ds",
			convert_to="Ds^Vda",
			adjacent_liked="Gg, Gs, Wwt, Wwt, Wwt, Re, Re, Hh, Gg^Ft, Gs^Ftp",
			rating=2
		},
		dr_village {
			terrain="Dd",
			convert_to="Dd^Vd",
			adjacent_liked="Gg, Gs, Wwt, Wwt, Wwt, Re, Re, Hh, Gg^Ft, Gs^Ftp",
			rating=4
		},
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vud",
			adjacent_liked=" Re,Hh,Hhd,Mm,Uu,Uh,Xu,Uue,Uue^Dr,Md,Mm^Xm,Md^Xm",
			rating=5
		},
		dr_village {
			terrain="Uh",
			convert_to="Uu^Vud",
			adjacent_liked=" Re,Hh,Hhd,Mm,Uu,Uh,Xu,Uue,Uue^Dr,Md,Mm^Xm,Md^Xm",
			rating=5
		},
		dr_village {
			terrain="Uue",
			convert_to="Uue^Vo",
			adjacent_liked=" Re,Hh,Hhd,Mm,Uu,Uh,Xu,Uue,Uue^Dr,Md,Mm^Xm,Md^Xm",
			rating=5
		},
		dr_village {
			terrain="Uue^Dr",
			convert_to="Uh^Vo",
			adjacent_liked=" Re,Hh,Hhd,Mm,Uu,Uh,Xu,Uue,Uue^Dr,Md,Mm^Xm,Md^Xm",
			rating=5
		},
		-- villages in forest are Orcish
		dr_village {
			terrain="Gs^Ftp",
			convert_to="Gs^Vo",
			adjacent_liked="Gg, Gs, Gd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd",
			rating=2
		},
		dr_village {
			terrain="Gg^Ft",
			convert_to="Gg^Vo",
			adjacent_liked="Gg, Gs, Gd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd",
			rating=2
		},
		dr_village {
			terrain="Gs^Ftr",
			convert_to="Gs^Vo",
			adjacent_liked="Gg, Gs, Gd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd",
			rating=2
		},
		dr_village {
			terrain="Gd^Fdf",
			convert_to="Gd^Vo",
			adjacent_liked="Gg, Gs, Gd, Dd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Md, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd, Qlf, Cud, Qlf, Qlf^Bsb|, Qlf^Bsb/, Qlf^Bsb\\",
			rating=2
		},
		dr_village {
			terrain="Gd^Fetd",
			convert_to="Gd^Vo",
			adjacent_liked="Gg, Gs, Gd, Dd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Md, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd, Qlf, Cud, Qlf, Qlf^Bsb|, Qlf^Bsb/, Qlf^Bsb\\",
			rating=2
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vhh",
			adjacent_liked="Gg, Gs, Gd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd",
			rating=4
		},
		dr_village {
			terrain="Hh^Ftp",
			convert_to="Hh^Vo",
			adjacent_liked="Gg, Gs, Gd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd",
			rating=2
		},
		dr_village {
			terrain="Hh^Ft",
			convert_to="Hh^Vo",
			adjacent_liked="Gg, Gs, Gd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd",
			rating=2
		},
		dr_village {
			terrain="Hhd^Fmf",
			convert_to="Hhd^Vo",
			adjacent_liked="Gg, Gs, Gd, Dd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Md, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd, Qlf, Cud, Qlf, Qlf^Bsb|, Qlf^Bsb/, Qlf^Bsb\\",
			rating=2
		},
		dr_village {
			terrain="Hhd",
			convert_to="Hhd^Vd",
			adjacent_liked="Gg, Gs, Gd, Dd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Md, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd, Qlf, Cud, Qlf, Qlf^Bsb|, Qlf^Bsb/, Qlf^Bsb\\",
			rating=5
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vhh",
			adjacent_liked="Gg, Gs, Gd, Dd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Md, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd, Qlf, Cud, Qlf, Qlf^Bsb|, Qlf^Bsb/, Qlf^Bsb\\",
			rating=3
		},
		dr_village {
			terrain="Md",
			convert_to="Md^Vd",
			adjacent_liked="Gg, Gs, Gd, Dd, Re, Re, Gs^Ftr, Gs^Ftp, Gs^Fet, Gg^Ft, Gd^Fdf, Gd^Fetd, Hh, Mm, Md, Hh^Ft, Hh^Ftp, Hhd^Fmf, Hhd, Qlf, Cud, Qlf, Qlf^Bsb|, Qlf^Bsb/, Qlf^Bsb\\",
			rating=4
		},
		-- mermen villages - give them low chance of appearing
		dr_village {
			terrain="Wwt",
			convert_to="Wwt^Vm",
			adjacent_liked="Wwt, Wwt, Wot, Ds",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gs, Gg, Gd, Gs^Fp, Gg^Ft, Gs^Ftr, Gs^Fet, Gd^Fetd, Gd^Fdf, Hh, Hhd, Dd, Mm, Md, Sm, Hh^Fp, Hh^Ftp, Hh^Fmf, Hh^Ft",
		min_distance=13,
	}

	return default_generate_map(res)
end
return generate
