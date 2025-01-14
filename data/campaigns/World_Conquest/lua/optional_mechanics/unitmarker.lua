local on_event = wesnoth.require("on_event")
local _ = wesnoth.textdomain 'wesnoth-wc'
local wc2_utils = wesnoth.require("./../game_mechanics/utils.lua")


local img_is_special_overlays = {
	"misc/is_special.png"
}
local img_is_special_menu = "misc/is_special.png~CROP(35,3,22,16)"

local strings = {
	special_overlay = _ "Special Overlay",
}

function wesnoth.effects.wc2_unitmarker(u, cfg)
	local number = math.min(cfg.number or 1, #img_is_special_overlays)

	u.variables["mods.wc2_unitmarker"] = number
	u:add_modification("object", {
		wml.tag.effect {
			apply_to = "overlay",
			add = img_is_special_overlays[number],
		}
	}, false)

end

function wesnoth.wml_actions.wc2_toggle_overlay(cfg)
	local units = wesnoth.units.find_on_map(cfg)
	for i, u in ipairs(units) do
		local overlay_nr = u.variables["mods.wc2_unitmarker"]
		if overlay_nr ~= nil then
			u:remove_modifications({ id = "wc2_unitmarker" })
		end
		if (overlay_nr or 0) < #img_is_special_overlays then
			overlay_nr = (overlay_nr or 0) + 1
			u:add_modification("object", {
				id = "wc2_unitmarker",
				wml.tag.effect {
					apply_to = "wc2_unitmarker",
					number = overlay_nr,
				}
			})
		end
	end
end

wc2_utils.menu_item {
	id = "2_WCT_Special_Overlay_Option",
	description = strings.special_overlay,
	image= img_is_special_menu,
	filter = function(x, y)
		if wml.variables.wc2_config_enable_unitmarker == false then
			return false
		end
		local u = wesnoth.units.get(x, y)
		return u and u.side == wesnoth.current.side
	end,
	handler = function(cx)
		wesnoth.wml_actions.wc2_toggle_overlay {
			x = cx.x1,
			y = cx.y1,
		}
		-- i coudl also unsync this instead
		wesnoth.allow_undo(false)
	end
}
