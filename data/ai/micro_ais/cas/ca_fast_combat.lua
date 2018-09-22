local AH = wesnoth.require "ai/lua/ai_helper.lua"
local FAU = wesnoth.require "ai/micro_ais/cas/ca_fast_attack_utils.lua"
local LS = wesnoth.require "location_set"

local fast_combat_units, fast_combat_unit_i,fast_target, fast_dst
local gamedata, move_cache

local ca_fast_combat = {}

function ca_fast_combat:evaluation(cfg, data)
    move_cache = { turn = wesnoth.current.turn }
    gamedata = FAU.gamedata_setup()

    local filter_own = wml.get_child(cfg, "filter")
    local filter_enemy = wml.get_child(cfg, "filter_second")

    local enemies
    local units_sorted = true
    if (not filter_own) and (not filter_enemy) then
        local attacks_aspect = ai.aspects.attacks
        if (not fast_combat_units) or (not fast_combat_units[1]) then
            -- Leader is dealt with in a separate CA
            fast_combat_units = {}
            for _,unit in ipairs(attacks_aspect.own) do
                if (not unit.canrecruit) then
                    table.insert(fast_combat_units, unit)
                end
            end
            if (not fast_combat_units[1]) then return 0 end
            units_sorted = false
        end
        enemies = attacks_aspect.enemy
    else
        if (not fast_combat_units) or (not fast_combat_units[1]) then
            fast_combat_units = AH.get_live_units(
                FAU.build_attack_filter("own", filter_own)
            )
            if (not fast_combat_units[1]) then return 0 end
            units_sorted = false
        end
        enemies = AH.get_live_units(
            FAU.build_attack_filter("enemy", filter_enemy)
        )
    end

    if not units_sorted then
        -- For speed reasons, we'll go through the arrays from the end, so they are sorted backwards
        if cfg.weak_units_first then
            table.sort(fast_combat_units, function(a,b) return a.hitpoints > b.hitpoints end)
        else
            table.sort(fast_combat_units, function(a,b) return a.hitpoints < b.hitpoints end)
        end
    end

    local enemy_map = LS.create()
    for _,e in ipairs(enemies) do
        enemy_map:insert(e.x, e.y)
    end

    -- Exclude hidden enemies, except if attack_hidden_enemies=yes is set in [micro_ai] tag
    local viewing_side = wesnoth.current.side
    if (not cfg.attack_hidden_enemies) then
        local hidden_enemies = AH.get_live_units {
            { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
            { "filter_vision", { side = wesnoth.current.side, visible = 'no' } }
        }

        for _,e in ipairs(hidden_enemies) do
            enemy_map:remove(e.x, e.y)
        end
    else
        viewing_side = 0
    end

    local aggression = ai.aspects.aggression
    if (aggression > 1) then aggression = 1 end

    -- Get the locations to be avoided
    local avoid_map = FAU.get_avoid_map(cfg)

    for i = #fast_combat_units,1,-1 do
        local unit = fast_combat_units[i]

        if unit and unit.valid and (unit.attacks_left > 0) and (#unit.attacks > 0) then
            local unit_info = FAU.get_unit_info(unit, gamedata)
            local unit_copy = FAU.get_unit_copy(unit.id, gamedata)
            local attacks = AH.get_attacks({ unit }, { include_occupied = cfg.include_occupied_attack_hexes, viewing_side = viewing_side })

            if (#attacks > 0) then
                local max_rating, best_target, best_dst = - math.huge
                for _,attack in ipairs(attacks) do
                    if enemy_map:get(attack.target.x, attack.target.y)
                        and (not avoid_map:get(attack.dst.x, attack.dst.y))
                    then
                        local target = wesnoth.get_unit(attack.target.x, attack.target.y)
                        local target_info = FAU.get_unit_info(target, gamedata)

                        local att_stat, def_stat = FAU.battle_outcome(
                            unit_copy, target, { attack.dst.x, attack.dst.y },
                            unit_info, target_info, gamedata, move_cache
                        )

                        local rating, attacker_rating, defender_rating, extra_rating = FAU.attack_rating(
                            { unit_info }, target_info, { { attack.dst.x, attack.dst.y } },
                            { att_stat }, def_stat, gamedata,
                            {
                                aggression = aggression,
                                leader_weight = cfg.leader_weight
                            }
                        )

                        local acceptable_attack = FAU.is_acceptable_attack(attacker_rating, defender_rating, aggression)

                        if acceptable_attack and (rating > max_rating) then
                            max_rating, best_target, best_dst = rating, target, attack.dst
                        end
                    end
                end

                if best_target then
                    fast_combat_unit_i = i
                    fast_target, fast_dst = best_target, best_dst
                    return cfg.ca_score
                end
            end
        end

        fast_combat_units[i] = nil
    end

    return 0
end

function ca_fast_combat:execution(cfg, data)
    AH.robust_move_and_attack(ai, fast_combat_units[fast_combat_unit_i], fast_dst, fast_target)
    fast_combat_units[fast_combat_unit_i] = nil
    fast_combat_unit_i,fast_target, fast_dst = nil, nil, nil
end

return ca_fast_combat
