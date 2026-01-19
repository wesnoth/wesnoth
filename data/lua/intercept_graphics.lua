if rawget(_G, "intercept_graphic_loaded") ~= nil then -- because of the double loading event isn't ideal, we have to keep this check
    return
end

intercept_graphic_loaded = true
local halo_id = "misc/new-battle.png"
local halo_locations = {}

local function has_intercept_ability(u) --helper function for filtering
    --if u.hitpoints <= 0 then
    --    return false
    --end
    for _, attack in ipairs(u.attacks) do
         local special = wml.find_child(attack.specials,{id="intercept"})
          if special and (not special.uses_per_turn or not u.variables.intercept_used or special.uses_per_turn > u.variables.intercept_used) then
            return true
          end
    end
    --TODO: check if ability is active once a convenient API is provided
    return false
end

local function clear_halo_locations() -- shared cleanup logic
    if next(halo_locations) ~= nil then --check if halo_locations is empty so that redraw is not unnecessarily called
        for _, loc in ipairs(halo_locations) do
            wesnoth.interface.remove_hex_overlay(loc[1], loc[2], halo_id)
            wml.fire("redraw")
        end
        halo_locations = {}
        return true
    end
    return false
end

local old_on_mouse_move = wesnoth.game_events.on_mouse_move
wesnoth.game_events.on_mouse_move = function(x,y)
    local sx, sy = wesnoth.interface.get_selected_hex()
    x = sx or x
    y = sy or y
    local u = wesnoth.units.get(x, y)
    clear_halo_locations()
    if u then
        for _, candidate_hex in ipairs(wesnoth.paths.find_reach(u)) do
            if not wesnoth.units.get(candidate_hex) then -- not placing under units, those tiles cannot be moved onto and the overlay looks weird placed under units
                local n, ne, se, s, sw, nw = wesnoth.map.get_adjacent_hexes(candidate_hex)
                for _, loc in ipairs({n, ne, se, s, sw, nw}) do
                    local e = wesnoth.units.get(loc[1], loc[2])
                    if e and wesnoth.sides.is_enemy(e.side, u.side) and has_intercept_ability(e) then
                        wesnoth.interface.add_hex_overlay(candidate_hex[1], candidate_hex[2], { image = halo_id })
                        table.insert(halo_locations, {candidate_hex[1], candidate_hex[2]})
                    end
                end
            end
        end
    end
    wml.fire("redraw")
    return old_on_mouse_move(x,y)
end

-- if possible, should be replaced when some sort of deselect trigger is available - not problem rn for any case I could think of, but in theory deselect could happen without left-clicking
local old_on_mouse_button = wesnoth.game_events.on_mouse_button
wesnoth.game_events.on_mouse_button = function(x,y,button,event)
    if event == "up" and not wesnoth.interface.get_selected_hex() then -- meaning a leftclick, but no unit was selected -> no unit is neither selectet nor hovered-over
        clear_halo_locations(x, y)
        wml.fire("redraw")
    end
    return old_on_mouse_button(x,y,button,event)
end
