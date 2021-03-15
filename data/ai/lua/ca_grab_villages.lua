------- Grab Villages CA --------------

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local LS = wesnoth.require "location_set"
local M = wesnoth.map

local GV_unit, GV_village

local ca_grab_villages = {}

function ca_grab_villages:evaluation(cfg, data, filter_own)
    local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'grab_villages'
    if AH.print_eval() then AH.print_ts('     - Evaluating grab_villages CA:') end

    -- Check if there are units with moves left
    local units = AH.get_units_with_moves({
        side = wesnoth.current.side,
        canrecruit = 'no',
        { "and", filter_own }
    }, true)
    if (not units[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local enemies = AH.get_attackable_enemies()

    local avoid_map = LS.of_pairs(ai.aspects.avoid)

    local all_villages, villages = wesnoth.map.find{gives_income = true}, {}
    for _,village in ipairs(all_villages) do
        if (not avoid_map:get(village[1], village[2])) then
            table.insert(villages, village)
        end
    end

    if (not villages[1]) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    -- First check if attacks are possible for any unit
    local return_value = 191000
    -- If one with > 50% chance of kill is possible, set return_value to lower than combat CA
    local attacks = ai.get_attacks()
    for i,a in ipairs(attacks) do
        if (#a.movements == 1) and (a.chance_to_kill > 0.5) then
            return_value = 90000
            break
        end
    end

    -- Also find which locations can be attacked by enemies
    local enemy_attack_map = BC.get_attack_map(enemies).units

    -- Now we go through the villages and units
    local max_rating, best_village, best_unit = - math.huge
    local village_ratings = {}
    for j,v in ipairs(villages) do
        -- First collect all information that only depends on the village
        local village_rating = 0  -- This is the unit independent rating

        local unit_in_way = wesnoth.units.get(v[1], v[2])

        -- If an enemy can get within one move of the village, we want to hold it
        if enemy_attack_map:get(v[1], v[2]) then
                village_rating = village_rating + 100
        end

        -- Unowned and enemy-owned villages get a large bonus
        local owner = wesnoth.map.get_owner(v)
        if (not owner) then
            village_rating = village_rating + 10000
        else
            if wesnoth.sides.is_enemy(owner, wesnoth.current.side) then village_rating = village_rating + 20000 end
        end

        local village_closest_enemy, enemy_distance_from_village = AH.get_closest_enemy(v)
        if (not village_closest_enemy) then enemy_distance_from_village = 0 end

        -- Now we go on to the unit-dependent rating
        local best_unit_rating = - math.huge
        local reachable = false
        for i,u in ipairs(units) do
            -- Skip villages that have units other than 'u' itself on them
            local village_occupied = false
            if AH.is_visible_unit(wesnoth.current.side, unit_in_way) and ((unit_in_way ~= u)) then
                village_occupied = true
            end

            -- Rate all villages that can be reached and are unoccupied by other units
            if (not village_occupied) then
                -- Path finding is expensive, so we do a first cut simply by distance
                -- There is no way a unit can get to the village if the distance is greater than its moves
                local dist = M.distance_between(u.x, u.y, v[1], v[2])
                if (dist <= u.moves) then
                    local path, cost = wesnoth.find_path(u, v[1], v[2])
                    if (cost <= u.moves) then
                        village_rating = village_rating - 1
                        reachable = true
                        local rating = 0

                        -- Prefer strong units if enemies can reach the village, injured units otherwise
                        if enemy_attack_map:get(v[1], v[2]) then
                            rating = rating + u.hitpoints
                        else
                            rating = rating + u.max_hitpoints - u.hitpoints
                        end

                        -- Prefer not backtracking and moving more distant units to capture villages
                        local unit_closest_enemy, enemy_distance_from_unit = AH.get_closest_enemy({u.x, u.y})
                        if (not unit_closest_enemy) then enemy_distance_from_unit = 0 end
                        rating = rating - (enemy_distance_from_village + enemy_distance_from_unit)/5

                        if (rating > best_unit_rating) then
                            best_unit_rating, best_unit = rating, u
                        end
                    end
                end
            end
        end
        village_ratings[v] = {village_rating, best_unit, reachable}
    end
    for j,v in ipairs(villages) do
        local rating = village_ratings[v][1]
        if village_ratings[v][3] and rating > max_rating then
            max_rating, best_village, best_unit = rating, v, village_ratings[v][2]
        end
    end

    if best_village then
        GV_unit, GV_village = best_unit, best_village
        if (max_rating >= 1000) then
            if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
            return return_value
        else
            if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
            return 0
        end
    end
    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 0
end

function ca_grab_villages:execution(cfg, data)
    if AH.print_exec() then AH.print_ts('   Executing grab_villages CA') end
    if AH.show_messages() then wesnoth.wml_actions.message { speaker = GV_unit.id, message = 'Grab villages' } end

    AH.movefull_stopunit(ai, GV_unit, GV_village)
    GV_unit, GV_village = nil, nil
end

return ca_grab_villages
