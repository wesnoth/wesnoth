local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "location_set"

local function get_guardian(cfg)
    local filter = H.get_child(cfg, "filter") or { id = cfg.id }
    local guardian = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]
    return guardian
end

local ca_zone_guardian = {}

function ca_zone_guardian:evaluation(cfg)
    if get_guardian(cfg) then return cfg.ca_score end
    return 0
end

function ca_zone_guardian:execution(cfg)
    local guardian = get_guardian(cfg)
    local reach = wesnoth.find_reach(guardian)

    local zone = H.get_child(cfg, "filter_location")
    local zone_enemy = H.get_child(cfg, "filter_location_enemy") or zone
    local enemies = AH.get_attackable_enemies { { "filter_location", zone_enemy } }
    if enemies[1] then
        local min_dist, target = 9e99
        for _,enemy in ipairs(enemies) do
            local dist = H.distance_between(guardian.x, guardian.y, enemy.x, enemy.y)
            if (dist < min_dist) then
                target, min_dist = enemy, dist
            end
        end

        -- If a valid target was found, guardian attacks this target, or moves toward it
        if target then
            -- Find tiles adjacent to the target
            -- Save the one with the highest defense rating that guardian can reach
            local best_defense, attack_loc = -9e99
            for xa,ya in H.adjacent_tiles(target.x, target.y) do
                -- Only consider unoccupied hexes
                local unit_in_way = wesnoth.get_unit(xa, ya)
                if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way))
                    or (unit_in_way == guardian)
                then
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
                AH.robust_move_and_attack(ai, guardian, attack_loc, target)
            else  -- Otherwise move toward that enemy
                local reach = wesnoth.find_reach(guardian)

                -- Go through all hexes the guardian can reach, find closest to target
                -- Cannot use next_hop here since target hex is occupied by enemy
                local min_dist, nh = 9e99
                for _,hex in ipairs(reach) do
                    -- Only consider unoccupied hexes
                    local unit_in_way = wesnoth.get_unit(hex[1], hex[2])
                    if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way))
                        or (unit_in_way == guardian)
                    then
                        local dist = H.distance_between(hex[1], hex[2], target.x, target.y)
                        if (dist < min_dist) then
                            min_dist, nh = dist, { hex[1], hex[2] }
                        end
                    end
                end

                AH.movefull_stopunit(ai, guardian, nh)
            end
        end

    -- If no enemy around or within the zone, move toward station or zone
    else
        local newpos
        -- If cfg.station_x/y are given, move toward that location
        if cfg.station_x and cfg.station_y then
            newpos = { cfg.station_x, cfg.station_y }
        -- Otherwise choose one randomly from those given in filter_location
        else
            local width, height = wesnoth.get_map_size()
            local locs_map = LS.of_pairs(wesnoth.get_locations {
                x = '1-' .. width,
                y = '1-' .. height,
                { "and", zone }
            })

            -- Check out which of those hexes the guardian can reach
            local reach_map = LS.of_pairs(wesnoth.find_reach(guardian))
            reach_map:inter(locs_map)

            -- If it can reach some hexes, use only reachable locations,
            -- otherwise move toward any (random) one from the entire set
            if (reach_map:size() > 0) then
                locs_map = reach_map
            end

            local locs = locs_map:to_pairs()

            if (#locs > 0) then
                local newind = math.random(#locs)
                newpos = { locs[newind][1], locs[newind][2] }
            else
                newpos = { guardian.x, guardian.y }
            end
        end

        -- Next hop toward that position
        local nh = AH.next_hop(guardian, newpos[1], newpos[2])
        if nh then
            AH.movefull_stopunit(ai, guardian, nh)
        end
    end
    if (not guardian) or (not guardian.valid) then return end

    AH.checked_stopunit_moves(ai, guardian)
    -- If there are attacks left and guardian ended up next to an enemy, we'll leave this to RCA AI
end

return ca_zone_guardian
