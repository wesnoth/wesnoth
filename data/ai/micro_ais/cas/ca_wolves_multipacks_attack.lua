local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"
local WMPF = wesnoth.require "ai/micro_ais/cas/ca_wolves_multipacks_functions.lua"

local ca_wolves_multipacks_attack = {}

function ca_wolves_multipacks_attack:evaluation(cfg)
    -- If wolves have attacks left, call this CA
    -- It will be disabled by being black-listed, so as to avoid
    -- having to do the full attack evaluation for every single move evaluation

    local wolves = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        type = cfg.type or "Wolf"
    }

    if wolves[1] then return cfg.ca_score end
    return 0
end

function ca_wolves_multipacks_attack:execution(cfg)
    local packs = WMPF.assign_packs(cfg)

    -- Attacks are dealt with on a pack by pack basis
    -- and we want all wolves in a pack to move first, before going on to the next pack
    for pack_number,pack in pairs(packs) do
        local keep_attacking_this_pack = true
        local pack_has_attacked = false

        -- This repeats until all wolves in a pack have attacked, or none can attack any more
        while keep_attacking_this_pack do
            local wolves, attacks = {}, {}
            for _,pack_wolf in ipairs(pack) do
                -- Wolf might have moved in previous attack -> use id to identify it
                local wolf = wesnoth.get_units { id = pack_wolf.id }[1]
                if wolf and (wolf.attacks_left > 0) then table.insert(wolves, wolf) end
            end

            local all_attacks = {}
            if wolves[1] then all_attacks = AH.get_attacks(wolves, { simulate_combat = true }) end

            -- Eliminate targets that would split up the wolves by more than 3 hexes
            local attacks = {}
            for _,attack in ipairs(all_attacks) do
                local attack_splits_pack = false
                for _,wolf in ipairs(wolves) do
                    local nh = AH.next_hop(wolf, attack.dst.x, attack.dst.y)
                    local dist = H.distance_between(nh[1], nh[2], attack.dst.x, attack.dst.y)
                    if (dist > 3) then
                        attack_splits_pack = true
                        break
                    end
                end
                if (not attack_splits_pack) then
                    table.insert(attacks, attack)
                end
            end

            if attacks[1] then
                -- Figure out how many different wolves can reach each target, and on how many hexes
                -- The target with the largest value for the smaller of these two numbers is chosen
                -- This is not an exact method, but good enough and much faster than simulating combat
                local attack_map_wolves, attack_map_hexes = {}, {}
                for _,attack in ipairs(attacks) do
                    -- Number different wolves
                    local att_xy = attack.src.x + attack.src.y * 1000
                    local def_xy = attack.target.x + attack.target.y * 1000
                    if (not attack_map_wolves[def_xy]) then attack_map_wolves[def_xy] = {} end
                    attack_map_wolves[def_xy][att_xy] = 1

                    -- Number different hexes
                    if (not attack_map_hexes[def_xy]) then attack_map_hexes[def_xy] = {} end
                    attack_map_hexes[def_xy][attack.dst.x + attack.dst.y * 1000] = 1
                end

                -- Find which target can be attacked by the most units, from the most hexes; and rate by fewest HP if equal
                local max_rating, best_target = -9e99
                for attack_ind,attack in pairs(attack_map_wolves) do
                    local number_wolves, number_hexes = 0, 0
                    for _,w in pairs(attack) do number_wolves = number_wolves + 1 end
                    for _,h in pairs(attack_map_hexes[attack_ind]) do number_hexes = number_hexes + 1 end
                    local rating = math.min(number_wolves, number_hexes)

                    local target = wesnoth.get_unit(attack_ind % 1000, math.floor(attack_ind / 1000))
                    rating = rating - target.hitpoints / 100.

                    -- Also, any target sitting next to a wolf of the same pack that has
                    -- no attacks left is priority targeted (in order to stick with
                    -- the same target for all wolves of the pack)
                    for xa,ya in H.adjacent_tiles(target.x, target.y) do
                        local adj_unit = wesnoth.get_unit(xa, ya)
                        if adj_unit then
                            local unit_pack_number = MAIUV.get_mai_unit_variables(adj_unit, cfg.ai_id, "pack_number")
                            if (unit_pack_number == pack_number)
                               and (adj_unit.side == wesnoth.current.side)
                               and (adj_unit.attacks_left == 0)
                            then
                                rating = rating + 10
                            end
                        end
                    end

                    if rating > max_rating then
                        max_rating, best_target = rating, target
                    end
                end

                -- Now we know the best target and need to attack
                -- This is done on a wolf-by-wolf basis, the outside while loop taking care of
                -- the next wolf in the pack on subsequent iterations
                local max_rating, best_attack = -9e99
                for _,attack in ipairs(attacks) do
                    if (attack.target.x == best_target.x) and (attack.target.y == best_target.y) then
                        local rating = attack.att_stats.average_hp / 2. - attack.def_stats.average_hp
                        if (rating > max_rating) then
                            max_rating, best_attack = rating, attack
                        end
                    end
                end

                if cfg.show_pack_number then
                    WMPF.clear_label(best_attack.src.x, best_attack.src.y)
                end

                local attacker = wesnoth.get_unit(best_attack.src.x, best_attack.src.y)
                local defender = wesnoth.get_unit(best_attack.target.x, best_attack.target.y)

                AH.robust_move_and_attack(ai, attacker, best_attack.dst, defender)

                if cfg.show_pack_number then
                    if attacker and attacker.valid then
                        if cfg.show_pack_number then WMPF.put_label(attacker.x, attacker.y, pack_number) end
                    end
                    if (not defender) or (not defender.valid) then
                        WMPF.clear_label(best_attack.target.x, best_attack.target.y)
                    end
                end

                pack_has_attacked = true
            else
                keep_attacking_this_pack = false
            end
        end

        -- Finally, if any of the wolves in this pack did attack, move the rest of the pack in close
        if pack_has_attacked then
            local wolves_moves, wolves_no_moves = {}, {}
            for _,pack_wolf in ipairs(pack) do
                -- Wolf might have moved in previous attack -> use id to identify it
                local wolf = wesnoth.get_units { id = pack_wolf.id }[1]
                if wolf then
                    if (wolf.moves > 0) then
                        table.insert(wolves_moves, wolf)
                    else
                        table.insert(wolves_no_moves, wolf)
                    end
                end
            end

            -- If we have both wolves that have moved and those that have not moved,
            -- move the latter toward the former
            if wolves_moves[1] and wolves_no_moves[1] then
                for _,wolf_moves in ipairs(wolves_moves) do
                    local best_hex = AH.find_best_move(wolf_moves, function(x, y)
                        local rating = 0
                        for _,wolf_no_moves in ipairs(wolves_no_moves) do
                            rating = rating - H.distance_between(x, y, wolf_no_moves.x, wolf_no_moves.y)
                        end
                        return rating
                    end)

                    if cfg.show_pack_number then
                        WMPF.clear_label(wolf_moves.x, wolf_moves.y)
                    end

                    AH.movefull_stopunit(ai, wolf_moves, best_hex)

                    if cfg.show_pack_number and wolf_moves and wolf_moves.valid then
                        WMPF.put_label(wolf_moves.x, wolf_moves.y, pack_number)
                    end
                end
            end
        end
    end
end

return ca_wolves_multipacks_attack
