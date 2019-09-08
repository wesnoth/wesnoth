-- This is basicially a lua port of the mapgenerator of the
-- scenario 1p_6B_Maritime from the addon World Conquest II

local functional = wesnoth.require "functional"
local helper = wesnoth.require "helper"
local time_stamp_start = wesnoth.get_time_stamp()

-- helper function to create filters.
local function f_terrain(terrain)
	return { "terrain", terrain }
end

local function f_and(...)
	return { "all",  ... }
end

local function f_not(...)
	return { "none",  ... }
end

local function f_or(...)
	return { "any",  ... }
end

local function f_adjacent(f, ad, count)
	return { "adjacent",  f, adjacent = ad, count = count }
end

local function f_findin(findin)
	return { "find_in", findin}
end

local function f_radius(r, f, f_r)
	return { "radius", r, f, filter_radius = f_r}
end

local function log(msg)
	wesnoth.log("err", msg)
end

-- arguments for the default map generator
local arg = {
	length = 66,
	villages = 14,
	castle = 10,
	iterations = 16320,
	size =8,
	nplayers = 7,
	island = 7,
}

-- build the wml table to pass to the default map generator
local d_mapgen_data = {	
	border_size = 0,
	map_width = arg.length,
	map_height = arg.length,
	iterations = arg.iterations,
	hill_size = arg.size,
	nvillages = arg.villages,
	nplayers = arg.nplayers,
	island_size = arg.island,
	castle_size = arg.castle,
	temperature_iterations = arg.iterations,
	
	seed = wesnoth.random(4000),

	max_lakes=90,
	min_lake_height=250,
	lake_size=60,
	river_frequency=100,
	temperature_size=7,
	roads=25,
	road_windiness=2,
}

-- list of common terrain types which come in at different heights, from highest to lowest
local d_mapgen_data_heights = {
	{910, "Uh"},
	{820, "Uu"},
	{780, "Xu"},
	{765, "Mm^Xm"},
	{725, "Mm"},
	{610, "Hh"},
	{600, "Gg"},
	{590, "Hh^Fp"},
	{580, "Gg"},
	{570, "Gs^Fp"},
	{410, "Gg"},
	{400, "Mm"},
	{360, "Gg"},
	{340, "Hh^Fp"},
	{320, "Gg"},
	{300, "Gs^Fp"},
	{240, "Gg"},
	{220, "Gs^Fp"},
	{200, "Hh^Fp"},
	{180, "Hh"},
	{100, "Gg"},
	{30, "Ds"},
	{1, "Wwg"},
	{0, "Wog"}
}

function fix_river_into_ocean(t, h, convert_data)
	table.insert(convert_data, {
		max_height = 0,
		from="Ww",
		to = "Wo" .. t,
	})
	table.insert(convert_data, {
		max_height = h,
		from="Ww",
		to = "Ww" .. t,
	})
end

function dr_convert(min, max, mint, maxt, from, to)
	return {
		min_height=min,
		max_height=max,
		min_temperature=mint,
		max_temperature=maxt,
		from=from,
		to=to,
	}
end

function dr_temperature(from, mint, maxt, to)
	return {
		min_temperature=mint,
		max_temperature=maxt,
		from=from,
		to=to,
	}
end

local d_mapgen_data_conv = {
	-- dr_convert MIN_HT MAX_HT MIN_TMP MAX_TMP FROM TO
	-- low temperatures
	dr_convert(50, 999, 0, 270, "Ww, Wo", "Ai"),
	dr_convert(350, 999, 0, 320, "Gg", "Aa"),
	dr_convert(330, 999, 320, 370, "Gg", "Gd"),
	dr_convert(100, 999, 370, 475, "Gg", "Gs"),
	dr_convert(350, 999, 0, 310, "Gs^Fp", "Aa^Fpa"),
	dr_convert(350, 999, 0, 345, "Hh", "Ha"),
	dr_convert(350, 999, 0, 335, "Hh^Fp", "Ha^Fpa"),
	dr_convert(350, 999, 0, 370, "Mm", "Ms"),
	dr_convert(350, 999, 0, 390, "Mm^Xm", "Ms^Xm"),
	-- swamp appears on low land, at mod temp
	dr_convert(0, 200, 400, 700, "Gg", "Ss"),
	-- pine appears at low temperatures
	dr_convert(150, 999, 320, 420, "Gg", "Gs^Fp"),
	dr_convert(150, 999, 320, 420, "Hh", "Hh^Fp"),
	-- decidius appears at mod temperatures with some heigh
	dr_convert(300, 999, 510, 540, "Gg,Gs", "Gg^Fds"),
	dr_convert(300, 999, 510, 540, "Hh", "Hh^Fds"),
	-- fungus appears at med temp and high
	dr_convert(825, 950, 500, 525, "Uu, Uh", "Uu^Uf"),
	dr_convert(825, 950, 550, 575, "Uu, Uh", "Uu^Uf"),
	dr_convert(825, 950, 600, 625, "Uu, Uh", "Uu^Uf"),
	-- high temperatures
	dr_convert(800, 999, 850, 999, "Uu, Uh, Uu^Uf", "Ql"),
	dr_convert(260, 999, 800, 999, "Gg", "Dd"),
	dr_convert(230, 999, 750, 999, "Gg", "Gd"),
	dr_convert(100, 999, 650, 999, "Gg", "Gs"),
	dr_convert(460, 630, 800, 999, "Ds, Hh", "Hd"),

	-- dr_temperature FROM MIN MAX TO
	-- convert forest at different temperatures
	dr_temperature("Gs^Fp", 420, 475, "Gs^Fdf"),
	dr_temperature("Hh^Fp", 420, 510, "Hh^Fmf"),
	dr_temperature("Gs^Fp", 475, 510, "Gg^Fdf"),
	dr_temperature("Gs^Fp", 510, 540, "Gg^Fds"),
	dr_temperature("Hh^Fp", 510, 540, "Hh^Fds"),
	dr_temperature("Gs^Fp", 540, 650, "Gg^Ftr"),
	dr_temperature("Hh^Fp", 540, 650, "Hh^Fms"),
}

function dr_road(from, to, cost)
	return {
		terrain = from, 
		cost = cost,
		convert_to = to
	}
end

function dr_bridge(from, bridge, road, cost)
	return {
		terrain = from, 
		cost = cost,
		convert_to_bridge = bridge .. "|," .. bridge .. "/," .. bridge .. "\\",
		convert_to = road,
	}
end

local d_mapgen_data_road = {
	--begin road classic
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
	--{DR_ROADS_OVER_BRIDGES Ww^Bw 2}
	dr_road("Ww^Bw|", "Ww^Bw|", 2),
	dr_road("Ww^Bw/", "Ww^Bw/", 2),
	dr_road("Ww^Bw\\", "Ww^Bw\\", 2),
	dr_road("Ch", "Ch", 2),
	dr_road("Ce", "Ce", 2),
	--end road classic
	dr_road("Gs", "Re", 10),
	dr_road("Gd", "Re", 10),
	dr_road("Gg^Fds", "Re", 20),
	dr_road("Gg^Ftr", "Re", 20),
	dr_road("Gg^Fdf", "Re", 20),
	dr_road("Hh^Fmf", "Re", 20),
	dr_road("Hh^Ft", "Re", 30),
	dr_road("Hh^Fp", "Re", 30),
	dr_road("Hh^Fds", "Re", 20),
	dr_road("Hh^Fms", "Re", 20),
	dr_road("Gs^Ft", "Re", 30),
	dr_road("Ds", "Re", 20),
	dr_bridge("Ww", "Ww^Bsb", "Chw", 35),
	dr_road("Chw", "Chw", 2),
	--{DR_ROADS_OVER_BRIDGES Ww^Bsb 2}
	dr_road("Ww^Bsb|", "Ww^Bsb|", 2),
	dr_road("Ww^Bsb/", "Ww^Bsb/", 2),
	dr_road("Ww^Bsb\\", "Ww^Bsb\\", 2)
}
local d_mapgen_data_village = {
	{
		terrain="Gg",
		convert_to="Gg^Vh",
		adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg",
		rating=8,
	},
	{
		terrain="Gs",
		convert_to="Gs^Vh",
		adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gd, Gd, Gs, Gs",
		rating=5,
	},
	{
		terrain="Gd",
		convert_to="Gd^Vc",
		adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gd, Gd, Gs, Gs, Gd",
		rating=5,
	},
	{
		terrain="Ds",
		convert_to="Dd^Vda",
		rating=2,
		adjacent_liked="Gg, Gs, Gd, Wwg, Wwg, Wwg, Re, Re, Hh, Ch, Wog,",
	},
	{
		terrain="Uu",
		convert_to="Uu^Vud",
		rating=4,
		adjacent_liked="Re,Hh,Mm,Uu,Uh,Xu",
	},
	{
		terrain="Uh",
		convert_to="Uu^Vu",
		rating=3,
		adjacent_liked="Re,Hh,Mm,Uu,Uh,Xu",
	},
	{
		terrain="Gg^Fds",
		convert_to="Gg^Vh",
		adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gg^Fds",
		rating=4,
	},
	{
		terrain="Gs^Fp",
		convert_to="Gs^Vh",
		rating=4,
		adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gd, Gd, Gs, Gs, Gs^Fp",
	},
	{
		terrain="Hh",
		convert_to="Hh^Vhh",
		rating=4,
		adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gd, Gd, Gs, Gs, Gs^Fp",
	},
	{
		terrain="Hh^Fp",
		convert_to="Hh^Vhh",
		rating=4,
		adjacent_liked="Gg, Ww, Ww, Ww, Ww, Ww, Ww, Ww, Re, Re, Re, Re, Hh, Gs, Gg, Gd, Gd, Gs, Gs, Gs^Fp",
	},
	{
		terrain="Mm",
		convert_to="Mm^Vhh",
		rating=4,
		adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bsb|, Ww^Bsb/, Ww^Bsb\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
	},
	-- villages in snow
	{
		terrain="Aa",
		convert_to="Aa^Vha",
		rating=3,
		adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bsb|, Ww^Bsb/, Ww^Bsb\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
	},
	{
		terrain="Aa^Fpa",
		convert_to="Aa^Vea",
		rating=3,
		adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bsb|, Ww^Bsb/, Ww^Bsb\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
	},
	-- swamp villages
	{
		terrain="Ss",
		convert_to="Ss^Vhs",
		rating=2,
		adjacent_liked="Gg, Ww, Ww, Ww, Ww^Bsb|, Ww^Bsb/, Ww^Bsb\\, Rr, Rr, Re, Re, Gg^Ve, Gg^Vh, Hh, Gs^Fp",
	},
	-- merfolk villages - give them low chance of appearing
	{
		terrain="Wwg",
		convert_to="Wwg^Vm",
		rating=1,
		adjacent_liked="Wwg, Wwg, Ch, Ss, Ds, Ds",
	}
}

local max_island = 10
local max_coastal = 5

local w, h = d_mapgen_data.map_width, d_mapgen_data.map_height
local orig_island_size = d_mapgen_data.island_size
local orig_nvillages = d_mapgen_data.nvillages

if true then
	-- port of the code in deault_map_generator.cpp
	d_mapgen_data.island_size = 0;
	d_mapgen_data.nvillages = (orig_nvillages * w * h) / 1000;
	d_mapgen_data.island_off_center = 0;

	if orig_island_size >= max_coastal then
		-- Islands look good with much fewer iterations than normal, and fewer lakes
		d_mapgen_data.iterations = d_mapgen_data.iterations // 10
		d_mapgen_data.max_lakes = (d_mapgen_data.max_lakes or 0) // 9

		-- The radius of the island should be up to half the width of the map
		local island_radius = 50 + ((max_island - orig_island_size) * 50) // (max_island - max_coastal)
		d_mapgen_data.island_size = (island_radius * (w/2)) // 100;
	elseif orig_island_size > 0 then
		-- The radius of the island should be up to twice the width of the map
		local island_radius = 40 + ((max_coastal - orig_island_size) * 40) // max_coastal;
		d_mapgen_data.island_size = (island_radius * w * 2) // 100;
		d_mapgen_data.island_off_center = math.min(w, h);
	else
	end
end

if true then
	for i, v in ipairs(d_mapgen_data_heights) do
		table.insert(d_mapgen_data, wml.tag.height {
			height=v[1],
			terrain=v[2],
		})
	end
	fix_river_into_ocean("g", 29, d_mapgen_data_conv)
	for i, v in ipairs(d_mapgen_data_conv) do
		table.insert(d_mapgen_data, wml.tag.convert(v))
	end
	for i, v in ipairs(d_mapgen_data_road) do
		table.insert(d_mapgen_data, wml.tag.road_cost(v))
	end
	for i, v in ipairs(d_mapgen_data_village) do
		table.insert(d_mapgen_data, wml.tag.village(v))
	end
	table.insert(d_mapgen_data, wml.tag.castle{
		valid_terrain="Gs, Gg, Gd, Gs^Fp, Gg^Ft, Gg^Fds, Gg^Ftr, Hh, Hh^Fp, Hh^Ft, Hh^Fds, Hh^Fms, Ha^Fpa, Ha, Hh^Fmf, Gg^Fdf",
		min_distance=15,
	})
end

local map = wesnoth.create_map(wesnoth.generate_default_map(w, h, d_mapgen_data))
--curretnly unused, todo: put this in the scenario when using scenario_generation.
local prestart_event = {}

-- helper functions for lua map generation.
function get_locations(t)
	return map:get_locations(t)
end

function set_terrain_impl(data)
	local locs = {}
	local nlocs_total = 0
	for i = 1, #data do
		local f = wesnoth.create_filter(data[i].filter, data[i].known or {})
		locs[i] = map:get_locations(f)
		nlocs_total = nlocs_total + #locs[1]
	end
	local nlocs_changed = 0
	for i = 1, #data do
		local d = data[i]
		local chance = d.per_thousand
		local terrains = d.terrain
		local layer = d.layer
		local num_tiles = d.nlocs and math.min(data[i], d.nlocs) or #locs[i]
		if d.exact then
			num_tiles = math.ceil(num_tiles * chance / 1000)
			chance = 1000
			helper.shuffle(locs[i])
		end
		for j = 1, num_tiles do
			local loc = locs[i][j] 
			if chance >= 1000 or chance >= wesnoth.random(1000) then
				map:set_terrain(loc, helper.rand(terrains), layer)
				nlocs_changed = nlocs_changed + 1
			end
		end
	end
end

function set_terrain_simul(cfg)
	cfg = helper.parsed(cfg)
	local data = {}
	for i, r in ipairs(cfg) do
		r_new = {
			filter = r[2],
			terrain = r[1],
			layer = r.layer,
			exact = r.exact ~= false,
			per_thousand = 1000,
			nlocs = r.nlocs,
			known = r.known
		}
		if r.percentage then
			r_new.per_thousand = r.percentage * 10
		elseif r.per_thousand then
			r_new.per_thousand = r.per_thousand;
		elseif r.fraction then
			r_new.per_thousand = math.ceil(1000 / r.fraction);
		elseif r.fraction_rand then
			r_new.per_thousand = math.ceil(1000 / helper.rand(r.fraction_rand));
		end
		table.insert(data, r_new)
	end
	set_terrain_impl(data)
end

function set_terrain(a)
	set_terrain_simul({a})
end

function noise_maritime()
	set_terrain_simul {
		{ "Gs^Fp,Gs^Fp,Hh,Hh,Hh",
			f_terrain("Gs"),
			per_thousand=125,
			exact=false,
		},
		{ "Hhd,Hhd,Hhd,Hd,Rd,Rd,Rd,Rd,Rd,Rd,Dd^Dr,Rd^Fet,Rd^Fdw,Dd^Do",
			f_terrain("Dd"),
			per_thousand=125,
			exact=false,
		},
		{ "Gg^Fet,Gg^Fet,Hh,Hh,Hh",
			f_terrain("Gg"),
			per_thousand=222,
			exact=false,
		},
		{ "Gd^Fmw,Gd^Fp,Hh^Fmw,Hh,Mm,Hh,Hh",
			f_terrain("Gd"),
			per_thousand=222,
			exact=false,
		},
		{ "Aa^Fpa,Aa^Fpa,Wwf,Ha,Ha,Ha",
			f_terrain("Aa"),
			per_thousand=222,
			exact=false,
		},
		{ "Ur,Uu^Uf,Uh,Uu",
			f_terrain("Uu,Uh,Uu^Uf"),
			per_thousand=410,
			exact=false,
		},
		{ "Gs,Gs,Gs,Gs,Hh,Hh,Hh,Gs^Fp,Gs^Fp,Gs^Fp,Mm^Xm",
			f_terrain("Mm"),
			per_thousand=460,
			exact=false,
		},
		{ "Gs,Gs,Gs,Gs,Gs,Gs,Gs^Fp,Gs^Fp,Gs^Fp,Mm",
			f_terrain("Hh"),
			per_thousand=590,
			exact=false,
		},
		{ "Aa,Aa,Aa,Aa,Wwf,Ai,Aa^Fpa,Aa^Fpa,Aa^Fpa,Mm",
			f_terrain("Ha"),
			per_thousand=610,
			exact=false,
		},
		{ "Mm,Hh,Hh",
			f_terrain("G*^F*"),
			per_thousand=125,
			exact=false,
		},
		{ "Aa,Aa,Aa,Aa,Ai,Wwf,Gg^Fet,Ha,Ha,Ha,Ha",
			f_terrain("Aa^Fpa"),
			per_thousand=680,
			exact=false,
		},
		{ "Mm,Mm",
			f_and(
				f_terrain("Mm^Xm"),
				f_adjacent(f_terrain("Xu")),
				f_adjacent(f_terrain("Uu,Uh,Uu^Uf")),
				f_adjacent(f_not(f_terrain("Uu,Uh,Uu^Uh,Xu")))
			),
			per_thousand=125,
			exact=false,
		}
	}
end

function reduce_wall_clusters(cave_terrain)
	set_terrain { cave_terrain,
		f_and(
			f_terrain("Xu"),
			f_adjacent(f_terrain("Xu,M*^Xm"), nil, "3-6")
		)
	}
end

function fill_lava_chasms()
	set_terrain { "Ql",
		f_and(
			f_terrain("Qxu^*"),
			f_radius(999, f_terrain("Ql^*"), f_terrain("Ql^*,Qxu^*"))
		),
		layer = "base"
	}
end

function volcanos()
	set_terrain { "Mv",
		f_and(
			f_terrain("Ql,Qlf"),
			f_adjacent(f_terrain("M*,M*^Xm,X*"), "se,s,sw", "3")
		),
	}
	set_terrain { "Md^Xm",
		f_and(
			f_terrain("X*,M*^Xm"),
			f_adjacent(f_terrain("Mv"), "n,ne,nw")
		),
	}
	set_terrain { "Md",
		f_and(
			f_terrain("Ms,Mm"),
			f_adjacent(f_terrain("Mv"), "n,ne,nw")
		),
	}
	local locs = get_locations(f_terrain("Mv"))
	for i, loc in ipairs(locs) do
		table.insert(prestart_event, wml.tag.sound_source {
			id = "volcano" .. i,
			sounds = "rumble.ogg",
			delay = 550000,
			chance = 1,
			x = loc[1],
			y = loc[2],
			full_range = 5,
			fade_range = 5,
			loop = 0
		})
	end
end

function volcanos_dirt()
	
	set_terrain { "*^Dr",
		f_and(
			f_terrain("Hh,Hd,Hhd"),
			f_radius(3, f_terrain("Mv"))
		),
		layer = "overlay",
		fraction = 3,
	}
	set_terrain { "Dd^Dc",
		f_and(
			f_terrain("Ds,Dd"),
			f_radius(4, f_terrain("Mv"))
		),
		fraction = 2,
	}
end

function cave_path_to(terrain)
	set_terrain { terrain,
		f_terrain("Ur")
	}
end

function noise_snow_to(terrain)
	local noise_snow = "Sm^Em"
	set_terrain { terrain,
		f_terrain(noise_snow)
	}
end

function get_roads_to_docks(radius)
	return get_locations(f_and(
		f_terrain("!,W*^*"),
		f_adjacent(f_and(
			f_terrain("Iwr^Vl,Rp"),
			f_adjacent(f_and(
				f_terrain("Rp"),
				f_radius(radius, f_terrain("Ch*,Kh*^*,Re"), f_terrain("!,W*^*"))
			), nil, "0"),
			f_not(f_radius(radius, f_terrain("Ch*,Kh*^*,Re"), f_terrain("!,W*^*")))
		)),
		f_radius(radius, f_terrain("Ch*,Kh*^*,Re"), f_terrain("!,W*^*"))
	))
end

function get_roads_to_river(radius)
	return get_locations(f_and(
		f_terrain("!,W*^*"),
		f_adjacent(f_and(
			--our target or the previs step
			f_terrain("*^Vhc,Rp"),
			--not a part of a road to a previous taget
			f_adjacent(f_and(
				f_terrain("Rp"),
				f_radius(radius, f_terrain("Ch*,Kh*^*,Re"), f_terrain("!,W*^*"))
			), nil, "0"),
			--and don't use parts that don't brong us closer
			f_not(f_radius(radius, f_terrain("Ch*,Kh*^*,Re"), f_terrain("!,W*^*")))
		)),
		f_radius(radius, f_terrain("Ch*,Kh*^*,Re"), f_terrain("!,W*^*"))
	))
end


function iterate_roads_to(get_next, radius, terrain)
	for r = radius, 1, -1 do
		local locs = get_next(r)
		while #locs > 0 do
			local loc = locs[wesnoth.random(#locs)]
			map:set_terrain(loc, terrain)
			locs = get_next(r)
		end
	end
end

function reduce_castle_expandingrecruit(castle, terrain)
	set_terrain { terrain,
		f_and (
			f_terrain(castle),
			f_adjacent(f_terrain("Ch,Cha,Kh*^*"))
		),
	}
end

function get_possible_maritime_bridge()
	return {
		{
			type = "Bsb|",
			locs = get_locations(f_and(
				f_adjacent(f_terrain("Chw"), "s,n"),
				f_adjacent(f_terrain("Ch,Kh"), "s,n"),
				f_adjacent(f_terrain("*^B*"), nil, "0")
			))
		},
		{
			
			type = "Bsb\\",
			locs = get_locations(f_and(
				f_adjacent(f_terrain("Chw"), "se,nw"),
				f_adjacent(f_terrain("Ch,Kh"), "se,nw"),
				f_adjacent(f_terrain("*^B*"), nil, "0")
			))
		},
		{
			
			type = "Bsb/",
			locs = get_locations(f_and(
				f_adjacent(f_terrain("Chw"), "sw,ne"),
				f_adjacent(f_terrain("Ch,Kh"), "sw,ne"),
				f_adjacent(f_terrain("*^B*"), nil, "0")
			))
		},
	}
end

function maritime_bridges()
	local pb = get_possible_maritime_bridge()
	while #pb[1] > 0 or #pb[2] > 0 or #pb[3] > 0 do
		pb = functional.filter(pb, function(t) return #t.locs >0 end)
		local sel = pb[wesnoth.random(#pb)]
		local loc = sel.locs[wesnoth.random(#sel.locs)]
		map:set_terrain(loc, "Ww^" .. sel.type)
		pb = get_possible_maritime_bridge()
	end
end

function decoration()
	-- rich terrain around rivers
	set_terrain { "*^Vhc",
		f_and(
			f_terrain("H*^V*"),
			f_adjacent(f_terrain("Ww"))
		),
		layer = "overlay"
	}
	set_terrain { "Rp^Vhc",
		f_and(
			f_terrain("G*^V*"),
			f_adjacent(
				f_terrain("Ww")
			)
		)
	}
	set_terrain { "Gg",
		f_and(
			f_terrain("G*^*"),
			f_adjacent(
				f_terrain("Ww")
			)
		),
		layer = "base"
	}
	set_terrain { "Gg",
		f_and(
			f_terrain("Gs^*,Gd^*"),
			f_radius(2,
				f_terrain("Ww")
			)
		),
		layer = "base",
		fraction = 3,
	}
	set_terrain { "Gg",
		f_and(
			f_terrain("Gs^*,Gd^*"),
			f_radius(3,
				f_terrain("Ww")
			)
		),
		layer = "base",
		fraction = 3,
	}
	set_terrain { "Gs^*",
		f_and(
			f_terrain("Gd*^*"),
			f_radius(3,
				f_terrain("Ww")
			)
		),
		layer = "base",
	}
	-- generate big docks villages
	set_terrain { "Iwr^Vl",
		f_and (
			f_terrain("*^V*"),
			f_adjacent(f_terrain("W*^*"), nil, "1-5"),
			f_adjacent(f_terrain("Wog,Wwg")),
			f_radius(4, f_terrain("Ch,Kh*^*"))
		)
	}
	
	iterate_roads_to(get_roads_to_docks, 3, "Rp")
	iterate_roads_to(get_roads_to_river, 3, "Rp")
	
	if #get_locations(f_terrain("Iwr^Vl")) == 0 then
		local locs = get_locations(
			f_and(
				f_terrain("*^V*"),
				f_adjacent(f_terrain("W*^*"), nil, "2-5"),
				f_adjacent(f_terrain("Wog,Wwg"))
			)
		)
		loc = locs[wesnoth.random(#locs)];
		map:set_terrain(loc, "Iwr^Vl")
	end
	
	set_terrain { "Wwg,Iwr,Wwg^Bw\\,Wwg^Bw\\,Wwg^Bw\\,Wwg^Bw\\",
		f_and (
			f_terrain("Wog,Wwg"),
			f_adjacent(f_terrain("Wog,Wwg"), "se,nw"),
			f_adjacent(f_terrain("Iwr^Vl"), "se,nw")
		)
	}
	set_terrain { "Wwg,Iwr,Wwg^Bw/,Wwg^Bw/,Wwg^Bw/,Wwg^Bw/",
		f_and (
			f_terrain("Wog,Wwg"),
			f_adjacent(f_terrain("Wog,Wwg"), "sw,ne"),
			f_adjacent(f_terrain("Iwr^Vl"), "sw,ne")
		)
	}
	set_terrain { "Wwg,Iwr,Wwg^Bw|,Wwg^Bw|,Wwg^Bw|,Wwg^Bw|",
		f_and (
			f_terrain("Wog,Wwg"),
			f_adjacent(f_terrain("Wog,Wwg"), "s,n"),
			f_adjacent(f_terrain("Iwr^Vl"), "s,n")
		)
	}
	set_terrain { "Wwg,Wwg^Bw\\",
		f_and (
			f_terrain("Wog,Wwg"),
			f_adjacent(f_terrain("Wog,Wwg"), "se,nw"),
			f_adjacent(f_terrain("Iwr"), "se,nw")
		)
	}
	set_terrain { "Wwg,Wwg^Bw/",
		f_and (
			f_terrain("Wog,Wwg"),
			f_adjacent(f_terrain("Wog,Wwg"), "sw,ne"),
			f_adjacent(f_terrain("Iwr"), "sw,ne")
		)
	}
	set_terrain { "Wwg,Wwg^Bw|",
		f_and (
			f_terrain("Wog,Wwg"),
			f_adjacent(f_terrain("Wog,Wwg"), "s,n"),
			f_adjacent(f_terrain("Iwr"), "s,n")
		)
	}
	for i, loc in ipairs(get_locations(f_terrain("Iwr"))) do
		if wesnoth.random(2) == 2 then
			table.insert(prestart_event, wml.tag.item {
				x = loc[1],
				y = loc[2],
				image = "misc/blank-hex.png~BLIT(units/transport/boat.png~CROP(0,15,72,57))"
			})
		else
			table.insert(prestart_event, wml.tag.item {
				x = loc[1],
				y = loc[2],
				image = "misc/blank-hex.png~BLIT(units/transport/boat.png~FL()~CROP(0,15,72,57))"
			})
		end
	end
	
	-- add trash around docks.
	set_terrain { "Ds^Edt",
		f_and (
			f_terrain("Ds"),
			f_adjacent(f_terrain("Iwr"))
		)
	}
	--todo: how has this any effect?
	-- add trash around docks.
	set_terrain { "Ds^Edt,Ds",
		f_and (
			f_terrain("Ds"),
			f_adjacent(f_terrain("Iwr"))
		)
	}
	set_terrain { "Ds^Edt",
		f_and (
			f_terrain("W*"),
			f_adjacent(f_terrain("Iwr")),
			f_adjacent(f_terrain("W*^*"), nil, "0")
		)
	}
	-- end docks generation.
	-- some villages tweaks.
	set_terrain { "^Vd",
		f_terrain("M*^V*"),
		fraction=2,
		layer="overlay",
	}
	set_terrain { "^Vo",
		f_terrain("H*^Vhh"),
		fraction=2,
		layer="overlay",
	}
	set_terrain { "*^Ve",
		f_and (
			f_terrain("G*^Vh"),
			f_adjacent(f_terrain("G*^F*"))
		),
		layer="overlay",
	}
	set_terrain { "*^Vht",
		f_and (
			f_terrain("G*^Vh"),
			f_adjacent(f_terrain("R*^*,C*^*,K*^*"), nil, "0"),
			f_adjacent(f_terrain("Ss"), nil, "2-6")
		),
		layer="overlay",
	}
	-- fix badlooking dunes
	set_terrain { "Hhd",
		f_and (
			f_terrain("Hd"),
			f_adjacent(f_terrain("D*^*,Rd^*"), nil, "0")
		),
	}
	-- expnad dock road
	set_terrain { "Rp",
		f_and (
			f_terrain("G*,Ds"),
			f_adjacent(f_terrain("*^Vl"))
		),
	}
	-- contruction dirt near beach roads
	set_terrain { "Hd,Ds^Dr",
		f_and (
			f_terrain("Ds"),
			f_adjacent(f_terrain("W*^*,G*^*,S*^*"), nil, "0"),
			f_adjacent(f_terrain("Rp"))
		),
	}
	-- rebuild some swamps far from rivers
	set_terrain { "Gs,Ds,Ds",
		f_and (
			f_terrain("Ss"),
			f_not(f_radius(6, f_terrain("Ww,Wwf")))
		),
		fraction=8
	}
	set_terrain { "Gs^Fp,Hh^Fp,Hh,Mm,Gs^Fp,Ss^Uf,Ss^Uf,Ss^Uf",
		f_and (
			f_terrain("Ss"),
			f_not(f_radius(6, f_terrain("Ww,Wwf")))
		),
		fraction=8
	}
	
	-- some mushrooms on hills near river or caves
	set_terrain { "Hh^Uf",
		f_and (
			f_terrain("Hh,Hh^F*"),
			f_radius(5, f_terrain("Ww,Wwf,U*^*"))
		),
		fraction=14
	}
	-- reefs
	set_terrain { "Wwrg",
		f_and (
			f_terrain("Wog"),
			f_adjacent(f_terrain("Wwg")),
			f_not(f_radius(7, f_terrain("*^Vl")))
		),
		fraction=6
	}
	-- chance of expand rivers into sea
	local r = tonumber(helper.rand("0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,2,2,3"))
	for i = 1 , r do
		local locs = get_locations(f_and(
			f_terrain("Wog,Wwg,Wwrg"),
			f_adjacent(f_terrain("Ww,Wwf,Wo"))
		))
		
		set_terrain { "Ww",
			f_and (
				f_terrain("Wwg"),
				f_findin("l")
			),
			known = { l = locs }
		}
		set_terrain { "Wo",
			f_and (
				f_terrain("Wog"),
				f_findin("l")
			),
			known = { l = locs }
		}
		set_terrain { "Wwr",
			f_and (
				f_terrain("Wwrg"),
				f_findin("l")
			),
			known = { l = locs }
		}
	end
	reduce_castle_expandingrecruit("Chw", "Wwf")
	-- soft castle towards river defense
	set_terrain { "Chw",
		f_and (
			f_terrain("Ww"),
			f_adjacent(f_terrain("C*,K*"), nil, "0"),
			f_adjacent(f_terrain("W*^*"), nil, "2-6"),
			f_adjacent(f_and(
				f_terrain("Ww"),
				f_adjacent(f_terrain("Ch,Kh"))
			)),
			f_not(f_radius(7, f_terrain("*^Vl")))
		),
		percentage=83,
		exact=false,
	}
	maritime_bridges()
end

function repaint()
	reduce_wall_clusters("Uu,Uu^Uf,Uh,Uu^Uf,Uu,Uu^Uf,Uh,Ql,Qxu,Xu,Uu,Ur")
	fill_lava_chasms()
	volcanos()
	decoration()
	volcanos_dirt()
	
	--volcanos dry mountains
	set_terrain {"Md",
		f_and(
			f_terrain("Mm^*"),
			f_radius(2, f_terrain("Mv"))
		),
		layer = "base"
	}
	--lava dry mountains
	set_terrain { "Md",
		f_and(
			f_terrain("Mm*^*"),
			f_radius(1, f_terrain("Ql"))
		),
		layer = "base"
	}
	--dirt beachs far from docks
	set_terrain { "Ds^Esd", 
		{ "all",
			{ "terrain", "Ds"},
			{ "adjacent",
				{ "terrain", "Wwg,Wog"}
			},
			{ "none",
				{ "radius", 6,
					{ "terrain", "*^Vl"}
				},
			},
		},
		fraction=10
	}
	
	cave_path_to("Rb")
	noise_snow_to("Rb")
end

function run()
	noise_maritime()
	repaint()
end

local time_stamp_middle = wesnoth.get_time_stamp()
run()
local time_stamp_end = wesnoth.get_time_stamp()
std_print("generation took " .. (time_stamp_end - time_stamp_start) .. " ticks of which " .. (time_stamp_end - time_stamp_middle) .. " were lua postgeneration.")
return map.data
