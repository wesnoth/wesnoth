local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_fast_move = {}

function ca_fast_move:evaluation(ai, cfg, self)
    local unit = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no' }[1]

    if unit then return 20000 end
    return 0
end

function ca_fast_move:execution(ai, cfg, self)
    local move_cost_factor = cfg.move_cost_factor or 2.0
    if (move_cost_factor < 1.1) then move_cost_factor = 1.1 end

    local all_units_MP = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no' }
    local units = {}
    for _,unit in ipairs(all_units_MP) do
        if (not unit.status.guardian) then table.insert(units, unit) end
    end

    local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]

    local goals = {}

    -- Villages get added first, so that (hopefully, scouts and faster units will go for them first)
    local village_value = ai.get_village_value()
    if (village_value > 0) then
        local villages = wesnoth.get_villages()

        -- Eliminate villages owned by a side that is not an enemy
        -- Also remove unowned villages if the AI has no leader
        for i = #villages,1,-1 do
            local owner = wesnoth.get_village_owner(villages[i][1], villages[i][2])
            if owner and (not wesnoth.is_enemy(owner, wesnoth.current.side)) then
                table.remove(villages, i)
            end

            if (not leader) and (not owner) then
                table.remove(villages, i)
            end
        end

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

        local villages_to_get = math.min(max_villages, #villages)

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

    -- Now add enemy leaders to the goals
    local enemy_leaders =
        wesnoth.get_units {
            { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
            canrecruit = 'yes'
        }

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

    -- Putting information about all the units into the goals
    -- This is a one-time expense up front, but generally not too bad
    for _,goal in ipairs(goals) do
        -- Insert information about the units
        for i_u,unit in ipairs(units) do
            local dist = H.distance_between(unit.x, unit.y, goal.x, goal.y)

            goal[i_u] = {
                dist = dist / unit.max_moves,
                rating = - math.ceil(dist / unit.max_moves) - dist / 10.,  -- Combination of fastest and closest unit
                i_unit = i_u
            }
        end
        table.sort(goal, function(a, b) return (a.rating > b.rating) end)
    end

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
                best_unit_info = unit_info
                break
            elseif (unit_info.cost < 1000) then
                local rating = - unit_info.cost
                if (rating > max_rating) then
                    max_rating, best_unit_info = rating, unit_info
                end
            end
        end

        if best_unit_info then
            local unit = units[best_unit_info.i_unit]

            -- We now want the hex that is 2 steps beyond the next hop for the unit
            -- on its way toward the goal, ignoring any unit along the way
            local path = wesnoth.find_path(unit, goal.x, goal.y, { ignore_units = true })

            -- Use current unit position as default
            local short_goal, index = { unit.x, unit.y }, 1

            for i = 2,#path do
                local _, sub_cost = wesnoth.find_path(unit, path[i][1], path[i][2], { ignore_units = true })

                if (sub_cost <= unit.moves) then
                    short_goal, index = path[i], i
                else
                    break
                end
            end

            -- Now go two hexes past that, if possible
            if path[index + 2] then
                short_goal = path[index + 2]
            elseif path[index + 1] then
                short_goal = path[index + 1]
            end

            -- Finally find the best move for this unit
            local reach = wesnoth.find_reach(unit)

            local pre_ratings = {}
            local max_rating, best_hex = -9e99
            for _,loc in ipairs(reach) do
                local rating = - H.distance_between(loc[1], loc[2], short_goal[1], short_goal[2])
                local other_rating = - H.distance_between(loc[1], loc[2], goal.x, goal.y) / 10.
                rating = rating + other_rating

                local unit_in_way
                if (rating > max_rating) then
                    unit_in_way = wesnoth.get_unit(loc[1], loc[2])
                    if (unit_in_way == unit) then unit_in_way = nil end

                    if unit_in_way and (unit_in_way.side == unit.side) then
                        local reach = AH.get_reachable_unocc(unit_in_way)
                        if (reach:size() > 1) then
                            unit_in_way = nil
                            rating = rating - 0.01
                            other_rating = other_rating - 0.01
                        end
                    end
                end

                if (not unit_in_way) then
                    if cfg.dungeon_mode then
                        table.insert(pre_ratings, {
                            rating = rating,
                            other_rating = other_rating,
                            x = loc[1], y = loc[2]
                        })
                    else
                        if (rating > max_rating) then
                            max_rating, best_hex = rating, { loc[1], loc[2] }
                        end
                    end
                end
            end

            -- If this is dungeon mode, we need another level of analysis, calculating
            -- the move cost from the target hex to the short goal hex, not just the distance
            if cfg.dungeon_mode then
                table.sort(pre_ratings, function(a,b) return (a.rating > b.rating) end)

                wesnoth.extract_unit(unit)
                local old_x, old_y = unit.x, unit.y

                local max_rating = -9e99
                for _,pre_rating in ipairs(pre_ratings) do
                    -- If pre_rating is worse than the full rating, we are done because the
                    -- move cost can never be less than the distance, so we cannot possibly do
                    -- better than the pre-rating
                    if (pre_rating.rating <= max_rating) then break end

                    unit.x, unit.y = pre_rating.x, pre_rating.y
                    local _,cost = wesnoth.find_path(unit, short_goal[1], short_goal[2])

                    local rating = - cost + pre_rating.other_rating

                    if (rating > max_rating) then
                        max_rating, best_hex = rating, { pre_rating.x, pre_rating.y }
                    end
                end

                unit.x, unit.y = old_x, old_y
                wesnoth.put_unit(unit)
            end

            if best_hex then
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
                table.remove(goals, next_goal)
                next_goal = next_goal - 1
            end
        else
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
end

return ca_fast_move
