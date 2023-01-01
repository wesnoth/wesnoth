
local _ = wesnoth.textdomain 'wesnoth-help'
local T = wml.tag

-- The explode event code
wesnoth.game_events.add_repeating("die", function()
	local ec = wesnoth.current.event_context

	if not ec.x1 or not ec.y1 then
		return
	end

	local u_exploder = wesnoth.units.get(ec.x1, ec.y1)

	if not u_exploder or not u_exploder:matches { ability_type = "explode" } then
		return
	end

        local adjacent_units = wesnoth.units.find_on_map{T.filter_adjacent { id = u_exploder.id}}

	for i,v in ipairs(wml.get_child(u_exploder.__cfg, "abilities")) do
		if v.tag == "explode" and v.contents.id == "explode" then
			local dmg = v.contents.value
                        if type(dmg) == 'string' then
                            if string.match( dmg, '^%s*%(.*%)%s*$') then
                                local params = {
                                    unit = u_exploder,
                                    other_unit = wesnoth.units.get(ec.x2, ec.y2)
                                    }
                                local result = wesnoth.eval_formula(dmg, params)
                                dmg = math.floor(tonumber(result))
                            else
                                wml.error('invalid formula')
                            end
                        end
                        if type(dmg) ~= 'number' then
                            wml.error('invalid value')
                        end
                        local dmg_type = v.contents.apply_to
                        -- more useful would be to check for one of the six damage types to catch mispellings,
                        -- but UMC might have more damage types, so string type it is
                        if type(dmg_type) ~= 'string' then
                            wml.error('invalid apply_to data')
                        end
			for j = 1, #(adjacent_units) do
			        local affected_unit_cfg = adjacent_units[j].__cfg
			        wml.fire("harm_unit", {
			            kill = "yes",
			            damage_type = dmg_type,
			            amount = dmg,
			            { "filter", { id = affected_unit_cfg.id}}
			        })
			end
		end
	end
end)
