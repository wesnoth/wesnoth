local _ = wesnoth.textdomain 'wesnoth-World_Conquest'
local T = wml.tag

local invest_tellunit = {}

invest_tellunit.dialog_wml = {
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
					label = _"You got",
					id = "title"
				}
			}
		},
		T.row {
			T.column {
				T.grid {
					T.row {
						T.column {
							T.image {
								id="icon"
							}
						},
					},
					T.row {
						T.column {
							T.label {
								id="name"
							}
						},
					},
				},
			},
		},
		T.row {
			T.column {
				T.button {
					label = _"Ok",
					id = "ok",
				},
			},
		},
	},
}

-- todo:maybe use a dialog ith the unit specific welcome mesage?
function invest_tellunit.execute(unit_type)
	if type(unit_type) == "string" then
		unit_type = wesnoth.unit_types[unit_type]
	end
	if not wesnoth.sides[wesnoth.current.side].is_local then
		return
	end
	local function preshow()
		wesnoth.set_dialog_value(wc2_color.tc_image(unit_type.image), "icon")
		wesnoth.set_dialog_value(unit_type.name, "name")
	end

	wesnoth.show_dialog(invest_tellunit.dialog_wml, preshow)
end

return invest_tellunit
