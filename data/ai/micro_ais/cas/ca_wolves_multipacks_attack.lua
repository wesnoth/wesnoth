local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"
local LS = wesnoth.require "lua/location_set.lua"
local WMPF = wesnoth.require "ai/micro_ais/cas/ca_wolves_multipacks_functions.lua"

local ca_wolves_multipacks_attack = {}

function ca_wolves_multipacks_attack:evaluation(ai, cfg)
    local unit_type = cfg.type or "Wolf"
    -- If wolves have attacks left, call this CA
    -- It will generally be disabled by being black-listed, so as to avoid
    -- having to do the full attack evaluation for every single move
    local wolves = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        type = unit_type
    }

    if wolves[1] then return cfg.ca_score end
    return 0
end

function ca_wolves_multipacks_attack:execution(ai, cfg)
    -- First get all the packs
    local packs = WMPF.assign_packs(cfg)

    -- Attacks are dealt with on a pack by pack basis
    -- and I want all wolves in a pack to move first, before going on to the next pack
    -- which makes this slightly more complicated than it would be otherwise
    for pack_number,pack in pairs(packs) do
        local keep_attacking_this_pack = true    -- whether there might be attacks left
        local pack_attacked = false   -- whether an attack by the pack has happened

        -- This repeats until all wolves in a pack have attacked, or none can attack any more
        while keep_attacking_this_pack do
            -- Get the wolves in the pack ...
            local wolves, attacks = {}, {}
            for i,p in ipairs(pack) do
                -- Wolf might have moved in previous attack -> use id to identify it
                local wolf = wesnoth.get_units { id = p.id }
                -- Wolf could have died in previous attack
                -- and only include wolves with attacks left to calc. possible attacks
                if wolf[1] and (wolf[1].attacks_left > 0) then table.insert(wolves, wolf[1]) end
            end

            -- ... and check if any targets are in reach
            local attacks = {}
            if wolves[1] then all_attacks = AH.get_attacks(wolves, { simulate_combat = true }) end
            --print('pack, wolves, attacks:', pack_number, #wolves, #all_attacks)

            -- Eliminate targets that would split up the wolves by more than 3 hexes
            -- This also takes care of wolves joining as a pack rather than attacking individually
            local attacks = {}
            for _, attack in ipairs(all_attacks) do
                for j,w in ipairs(wolves) do
                    local nh = AH.next_hop(w, attack.dst.x, attack.dst.y)
                    local d = H.distance_between(nh[1], nh[2], attack.dst.x, attack.dst.y)
                    if d <= 3 then
                        table.insert(attacks, attack)
                        break
                    end
                end
            end
            --print('-> pack, wolves, attacks:', pack_number, #wolves, #attacks)

            -- If valid attacks were found for this pack
            if attacks[1] then
                -- Figure out how many different wolves can reach each target, and on how many hexes
                -- The target with the largest value for the smaller of these two numbers is chosen
                -- This is not an exact method, but good enough in most cases
                local diff_wolves, diff_hexes = {}, {}
                for i,a in ipairs(attacks) do
                    -- Number different wolves
                    local att_xy = a.src.x + a.src.y * 1000
                    local def_xy = a.target.x + a.target.y * 1000
                    if (not diff_wolves[def_xy]) then diff_wolves[def_xy] = {} end
                    diff_wolves[def_xy][att_xy] = 1
                    -- Number different hexes
                    if (not diff_hexes[def_xy]) then diff_hexes[def_xy] = {} end
                    diff_hexes[def_xy][a.dst.x + a.dst.y * 1000] = 1
                end

                -- Find which target can be attacked by the most units, from the most hexes; and rate by fewest HP if equal
                local max_rating, best_target = -9e99, {}
                for k,t in pairs(diff_wolves) do
                    local n_w, n_h = 0, 0
                    for k1,w in pairs(t) do n_w = n_w + 1 end
                    for k2,h in pairs(diff_hexes[k]) do n_h = n_h + 1 end
                    local rating = math.min(n_w, n_h)

                    local target = wesnoth.get_unit( k % 1000, math.floor(k / 1000))
                    rating = rating - target.hitpoints / 100.

                    -- Also, any target sitting next to a wolf of the same pack that has
                    -- no attacks left is priority targeted (in order to stick with
                    -- the same target for all wolves of the pack)
                    for x, y in H.adjacent_tiles(target.x, target.y) do
                        local adj_unit = wesnoth.get_unit(x, y)
                        if adj_unit then
                            local pack = MAIUV.get_mai_unit_variables(adj_unit, cfg.ai_id, "pack")
                            if (pack == pack_number) and (adj_unit.side == wesnoth.current.side)
                               and (adj_unit.attacks_left == 0)
                            then
                                rating = rating + 10 -- very strongly favors this target
                            end
                        end
                    end

                    --print(k, n_w, n_h, rating)
                    if rating > max_rating then
                        max_rating, best_target = rating, target
                    end
                end
                --print('Best target:', best_target.id, best_target.x, best_target.y)

                -- Now we know what the best target is, we need to attack now
                -- This is done on a wolf-by-wolf basis, the outside while loop taking care of
                -- the next wolf in the pack on subsequent iterations
                local max_rating, best_attack = -9e99, {}
                for i,a in ipairs(attacks) do
                    if (a.target.x == best_target.x) and (a.target.y == best_target.y) then
                        -- HP outcome is rating, twice as important for target as for attacker
                        local rating = a.att_stats.average_hp / 2. - a.def_stats.average_hp
                        if (rating > max_rating) then
                            max_rating, best_attack = rating, a
                        end
                    end
                end

                local attacker = wesnoth.get_unit(best_attack.src.x, best_attack.src.y)
                local defender = wesnoth.get_unit(best_attack.target.x, best_attack.target.y)
                if cfg.show_pack_number then
                    W.label { x = attacker.x, y = attacker.y, text = "" }
                end
                AH.movefull_stopunit(ai, attacker, best_attack.dst.x, best_attack.dst.y)
                if cfg.show_pack_number then
                    WMPF.color_label(attacker.x, attacker.y, pack_number)
                end

                local a_x, a_y, d_x, d_y = attacker.x, attacker.y, defender.x, defender.y
                AH.checked_attack(ai, attacker, defender)
                -- Remove the labels, if one of the units died
                if cfg.show_pack_number then
                    if (not attacker.valid) then W.label { x = a_x, y = a_y, text = "" } end
                    if (not defender.valid) then W.label { x = d_x, y = d_y, text = "" } end
                end

                pack_attacked = true    -- This pack has done an attack
            else
                keep_attacking_this_pack = false    -- no more valid attacks found
            end
        end

        -- Finally, if any of the wolves in this pack did attack, move the rest of the pack in close
       if pack_attacked then
            local wolves_moves, wolves_no_moves = {}, {}
            for i,p in ipairs(pack) do
                -- Wolf might have moved in previous attack -> use id to identify it
                local wolf = wesnoth.get_unit(p.x, p.y)
                -- Wolf could have died in previous attack
                if wolf then
                    if (wolf.moves > 0) then
                        table.insert(wolves_moves, wolf)
                    else
                        table.insert(wolves_no_moves, wolf)
                    end
                end
            end
            --print('#wolves_moves, #wolves_no_moves', #wolves_moves, #wolves_no_moves)

            -- If we have both wolves that have moved and those that have not moved,
            -- move the latter toward the former
            if wolves_moves[1] and wolves_no_moves[1] then
                --print('Collecting stragglers')
                for i,w in ipairs(wolves_moves) do
                    local best_hex = AH.find_best_move(w, function(x, y)
                        local rating = 0
                        for j,w_nm in ipairs(wolves_no_moves) do
                            rating = rating - H.distance_between(x, y, w_nm.x, w_nm.y)
                        end
                        return rating
                    end)
                    if cfg.show_pack_number then
                        W.label { x = w.x, y = w.y, text = "" }
                    end
                    AH.movefull_stopunit(ai, w, best_hex)
                    if cfg.show_pack_number and w and w.valid then
                        WMPF.color_label(w.x, w.y, pack_number)
                    end
                end
            end
        end
    end
end

return ca_wolves_multipacks_attack
