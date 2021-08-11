-- Lakes
local function world_conquest_tek_map_repaint_2b()
	-- Add snow and ice
	if mathx.random(2) == 1 then
		local terrain_to_change = map:find(f.all(
			f.terrain("!,Ss,D*^*,Hd,W*^*,Mm^Xm,Xu,Mv,Q*^*,U*^*"),
			f.radius(2, f.terrain("M*^*"))
		))

		-- base amount in map surface
		local r = mathx.random_choice(tostring(total_tiles // 675) .. ".." .. tostring(total_tiles // 330))
		wct_storm(terrain_to_change, r)
	end
	wct_expand_snow()
	set_terrain { "Ai",
		f.all(
			f.terrain("Wwt,Wot"),
			f.adjacent(f.terrain("A*^*,Ms^*,Ha^*,Kha,Cha"), nil, mathx.random(3, 5))
		),
	}
	-- randomize snowed forests
	set_terrain { "Aa^Fpa,Aa^Fda,Aa^Fma",
		f.terrain("*^Fpa"),
		layer = "overlay",
	}
	set_terrain { "Gg^Fms,Gg^Fds",
		f.terrain("G*^Ft"),
	}
	set_terrain { "Hh^Ft,Hh^Fms,Hh^Fds",
		f.terrain("H*^Ft"),
	}
	set_terrain { "Gg^Fms",
		f.terrain("G*^Fp"),
		fraction = 2,
	}
	set_terrain { "Hh^Fms",
		f.terrain("H*^Fp"),
		fraction = 6,
	}
	set_terrain { "Rb",
		f.all(
			f.terrain("R*"),
			f.none(
				f.radius(4, f.terrain("C*^*,K*^*"))
			)
		),
		fraction_rand = "3..5",
	}
	set_terrain { "Gg^Vc",
		f.terrain("*^Ve"),
		fraction = 2,
	}
	set_terrain { "Gg^Vhs",
		f.terrain("G*^Vht"),
	}
	set_terrain { "Wwf^Vht",
		f.terrain("Hh^Vhh"),
		fraction = 2,
	}

	if mathx.random(20) ~= 1 then
		set_terrain { "Gg",
			f.terrain("Gs^*"),
			layer = "base",
		}

	end
	if mathx.random(20) ~= 1 then
		set_terrain { "Gg^Gvs",
			f.all(
				f.terrain("Gg"),
				f.adjacent(f.terrain("S*^*,Dd,Hd,A*^*,Ha*^*,Ms*^*"), nil, 0),
				f.radius(2, f.terrain("*^Vh,*^Ve,*^Vc")),
				f.radius(2, f.terrain("W*^*"))
			),
			fraction_rand = "5..17",
		}

	else
		set_terrain { "Gg^Efm",
			f.all(
				f.terrain("Gg"),
				f.adjacent(f.terrain("D*^*"), nil, 0),
				f.radius(3, f.terrain("*^Fet")),
				f.radius(3, f.terrain("Wwf"))
			),
			fraction_rand = "3..5",
		}

	end
	if mathx.random(20) == 1 then
		wct_map_decorative_docks()
	end
	wct_dirt_beachs("9..11")
	-- chance of different lakes water
	-- todo: does this syntax really work?
	local terrain_mod = mathx.random_choice("g,,,,,,,,,,,,,,,,,,t")
	set_terrain { "Ww" .. terrain_mod,
		f.terrain("Wwt^*"),
		layer = "base",
	}
	set_terrain { "Wo" .. terrain_mod,
		f.terrain("Wot"),
	}

	-- chance of frozen lakes
	if mathx.random(9) == 1 then
		set_terrain { "Ai",
			f.all(
				f.terrain("Ww,Wwg,Wo,Wog"),
				f.adjacent(f.terrain("*^Tf"), nil, 0),
				f.none(
					f.find_in("oceanic")
				)
			),
			exact = false,
			percentage = 9,
			filter_extra = { oceanic = get_oceanic() },
		}
		set_terrain { "Ms",
			f.all(
				f.terrain("M*"),
				f.adjacent(f.terrain("*^Tf"), nil, 0),
				f.radius(2, f.terrain("Ai"))
			),
		}
		set_terrain { "Ha",
			f.all(
				f.terrain("Hh"),
				f.adjacent(f.terrain("*^Tf"), nil, 0),
				f.radius(1, f.terrain("Ai"))
			),
		}
		set_terrain { "Ha^Fpa",
			f.all(
				f.terrain("Hh^F*"),
				f.adjacent(f.terrain("*^Tf"), nil, 0),
				f.radius(1, f.terrain("Ai"))
			),
		}

	end
	-- chance of diferent forest based in map temperature
	local terrain_to_change = map:find(f.terrain("A*^*,Ha*^*,Ms^*"))

	local chance = 2000 * #terrain_to_change // total_tiles
	if mathx.random(0, 99 ) > chance then
		set_terrain { "*^Ftd",
			f.terrain("*^Ft"),
			layer = "overlay",
		}
	end
	set_terrain { "*^Fmf",
		f.terrain("*^Ft"),
		layer = "overlay",
	}


end

function world_conquest_tek_map_constructor_lakes()
	wct_map_reduce_castle_expanding_recruit("Ce", "Re")
	-- convert caves to water
	set_terrain { "Wwt^Vm",
		f.terrain("U*^V*"),
	}
	set_terrain { "Wwt,Wwt,Wwt,Wot",
		f.terrain("U*^*,Q*^*"),
	}
	set_terrain { "Wwt,Wwt,Wwt,Wwt,Wwt,Wwt,Wot",
		f.terrain("Xu"),
	}
	set_terrain { "Wwt,Wwt,Wwt,Wwt,Wwt,Wwt,Wwt,Wwt,Wwt,Wot",
		f.terrain("Mm^Xm"),
	}

	-- convert road to bridge if possible
	set_terrain { "Wwt^Bw|",
		f.all(
			f.terrain("R*"),
			f.adjacent(f.terrain("R*"), "n,s", 2),
			f.adjacent(f.terrain("W*^*"), "ne,se", "1-2"),
			f.adjacent(f.terrain("W*^*"), "nw,sw", "1-2")
		),
	}
	set_terrain { "Wwt^Bw/",
		f.all(
			f.terrain("R*"),
			f.adjacent(f.terrain("R*"), "ne,sw", 2),
			f.adjacent(f.terrain("W*^*"), "n,nw", "1-2"),
			f.adjacent(f.terrain("W*^*"), "s,se", "1-2")
		),
	}
	set_terrain { "Wwt^Bw\\",
		f.all(
			f.terrain("R*"),
			f.adjacent(f.terrain("R*"), "nw,se", 2),
			f.adjacent(f.terrain("W*^*"), "n,ne", "1-2"),
			f.adjacent(f.terrain("W*^*"), "s,sw", "1-2")
		),
	}

	-- expand lakes with fords
	set_terrain { "Wwf",
		f.all(
			f.adjacent(f.terrain("Wwt^*,Wot,Wwf")),
			f.none(
				f.terrain("W*^*,R*^*,*^V*,K*^*,C*^*,S*,D*^*")
			)
		),
		fraction = 9,
	}
	set_terrain { "Wwf",
		f.all(
			f.terrain("G*"),
			f.adjacent(f.terrain("Wwt^*,Wot,Wwf")),
			f.adjacent(f.terrain("M*^*"))
		),
		fraction = 2,
	}

	-- add mushrooms
	set_terrain { "Gg^Tf",
		f.all(
			f.adjacent(f.terrain("Wwt^*,Wot,Wwf")),
			f.adjacent(f.terrain("A*^*,Ha^*,Ms^*"), nil, 0),
			f.none(
				f.terrain("W*^*,R*^*,*^V*,K*^*,C*^*,S*,D*^*,G*^*")
			)
		),
		fraction_rand = "11..13",
	}

	local r = mathx.random_choice(tostring(total_tiles // 675) .. ".." .. tostring(total_tiles // 285))

	set_terrain { "Hh^Tf",
		f.all(
			f.terrain("Hh^F*,Hh"),
			f.adjacent(f.terrain("A*^*,Ha^*,Ms^*"), nil, 0)
		),
		nlocs = r,
	}
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Lakes")
	world_conquest_tek_map_constructor_lakes()
	world_conquest_tek_map_noise_classic("Gs^Fp")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_2b()
	world_conquest_tek_bonus_points()
	wct_noise_snow_to("Gd,Wwf,Rb")
end
