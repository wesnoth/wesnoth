return {
    init = function(ai, existing_engine)
        local engine = existing_engine or {}

        local H = wesnoth.require "lua/helper.lua"
        local AH = wesnoth.require "ai/lua/ai_helper.lua"
        local LS = wesnoth.require "lua/location_set.lua"

        ----- Beginning of Herding Animals AI -----
        -- We'll keep a lot of things denoted as sheep/dogs, because herder/herded is too similar
        function engine:mai_animals_herding_area(cfg)
            -- Find the area that the sheep can occupy
            -- First, find all contiguous hexes around center hex that are inside herding_perimeter
            local herding_area = LS.of_pairs(wesnoth.get_locations {
                x = cfg.herd_x, y = cfg.herd_y, radius = 999,
                {"filter_radius", { { "not", cfg.filter_location } } }
            } )

            -- Then, also exclude hexes next to herding_perimeter; some of the functions work better like that
            herding_area:iter( function(x, y, v)
                for xa, ya in H.adjacent_tiles(x, y) do
                    if (wesnoth.match_location(xa, ya, cfg.filter_location) ) then herding_area:remove(x, y) end
                end
            end)
            --AH.put_labels(herding_area)

            return herding_area
        end

        function engine:mai_animals_herding_attack_close_enemy_eval(cfg)
            -- Any enemy within attention_distance (default = 8) hexes of a sheep will get the dogs' attention
            -- with enemies within attack_distance (default: 4) being attacked
            local enemies = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
                { "filter_location",
                    { radius = (cfg.attention_distance or 8),
                    { "filter", { side = wesnoth.current.side, {"and", cfg.filter_second} } } }
                }
            }
            local dogs = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter},
                formula = '$this_unit.moves > 0'
            }
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second} }

            if enemies[1] and dogs[1] and sheep[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_animals_herding_attack_close_enemy_exec(cfg)
            local dogs = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter},
                formula = '$this_unit.moves > 0' }
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second} }

            -- We start with enemies within attack_distance (default: 4) hexes, which will be attacked
            local enemies = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
                { "filter_location",
                    { radius = (cfg.attack_distance or 4),
                    { "filter", { side = wesnoth.current.side, {"and", cfg.filter_second} } } }
                }
            }

            max_rating, best_dog, best_enemy, best_hex = -9e99, {}, {}, {}
            for i,e in ipairs(enemies) do
                for j,d in ipairs(dogs) do
                    local reach_map = AH.get_reachable_unocc(d)
                    reach_map:iter( function(x, y, v)
                        -- most important: distance to enemy
                        local rating = - H.distance_between(x, y, e.x, e.y) * 100.
                        -- 2nd: distance from any sheep
                        for k,s in ipairs(sheep) do
                            rating = rating - H.distance_between(x, y, s.x, s.y)
                        end
                        -- 3rd: most distant dog goes first
                        rating = rating + H.distance_between(e.x, e.y, d.x, d.y) / 100.
                        reach_map:insert(x, y, rating)

                        if (rating > max_rating) then
                            max_rating = rating
                            best_hex = { x, y }
                            best_dog, best_enemy = d, e
                        end
                    end)
                    --AH.put_labels(reach_map)
                    --W.message { speaker = d.id, message = 'My turn' }
                end
            end

            -- If we found a move, we do it, and attack if possible
            if max_rating > -9e99 then
                --print('Dog moving in to attack')
                AH.movefull_stopunit(ai, best_dog, best_hex)
                if H.distance_between(best_dog.x, best_dog.y, best_enemy.x, best_enemy.y) == 1 then
                    ai.attack(best_dog, best_enemy)
                end
                return
            end

            -- If we got here, no enemies to attack where found, so we go on to block other enemies
            --print('Dogs: No enemies close enough to warrant attack')
            -- Now we get all enemies within attention_distance hexes
            local enemies = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
                { "filter_location",
                    { radius = (cfg.attention_distance or 8),
                    { "filter", { side = wesnoth.current.side, {"and", cfg.filter_second} } } }
                }
            }

            -- Find closest sheep/enemy pair first
            local min_dist, closest_sheep, closest_enemy = 9e99, {}, {}
            for i,e in ipairs(enemies) do
                for j,s in ipairs(sheep) do
                    local d = H.distance_between(e.x, e.y, s.x, s.y)
                    if d < min_dist then
                        min_dist = d
                        closest_sheep, closest_enemy = s, e
                    end
                end
            end
            --print('Closest enemy, sheep:', closest_enemy.id, closest_sheep.id)

            -- Move dogs in between enemies and sheep
            max_rating, best_dog, best_hex = -9e99, {}, {}
            for i,d in ipairs(dogs) do
                local reach_map = AH.get_reachable_unocc(d)
                reach_map:iter( function(x, y, v)
                    -- We want equal distance between enemy and closest sheep
                    local rating = - math.abs(H.distance_between(x, y, closest_sheep.x, closest_sheep.y) - H.distance_between(x, y, closest_enemy.x, closest_enemy.y)) * 100
                    -- 2nd: closeness to sheep
                    rating = rating - H.distance_between(x, y, closest_sheep.x, closest_sheep.y)
                    reach_map:insert(x, y, rating)
                    -- 3rd: most distant dog goes first
                    rating = rating + H.distance_between(closest_enemy.x, closest_enemy.y, d.x, d.y) / 100.
                    reach_map:insert(x, y, rating)

                    if (rating > max_rating) then
                        max_rating = rating
                        best_hex = { x, y }
                        best_dog = d
                    end
                end)
                --AH.put_labels(reach_map)
                --W.message { speaker = d.id, message = 'My turn' }
            end

            -- Move dog to intercept
            --print('Dog moving in to intercept')
            AH.movefull_stopunit(ai, best_dog, best_hex)
        end

        function engine:mai_animals_sheep_runs_enemy_eval(cfg)
            -- Sheep runs from any enemy within attention_distance hexes (after the dogs have moved in)
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second},
                formula = '$this_unit.moves > 0',
                { "filter_location",
                    {
                        { "filter", { { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} } }
                        },
                        radius = (cfg.attention_distance or 8)
                    }
                }
            }

            if sheep[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_animals_sheep_runs_enemy_exec(cfg)
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second},
                formula = '$this_unit.moves > 0',
                { "filter_location",
                    {
                        { "filter", { { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} } }
                        },
                        radius = (cfg.attention_distance or 8)
                    }
                }
            }

            -- Simply start with the first of these sheep
            sheep = sheep[1]
            -- And find the close enemies
            local enemies = wesnoth.get_units {
                { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} },
                { "filter_location", { x = sheep.x, y = sheep.y , radius = (cfg.attention_distance or 8) } }
            }
            --print('#enemies', #enemies)

            -- Maximize distance between sheep and enemies
            local best_hex = AH.find_best_move(sheep, function(x, y)
                local rating = 0
                for i,e in ipairs(enemies) do rating = rating + H.distance_between(x, y, e.x, e.y) end
                return rating
            end)

            AH.movefull_stopunit(ai, sheep, best_hex)
        end

        function engine:mai_animals_sheep_runs_dog_eval(cfg)
            -- Any sheep with moves left next to a dog runs aways
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second},
                formula = '$this_unit.moves > 0',
                { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter} } }
            }

            if sheep[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_animals_sheep_runs_dog_exec(cfg)
            -- simply get the first sheep
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second},
                formula = '$this_unit.moves > 0',
                { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter} } }
            }[1]
            -- and the first dog it is adjacent to
            local dog = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter},
                { "filter_adjacent", { x = sheep.x, y = sheep.y } }
            }[1]

            local c_x, c_y = cfg.herd_x, cfg.herd_y
            -- If dog is farther from center, sheep moves in, otherwise it moves out
            local sign = 1
            if (H.distance_between(dog.x, dog.y, c_x, c_y) >= H.distance_between(sheep.x, sheep.y, c_x, c_y)) then
                sign = -1
            end
            local best_hex = AH.find_best_move(sheep, function(x, y)
                return H.distance_between(x, y, c_x, c_y) * sign
            end)

            AH.movefull_stopunit(ai, sheep, best_hex)
        end

        function engine:mai_animals_herd_sheep_eval(cfg)
            -- If dogs have moves left, and there is a sheep with moves left outside the
            -- herding area, chase it back
            -- We'll do a bunch of nested if's, to speed things up
            local dogs = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter}, formula = '$this_unit.moves > 0' }
            if dogs[1] then
                local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second},
                    { "not", { { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter} } } } }
                }
                if sheep[1] then
                    local herding_area = self:mai_animals_herding_area(cfg)
                    for i,s in ipairs(sheep) do
                        -- If a sheep is found outside the herding area, we want to chase it back
                        if (not herding_area:get(s.x, s.y)) then return cfg.ca_score end
                    end
                end
            end

            -- If we got here, no valid dog/sheep combos were found
            return 0
        end

        function engine:mai_animals_herd_sheep_exec(cfg)
            local dogs = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter}, formula = '$this_unit.moves > 0' }
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second},
                { "not", { { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter} } } } }
            }
            local herding_area = self:mai_animals_herding_area(cfg)
            local sheep_to_herd = {}
            for i,s in ipairs(sheep) do
                -- If a sheep is found outside the herding area, we want to chase it back
                if (not herding_area:get(s.x, s.y)) then table.insert(sheep_to_herd, s) end
            end
            sheep = nil

            -- Find the farthest out sheep that the dogs can get to (and that has moves left)

            -- Find all sheep that have stepped out of bound
            local max_rating, best_dog, best_hex = -9e99, {}, {}
            local c_x, c_y = cfg.herd_x, cfg.herd_y
            for i,s in ipairs(sheep_to_herd) do
                -- This is the rating that depends only on the sheep's position
                -- Farthest sheep goes first
                local sheep_rating = H.distance_between(c_x, c_y, s.x, s.y) / 10.
                -- Sheep with no movement left gets big hit
                if (s.moves == 0) then sheep_rating = sheep_rating - 100. end

                for i,d in ipairs(dogs) do
                    local reach_map = AH.get_reachable_unocc(d)
                    reach_map:iter( function(x, y, v)
                        local dist = H.distance_between(x, y, s.x, s.y)
                        local rating = sheep_rating - dist
                        -- Needs to be on "far side" of sheep, wrt center for adjacent hexes
                        if (H.distance_between(x, y, c_x, c_y) <= H.distance_between(s.x, s.y, c_x, c_y))
                            and (dist == 1)
                        then rating = rating - 1000 end
                        -- And the closer dog goes first (so that it might be able to chase another sheep afterward)
                        rating = rating - H.distance_between(x, y, d.x, d.y) / 100.
                        -- Finally, prefer to stay on path, if possible
                        if (wesnoth.match_location(x, y, cfg.filter_location) ) then rating = rating + 0.001 end

                        reach_map:insert(x, y, rating)

                        if (rating > max_rating) then
                            max_rating = rating
                            best_dog = d
                            best_hex = { x, y }
                        end
                    end)
                    --AH.put_labels(reach_map)
                    --W.message{ speaker = d.id, message = 'My turn' }
                 end
            end

            -- Now we move the best dog
            -- If it's already in the best position, we just take moves away from it
            -- (to avoid black-listing of CA, in the worst case)
            if (best_hex[1] == best_dog.x) and (best_hex[2] == best_dog.y) then
                ai.stopunit_moves(best_dog)
            else
                --print('Dog moving to herd sheep')
                ai.move(best_dog, best_hex[1], best_hex[2])  -- partial move only
            end
        end

        function engine:mai_animals_sheep_move_eval(cfg)
           -- If nothing else is to be done, the sheep do a random move
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second}, formula = '$this_unit.moves > 0' }
            if sheep[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_animals_sheep_move_exec(cfg)
            -- We simply move the first sheep first
            local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second}, formula = '$this_unit.moves > 0' }[1]

            local reach_map = AH.get_reachable_unocc(sheep)
            -- Exclude those that are next to a dog
            reach_map:iter( function(x, y, v)
                for xa, ya in H.adjacent_tiles(x, y) do
                    local dog = wesnoth.get_unit(xa, ya)
                    if dog and (wesnoth.match_unit(dog, cfg.filter)) then
                        reach_map:remove(x, y)
                    end
                end
            end)
            --AH.put_labels(reach_map)

            -- Choose one of the possible locations  at random (or the current location, if no move possible)
            local x, y = sheep.x, sheep.y
            if (reach_map:size() > 0) then
                x, y = AH.LS_random_hex(reach_map)
                --print('Sheep -> :', x, y)
            end

            -- If this move remains within herding area or dogs have no moves left, or sheep doesn't move
            -- make it a full move, otherwise partial move
            local herding_area = self:mai_animals_herding_area(cfg)
            local dogs = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter}, formula = '$this_unit.moves > 0' }
            if herding_area:get(x, y) or (not dogs[1]) or ((x == sheep.x) and (y == sheep.y)) then
                AH.movefull_stopunit(ai, sheep, x, y)
            else
                ai.move(sheep, x, y)
            end
        end

        function engine:mai_animals_dog_move_eval(cfg)
            -- As a final step, any dog not adjacent to a sheep moves within herding_perimeter
            local dogs = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter},
                formula = '$this_unit.moves > 0',
                { "not", { { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter_second} } } } }
            }
            if dogs[1] then return cfg.ca_score end
            return 0
        end

        function engine:mai_animals_dog_move_exec(cfg)
            -- We simply move the first dog first
            local dog = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter},
                formula = '$this_unit.moves > 0',
                { "not", { { "filter_adjacent", { side = wesnoth.current.side, {"and", cfg.filter_second} } } } }
            }[1]

            local herding_perimeter = LS.of_pairs(wesnoth.get_locations(cfg.filter_location))
            --AH.put_labels(herding_perimeter)

            -- Find average distance of herding_perimeter from center
            local av_dist = 0
            herding_perimeter:iter( function(x, y, v)
                av_dist = av_dist + H.distance_between(x, y, cfg.herd_x, cfg.herd_y)
            end)
            av_dist = av_dist / herding_perimeter:size()
            --print('Average distance:', av_dist)

            local best_hex = AH.find_best_move(dog, function(x, y)
                -- Prefer hexes on herding_perimeter, or close to it
                -- Or, if dog cannot get there, prefer to be av_dist from the center
                local rating = 0
                if herding_perimeter:get(x, y) then
                    rating = rating + 1000 + AH.random(99) / 100.
                else
                    rating = rating - math.abs(H.distance_between(x, y, cfg.herd_x, cfg.herd_y) - av_dist) + AH.random(99) / 100.
                end

                return rating
            end)

            --print('Dog wandering')
            AH.movefull_stopunit(ai, dog, best_hex)
        end

        return engine
    end
}
