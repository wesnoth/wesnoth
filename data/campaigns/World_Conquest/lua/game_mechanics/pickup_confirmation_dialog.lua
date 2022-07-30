local _ = wesnoth.textdomain 'wesnoth-wc'

local pickup_confirmation_dialog = {}

-- returns true when the item should be picked up.
local function show_dialog(unit, item_image)
	if wc2_utils.global_vars.skip_pickup_dialog then
		return 1
	end

	local dialog_wml = wml.load "campaigns/World_Conquest/gui/pickup_confirmation.cfg"
	local function preshow(dialog)
		dialog.unit_icon.label = wc2_color.tc_image(wesnoth.unit_types[unit.type].image)
		dialog.arrow.label = wc2_color.tc_text(" → ")
		dialog.item_icon.label = item_image
		dialog.res_yes:focus()
	end
	local res = gui.show_dialog(wml.get_child(dialog_wml, 'resolution'), preshow)
	return res == 1 or res == -1
end

-- returns true when the item should be picked up.
function pickup_confirmation_dialog.promt_synced(unit, item_image)
	local res = wesnoth.sync.evaluate_single("Item Pickup Choice", function()
		return { take_item = show_dialog(unit, item_image) }
	end)
	return res.take_item
end

return pickup_confirmation_dialog
