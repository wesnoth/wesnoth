-- Feudal

-- unused function, replaces by roads_to_feudal_castle below,
-- the odl code used
-- `wct_iterate_roads_to(wct_roads_to_feudal_castle, 3, "Rr")`
-- instead of `roads_to_feudal_castle(3)`
local function wct_roads_to_feudal_castle(radius)
	return map:find(f.all(
		f.terrain("!,W*,*^V*,Ds,C*,K*,R*"),
		f.adjacent(f.all(
			f.terrain("Chs,Rr"),
			f.adjacent(f.all(
				f.terrain("Rr,Chs"),
				f.radius(radius, f.terrain("Re,Khs"), f.terrain("!,W*,*^V*,Ds,Ch,Kh^*"))
			), nil, 0),
			f.none(
				f.radius(radius, f.terrain("Re,Khs"), f.terrain("!,W*,*^V*,Ds,Ch,Kh^*"))
			),
			--not if it is already connected to another Keep?
			f.none(
				f.radius(999, f.terrain("K*"), f.terrain("R*,*^B*,K*,C*"))
			)
		)),
		f.radius(radius, f.terrain("Re,Khs"), f.terrain("!,W*,*^V*,Ds,Ch,Kh^*"))
	))
end

local function roads_to_feudal_castle(radius)
	wct_iterate_roads_to_ex {
		terrain_road = "Rr",
		f_validpath = f.terrain("!,W*,*^V*,Ds,C*,K*,R*"),
		f_src = f.terrain("Chs"),
		f_dest = f.terrain("Khs"),
		radius = radius
	}
end

local function world_conquest_tek_map_repaint_6d()
	-- fix generator mark "anti-castle-generation"
	set_terrain { "Gg",
		f.terrain("Wot"),
	}

	-- soft rough terrain around caves
	set_terrain { "Aa,Aa,Aa,Gs",
		f.terrain("Ha"),
		fraction = 3,
	}
	set_terrain { "Gs",
		f.terrain("Hh"),
		fraction = 2,
	}
	set_terrain { "Gg",
		f.terrain("Hhd"),
		fraction = 2,
	}
	set_terrain { "Aa,Aa,Aa,Aa,Gs",
		f.terrain("Ms"),
		fraction = 3,
	}
	set_terrain { "Gs",
		f.terrain("Mm"),
		fraction = 2,
	}

	-- rough terrain on snow
	set_terrain { "Ha,Ha,Ha,Aa^Fpa,Aa^Fpa,Ha^Fpa,Ms,Gs,Ha,Ha,Ha,Aa^Fpa,Aa^Fma,Ha^Fma,Ms,Wwf",
		f.terrain("Aa"),
		fraction = 6,
	}
	set_terrain { "Coa,Coa,Coa,Coa,Aa,Re",
		f.terrain("Coa"),
		fraction = 3,
	}

	-- rough terrain on dessert
	set_terrain { "Hd,Hd,Hd,Hd,Hd,Hd,Md",
		f.terrain("Dd"),
		fraction = 7,
	}
	set_terrain { "Dd,Dd,Dd^Ftp,Dd^Do,Dd,Dd,Dd^Ftp,Dd^Do,Sm",
		f.all(
			f.terrain("Dd"),
			f.adjacent(f.terrain("Dd^*,Hd^*,Md"), nil, "0-5")
		),
		fraction = 6,
	}
	set_terrain { "Cd,Cd,Cd,Cd,Re",
		f.terrain("Cd"),
		fraction = 3,
	}

	-- castles around roads
	set_terrain { "Chs",
		f.all(
			f.terrain("!,W*,Ds,Ss,C*,K*,*^V*"),
			f.adjacent(f.terrain("Re,C*,K*"), nil, 0),
			f.radius(4, f.terrain("Re"), f.terrain("!,W*,*^V*,Ds,Ch,Kh^*"))
		),
		fraction = 40,
	}
	roads_to_feudal_castle(3)
	set_terrain { "Khs",
		f.terrain("Chs"),
	}
	set_terrain { "Chs",
		f.all(
			f.terrain("!,W*,Ds,Ss,C*,K*,*^V*"),
			f.adjacent(f.terrain("C*,K*"), nil, 0),
			f.none(
				f.radius(4, f.terrain("Re"))
			),
			f.radius(5, f.terrain("Khs"))
		),
		fraction = 40,
	}
	roads_to_feudal_castle(4)
	set_terrain { "Khs",
		f.terrain("Chs"),
	}
	if false then
		-- this one was slow.
		set_terrain { "Chs",
			f.all(
				f.terrain("!,W*,Ds,Ss,C*,K*,*^V*"),
				f.adjacent(f.terrain("C*,K*"), nil, 0),
				f.none(
					f.radius(8, f.terrain("Re"))
				),
				f.radius(6, f.terrain("Khs"))
			),
			fraction = 40,
		}
	else
		-- this is faster.
		local r8_Re = map:find_in_radius(
			map:find(f.terrain("Re")),
			8,
			wesnoth.map.filter(f.all())
		)
		local r6_Khs = map:find_in_radius(
			map:find(f.terrain("Khs")),
			6,
			wesnoth.map.filter(f.all())
		)
		set_terrain { "Chs",
			f.all(
				f.terrain("!,W*,Ds,Ss,C*,K*,*^V*"),
				f.adjacent(f.terrain("C*,K*"), nil, 0),
				f.none(
					f.find_in("r8_Re")
				),
				f.find_in("r6_Khs")
			),
			filter_extra = {
				r6_Khs = r6_Khs,
				r8_Re = r8_Re,
			},
			fraction = 40,
		}
	end
	roads_to_feudal_castle(5)
	-- rebuild cave
	wct_reduce_wall_clusters("Uu")
	set_terrain { "Uh,Uh,Uu^Tf,Uh,Uh,Uu^Tf,Uh,Uh,Uu^Tf,Uh,Uh,Uu^Tf,Uh,Uh,Uu^Tf,Uh,Uu^Tf,Uu,Qxu,Qxu,Ql",
		f.terrain("Uu"),
		fraction = 4,
	}
	set_terrain { "Qxu,Uh^Tf,Ql,Urb,Urb,Urb,Urb,Urb,Urb,Urb,Urb,Urb,Urb,Urb,Urb,Urb,Urb,Uh,Uh,Uu^Tf",
		f.terrain("Uu"),
		fraction = 5,
	}

	wct_fill_lava_chasms()
	wct_volcanos()
	wct_volcanos_dirt()
	wct_break_walls("M*^Xm", "Mm,Mm,Hh,Hh,Hh^Fp,Hh^Fp,Hh^Tf,Hh^Tf,Gs^Fp,Rb,Rb,Rb")
	wct_break_walls("X*", "Uh,Uh,Uh,Uh,Uh^Tf,Uu^Tf,Uu,Rd,Rd,Rd")
	set_terrain { "Mm^Xm",
		f.all(
			f.terrain("X*"),
			f.adjacent(f.terrain("U*^*,Q*"), nil, 0)
		),
	}
	set_terrain { "Xu,Xu,Xu,Xu,Xuc",
		f.all(
			f.terrain("Mm^Xm"),
			f.adjacent(f.terrain("U*^*,Q*"))
		),
		fraction = 2,
	}
	set_terrain { "Mm^Xm",
		f.terrain("Rb"),
	}
	set_terrain { "Xu",
		f.terrain("Rd"),
	}
	set_terrain { "Mm^Xm,Xu",
		f.all(
			f.terrain("Mm^Xm"),
			f.adjacent(f.terrain("U*^*"))
		),
	}
	set_terrain { "Xuc",
		f.all(
			f.terrain("Xu"),
			f.radius(999, f.terrain("Xuc"), f.terrain("X*"))
		),
	}

	-- forest on some hills
	set_terrain { "Hh^Fp",
		f.terrain("Hh"),
		fraction = 5,
	}
	set_terrain { "Hh^Fp,Hh^Tf",
		f.terrain("Hh"),
		fraction = 12,
	}
	set_terrain { "Hh^Fp",
		f.terrain("Hhd"),
		fraction = 5,
	}
	set_terrain { "Hhd^Fp,Hhd^Fp,Hh^Tf,Gg^Tf",
		f.terrain("Hhd"),
		fraction = 10,
	}

	-- extra rough terrain
	set_terrain { "Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Gg^Fp,Hh,Hh,Hh,Hh,Hh,Hh,Hh,Hh,Hhd^Fp,Hhd^Fp,Mm,Mm,Mm,Gg^Tf,Hh^Tf,Ss",
		f.terrain("Gg"),
		fraction = 3,
	}
	set_terrain { "Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Gs^Fp,Hh,Hh,Hh,Hh,Hh,Hh,Hh,Hh,Hh^Fp,Hh^Fp,Mm,Mm,Mm,Gs^Tf,Hh^Tf,Ss",
		f.terrain("Gs"),
		fraction = 4,
	}

	-- expand snow from lower altitude
	set_terrain { "Ha",
		f.all(
			f.terrain("Hh,Hhd"),
			f.adjacent(f.terrain("A*^*"), nil, "2-6")
		),
		layer = "base",
	}
	set_terrain { "Ms",
		f.all(
			f.terrain("Mm^*"),
			f.adjacent(f.terrain("A*^*,Ha^*"))
		),
		layer = "base",
	}

	-- feudal villages
	set_terrain { "Ch^Vh",
		f.all(
			f.terrain("*^V*"),
			f.adjacent(f.terrain("Khs,Chs,R*")),
			f.adjacent(f.terrain("Kh^*,Ch"), nil, 0)
		),
	}

	-- theme castles
	set_terrain { "Koa",
		f.all(
			f.terrain("Chs"),
			f.adjacent(f.terrain("A*^*,Ha^*,Ms^*"))
		),
	}
	set_terrain { "Coa",
		f.all(
			f.terrain("Khs"),
			f.adjacent(f.terrain("A*^*,Ha^*,Ms^*"))
		),
	}
	set_terrain { "Kv",
		f.all(
			f.terrain("Chs"),
			f.adjacent(f.terrain("*^F*"), nil, "3-6")
		),
	}
	set_terrain { "Cv",
		f.all(
			f.terrain("Khs"),
			f.adjacent(f.terrain("*^F*"), nil, "3-6")
		),
	}
	set_terrain { "Kd",
		f.all(
			f.terrain("Chs"),
			f.adjacent(f.terrain("Dd^*,Hd^*,Md"))
		),
	}
	set_terrain { "Cd",
		f.all(
			f.terrain("Khs"),
			f.adjacent(f.terrain("Dd^*,Hd^*,Md"))
		),
	}
	set_terrain { "Kud",
		f.all(
			f.terrain("Chs"),
			f.adjacent(f.terrain("U*^*,Q*"))
		),
	}
	set_terrain { "Cud",
		f.all(
			f.terrain("Khs"),
			f.adjacent(f.terrain("U*^*,Q*"))
		),
	}
	set_terrain { "Ce",
		f.all(
			f.terrain("Khs"),
			f.adjacent(f.terrain("Ss^*"))
		),
	}
	set_terrain { "Kh",
		f.terrain("Chs"),
	}
	set_terrain { "Ch",
		f.terrain("Khs"),
	}
	wct_map_reduce_castle_expanding_recruit("Ce,Cd,Coa", "Rrc")
	set_terrain { "Wwf",
		f.all(
			f.terrain("Rrc"),
			f.adjacent(f.terrain("W*^*"), nil, "2-6")
		),
	}
	set_terrain { "Re",
		f.terrain("Rrc"),
	}
	set_terrain { "Coa^Vo",
		f.all(
			f.terrain("Ch^Vh"),
			f.adjacent(f.terrain("Coa,Koa"))
		),
	}
	set_terrain { "Cud^Vud",
		f.all(
			f.terrain("Ch^Vh"),
			f.adjacent(f.terrain("Cud,Kud,U*^*,Q*"))
		),
	}
	set_terrain { "Cv^Ve",
		f.all(
			f.terrain("Ch^Vh"),
			f.adjacent(f.terrain("Cv,Kv"))
		),
	}
	set_terrain { "Cd^Vd",
		f.all(
			f.terrain("Ch^Vh"),
			f.adjacent(f.terrain("Cd,Kd"))
		),
	}
	set_terrain { "Ce^Vl",
		f.all(
			f.terrain("Ch^Vh"),
			f.adjacent(f.terrain("Ce"))
		),
	}
	set_terrain { "Ch^Vhc",
		f.all(
			f.terrain("Ch^Vh"),
			f.radius(2, f.terrain("Ch,Kh^*"))
		),
	}

	-- elvish villages next to 3 forest
	set_terrain { "G*^Ve",
		f.all(
			f.terrain("G*^Vc"),
			f.adjacent(f.terrain("*^F*"), nil, "3-6")
		),
		layer = "overlay",
	}

	-- fix roads
	set_terrain { "Re",
		f.terrain("Rr"),
	}
	set_terrain { "Urb",
		f.all(
			f.terrain("Re"),
			f.adjacent(f.terrain("U*^*,Cud^*,Kud,X*,R*,Q*,Mv,*^Xm"), nil, 6)
		),
	}

	-- fix forest types
	set_terrain { "*^Fet",
		f.all(
			f.terrain("*^F*"),
			f.radius(2, f.terrain("Cv^*,Kv"))
		),
		layer = "overlay",
	}
	set_terrain { "Gg^Fms",
		f.terrain("Gg^Fp"),
		fraction = 2,
	}
	set_terrain { "Gg^Ftr",
		f.all(
			f.terrain("Gg^Fp"),
			f.adjacent(f.terrain("*^F*,Ss^*"), nil, "2-6")
		),
	}
	set_terrain { "Gs^Fmw",
		f.terrain("Gs^Fp"),
		fraction = 3,
	}
	set_terrain { "Hh^Fmw",
		f.terrain("Hh^Fp"),
		fraction = 2,
	}
	set_terrain { "Hh^Fms",
		f.terrain("Hhd^Fp"),
		fraction = 5,
	}
	set_terrain { "Gg^Fds",
		f.all(
			f.terrain("Gg^Ftr"),
			f.adjacent(f.terrain("*^Fmw"))
		),
	}

	-- remove temperature mark on hills
	set_terrain { "Hh",
		f.terrain("Hhd^*"),
		layer = "base",
	}

	-- fix rivers
	set_terrain { "Wwf,Wwf,Wwf,Wwf,Wwf,Wwf,Wwf,Wwf,Ww,Ww,Ss",
		f.all(
			f.terrain("Ww"),
			f.adjacent(f.terrain("*^B*"), nil, 0),
			f.none(
				f.radius(4, f.terrain("Wwg^*,Wog"), f.terrain("W*^*,C*^*,K*^*"))
			)
		),
		fraction = 2,
	}

	-- reefs
	set_terrain { "Wwrg",
		f.all(
			f.terrain("Wwg,Wog"),
			f.radius(4, f.terrain("G*^*,H*^*,M*^*"))
		),
		fraction_rand = "24..240",
	}

	if mathx.random(20) == 1 then
		wct_map_decorative_docks()
	end
	-- beachs sand and stones
	set_terrain { "Ds^Esd",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Wwg,Wog"))
		),
		fraction_rand = "7..12",
	}

end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Feudal")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_6d()
	wct_map_enemy_themed("undead", "Soulless", "ha", "Aa^Vha", 1)
	wct_map_enemy_themed("elf", "Wolf", "v", "Gg^Ve", 1)
	wct_map_enemy_themed("dwarf", "Giant Mudcrawler", "ud", "Ur^Vud", 1)
	wct_map_enemy_themed("orc", "Giant Scorpion", "o", "Gs^Vo", 1)
	wct_map_enemy_themed("troll", "Giant Scorpion", "o", "Gs^Vo", 1)
	wct_map_enemy_themed("wolf", "Giant Scorpion", "o", "Gs^Vo", 1)
	wct_map_enemy_themed("human", "Young Ogre", "e", "Gg^Vl", 1)
	wct_map_enemy_themed("drake", "Fire Guardian", "d", "Gg^Vd", 1)
	wct_map_enemy_themed("lizard", "Fire Guardian", "d", "Gg^Vd", 1)
end
