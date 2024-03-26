
local _ = wesnoth.textdomain 'wesnoth-help'
local T = wml.tag

-- The feeding event code
wesnoth.game_events.add_repeating("die", function()
	local ec = wesnoth.current.event_context

	if not ec.x1 or not ec.y1 or not ec.x2 or not ec.y2 then
		return
	end

	local u_killer = wesnoth.units.get(ec.x2, ec.y2)
	local u_victim = wesnoth.units.get(ec.x1, ec.y1)

	if not u_killer or not u_killer:matches { ability = "feeding" } then
		return
	end
	if not u_victim or u_victim:matches { status = "unplagueable" } then
		return
	end
	local u_killer_cfg = u_killer.__cfg
	for i,v in ipairs(wml.get_child(u_killer_cfg, "modifications"))do
		if v.tag == "object" and v.contents.feeding == true then
			local effect = wml.get_child(v.contents, "effect")
			effect.increase_total = effect.increase_total + 1
			u_killer_cfg.max_hitpoints = u_killer_cfg.max_hitpoints + 1
			u_killer_cfg.hitpoints = u_killer_cfg.hitpoints + 1
			wesnoth.units.to_map(u_killer_cfg, false)
			wesnoth.interface.float_label(ec.x2, ec.y2, _ "+1 max HP", "0,255,0")
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
	wesnoth.interface.float_label(ec.x2, ec.y2, _ "+1 max HP", "0,255,0")
end)
