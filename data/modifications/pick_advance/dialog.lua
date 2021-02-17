-- << pickadvance_dialog

pickadvance = {}
local T = wesnoth.require("lua/helper.lua").set_wml_tag_metatable {}
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

	local description_row = T.row {
		T.column {
			border = "all",
			border_size = 5,
			horizontal_alignment = "left",
			T.label {
				definition = "title",
				label = _ "Plan Advancement"
			}
		},
	}

	local list_row_definition = T.grid {
		T.row {
			T.column {
				border = "all",
				border_size = 5,
				grow_factor = 0,
				horizontal_alignment = "left",
				T.image {
					id = "the_icon",
					linked_group = "image"
				}
			},
			T.column {
				border = "all",
				border_size = 5,
				grow_factor = 1,
				horizontal_alignment = "left",
				T.label {
					use_markup = true,
					id = "the_label",
					linked_group = "type"
				}
			},
			T.column {
				border = "all",
				border_size = 5,
				horizontal_alignment = "center",
				vertical_alignment = "center",
				T.image {
					id = "global_icon",
					linked_group = "global_icon",
					label = "icons/action/editor-tool-unit_30-pressed.png",
					tooltip = _ "This advancement is currently the default for all units of the same type"
				}
			}
		}
	}

	local listbox = T.listbox {
		id = "the_list",
		has_minimum = true,
		T.list_definition {
			T.row {
				T.column {
					horizontal_grow = true,
					vertical_grow = true,
					T.toggle_panel {
						return_value = single_unit_code,
						list_row_definition
					}
				}
			}
		}
	}

-- main dialog definition
	local dialog = {
		T.tooltip {
			id = "tooltip"
		},
		T.helptip {
			id = "tooltip"
		},
		T.linked_group {
			id = "image",
			fixed_width = true
		},
		T.linked_group {
			id = "type",
			fixed_width = true
		},
		T.linked_group {
			id = "global_icon",
			fixed_width = true
		},
		T.grid {
			description_row,
			T.row {
				grow_factor = 1,
				T.column {
					border = "all",
					border_size = 5,
					horizontal_grow = true,
					listbox
				}
			},
			T.row {
				grow_factor = 0,
				T.column {
					border = "all",
					border_size = 5,
					horizontal_alignment = "left",
					T.toggle_button {
						id = "apply_to_all",
						label = _ "Apply to all units of this type"
					}
				}
			},
			T.row {
				T.column {
					horizontal_alignment = "right",
					T.grid {
						T.row {
							grow_factor = 0,
							T.column {
								border = "all",
								border_size = 5,
								horizontal_alignment = "right",
								T.button {
									return_value = single_unit_code,
									label = _ "Save"
								}
							},
							T.column {
								border = "all",
								border_size = 5,
								horizontal_alignment = "right",
								T.button {
									id = "cancel",
									label = _ "Cancel"
								}
							}
						}
					}
				}
			}
		}
	}

-- dialog preshow function
	local function preshow()
		wesnoth.set_dialog_visible(not unit.canrecruit, "apply_to_all")

		local selection = 0

		local empty_icon_unit = "misc/blank-hex.png"

		wesnoth.set_dialog_value(empty_icon_unit, "the_list", 1, "the_icon")
		wesnoth.set_dialog_value( _ "No planned advancement", "the_list", 1, "the_label")
		wesnoth.set_dialog_visible(false, "the_list", 1, "global_icon")

		for i, advance_type in ipairs(options) do
			local n = i + 1
			local text = advance_type.name
			if advance_type.id == game_override_one or advance_type.id == unit_override_one then
				selection = n
			end
			wesnoth.set_dialog_value(text, "the_list", n, "the_label")
			local img = advance_type.__cfg.image
			if img then
				img = ("%s~TC(%d,%s)"):format(img, unit.side, advance_type.__cfg.flag_rgb or "magenta")
			else
				img = empty_icon_unit
			end
			wesnoth.set_dialog_value(img, "the_list", n, "the_icon")
			wesnoth.set_dialog_visible(not not (advance_type.id == game_override_one) or "hidden", "the_list", n, "global_icon")
		end

		wesnoth.set_dialog_focus("the_list")
		if selection > 0 then
			wesnoth.set_dialog_value(selection, "the_list")
		end
	end

-- dialog postshow function
	local item_result
	local apply_to_all
	local function postshow()
		item_result = wesnoth.get_dialog_value("the_list") - 1
		apply_to_all = wesnoth.get_dialog_value("apply_to_all")
	end

	local dialog_exit_code = wesnoth.show_dialog(dialog, preshow, postshow)

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
