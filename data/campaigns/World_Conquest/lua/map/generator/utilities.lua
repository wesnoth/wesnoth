
function flatten1(list)
	local res = {}
	assert(type(list) == "table")
	for i1, v1 in ipairs(list) do
		assert(type(v1) == "table")
		for i2, v2 in ipairs(v1) do
			res[#res + 1] = v2
		end
	end
	return res
end

function dr_height(height, terrain)
	return {
		height = height,
		terrain = terrain,
	}
end

--	dr_convert(nil, nil, nil, nil, "aaaa", "aaaa"),
function dr_convert(min, max, mint, maxt, from, to)
	return { {
		min_height=min,
		max_height=max,
		min_temperature=mint,
		max_temperature=maxt,
		from=from,
		to=to,
	} }
end

function dr_temperature(from, mint, maxt, to)
	return { {
		min_temperature=mint,
		max_temperature=maxt,
		from=from,
		to=to,
	} }
end


function dr_village(t)
	return { t }
end

function wct_fix_river_into_ocean(t, height)
	-- generator uses Ww for rivers
	-- best way to difference ocean from river in postgeneration is use different terrain for ocean
	-- this macro fix water of different color into ocean
	-- as rivers are generated until map border
	return {
		{
			max_height=0,
			from="Ww",
			to="Wo" ..t ,
		},
		{
			max_height=height,
			from="Ww",
			to="Ww" .. t,
		}
	}
end

function dr_road(from, to, cost)
	return { {
		terrain = from, 
		cost = cost,
		convert_to = to
	} }
end

function dr_bridge(from, bridge, road, cost)
	return { {
		terrain = from, 
		cost = cost,
		convert_to_bridge = bridge .. "|," .. bridge .. "/," .. bridge .. "\\",
		convert_to = road,
	} }
end


function dr_road_over_bridges(bridge, cost)
	return flatten1 {
		dr_road(bridge .. "|", bridge .. "|", cost),
		dr_road(bridge .. "/", bridge .. "/", cost),
		dr_road(bridge .. "\\", bridge .. "\\", cost),
	}
end

function wct_generator_settings_arguments(length, villages, castle, iterations, hill_size, players, island)
	local width = length
	if width % 2 == 1 then
		width = width + 1
	end
	return {
		border_size=0,
		map_width=width,
		map_height=length,
		iterations=iterations,
		hill_size=hill_size,
		villages=villages,
		nplayers=players,
		island_size=island,
		castle_size=castle,
		temperature_iterations=iterations,
		link_castles=true,
	}
end

function wct_generator_village(GG, GS, DS, UU, UH, GSFP, HH, MM, AA, AAFPA, SS, WW)
	return flatten1 {
		dr_village {
			terrain="Gg",
			convert_to="Gg^Vh",
			rating=GG,
			adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
		},
		dr_village {
			terrain="Gs",
			convert_to="Gs^Vht",
			rating=GS,
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
		},
		dr_village {
			terrain="Ds",
			convert_to="Dd^Vda",
			rating=DS,
			adjacent_liked="Gg, Gs, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
		},
		dr_village {
			terrain="Uu",
			convert_to="Uu^Vud",
			rating=UU,
			adjacent_liked="Re,Hh,Mm,Uu,Uh,Xu",
		},
		dr_village {
			terrain="Uh",
			convert_to="Uu^Vu",
			rating=UH,
			adjacent_liked="Re,Hh,Mm,Uu,Uh,Xu",
		},
		dr_village {
			terrain="Hh",
			convert_to="Hh^Vhh",
			rating=HH,
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
		},
		dr_village {
			terrain="Mm",
			convert_to="Mm^Vhh",
			rating=MM,
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
		},
		-- villages in forest are Elvish
		dr_village {
			terrain="Gs^Fp",
			convert_to="Gg^Ve",
			rating=GSFP,
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp, Gs^Fp, Gs^Fp",
		},
		dr_village {
			terrain="Aa^Fpa",
			convert_to="Aa^Vea",
			rating=AAFPA,
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
		},
		dr_village {
			terrain="Aa",
			convert_to="Aa^Vha",
			rating=AA,
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
		},
		dr_village {
			terrain="Ss",
			convert_to="Ss^Vhs",
			rating=SS,
			adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bw|, Ww^Bw/, Ww^Bw\\, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
		},
		-- mermen villages - give them low chance of appearing
		dr_village {
			terrain="Ww",
			convert_to="Ww^Vm",
			rating=WW,
			adjacent_liked="Ww, Ww",
		},
	}
end

function wct_generator_road_cost_classic()
	return flatten1 {
		dr_road("Gg", "Re", 10),
		dr_road("Gs^Fp", "Re", 20),
		dr_road("Hh", "Re", 30),
		dr_road("Mm", "Re", 40),
		dr_road("Mm^Xm", "Re", 75),
		dr_bridge("Ql", "Ql^Bs", "Re", 100),
		dr_bridge("Qxu", "Qxu^Bs", "Re", 100),
		dr_road("Uu", "Re", 10),
		dr_road("Uh", "Re", 40),
		dr_road("Xu", "Re", 90),
		-- road going through snow is covered over by the snow
		-- (in classic WC people were too lazy to clean it :P)
		dr_road("Aa", "Aa", 20),
		dr_road("Re", "Re", 2),
		dr_road_over_bridges("Ww^Bw", 2),
		dr_road("Ch", "Ch", 2),
		dr_road("Ce", "Ce", 2),
	}
end

function default_generate_map(data)
	local max_island = 10
	local max_coastal = 5
	
	data.village = flatten1(data.village or {})
	data.road_cost = flatten1(data.road_cost or {})
	data.convert = flatten1(data.convert or {})
	
	local cfg = wc2_convert.lon_to_wml(data, "mg_main")
	
	local w, h = cfg.map_width, cfg.map_height
	local orig_island_size = cfg.island_size
	local orig_nvillages = cfg.villages
	
	cfg.island_size = 0;
	cfg.nvillages = (orig_nvillages * w * h) // 1000;
	cfg.island_off_center = 0;
	
	if orig_island_size >= max_coastal then
		-- Islands look good with much fewer iterations than normal, and fewer lakes
		cfg.iterations = cfg.iterations // 10
		cfg.max_lakes = (cfg.max_lakes or 0) // 9
		
		-- The radius of the island should be up to half the width of the map
		local island_radius = 50 + ((max_island - orig_island_size) * 50) // (max_island - max_coastal)
		cfg.island_size = (island_radius * (w/2)) // 100;
	elseif orig_island_size > 0 then
		-- The radius of the island should be up to twice the width of the map
		local island_radius = 40 + ((max_coastal - orig_island_size) * 40) // max_coastal;
		cfg.island_size = (island_radius * w * 2) // 100;
		cfg.island_off_center = math.min(w, h);
	else
	end
	for i = 1, 20 do
		local status, map = pcall(function()
			cfg.seed = mathx.random(5000) + 7
			return wesnoth.map.generate(w, h, cfg)
		end)
		if status then
			return map
		end
	end
	cfg.seed = mathx.random(5000) + 7
	return wesnoth.map.generate(w, h, cfg)
end
