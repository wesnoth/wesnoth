-- Rural

local function world_conquest_tek_map_decoration_6a()
	set_terrain { "Gd^Fdf",
		f.terrain("Gs^Ft"),
	}
	set_terrain { "Gg^Fp",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Ss"))
		),
	}
	set_terrain { "Gs^Fmf",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Gg^Fp"))
		),
	}
	set_terrain { "Gs^Fmw",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Gs^Fmf"), nil, 0),
			f.radius(2, f.terrain("Gg^Fp,Ss,Ds"))
		),
	}
	set_terrain { "Gs^Fmf",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Gs^Fmw"))
		),
	}
	set_terrain { "Gs^Fmf",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Gs^Fmf"))
		),
	}
	set_terrain { "Gg^Fms",
		f.all(
			f.terrain("Gg^Fp"),
			f.adjacent(f.terrain("Gg^Fp,Gs^Fmf"), nil, "4-6")
		),
	}
	set_terrain { "Gs^Fetd",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Ds,Dd,Hd"))
		),
	}
	set_terrain { "Hh^Fdf",
		f.terrain("Hh^Ft"),
	}
	set_terrain { "Hh^Fmf",
		f.all(
			f.terrain("Hh^Fdf"),
			f.adjacent(f.terrain("Gg^Fp,Gs^Fmw"))
		),
	}
	set_terrain { "Gs^Fmf",
		f.terrain("Gs^Fp"),
	}
	set_terrain { "Hh^Fmf",
		f.terrain("Hh^Fp"),
	}
	set_terrain { "Gs^Ve",
		f.terrain("Gs^Vht"),
	}

	-- stone roads, better ones near castle
	local rad = mathx.random(4, 6)
	set_terrain { "Rrc",
		f.all(
			f.terrain("Re"),
			f.radius(rad, f.terrain("Ch"))
		),
	}
	set_terrain { "Rr",
		f.terrain("Re"),
	}

	-- change some villages, best ones near castle or road
	set_terrain { "Gs^Vc",
		f.all(
			f.terrain("Gg^Vh"),
			f.adjacent(f.terrain("Ss,Dd,Hd"))
		),
	}
	set_terrain { "Rr^Vhc",
		f.all(
			f.terrain("Gg^Vh"),
			f.radius(4, f.terrain("Ch,Rr,Rrc"))
		),
	}

	wct_road_to_village( "Rr", "Rr^Vhc")
	-- change best villages with no road
	set_terrain { "Gs^Vd",
		f.all(
			f.terrain("Rr^Vhc"),
			f.adjacent(f.terrain("Rr,Rrc,Ch,Kh*^*"), nil, 0)
		),
	}
	set_terrain { "Gg^Fet",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Ch,Kh*^*,Rr^Vhc"))
		),
	}
	set_terrain { "Gs^Fmf",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Gd^Fdf,Hh^Fdf"), nil, 6),
			f.radius(3, f.terrain("Gg^Fet"))
		),
	}
	set_terrain { "Gs^Vl",
		f.all(
			f.terrain("Gg^Vh"),
			f.adjacent(f.terrain("Gs,Gg"), nil, 6)
		),
	}
	set_terrain { "Hh^Vo",
		f.all(
			f.terrain("Hh^Vhh"),
			f.none(
				f.radius(5, f.terrain("Kh*^*,Ch,Rr,Rrc,Rr^Vhc"))
			)
		),
	}
	set_terrain { "Gs^Fetd",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Gs^Vd,Hh^Vo"))
		),
	}
	set_terrain { "Gs",
		f.all(
			f.terrain("Gg"),
			f.adjacent(f.terrain("Gs^Fetd,Dd,Hd"))
		),
	}

	-- stone bridges near castle
	set_terrain { "Ww^Bsb|",
		f.all(
			f.terrain("Ww^Bw|"),
			f.radius(6, f.terrain("Ch"))
		),
	}
	set_terrain { "Ww^Bsb/",
		f.all(
			f.terrain("Ww^Bw/"),
			f.radius(6, f.terrain("Ch"))
		),
	}
	set_terrain { "Ww^Bsb\\",
		f.all(
			f.terrain("Ww^Bw\\"),
			f.radius(6, f.terrain("Ch"))
		),
	}

	-- stone isolated castles
	wct_map_reduce_castle_expanding_recruit("Ce", "Rrc")
	set_terrain { "Ch",
		f.terrain("Ce"),
	}

	-- add snow, base amount in map surface
	local terrain_to_change = map:find(f.all(
		f.terrain("!,Ss,D*^*,Hd,W*^*,Mm^Xm,Xu,Mv,Q*^*,U*^*"),
		f.radius(3, f.all(
			f.terrain("Gd^Fdf,Hh,Hh^Fdf"),
			f.adjacent(f.terrain("Gd^Fdf,Hh,Hh^Fdf"), nil, 6)
		))
	))

	local r = mathx.random_choice(tostring(total_tiles // 930) .. ".." .. tostring(total_tiles // 210))
	wct_storm(terrain_to_change, r + 2)

	wct_expand_snow()
	wct_storm(terrain_to_change, r)
	-- snow can change adyacent forests
	set_terrain { "Hh^Fmw",
		f.all(
			f.terrain("Hh^Fmf"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms,Ha^*,Cha,Kha^*"), nil, "3-6")
		),
	}
	set_terrain { "Gs^Fmw",
		f.all(
			f.terrain("Gs^Fmf"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms,Ha^*,Cha,Kha^*"), nil, "3-6")
		),
	}
	set_terrain { "Hh^Fmw",
		f.all(
			f.terrain("Hh^Fdf"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms,Ha^*,Cha,Kha^*"), nil, "3-6")
		),
	}
	set_terrain { "Gs^Fmw",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms,Ha^*,Cha,Kha^*"), nil, "3-6")
		),
	}

	-- fallen forests lose leaves near orc villages
	set_terrain { "Hh^Fdw",
		f.all(
			f.terrain("Hh^Fdf"),
			f.adjacent(f.terrain("Hh^Fdf,Gd^Fdf"), "n,ne,se,s,sw,nw", "5-6"),
			f.radius(7, f.terrain("Hh^Vo*"))
		),
	}
	set_terrain { "Gd^Fdw",
		f.all(
			f.terrain("Gd^Fdf"),
			f.adjacent(f.terrain("Hh^Fdf,Gd^Fdf"), nil, "5-6"),
			f.radius(7, f.terrain("Hh^Vo*"))
		),
	}

	-- soft forest clusters
	set_terrain { "Hh^Fmw,Hh^Fdw,Hh^Fdf,Hh^Fmf",
		f.all(
			f.terrain("Hh^Fmw,Hh^Fdw,Hh^Fdf"),
			f.adjacent(f.terrain("Hh^Fmw,Hh^Fdw,Hh^Fdf"), nil, "4-6")
		),
		fraction = 6,
	}
	set_terrain { "Gs^Fmw,Gs^Fdw,Gs^Fdf,Gs^Fmf",
		f.all(
			f.terrain("G*^Fmw,G*^Fdw,G^Fdf"),
			f.adjacent(f.terrain("G*^Fmw,G*^Fdw,G*^Fdf"), nil, "4-6")
		),
		fraction = 6,
	}
	set_terrain { "Hh^Fmw,Hh^Fdw,Hh^Fdf",
		f.all(
			f.terrain("Hh^Fmf"),
			f.adjacent(f.terrain("Hh^Fmf,G*^Fmf"), nil, "5-6")
		),
		fraction = 6,
	}
	set_terrain { "Gs^Fmw,Gs^Fdw,Gs^Fdf",
		f.all(
			f.terrain("Gs^Fmf"),
			f.adjacent(f.terrain("Hh^Fmf,G*^Fmf"), nil, "5-6")
		),
		fraction = 6,
	}

	-- pines near lava
	set_terrain { "Hh^Fp",
		f.all(
			f.terrain("Hh^F*"),
			f.radius(2, f.terrain("Ql,Mv"))
		),
	}
	set_terrain { "Gg^Fp",
		f.all(
			f.terrain("G*^F*"),
			f.radius(2, f.terrain("Ql,Mv"))
		),
	}

	-- chances of few dwarven castles

	local terrain_to_change = wct_store_possible_dwarven_castle()
	while #terrain_to_change > 0 and mathx.random(2) == 1 do
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = "Cud"
		terrain_to_change = wct_store_possible_dwarven_castle()
	end
	-- decorative farmlands in base to log villages
	local terrain_to_change = map:find(f.terrain("Gs^Vl"))
	for i = 1, mathx.random(0, 2 * #terrain_to_change) do
		set_terrain { "Gg^Gvs",
			f.all(
				f.terrain("Gs,Gg"),
				f.adjacent(f.terrain("Gg^Gvs,Gs^Vl"))
			),
			-- change one location randomly.
			-- note that the change might add more available location to the filter above.
			-- so this loop cannot e replaced by [random_placement]
			nitems = 1,
		}
	end
	-- randomize a few forest
	set_terrain { "Gs^Fmw,Gs^Fdw,Gs^Fp,Gs^Fms",
		f.terrain("G*^Fmf"),
		fraction = 11,
	}
	set_terrain { "Gs^Fmw,Gs^Fp,Gs^Fmw,Gs^Fdf,Gs^Fdw",
		f.terrain("G*^Fms"),
		fraction = 6,
	}
	set_terrain { "Gs^Fmw,Gs^Fms,Gs^Fmw,Gs^Fms,Gs^Fdw",
		f.terrain("G*^Fp"),
		fraction = 6,
	}
	set_terrain { "Gs^Fmw,Gs^Fms,Gs^Fmw,Gs^Fdw,Gs^Fdf,Gs^Fp",
		f.terrain("G*^Fmf"),
		fraction = 12,
	}
	set_terrain { "Hh^Fmw,Hh^Fdw,Hh^Fp,Hh^Fms",
		f.terrain("Hh*^Fmf"),
		fraction = 11,
	}

	-- leaf litter
	set_terrain { "Gll",
		f.all(
			f.terrain("Gg,Gs"),
			f.adjacent(f.terrain("*^Fdf"), nil, "3-6")
		),
	}

	-- chance of fences near farmlands
	if mathx.random(2) == 1 then
		local terrain_to_change = map:find(f.all(
			f.terrain("Gs,Gg,Gll,Aa,Ai"),
			f.adjacent(f.terrain("Gg^Gvs")),
			f.adjacent(f.all(
				f.terrain("Gs,Gg,Gll,Aa,Ai"),
				f.adjacent(f.terrain("Gg^Gvs"))
			), "ne,se,sw,nw", "1-6")
		))
		for i, loc in ipairs(terrain_to_change) do
			map[loc] = "^Eff"
		end
	end

	if mathx.random(2) == 1 then
		set_terrain { "Gs^Eff",
			f.any(
				f.all(
					f.terrain("Gs,Gg,Gll,Aa,Ai,R*,D*"),
					f.adjacent(f.terrain("*^Eff"), "ne,se", 2)
				),
				f.all(
					f.terrain("Gs,Gg,Gll,Aa,Ai,R*,D*"),
					f.adjacent(f.terrain("*^Eff"), "nw,sw", 2)
				)
			),
			layer = "overlay",
		}

	end
	-- chances of stone walls and dark roads near darven castls
	local rad = mathx.random(1, 4)
	set_terrain { "Xos",
		f.all(
			f.terrain("Xu"),
			f.radius(rad, f.terrain("Cud"))
		),
	}

	wct_map_cave_path_to("Re")
	local r = mathx.random_choice("Ur,Urb")
	set_terrain { r,
		f.all(
			f.terrain("Re"),
			f.radius(6, f.terrain("Cud"))
		),
	}

	if mathx.random(3) == 0 then
		set_terrain { "Xuc",
			f.all(
				f.terrain("Xu"),
				f.radius(2, f.terrain("Cud,Xos,$rand"))
			),
		}
		set_terrain { "Xuc",
			f.all(
				f.terrain("Xu"),
				f.radius(999, f.terrain("Xuc"), f.terrain("Xu*"))
			),
		}

	end
	if mathx.random(20) == 1 then
		wct_map_decorative_docks()
	end
	if mathx.random(20) == 1 then
		wct_change_map_water("g")
	end
	wct_noise_snow_to("Wwf")

end

local function world_conquest_tek_map_repaint_6a()
	world_conquest_tek_map_rebuild("Uu,Uu^Tf,Uh,Uu^Tf,Uu,Uu^Tf,Uh,Ql,Qxu,Xu", 3)
	world_conquest_tek_map_decoration_6a()
	world_conquest_tek_map_dirt("Gg^Tf,Gs^Tf")
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Rural")
	world_conquest_tek_map_noise_classic("Gs^Fp")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_6a()
end
