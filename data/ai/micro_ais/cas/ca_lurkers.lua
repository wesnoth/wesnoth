local LS = wesnoth.require "lua/location_set.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local ca_lurkers = {}

function ca_lurkers:evaluation(ai, cfg)
    -- If any lurker has moves left, we return score just above standard combat CA
    local units = wesnoth.get_units { side = wesnoth.current.side,
        { "and", cfg.filter }, formula = '$this_unit.moves > 0'
    }

    if units[1] then return cfg.ca_score end
    return 0
end

function ca_lurkers:execution(ai, cfg)
    -- We simply pick the first of the lurkers, they have no strategy
    local me = wesnoth.get_units { side = wesnoth.current.side,
        { "and", cfg.filter }, formula = '$this_unit.moves > 0'
    }[1]
    --print("me at:" .. me.x .. "," .. me.y)

    -- Potential targets
    local targets = wesnoth.get_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }
    -- sort targets by hitpoints (lurkers choose lowest HP target)
    table.sort(targets, function (a,b) return (a.hitpoints < b.hitpoints) end)
    --print("Number of potential targets:", #targets)

    -- all reachable hexes
    local reach = LS.of_pairs( wesnoth.find_reach(me.x, me.y) )
    -- all reachable attack hexes
    local reachable_attack_terrain =
         LS.of_pairs( wesnoth.get_locations  {
            {"and", {x = me.x, y = me.y, radius = me.moves} },
            {"and", cfg.filter_location}
        } )
    reachable_attack_terrain:inter(reach)
    --print("  reach: " .. reach:size() .. "    reach_attack: " .. reachable_attack_terrain:size())

    -- need to restrict that to reachable and not occupied by an ally (except own position)
    local reachable_attack_terrain = reachable_attack_terrain:filter(function(x, y, v)
        local occ_hex = wesnoth.get_units { x = x, y = y, { "not", { x = me.x, y = me.y } } }[1]
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
            AH.movefull_stopunit(ai, me, dst[rand])
            if (not me) or (not me.valid) then return end
            AH.checked_attack(ai, me, target)
            attacked = true
            break
       end
    end
    if (not me) or (not me.valid) then return end

    -- If unit has moves left (that is, it didn't attack), go to random wander terrain hex
    -- Check first that unit wasn't killed in the attack
    if (not attacked) and (not cfg.stationary) then

        local reachable_wander_terrain =
            LS.of_pairs( wesnoth.get_locations  {
                {"and", {x = me.x, y = me.y, radius = me.moves} },
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
            dst = { me.x, me.y }
        end
        AH.movefull_stopunit(ai, me, dst)
    end
    if (not me) or (not me.valid) then return end

    -- If the unit has moves or attacks left at this point, take them away
    AH.checked_stopunit_all(ai, me)
end

return ca_lurkers
