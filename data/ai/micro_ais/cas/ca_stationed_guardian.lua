local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_stationed_guardian = {}

function ca_stationed_guardian:evaluation(ai, cfg)
    local filter = cfg.filter or { id = cfg.id }
    local unit = wesnoth.get_units({
        side = wesnoth.current.side,
        { "and", filter },
        formula = '$this_unit.moves > 0' }
    )[1]

    if unit then return cfg.ca_score end
    return 0
end

function ca_stationed_guardian:execution(ai, cfg)
    -- (s_x,s_y): coordinates where unit is stationed; tries to move here if there is nobody to attack
    -- (g_x,g_y): location that the unit guards

    local filter = cfg.filter or { id = cfg.id }
    local unit = wesnoth.get_units({
        side = wesnoth.current.side,
        { "and", filter },
        formula = '$this_unit.moves > 0' }
    )[1]

    -- find if there are enemies within 'distance'
    local enemies = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
        { "filter_location", {x = unit.x, y = unit.y, radius = cfg.distance} }
    }

    -- if no enemies are within 'distance': keep unit from doing anything and exit
    if not enemies[1] then
        --print("No enemies close -> sleeping:",unit.id)
        AH.checked_stopunit_moves(ai, unit)
        return
    end

    -- Otherwise, unit will either attack or move toward station
    --print("Guardian unit waking up",unit.id)
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

    -- If a valid target was found, unit attacks this target, or moves toward it
    if (min_dist ~= 9999) then
        --print ("Go for enemy unit:", target.id)

        -- Find tiles adjacent to the target, and save the one that our unit
        -- can reach with the highest defense rating
        local best_defense, attack_loc = -9e99, {}
        for x,y in H.adjacent_tiles(target.x, target.y) do
            -- only consider unoccupied hexes
            local occ_hex = wesnoth.get_units { x=x, y=y, { "not", { id = unit.id } } }[1]
            if not occ_hex then
                -- defense rating of the hex
                local defense = 100 - wesnoth.unit_defense(unit, wesnoth.get_terrain(x, y))
                --print(x,y,defense)
                local nh = AH.next_hop(unit, x, y)
                -- if this is best defense rating and unit can reach it, save this location
                if (nh[1] == x) and (nh[2] == y) and (defense > best_defense) then
                    best_defense, attack_loc = defense, {x, y}
                end
            end
        end

        -- If a valid hex was found: move there and attack
        if (best_defense ~= -9e99) then
            --print("Attack at:",attack_loc[1],attack_loc[2],best_defense)
            AH.movefull_stopunit(ai, unit, attack_loc)
            if (not unit) or (not unit.valid) then return end
            if (not target) or (not target.valid) then return end
            AH.checked_attack(ai, unit, target)
        else  -- otherwise move toward that enemy
            --print("Cannot reach target, moving toward it")
            local reach = wesnoth.find_reach(unit)

            -- Go through all hexes the unit can reach, find closest to target
            local nh = {}  -- cannot use next_hop here since target hex is occupied by enemy
            local min_dist = 9999
            for i,r in ipairs(reach) do
                -- only consider unoccupied hexes
                local occ_hex = wesnoth.get_units { x=r[1], y=r[2], { "not", { id = unit.id } } }[1]
                if not occ_hex then
                    local d = H.distance_between(r[1], r[2], target.x, target.y)
                    if d < min_dist then
                        min_dist = d
                        nh = {r[1], r[2]}
                    end
                end
            end

            -- Finally, execute the move toward the target
            AH.movefull_stopunit(ai, unit, nh)
        end

    -- If no enemy within the target zone, move toward station position
    else
        --print "Move toward station"
        local nh = AH.next_hop(unit, cfg.station_x, cfg.station_y)
        AH.movefull_stopunit(ai, unit, nh)
    end

    if (not unit) or (not unit.valid) then return end

    AH.checked_stopunit_moves(ai, unit)
    -- If there are attacks left and unit ended up next to an enemy, we'll leave this to RCA AI
end

return ca_stationed_guardian
