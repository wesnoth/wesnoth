-- Volcanic

function world_conquest_tek_map_repaint_4b()
	set_terrain { "Ql,Md,Md^Xm",
		f.terrain("U*,U*^Tf"),
		exact = false,
		percentage = 10,
	}
	set_terrain { "Ql,Uu,Uh,Uh,Uu^Tf,Qxu,Uh^Tf",
		f.terrain("Xu"),
	}
	set_terrain { "Ql,Uu^Tf,Qxu,Uh^Tf,Uh,Uh,Uu,Ql,Md",
		f.terrain("Mm^Xm"),
		fraction = 2,
	}
	set_terrain { "Ql,Uu^Tf,Qxu,Uh^Tf",
		f.all(
			f.terrain("Hh^F*"),
			f.adjacent(f.terrain("Ql,Uu,Uh,Uu^Tf,Qxu,Uh^Tf"))
		),
		fraction = 3,
	}
	set_terrain { "Uh",
		f.all(
			f.terrain("Hh"),
			f.adjacent(f.terrain("Ql,Uu,Uh,Uu^Tf,Qxu,Uh^Tf,Ur"))
		),
		fraction = 3,
	}

	wct_fill_lava_chasms()
	set_terrain { "Uh",
		f.all(
			f.adjacent(f.terrain("Ql")),
			f.terrain("Hh*^*")
		),
		layer = "base",
	}
	set_terrain { "Ur",
		f.all(
			f.terrain("Re"),
			f.none(
				f.radius(999, f.terrain("Ch*^*,Kh*^*"), f.terrain("Re"))
			)
		),
	}

	for obsidian_i = 1, 3 do
		set_terrain { "Ur",
			f.all(
				f.adjacent(f.terrain("Ql,Uu^*,Qxu,Uh^*,Ur^*")),
				f.terrain("G*^*")
			),
			layer = "base",
		}
	end

	for scorched_i = 1, 2 do
		set_terrain { "Rd",
			f.all(
				f.radius(2, f.terrain("Ql,Uu^*,Qxu,Uh^*,Ur^*,Rd")),
				f.terrain("G*^*")
			),
			layer = "base",
		}
	end
	-- change villages
	set_terrain { "Ur^Vu",
		f.terrain("Ur^V*"),
	}
	set_terrain { "*^Vd",
		f.terrain("*^Vhh"),
		layer = "overlay",
	}
	set_terrain { "*^Vo",
		f.terrain("*^Ve"),
		layer = "overlay",
	}
	set_terrain { "Gd^Vc",
		f.terrain("G*^Vh"),
	}
	set_terrain { "*^Vh",
		f.terrain("*^Vht"),
		layer = "overlay",
	}
	set_terrain { "Rd^Vhh",
		f.terrain("Rd^Vh"),
	}
	set_terrain { "Ds^Vct",
		f.terrain("Dd^Vda"),
		fraction = 3,
	}

	if mathx.random(2) == 1 then
		set_terrain { "Ur^Vd",
			f.terrain("Ur^Vu"),
			fraction_rand = "2..3",
		}

	end
	if mathx.random(2) == 1 then
		set_terrain { "Rd^Vd",
			f.terrain("Rd^Vhh"),
			fraction_rand = "2..3",
		}

	end
	if mathx.random(2) == 1 then
		set_terrain { "Ds^Vd",
			f.terrain("D*^V*"),
			fraction_rand = "3..4",
		}

	end
	-- dry terrain
	set_terrain { "Sm",
		f.terrain("Ss^*"),
		layer = "base",
	}
	set_terrain { "Gd",
		f.terrain("G*^*"),
		layer = "base",
	}
	set_terrain { "Md",
		f.terrain("M*^*"),
		layer = "base",
	}
	set_terrain { "Hhd",
		f.terrain("Hh^*"),
		layer = "base",
	}
	set_terrain { "*^Fdw",
		f.all(
			f.terrain("U*^F*"),
			f.none(
				f.terrain("*^Fet")
			)
		),
		layer = "overlay",
	}
	set_terrain { "*^Fmw",
		f.all(
			f.terrain("Rd^F*"),
			f.none(
				f.terrain("Rd^Fet")
			)
		),
		layer = "overlay",
	}
	set_terrain { "*^Fetd",
		f.terrain("*^Fet"),
		layer = "overlay",
	}

	-- change some forest
	set_terrain { "Gd^Fdw,Gd^Fmw",
		f.terrain("G*^Ft"),
		fraction = 3,
	}
	set_terrain { "Gd^Fms,Gd^Fmw,Gd^Fet",
		f.terrain("G*^Ft"),
		fraction = 2,
	}
	set_terrain { "Hhd^Fdw,Hhd^Fmw",
		f.terrain("Hhd^Ft"),
		fraction = 3,
	}
	set_terrain { "Hhd^Fms,Hhd^Fmw",
		f.terrain("Hhd^Ft"),
		fraction = 2,
	}

	-- difumine dry/grass border
	set_terrain { "Rd",
		f.all(
			f.terrain("Gd"),
			f.adjacent(f.terrain("Rd"))
		),
		fraction = 3,
	}
	set_terrain { "Gd",
		f.all(
			f.terrain("Rd"),
			f.adjacent(f.terrain("Gd"))
		),
		fraction = 3,
	}

	-- create volcanos where possible and force one
	local terrain_to_change = map:find(f.all(
		f.terrain("Ql"),
		f.adjacent(f.terrain("M*,M*^Xm"), "se,s,sw", 2),
		f.adjacent(f.terrain("K*^*,C*^*,*^V"), "se,s,sw", 0)
	))
	if #terrain_to_change > 0 then
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		set_terrain { "Md^Xm",
			f.all(
				f.none(f.terrain("M*^*")),
				f.adjacent(f.is_loc(loc), "ne,n,nw")
			)
		}
	end
	set_terrain { "Mv",
		f.all(
			f.terrain("Ql"),
			f.adjacent(f.terrain("Md^Xm,Md"), "se,s,sw", 3)
		),
	}
	local terrain_to_change = map:find(f.terrain("Mv"))

	for i, v in ipairs(terrain_to_change) do
		local loc = terrain_to_change[i]
		table.insert(prestart_event, wml.tag.sound_source {
			id = "volcano" .. tostring(i),
			sounds = "rumble.ogg",
			delay = 450000,
			chance = 1,
			x = loc[1],
			y = loc[2],
			full_range = 5,
			fade_range = 5,
			loop = 0,
		})
	end
	-- some volcanic coast and reefs
	set_terrain { "Uue",
		f.all(
			f.terrain("Ww,Wo"),
			f.adjacent(f.terrain("S*^*"))
		),
		fraction = 2,
	}
	set_terrain { "Uue,Wwr",
		f.all(
			f.terrain("Ww,Wo"),
			f.adjacent(f.terrain("Uu*,Ur,Uh,D*^*"))
		),
		fraction_rand = "9..11",
	}
	set_terrain { "Uue",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("U*,S*^*,W*^*,Gd,Rd"))
		),
		fraction_rand = "9..11",
	}
	set_terrain { "Uue",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("U*"))
		),
		fraction_rand = "9..11",
	}

	-- rubble
	set_terrain { "Rd^Dr,Hhd^Dr",
		f.all(
			f.terrain("Hhd"),
			f.adjacent(f.terrain("Rd*^*")),
			f.radius(4, f.all(
			))
		),
		fraction_rand = "16..20",
	}
	set_terrain { "Ur^Dr,Hhd^Dr",
		f.all(
			f.terrain("Hhd"),
			f.adjacent(f.terrain("U*^*"))
		),
		fraction_rand = "10..15",
	}
	set_terrain { "Uu^Dr,Uh^Dr",
		f.terrain("Uh"),
		fraction_rand = "9..13",
	}

	-- mushrooms, base amount in map surface
	local terrain_to_change = map:find(f.terrain("Hhd,Hhd^F^*"))
	mathx.shuffle(terrain_to_change)
	local r = mathx.random_choice(tostring(total_tiles // 600) .. ".." .. tostring(total_tiles // 300))

	for mush_i = 1, math.min(r, #terrain_to_change) do
		map[terrain_to_change[mush_i]] = "Hhd^Tf"
	end
	-- chances of few orcish castles
	wct_possible_map4_castle("Co", 2)
	-- craters
	set_terrain { "Dd^Dc",
		f.terrain("Dd"),
		fraction_rand = "4..6",
	}

	-- grass near muddy or earthy cave become dark dirt
	set_terrain { "Rb",
		f.all(
			f.adjacent(f.terrain("S*^*,Uue"), nil, "2-6"),
			f.terrain("G*^*")
		),
	}

	-- some small mushrooms on dark dirt
	set_terrain { "Rb^Em",
		f.terrain("Rb"),
		fraction_rand = "9..11",
	}

	-- some extra reefs
	set_terrain { "Wwr",
		f.all(
			f.terrain("Ww,Wo"),
			f.adjacent(f.terrain("Ds,Ww,Uu"))
		),
		fraction_rand = "18..22",
	}

	-- whirpools
	local whirlpool_candidats  = map:find(f.all(
		f.terrain("Ww"),
		f.adjacent(f.terrain("Wo")),
		f.adjacent(f.terrain("Uue"))
	))

	mathx.shuffle(whirlpool_candidats)
	for i = 1, #whirlpool_candidats // mathx.random(4, 15) do

		local loc = whirlpool_candidats[i]
		table.insert(prestart_event, wml.tag.item {
			image = "scenery/whirlpool.png",
			x = loc[1],
			y = loc[2],
		})
		table.insert(prestart_event, wml.tag.sound_source {
			id = "whirlppol" .. tostring(i),
			sounds = "mermen-hit.wav",
			delay = 90000,
			chance = 1,
			x = loc[1],
			y = loc[2],
			full_range = 5,
			fade_range = 5,
			loop = 0,
		})
	end

	-- dessert sand no near water or village
	set_terrain { "Dd",
		f.all(
			f.adjacent(f.terrain("W*^*,*^V*"), nil, 0),
			f.terrain("Ds")
		),
	}

	-- very dirt coast
	local terrain_to_change = map:find(f.terrain("Ds"))

	mathx.shuffle(terrain_to_change)
	for i = 1, #terrain_to_change // mathx.random(3, 4) do
		map[terrain_to_change[i]] = "Ds^Esd"
	end
	mathx.shuffle(terrain_to_change)
	for i = 1, #terrain_to_change // 6 do
		map[terrain_to_change[i]] = "Ds^Es"
	end
	set_terrain { "Dd^Es",
		f.terrain("Dd"),
		fraction_rand = "4..6",
	}
	set_terrain { "Uue^Es",
		f.terrain("Uue"),
		fraction_rand = "2..3",
	}
	set_terrain { "Sm^Es",
		f.terrain("Sm"),
		fraction_rand = "4..6",
	}

	-- 1.12 new forest
	set_terrain { "*^Ft",
		f.terrain("*^Fp"),
		fraction_rand = "6..10",
		layer = "overlay",
	}
	set_terrain { "*^Ftd",
		f.terrain("H*^Ft"),
		layer = "overlay",
	}
	set_terrain { "*^Fts",
		f.terrain("*^Ft"),
		layer = "overlay",
	}
	set_terrain { "*^Fts",
		f.terrain("*^Fmw"),
		fraction_rand = "4..7",
		layer = "overlay",
	}
	set_terrain { "*^Ft",
		f.terrain("*^Fms"),
		fraction_rand = "2..4",
		layer = "overlay",
	}


	local r = mathx.random(20)
	if r == 1 then
		wct_change_map_water("g")
	elseif r == 2 then
		wct_change_map_water("t")
	end

end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Volcanic")
	world_conquest_tek_map_noise_classic("Gs^Fp")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_4b()
	world_conquest_tek_bonus_points("volcanic")
	wct_map_enemy_themed("dwarf", "Giant Mudcrawler", "ud", "Ur^Vud", 5)
end
