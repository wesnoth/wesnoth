-- snow utilities were created when I was a noob about generator, mainly because classic generator handled bad snow
-- it would be more efficient fix generator, but it would mean fix a code already working and probably break some postgeneration [filter]
-- it also gives snow configurations different shape than generated ones, wich I like

function wct_randomize_snowed_forest()
	set_terrain { "Aa^Fpa,Aa^Fda,Aa^Fma",
		f.terrain("Aa^F*,Ha^F*"),
		layer = "overlay"
	}
end

function wct_expand_snow()
	set_terrain { "Ms^Vhha",
		f.all(
			f.terrain("Mm^Vhh"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"))
		),
	}
	set_terrain { "Ms",
		f.all(
			f.terrain("Mm"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha")),
			f.adjacent(f.terrain("Mv"), nil, 0)
		),
	}
	set_terrain { "Ha^Vhha",
		f.all(
			f.terrain("Hh^Vhh"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Cha",
		f.all(
			f.terrain("Ch"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Kha",
		f.all(
			f.terrain("Kh"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Ha^Fpa",
		f.all(
			f.terrain("Hh^F*"),
			f.adjacent(f.terrain("Aa^V*,Ha^V*,Kha,Cha"))
		),
	}
	set_terrain { "Ha",
		f.all(
			f.terrain("Hh"),
			f.adjacent(f.terrain("Aa^V*,Ha^V*,Kha,Cha"))
		),
	}
	set_terrain { "Ha",
		f.all(
			f.terrain("Hh"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Ha^Fpa",
		f.all(
			f.terrain("Hh^F*"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Ms",
		f.all(
			f.terrain("Mm"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha")),
			f.adjacent(f.terrain("Mv"), nil, 0)
		),
	}
	set_terrain { "Aa^Vea",
		f.all(
			f.terrain("G*^Ve,G*^Vd"),
			f.adjacent(f.terrain("Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Aa^Voa",
		f.all(
			f.terrain("G*^Vo"),
			f.adjacent(f.terrain("Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Aa^Vla",
		f.all(
			f.terrain("G*^Vl"),
			f.adjacent(f.terrain("Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Aa^Vha",
		f.all(
			f.terrain("G*^V*"),
			f.adjacent(f.terrain("Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Aa^Vhca",
		f.all(
			f.terrain("Rr^V*"),
			f.adjacent(f.terrain("Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Ha^Voa",
		f.all(
			f.terrain("H*^V*"),
			f.adjacent(f.terrain("Ms*^*,Ha^*,Kha,Cha"), nil, "2-6")
		),
	}
	set_terrain { "Ms^Vhha",
		f.all(
			f.terrain("Mm^Vhh"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"))
		),
	}
	set_terrain { "Ha^Tf",
		f.all(
			f.terrain("Hh^Tf"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"), nil, "3-6")
		),
	}
	set_terrain { "Ms^Xm",
		f.all(
			f.terrain("Mm^Xm"),
			f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha")),
			f.adjacent(f.terrain("Mv"), nil, 0)
		),
	}
	-- chances of expand ice
	if mathx.random(20) == 1 then
		local r = mathx.random(3)
		set_terrain { "Ai",
			f.all(
				f.terrain("Ww,Wo"),
				f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"), nil, r .. "-6")
			),
		}

	end
	if mathx.random(20) == 1 then
		local r = mathx.random(4, 5)
		set_terrain { "Ai",
			f.all(
				f.terrain("Aa"),
				f.adjacent(f.terrain("Aa^*,Ai,Ms*^*,Ha^*,Kha,Cha"), nil, r .. "-6")
			),
			fraction_rand = "1..3",
		}
	end
end

function wct_storm(terrain_to_change, snow)
	if #terrain_to_change == 0 then
		wesnoth.log("info", "wct_storm: #terrain_to_change == 0")
		return
	end
	for show_i = 1, snow do
		local loc = terrain_to_change[mathx.random(#terrain_to_change)]
		set_terrain { "Ai",
			f.terrain("Aa"),
			locs = { loc },
		}
		set_terrain { "Ha^Voa",
			f.terrain("Hh^V*"),
			locs = { loc },
		}
		set_terrain { "Ha",
			f.terrain("Hh"),
			locs = { loc },
		}
		set_terrain { "Ms^Vhha",
			f.terrain("Mm^V*"),
			locs = { loc },
		}
		set_terrain { "Ms",
			f.all(
				f.terrain("Mm"),
				f.adjacent(f.terrain("Mv"), nil, 0)
			),
			locs = { loc },
		}
		set_terrain { "Aa^Vea",
			f.terrain("G*^Ve,G*^Vd"),
			locs = { loc },
		}
		set_terrain { "Aa^Vha",
			f.terrain("G*^V*"),
			locs = { loc },
		}
		set_terrain { "Aa^Vhca",
			f.terrain("R*^V*"),
			locs = { loc },
		}
		set_terrain { "Aa^Fpa",
			f.terrain("G*^F*"),
			locs = { loc },
		}
		set_terrain { "Ha^Fpa",
			f.terrain("H*^F*"),
			locs = { loc },
		}
		set_terrain { "Aa",
			f.terrain("G*,R*"),
			locs = { loc },
		}
		set_terrain { "Cha",
			f.terrain("Ch"),
			locs = { loc },
		}
		set_terrain { "Kha",
			f.terrain("Kh*^*"),
			layer = "base",
			locs = { loc },
		}
	end
end
