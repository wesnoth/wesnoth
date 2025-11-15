local _ = wesnoth.textdomain "wesnoth-h2tt"

-- https://wiki.wesnoth.org/LuaAPI/types/widget

-- metatable for GUI tags
local T = wml.tag

-- helpful debug function for printing a table
function tprint (tbl, indent)
	if not indent then indent = 0 end
	local toprint = string.rep(" ", indent) .. "{\r\n"
	indent = indent + 2
	for k, v in pairs(tbl) do
		toprint = toprint .. string.rep(" ", indent)
		if (type(k) == "number") then
			toprint = toprint .. "[" .. k .. "] = "
		elseif (type(k) == "string") then
			toprint = toprint  .. k ..  "= "
		end
		if (type(v) == "number") then
			toprint = toprint .. v .. ",\r\n"
		elseif (type(v) == "string") then
			toprint = toprint .. "\"" .. v .. "\",\r\n"
		elseif (type(v) == "table") then
			toprint = toprint .. tprint(v, indent + 2) .. ",\r\n"
		else
			toprint = toprint .. "\"" .. tostring(v) .. "\",\r\n"
		end
	end
	toprint = toprint .. string.rep(" ", indent-2) .. "}"
	return toprint
end





















--###########################################################################################################################################################
--                                                                 SCENARIO PREVIEW
--###########################################################################################################################################################
function wesnoth.wml_actions.display_scenario_preview(cfg)
	--###############################
	-- PREPARE ARGUMENTS
	--###############################
	local title           = cfg.title
	local scenario        = cfg.scenario -- e.g. "s01"
	local preview_image   = cfg.preview or "bigmap/preview-"..scenario..".png"
	local difficulty      = cfg.difficulty and "bigmap/difficulty"..cfg.difficulty..".png" or "" -- e.g. "difficulty1.png"
	local difficultylabel = cfg.difficulty and _"Difficulty: " or ""

	-------------------------
	-- REWARD: GOLD
	-------------------------
	-- hopefully "reward" makes it clear that this is what you're expected to have after the scenario, not before
	local gold = cfg.gold
	if (gold==0  ) then goldlabel=_"Expected Gold Carryover Reward: <b><span color='#ad6a61'>NONE</span></b>" end
	if (gold==1  ) then goldlabel=_"Expected Gold Carryover Reward: <b><span color='#a9a150'>LOW</span></b>"  end
	if (gold==2  ) then goldlabel=_"Expected Gold Carryover Reward: <b><span color='#6ca364'>HIGH</span></b>" end
	if (gold==nil) then goldlabel = "" end

	-------------------------
	-- REWARD: RECRUITS
	-------------------------
	local recruitcolor = cfg.recruitcolor and cfg.recruitcolor or "blue"
	local recruitlabel = cfg.recruitlabel and cfg.recruitlabel or _"New Recruits:"
	local recruit1     = cfg.recruit1     and cfg.recruit1.."~RC(magenta>"..recruitcolor..")" or "misc/blank-hex.png"
	local recruit2     = cfg.recruit2     and cfg.recruit2.."~RC(magenta>"..recruitcolor..")" or "misc/blank-hex.png"
	local recruit3     = cfg.recruit3     and cfg.recruit3.."~RC(magenta>"..recruitcolor..")" or "misc/blank-hex.png"
	local recruit4     = cfg.recruit4     and cfg.recruit4.."~RC(magenta>"..recruitcolor..")" or "misc/blank-hex.png"

	-------------------------
	-- REWARD: COMPANION
	-------------------------
	local companioncolor = cfg.companioncolor and cfg.companioncolor or "blue"
	local companionlabel = cfg.companionlabel and cfg.companionlabel or _"New Companions:"
	local companion1     = cfg.companion1 and cfg.companion1.."~RC(magenta>"..companioncolor..")~BLIT(misc/loyal-icon.png,5,10)" or "misc/blank-hex.png"
	local companion2     = cfg.companion2 and cfg.companion2.."~RC(magenta>"..companioncolor..")~BLIT(misc/loyal-icon.png,5,10)" or "misc/blank-hex.png"

	-------------------------
	-- REWARD: OTHER
	-------------------------
	local otherlabel = cfg.otherlabel and "\n"..cfg.otherlabel.."\n" or ""

	--##############################
	-- DEFINE GRID
	--###############################
	local grid = T.grid{ T.row{
		T.column{ T.label{  use_markup=true,  label="<span size='40000'> </span>"  }},
		T.column{ border="right,left,bottom", border_size=18, T.grid{
			-------------------------
			-- TITLE
			-------------------------
			T.row{ T.column{ T.image{  label="icons/banner3.png"  }}},
			T.row{ T.column{ T.grid{ T.row{
				T.column{ T.image{
					-------------------------
					-- PREVIEW IMAGE
					-------------------------
					horizontal_alignment="left",
					label=preview_image,
				}},
				T.column{ T.label{ label="   " }},
				T.column{ T.grid{
					-------------------------
					-- TITLE AND DIFFICULTY
					-------------------------
					T.row{ T.column{ T.label{  use_markup=true,  label="<span size='5000'> </span>"  }}},
					T.row{ T.column{
						horizontal_alignment="left",
						T.label{  definition="title",label=title,  }
					}},
					T.row{ T.column{ T.label{  use_markup=true,  label="<span size='5000'> </span>"  }}},
					T.row{ T.column{
						horizontal_alignment="left",
						T.grid{ T.row{
							T.column{ T.label{  use_markup=true,  label=difficultylabel,  }},
							T.column{ T.image{  label=difficulty,  }},
						}}
					}},
					T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},
					T.row{ T.column{ horizontal_alignment="left", T.image{  label="icons/banner2-half.png"  }}},
					T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},

					-------------------------
					-- REWARDS
					-------------------------
					T.row{ T.column{
						horizontal_alignment="left",
						T.label{  use_markup=true,  label=goldlabel,  },
					}},
					-- New Recruit
					T.row{ T.column{
						vertical_alignment="top",
						horizontal_alignment="left",
						T.grid{ T.row{
							T.column{ T.label{  id="recruit0",  use_markup=true,  label=recruitlabel,  }},
							T.column{ T.image{  id="recruit1",  label=recruit1  }},
							T.column{ T.image{  id="recruit2",  label=recruit2  }},
							T.column{ T.image{  id="recruit3",  label=recruit3  }},
							T.column{ T.image{  id="recruit4",  label=recruit4  }},
						}},
					}},
					-- Other (place this in the middle so that there's less margin between Recruit/Other; useful for Elensefar where you get both a recruit and an item)
					T.row{ T.column{
						vertical_alignment="top",
						horizontal_alignment="left",
						T.label{  id="other0",  use_markup=true,  label=otherlabel  }
					}},
					-- New Companion
					T.row{ T.column{
						vertical_alignment="top",
						horizontal_alignment="left",
						T.grid{ T.row{
							T.column{ T.label{  id="companion0",  use_markup=true,  label=companionlabel,  }},
							T.column{ T.image{  id="companion1",  label=companion1  }},
							T.column{ T.image{  id="companion2",  label=companion2  }},
						}},
					}},
				}},
			}}}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='5000'> </span>"  }}},
			-------------------------
			-- BUTTONS
			-------------------------
			T.row{ T.column{ T.grid{ T.row{
				T.column{ T.grid{ T.row{
					T.column{ T.button{
						return_value=1, use_markup=true,
						label=_"Continue",
					}},
					T.column{ T.label{ label="    " }},
					T.column{ T.button{
						return_value=2, use_markup=true,
						label=_"Cancel",
					}},
				}}},
			}}}},
		}},
		T.column{ T.label{  use_markup=true,  label="<span size='40000'> </span>"  }},
	}}


	--###############################
	-- CREATE DIALOG
	--###############################
	local result = wesnoth.sync.evaluate_single(function()
		local button = gui.show_dialog(
			{-- dialog definition
				definition="menu",
				T.helptip{ id="tooltip_large" }, -- mandatory field
				T.tooltip{ id="tooltip_large" }, -- mandatory field
				grid,
			},
			-- preshow
			function(dialog)
				if (not cfg.recruit1) then
					dialog["recruit0"].visible = false
					dialog["recruit1"].visible = false
					dialog["recruit2"].visible = false
					dialog["recruit3"].visible = false
					dialog["recruit4"].visible = false
				end
				if (not cfg.companion1) then
					dialog["companion0"].visible = false
					dialog["companion1"].visible = false
					dialog["companion2"].visible = false
				end
			end
		)
		return { button=button }
	end)
	-- e.g `status_s01 = "in progress"`
	if (result.button==1) then wml.variables["status_"..cfg.scenario]="in_progress" end
end



















--###########################################################################################################################################################
--                                                                 OVERWORLD TUTORIAL
--###########################################################################################################################################################
function wesnoth.wml_actions.display_overworld_tutorial()
	--###############################
	-- DEFINE GRID
	--###############################
	local grid = T.grid{ T.row{
		T.column{ border="right,left,bottom", border_size=18, T.grid{
			-------------------------
			-- INTRO
			-------------------------
			T.row{ T.column{ T.image{  label="icons/banner3-narrow.png"  }}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='5000'> </span>"  }}},
			T.row{ T.column{
				horizontal_alignment="center",
				T.label{  definition="title",  label=_"Welcome to the Great Continent",  }
			}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},
			T.row{ T.column{
				horizontal_alignment="left",
				border="right,left", border_size=18,
				T.label{
					use_markup=true,
					label=_"The Great Continent is a place of many possibilities.\nWhere shall you go? What shall you do?",
				}
			}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},
			-------------------------
			-- IMAGE
			-------------------------
			T.row{ T.column{ T.grid{ T.row{
				T.column{
					T.image{  label="bigmap/overworld-tutorial.png"  }
				},
				T.column{ T.label{  use_markup=true,  label="<span size='40000'> </span>"  }},
				T.column{
					horizontal_alignment="left",
					T.label{
						use_markup=true,
						label=_"To leave the overworld and play\na battle scenario, <i><b>move Konrad\nto one of the marked hexes.</b></i>",
					}
				}
			}}}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='9000'> </span>"  }}},
			T.row{ T.column {T.image{  label="icons/banner2-narrow.png"  }}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='9000'> </span>"  }}},
			-------------------------
			-- BUTTONS
			-------------------------
			T.row{ T.column{ T.button{
				return_value=1, use_markup=true,
				label=_"Understood",
			}}},
		}},
	}}

	--###############################
	-- CREATE DIALOG
	--###############################
	local result = wesnoth.sync.evaluate_single(function()
		local button = gui.show_dialog({
			definition="menu",
			T.helptip{ id="tooltip_large" }, -- mandatory field
			T.tooltip{ id="tooltip_large" }, -- mandatory field
			grid
		})
		return { button=button }
	end)
end



--###########################################################################################################################################################
--                                                                    LISAR TUTORIAL
--###########################################################################################################################################################
function wesnoth.wml_actions.display_lisar_tutorial()
	--###############################
	-- DEFINE GRID
	--###############################
	local grid = T.grid{ T.row{
		T.column{ border="right,left,bottom", border_size=18, T.grid{
			-------------------------
			-- INTRO
			-------------------------
			T.row{ T.column{ T.image{  label="icons/banner3-narrow.png"  }}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='5000'> </span>"  }}},
			T.row{ T.column{
				horizontal_alignment="center",
				T.label{  definition="title",  label=_"Li’sar’s Army",  }
			}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},
			-------------------------
			-- IMAGE
			-------------------------
			T.row{ T.column{ T.grid{ T.row{
				T.column{
					T.image{  label="bigmap/lisar-tutorial.png"  }
				},
				T.column{ T.label{  use_markup=true,  label="<span size='50000'> </span>"  }},
				T.column{
					horizontal_alignment="left",
					T.label{
						use_markup=true,
						label=_"Li’sar and her army have been greatly weakened\nfrom their trial underground, but nonetheless\nremain a significant presence on the battlefield.\n\nThe princess’s army is under AI control by default,\nwith a bonus to income. <b>At the start of each scenario\nyou will have the option to instead assume direct\ncontrol over her units.</b>\n\nEither way, Li’sar will have normal gold carryover\nand a normal early finish bonus.",
					}
				}
			}}}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='9000'> </span>"  }}},
			T.row{ T.column {T.image{  label="icons/banner2-narrow.png"  }}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='9000'> </span>"  }}},
			-------------------------
			-- BUTTONS
			-------------------------
			T.row{ T.column{ T.button{
				return_value=1, use_markup=true,
				label=_"Understood",
			}}},
		}},
	}}

	--###############################
	-- CREATE DIALOG
	--###############################
	local result = wesnoth.sync.evaluate_single(function()
		local button = gui.show_dialog({
			definition="menu",
			T.helptip{ id="tooltip_large" }, -- mandatory field
			T.tooltip{ id="tooltip_large" }, -- mandatory field
			grid
		})
		return { button=button }
	end)
end












--###########################################################################################################################################################
--                                                                 SELECT LINTANIR BOON
--###########################################################################################################################################################
function wesnoth.wml_actions.select_lintanir_boon(cfg)
	--###############################
	-- DEFINE GRID
	--###############################
	local grid = T.grid{ T.row{
		T.column{ T.label{  use_markup=true,  label="<span size='40000'> </span>"  }},
		T.column{ border="bottom", border_size=18, T.grid{
			-------------------------
			-- TITLE
			-------------------------
			T.row{ T.column{ T.image{  label="icons/banner3.png~CROP(50,0,400,22)"  }}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='8000'> </span>"  }}},
			T.row{ T.column{
				horizontal_alignment="center",
				T.label{  definition="title",  label=_"Select an item to learn more..."  }
			}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},

			-------------------------
			-- PORTRAITS
			-------------------------
			T.row{ T.column{ T.horizontal_listbox{
				id="boons",
				has_minimum=false,
				T.list_definition{ T.row{ T.column{ T.toggle_panel{ T.grid{
					T.row{ T.column{
						border="all", border_size=5,
						T.image{ id="image" }
					}},
					T.row{ T.column{
						border="all", border_size=5,
						T.label{ id="name", use_markup=true, text_alignment="center" }
					}},
				}}}}},
				T.list_data{
					T.row{ T.column{
						T.widget{  id="image", label="misc/blank-hex.png~SCALE(120,100)~BLIT(icons/book.png~SCALE(76,76),22,22)"           },
						T.widget{  id="name",  label=stringx.join("", {"<span size='large' face='WesScript'>",_"Wose Lore","</span>"})     },
					}},
					T.row{ T.column{
						T.widget{  id="image", label="misc/blank-hex.png~SCALE(120,100)~BLIT(attacks/sword-flaming.png~SCALE(76,76),22,22)"},
						T.widget{  id="name",  label=stringx.join("", {"<span size='large' face='WesScript'>",_"Flaming Sword","</span>" })},
					}},
					T.row{ T.column{
						T.widget{  id="image", label="misc/blank-hex.png~SCALE(120,100)~BLIT(icons/breastplate2.png~SCALE(76,76),22,22)"   },
						T.widget{  id="name",  label=stringx.join("", {"<span size='large' face='WesScript'>",_"Void Armor","</span>"})    },
					}},
				}
			}}},

			-------------------------
			-- LEAVE WITH A COMPANION
			-------------------------
			T.row{ T.column{ T.grid{
				T.row{
					T.column{ T.button{ id="take_lore",  return_value=1, label=_"Take"}},
					T.column{ T.button{ id="take_sword", return_value=2, label=_"Take"}},
					T.column{ T.button{ id="take_armor", return_value=3, label=_"Take"}},
				},
				-- this second row helps keep the table's formatting the same, even when 1+ buttons are hidden
				T.row{
					T.column{ T.spacer{ width=132 }},
					T.column{ T.spacer{ width=132 }},
					T.column{ T.spacer{ width=132 }},
				}
			}}},
		}},
		T.column{ T.label{  use_markup=true,  label="<span size='80000'> </span>"  }},
	}}

	--###############################
	-- SHOW/HIDE SELECTION BUTTONS
	--###############################
	local function preshow(dialog)
		if (not wml.variables['asked_lore' ]) then dialog["take_lore" ].visible=false end
		if (not wml.variables['asked_sword']) then dialog["take_sword"].visible=false end
		if (not wml.variables['asked_armor']) then dialog["take_armor"].visible=false end

		dialog["boons"].on_modified = function()
			wesnoth.interface.skip_messages(false) -- each time the player picks an option, clear the skip_message flag
			if dialog["boons"].selected_index==1 then wesnoth.game_events.fire('ask_about_lore' ) end
			if dialog["boons"].selected_index==2 then wesnoth.game_events.fire('ask_about_sword') end
			if dialog["boons"].selected_index==3 then wesnoth.game_events.fire('ask_about_armor') end
			dialog:close()
		end
	end

	--###############################
	-- CREATE DIALOG
	--###############################
	local result = wesnoth.sync.evaluate_single(function()
		local button = gui.show_dialog({
			definition="menu",
			T.helptip{ id="tooltip_large" }, -- mandatory field
			T.tooltip{ id="tooltip_large" }, -- mandatory field
			grid
		}, preshow)

		wesnoth.interface.skip_messages(false) -- each time the player picks an option, clear the skip_message flag
		if     button==1 then wesnoth.game_events.fire('take_lore')
		elseif button==2 then wesnoth.game_events.fire('take_sword')
		elseif button==3 then wesnoth.game_events.fire('take_armor')
		elseif button==-1 or button==-2 then
			-- if we close the dialog with enter or escape, open it back up. Wait for the player to select a companion.
			wesnoth.wml_actions.select_lintanir_boon()
		end
		return { button=button }
	end)
end















