local halo_id = "misc/new-battle.png"
local halo_locations = {}

local function has_intercept_special(u) --helper function for filtering
    --if u.hitpoints <= 0 then
    --    return false
    --end
    for _, attack in ipairs(u.attacks) do
		local special = wml.child_array(attack.specials,"intercept")[1] --TODO this should be removed and the filter should be in the weapon special
		if attack:matches({special_type_active = "intercept"}) and (not special.uses_per_turn or special.uses_per_turn == -1 or not u.variables.intercept_used or special.uses_per_turn > u.variables.intercept_used) then
			return true
		end
    end
    --TODO: check if secial is active once a convenient API is provided
    return false
end

local function clear_halo_locations()
    if next(halo_locations) ~= nil then
        for _, loc in ipairs(halo_locations) do
            wesnoth.interface.remove_hex_overlay(loc, halo_id)
        end
        wml.fire("redraw")
        halo_locations = {}
    end
end

local function halos_for_unit(u,r)
	local viewing_side = wesnoth.interface.get_viewing_side()
	for _, l in ipairs(r) do
		if not (wesnoth.units.get(l)) then -- not placing under units, those tiles cannot be moved onto and the overlay looks weird placed under units
			local n, ne, se, s, sw, nw = wesnoth.map.get_adjacent_hexes(l)
			for _, adj in ipairs({n, ne, se, s, sw, nw}) do
				local e = wesnoth.units.get(adj)
				if e and wesnoth.sides.is_enemy(u.side, e.side) and has_intercept_special(e) and not (wesnoth.sides.is_fogged(viewing_side,adj) or wesnoth.sides.is_shrouded(viewing_side,adj)) then
					wesnoth.interface.add_hex_overlay(l, { image = halo_id })
					table.insert(halo_locations, l)
					break
				end
			end
		end
	end
end

local old_reachmap_updated = wesnoth.game_events.reachmap_updated
wesnoth.game_events.reachmap_updated = function(x,y,r)
	clear_halo_locations()
	local u = wesnoth.units.get(x,y)
	if u then
		halos_for_unit(u,r)
	end
	wml.fire("redraw")
    return old_reachmap_updated(x,y)
end
