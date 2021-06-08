-- Paradise

local images = {
	citadel_leanto = "misc/blank-hex.png~BLIT(scenery/leanto.png~CS(10,10,0)~CROP(0,6,72,66))",
}

function wct_map_2e_post_bunus_decoration()
	wct_map_decoration_3e_keeps()
	wct_map_decoration_3e_leantos()
end

function world_conquest_tek_map_repaint_2e()
	world_conquest_tek_map_noise_proxy(1, mathx.random(1,2), "!,W*^*,Ds*^*,X*,M*^Xm,R*^*,Ch*,K*,U*^*,Ql^B*")

	-- create citadel castles
	wct_map_reduce_castle_expanding_recruit("Xos", "Rr^Fet")
	set_terrain { "Ch",
		f.terrain("Xos"),
	}
	-- create mini lakes
	set_terrain { "Wwt",
		f.all(
			f.terrain("Gg"),
			f.adjacent(f.terrain("W*^*,Ds*^*,S*^*"), nil, 0)
		),
		fraction = 20,
	}
	-- chance of dirt beachs
	set_terrain { "Ds^Edt,Ds",
		f.all(
			f.terrain("Ds"),
			f.adjacent(f.terrain("Ds^Vc"))
		),
		fraction_rand = "2..10",
	}
	-- eliminate bridges bad generated
	set_terrain { "Rr",
		f.all(
			f.terrain("W*^Bsb/"),
			f.adjacent(f.terrain("R*^*,C*,G*^V*,W*^Bsb/"), "ne,sw", "0-1")
		),
	}
	set_terrain { "Rr",
		f.all(
			f.terrain("W*^Bsb\\"),
			f.adjacent(f.terrain("R*^*,C*,G*^V*,W*^Bsb\\"), "nw,se", "0-1")
		),
	}
	set_terrain { "Rr",
		f.all(
			f.terrain("W*^Bsb|"),
			f.adjacent(f.terrain("R*^*,C*,G*^V*,W*^Bsb|"), "n,s", "0-1")
		),
	}

	wct_conect_isolated_citadel()
	-- create villages in empty citadels
	local terrain_to_change = wct_store_empty_citadel()
	while #terrain_to_change > 0 do
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = "Rr^Vhc"
		terrain_to_change = wct_store_empty_citadel()
	end
	-- improve roads quality
	local r = mathx.random(2,4)
	set_terrain { "Rr",
		f.all(
			f.terrain("Re"),
			f.radius(r, f.terrain("Rr*^*,Ch,Kh,W*^Bsb*"), f.terrain("R*^*,C*,K*,W*^Bsb*"))
		),
	}
	set_terrain { "Rr^Vhc",
		f.all(
			f.terrain("G*^Vh"),
			f.adjacent(f.terrain("Rr^*,W*^Bsb*"))
		),
	}
	-- log villages
	set_terrain { "*^Vl",
		f.all(
			f.terrain("*^Vh"),
			f.adjacent(f.terrain("G*^F*")),
			f.adjacent(f.terrain("R*"), nil, 0)
		),
		layer = "overlay",
	}

	local max_yards = map.height * map.width // 300
	local nyards = tonumber(mathx.random_choice("1,0.." .. max_yards))
	for i = 1, nyards do
		local yard_dir = "n,nw,ne"
		local yard_cdir = "s,sw,se"
		if mathx.random(2) == 1 then
			yard_dir, yard_cdir = yard_cdir, yard_dir
		end
		wct_map_yard(yard_dir, yard_cdir)
	end


	-- chance of farms replacing yards
	if mathx.random(20) == 1 then
		set_terrain { "Rb^Gvs,Rb^Gvs,Rb^Gvs,Gg",
			f.terrain("*^Eff"),
		}

	end
	-- fix mountains map amount
	set_terrain { "Mm",
		f.all(
			f.terrain("Gg"),
			f.adjacent(f.terrain("W*^*,Ds*^*,S*^*"), nil, 0)
		),
		fraction = 20,
	}
	set_terrain { "Gg",
		f.terrain("Mm"),
		fraction = 2,
	}

	wct_conect_isolated_citadel()
end

function wct_map_yard(directions, counter_directions)
	-- todo: is this code 'symmetric' andin the sense that switching
	--       directions and counter_directions  doesn't change anythign at all?
	local terrain_to_change = map:find(f.all(
		f.terrain("Gg"),
		f.adjacent(f.terrain("Gg"), directions, 3),
		f.any(
			f.adjacent(f.terrain("Gg^Vl")),
			f.adjacent(f.all(
				f.terrain("Gg"),
				f.adjacent(f.terrain("Gg^Vl"))
			), directions, nil)
		)
	))

	if #terrain_to_change > 0 then
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		map[loc] = "Gg^Eff"
		set_terrain { "Gg^Eff",
			f.adjacent( f.is_loc(loc), counter_directions, nil)
		}
	end
end

function wct_conect_isolated_citadel()
	local isolated = map:find(f.all(
		f.terrain("Rr*^*,Ch,Kh,W*^Bsb*"),
		f.adjacent(f.terrain("R*^*,Ch,Kh,W*^Bsb*"), nil, 0)
	))
	set_terrain { "Rr",
		f.all(
			f.terrain("Gg"),
			f.adjacent(f.find_in("isolated")),
			f.adjacent(f.all(
				f.terrain("R*^*,Ch,Kh,W*^Bsb*"),
				f.none(
					f.find_in("isolated")
				)
			))
		),
		filter_extra = { isolated = isolated}
	}
end

function wct_store_empty_citadel()
	return map:find(f.all(
		f.terrain("Rr"),
		f.none(
			f.radius(4, f.terrain("Rr^Vhc"), f.terrain("Rr*^*,Ch*,Kh*,W*^Bsb*"))
		)
	))

end

function wct_map_decoration_3e_keeps()
	set_terrain { "Ch,Kh",
		f.all(
			f.terrain("Ch"),
			f.adjacent(f.terrain("*^Bsb*")),
			f.adjacent(f.terrain("Ch"), nil, 0)
		),
	}

end

function wct_map_decoration_3e_leantos()
	local terrain_to_change = map:find(f.all(
		f.terrain("Rr"),
		f.none(
			f.find_in("bonus_point")
		),
		f.any(
			f.adjacent(f.all(
				f.terrain("Ch"),
				f.none(
					f.radius(999, f.terrain("K*"), f.terrain("C*"))
				)
			)),
			f.adjacent(f.terrain("Ch,Rr^*"), nil, "5-6")
		)
	))
	for i, v in ipairs(terrain_to_change) do
		if mathx.random(3) == 1 then
			map[v] = "Rrc"

			table.insert(prestart_event, wml.tag.item {
				x = v[1],
				y = v[2],
				image = images.citadel_leanto,
				name = "wc2_citadel_leanto",
			})
		end
	end
end

local _ = wesnoth.textdomain 'wesnoth-wc'

return function()
	set_map_name(_"Paradise")
	wct_enemy_castle_expansion()
	world_conquest_tek_map_repaint_2e()
	world_conquest_tek_bonus_points("paradise")
	wct_map_2e_post_bunus_decoration()
end
