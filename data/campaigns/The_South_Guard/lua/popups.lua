local _ = wesnoth.textdomain "wesnoth-tsg"

-- https://wiki.wesnoth.org/LuaAPI/types/widget

-- metatable for GUI tags
local T = wml.tag
























--###########################################################################################################################################################
--                                                                     DISPLAY TIP
--###########################################################################################################################################################
function display_tip(cfg)
	--###############################
	-- CREATE DIALOG
	--###############################
	local result = wesnoth.sync.evaluate_single(function()
		local dialog_cfg = wml.load "campaigns/The_South_Guard/gui/display_tip.cfg"
		local dialog_wml = wml.get_child(dialog_cfg, 'resolution')
		local button = gui.show_dialog(dialog_wml, function(dialog)
			dialog["title"        ].label = _"Tip: "..cfg.title
			dialog["tutor_message"].label = cfg.message
			dialog["tutor_image"  ].label = cfg.image
		end)
		if (button==2) then wml.variables['enable_tutorial_elements']='no' end
		return { button=button }
	end)
end














--###########################################################################################################################################################
--                                                                 CHOOSE COMPANION
--###########################################################################################################################################################
function choose_companion(cfg)
	--###############################
	-- SHOW/HIDE SELECTION BUTTONS
	--###############################
	local function preshow(dialog)
		if (not wml.variables['asked_gerrick']) then dialog["leave_with_gerrick"].visible=false end
		if (not wml.variables['asked_mari']   ) then dialog["leave_with_mari"   ].visible=false end
		if (not wml.variables['asked_hylas']  ) then dialog["leave_with_hylas"  ].visible=false end

		dialog["characters"].on_modified = function()
			wesnoth.interface.skip_messages(false) -- each time the player picks an option, clear the skip_message flag
			if dialog["characters"].selected_index==1 then wesnoth.game_events.fire('ask_about_gerrick' ) end
			if dialog["characters"].selected_index==2 then wesnoth.game_events.fire('ask_about_mari'    ) end
			if dialog["characters"].selected_index==3 then wesnoth.game_events.fire('ask_about_hylas'   ) end
			dialog:close()
		end
	end

	--###############################
	-- CREATE DIALOG
	--###############################
	local result = wesnoth.sync.evaluate_single(function()
		local dialog_cfg = wml.load "campaigns/The_South_Guard/gui/choose_companion.cfg"
		local dialog_wml = wml.get_child(dialog_cfg, 'resolution')
		local button = gui.show_dialog(dialog_wml, preshow)

		wesnoth.interface.skip_messages(false) -- each time the player picks an option, clear the skip_message flag
		if     button==1 then wesnoth.game_events.fire('leave_with_gerrick')
		elseif button==2 then wesnoth.game_events.fire('leave_with_mari')
		elseif button==3 then wesnoth.game_events.fire('leave_with_hylas')
		elseif button==-1 or button==-2 then
			-- if we close the dialog with enter or escape, open it back up. Wait for the player to select a companion.
			wesnoth.wml_actions.choose_companion()
		end
		return { button=button }
	end)
end
















--###########################################################################################################################################################
--                                                                      "MAIN"
--###########################################################################################################################################################
-------------------------
-- DEFINE WML TAGS
-------------------------
function wesnoth.wml_actions.display_tip(cfg)
	display_tip(cfg)
end
function wesnoth.wml_actions.choose_companion(cfg)
	choose_companion(cfg)
end
