
local _ = wesnoth.textdomain 'wesnoth-help'
local T = wml.tag
local on_event = wesnoth.require("on_event")

-- The feeding event code
on_event("die", function()
	local ec = wesnoth.current.event_context

	if not ec.x1 or not ec.y1 or not ec.x2 or not ec.y2 then
		return
	end

	local u_killer = wesnoth.units.get(ec.x2, ec.y2)
	local u_victim = wesnoth.units.get(ec.x1, ec.y1)

	if not u_killer or not u_killer.active_attack then
		return
	end

	local feeding = u_killer.active_attack:active_specials {
		tag = "feeding",
		base_value = 0,
	}

	if feeding.value <= 0 then
		return
	end

	if not u_victim or u_victim:matches { status = "unplagueable" } then
		return
	end

	-- po: Floating text shown when a unit with the "feeding" ability gets a kill
	local text = _(_"+$value max HP", _"+$value max HP", feeding.value)
	local text = text:vformat { value = feeding.value }

	local u_killer_cfg = u_killer.__cfg
	for i,v in ipairs(wml.get_child(u_killer_cfg, "modifications"))do
		if v[1] == "object" and v[2].feeding == true then
			local effect = wml.get_child(v[2], "effect")
			effect.increase_total = effect.increase_total + feeding.value
			u_killer_cfg.max_hitpoints = u_killer_cfg.max_hitpoints + feeding.value
			u_killer_cfg.hitpoints = u_killer_cfg.hitpoints + feeding.value
			wesnoth.units.to_map(u_killer_cfg)
			wesnoth.interface.float_label(ec.x2, ec.y2, text, "0,255,0")
			return
		end
	end
	-- reaching this point means that the unit didn't have the feedng object yet.
	u_killer:add_modification("object", {
		feeding = true,
		T.effect {
			apply_to = "hitpoints",
			increase_total = feeding.value,
		},
	})
	u_killer.hitpoints = u_killer.hitpoints + feeding.value
	wesnoth.interface.float_label(ec.x2, ec.y2, text, "0,255,0")
end)
