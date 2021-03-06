-------- Castle Switch CA --------------

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local CS_leader_score
-- Note that CS_leader and CS_leader_target are also needed by the recruiting CA, so they must be stored in 'data'

local high_score = 195000
local low_score = 15000

local function get_reachable_enemy_leaders(unit, avoid_map)
    -- We're cheating a little here and also find hidden enemy leaders. That's
    -- because a human player could make a pretty good educated guess as to where
    -- the enemy leaders are likely to be while the AI does not know how to do that.
    local potential_enemy_leaders = AH.get_live_units { canrecruit = 'yes',
        { "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
    }
    local enemy_leaders = {}
    for _,e in ipairs(potential_enemy_leaders) do
        -- Cannot use AH.find_path_with_avoid() here as there might be enemies all around the enemy leader
        if (not avoid_map:get(e.x, e.y)) then
            local path, cost = wesnoth.find_path(unit, e.x, e.y, { ignore_units = true, ignore_visibility = true })
            if cost < AH.no_path then
                table.insert(enemy_leaders, e)
            end
        end
    end

    return enemy_leaders
end

local function other_units_on_keep(leader)
    -- if we're on a keep, wait until there are no movable non-leader units on the castle before moving off
    local leader_score = high_score
    if wesnoth.terrain_types[wesnoth.current.map[leader]].keep then
        local castle = AH.get_locations_no_borders {
            { "and", {
                x = leader.x, y = leader.y, radius = 200,
                { "filter_radius", { terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*' } }
            }}
        }
        local should_wait = false
        for i,loc in ipairs(castle) do
            local unit = wesnoth.units.get(loc[1], loc[2])
            if unit and (unit.side == wesnoth.current.side) and (not unit.canrecruit) and (unit.moves > 0) then
                should_wait = true
                break
            end
        end
        if should_wait then
            leader_score = low_score
        end
    end

    return leader_score
end

local ca_castle_switch = {}

function ca_castle_switch:evaluation(cfg, data, filter_own, recruiting_leader)
    -- @recruiting_leader is passed from the recuit_rushers CA for the leader_takes_village()
    -- evaluation. If it is set, we do the castle switch evaluation only for that leader

    local start_time, ca_name = wesnoth.get_time_stamp() / 1000., 'castle_switch'
    if AH.print_eval() then AH.print_ts('     - Evaluating castle_switch CA:') end

    if ai.aspects.passive_leader then
        -- Turn off this CA if the leader is passive
        return 0
    end

    local leaders
    if recruiting_leader then
        -- Note that doing this might set the stored castle switch information to a different leader.
        -- This is fine though, the order in which these are done is not particularly important.
        leaders = { recruiting_leader }
    else
        leaders = AH.get_units_with_moves({
            side = wesnoth.current.side,
            canrecruit = 'yes',
            formula = '(movement_left = total_movement) and (hitpoints = max_hitpoints)',
            { "and", filter_own }
        }, true)
    end

    local leader
    for _,l in pairs(leaders) do
        if (not AH.is_passive_leader(ai.aspects.passive_leader, l.id)) then
            leader = l
            break
        end
    end

    if not leader then
        -- CA is irrelevant if no leader or the leader may have moved from another CA
        data.CS_leader, data.CS_leader_target = nil, nil
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return 0
    end

    local avoid_map = AH.get_avoid_map(ai, nil, true)

    if data.CS_leader and wesnoth.sides[wesnoth.current.side].gold >= AH.get_cheapest_recruit_cost(data.CS_leader)
        and ((not recruiting_leader) or (recruiting_leader.id == data.CS_leader.id))
    then
        -- If the saved score is the low score, check whether there are still other units on the keep
        if (CS_leader_score == low_score) then
            CS_leader_score = other_units_on_keep(data.CS_leader)
        end

        -- make sure move is still valid
        local path, cost = AH.find_path_with_avoid(data.CS_leader, data.CS_leader_target[1], data.CS_leader_target[2], avoid_map)
        local next_hop = AH.next_hop(data.CS_leader, nil, nil, { path = path, avoid_map = avoid_map })
        if next_hop and next_hop[1] == data.CS_leader_target[1]
            and next_hop[2] == data.CS_leader_target[2]
        then
            return CS_leader_score
        else
            data.CS_leader, data.CS_leader_target = nil, nil
        end
    end

    -- Look for the best keep
    local overall_best_score = 0
    for _,leader in ipairs(leaders) do
        local best_score, best_loc, best_turns, best_path = 0, {}, 3
        local keeps = AH.get_locations_no_borders {
            terrain = 'K*,K*^*,*^K*', -- Keeps
            { "not", { {"filter", {}} }}, -- That have no unit
            { "not", { radius = 6, {"filter", { canrecruit = 'yes',
                { "filter_side", { { "enemy_of", {side = wesnoth.current.side} } } }
            }} }}, -- That are not too close to an enemy leader
            { "not", {
                x = leader.x, y = leader.y, terrain = 'K*,K*^*,*^K*',
                radius = 3,
                { "filter_radius", { terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*' } }
            }}, -- That are not close and connected to a keep the leader is on
            { "filter_adjacent_location", {
                terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*'
            }} -- That are not one-hex keeps
        }
        if #keeps < 1 then
            -- Skip if there aren't extra keeps to evaluate
            -- In this situation we'd only switch keeps if we were running away
            if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
            return 0
        end

        local enemy_leaders = get_reachable_enemy_leaders(leader, avoid_map)

        for i,loc in ipairs(keeps) do
            -- Only consider keeps within 2 turns movement
            local path, cost = AH.find_path_with_avoid(leader, loc[1], loc[2], avoid_map)
            local score = 0
            -- Prefer closer keeps to enemy
            local turns = math.ceil(cost/leader.max_moves)
            if turns <= 2 then
                score = 1/turns
                for j,e in ipairs(enemy_leaders) do
                    score = score + 1 / M.distance_between(loc[1], loc[2], e.x, e.y)
                end

                if score > best_score then
                    best_score = score
                    best_loc = loc
                    best_turns = turns
                    best_path = path
                end
            end
        end

        -- If we're on a keep,
        -- don't move to another keep unless it's much better when uncaptured villages are present
        if best_score > 0 and wesnoth.terrain_types[wesnoth.current.map[leader]].keep then
            local close_unowned_village = (wesnoth.map.find {
                wml.tag['and']{
                    x = leader.x,
                    y = leader.y,
                    radius = leader.max_moves
                },
                gives_income = true,
                owner_side = 0
            })[1]
            if close_unowned_village then
                local score = 1/best_turns
                for j,e in ipairs(enemy_leaders) do
                    -- count all distances as three less than they actually are
                    score = score + 1 / (M.distance_between(leader.x, leader.y, e.x, e.y) - 3)
                end

                if score > best_score then
                    best_score = 0
                end
            end
        end

        if best_score > 0 then
            local next_hop = AH.next_hop(leader, nil, nil, { path = best_path, avoid_map = avoid_map })

            if next_hop and ((next_hop[1] ~= leader.x) or (next_hop[2] ~= leader.y)) then
                -- See if there is a nearby village that can be captured without delaying progress
                local close_villages = wesnoth.map.find( {
                    wml.tag["and"]{ x = next_hop[1], y = next_hop[2], radius = leader.max_moves },
                    gives_income = true,
                    owner_side = 0 })
                local cheapest_unit_cost = AH.get_cheapest_recruit_cost(leader)
                for i,loc in ipairs(close_villages) do
                    local path_village, cost_village = AH.find_path_with_avoid(leader, loc[1], loc[2], avoid_map)
                    if cost_village <= leader.moves then
                        local dummy_leader = leader:clone()
                        dummy_leader.x = loc[1]
                        dummy_leader.y = loc[2]
                        local path_keep, cost_keep = wesnoth.find_path(dummy_leader, best_loc[1], best_loc[2], avoid_map)
                        local turns_from_keep = math.ceil(cost_keep/leader.max_moves)
                        if turns_from_keep < best_turns
                        or (turns_from_keep == 1 and wesnoth.sides[wesnoth.current.side].gold < cheapest_unit_cost)
                        then
                            -- There is, go there instead
                            next_hop = loc
                            break
                        end
                    end
                end
            end

            local leader_score = other_units_on_keep(leader)
            best_score = best_score + leader_score

            if (best_score > overall_best_score) then
                overall_best_score = best_score
                CS_leader_score = leader_score
                data.CS_leader = leader
                data.CS_leader_target = next_hop
            end
        end
    end

    if (overall_best_score > 0) then
        if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
        return CS_leader_score
    end

    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return 0
end

function ca_castle_switch:execution(cfg, data, filter_own)
    if AH.print_exec() then AH.print_ts('   Executing castle_switch CA') end
    if AH.show_messages() then wesnoth.wml_actions.message { speaker = data.leader.id, message = 'Switching castles' } end

    AH.robust_move_and_attack(ai, data.CS_leader, data.CS_leader_target, nil, { partial_move = true })
    data.CS_leader, data.CS_leader_target = nil
end

return ca_castle_switch
