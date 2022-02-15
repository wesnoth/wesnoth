local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"
local M = wesnoth.map

local function bloodlust_unit_attack_weakest_adj_enemy(ai, bloodlust_unit)
    -- Attack the enemy with the fewest hitpoints adjacent to 'bloodlust_unit', if there is one
    -- Returns status of the attack:
    --   'attacked': if a unit was attacked
    --   'killed': if a unit was killed
    --   'no_attack': if no unit was attacked

    if (bloodlust_unit.attacks_left == 0) then return 'no_attack' end

    local min_hp, target = math.huge, nil
    for xa,ya in wesnoth.current.map:iter_adjacent(bloodlust_unit) do
        local enemy = wesnoth.units.get(xa, ya)
        if AH.is_attackable_enemy(enemy) then
            if (enemy.hitpoints < min_hp) then
                min_hp, target = enemy.hitpoints, enemy
            end
        end
    end

    if target then
        AH.checked_attack(ai, bloodlust_unit, target)
        if target and target.valid then
            return 'attacked'
        else
            return 'killed'
        end
    end

    return 'no_attack'
end

local function get_level_bloodlust_unit(cfg)
    local filter = wml.get_child(cfg, "filter") or { id = cfg.id }
    local bloodlust_unit = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]
    return bloodlust_unit
end

local ca_bloodlust = {}

function ca_bloodlust:evaluation(cfg)
    if get_level_bloodlust_unit(cfg) then return cfg.ca_score end
    return 0
end

function ca_bloodlust:execution(cfg)
    -- the bloodlust unit does a random wander in area given by @cfg.wandering_ground until it finds
    -- and kills an enemy unit and then rinse and repeat
    -- note most of this is just some edits of the hunter Micro AI
    -- but with no home/returning things
    -- the bloodlust_unit kills and eliminates anything in the defined arena

    local bloodlust_unit = get_level_bloodlust_unit(cfg)
    local bloodlust_unit_vars = MAIUV.get_mai_unit_variables(bloodlust_unit, cfg.ai_id)

    -- If bloodlust_unit_status is not set for the bloodlust_unit -> default behavior -> random wander
    if (not bloodlust_unit_vars.bloodlust_unit_status) then
        -- bloodlust_unit gets a new goal if none exist or on any move with 10% random chance
        if (not bloodlust_unit_vars.goal_x) or (math.random(10) == 1) then
            -- 'locs' includes border hexes, but that does not matter here
            locs = AH.get_passable_locations((wml.get_child(cfg, "filter_location") or {}), bloodlust_unit)
            local rand = math.random(#locs)

            bloodlust_unit_vars.goal_x, bloodlust_unit_vars.goal_y = locs[rand][1], locs[rand][2]
            MAIUV.set_mai_unit_variables(bloodlust_unit, cfg.ai_id, bloodlust_unit_vars)
        end

        local reach_map = AH.get_reachable_unocc(bloodlust_unit)

        -- Now find the one of these hexes that is closest to the goal
        local max_rating, best_hex = - math.huge, nil
        reach_map:iter( function(x, y, v)
            -- Distance from goal is first rating
            local rating = -M.distance_between(x, y, bloodlust_unit_vars.goal_x, bloodlust_unit_vars.goal_y)

            -- Huge rating bonus if this is next to an enemy
            local enemy_hp = 500
            for xa,ya in wesnoth.current.map:iter_adjacent(x, y) do
                local enemy = wesnoth.units.get(xa, ya)
                if AH.is_attackable_enemy(enemy) then
                    if (enemy.hitpoints < enemy_hp) then enemy_hp = enemy.hitpoints end
                end
            end
            rating = rating + 500 - enemy_hp  -- prefer attack on weakest enemy

            if (rating > max_rating) then
                max_rating, best_hex = rating, { x, y }
            end
        end)

        if (best_hex[1] ~= bloodlust_unit.x) or (best_hex[2] ~= bloodlust_unit.y) then
            AH.checked_move(ai, bloodlust_unit, best_hex[1], best_hex[2])  -- partial move only
            if (not bloodlust_unit) or (not bloodlust_unit.valid) then return end
        else  -- If bloodlust_unit did not move, we need to stop it (also delete the goal)
            AH.checked_stopunit_moves(ai, bloodlust_unit)
            if (not bloodlust_unit) or (not bloodlust_unit.valid) then return end
            bloodlust_unit_vars.goal_x, bloodlust_unit_vars.goal_y = nil, nil
            MAIUV.set_mai_unit_variables(bloodlust_unit, cfg.ai_id, bloodlust_unit_vars)
        end

        -- If this gets the bloodlust_unit to the goal, we delete the goal
        if (bloodlust_unit.x == bloodlust_unit_vars.goal_x) and (bloodlust_unit.y == bloodlust_unit_vars.goal_y) then
            bloodlust_unit_vars.goal_x, bloodlust_unit_vars.goal_y = nil, nil
            MAIUV.set_mai_unit_variables(bloodlust_unit, cfg.ai_id, bloodlust_unit_vars)
        end

        -- Finally, if the bloodlust_unit ended up next to enemies, attack the weakest of those
        local attack_status = bloodlust_unit_attack_weakest_adj_enemy(ai, bloodlust_unit)

        -- If the enemy was killed, bloodlust_unit resets to nil status
        if bloodlust_unit and bloodlust_unit.valid and (attack_status == 'killed') then
            bloodlust_unit_vars.goal_x, bloodlust_unit_vars.goal_y = nil, nil
            -- here, we just set it back to nil
            -- so that the bloodlust_unit_status is not set and then repeats
            bloodlust_unit_vars.bloodlust_unit_status = nil
            MAIUV.set_mai_unit_variables(bloodlust_unit, cfg.ai_id, bloodlust_unit_vars)
        end

        return
    end

    bloodlust_unit_vars.bloodlust_unit_status = nil
    MAIUV.set_mai_unit_variables(bloodlust_unit, cfg.ai_id, bloodlust_unit_vars)
end

return ca_bloodlust