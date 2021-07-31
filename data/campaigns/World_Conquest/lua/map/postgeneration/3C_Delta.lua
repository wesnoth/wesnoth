-- Delta

function world_conquest_tek_map_repaint_3c()
	wct_noise_delta()
	world_conquest_tek_map_decoration_3c()
end

function world_conquest_tek_map_constructor_delta()
	-- oasis is a mark for extra land acumulation
	set_terrain { "Ds,Ww,Gs,Ww",
		f.all(
			f.terrain("W*"),
			f.adjacent(f.terrain("Dd^Do,Ds^Vdt"))
		),
	}
	set_terrain { "Gs,Ww,Wwf,Ww,Ds,Ss,Ww,Ds^Ftd,Ds,Hd,Ww",
		f.all(
			f.terrain("W*,Dd^Do"),
			f.adjacent(f.terrain("Dd^Do,Ds^Vdt"))
		),
	}

	-- "fill" water inside island border
	-- terrain is replaced for ford, wich works as "mark" to be randomized in map repaint
	-- to create an island shape zone, we define a cross with 2 "axis", and then generate a elipse by keeping constant addition of both distances from axis

	local top = map.height // 10
	local bot = map.height - top + 1
	local temp_size = map.width * 3 // 7
	temp_size = map.height- temp_size * 2
	local radius = (bot - top - temp_size - 1 ) // 2

	function is_in_octaegon(x, y)
		-- the octaegon is defined as he convex hull of a 'cross'
		-- the horizonal 'axis' of the cross goes from y_start to y_end (and infinitely horizontally)
		-- the vertical 'axis' of the cross goes from x_start to x_end (and infinitely verticially )
		local y_start = map.height * 9 // 19
		local y_end = map.height - y_start
		local x_start = map.width * 3 // 7
		local x_end = map.width - x_start
		local max_dist = radius
		local d_x = math.max(0, x - x_end, x_start - x)
		local d_y = math.max(0, y - y_end, y_start - y)
		return d_x + d_y < max_dist
	end
	for x = 0, map.width - 1 do
		for y = 0, map.height - 1 do
			if is_in_octaegon(x, y) then

			end
		end
	end

	local water_tiles = map:find(f.terrain("W*"))
	for i, loc in ipairs(water_tiles) do
		-- todo: it mighjt be nice to add suppot for a lua function filter, so that we
		--      can pass is_in_octaegon to get_location, and use set_terrain directly.
		-- note: the reason why i didnt add support for lua functions yet is that i
		--       might want lua filter objects to be serializable.
		if is_in_octaegon(loc[1], loc[2]) then
			map[loc] = "Wwf"
		end
	end
end

function wct_noise_delta()
	set_terrain { "Gs^Ft,Gs^Ft,Hh,Hh,Hh",
		f.terrain("Gg"),
		fraction = 5,
	}
	set_terrain { "Ds^Ftd,Ds^Ftd,Hd,Hd,Hd",
		f.terrain("Dd"),
		fraction = 5,
	}
	set_terrain { "Ur,Uue^Tf,Uue^Dr,Uue",
		f.terrain("U*,U*^Tf"),
		fraction = 3,
	}
	set_terrain { "Gs,Gs,Gs,Gs,Hh,Hh,Hh,Gs^Ft,Gs^Ft,Gs^Ft,Gs^Tf",
		f.terrain("Mm"),
		fraction = 2,
	}
	set_terrain { "Gs,Gs,Gs,Gs,Gs,Gs,Gs^Ft,Gs^Ft,Gs^Ft,Mm",
		f.terrain("Hh"),
		fraction = 2,
	}
	set_terrain { "Gs,Gs,Gs,Gs,Gs,Gg,Hh^Ft,Gs^Ft,Gs^Ft,Gs^Ft,Mm,Gs^Tf",
		f.terrain("Hh"),
		fraction = 5,
	}
	set_terrain { "Dd,Dd,Dd,Dd,Dd^Do,Ds^Ftd,Ds^Ftd,Ds^Ftd,Mm,Gs",
		f.terrain("Hd"),
		fraction = 2,
	}
	set_terrain { "Dd,Dd,Dd,Dd,Sm,Ds^Ftd,Ds^Ftd,Ds^Ftd,Mm,Hd^Ftd",
		f.terrain("Hd"),
		fraction = 5,
	}
	set_terrain { "Gs,Gs,Gs,Gs,Gs,Gs,Gg^Fet,Hh,Hh,Hh^Ft",
		f.terrain("Gs^Fp,Gs^Ft,Gg^Fet"),
		fraction = 3,
	}
end

function world_conquest_tek_map_decoration_3c()
	set_terrain { "Ql",
		f.all(
			f.terrain("!,Wo"),
			f.none(
				f.radius(2, f.terrain("!,W*"))
			)
		),
	}
	set_terrain { "Gs,Gg,Hh^Ft,Ss,Ss,Hh,Ss,Mm,Ss,Hh^Tf,Ww,Gg,Ss,Ww,Ss",
		f.terrain("Wwf"),
		fraction = 2,
	}
	set_terrain { "Ww",
		f.terrain("Wwf"),
		fraction = 9,
	}
	set_terrain { "Ss,Ss,Ss,Ds^Ftd,Ss,Mm,Ss,Ww,Ss,Hd,Ss,Gs^Tf,Ww,Gg,Ss",
		f.terrain("Wwf"),
		fraction = 2,
	}
	set_terrain { "Ww,Hh^Tf",
		f.terrain("Xuce"),
		fraction = 8,
	}
	set_terrain { "Ww",
		f.all(
			f.adjacent(f.terrain("Wo"), nil, 0),
			f.terrain("Wo")
		),
	}
	set_terrain { "Ww",
		f.all(
			f.adjacent(f.terrain("Wo")),
			f.terrain("Wwf")
		),
	}
	set_terrain { "Wwr",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("!,Wo,Ww"), nil, "1-6")
		),
		fraction = 10,
	}
	set_terrain { "Gs",
		f.all(
			f.adjacent(f.terrain("Ch,Kh")),
			f.terrain("Dd,Ds,Xuce")
		),
	}
	set_terrain { "Hh^Tf,Mm,Gg,Gs,Ss,Ds^Ftd,Hh,Hh^Ftp",
		f.all(
			f.terrain("Xuce"),
			f.radius(3, f.terrain("K*^*"))
		),
		fraction = 2,
	}
	set_terrain { "Gs",
		f.terrain("R*"),
	}
	set_terrain { "*^Ftp",
		f.terrain("*^Fp"),
		layer = "overlay",
	}
	set_terrain { "Wwf",
		f.terrain("*^B*"),
	}
	set_terrain { "Wwf",
		f.all(
			f.terrain("Gg,Gs"),
			f.adjacent(f.terrain("Wwf"))
		),
		fraction = 2,
	}
	set_terrain { "Ww,Ds",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("!,Wo,Ww"), nil, "1-6")
		),
		fraction = 10,
	}
	set_terrain { "Ww^Vm",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("!,Wo,Ww,*^V*"), nil, "1-6"),
			f.adjacent(f.terrain("*^V*"), nil, 0)
		),
		fraction = 31,
	}
	set_terrain { "Ww^Vm",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("!,Wo,Ww,*^V*"), nil, "1-6"),
			f.adjacent(f.terrain("*^V*"), nil, 0)
		),
		fraction = 31,
	}
	set_terrain { "Ds",
		f.all(
			f.terrain("G*"),
			f.adjacent(f.terrain("Wo"))
		),
		fraction = 10,
	}
	set_terrain { "Hh",
		f.all(
			f.adjacent(f.terrain("H*^F*")),
			f.terrain("Hd")
		),
	}
	set_terrain { "Ds^Vdt,Ds^Vda,Ds^Vda,Ds^Vda",
		f.all(
			f.terrain("G*^V*"),
			f.adjacent(f.terrain("D*^*"))
		),
	}
	set_terrain { "Gg^Vc,Gg^Vht,Gs^Vht",
		f.terrain("G*^V*"),
	}
	set_terrain { "Wwf^Vhs",
		f.terrain("D*^Do"),
		fraction = 6,
	}
	set_terrain { "Ww,Ww,Ds",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Hd"))
		),
	}
	set_terrain { "Ww",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Ww,G*^*"))
		),
		fraction = 30,
	}
	set_terrain { "Wwr",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Ww"))
		),
		fraction = 40,
	}
	set_terrain { "Ds,Hd,Ss,Wwf,Ds,Ss,Ww,Ds^Do",
		f.all(
			f.terrain("*^Do"),
			f.radius(2, f.terrain("Wo"))
		),
	}
	set_terrain { "Ds,Ww",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Ss"))
		),
		fraction = 2,
	}
	set_terrain { "Ds",
		f.all(
			f.adjacent(f.terrain("Wo")),
			f.terrain("Hh^Tf,G*^Tf,Mm")
		),
	}
	set_terrain { "Hh",
		f.all(
			f.adjacent(f.terrain("D*^*,Hd"), nil, 0),
			f.terrain("Hd")
		),
	}
	set_terrain { "Hh",
		f.all(
			f.adjacent(f.terrain("Ch,Kh")),
			f.terrain("Hd")
		),
	}
	set_terrain { "Ds,Ww^Bw\\,Gs",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Ch,Kh"), "nw", nil),
			f.adjacent(f.terrain("W*^*"), "se", nil)
		),
		fraction = 2,
	}
	set_terrain { "Ds,Ww^Bw/,Gs",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Ch,Kh"), "sw,ne", nil),
			f.adjacent(f.terrain("W*^*"), "sw,ne", nil)
		),
		fraction = 2,
	}
	set_terrain { "Ds,Ww^Bw\\,Gs",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Ch,Kh"), "se", nil),
			f.adjacent(f.terrain("W*^*"), "nw", nil)
		),
		fraction = 2,
	}
	set_terrain { "Ds,Gs,Ww",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("Ch,Kh"))
		),
	}
	set_terrain { "Ds",
		f.all(
			f.adjacent(f.terrain("W*^*")),
			f.terrain("Dd")
		),
	}
	set_terrain { "Ss^Tf",
		f.all(
			f.adjacent(f.terrain("Ss")),
			f.terrain("Hh^Tf")
		),
	}
	set_terrain { "Sm^Tf",
		f.all(
			f.adjacent(f.terrain("D*^*,Hd*^*"), nil, 6),
			f.terrain("Hh^Tf")
		),
	}
	set_terrain { "Uue,Uue,Uue,Uue,Ds,Ww,Uue^Dr,Sm",
		f.all(
			f.terrain("Xuce"),
			f.adjacent(f.terrain("Xuce"), nil, "3-6")
		),
	}
	set_terrain { "Gs,Gs,Gg",
		f.all(
			f.terrain("Ds,Hd,Dd,Hh,Mm,Ss"),
			f.adjacent(f.terrain("Gg,Gs")),
			f.none(
				f.all(
					f.terrain("Ds"),
					f.radius(2, f.terrain("Wo"))
				)
			)
		),
		fraction = 30,
	}
	set_terrain { "Ds,Gs",
		f.all(
			f.terrain("Wo,Ww"),
			f.adjacent(f.terrain("Ch,Kh")),
			f.adjacent(f.terrain("W*^V*,Wwr,W*^B*"), nil, 0)
		),
		fraction = 2,
	}
	set_terrain { "Ds^Esd,Ds,Ds,Ds^Esd,Ds,Ds,Ds^Esd,Ds,Ds,Dd^Do,Ds,Ds,Ds^Ftd",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("G*^*,C*,K*,H*^V*,M*^V*")),
			f.adjacent(f.terrain("*^B*"), nil, 0)
		),
		fraction = 2,
	}

	wct_map_reduce_castle_expanding_recruit("Ce", "Gs")
	-- random sandbank
	set_terrain { "Ww",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("!,Wo"), nil, 0)
		),
		fraction = 35,
	}
	set_terrain { "Ds",
		f.all(
			f.terrain("Wo"),
			f.adjacent(f.terrain("!,Wo,Ww*^*"), nil, 0)
		),
		fraction = 45,
	}


	if mathx.random(2) == 1 then
		wct_change_map_water("t")
	end
end

function wct_map_3c_post_bunus_decoration()
	wct_map_cave_path_to("Re")
	set_terrain { "Ds",
		f.terrain("Ql"),
	}
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"river^Delta")
	world_conquest_tek_map_constructor_delta()
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_3c()
	world_conquest_tek_bonus_points()
	wct_map_3c_post_bunus_decoration()
end
