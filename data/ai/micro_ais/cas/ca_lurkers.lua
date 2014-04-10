local LS = wesnoth.require "lua/location_set.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_lurker(cfg)
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
    -- We simply pick the first of the lurkers, they have no strategy
    local lurker = get_lurker(cfg)
    local targets = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }
    -- sort targets by hitpoints (lurkers choose lowest HP target)
    table.sort(targets, function (a,b) return (a.hitpoints < b.hitpoints) end)
    --print("Number of potential targets:", #targets)

    -- all reachable hexes
    local reach = LS.of_pairs(wesnoth.find_reach(lurker.x, lurker.y))
    -- all reachable attack hexes
    local reachable_attack_terrain =
         LS.of_pairs( wesnoth.get_locations  {
            {"and", {x = lurker.x, y = lurker.y, radius = lurker.moves} },
            {"and", cfg.filter_location}
        } )
    reachable_attack_terrain:inter(reach)
    --print("  reach: " .. reach:size() .. "    reach_attack: " .. reachable_attack_terrain:size())

    -- need to restrict that to reachable and not occupied by an ally (except own position)
    local reachable_attack_terrain = reachable_attack_terrain:filter(function(x, y, v)
        local occ_hex = wesnoth.get_units { x = x, y = y, { "not", { x = lurker.x, y = lurker.y } } }[1]
        return not occ_hex
    end)
    --print("  reach: " .. reach:size() .. "    reach_attack no allies: " .. reachable_attack_terrain:size())

    -- Attack the weakest reachable enemy
    local attacked = false  -- Need this, because unit might die in attack
    for j, target in ipairs(targets) do

        -- Get reachable attack terrain next to target unit
        local rattack_nt_target = LS.of_pairs(wesnoth.get_locations {  x=target.x, y=target.y, radius=1 } )
        rattack_nt_target:inter(reachable_attack_terrain)
        --print("  targets: " .. target.x .. "," .. target.y .. "  adjacent attack terrain: " .. rattack_nt_target:size())

        -- if we found a reachable enemy, attack it
        -- since they are sorted by hitpoints, we can simply attack the first enemy found and break the loop
        if rattack_nt_target:size() > 0 then

            -- Choose one of the possible attack locations  at random
            local rand = math.random(1, rattack_nt_target:size())
            local dst = rattack_nt_target:to_stable_pairs()
            AH.movefull_stopunit(ai, lurker, dst[rand])
            if (not lurker) or (not lurker.valid) then return end
            AH.checked_attack(ai, lurker, target)
            attacked = true
            break
       end
    end
    if (not lurker) or (not lurker.valid) then return end

    -- If unit has moves left (that is, it didn't attack), go to random wander terrain hex
    -- Check first that unit wasn't killed in the attack
    if (not attacked) and (not cfg.stationary) then

        local reachable_wander_terrain =
            LS.of_pairs( wesnoth.get_locations  {
                {"and", {x = lurker.x, y = lurker.y, radius = lurker.moves} },
                {"and", (cfg.filter_location_wander or cfg.filter_location)}
            } )
        reachable_wander_terrain:inter(reach)

        -- get one of the reachable wander terrain hexes randomly
        local rand = math.random(1, reachable_wander_terrain:size())
        --print("  reach_wander no allies: " .. reachable_wander_terrain:size() .. "  rand #: " .. rand)
        local dst = reachable_wander_terrain:to_stable_pairs()
        if dst[1] then
            dst = dst[rand]
        else
            dst = { lurker.x, lurker.y }
        end
        AH.movefull_stopunit(ai, lurker, dst)
    end
    if (not lurker) or (not lurker.valid) then return end

    -- If the unit has moves or attacks left at this point, take them away
    AH.checked_stopunit_all(ai, lurker)
end

return ca_lurkers
