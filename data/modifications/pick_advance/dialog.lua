-- << pickadvance_dialog

pickadvance = {}
local T = wml.tag
local _ = wesnoth.textdomain "wesnoth"

function pickadvance.show_dialog_unsynchronized(advance_info, unit)
-- dialog exit codes --
	local cancel_code = -2
	local single_unit_code = -1
--
	local unit_type_options = advance_info.type_advances
	local options = {}
	for _, ut in ipairs(unit_type_options) do
		options[#options + 1] = wesnoth.unit_types[ut]
	end

	local unit_override_one = (advance_info.unit_override or {})[2] == nil
		and (advance_info.unit_override or {})[1] or nil
	local game_override_one = (advance_info.game_override or {})[2] == nil
		and (advance_info.game_override or {})[1] or nil

	local pick_advance_dialog = wml.load "modifications/pick_advance/gui/pick_advance.cfg"
	local dialog_wml = wml.get_child(pick_advance_dialog, 'resolution')

-- dialog preshow function
	local function preshow(window)
		window.apply_to_all.visible = not unit.canrecruit

		local selection = 0

		local empty_icon_unit = "misc/blank-hex.png"

		local null_row = window.the_list[1]
		null_row.the_icon.label = empty_icon_unit
		null_row.the_label.label = _ "No planned advancement"
		null_row.global_icon.visible = false

		for i, advance_type in ipairs(options) do
			local n = i + 1
			local text = advance_type.name
			if advance_type.id == game_override_one or advance_type.id == unit_override_one then
				selection = n
			end
			local this_row = window.the_list[n]
			this_row.the_label.label = text
			local img = advance_type.__cfg.image
			if img then
				img = ("%s~TC(%d,%s)"):format(img, unit.side, advance_type.__cfg.flag_rgb or "magenta")
			else
				img = empty_icon_unit
			end
			this_row.the_icon.label = img
			this_row.global_icon.visible = advance_type.id == game_override_one or "hidden"
		end

		window.the_list:focus()
		if selection > 0 then
			window.the_list.selected_index = selection
		end
	end

-- dialog postshow function
	local item_result
	local apply_to_all
	local function postshow(window)
		item_result = window.the_list.selected_index - 1
		apply_to_all = window.apply_to_all.selected
	end

	local dialog_exit_code = gui.show_dialog(dialog_wml, preshow, postshow)

	if dialog_exit_code == cancel_code then
		return { ignore = true }
	end

-- determine the choice made
	local is_reset = item_result == 0
	return {
		ignore = false,
		is_unit_override = not apply_to_all,
		unit_override = not is_reset and options[item_result].id or table.concat(unit_type_options, ","),

		is_game_override = apply_to_all,
		game_override = apply_to_all and (not is_reset and options[item_result].id) or nil,
	}
end

-- >>
