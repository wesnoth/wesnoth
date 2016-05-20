
local helper = wesnoth.require("lua/helper.lua")
local _ = wesnoth.textdomain 'wesnoth-help'
local T = helper.set_wml_tag_metatable {}

-- Code taken form 2p_dark_forecast.lua
-- TODO: maybe put it in a place where everyone can use it?
local event_handlers = {}
local old_on_event = wesnoth.game_events.on_event or function(eventname) end
wesnoth.game_events.on_event = function(eventname)
	for k,v in pairs(event_handlers[eventname] or {}) do
		v()
	end
	old_on_event(eventname)
end

local function on_event(eventname, handler)
	eventname = string.gsub(eventname, " ", "_")
	event_handlers[eventname] = event_handlers[eventname] or {}
	table.insert(event_handlers[eventname], handler)
end
-- end code taken from dark forecast.


-- The feeding event code
on_event("die", function()
	local ec = wesnoth.current.event_context
	local u_killer = wesnoth.get_unit(ec.x2, ec.y2)
	local u_victim = wesnoth.get_unit(ec.x1, ec.y1)

	if not u_killer or not u_killer:matches { ability = "feeding" } then
		return
	end
	if not u_victim or u_victim:matches { status = "unplagueable" } then
		return
	end
	local u_killer_cfg = u_killer.__cfg
	for i,v in ipairs(helper.get_child(u_killer_cfg, "modifications"))do
		if v[1] == "object" and v[2].feeding == true then
			local effect = helper.get_child(v[2], "effect")
			effect.increase_total = effect.increase_total + 1
			u_killer_cfg.max_hitpoints = u_killer_cfg.max_hitpoints + 1
			u_killer_cfg.hitpoints = u_killer_cfg.hitpoints + 1
			wesnoth.create_unit(u_killer_cfg):to_map()
			wesnoth.float_label(ec.x2, ec.y2, _ "+1 max HP", "0,255,0")
			return
		end
	end
	-- reaching this point means that the unit didn't have the feedng object yet.
	u_killer:add_modification("object", {
		feeding = true,
		T.effect {
			apply_to = "hitpoints",
			increase_total = 1,
		},
	})
	u_killer.hitpoints = u_killer.hitpoints + 1
	wesnoth.float_label(ec.x2, ec.y2, _ "+1 max HP", "0,255,0")
end)