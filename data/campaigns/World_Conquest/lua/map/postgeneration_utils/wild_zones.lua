
local function wct_terrain_replace(t)
	return t
end

local function wild_volcano_for_lava_zone(terrain_to_change)
	local possible_volcano = map:find(f.all(
		f.find_in("terrain_to_change"),
		f.adjacent(f.find_in("terrain_to_change"), "se,s,sw", 3)
	), { terrain_to_change = terrain_to_change })
	
	if #possible_volcano > 0 then
		local loc = possible_volcano[mathx.random(#possible_volcano)]
		set_terrain { "Md^Xm",
			f.adjacent(f.is_loc(loc), "ne,n,nw")
		}
		set_terrain { "Ql",
			f.is_loc(loc)
		}
	end
end

local wild_replacement_chances_zone_1_1 = {
	default = {
		wct_terrain_replace { terrain = "Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds^Es"}
	},
	chances = {
		{
			value=4,
			command = {
				wct_terrain_replace { terrain = "Wwf,Ai,Ai,Aa,Aa,Aa,Ha,Ha,Ms,Ha,Ha,Ms"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwf,Wwf,Ds"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwg,Ds"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwg,Wwrg,Wwrg,Ds,Ds"}
			}
		}
	}
}

local wild_replacement_chances_zone_1_2 = {
	default = {
		wct_terrain_replace { terrain = "Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds^Esd"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ds,Ss"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwf,Ds"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwrg" }
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwg,Wwrg,Wwrg,Ds,Ds,Wwrg,Wwrg,Ds,Ds,Wwf"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Gg,Gg,Gg,Gg,Gg^Fds,Hh,Hh^Fds,Mm"}
			}
		}
	}
}

local wild_replacement_chances_zone_1_3 = {
	default = {
		wct_terrain_replace { terrain = "Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds,Ds^Esd"}
	},
	chances = {
		{
			value=2,
			command = {
				wct_terrain_replace { terrain = "Ds,Ss"}
			}
		},
		{
			value=2,
			command = {
				wct_terrain_replace { terrain = "Ds^Esd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Do"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwr" }
			}
		}
	}
}

local wild_replacement_chances_zone_1_4 = {
	default = {
		wct_terrain_replace { terrain = "Ds^Esd,Ds"}
	},
	chances = {
		{
			value=9,
			command = {
				wct_terrain_replace { terrain = "Dd^Esd,Dd"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Uue^Esd,Uue,Uue,Uue^Es"},
				wild_volcano_for_lava_zone
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ds,Sm"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Dd,Dd,Dd,Dd^Esd,Hd,Hd,Md"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ds,Ds,Ds,Ds,Ds,Ds^Esd,Ds^Ftd,Ds^Do,Hhd"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ds^Esd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Ftd,Ds^Do"}
			}
		}
	}
}

local wild_replacement_chances_zone_2_1 = {
	default = {
		wct_terrain_replace { terrain = "Gg" },
		wct_terrain_replace { terrain = "Gg^Fp,Gg^Fp,Hh,Hh,Hh^Fp,Gg^Fp,Gg^Fp,Hh,Hh,Hh^Fp,Mm,Gg^Tf,Gg^Fmf", percentage = 20, strict = false}
	},
	chances = {
		{
			value=8,
			command = {
				wct_terrain_replace { terrain = "Aa,Aa,Aa,Aa,Aa,Aa,Aa,Aa,Ai,Gg,Wwf,Rb"},
				wct_terrain_replace { terrain = "Aa^Fpa,Aa^Fpa,Ha,Ha,Ha^Fpa,Gg^Fp,Aa^Fpa,Ha,Ha,Ha^Fpa,Ms,Ms,Ha,Aa^Fmf", percentage = 23, strict = false}
				--				wct_terrain_replace { terrain = "Aa^Fpa,Aa^Fpa,Ha,Ha,Ha^Fpa,Gg^Fp,Aa^Fpa,Ha,Ha,Ha^Fpa,Ms,Ms,Aa^Fmwa,Ha,Aa^Fmf", percentage = 23, strict = false}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss,Wwf,Ss,Wwf,Ww"}
			}
		}
	}
}

local wild_replacement_chances_zone_2_2 = {
	default = {
		wct_terrain_replace { terrain = "Gg,Gg,Gg,Ss"},
		wct_terrain_replace { terrain = "Gg^Fp,Gg^Fms,Hh,Hh,Hh^Fms,Gg^Fms,Gg^Fds,Hh,Hh,Hh^Fp,Mm,Gg^Tf,Gg^Fds,Gg^Fp", percentage = 20, strict = false}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss,Ss,Ss,Ss,Ss,Ss,Ss^Tf"}
			}
		}
	}
}

local wild_replacement_chances_zone_2_3 = {
	default = {
		wct_terrain_replace { terrain = "Gg,Gs,Ss"},
		wct_terrain_replace { terrain = "Gg^Fds,Gg^Fms,Hh,Hh,Hh^Fms,Gg^Ftr,Gg^Ftr,Hh,Hh,Hh^Fds,Mm,Hh^Tf,Gs^Ftp,Gs^Fds,Gs^Ftp", percentage = 20, strict = false}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss^Fts,Ss^Fts,Ss^Fts,Ss^Fts,Ss"}
			}
		}
	}
}

local wild_replacement_chances_zone_2_4 = {
	default = {
		wct_terrain_replace { terrain = "Gs" },
		wct_terrain_replace { terrain = "Gs^Fds,Gs^Ftp,Hh,Hh,Hh^Ftp,Gs^Fts,Gs^Ft,Hh,Hh,Hh^Fds,Md,Hh^Tf,Gs^Ftp,Gs^Ftd", percentage = 20, strict = false}
	},
	chances = {
		{
			value=2,
			command = {
				wct_terrain_replace { terrain = "Ds,Sm"}
			}
		},
		{
			value=17,
			command = {
				wct_terrain_replace { terrain = "Dd,Dd,Dd,Hd,Dd,Dd,Dd,Hd,Dd,Dd,Dd,Hd,Dd,Dd,Dd,Hhd,Dd,Dd,Dd,Md"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Hhd,Ur"},
				wild_volcano_for_lava_zone
			}
		}
	}
}

local wild_replacement_chances_zone_3_1 = {
	default = {
		wct_terrain_replace { terrain = "Gg" },
		wct_terrain_replace { terrain = "Gg^Fp,Gg^Fp,Hh,Hh,Hh^Fp,Gg^Fp,Gg^Fp,Hh,Hh,Hh^Fp,Mm,Gg^Tf,Gg^Fmf", percentage = 25, strict = false}
	},
	chances = {
		{
			value=11,
			command = {
				wct_terrain_replace { terrain = "Aa,Aa,Aa,Aa,Aa,Aa,Aa,Aa,Ai,Gg,Wwf,Rb"},
				wct_terrain_replace { terrain = "Aa^Fpa,Aa^Fpa,Ha,Ha,Ha^Fpa,Gg^Fp,Aa^Fpa,Ha,Ha,Ha^Fpa,Ms,Ms,Ha,Aa^Fmf", percentage = 30, strict = false}
				--wct_terrain_replace { terrain = "Aa^Fpa,Aa^Fpa,Ha,Ha,Ha^Fpa,Gg^Fp,Aa^Fpa,Ha,Ha,Ha^Fpa,Ms,Ms,Aa^Fmwa,Ha,Aa^Fmf", percentage = 30, strict = false}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss,Wwf,Ss,Wwf,Ww"}
			}
		}
	}
}

local wild_replacement_chances_zone_3_2 = {
	default = {
		wct_terrain_replace { terrain = "Gg" },
		wct_terrain_replace { terrain = "Gg^Fp,Gg^Fms,Hh,Hh,Hh^Fms,Gg^Fms,Gg^Fds,Hh,Hh,Hh^Fp,Mm,Gg^Tf,Gg^Fds,Gg^Fp", percentage = 25, strict = false}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss,Ss,Ss,Ss,Ss,Ss,Ss^Tf"}
			}
		}
	}
}

local wild_replacement_chances_zone_3_3 = {
	default = {
		wct_terrain_replace { terrain = "Gg,Gg,Gs"},
		wct_terrain_replace { terrain = "Gg^Fds,Gg^Fms,Hh,Hh,Hh^Fms,Gg^Ftr,Gg^Ftr,Hh,Hh,Hh^Fds,Mm,Hh^Tf,Gs^Ftp,Gs^Fds,Gs^Ftp", percentage = 25, strict = false}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss^Fts,Ss^Fts,Ss^Fts,Ss^Fts,Ss^Fts,Ss"}
			}
		}
	}
}

local wild_replacement_chances_zone_3_4 = {
	default = {
		wct_terrain_replace { terrain = "Gs" },
		wct_terrain_replace { terrain = "Gs^Fds,Gs^Ftp,Hh,Hh,Hh^Ftp,Gs^Fts,Gs^Ft,Hh,Hh,Hh^Fds,Md,Hh^Tf,Gs^Ftp,Gs^Ftd", percentage = 25, strict = false}
	},
	chances = {
		{
			value=18,
			command = {
				wct_terrain_replace { terrain = "Dd,Dd,Dd,Hd,Dd,Dd,Dd,Hd,Dd,Dd,Dd,Hd,Dd,Dd,Dd,Hhd,Dd,Dd,Dd,Md"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Sm^Fdw,Sm^Fdw,Sm,Sm,Wwf"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Hhd,Ur,Hhd,Ur,Hhd,Ur,Hhd,Ql"},
				wild_volcano_for_lava_zone
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Gd,Gd,Gd,Hhd,Gd,Gd,Gd,Hhd,Gd,Gd,Gd,Hh,Gd,Gd,Gd,Hh^Fts"}
			}
		}
	}
}

local wild_replacement_chances_zone_4_1 = {
	default = {
		wct_terrain_replace { terrain = "Gg" },
		wct_terrain_replace { terrain = "Gg^Fp,Gg^Fp,Hh,Hh,Hh^Fp,Gg^Fp,Gg^Fp,Hh,Hh,Hh^Fp,Mm,Gg^Tf,Hh,Hh,Hh^Fp,Ms,Hh,Hh,Hh^Fp,Gg^Fmf", percentage = 52, strict = false}
	},
	chances = {
		{
			value=14,
			command = {
				wct_terrain_replace { terrain = "Ms,Ms,Ha,Ha,Ha,Ha,Ha^Fpa,Aa,Aa^Fpa"}
			}
		}
	}
}

local wild_replacement_chances_zone_4_2 = {
	default = {
		wct_terrain_replace { terrain = "Gg,Gg^Efm"},
		wct_terrain_replace { terrain = "Gg^Fp,Gg^Fms,Hh,Hh,Hh^Fms,Gg^Fms,Gg^Fds,Hh,Hh,Hh^Fp,Mm,Gg^Tf,Hh,Hh,Hh^Fp,Mm,Hh,Hh,Hh^Fp,Gg^Fds,Gg^Fp", percentage = 52, strict = false}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss,Ss,Ss,Ss,Ss,Ss,Ss^Tf"}
			}
		}
	}
}

local wild_replacement_chances_zone_4_3 = {
	default = {
		wct_terrain_replace { terrain = "Gg,Gg,Gs"},
		wct_terrain_replace { terrain = "Gg^Fds,Gg^Fms,Hh,Hh,Hh^Fms,Gg^Ftr,Gg^Ftr,Hh,Hh,Hh^Fds,Mm,Hh^Tf,Hh,Hh,Hh^Fds,Mm,Hh,Hh,Hh^Fds,Gs^Fds,Gs^Ftp", percentage = 52, strict = false}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss^Fts,Ss^Fts,Ss^Fts,Ss^Fts,Ss^Fts,Hh^Ft,Ss"}
			}
		}
	}
}

local wild_replacement_chances_zone_4_4 = {
	default = {
		wct_terrain_replace { terrain = "Gs" },
		wct_terrain_replace { terrain = "Gs^Fds,Gs^Ftp,Hh,Hh,Hh^Ftp,Gs^Fts,Gs^Ft,Hh,Hh,Hh^Fds,Md,Hh^Tf,Hh,Hh,Hh^Fds,Md,Hh,Hh,Hh^Fds,Gs^Ftp", percentage = 52, strict = false}
	},
	chances = {
		{
			value=9,
			command = {
				wct_terrain_replace { terrain = "Dd,Dd,Dd,Dd,Hd,Hd,Hhd,Md"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Sm^Fdw,Sm^Fdw,Sm^Fdw,Sm,Wwf"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Hhd,Ur,Hhd,Ur,Hhd,Ql"},
				wild_volcano_for_lava_zone
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Gd,Hhd,Gd,Hhd,Gd,Hh,Gd,Hh^Fts,Gd,Md"}
			}
		}
	}
}

local wild_replacement_chances_zone_5_1 = {
	default = {
		wct_terrain_replace { terrain = "Gg^Fp,Aa^Fpa,Hh,Hh,Hh^Fp,Gg^Fp,Gg^Fp,Ha,Ha,Ha^Fpa,Ms,Aa^Fpa,Ms,Ms,Ms,Ms,Ms,Ms,Ms,Ms,Gg,Gg,Gg,Gg,Gg,Gg,Wwf,Gg^Fmf,Hh^Fmw,Ha^Fpa,Ha"}
	},
	chances = {
		{
			value=17,
			command = {
				wct_terrain_replace { terrain = "Ms,Ha,Ms,Ms,Ha^Fpa,Aa"}
				--				--todo: the odl code contained invaldi terraisncodes
				--				wct_terrain_replace { terrain = "Ms,Ha,Ms,Ms,Ha^Fdwa,Ha^Fmwa,,Ha^Fpa,Aa^Fmwa,Aa^Fdwa,Aa"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ww" }
			}
		}
	}
}

local wild_replacement_chances_zone_5_2 = {
	default = {
		wct_terrain_replace { terrain = "Gg^Fp,Gg^Fms,Hh,Hh,Hh^Fms,Gg^Fms,Gg^Fds,Hh,Hh,Hh^Fp,Hh^Fp,Mm,Gg^Tf,Mm,Mm,Mm,Mm,Mm,Mm,Mm,Mm,Gg,Gg,Gg,Gg,Gg,Gg,Gg,Hh^Fp,Gg^Fds,Hh"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ww,Hh,Mm"}
			}
		}
	}
}

local wild_replacement_chances_zone_5_3 = {
	default = {
		wct_terrain_replace { terrain = "Gg^Fds,Gg^Fms,Hh,Hh,Hh^Fms,Hh^Fms,Gg^Ftr,Gg^Ftr,Hh,Hh,Hh^Fds,Mm,Hh^Tf,Mm,Mm,Mm,Mm,Mm,Mm,Mm,Mm,Gg,Gg,Gs,Gg,Gg,Gs,Gg,Gs^Ftp,Gs^Fds,Hh"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss^Fts,Ss,Ss,Hh^Ft"}
			}
		}
	}
}

local wild_replacement_chances_zone_5_4 = {
	default = {
		wct_terrain_replace { terrain = "Gs^Fds,Gs^Ftp,Hh,Hh,Hh^Ftp,Gs^Fts,Gs^Ft,Hh,Hh,Hh^Fds,Hh^Fds,Md,Hh^Tf,Md,Md,Md,Md,Md,Md,Md,Md,Gg,Gs,Gg,Gs,Gg,Gs,Gs,Gs^Ftp,Gs^Ftr,Hh"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ql" },
				wild_volcano_for_lava_zone
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Gd,Hhd,Gd,Hhd,Gd,Hh,Gd,Hh^Fts,Gd,Md,Md,Md"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Sm^Fdw,Sm^Fdw,Sm^Fdw,Sm,Wwf,Hh,Hh^Fds"}
			}
		}
	}
}

local wild_replacement_chances_zone_6_1 = {
	default = {
		wct_terrain_replace { terrain = "Ms,Ms^Xm,Ms^Xm,Ms^Xm,Ms,Ms^Xm,Ms^Xm,Ms^Xm,Ms,Ms^Xm,Ms^Xm,Ms^Xm,Ha,Ha^Fpa,Aa,Ha^Fpa,Ha"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ai" }
			}
		}
	}
}

local wild_replacement_chances_zone_6_2 = {
	default = {
		wct_terrain_replace { terrain = "Mm,Ms^Xm,Mm^Xm,Mm^Xm,Mm,Ms^Xm,Mm^Xm,Mm^Xm,Mm,Ms^Xm,Mm^Xm,Mm^Xm,Hh^Fp,Hh^Tf,Gg,Hh^Fms,Hh"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwg" }
			}
		}
	}
}

local wild_replacement_chances_zone_6_3 = {
	default = {
		wct_terrain_replace { terrain = "Mm,Mm^Xm,Mm^Xm,Mm^Xm,Mm,Mm^Xm,Mm^Xm,Mm^Xm,Mm,Mm^Xm,Mm^Xm,Mm^Xm,Hh^Fms,Hh^Tf,Gg,Hh^Fds,Hh"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss^Fts,Ss,Hh^Ft,Mm"}
			}
		}
	}
}

local wild_replacement_chances_zone_6_4 = {
	default = {
		wct_terrain_replace { terrain = "Md,Md^Xm,Md^Xm,Md^Xm,Md,Md^Xm,Md^Xm,Md^Xm,Md,Md^Xm,Md^Xm,Md^Xm,Hh^Fds,Hh^Tf,Gs,Hh^Ftp,Hh"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ql" },
				wild_volcano_for_lava_zone
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Gd,Hhd,Gd,Hhd,Gd,Hh,Gd,Hh^Fts,Gd,Md,Md,Md,Hhd,Md"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Sm,Sm,Sm,Sm,Sm,Sm^Tf,Mm,Hh"}
			}
		}
	}
}

local wild_replacement_chances_zone_7_1 = {
	default = {
		wct_terrain_replace { terrain = "Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uu,Rb,Wwg,Ai,Qxu,Uh,Uu,Uu,Uh,Uu,Uu^Em"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Qxu"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ai"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwg"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Uh,Ai"}
			}
		}
	}
}

local wild_replacement_chances_zone_7_2 = {
	default = {
		wct_terrain_replace { terrain = "Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uu,Re,Ww,Wwf,Qxu,Uu^Tf,Uh,Uu,Uu,Uh,Uu,Uu^Em,Uu^Tf"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ww"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ww,Wwf"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Wwf"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Uh,Ww"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Uu,Qxu"}
			}
		}
	}
}

local wild_replacement_chances_zone_7_3 = {
	default = {
		wct_terrain_replace { terrain = "Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uh,Uu,Uu^Tf,Uu,Uu,Re,Uh^Tf,Wwf,Ql,Uu^Tf,Uh,Uu,Uu,Uh,Uu,Uu^Em,Uu^Tf"}
	},
	chances = {
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Gll^Tf"}
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Ss"}
			}
		}
	}
}

local wild_replacement_chances_zone_7_4 = {
	default = {
		wct_terrain_replace { terrain = "Ql,Uh,Uu,Uu^Tf,Uh^Tf,Uh,Uu,Ql,Uh,Uu,Re,Ur,Uh,Uu,Uu,Uh,Uu,Uu^Em,Uu^Tf"}
	},
	chances = {
		{
			value=2,
			command = {
				wct_terrain_replace { terrain = "Ql"},
				wild_volcano_for_lava_zone
			}
		},
		{
			value=1,
			command = {
				wct_terrain_replace { terrain = "Sm,Sm,Sm,Sm,Sm,Sm,Sm^Tf"}
			}
		}
	}
}

local function river_to_lava_postfix(terrain_to_change)
	set_terrain {
		terrain = "Ur^Es",
		filter = f.all(
			f.terrain("Wwf"),
			f.adjacent(f.find_in("terrain_to_change"))
		),
		filter_extra = { terrain_to_change = terrain_to_change },
	}
	
	wild_volcano_for_lava_zone(terrain_to_change)
	local filter_adjacent_grassland = wesnoth.map.filter(f.all(
		f.terrain("G*^*"),
		f.adjacent(f.find_in("terrain_to_change"))
	), { terrain_to_change = terrain_to_change })
	
	adjacent_grssland = map:find(filter_adjacent_grassland)
	
	set_terrain {
		terrain = "Ur,Re,Re,Gd,Gd,Gd",
		locs = 	adjacent_grssland,
		layer = "base"
	}
	
	set_terrain {
		terrain = "*^Fdw",
		filter = f.all(
			f.find_in("adjacent_grssland"),
			f.terrain("*^F*")
		),
		filter_extra = { adjacent_grssland = adjacent_grssland },
		layer = "overlay"
	}
	set_terrain {
		terrain = "*",
		filter = f.all(
			f.find_in("adjacent_grssland"),
			f.terrain("*^Efm")
		),
		filter_extra = { adjacent_grssland = adjacent_grssland },
		layer = "overlay"
	}
	
end

local wild_replacement_chances_zone_8_1 = {
	default = {
		-- do nothing
	},
	chances = {
		{
			value=5,
			command = {
				wct_terrain_replace { terrain = "Wwf", percentage = 30, strict = false}
			}
		},
		{
			value=10,
			command = {
				wct_terrain_replace { terrain = "Ss", filter = f.none(f.terrain("Wwf")) }
			}
		},
		{
			value=8,
			command = {
				wct_terrain_replace { terrain = "Wwg", filter = f.none(f.terrain("Wwf")) }
			}
		},
		{
			value=7,
			command = {
				wct_terrain_replace { terrain = "Wwt", filter = f.none(f.terrain("Wwf")) }
			}
		},
		{
			value=3,
			command = {
				wct_terrain_replace { terrain = "Qlf", filter = f.none(f.radius(1, f.terrain("Wwf"))) },
				wct_terrain_replace { terrain = "Ur^Es", filter = f.terrain("Wwf") },
				river_to_lava_postfix
			}
		}
	}
}

return {
	{
		{
			terrain="Wwrg",
			wild_replacement_chances_zone_1_1
		},
		{
			terrain="Wwr",
			wild_replacement_chances_zone_1_2
		},
		{
			terrain="Wwrt",
			wild_replacement_chances_zone_1_3
		},
		{
			terrain="Ds",
			wild_replacement_chances_zone_1_4
		}
	},
	{
		{
			terrain="Gll",
			wild_replacement_chances_zone_2_1
		},
		{
			terrain="Gg",
			wild_replacement_chances_zone_2_2
		},
		{
			terrain="Gs",
			wild_replacement_chances_zone_2_3
		},
		{
			terrain="Gd",
			wild_replacement_chances_zone_2_4
		}
	},
	{
		{
			terrain="Aa",
			wild_replacement_chances_zone_3_1
		},
		{
			terrain="Ss",
			wild_replacement_chances_zone_3_2
		},
		{
			terrain="Sm",
			wild_replacement_chances_zone_3_3
		},
		{
			terrain="Dd",
			wild_replacement_chances_zone_3_4
		}
	},
	{
		{
			terrain="Ha",
			wild_replacement_chances_zone_4_1
		},
		{
			terrain="Hh",
			wild_replacement_chances_zone_4_2
		},
		{
			terrain="Hhd",
			wild_replacement_chances_zone_4_3
		},
		{
			terrain="Hd",
			wild_replacement_chances_zone_4_4
		}
	},
	{
		{
			terrain="Ms",
			wild_replacement_chances_zone_5_1
		},
		{
			terrain="Mm",
			wild_replacement_chances_zone_5_2
		},
		{
			terrain="Md",
			wild_replacement_chances_zone_5_3
		},
		{
			terrain="Mv",
			wild_replacement_chances_zone_5_4
		}
	},
	{
		{
			terrain="Ms^Xm",
			wild_replacement_chances_zone_6_1
		},
		{
			terrain="Mm^Xm",
			wild_replacement_chances_zone_6_2
		},
		{
			terrain="Md^Xm",
			wild_replacement_chances_zone_6_3
		},
		{
			terrain="Md^Dr",
			wild_replacement_chances_zone_6_4
		}
	},
	{
		{
			terrain="Xu",
			wild_replacement_chances_zone_7_1
		},
		{
			terrain="Xuc",
			wild_replacement_chances_zone_7_2
		},
		{
			terrain="Xue",
			wild_replacement_chances_zone_7_3
		},
		{
			terrain="Xuce",
			wild_replacement_chances_zone_7_4
		}
	},
	{
		{
			--Ww = river.
			terrain="Ww,Wwf",
			wild_replacement_chances_zone_8_1
		}
	}
}
