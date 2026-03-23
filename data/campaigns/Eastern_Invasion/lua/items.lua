
-- to make code shorter
local wml_actions = wesnoth.wml_actions

-- metatable for GUI tags
local T = wml.tag


-- from the Wesnoth Lua Pack
-- [item_dialog]
-- an alternative interface to pick items
-- could be used in place of [message] with [option] tags
function wml_actions.item_dialog( cfg )
	local function item_preshow(dialog)
		-- here set all widget starting values
		dialog.item_description.use_markup = true
		dialog.item_effect.use_markup = true
		dialog.item_name.label = cfg.name or ""
		dialog.image_name.label = cfg.image or ""
		dialog.item_description.label = cfg.description or ""
		dialog.item_effect.label = cfg.effect or ""
		dialog.take_button.label = cfg.take_string or wml.error( "Missing take_string= key in [item_dialog]" )
		dialog.leave_button.label = cfg.leave_string or wml.error( "Missing leave_string= key in [item_dialog]" )
	end

	local function sync()
		local dialog_cfg = wml.load "campaigns/Eastern_Invasion/gui/item_dialog.cfg"
		local dialog_wml = wml.get_child(dialog_cfg, 'resolution')
		local return_value = gui.show_dialog( dialog_wml, item_preshow )
		return { return_value = return_value }
	end

	local return_table = wesnoth.sync.evaluate_single(sync)
	if return_table.return_value == 1 or return_table.return_value == -1 then
		wml.variables[cfg.variable or "item_picked"] = "yes"
	else wml.variables[cfg.variable or "item_picked"] = "no"
	end
end

-- a variation for when you are required to take the item
function wml_actions.item_dialog_musttake( cfg )
	local function item_preshow(dialog)
		-- here set all widget starting values
		dialog.item_description.use_markup = true
		dialog.item_effect.use_markup = true
		dialog.item_name.label = cfg.name or ""
		dialog.image_name.label = cfg.image or ""
		dialog.item_description.label = cfg.description or ""
		dialog.item_effect.label = cfg.effect or ""
		dialog.take_button.label = cfg.take_string or wml.error( "Missing take_string= key in [item_dialog]" )
	end

	local function sync()
		local dialog_cfg = wml.load "campaigns/Eastern_Invasion/gui/item_dialog_musttake.cfg"
		local dialog_wml = wml.get_child(dialog_cfg, 'resolution')
		local return_value = gui.show_dialog( dialog_wml, item_preshow )
		return { return_value = return_value }
	end

	local return_table = wesnoth.sync.evaluate_single(sync)
	if return_table.return_value == 1 or return_table.return_value == -1 then
		wml.variables[cfg.variable or "item_picked"] = "yes"
	else wml.variables[cfg.variable or "item_picked"] = "no"
	end
end
