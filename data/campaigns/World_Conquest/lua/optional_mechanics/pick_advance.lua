-- The addons own 'pick your advances' mod. 
-- Works independed from the reset of the wc2 code (but needs wc2_utils.lua)
local on_event = wesnoth.require("on_event")
local wc2_utils = wesnoth.require("./../game_mechanics/utils.lua")
local _ = wesnoth.textdomain 'wesnoth-wc'

local pick_advance = {}

local strings = {
	presect_advacement = _ "Preset Advancement",
}

function pick_advance.has_options(u)
	return u and #u.advances_to > 1 and wml.variables.wc2_config_enable_pya
end

on_event("pre_advance", function(ec)
	local u = wesnoth.units.get(ec.x1, ec.y1)
	if not pick_advance.has_options(u) then
		return
	end
	local picked = u.variables.wc2_pya_pick
	u.variables.wc2_pya_pick = nil
	if picked ~= nil then
		u.advances_to = { picked }
	end
end)

function wesnoth.wml_actions.wc2_pya_set_pick(cfg)
	local u = wesnoth.units.get(cfg.x, cfg.y)
	u.variables.wc2_pya_pick = cfg.pick
end

function wesnoth.wml_actions.wc2_pya_pick(cfg)
	local u = wesnoth.units.get(cfg.x, cfg.y)
	if not pick_advance.has_options(u) then
		return
	end
	local picked = u.variables.wc2_pya_pick
	local options = u.advances_to
	local str_advancer_option = _ "Currently I’m set to advance towards: $name \n\nWhat are your new orders?"
	local current_name = picked and wesnoth.unit_types[picked].name or _"Random"
	local message_wml = {
		x=cfg.x,
		y=cfg.y,
		message= stringx.vformat(str_advancer_option, {name  = current_name}),
		wml.tag.option {
			label = _"Random",
			image = wc2_color.tc_image("units/unknown-unit.png"),
			wml.tag.command {
				wml.tag.wc2_pya_set_pick {
					x=cfg.x,
					y=cfg.y,
				}
			}
		}
	}
	for i,v in ipairs(options) do
		local ut = wesnoth.unit_types[v]
		table.insert(message_wml, wml.tag.option {
			label = ut.name,
			image = wc2_color.tc_image(ut.image),
			wml.tag.command {
				wml.tag.wc2_pya_set_pick {
					x=cfg.x,
					y=cfg.y,
					pick=v,
				}
			}
		})
	end
	wesnoth.wml_actions.message(message_wml)
end


wc2_utils.menu_item {
	id = "3_WCT_Preset_Advancement_Option",
	description = strings.presect_advacement,
	filter = function (x, y)
		if wml.variables.wc2_config_enable_pya == false then
			return false
		end
		local u = wesnoth.units.get(x, y)
		return u and u.side == wesnoth.current.side and pick_advance.has_options(u)
	end,
	handler = function(cx)
		wesnoth.wml_actions.wc2_pya_pick {
			x=cx.x1,
			y=cx.y1,
		}
		--todo: i guess i could also use on_undo here.
		wesnoth.allow_undo(false)
	end
}

return pick_advance
