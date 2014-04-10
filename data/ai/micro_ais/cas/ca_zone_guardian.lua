local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

local function get_guardian(cfg)
    local filter = cfg.filter or { id = cfg.id }
    local guardian = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]
    return guardian
end

local ca_zone_guardian = {}

function ca_zone_guardian:evaluation(ai, cfg)
    if get_guardian(cfg) then return cfg.ca_score end
    return 0
end

function ca_zone_guardian:execution(ai, cfg)
    local guardian = get_guardian(cfg)
    local reach = wesnoth.find_reach(guardian)
    local zone_enemy = cfg.filter_location_enemy or cfg.filter_location
    -- enemy units within reach

    local enemies = wesnoth.get_units {
            { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
            { "filter_location", zone_enemy }
        }
    if enemies[1] then

        local target = {}
        local min_dist = 9999
        for i,e in ipairs(enemies) do
            local dg = H.distance_between(guardian.x, guardian.y, e.x, e.y)

            -- If valid target found, save the one with the shortest distance from guardian
            if (dg < min_dist) then
                --print("target:", e.id, ds, dg)
                target = e
                min_dist = dg
            end
        end

        -- If a valid target was found, guardian attacks this target, or moves toward it
        if (min_dist ~= 9999) then
            --print ("Go for enemy unit:", target.id)

            -- Find tiles adjacent to the target, and save the one that our guardian
            -- can reach with the highest defense rating
            local best_defense, attack_loc = -9e99, {}
            for x,y in H.adjacent_tiles(target.x, target.y) do
                -- only consider unoccupied hexes
                local occ_hex = wesnoth.get_units { x=x, y=y, { "not", { id = guardian.id } } }[1]
                if not occ_hex then
                    -- defense rating of the hex
                    local defense = 100 - wesnoth.unit_defense(guardian, wesnoth.get_terrain(x, y))
                    --print(x,y,defense)
                    local nh = AH.next_hop(guardian, x, y)
                    -- if this is best defense rating and guardian can reach it, save this location
                    if nh then
                        if (nh[1] == x) and (nh[2] == y) and (defense > best_defense) then
                            best_defense, attack_loc = defense, {x, y}
                        end
                    end
                end
            end

            -- If a valid hex was found: move there and attack
            if (best_defense ~= -9e99) then
                --print("Attack at:",attack_loc[1],attack_loc[2],best_defense)
                AH.movefull_stopunit(ai, guardian, attack_loc)
                if (not guardian) or (not guardian.valid) then return end
                AH.checked_attack(ai, guardian, target)
            else  -- otherwise move toward that enemy
                --print("Cannot reach target, moving toward it")
                local reach = wesnoth.find_reach(guardian)

                -- Go through all hexes the guardian can reach, find closest to target
                local nh = {}  -- cannot use next_hop here since target hex is occupied by enemy
                local min_dist = 9999
                for i,r in ipairs(reach) do
                    -- only consider unoccupied hexes
                    local occ_hex = wesnoth.get_units { x=r[1], y=r[2], { "not", { id = guardian.id } } }[1]
                    if not occ_hex then
                        local d = H.distance_between(r[1], r[2], target.x, target.y)
                        if d < min_dist then
                            min_dist = d
                            nh = {r[1], r[2]}
                        end
                    end
                end

                -- Finally, execute the move toward the target
                AH.movefull_stopunit(ai, guardian, nh)
            end
        end

    -- If no enemy around or within the zone, move toward "random" position which are mainy the borders
    else
        --print "Move toward newpos"
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
                { "and", cfg.filter_location }
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

            -- If possible locations were found, move guardian toward a random one,
            -- otherwise the guardian stays where it is
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
