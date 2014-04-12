local LS = wesnoth.require "lua/location_set.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_lurker(cfg)
    -- We simply pick the first of the lurkers, they have no strategy
    local lurker = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter }
    }[1]
    return lurker
end

local ca_lurkers = {}

function ca_lurkers:evaluation(ai, cfg)
    if get_lurker(cfg) then return cfg.ca_score end
    return 0
end

function ca_lurkers:execution(ai, cfg)
    local lurker = get_lurker(cfg)
    local targets = wesnoth.get_units {
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } }
    }

    -- Sort targets by hitpoints (lurkers choose lowest HP target)
    table.sort(targets, function(a, b) return (a.hitpoints < b.hitpoints) end)

    local reach = LS.of_pairs(wesnoth.find_reach(lurker.x, lurker.y))
    local reachable_attack_terrain =
         LS.of_pairs(wesnoth.get_locations  {
            { "and", { x = lurker.x, y = lurker.y, radius = lurker.moves } },
            { "and", cfg.filter_location }
        })
    reachable_attack_terrain:inter(reach)

    -- Need to restrict that to reachable and not occupied by an ally (except own position)
    local reachable_attack_terrain = reachable_attack_terrain:filter(function(x, y, v)
        local occ_hex = wesnoth.get_units {
            x = x, y = y,
            { "not", { x = lurker.x, y = lurker.y } }
        }[1]
        return not occ_hex
    end)

    -- Attack the weakest reachable enemy
    for _,target in ipairs(targets) do
        -- Get reachable attack terrain next to target unit
        local reachable_attack_terrrain_adj_target = LS.of_pairs(
            wesnoth.get_locations { x = target.x, y = target.y, radius = 1 }
        )
        reachable_attack_terrrain_adj_target:inter(reachable_attack_terrain)

        -- Since enemies are sorted by hitpoints, we can simply attack the first enemy found
        if reachable_attack_terrrain_adj_target:size() > 0 then
            local rand = math.random(1, reachable_attack_terrrain_adj_target:size())
            local dst = reachable_attack_terrrain_adj_target:to_stable_pairs()

            AH.movefull_stopunit(ai, lurker, dst[rand])
            if (not lurker) or (not lurker.valid) then return end

            AH.checked_attack(ai, lurker, target)
            return
       end
    end

    -- If we got here, unit did not attack: go to random wander terrain hex
    if (lurker.moves > 0) and (not cfg.stationary) then
        local reachable_wander_terrain =
            LS.of_pairs( wesnoth.get_locations {
                { "and", { x = lurker.x, y = lurker.y, radius = lurker.moves } },
                { "and", (cfg.filter_location_wander or cfg.filter_location) }
            })
        reachable_wander_terrain:inter(reach)

        if (reachable_wander_terrain:size() > 0) then
            local dst = reachable_wander_terrain:to_stable_pairs()
            local rand = math.random(1, reachable_wander_terrain:size())
            AH.movefull_stopunit(ai, lurker, dst[rand])
            return
        end
    end

    -- If the unit has moves or attacks left at this point, take them away
    AH.checked_stopunit_all(ai, lurker)
end

return ca_lurkers
