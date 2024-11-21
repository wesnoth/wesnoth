-- #textdomain wesnoth-tutorial

-- Allows the player to choose whether they want to play Konrad or Li’sar
-- for the tutorial

local T = wml.tag
local wml_actions = wesnoth.wml_actions
local _ = wesnoth.textdomain "wesnoth-tutorial"

selected = 1

function pre_show(dialog)
	local list = dialog:find("characters")
	list.on_modified = function()
		selected = list.selected_index
		dialog:close()
	end
end

function wml_actions.select_character()
	local character_selection_dialog = wml.load "campaigns/tutorial/gui/character_selection.cfg"
	local dialog_wml = wml.get_child(character_selection_dialog, 'resolution')

	local result = wesnoth.sync.evaluate_single(function()
		return { value = gui.show_dialog(dialog_wml, pre_show, function() end) }
	end)
	local unit = wml.variables.student_store

	if selected == 2 then
		wesnoth.units.to_map({
			type = "Fighteress",
			side = 1,
			id = unit.id,
			name = _"Li’sar",
			unrenamable = true,
			profile = "portraits/lisar.webp",
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
