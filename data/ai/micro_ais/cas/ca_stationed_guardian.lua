local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_guardian(cfg)
    local filter = cfg.filter or { id = cfg.id }
    local guardian = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]
    return guardian
end

local ca_stationed_guardian = {}

function ca_stationed_guardian:evaluation(ai, cfg)
    if get_guardian(cfg) then return cfg.ca_score end
    return 0
end

function ca_stationed_guardian:execution(ai, cfg)
    -- (s_x, s_y): coordinates where guardian is stationed; tries to move here if there is nobody to attack
    -- (g_x, g_y): location that the guardian guards

    local guardian = get_guardian(cfg)

    local enemies = wesnoth.get_units {
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
        { "filter_location", { x = guardian.x, y = guardian.y, radius = cfg.distance } }
    }

    -- If no enemies are within cfg.distance: keep guardian from doing anything and exit
    if not enemies[1] then
        AH.checked_stopunit_moves(ai, guardian)
        return
    end

    -- Otherwise, guardian will either attack or move toward station
    -- Enemies must be within cfg.distance of guardian, (s_x, s_y) *and* (g_x, g_y)
    -- simultaneously for guardian to attack
    local min_dist, target = 9e99
    for _,enemy in ipairs(enemies) do
        local dist_s = H.distance_between(cfg.station_x, cfg.station_y, enemy.x, enemy.y)
        local dist_g = H.distance_between(cfg.guard_x or cfg.station_x, cfg.guard_y or cfg.station_y, enemy.x, enemy.y)

        -- If valid target found, save the one with the shortest distance from (g_x, g_y)
        if (dist_s <= cfg.distance) and (dist_g <= cfg.distance) and (dist_g < min_dist) then
            target, min_dist = enemy, dist_g
        end
    end

    -- If a valid target was found, guardian attacks this target, or moves toward it
    if target then
        -- Find tiles adjacent to the target
        -- Save the one with the highest defense rating that guardian can reach
        local best_defense, attack_loc = -9e99
        for xa,ya in H.adjacent_tiles(target.x, target.y) do
            -- Only consider unoccupied hexes
            local occ_hex = wesnoth.get_units { x = xa, y = ya, { "not", { id = guardian.id } } }[1]
            if not occ_hex then
                local defense = 100 - wesnoth.unit_defense(guardian, wesnoth.get_terrain(xa, ya))
                local nh = AH.next_hop(guardian, xa, ya)
                if nh then
                    if (nh[1] == xa) and (nh[2] == ya) and (defense > best_defense) then
                        best_defense, attack_loc = defense, { xa, ya }
                    end
                end
            end
        end

        -- If a valid hex was found: move there and attack
        if attack_loc then
            AH.movefull_stopunit(ai, guardian, attack_loc)
            if (not guardian) or (not guardian.valid) then return end
            if (not target) or (not target.valid) then return end

            AH.checked_attack(ai, guardian, target)
        else  -- Otherwise move toward that enemy
            local reach = wesnoth.find_reach(guardian)

            -- Go through all hexes the guardian can reach, find closest to target
            -- Cannot use next_hop here since target hex is occupied by enemy
            local min_dist, nh = 9e99
            for _,hex in ipairs(reach) do
                -- Only consider unoccupied hexes
                local occ_hex = wesnoth.get_units { x = hex[1], y = hex[2], { "not", { id = guardian.id } } }[1]
                if not occ_hex then
                    local dist = H.distance_between(hex[1], hex[2], target.x, target.y)
                    if (dist < min_dist) then
                        min_dist, nh = dist, { hex[1], hex[2] }
                    end
                end
            end

            AH.movefull_stopunit(ai, guardian, nh)
        end

    -- If no enemy is within the target zone, move toward station position
    else
        local nh = AH.next_hop(guardian, cfg.station_x, cfg.station_y)
        if not nh then
            nh = { guardian.x, guardian.y }
        end

        AH.movefull_stopunit(ai, guardian, nh)
    end

    if (not guardian) or (not guardian.valid) then return end

    AH.checked_stopunit_moves(ai, guardian)
    -- If there are attacks left and guardian ended up next to an enemy, we'll leave this to RCA AI
end

return ca_stationed_guardian
