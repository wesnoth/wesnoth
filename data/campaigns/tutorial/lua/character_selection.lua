-- #textdomain wesnoth-tutorial

-- Allows the player to choose whether they want to play Konrad, Li’sar,
-- Kaleh or Nym for the tutorial

local T = wml.tag
local wml_actions = wesnoth.wml_actions
local _ = wesnoth.textdomain "wesnoth-tutorial"

function wml_actions.select_character()
	local character_selection_dialog = {
		maximum_height = 250,
		maximum_width = 600,
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
									label = "units/kaleh.png"
								}
							},
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
							},
							T.column {
								grow_factor = 1,
								border = "all",
								border_size = 5,
								T.image {
									label = "units/nym.png"
								}
							}
						},
						T.row {
							T.column {
								grow_factor = 1,
								border = "all",
								border_size = 5,
								T.button {
									label = _"Kaleh",
									return_value = 3
								}
							},
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
							},
							T.column {
								grow_factor = 1,
								border = "all",
								border_size = 5,
								T.button {
									label = _"Nym",
									return_value = 4
								}
							}
						}
					}
				}
			}
		}
	}

	local character = wesnoth.show_dialog(character_selection_dialog)
	local unit = wml.variables.student_store

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
		wesnoth.sides[1].side_name = _"Li’sar"
		-- enable the help to display this unit's page
		wesnoth.add_known_unit("Fighteress")
	elseif character == 3 then
		wesnoth.put_unit({
			type = "Tutorial_Quenoth_Fighter",
			id = unit.id,
			name = _"Kaleh",
			unrenamable = true,
			profile = "portraits/kaleh.png",
			canrecruit = true,
			facing = unit.facing,
		}, unit.x, unit.y )
		wesnoth.sides[1].side_name = _"Kaleh"
		-- enable the help to display this unit's page
		wesnoth.add_known_unit("Tutorial_Quenoth_Fighter")
	elseif character == 4 then
		wesnoth.put_unit({
			type = "Tutorial_Quenoth_Fighteress",
			id = unit.id,
			name = _"Nym",
			unrenamable = true,
			profile = "portraits/nym_no_bow.png",
			canrecruit = true,
			facing = unit.facing,
		}, unit.x, unit.y )
		wesnoth.sides[1].side_name = _"Nym"
		-- enable the help to display this unit's page
		wesnoth.add_known_unit("Tutorial_Quenoth_Fighteress")
	else
		wesnoth.put_unit(unit)
	end

	wml.variables.student_name = wesnoth.sides[1].side_name

	wesnoth.redraw {}
end
