local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"
local LS = wesnoth.require "lua/location_set.lua"
local WMPF = wesnoth.require "ai/micro_ais/cas/ca_wolves_multipacks_functions.lua"

local ca_wolves_multipacks_wander = {}

function ca_wolves_multipacks_wander:evaluation(ai, cfg)
    local unit_type = cfg.type or "Wolf"

    -- When there's nothing to attack, the wolves wander and regroup into their packs
    local wolves = AH.get_units_with_moves {
        side = wesnoth.current.side,
        type = unit_type
    }

    if wolves[1] then return cfg.ca_score end
    return 0
end

function ca_wolves_multipacks_wander:execution(ai, cfg)
    -- First get all the packs
    local packs = WMPF.assign_packs(cfg)

    for k,pack in pairs(packs) do
        -- If any of the wolves has a goal set, this is used for the entire pack
        local wolves, goal = {}, {}
        for i,loc in ipairs(pack) do
            local wolf = wesnoth.get_unit(loc.x, loc.y)
            --print(k, i, wolf.id)
            table.insert(wolves, wolf)

            -- If any of the wolves in the pack has a goal set, we use that one
            local wolf_goal = MAIUV.get_mai_unit_variables(wolf, cfg.ai_id)
            if wolf_goal.goal_x then
                goal = { wolf_goal.goal_x, wolf_goal.goal_y }
            end
        end

        -- If the position of any of the wolves is at the goal, delete it
        for i,w in ipairs(wolves) do
            if (w.x == goal[1]) and (w.y == goal[2]) then goal = {} end
        end

        -- Pack gets a new goal if none exist or on any move with 10% random chance
        local r = math.random(10)
        if (not goal[1]) or (r == 1) then
            local w,h,b = wesnoth.get_map_size()
            local locs = {}
            locs = wesnoth.get_locations { x = '1-'..w, y = '1-'..h }

            -- Need to find reachable terrain for this to be a viable goal
            -- We only check whether the first wolf can get there
            local unreachable = true
            while unreachable do
                local rand = math.random(#locs)
                local next_hop = AH.next_hop(wolves[1], locs[rand][1], locs[rand][2])
                if next_hop then
                    goal = { locs[rand][1], locs[rand][2] }
                    unreachable = nil
                end
            end
        end
        --print('Pack goal: ', goal[1], goal[2])

        -- This goal is saved with every wolf of the pack
        for i,w in ipairs(wolves) do
            MAIUV.insert_mai_unit_variables(w, cfg.ai_id, { goal_x = goal[1], goal_y = goal[2] })
        end

        -- The pack wanders with only 2 considerations
        -- 1. Keeping the pack together (most important)
        --   Going through all combinations of all hexes for all wolves is too expensive
        --   -> find hexes that can be reached by all wolves
        -- 2. Getting closest to the goal (secondary to 1.)

        -- Number of wolves that can reach each hex,
        local reach_map = LS.create()
        for i,w in ipairs(wolves) do
            local reach = wesnoth.find_reach(w)
            for j,loc in ipairs(reach) do
                reach_map:insert(loc[1], loc[2], (reach_map:get(loc[1], loc[2]) or 0) + 100)
            end
        end

        -- Keep only those hexes that can be reached by all wolves in the pack
        -- and add distance from goal for those
        local max_rating, goto_hex = -9e99, {}
        reach_map:iter( function(x, y, v)
            local rating = reach_map:get(x, y)
            if (rating == #pack * 100) then
                rating = rating - H.distance_between(x, y, goal[1], goal[2])
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
        if (not goto_hex[1]) then
            local cg = { 0, 0 }  -- Center of gravity hex
            for i,w in ipairs(wolves) do
                cg = { cg[1] + w.x, cg[2] + w.y }
            end
            cg[1] = math.floor(cg[1] / #pack)
            cg[2] = math.floor(cg[2] / #pack)
            --print('cg', cg[1], cg[2])

            -- Find closest move for Wolf #1 to that position, which then becomes the goto hex
            goto_hex = AH.find_best_move(wolves[1], function(x, y)
                return -H.distance_between(x, y, cg[1], cg[2])
            end)
            -- We could move this wolf right here, but for convenience all the actual moves are
            -- grouped together below. Speed wise that should not really make a difference, but could be optimized
        end
        --print('goto_hex', goto_hex[1], goto_hex[2])
        --AH.put_labels(reach_map)

        -- Now all wolves in the pack are moved toward goto_hex, starting with the one with fewest MP
        -- Distance to goal hex is taken into account as secondary criterion
        for i,w in ipairs(wolves) do
            local best_hex = AH.find_best_move(w, function(x, y)
                local rating = - H.distance_between(x, y, goto_hex[1], goto_hex[2])
                rating = rating - H.distance_between(x, y, goal[1], goal[2]) / 100.
                return rating
            end)
            if cfg.show_pack_number then
                W.label { x = w.x, y = w.y, text = "" }
            end
            AH.movefull_stopunit(ai, w, best_hex)
            if cfg.show_pack_number and w and w.valid then
                WMPF.color_label(w.x, w.y, k)
            end
        end
    end
end

return ca_wolves_multipacks_wander
