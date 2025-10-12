local _ = wesnoth.textdomain "wesnoth-tsg"

-- https://wiki.wesnoth.org/LuaAPI/types/widget

-- metatable for GUI tags
local T = wml.tag
























--###########################################################################################################################################################
--                                                                     DISPLAY TIP
--###########################################################################################################################################################
function display_tip(cfg)
	local tutor_title = cfg.title
	local tutor_message = cfg.message
	local tutor_image = cfg.image

	--###############################
	-- DEFINE GRID
	--###############################
	local grid = T.grid{ T.row{
		T.column{ T.label{  use_markup=true,  label="<span size='40000'> </span>"  }},
		T.column{ border="right,left,bottom", border_size=18, T.grid{
			-------------------------
			-- TITLE
			-------------------------
			T.row{ T.column{ T.image{  label="icons/banner3.png"  }}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='8000'> </span>"  }}},
			T.row{ T.column{
				horizontal_alignment="center",
				T.label{  definition="title",  label=_"Tip: "..tutor_title,  }
			}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},
			-------------------------
			-- INFO
			-------------------------
			T.row{ T.column{ T.grid{ T.row{
				T.column{
					horizontal_alignment="left",
					T.label{
						use_markup=true,
						label=tutor_message,
					}
				},
				T.column{ T.label{  use_markup=true,  label="<span size='80000'> </span>"  }},
				T.column{
					T.image{  label=tutor_image  }
				},
			}}}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},
			T.row{ T.column {T.image{  label="icons/banner2.png"  }}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},
			-------------------------
			-- BUTTONS
			-------------------------
			T.row{T.column{ T.grid{ T.row{
				T.column{ T.button{
					return_value=1, use_markup=true,
					label=_"Understood",
				}},
				T.column{ T.label{  use_markup=true,  label="<span size='15000'>     </span>"  }},
				T.column{ T.button{
					return_value=2, use_markup=true,
					label=_"Disable Tip Popups &amp; Dialogue",
				}},
			}}}},
		}},
		T.column{ T.label{  use_markup=true,  label="<span size='40000'> </span>"  }},
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
		if (button==2) then wml.variables['enable_tutorial_elements']='no' end
		return { button=button }
	end)
end














--###########################################################################################################################################################
--                                                                 CHOOSE COMPANION
--###########################################################################################################################################################
function choose_companion(cfg)
	--###############################
	-- DEFINE GRID
	--###############################
	local grid = T.grid{ T.row{
		T.column{ T.label{  use_markup=true,  label="<span size='40000'> </span>"  }},
		T.column{ border="right,left,bottom", border_size=18, T.grid{
			-------------------------
			-- TITLE
			-------------------------
			T.row{ T.column{ T.image{  label="icons/banner3.png"  }}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='8000'> </span>"  }}},
			T.row{ T.column{
				horizontal_alignment="center",
				T.label{  definition="title",  label=_"Choose an officer to accompany Deoran:"  }
			}},
			T.row{ T.column{ T.label{  use_markup=true,  label="<span size='15000'> </span>"  }}},

			-------------------------
			-- PORTRAITS
			-------------------------
			T.row{ T.column{ T.horizontal_listbox{
				id="characters",
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
						T.widget{  id="image", label="portraits/sir-gerrick.webp~SCALE(250,278)~CROP(0,58,250,220)"                                                  },
						T.widget{  id="name",  label=stringx.join("", {"<span size='x-large' face='WesScript'>",_"Sir Gerrick","</span>\n",_"Veteran Infantryman"})  },
					}},
					T.row{ T.column{
						T.widget{  id="image", label="portraits/mari.webp~SCALE(       250,250)~CROP(0, 0,250,220)"                                                  },
						T.widget{  id="name",  label=stringx.join("", {"<span size='x-large' face='WesScript'>",_"Captain Mari","</span>\n",_"Veteran Fencer"})      },
					}},
					T.row{ T.column{
						T.widget{  id="image", label="portraits/hylas.webp~SCALE(      250,250)~CROP(0, 0,250,220)"                                                  },
						T.widget{  id="name",  label=stringx.join("", {"<span size='x-large' face='WesScript'>",_"Minister Hylas","</span>\n",_"White Mage"})        },
					}},
				}
			}}},

			-------------------------
			-- LEAVE WITH A COMPANION
			-------------------------
			T.row{ T.column{ T.grid{
				T.row{
					T.column{ T.button{ id="leave_with_gerrick", return_value=1, label=_"Leave with Sir Gerrick"    }},
					T.column{ T.button{ id="leave_with_mari",    return_value=2, label=_"Leave with Captain Mari"   }},
					T.column{ T.button{ id="leave_with_hylas",   return_value=3, label=_"Leave with Minister Hylas" }},
				},
				-- this second row helps keep the table's formatting the same, even when 1+ buttons are hidden
				T.row{
					T.column{ T.spacer{ width=260 }},
					T.column{ T.spacer{ width=260 }},
					T.column{ T.spacer{ width=260 }},
				}
			}}},
		}},
		T.column{ T.label{  use_markup=true,  label="<span size='80000'> </span>"  }},
	}}

	--###############################
	-- SHOW/HIDE SELECTION BUTTONS
	--###############################
	local function preshow(dialog)
		if (not wml.variables['asked_gerrick']) then dialog["leave_with_gerrick"].visible=false end
		if (not wml.variables['asked_mari']   ) then dialog["leave_with_mari"   ].visible=false end
		if (not wml.variables['asked_hylas']  ) then dialog["leave_with_hylas"  ].visible=false end

		dialog["characters"].on_modified = function()
			wesnoth.interface.skip_messages(false) -- each time the player picks an option, clear the skip_message flag
			if dialog["characters"].selected_index==1 then wesnoth.game_events.fire('ask_about_gerrick' ) end
			if dialog["characters"].selected_index==2 then wesnoth.game_events.fire('ask_about_mari'    ) end
			if dialog["characters"].selected_index==3 then wesnoth.game_events.fire('ask_about_hylas'   ) end
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
		if     button==1 then wesnoth.game_events.fire('leave_with_gerrick')
		elseif button==2 then wesnoth.game_events.fire('leave_with_mari')
		elseif button==3 then wesnoth.game_events.fire('leave_with_hylas')
		elseif button==-1 or button==-2 then
			-- if we close the dialog with enter or escape, open it back up. Wait for the player to select a companion.
			wesnoth.wml_actions.choose_companion()
		end
		return { button=button }
	end)
end
















--###########################################################################################################################################################
--                                                                      "MAIN"
--###########################################################################################################################################################
-------------------------
-- DEFINE WML TAGS
-------------------------
function wesnoth.wml_actions.display_tip(cfg)
	display_tip(cfg)
end
function wesnoth.wml_actions.choose_companion(cfg)
	choose_companion(cfg)
end
