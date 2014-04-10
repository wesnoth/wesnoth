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
    -- (s_x,s_y): coordinates where guardian is stationed; tries to move here if there is nobody to attack
    -- (g_x,g_y): location that the guardian guards

    local guardian = get_guardian(cfg)

    -- find if there are enemies within 'distance'
    local enemies = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
        { "filter_location", {x = guardian.x, y = guardian.y, radius = cfg.distance} }
    }

    -- if no enemies are within 'distance': keep guardian from doing anything and exit
    if not enemies[1] then
        --print("No enemies close -> sleeping:",guardian.id)
        AH.checked_stopunit_moves(ai, guardian)
        return
    end

    -- Otherwise, guardian will either attack or move toward station
    --print("Guardian unit waking up",guardian.id)
    -- enemies must be within 'distance' of guard, (s_x,s_y) *and* (g_x,g_y)
    -- simultaneous for guard to attack
    local target = {}
    local min_dist = 9999
    for i,e in ipairs(enemies) do
        local ds = H.distance_between(cfg.station_x, cfg.station_y, e.x, e.y)
        local dg = H.distance_between(cfg.guard_x, cfg.guard_y, e.x, e.y)

        -- If valid target found, save the one with the shortest distance from (g_x,g_y)
        if (ds <= cfg.distance) and (dg <= cfg.distance) and (dg < min_dist) then
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
                if (nh[1] == x) and (nh[2] == y) and (defense > best_defense) then
                    best_defense, attack_loc = defense, {x, y}
                end
            end
        end

        -- If a valid hex was found: move there and attack
        if (best_defense ~= -9e99) then
            --print("Attack at:",attack_loc[1],attack_loc[2],best_defense)
            AH.movefull_stopunit(ai, guardian, attack_loc)
            if (not guardian) or (not guardian.valid) then return end
            if (not target) or (not target.valid) then return end
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

    -- If no enemy within the target zone, move toward station position
    else
        --print "Move toward station"
        local nh = AH.next_hop(guardian, cfg.station_x, cfg.station_y)
        AH.movefull_stopunit(ai, guardian, nh)
    end

    if (not guardian) or (not guardian.valid) then return end

    AH.checked_stopunit_moves(ai, guardian)
    -- If there are attacks left and guardian ended up next to an enemy, we'll leave this to RCA AI
end

return ca_stationed_guardian
