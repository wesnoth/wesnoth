local _ = wesnoth.textdomain 'wesnoth-wc'
local T = wml.tag

local invest_tellunit = {}

invest_tellunit.dialog_wml = wml.load "campaigns/World_Conquest/gui/invest_tellunit.cfg"

-- todo:maybe use a dialog ith the unit specific welcome mesage?
function invest_tellunit.execute(unit_type)
	if type(unit_type) == "string" then
		unit_type = wesnoth.unit_types[unit_type]
	end
	if not wesnoth.sides[wesnoth.current.side].is_local then
		return
	end
	local function preshow(dialog)
		dialog.icon.label = wc2_color.tc_image(unit_type.image)
		dialog.name.label = unit_type.name
	end

	gui.show_dialog(wml.get_child(invest_tellunit.dialog_wml, 'resolution'), preshow)
end

return invest_tellunit
