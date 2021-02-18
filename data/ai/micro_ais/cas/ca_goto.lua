local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local LS = wesnoth.require "location_set"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"
local MAISD = wesnoth.require "ai/micro_ais/micro_ai_self_data.lua"
local M = wesnoth.map

local function custom_cost(x, y, unit, avoid_map, enemy_map, enemy_attack_map, multiplier)
    local terrain = wesnoth.current.map[{x, y}]
    local move_cost = unit:movement(terrain)

    if avoid_map and avoid_map:get(x, y) then
        move_cost = move_cost + AH.no_path
    end

    if enemy_map then
        move_cost = move_cost + (enemy_map:get(x,y) or 0)
        move_cost = move_cost + (enemy_attack_map.units:get(x,y) or 0) * multiplier
    end

    return move_cost
end

local ca_goto, GO_units, GO_locs, GO_avoid_map = {}

function ca_goto:evaluation(cfg, data)
    -- If cfg.release_all_units_at_goal is set, check whether the goal has
    -- already been reached, in which case we do not do anything
    if MAISD.get_mai_self_data(data, cfg.ai_id, "release_all") then
        return 0
    end

    local all_units = AH.get_units_with_moves {
        { "and", { side = wesnoth.current.side } },
        { "and", wml.get_child(cfg, "filter") }
    }

    local units = {}
    if cfg.release_unit_at_goal then
        for _,unit in ipairs(all_units) do
            if (not MAIUV.get_mai_unit_variables(unit, cfg.ai_id, "release")) then
                table.insert(units, unit)
            end
        end
    else
        units = all_units
    end

    if (not units[1]) then return 0 end

    -- For convenience, we check for locations here, and just pass that to the exec function
    -- This is mostly to make the unique_goals option easier
    local all_locs = AH.get_locations_no_borders(wml.get_child(cfg, "filter_location"))
    if (#all_locs == 0) then return 0 end

    -- Exclude locations specified in an [avoid] tag in either the Micro AI or the default AI
    GO_avoid_map = AH.get_avoid_map(ai, wml.get_child(cfg, "avoid"), true)
    local valid_locs = {} -- Copying to new table is faster than deleting elements
    for _,loc in ipairs(all_locs) do
        if (not GO_avoid_map:get(loc[1], loc[2])) then
            table.insert(valid_locs, loc)
        end
    end
    if (#valid_locs == 0) then return 0 end

    -- If 'unique_goals' is set, check whether there are locations left to go to.
    -- This does not have to be a persistent variable
    local locs = {}
    if cfg.unique_goals then
        -- First, some cleanup of previous turn data
        local str = 'goal_taken_' .. (wesnoth.current.turn - 1)
        local old_goals = MAISD.get_mai_self_data(data, cfg.ai_id)
        for goal,_ in pairs(old_goals) do
            if string.find(goal, str) then
                old_goals[goal] = nil  -- This also removes it from data
            end
        end

        -- Now on to the current turn
        for _,loc in ipairs(valid_locs) do
            local str = 'goal_taken_' .. wesnoth.current.turn  .. '_' .. loc[1] .. '_' .. loc[2]
            if (not MAISD.get_mai_self_data(data, cfg.ai_id, str)) then
                table.insert(locs, loc)
            end
        end
    else
        locs = valid_locs
    end
    if (not locs[1]) then return 0 end

    -- Now store units and locs, so that we don't need to duplicate this in the exec function
    GO_units, GO_locs = units, locs

    return cfg.ca_score
end

function ca_goto:execution(cfg, data)
    local units, locs = GO_units, GO_locs

    local enemy_map, enemy_attack_map
    if cfg.avoid_enemies then
        if (type(cfg.avoid_enemies) ~= 'number') then
            wml.error("Goto AI avoid_enemies= requires a number as argument")
        elseif (cfg.avoid_enemies <= 0) then
            wml.error("Goto AI avoid_enemies= argument must be >0")
        end

        local enemies = AH.get_visible_units(wesnoth.current.side, {
            { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
        })
        local live_enemies = {}
        for _,enemy in ipairs(enemies) do
            if (not enemy.status.petrified) then
                table.insert(live_enemies, enemy)
            end
        end

        enemy_map = LS.create()
        for _,enemy in ipairs(enemies) do
            enemy_map:insert(enemy.x, enemy.y, (enemy_map:get(enemy.x, enemy.y) or 0) + 1000)
        end
        for _,enemy in ipairs(live_enemies) do
            for xa,ya in H.adjacent_tiles(enemy.x, enemy.y) do
                enemy_map:insert(xa, ya, (enemy_map:get(xa, ya) or 0) + 10)
            end
        end

        enemy_attack_map = BC.get_attack_map(live_enemies)
    end

    local max_rating, closest_hex, best_path, best_unit = - math.huge
    for _,unit in ipairs(units) do
        for _,loc in ipairs(locs) do
            -- If cfg.use_straight_line is set, we simply find the closest
            -- hex to the goal that the unit can get to
            if cfg.use_straight_line then
                local hex, _, rating = AH.find_best_move(unit, function(x, y)
                    local r = -M.distance_between(x, y, loc[1], loc[2])
                    -- Also add distance from unit as very small rating component
                    -- This is mostly here to keep unit in place when no better hexes are available
                    r = r - M.distance_between(x, y, unit.x, unit.y) / 1000.
                    return r
                end, { avoid_map = GO_avoid_map, no_random = true })

                if (rating > max_rating) then
                    max_rating = rating
                    closest_hex, best_unit = hex, unit
                end
            else  -- Otherwise find the best path to take
                local path, cost
                if GO_avoid_map or cfg.avoid_enemies then
                    path, cost = wesnoth.find_path(unit, loc[1], loc[2], {
                        calculate = function(x, y, current_cost)
                            return custom_cost(x, y, unit, GO_avoid_map, enemy_map, enemy_attack_map, cfg.avoid_enemies)
                        end
                    })
                else
                    local enemy_at_goal
                    if cfg.ignore_enemy_at_goal then
                        enemy_at_goal = wesnoth.units.get(loc[1], loc[2])
                        if enemy_at_goal and wesnoth.sides.is_enemy(wesnoth.current.side, enemy_at_goal.side) then
                             enemy_at_goal:extract()
                        else
                            enemy_at_goal = nil
                        end
                    end
                    path, cost = AH.find_path_with_shroud(unit, loc[1], loc[2], { ignore_units = cfg.ignore_units })
                    if enemy_at_goal then
                        enemy_at_goal:to_map()
                        --- Give massive penalty for this goal hex
                        cost = cost + 100
                    end
                end

                -- Make all hexes within the unit's current MP equivalent
                if (cost <= unit.moves) then cost = 0 end

                local rating = - cost

                -- Add a small penalty for hexes occupied by an allied unit
                local unit_in_way = wesnoth.units.get(loc[1], loc[2])
                if unit_in_way and (unit_in_way ~= unit) then
                    rating = rating - 0.01
                end

                if (rating > max_rating) then
                    max_rating = rating
                    closest_hex, best_unit, best_path = loc, unit, path
                end
            end
        end
    end

    -- If 'unique_goals' is set, mark this location as being taken
    if cfg.unique_goals then
        local str = 'goal_taken_' .. wesnoth.current.turn  .. '_' .. closest_hex[1] .. '_' .. closest_hex[2]
        local tmp_table = {}
        tmp_table[str] = true
        MAISD.insert_mai_self_data(data, cfg.ai_id, tmp_table)
    end

    -- If any of the non-standard path finding options were used,
    -- we need to pick the farthest reachable hex along that path
    -- For simplicity, we do it for all types of pathfinding here,
    -- rather than using ai_helper.next_hop for standard pathfinding
    -- Also, straight-line does not produce a path, so we do that first
    if not best_path then
        best_path = AH.find_path_with_shroud(best_unit, closest_hex[1], closest_hex[2])
    end

    -- Now go through the hexes along that path, use normal path finding
    closest_hex = best_path[1]
    for i = 2,#best_path do
        local sub_path, sub_cost = AH.find_path_with_shroud(best_unit, best_path[i][1], best_path[i][2], cfg)
        if sub_cost <= best_unit.moves then
            local unit_in_way = wesnoth.units.get(best_path[i][1], best_path[i][2])
            if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way)) then
                closest_hex = best_path[i]
            end
        else
            break
        end
    end

    if closest_hex then
        AH.checked_move_full(ai, best_unit, closest_hex[1], closest_hex[2])
    else
        AH.checked_stopunit_moves(ai, best_unit)
    end

    if (not best_unit) or (not best_unit.valid) then return end

    -- If release_unit_at_goal= or release_all_units_at_goal= key is set:
    -- Check if the unit made it to one of the goal hexes
    -- This needs to be done for the original goal hexes, not by checking the SLF again,
    -- as that might have changed based on the new situation on the map
    if cfg.release_unit_at_goal or cfg.release_all_units_at_goal then
        local is_unit_at_goal = false
        for _,loc in ipairs(locs) do
            if (best_unit.x == loc[1]) and (best_unit.y == loc[2]) then
                is_unit_at_goal = true
                break
            end
        end

        if is_unit_at_goal then
            if cfg.release_unit_at_goal then
                MAIUV.set_mai_unit_variables(best_unit, cfg.ai_id, { release = true })
            end

            if cfg.release_all_units_at_goal then
                MAISD.insert_mai_self_data(data, cfg.ai_id, { release_all = true })
            end
        end
    end

    GO_units, GO_locs = nil, nil
end

return ca_goto
