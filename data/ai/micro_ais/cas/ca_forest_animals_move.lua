local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

local function get_forest_animals(cfg)
    -- We want the deer/rabbits to move first, tuskers afterward
    local deer_type = cfg.deer_type or "no_unit_of_this_type"
    local rabbit_type = cfg.rabbit_type or "no_unit_of_this_type"
    local forest_animals = AH.get_units_with_moves {
        side = wesnoth.current.side,
        type = deer_type .. ',' .. rabbit_type
    }

    local tusker_type = cfg.tusker_type or "no_unit_of_this_type"
    local all_tuskers = wesnoth.get_units { side = wesnoth.current.side, type = tusker_type }
    for _,tusker in ipairs(all_tuskers) do
        if (tusker.moves > 0) then table.insert(forest_animals, tusker) end
    end

    -- Tusklets get moved by this CA if there are no tuskers left
    if not all_tuskers[1] then
        local tusklet_type = cfg.tusklet_type or "no_unit_of_this_type"
        local tusklets = wesnoth.get_units { side = wesnoth.current.side, type = tusklet_type }
        for _,tusklet in ipairs(tusklets) do
            if (tusklet.moves > 0) then table.insert(forest_animals, tusklet) end
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
    -- These animals run from any enemy
    local forest_animals = get_forest_animals(cfg)
    local enemies = wesnoth.get_units { { "filter_side", { { "enemy_of", {side = wesnoth.current.side } } } } }

    -- Get the locations of all the rabbit holes
    W.store_items { variable = 'holes_wml' }
    local all_items = H.get_variable_array('holes_wml')
    W.clear_variable { name = 'holes_wml' }

    -- If cfg.rabbit_hole_img is set, only items with that image or halo count as holes
    local holes
    if cfg.rabbit_hole_img then
        for _,item in ipairs(all_items) do
            if (item.image == cfg.rabbit_hole_img) or (item.halo == cfg.rabbit_hole_img) then
                table.insert(holes, item)
            end
        end
    else
        holes = all_items
    end

    local hole_map = LS.create()
    for _,hole in ipairs(holes) do hole_map:insert(hole.x, hole.y, 1) end

    -- Each unit moves independently
    for _,unit in ipairs(forest_animals) do
        -- Behavior is different depending on whether a predator is close or not
        local close_enemies = {}
        for _,enemy in ipairs(enemies) do
            if (H.distance_between(unit.x, unit.y, enemy.x, enemy.y) <= unit.max_moves+1) then
                table.insert(close_enemies, enemy)
            end
        end

        -- If no close enemies, do a random move
        local wander_terrain = cfg.filter_location or {}
        if (not close_enemies[1]) then
            local reach = AH.get_reachable_unocc(unit)
            local wander_locs = wesnoth.get_locations(wander_terrain)
            local locs_map = LS.of_pairs(wander_locs)

            local reachable_wander_terrain = {}
            reach:iter( function(x, y, v)
                if locs_map:get(x,y) then
                    table.insert(reachable_wander_terrain, {x, y})
                end
            end)

            -- Choose one of the possible locations at random
            if reachable_wander_terrain[1] then
                local rand = math.random(#reachable_wander_terrain)
                -- This is not a full move, as running away might happen next
                if (unit.x ~= reachable_wander_terrain[rand][1]) or (unit.y ~= reachable_wander_terrain[rand][2]) then
                    AH.checked_move(ai, unit, reachable_wander_terrain[rand][1], reachable_wander_terrain[rand][2])
                end
            else  -- Or if no close reachable terrain was found, move toward the closest
                local best_hex, min_dist = {}, 9e99
                for _,loc in ipairs(wander_locs) do
                    local dist = H.distance_between(loc[1], loc[2], unit.x, unit.y)
                    if dist < min_dist then
                        best_hex, min_dist = loc, dist
                    end
                end

                if (best_hex[1]) then
                    local x,y = wesnoth.find_vacant_tile(best_hex[1], best_hex[2], unit)
                    local next_hop = AH.next_hop(unit, x, y)

                    if (unit.x ~= next_hop[1]) or (unit.y ~= next_hop[2]) then
                        AH.checked_move(ai, unit, next_hop[1], next_hop[2])
                    end
                end
            end
        end

        -- Now we check for close enemies again, as we might just have moved within reach of some
        local close_enemies = {}
        if unit and unit.valid then
            for _,enemy in ipairs(enemies) do
                if (H.distance_between(unit.x, unit.y, enemy.x, enemy.y) <= unit.max_moves+1) then
                    table.insert(close_enemies, enemy)
                end
            end
        end

        -- If there are close enemies, run away (and rabbits disappear into holes)
        local rabbit_type = cfg.rabbit_type or "no_unit_of_this_type"
        if close_enemies[1] then
            -- Calculate the hex that maximizes distance of unit from enemies
            -- Returns nil if the only hex that can be reached is the one the unit is on
            local farthest_hex = AH.find_best_move(unit, function(x, y)
                local rating = 0
                for _,enemy in ipairs(close_enemies) do
                    local dist = H.distance_between(enemy.x, enemy.y, x, y)
                    rating = rating - 1 / dist^2
                end

                -- If this is a rabbit, try to go for holes
                if (unit.type == rabbit_type) and hole_map:get(x, y) then
                    rating = rating + 1000
                    -- But if possible, go to another hole if unit is on one
                    if (x == unit.x) and (y == unit.y) then rating = rating - 10 end
                end

                return rating
            end)

            AH.movefull_stopunit(ai, unit, farthest_hex)

            -- If this is a rabbit ending on a hole -> disappears
            if unit and unit.valid
                and (unit.type == rabbit_type) and hole_map:get(farthest_hex[1], farthest_hex[2])
            then
                local command =  "wesnoth.put_unit(x1, y1)"
                ai.synced_command(command, farthest_hex[1], farthest_hex[2])
            end
        end

        -- Finally, take moves away, as only partial move might have been done
        -- Also take attacks away, as these units never attack
        if unit and unit.valid then AH.checked_stopunit_all(ai, unit) end
    end
end

return ca_forest_animals_move
