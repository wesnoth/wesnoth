local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"
local LS = wesnoth.require "location_set"
local WMPF = wesnoth.require "ai/micro_ais/cas/ca_wolves_multipacks_functions.lua"
local M = wesnoth.map

local ca_wolves_multipacks_wander = {}

function ca_wolves_multipacks_wander:evaluation(cfg)
    -- When there's nothing to attack, the wolves wander and regroup into their packs

    local wolves = AH.get_units_with_moves {
        side = wesnoth.current.side,
        type = cfg.type or "Wolf"
    }

    if wolves[1] then return cfg.ca_score end
    return 0
end

function ca_wolves_multipacks_wander:execution(cfg)
    local packs = WMPF.assign_packs(cfg)

    local avoid_map = AH.get_avoid_map(ai, wml.get_child(cfg, "avoid"), true)

    for pack_number,pack in pairs(packs) do
        -- If any of the wolves has a goal set, this is used for the entire pack
        local wolves, goal = {}, {}
        for _,loc in ipairs(pack) do
            local wolf = wesnoth.units.get(loc.x, loc.y)
            table.insert(wolves, wolf)

            -- If any of the wolves in the pack has a goal set, we use that one
            local wolf_goal = MAIUV.get_mai_unit_variables(wolf, cfg.ai_id)
            if wolf_goal.goal_x then
                goal = { wolf_goal.goal_x, wolf_goal.goal_y }
            end
        end

        -- If the position of any of the wolves is at the goal, delete it
        for _,wolf in ipairs(wolves) do
            if (wolf.x == goal[1]) and (wolf.y == goal[2]) then goal = {} end
        end

        -- Pack gets a new goal if none exist or on any move with 10% random chance
        local rand = math.random(10)
        if (not goal[1]) or (rand == 1) then
            local all_locs = AH.get_locations_no_borders {}
            local locs = {}
            for _,loc in ipairs(all_locs) do
                if (not avoid_map:get(loc[1], loc[2])) then
                    table.insert(locs, loc)
                end
            end

            -- Need to find reachable terrain for this to be a viable goal
            -- We only check whether the first wolf can get there
            local unreachable = true
            while locs[1] and unreachable do
                local rand = math.random(#locs)
                local next_hop = AH.next_hop(wolves[1], locs[rand][1], locs[rand][2])
                if next_hop then
                    goal = { locs[rand][1], locs[rand][2] }
                    unreachable = nil
                else
                    table.remove(locs, rand)
                end
            end
        end

        -- It is possible that no goal is found here due to the combination of
        -- impassable terrain and [avoid] instructions
        if (not goal[1]) then
            for _,wolf in ipairs(wolves) do
                AH.checked_stopunit_moves(ai, wolf)
            end
            wolves = {}
        end

        -- This goal is saved with every wolf of the pack
        for _,wolf in ipairs(wolves) do
            MAIUV.insert_mai_unit_variables(wolf, cfg.ai_id, { goal_x = goal[1], goal_y = goal[2] })
        end

        -- The pack wanders with only 2 considerations
        -- 1. Keeping the pack together (most important)
        --   Going through all combinations of all hexes for all wolves is too expensive
        --   -> find hexes that can be reached by all wolves
        -- 2. Getting closest to the goal

        -- Number of wolves that can reach each hex,
        local reach_map = LS.create()
        for _,wolf in ipairs(wolves) do
            local single_reach_map = AH.get_reachmap(wolf, { avoid_map = avoid_map })
            single_reach_map:iter( function(x, y, v)
                reach_map:insert(x, y, (reach_map:get(x, y) or 0) + 100)
            end)
        end

        -- Keep only those hexes that can be reached by all wolves in the pack
        -- and add distance from goal for those
        local max_rating, goto_hex = - math.huge
        reach_map:iter( function(x, y, v)
            local rating = reach_map:get(x, y)
            if (rating == #pack * 100) then
                rating = rating - M.distance_between(x, y, goal[1], goal[2])
                reach_map:insert(x,y, rating)
                if rating > max_rating then
                    max_rating, goto_hex = rating, { x, y }
                end
            else
                reach_map:remove(x, y)
            end
        end)

        -- Sort wolves by MP, the one with fewest moves goes first
        table.sort(wolves, function(a, b) return a.moves < b.moves end)

        -- If there's no hex that all units can reach, use the 'center of gravity' between them
        -- Then we move the first wolf (fewest MP) toward that hex, and the position of that wolf
        -- becomes the goto coordinates for the others
        if wolves[1] and (not goto_hex) then
            local cg = { 0, 0 }  -- Center of gravity hex
            for _,wolf in ipairs(wolves) do
                cg = { cg[1] + wolf.x, cg[2] + wolf.y }
            end
            cg[1] = math.floor(cg[1] / #pack)
            cg[2] = math.floor(cg[2] / #pack)

            -- Find closest move for Wolf #1 to that position, which then becomes the goto hex
            goto_hex = AH.find_best_move(wolves[1], function(x, y)
                return -M.distance_between(x, y, cg[1], cg[2])
            end, { avoid_map = avoid_map })
            -- We could move this wolf right here, but for convenience all the actual moves
            -- are done together below. This should be a small extra calculation cost
        end

        -- It is possible here also that no goto_hex is found
        if (not goto_hex) then
            for _,wolf in ipairs(wolves) do
                AH.checked_stopunit_moves(ai, wolf)
            end
            wolves = {}
        end

        -- Now all wolves in the pack are moved toward goto_hex, starting with the one with fewest MP
        -- Distance to goal hex is taken into account as secondary criterion
        for _,wolf in ipairs(wolves) do
            local best_hex = AH.find_best_move(wolf, function(x, y)
                local rating = -M.distance_between(x, y, goto_hex[1], goto_hex[2])
                rating = rating -M.distance_between(x, y, goal[1], goal[2]) / 100.
                return rating
            end, { avoid_map = avoid_map })

            if cfg.show_pack_number then
                WMPF.clear_label(wolf.x, wolf.y)
            end

            local move_result = AH.movefull_stopunit(ai, wolf, best_hex or { wolf.x, wolf.y })

            if cfg.show_pack_number and wolf and wolf.valid then
                WMPF.put_label(wolf.x, wolf.y, pack_number)
            end

            -- If the wolf was ambushed, return and reconsider
            if (AH.is_incomplete_move(move_result)) then return end
        end
    end
end

return ca_wolves_multipacks_wander
