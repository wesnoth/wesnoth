
local function generate(length, villages, castle, iterations, size, players, island)
	local width = length
	if width % 2 == 1 then
		width = width + 1
	end

	local res = {}
	res.border_size=0
	res.map_width=width
	res.map_height=length
	res.iterations=iterations
	res.hill_size=size
	res.villages=villages
	res.nplayers=players
	res.island_size=island
	res.castle_size=castle
	res.temperature_iterations=0
	--!? for some reason still can generate rivers with value 0
	res.max_lakes=1
	res.river_frequency=1
	res.roads=18
	res.road_windiness=4
	res.link_castles=true

	res.height = {
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
		dr_height(540, "Gg"),
		dr_height(380, "Gs"),
		dr_height(310, "Wog"),-- just to avoid castle generation
		dr_height(280, "Ds"),
		dr_height(270, "Wwt"),
		dr_height(260, "Ds"),
		dr_height(250, "Wwrt"),
		dr_height(240, "Ds"),
		dr_height(230, "Wwt"),
		dr_height(220, "Ds"),
		dr_height(210, "Wwrt"),
		dr_height(200, "Ds"),
		dr_height(190, "Wwt"),
		dr_height(180, "Ds"),
		dr_height(170, "Wwt"),
		dr_height(160, "Ds"),
		dr_height(150, "Wwrt"),
		dr_height(140, "Ds"),
		dr_height(130, "Wwrt"),
		dr_height(120, "Ds"),
		dr_height(110, "Wwrt"),
		dr_height(100, "Ds"),
		dr_height(90, "Wwrt"),
		dr_height(80, "Dd"),-- just to avoid village generation
		dr_height(70, "Wwrt"),
		dr_height(60, "Dd"),
		dr_height(50, "Wwrt"),
		dr_height(40, "Dd"),
		dr_height(20, "Wwrt"),
		dr_height(1, "Wwt"),
		dr_height(0, "Wot"),
	}

	res.road_cost = {
		dr_road("Gs", "Re", 10),
		dr_road("Gg", "Re", 9),
		dr_road("Re", "Re", 4),
	}
	res.village = {
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vd",
			adjacent_liked="Gg, Gs, Wwt, Wwrt, Re, Gg, Gs, Re, Gg, Gs, Hh, Mm, Hh^Fp",
			rating=8
		},
		dr_village {
			terrain = "Gs",
			convert_to="Gs^Vc",
			adjacent_liked="Gg, Gs, Wwt, Wwrt, Re, Gg, Gs, Re, Gg, Gs, Hh",
			rating=8
		},
		dr_village {
			terrain = "Wog",
			convert_to="Gs^Vc",
			adjacent_liked="Gg, Gs, Wwt, Wwrt, Re, Gg, Gs, Re, Gg, Gs, Hh",
			rating=6
		},
		dr_village {
			terrain = "Ds",
			convert_to="Ds^Vc",
			adjacent_liked="Gg, Gs, Wwt, Wwrt, Ds, Ds, Gs, Gs, Gs, Gs",
			rating=1
		},
		-- rough villages
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vd",
			adjacent_liked="Gg, Gs, Wwt, Wwrt, Re, Gg, Gs, Re, Gg, Gs, Hh, Hh^Fp, Hh^Tf, Mm",
			rating=5
		},
		dr_village {
			terrain="Hh^Fp",
			convert_to="Hh^Vd",
			adjacent_liked="Gg, Gs, Wwt, Wwrt, Re, Gg, Gs, Re, Gg, Gs, Hh, Hh^Fp, Hh^Tf, Mm",
			rating=4
		},
		dr_village {
			terrain="Hh^Tf",
			convert_to="Hh^Vd",
			adjacent_liked="Gg, Gs, Wwt, Wwrt, Re, Gg, Gs, Re, Rd, Rb, Gg, Gs, Hh, Hh^Fp, Hh^Tf, Mm",
			rating=3
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vd",
			adjacent_liked="Gg, Gs, Wwt, Wwrt, Re, Gg, Gs, Re, Rd, Rb, Gg, Gs, Hh, Hh^Fp, Hh^Tf, Mm",
			rating=3
		},
		-- cave villages
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vu",
			adjacent_liked="Hh,Hh^Fp,Mm,Uu,Uh,Hh^Tf,Xu,Uu^Tf,Mm^Xm",
			rating=4
		},
		dr_village {
			terrain="Uh",
			convert_to="Uu^Vu",
			adjacent_liked="Hh,Hh^Fp,Mm,Uu,Uh,Hh^Tf,Xu,Uu^Tf,Mm^Xm",
			rating=4
		},
		-- water villages
		dr_village {
			terrain="Wwt",
			convert_to="Wwt^Vm",
			adjacent_liked="Wwt, Wwt, Wwrt",
			rating=1
		},
	}
	res.castle = {
		valid_terrain="Gs,Gg",
		min_distance=14,
	}

	return default_generate_map(res)
end
return generate
