local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

local function get_forest_animals(cfg)
    -- We want the deer/rabbits to move first, tuskers later
    local deer_type = cfg.deer_type or "no_unit_of_this_type"
    local rabbit_type = cfg.rabbit_type or "no_unit_of_this_type"
    local forest_animals = AH.get_units_with_moves {
        side = wesnoth.current.side,
        type = deer_type .. ',' .. rabbit_type
    }

    local tusker_type = cfg.tusker_type or "no_unit_of_this_type"
    local all_tuskers = wesnoth.get_units { side = wesnoth.current.side, type = tusker_type }
    for i,t in ipairs(all_tuskers) do
        if (t.moves > 0) then table.insert(forest_animals, t) end
    end

    -- Tusklets get moved by this CA if there are no tuskers left
    if not all_tuskers[1] then
        local tusklet_type = cfg.tusklet_type or "no_unit_of_this_type"
        local tusklets = wesnoth.get_units { side = wesnoth.current.side, type = tusklet_type }
        for i,t in ipairs(tusklets) do
            if (t.moves > 0) then table.insert(forest_animals, t) end
        end
    end

    return forest_animals
end

local ca_forest_animals_move = {}

function ca_forest_animals_move:evaluation(ai, cfg)
    if get_forest_animals(cfg)[1] then return cfg.ca_score end
    return 0
end

function ca_forest_animals_move:execution(ai, cfg)
    local forest_animals = get_forest_animals(cfg)

    -- These animals run from any enemy
    local enemies = wesnoth.get_units {  { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} } }

    -- Get the locations of all the rabbit holes
    W.store_items { variable = 'holes_wml' }
    local all_items = H.get_variable_array('holes_wml')
    W.clear_variable { name = 'holes_wml' }

    -- If cfg.rabbit_hole_img is set, only items with that image or halo count as holes
    local holes = {}
    for _, item in ipairs(all_items) do
        if cfg.rabbit_hole_img then
            if (item.image == cfg.rabbit_hole_img) or (item.halo == cfg.rabbit_hole_img) then
                table.insert(holes, item)
            end
        else
            table.insert(holes, item)
        end
    end

    local hole_map = LS.create()
    for i,h in ipairs(holes) do hole_map:insert(h.x, h.y, 1) end
    --AH.put_labels(hole_map)

    -- Each unit moves independently
    for i,unit in ipairs(forest_animals) do
        --print('Unit', i, unit.x, unit.y)
        -- Behavior is different depending on whether a predator is close or not
        local close_enemies = {}
        for j,e in ipairs(enemies) do
            if (H.distance_between(unit.x, unit.y, e.x, e.y) <= unit.max_moves+1) then
                table.insert(close_enemies, e)
            end
        end
        --print('  #close_enemies', #close_enemies)

        -- If no close enemies, do a random move
        local wander_terrain = cfg.filter_location or {}
        if (not close_enemies[1]) then
            -- All hexes the unit can reach that are unoccupied
            local reach = AH.get_reachable_unocc(unit)
            local locs = wesnoth.get_locations(wander_terrain)
            local locs_map = LS.of_pairs(locs)
            --print('  #all reachable', reach:size())

            -- Select only those that satisfy wander_terrain
            local reachable_terrain = {}
            reach:iter( function(x, y, v)
                local terrain = wesnoth.get_terrain(x,y)
                --print(x, y, terrain)
                if locs_map:get(x,y) then  -- doesn't work with '^', so start search at char 2
                    table.insert(reachable_terrain, {x, y})
                end
            end)
            --print('  #reachable_terrain', #reachable_terrain)

            -- Choose one of the possible locations at random
            if reachable_terrain[1] then
                local rand = math.random(#reachable_terrain)
                -- This is not a full move, as running away might happen next
                if (unit.x ~= reachable_terrain[rand][1]) or (unit.y ~= reachable_terrain[rand][2]) then
                    AH.checked_move(ai, unit, reachable_terrain[rand][1], reachable_terrain[rand][2])
                end
            else  -- or if no close reachable terrain was found, move toward the closest
                local locs = wesnoth.get_locations(wander_terrain)
                local best_hex, min_dist = {}, 9e99
                for j,l in ipairs(locs) do
                    local d = H.distance_between(l[1], l[2], unit.x, unit.y)
                    if d < min_dist then
                        best_hex, min_dist = l,d
                    end
                end
                if (best_hex[1]) then
                    local x,y = wesnoth.find_vacant_tile(best_hex[1], best_hex[2], unit)
                    local next_hop = AH.next_hop(unit, x, y)
                    --print(next_hop[1], next_hop[2])
                    if (unit.x ~= next_hop[1]) or (unit.y ~= next_hop[2]) then
                        AH.checked_move(ai, unit, next_hop[1], next_hop[2])
                    end
                end
            end
        end

        -- Now we check for close enemies again, as we might just have moved within reach of some
        local close_enemies = {}

        -- We use a trick here to exclude the case when the unit might have been
        -- removed in an event above
        if unit and unit.valid then
            for j,e in ipairs(enemies) do
                if (H.distance_between(unit.x, unit.y, e.x, e.y) <= unit.max_moves+1) then
                    table.insert(close_enemies, e)
                end
            end
        end
        --print('  #close_enemies after move', #close_enemies, #enemies, unit.id)

        -- If there are close enemies, run away (and rabbits disappear into holes)
        local rabbit_type = cfg.rabbit_type or "no_unit_of_this_type"
        if close_enemies[1] then
            -- Calculate the hex that maximizes distance of unit from enemies
            -- Returns nil if the only hex that can be reached is the one the unit is on
            local farthest_hex = AH.find_best_move(unit, function(x, y)
                local rating = 0
                for i,e in ipairs(close_enemies) do
                    local d = H.distance_between(e.x, e.y, x, y)
                    rating = rating - 1 / d^2
                end
                -- If this is a rabbit, try to go for holes
                if (unit.type == rabbit_type) and hole_map:get(x, y) then
                    rating = rating + 1000
                    -- but if possible, go to another hole
                    if (x == unit.x) and (y == unit.y) then rating = rating - 10 end
                end

                return rating
            end)
            --print('  farthest_hex: ', farthest_hex[1], farthest_hex[2])

            -- This will always find at least the hex the unit is on
            -- so no check is necessary
            AH.movefull_stopunit(ai, unit, farthest_hex)
            -- If this is a rabbit ending on a hole -> disappears
            if (unit.type == rabbit_type) and hole_map:get(farthest_hex[1], farthest_hex[2]) then
                local command =  "wesnoth.put_unit(x1, y1)"
                ai.synced_command(command, farthest_hex[1], farthest_hex[2])
            end
        end

        -- Finally, take moves away, as only partial move might have been done
        -- Also attacks, as these units never attack
        if unit and unit.valid then AH.checked_stopunit_all(ai, unit) end
    end
end

return ca_forest_animals_move
