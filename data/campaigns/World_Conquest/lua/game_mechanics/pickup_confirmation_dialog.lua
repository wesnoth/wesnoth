local _ = wesnoth.textdomain 'wesnoth-World_Conquest'
local T = wml.tag

local pickup_confirmation_dialog = {}

-- returns true when the item should be picked up.
local function show_dialog(unit, item_image)
	if wc2_utils.global_vars.skip_pickup_dialog then
		return 1
	end

	local dialog_wml = {
		maximum_width = 1200,
		maximum_height = 700,
		T.helptip { id = "tooltip_large" }, -- mandatory field
		T.tooltip { id = "tooltip_large" }, -- mandatory field

		T.grid {
			T.row {
				T.column {
					border = "all",
					border_size = 5,
					horizontal_alignment = "left",
					T.label {
						definition = "title",
						label = _"Pick up item?",
						id = "title",
					},
				},
			},
			T.row {
				T.column {
					T.grid {
						T.row {
							T.column {
								T.image {
									id="item_icon",
								},
							},
							T.column {
								T.label {
									id="arrow",
									use_markup=true
								},
							},
							T.column {
								T.image {
									id="unit_icon",
								},
							},
						},
					},
				},
			},
			T.row {
				T.column {
					T.grid {
						T.row {
							T.column {
								T.button {
									label = _"Yes (enter)",
									id="res_yes",
									return_value = 1,
								},
							},
							T.column {
								T.button {
									label = _"No (esc)",
									id="res_no",
									return_value = 2
								},
							},
						},
					},
				},
			},
		},

	}
	local function preshow()
		wesnoth.set_dialog_value(wc2_color.tc_image(wesnoth.unit_types[unit.type].image), "unit_icon")
		wesnoth.set_dialog_value(wc2_color.tc_text(" → "), "arrow")
		wesnoth.set_dialog_value(item_image, "item_icon")
		wesnoth.set_dialog_focus("res_yes")
	end
	local res = wesnoth.show_dialog(dialog_wml, preshow)
	return res == 1 or res == -1
end

-- returns true when the item should be picked up.
function pickup_confirmation_dialog.promt_synced(unit, item_image)
	local res = wesnoth.synchronize_choice("Item Pickup Choice", function()
		return { take_item = show_dialog(unit, item_image) }
	end)
	return res.take_item
end

return pickup_confirmation_dialog
