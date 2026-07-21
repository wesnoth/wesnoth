local _ = wesnoth.textdomain "wesnoth-tsg"

-- metatable for GUI tags
local T = wml.tag

--###########################################################################################################################################################
--                                                                 SCENARIO PREVIEW
--###########################################################################################################################################################
function display_tip(cfg)
	local tutor_title = cfg.title
	local tutor_message = cfg.message

	--###############################
	-- CREATE DIALOG
	--###############################
	local result = wesnoth.sync.evaluate_single(function()
		local dialog_cfg = wml.load "campaigns/Of_Pearls_and_Pirates/gui/display_tip.cfg"
		local dialog_wml = wml.get_child(dialog_cfg, 'resolution')
		local button = gui.show_dialog(dialog_wml, function(dialog)
			dialog["title"        ].label = _"Tip: "..cfg.title
			dialog["tutor_message"].label = cfg.message
		end)
		if (button==2) then wml.variables['enable_tutorial_elements']='no' end
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
