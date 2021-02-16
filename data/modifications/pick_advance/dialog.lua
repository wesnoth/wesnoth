-- << pickadvance_dialog

pickadvance = {}
local T = wesnoth.require("lua/helper.lua").set_wml_tag_metatable {}
local _ = wesnoth.textdomain "wesnoth"

local function filter_false(arr)
	local result = {}
	for _, v in ipairs(arr) do
		if v ~= false then
			result[#result + 1] = v
		end
	end
	return result
end


function pickadvance.show_dialog_unsynchronized(advance_info, unit)
-- dialog exit codes --
	local reset_code = -3
	local single_unit_code = -1
	local all_units_code = 1
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

	local description_row = T.row {
		T.column { T.label { use_markup = true, label = _"Plan advance:" } },
	}

	local list_sub_row = T.row {
			T.column { T.image { id = "the_icon" } },
			T.column { grow_factor = 0, T.label { use_markup = true, id = "the_label" } },
			T.column { grow_factor = 1, T.spacer {} },
		}

	local toggle_panel = T.toggle_panel { return_value = single_unit_code, T.grid { list_sub_row } }

	local list_definition = T.list_definition { T.row { T.column { horizontal_grow = true, toggle_panel } } }

	local listbox = T.listbox { id = "the_list", list_definition, has_minimum = true }

	local reset_button = T.button {
		return_value = reset_code,
		label = _"Reset",
		tooltip = _"Reset advancements to default"
	}
	local unit_button = T.button {
		return_value = single_unit_code,
		label = _"Save",
		tooltip = _"Save the advancement for this unit only"
	}
	local recruits_subbutton = T.button {
		return_value = all_units_code,
		label = _"Save (all)",
		tooltip = _"Save the advancement for all units of this type"
	}
	local recruits_button = not unit.canrecruit and T.row { T.column { horizontal_grow = true, recruits_subbutton } }

-- main dialog definition
	local dialog = {
		T.tooltip { id = "tooltip_large" },
		T.helptip { id = "tooltip_large" },
		T.grid(filter_false {
			T.row { T.column { T.spacer { width = 250 } } },
			description_row,
			T.row { T.column { horizontal_grow = true, listbox } },
			T.row { T.column { horizontal_grow = true, unit_button } },
			recruits_button,
			T.row { T.column { horizontal_grow = true, (unit_override_one or game_override_one) and reset_button or T.spacer { width = 250 } } },
		})
	}

-- dialog preshow function
	local function preshow()
		for i, advance_type in ipairs(options) do
			local text = advance_type.name
			if advance_type.id == game_override_one then
				text = text .. _" (chosen, all)"
			elseif advance_type.id == unit_override_one then
				text = text .. _" (chosen)"
			end
			wesnoth.set_dialog_value(text, "the_list", i, "the_label")
			local img = advance_type.__cfg.image
			wesnoth.set_dialog_value(img or "misc/blank-hex.png", "the_list", i, "the_icon")
		end

		wesnoth.set_dialog_focus("the_list")

		local function select()
			local i = wesnoth.get_dialog_value "the_list"
			if i > 0 then
				local img = options[i].__cfg.image
				wesnoth.set_dialog_value(img or "misc/blank-hex.png", "the_list", i, "the_icon")
			end
		end
		wesnoth.set_dialog_callback(select, "the_list")
	end

-- dialog postshow function
	local item_result
	local function postshow()
		item_result = wesnoth.get_dialog_value("the_list")
	end

	local dialog_exit_code = wesnoth.show_dialog(dialog, preshow, postshow)

-- determine the choice made
	local is_reset = dialog_exit_code == reset_code
	local is_ok = dialog_exit_code >= single_unit_code and item_result >= 1
	local game_scope = dialog_exit_code == all_units_code
	return {
		is_unit_override = is_reset or is_ok,
		unit_override = is_ok and options[item_result].id or is_reset and table.concat(unit_type_options, ","),

		is_game_override = is_reset or game_scope,
		game_override = game_scope and options[item_result].id or nil,
	}
end

-- >>
