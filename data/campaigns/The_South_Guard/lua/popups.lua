local _ = wesnoth.textdomain "wesnoth-tsg"

-- https://wiki.wesnoth.org/LuaAPI/types/widget

-- metatable for GUI tags
local T = wml.tag
























--###########################################################################################################################################################
--                                                                 SCENARIO PREVIEW
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
					label=_"Disable Tip Popups & Dialogue",
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
--                                                                      "MAIN"
--###########################################################################################################################################################
-------------------------
-- DEFINE WML TAGS
-------------------------
function wesnoth.wml_actions.display_tip(cfg)
	display_tip(cfg)
end


