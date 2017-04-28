-- #textdomain wesnoth-tutorial

-- Allows the player to choose whether they want to play Konrad or Li’sar
-- for the tutorial

local helper = wesnoth.require "lua/helper.lua"
local T = helper.set_wml_tag_metatable {}
local wml_actions = wesnoth.wml_actions
local _ = wesnoth.textdomain "wesnoth-tutorial"

function wml_actions.select_character()
	local character_selection_dialog = {
		maximum_height = 250,
		maximum_width = 400,
		T.helptip { id="tooltip_large" }, -- mandatory field
		T.tooltip { id="tooltip_large" }, -- mandatory field
		T.grid {
			T.row {
				T.column {
					grow_factor = 1,
					border = "all",
					border_size = 5,
					horizontal_alignment = "left",
					T.label {
						definition = "title",
						label = _"Select Character"
					}
				}
			},
			T.row {
				T.column {
					grow_factor = 1,
					border = "all",
					border_size = 5,
					horizontal_alignment = "left",
					T.label {
						label = _"Who do you want to play?"
					}
				}
			},
			T.row {
				T.column {
					T.grid {
						T.row {
							T.column {
								grow_factor = 1,
								border = "all",
								border_size = 5,
								T.image {
									label = "units/konrad-fighter.png"
								}
							},
							T.column {
								grow_factor = 1,
								border = "all",
								border_size = 5,
								T.image {
									label = "units/human-princess.png~TC(1,magenta)"
								}
							}
						},
						T.row {
							T.column {
								grow_factor = 1,
								border = "all",
								border_size = 5,
								T.button {
									label = _"Konrad",
									return_value = 1
								}
							},
							T.column {
								grow_factor = 1,
								border = "all",
								border_size = 5,
								T.button {
									label = _"Li’sar",
									return_value = 2
								}
							}
						}
					}
				}
			}
		}
	}

	local character = wesnoth.show_dialog(character_selection_dialog)
	local unit = wesnoth.get_variable("student_store")

	if character == 2 then
		wesnoth.put_unit({
			type = "Fighteress",
			id = unit.id,
			name = _"Li’sar",
			unrenamable = true,
			profile = "portraits/lisar.png",
			canrecruit = true,
			facing = unit.facing,
		}, unit.x, unit.y )
	else
		wesnoth.put_unit(unit)
	end

	wesnoth.redraw {}
end
