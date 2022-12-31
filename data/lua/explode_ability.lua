
local _ = wesnoth.textdomain 'wesnoth-help'
local T = wml.tag

-- The explode event code
wesnoth.game_events.add_repeating("die", function()
	local ec = wesnoth.current.event_context

	if not ec.x1 or not ec.y1 then
		return
	end

	local u_exploder = wesnoth.units.get(ec.x1, ec.y1)

	if not u_exploder or not u_exploder:matches { ability = "explode" } then
		return
	end

	local u_exploder_cfg = u_exploder.__cfg
        local adjacent_units = wesnoth.units.find_on_map{T.filter_adjacent { id = u_exploder_cfg.id}}

	for i,v in ipairs(wml.get_child(u_exploder_cfg, "abilities")) do
		if v[1] == "dummy" and v[2].id == "explode" then
			local dmg = v[2].value
                        local dmg_type = v[2].apply_to
			for i = 1, #(adjacent_units) do
			        local affected_unit_cfg = adjacent_units[i].__cfg
			        wml.fire("harm_unit", { 
			            kill = "yes", 
			            damage_type = dmg_type, 
			            amount = dmg, 
			            -- animate= "yes",
			            { "filter", { id = affected_unit_cfg.id}}
			        })
			end
		end
	end
end)
