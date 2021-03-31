-- #textdomain wesnoth-tutorial

-- Allows the player to choose whether they want to play Konrad or Li’sar
-- for the tutorial

local T = wml.tag
local wml_actions = wesnoth.wml_actions
local _ = wesnoth.textdomain "wesnoth-tutorial"

function wml_actions.select_character()
	local character_selection_dialog = wml.load "campaigns/tutorial/gui/character_selection.cfg"
	local dialog_wml = wml.get_child(character_selection_dialog, 'resolution')

	local character = gui.show_dialog(dialog_wml)
	local unit = wml.variables.student_store

	if character == 2 then
		wesnoth.units.to_map({
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
	else
		wesnoth.units.to_map(unit)
	end

	wesnoth.redraw {}
end
