local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.dofile "ai/lua/ai_helper.lua"

local ca_fast_move = {}

function ca_fast_move:evaluation(ai, cfg, self)
    local unit = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no' }[1]

    if unit then return 20000 end
    return 0
end

function ca_fast_move:execution(ai, cfg, self)
    local move_cost_factor = cfg.move_cost_factor or 2.0
    if (move_cost_factor < 1.1) then move_cost_factor = 1.1 end

    local units = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no' }
    local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]

    local goals = {}

    -- Villages get added first, so that (hopefully, scouts and faster units will go for them first)
    local village_value = ai.get_village_value()
    if (village_value > 0) then
        local villages = wesnoth.get_villages()
        --print('#villages', #villages)

        -- Eliminate villages owned by a side that is not an enemy
        for i = #villages,1,-1 do
            local owner = wesnoth.get_village_owner(villages[i][1], villages[i][2])
            if owner and (not wesnoth.is_enemy(owner, wesnoth.current.side)) then
                table.remove(villages, i)
            end
        end
        --print('#villages up for grabs', #villages)

        -- Add rating that is sum of inverse distances from all other villages
        for i = 1,#villages-1 do
            local v1 = villages[i]
            for j = i+1,#villages do
                local v2 = villages[j]

                local dist = H.distance_between(v1[1], v1[2], v2[1], v2[2])
                dist = math.ceil(dist / 5.)  -- In discrete steps of 5 hexes

                v1.rating = (v1.rating or 0) + 1. / dist
                v2.rating = (v2.rating or 0) + 1. / dist
            end
        end

        -- Multiply by distance from side leader
        for _,village in ipairs(villages) do
            local dist = 1  -- Just in case there is no leader
            dist = math.ceil(dist / 5.)  -- In discrete steps of 5 hexes
            if leader then
                dist = H.distance_between(village[1], village[2], leader.x, leader.y)
            end

            village.rating = (village.rating or 1.) * 1. / dist
        end

        -- Now we figure out how many villages we want to go for
        local max_villages = math.floor(#units / 4. * village_value)
        --print('max_villages', max_villages)

        local villages_to_get = math.min(max_villages, #villages)
        --print('villages_to_get', villages_to_get)

        for _ = 1,villages_to_get do
            -- Sort villages by rating (highest rating last, because that makes table.remove faster)
            table.sort(villages, function(a, b) return (a.rating < b.rating) end)

            local x,y = villages[#villages][1], villages[#villages][2]
            table.insert(goals, { x = x, y = y, is_village = true })
            table.remove(villages)

            -- Now re-rate the villages, want those farthest from the last selected one first
            -- Need to check whether this was the last village
            if (#villages > 1) then
                local base_rating = villages[#villages].rating

                for _,village in ipairs(villages) do
                    village.rating = village.rating / base_rating
                        * H.distance_between(x, y, village[1], village[2])
                end
            end
        end
    end
    --print('Time after setting up village goals:', wesnoth.get_time_stamp() - start_time)

    -- Now add enemy leaders to the goals
    local enemy_leaders =
        wesnoth.get_units {
            { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
            canrecruit = 'yes'
        }
    --print('#enemy_leaders', #enemy_leaders)

    -- Sort enemy leaders by distance to AI leader
    if leader then
        table.sort(enemy_leaders, function(a, b)
            local dist_a = H.distance_between(leader.x, leader.y, a.x, a.y)
            local dist_b = H.distance_between(leader.x, leader.y, b.x, b.y)
            return (dist_a < dist_b)
        end)
    end

    for i_el,enemy_leader in ipairs(enemy_leaders) do
        local goal = { x = enemy_leader.x, y = enemy_leader.y }
        table.insert(goals, goal)
    end
    --print('Time after setting up enemy leader goals:', wesnoth.get_time_stamp() - start_time)

    -- Putting information about all the units into the goals
    -- This is a one-time expense up front, but generally not too bad
    for _,goal in ipairs(goals) do
        -- Insert information about the units
        for i_u,unit in ipairs(units) do
            local dist = H.distance_between(unit.x, unit.y, goal.x, goal.y)
            goal[i_u] = { dist = dist / unit.max_moves, i_unit = i_u }
        end
        table.sort(goal, function(a, b) return (a.dist < b.dist) end)
    end
    --print('Time after adding unit info into goals:', wesnoth.get_time_stamp() - start_time)

    local keep_moving, next_goal = true, 0
    while keep_moving do
        keep_moving = false

        next_goal = next_goal + 1
        if (next_goal > #goals) then next_goal = 1 end
        local goal = goals[next_goal]

        local max_rating, best_unit_info = -9e99
        for _,unit_info in ipairs(goal) do
            if (not unit_info.cost) then
                local _,cost =
                    wesnoth.find_path(
                        units[unit_info.i_unit].x, units[unit_info.i_unit].y,
                        goal.x, goal.y,
                        { ignore_units = true }
                    )
                cost = cost / units[unit_info.i_unit].max_moves
                unit_info.cost = cost
            end

            if (unit_info.cost < unit_info.dist * move_cost_factor) then
                --print('Efficient move found:', units[unit_info.i_unit].x, units[unit_info.i_unit].y, goal.x, goal.y)

                best_unit_info = unit_info
                break
            elseif (unit_info.cost < 1000) then
                --print('No efficient move found:', units[unit_info.i_unit].x, units[unit_info.i_unit].y, goal.x, goal.y)
                local rating = - unit_info.cost
                if (rating > max_rating) then
                    max_rating, best_unit_info = rating, unit_info
                end
            end
        end

        if best_unit_info then
            local unit = units[best_unit_info.i_unit]

            -- Next hop if there weren't any other units around
            local next_hop = AH.next_hop(unit, goal.x, goal.y, { ignore_units = true })

            -- Finally find the best move for this unit
            local reach = wesnoth.find_reach(unit)
            local max_rating, best_hex = -9e99
            for _,loc in ipairs(reach) do
                local rating = - H.distance_between(loc[1], loc[2], next_hop[1], next_hop[2])
                rating = rating - H.distance_between(loc[1], loc[2], goal.x, goal.y) / 2.

                local unit_in_way
                if (rating > max_rating) then
                    unit_in_way = wesnoth.get_unit(loc[1], loc[2])
                    if (unit_in_way == unit) then unit_in_way = nil end

                    if unit_in_way and (unit_in_way.side == unit.side) then
                        local reach = AH.get_reachable_unocc(unit_in_way)
                        if (reach:size() > 1) then
                            unit_in_way = nil
                            rating = rating - 0.01
                        end
                    end
                end

                if (rating > max_rating) and (not unit_in_way) then
                    max_rating, best_hex = rating, { loc[1], loc[2] }
                end
            end

            if best_hex then
                --print('Doing move:', unit.x, unit.y, best_hex[1], best_hex[2], goal.x, goal.y)

                local dx, dy = goal.x - best_hex[1], goal.y - best_hex[2]
                local r = math.sqrt(dx * dx + dy * dy)
                dx, dy = dx / r, dy / r
                AH.movefull_outofway_stopunit(ai, unit, best_hex[1], best_hex[2], { dx = dx, dy = dy })
            end

            -- Also remove this unit from all the tables; using table.remove is fine here
            for _,tmp_goal in ipairs(goals) do
                for i_info,tmp_info in ipairs(tmp_goal) do
                    if (tmp_info.i_unit == best_unit_info.i_unit) then
                        table.remove(tmp_goal, i_info)
                        break
                    end
                end
            end

            -- Finally, if this was a village, remove the goal (we only do one unit per village
            if goal.is_village then
                --print('Removing goal (only one unit per village):', goal.x, goal.y)
                table.remove(goals, next_goal)
                next_goal = next_goal - 1
            end
        else
            --print('Removing goal (no more unit can get there):', goal.x, goal.y)
            table.remove(goals, next_goal)
            next_goal = next_goal - 1
        end

        for _,goal in ipairs(goals) do
            if goal[1] then
                keep_moving = true
                break
            end
        end
    end
    --print('Overall time:', wesnoth.get_time_stamp() - start_time)
end

return ca_fast_move
