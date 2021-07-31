
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
	res.max_lakes=80
	res.min_lake_height=300
	res.river_frequency=80
	res.roads=18
	res.road_windiness=2
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
		dr_height(660, "Hh^Fds"),
		dr_height(625, "Hh"),
		dr_height(615, "Hh^Tf"),
		dr_height(600, "Hh^Fds"),
		dr_height(500, "Gd"),
		dr_height(310, "Gs"),
		dr_height(210, "Gg"),
		dr_height(70, "Ss"),
		dr_height(1, "Wwt"),
		dr_height(0, "Wot"),
	}
	res.convert = {
		wct_fix_river_into_ocean("t", 65),
	}
	res.road_cost = {
		dr_road("Gs", "Rb", 10),
		dr_road("Gg", "Rb", 25),
		dr_road("Gd", "Rb", 25),
		dr_bridge("Ww", "Ww^Bw", "Wwf", 45),
		dr_road("Hh", "Rb", 55),
		dr_road("Hh^Fds", "Rb", 55),
		dr_road("Hh^Tf", "Rb", 55),
		dr_road("Mm", "Rb", 70),
		dr_road("Rb", "Rb", 4),
	}
	res.village = {
		dr_village {
			terrain = "Gg",
			convert_to="Gg^Vo",
			adjacent_liked="Gg, Gs, Ww, Rb, Gd, Gs, Rb, Gg, Gs, Hh, Mm, Hh^Fds, Ss",
			rating=8
		},
		dr_village {
			terrain = "Gs",
			convert_to="Gs^Ve",
			adjacent_liked="Gg, Gs, Ww, Rb, Rb, Rb, Gd, Rb, Rb, Gs, Hh, Hh^Fds, Hh^Tf, Gg, Gs",
			rating=9
		},
		dr_village {
			terrain = "Gd",
			convert_to="Gg^Vo",
			adjacent_liked="Gg, Gs, Ww, Rb, Gd, Gs, Rb, Gg, Gs, Hh, Mm, Hh^Fds, Hh^Tf",
			rating=8
		},
		dr_village {
			terrain = "Ss",
			convert_to="Ss^Vhs",
			adjacent_liked="Gg, Gs, Ww, Rb, Rb, Gd, Gg, Rb, Gg, Gs, Rb",
			rating=7
		},
		-- rough villages
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vo",
			adjacent_liked="Gs, Gs, Ww, Rb, Rb, Gd, Gd, Rb, Rb, Gd, Hh, Hh^Fds, Hh^Tf, Mm",
			rating=6
		},
		dr_village {
			terrain="Hh^Fds",
			convert_to="Hh^Vo",
			adjacent_liked="Gs, Gs, Ww, Rb, Rb, Gd, Gd, Rb, Rb, Gd, Hh, Hh^Fds, Hh^Tf, Mm",
			rating=6
		},
		dr_village {
			terrain="Hh^Tf",
			convert_to="Hh^Vo",
			adjacent_liked="Gs, Gs, Ww, Rb, Rb, Gd, Gd, Rb, Rb, Gd, Hh, Hh^Fds, Hh^Tf, Mm",
			rating=6
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vo",
			adjacent_liked="Gs, Gs, Ww, Rb, Rb, Gd, Gd, Rb, Rb, Gd, Hh, Hh^Fds, Hh^Tf, Mm",
			rating=5
		},
		-- cave villages
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vud",
			adjacent_liked="Hh,Uu,Mm,Uu,Uh,Uh,Uh,Hh^Tf,Xu,Uu^Tf,Mm^Xm",
			rating=6
		},
		dr_village {
			terrain="Uh",
			convert_to="Uu^Vud",
			adjacent_liked="Hh,Uu,Mm,Uu,Uh,Uh,Uh,Hh^Tf,Xu,Uu^Tf,Mm^Xm",
			rating=6
		},
		dr_village {
			terrain="Uu^Tf",
			convert_to="Uu^Vud",
			adjacent_liked="Hh,Uu,Mm,Uu,Uh,Uh,Uh,Hh^Tf,Xu,Uu^Tf,Mm^Xm",
			rating=6
		},
		-- water villages
		dr_village {
			terrain="Wwt",
			convert_to="Wwt^Vm",
			adjacent_liked="Wwt, Ww, Gs, Gg",
			rating=1
		},

	}
	res.castle = {
		valid_terrain="Gs",
		min_distance=14,
	}

	return default_generate_map(res)
end
return generate
