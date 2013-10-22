return {
    init = function(ai, existing_engine)
        local engine = existing_engine or {}

        local H = wesnoth.require "lua/helper.lua"
        local W = H.set_wml_action_metatable {}
        local AH = wesnoth.require "ai/lua/ai_helper.lua"
        local LS = wesnoth.require "lua/location_set.lua"

        function engine:mai_wolves_multipacks_color_label(x, y, text)
            -- For displaying the wolf pack number in color underneath each wolf
            -- only using gray for the time being
            text = "<span color='#c0c0c0'>" .. text .. "</span>"
            W.label{ x = x, y = y, text = text }
        end

        function engine:mai_wolves_multipacks_assign_packs(cfg)
            local unit_type = cfg.type or "Wolf"
            local pack_size = cfg.pack_size or 3

            -- Assign the pack numbers to each wolf. Keeps numbers of existing packs
            -- (unless pack size is down to one). Pack number is stored in wolf unit variables
            -- Also returns a table with the packs (locations and id's of each wolf in a pack)
            local wolves = wesnoth.get_units { side = wesnoth.current.side, type = unit_type }
            --print('#wolves:', #wolves)

            -- Array for holding the packs
            local packs = {}
            -- Find wolves that already have a pack number assigned
            for i,w in ipairs(wolves) do
                if w.variables.pack then
                    if (not packs[w.variables.pack]) then packs[w.variables.pack] = {} end
                    table.insert(packs[w.variables.pack], { x = w.x, y = w.y, id = w.id })
                end
            end

            -- Remove packs of one
            -- Pack numbers might not be consecutive after a while -> need pairs(), not ipairs()
            for k,p in pairs(packs) do
                --print(' have pack:', k, ' #members:', #p)
                if (#p == 1) then
                    local wolf = wesnoth.get_unit(p[1].x, p[1].y)
                    wolf.variables.pack, wolf.variables.goal_x, wolf.variables.goal_y = nil, nil, nil
                    packs[k] = nil
                end
            end
            --print('After removing packs of 1')
            --for k,p in pairs(packs) do print(' have pack:', k, ' #members:', #p) end

            -- Wolves that are not in a pack (new ones or those removed above)
            local nopack_wolves = {}
            for i,w in ipairs(wolves) do
                if (not w.variables.pack) then
                    table.insert(nopack_wolves, w)
                    -- Also erase any goal one of these might have
                    w.variables.pack, w.variables.goal_x, w.variables.goal_y = nil, nil, nil
                end
            end
            --print('#nopack_wolves:', #nopack_wolves)

            -- Now assign the nopack wolves to packs
            -- First, go through packs that have less than pack_size members
            for k,p in pairs(packs) do
                if (#p < pack_size) then
                    local min_dist, best_wolf, best_ind = 9e99, {}, -1
                    for i,w in ipairs(nopack_wolves) do
                        local d1 = H.distance_between(w.x, w.y, p[1].x, p[1].y)
                        local d2 = H.distance_between(w.x, w.y, p[2].x, p[2].y)
                        if (d1 + d2 < min_dist) then
                            min_dist = d1 + d2
                            best_wolf, best_ind = w, i
                        end
                    end
                    if (min_dist < 9e99) then
                        table.insert(packs[k], { x = best_wolf.x, y = best_wolf.y, id = best_wolf.id })
                        best_wolf.variables.pack = k
                        table.remove(nopack_wolves, best_ind)
                    end
                end
            end
            --print('After completing packs of 2')
            --for k,p in pairs(packs) do print(' have pack:', k, ' #members:', #p) end

            -- Second, group remaining single wolves
            -- At the beginning of the scenario, this is all wolves
            while (#nopack_wolves > 0) do
                --print('Grouping the remaining wolves', #nopack_wolves)
                -- First find the first available pack number
                new_pack = 1
                while packs[new_pack] do new_pack = new_pack + 1 end
                --print('Building pack', new_pack)

                -- If there are <=pack_size wolves left, that's the pack (we also assign a single wolf to a 1-wolf pack here)
                if (#nopack_wolves <= pack_size) then
                    --print('<=pack_size nopack wolves left', #nopack_wolves)
                    packs[new_pack] = {}
                    for i,w in ipairs(nopack_wolves) do
                        table.insert(packs[new_pack], { x = w.x, y = w.y, id = w.id })
                        w.variables.pack = new_pack
                    end
                    break
                end

                -- If more than pack_size wolves left, find those that are closest together
                -- They form the next pack
                --print('More than pack_size nopack wolves left', #nopack_wolves)
                local best_wolves = {}
                while #best_wolves < pack_size do
                    local min_dist, best_wolf, best_wolf_i = 9999, {}, -1
                    for i,tw in ipairs(nopack_wolves) do
                        local dist = 0
                        for j,sw in ipairs(best_wolves) do
                            dist = dist + H.distance_between(tw.x, tw.y, sw.x, sw.y)
                        end
                        if dist < min_dist then
                            min_dist, best_wolf, best_wolf_i = dist, tw, i
                        end
                    end
                    table.insert(best_wolves, best_wolf)
                    table.remove(nopack_wolves, best_wolf_i)
                end
                -- Now insert the best pack into that 'packs' array
                packs[new_pack] = {}
                -- Need to count down for table.remove to work correctly
                for i = pack_size,1,-1 do
                    table.insert(packs[new_pack], { x = best_wolves[i].x, y = best_wolves[i].y, id = best_wolves[i].id })
                    best_wolves[i].variables.pack = new_pack
                end
            end
            --print('After grouping remaining single wolves')
            --for k,p in pairs(packs) do print(' have pack:', k, ' #members:', #p) end

            -- Put labels out there for all wolves
            if cfg.show_pack_number then
                for k,p in pairs(packs) do
                    for i,loc in ipairs(p) do
                        self:mai_wolves_multipacks_color_label(loc.x, loc.y, k)
                    end
                end
            end

            return packs
        end

        function engine:mai_wolves_multipacks_attack_eval(cfg)
            local unit_type = cfg.type or "Wolf"

            -- If wolves have attacks left, call this CA
            -- It will generally be disabled by being black-listed, so as to avoid
            -- having to do the full attack evaluation for every single move
            local wolves = wesnoth.get_units { side = wesnoth.current.side, type = unit_type, formula = '$this_unit.attacks_left > 0' }

            if wolves[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_wolves_multipacks_attack_exec(cfg)
            -- First get all the packs
            local packs = self:mai_wolves_multipacks_assign_packs(cfg)

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
                    if wolves[1] then attacks = AH.get_attacks(wolves, { simulate_combat = true }) end
                    --print('pack, wolves, attacks:', pack_number, #wolves, #attacks)

                    -- Eliminate targets that would split up the wolves by more than 3 hexes
                    -- This also takes care of wolves joining as a pack rather than attacking individually
                    for i=#attacks,1,-1 do
                        --print(i, attacks[i].x, attacks[i].y)
                        for j,w in ipairs(wolves) do
                            local nh = AH.next_hop(w, attacks[i].dst.x, attacks[i].dst.y)
                            local d = H.distance_between(nh[1], nh[2], attacks[i].dst.x, attacks[i].dst.y)
                            --print('  ', i, w.x, w.y, d)
                            if d > 3 then
                                table.remove(attacks, i)
                                --print('Removing attack')
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
                                if adj_unit and (adj_unit.variables.pack == pack_number)
                                    and (adj_unit.side == wesnoth.current.side) and (adj_unit.attacks_left == 0)
                                then
                                    rating = rating + 10 -- very strongly favors this target
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
                            self:mai_wolves_multipacks_color_label(attacker.x, attacker.y, pack_number)
                        end

                        local a_x, a_y, d_x, d_y = attacker.x, attacker.y, defender.x, defender.y
                        ai.attack(attacker, defender)
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
                            if cfg.show_pack_number then
                                self:mai_wolves_multipacks_color_label(w.x, w.y, pack_number)
                            end
                        end
                    end
                end

            end

        end

        function engine:mai_wolves_multipacks_wander_eval(cfg)
            local unit_type = cfg.type or "Wolf"

            -- When there's nothing to attack, the wolves wander and regroup into their packs
            local wolves = wesnoth.get_units { side = wesnoth.current.side, type = unit_type, formula = '$this_unit.moves > 0' }

            if wolves[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_wolves_multipacks_wander_exec(cfg)
            -- First get all the packs
            local packs = self:mai_wolves_multipacks_assign_packs(cfg)

            for k,pack in pairs(packs) do
                -- If any of the wolves has a goal set, this is used for the entire pack
                local wolves, goal = {}, {}
                for i,loc in ipairs(pack) do
                    local wolf = wesnoth.get_unit(loc.x, loc.y)
                    --print(k, i, wolf.id)
                    table.insert(wolves, wolf)
                    -- If any of the wolves in the pack has a goal set, we use that one
                    if wolf.variables.goal_x then
                        goal = { wolf.variables.goal_x, wolf.variables.goal_y }
                    end
                end

                -- If the position of any of the wolves is at the goal, delete it
                for i,w in ipairs(wolves) do
                    if (w.x == goal[1]) and (w.y == goal[2]) then goal = {} end
                end

                -- Pack gets a new goal if none exist or on any move with 10% random chance
                local r = AH.random(10)
                if (not goal[1]) or (r == 1) then
                    local w,h,b = wesnoth.get_map_size()
                    local locs = {}
                    locs = wesnoth.get_locations { x = '1-'..w, y = '1-'..h }

                    -- Need to find reachable terrain for this to be a viable goal
                    -- We only check whether the first wolf can get there
                    local unreachable = true
                    while unreachable do
                        local rand = AH.random(#locs)
                        local next_hop = AH.next_hop(wolves[1], locs[rand][1], locs[rand][2])
                        if next_hop then
                            goal = { locs[rand][1], locs[rand][2] }
                            unreachable = nil
                        end
                    end
                end
                --print('Pack goal: ', goal[1], goal[2])

                -- This goal is saved with every wolf of the pack
                for i,w in ipairs(wolves) do
                    w.variables.goal_x, w.variables.goal_y = goal[1], goal[2]
                end

                -- The pack wanders with only 2 considerations
                -- 1. Keeping the pack together (most important)
                --   Going through all combinations of all hexes for all wolves is too expensive
                --   -> find hexes that can be reached by all wolves
                -- 2. Getting closest to the goal (secondary to 1.)

                -- Number of wolves that can reach each hex,
                local reach_map = LS.create()
                for i,w in ipairs(wolves) do
                    local reach = wesnoth.find_reach(w)
                    for j,loc in ipairs(reach) do
                        reach_map:insert(loc[1], loc[2], (reach_map:get(loc[1], loc[2]) or 0) + 100)
                    end
                end

                -- Keep only those hexes that can be reached by all wolves in the pack
                -- and add distance from goal for those
                local max_rating, goto_hex = -9e99, {}
                reach_map:iter( function(x, y, v)
                    local rating = reach_map:get(x, y)
                    if (rating == #pack * 100) then
                        rating = rating - H.distance_between(x, y, goal[1], goal[2])
                        reach_map:insert(x,y, rating)
                        if rating > max_rating then
                            max_rating, goto_hex = rating, { x, y }
                        end
                    else
                        reach_map:remove(x, y)
                    end
                end)

                -- Sort wolves by MP, the one with fewest moves goes first
                table.sort(wolves, function(a, b) return a.moves < b.moves end)

                -- If there's no hex that all units can reach, use the 'center of gravity' between them
                -- Then we move the first wolf (fewest MP) toward that hex, and the position of that wolf
                -- becomes the goto coordinates for the others
                if (not goto_hex[1]) then
                    local cg = { 0, 0 }  -- Center of gravity hex
                    for i,w in ipairs(wolves) do
                        cg = { cg[1] + w.x, cg[2] + w.y }
                    end
                    cg[1] = math.floor(cg[1] / #pack)
                    cg[2] = math.floor(cg[2] / #pack)
                    --print('cg', cg[1], cg[2])

                    -- Find closest move for Wolf #1 to that position, which then becomes the goto hex
                    goto_hex = AH.find_best_move(wolves[1], function(x, y)
                        return -H.distance_between(x, y, cg[1], cg[2])
                    end)
                    -- We could move this wolf right here, but for convenience all the actual moves are
                    -- grouped together below. Speed wise that should not really make a difference, but could be optimized
                end
                --print('goto_hex', goto_hex[1], goto_hex[2])
                --AH.put_labels(reach_map)

                -- Now all wolves in the pack are moved toward goto_hex, starting with the one with fewest MP
                -- Distance to goal hex is taken into account as secondary criterion
                for i,w in ipairs(wolves) do
                    local best_hex = AH.find_best_move(w, function(x, y)
                        local rating = - H.distance_between(x, y, goto_hex[1], goto_hex[2])
                        rating = rating - H.distance_between(x, y, goal[1], goal[2]) / 100.
                        return rating
                    end)
                    if cfg.show_pack_number then
                        W.label { x = w.x, y = w.y, text = "" }
                    end
                    AH.movefull_stopunit(ai, w, best_hex)
                    if cfg.show_pack_number then
                        self:mai_wolves_multipacks_color_label(w.x, w.y, k)
                    end
                end
            end
        end

        return engine
    end
}
