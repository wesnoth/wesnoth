

local function dr_wild_temp(terrain_a, terrain_b, terrain_c, terrain_d)
	return flatten1 {
		dr_temperature(terrain_a, 325, 500, terrain_b),
		dr_temperature(terrain_a, 500, 675, terrain_c),
		dr_temperature(terrain_a, 675, 999, terrain_d),
	}
end

local function dr_wild_roads(cost, terrain_a, terrain_b, terrain_c, terrain_d)
	return flatten1 {
		dr_road(terrain_a, "Rrc", cost),
		dr_road(terrain_b, "Rrc", cost),
		dr_road(terrain_c, "Rr", cost),
		dr_road(terrain_d, "Rr", cost),
	}
end

local function dr_wild_villagess(terrain_a, terrain_b, terrain_c, terrain_d, liked, rating)
	return flatten1 {
		dr_village {
			terrain=terrain_a,
			convert_to= terrain_a .."^Vh",
			rating=rating,
			adjacent_liked=liked,
		},
		dr_village {
			terrain=terrain_b,
			convert_to= terrain_b .. "^Vh",
			rating=rating,
			adjacent_liked=liked,
		},
		dr_village {
			terrain=terrain_c,
			convert_to=terrain_c .. "^Vh",
			rating=rating,
			adjacent_liked=liked,
		},
		dr_village {
			terrain=terrain_d,
			convert_to=terrain_d .. "^Vh",
			rating=rating,
			adjacent_liked=liked,
		},
	}
end

----------------------------------------
local function generate(length, villages, castle, iterations, size, players, island)
	----------------------------------------
	local res = wct_generator_settings_arguments( length, villages, castle, iterations, size, players, island)
	res.max_lakes=20
	res.min_lake_height=360
	res.lake_size=55
	res.river_frequency=60
	res.temperature_size=size
	res.roads=15
	res.road_windiness=6
	-- divide map in zones by altitude and temperature
	-- all terrain generated is just a "token" for postgeneration
	res.height = {
		dr_height(780, "Xu"),
		dr_height(725, "Ms^Xm"),
		dr_height(625, "Ms"),
		dr_height(475, "Ha"),
		dr_height(325, "Aa"),
		dr_height(120, "Gll"),
		dr_height(30, "Wwrg"),
		dr_height(1, "Wwg"),
		dr_height(0, "Wog"),
	}
	res.convert = {
		wct_fix_river_into_ocean("g", 29),
		
		dr_wild_temp("Xu", "Xuc", "Xue", "Xuce"),
		dr_wild_temp("Ms^Xm", "Mm^Xm", "Md^Xm", "Md^Dr"),
		dr_wild_temp("Ms", "Mm", "Md", "Mv"),
		dr_wild_temp("Ha", "Hh", "Hhd", "Hd"),
		dr_wild_temp("Aa", "Ss", "Sm", "Dd"),
		dr_wild_temp("Gll", "Gg", "Gs", "Gd"),
		dr_wild_temp("Wwrg", "Wwr", "Wwrt", "Ds"),
	}
	res.road_cost = {
		dr_wild_roads(30, "Xu", "Xuc", "Xue", "Xuce"),
		dr_wild_roads(25, "Ms^Xm", "Mm^Xm", "Md^Xm", "Md^Dr"),
		dr_wild_roads(20, "Ms", "Mm", "Md", "Mv"),
		dr_wild_roads(15, "Ha", "Hh", "Hhd", "Hd"),
		dr_wild_roads(12, "Aa", "Ss", "Sm", "Dd"),
		dr_wild_roads(10, "Gll", "Gg", "Gs", "Gd"),
		dr_road("Ww", "Wwf", 18),
		dr_road("Rr", "Rr", 2),
		dr_road("Rrc", "Rrc", 2),
		dr_road("Ch", "Ch", 2),
	}
	res.village = {
		dr_wild_villagess("Wwrg", "Wwr", "Wwrt", "Ds", "Ww,Ww,Ww,Wwf,Wwf,Wwf,Wwrg,Wwr,Wwrt,Ds,Gll,Gg,Gs,Gd", 2),
		dr_wild_villagess("Gll", "Gg", "Gs", "Gd", "Ww,Ww,Ww,Wwf,Wwf,Wwf,Aa,Ss,Sm,Dd,Gll,Gg,Gs,Gd", 6),
		dr_wild_villagess("Aa", "Ss", "Sm", "Dd", "Ww,Ww,Ww,Wwf,Wwf,Wwf,Aa,Ss,Sm,Dd,Gll,Gg,Gs,Gd,Ha,Hh,Hhd,Hd", 6),
		dr_wild_villagess("Ha", "Hh", "Hhd", "Hd", "Ww,Ww,Ww,Wwf,Wwf,Wwf,Aa,Ss,Sm,Dd,Ms,Mm,Md,Mv,Ha,Hh,Hhd,Hd", 5),
		dr_wild_villagess("Ms", "Mm", "Md", "Mv", "Ww,Ww,Ww,Wwf,Wwf,Wwf,Ms,Mm,Md,Mv,Ha,Hh,Hhd,Hd", 4),
		dr_wild_villagess("Xu", "Xuc", "Xue", "Xuce", "Ww,Xu Xuc,Xue,Xuce", 3),
		-- mermen villages
		dr_village {
			terrain="q",
			convert_to="Wwg^Vm",
			rating=1,
			adjacent_liked="Wwg, Wwg",
		},
	}
	res.castle = { 
		valid_terrain="Gll, Gs, Gg, Gd, Aa, Ss, Sm, Dd, Ms, Mm, Md, Mv, Ha, Hh, Hhd, Hd",
		min_distance=13,
	}
	return default_generate_map(res)
end
return generate
