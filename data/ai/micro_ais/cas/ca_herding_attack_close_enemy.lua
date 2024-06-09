local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map

local function get_sheep(cfg)
    local sheep = wesnoth.units.find_on_map {
        side = wesnoth.current.side,
        wml.tag["and"] ( wml.get_child(cfg, "filter_second") )
    }
    return sheep
end

local function get_dogs(cfg)
    local dogs = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        wml.tag["and"] ( wml.get_child(cfg, "filter") )
    }
    return dogs
end

local function get_enemies(cfg, radius)
    local enemies = AH.get_attackable_enemies {
        wml.tag.filter_location {
            radius = radius,
            wml.tag.filter {
                side = wesnoth.current.side,
                wml.tag["and"] ( wml.get_child(cfg, "filter_second") )
            }
        }
    }
    return enemies
end

local ca_herding_attack_close_enemy = {}

function ca_herding_attack_close_enemy:evaluation(cfg)
    -- Any enemy within attention_distance (default = 8) hexes of a sheep will get the dogs' attention
    -- with enemies within attack_distance (default: 4) being attacked
    if not get_sheep(cfg)[1] then return 0 end
    if not get_dogs(cfg)[1] then return 0 end

    local radius = cfg.attention_distance or 8
    if get_enemies(cfg, radius)[1] then return cfg.ca_score end
    return 0
end

function ca_herding_attack_close_enemy:execution(cfg)
    local sheep = get_sheep(cfg)
    local dogs = get_dogs(cfg)

    -- We start with enemies within attack_distance (default: 4) hexes, which will be attacked
    local radius1 = cfg.attack_distance or 4
    local enemies1 = get_enemies(cfg, radius1)

    local max_rating1, best_dog1, best_enemy, best_hex1 = - math.huge, nil, nil, nil
    for _,enemy in ipairs(enemies1) do
        wesnoth.interface.handle_user_interact()
        for _,dog in ipairs(dogs) do
            local reach_map = AH.get_reachable_unocc(dog)
            reach_map:iter( function(x, y, v)
                -- Most important: distance from enemy
                local rating = -M.distance_between(x, y, enemy.x, enemy.y) * 100.
                -- 2nd: distance from any sheep
                for _,single_sheep in ipairs(sheep) do
                    rating = rating - M.distance_between(x, y, single_sheep.x, single_sheep.y)
                end
                -- 3rd: most distant dog goes first
                rating = rating + M.distance_between(enemy.x, enemy.y, dog.x, dog.y) / 100.
                reach_map:insert(x, y, rating)

                if (rating > max_rating1) then
                    max_rating1 = rating
                    best_dog1, best_enemy, best_hex1 = dog, enemy, { x, y }
                end
            end)
        end
    end

    -- If we found a move, we do it, and attack if possible
    if best_dog1 then
        AH.robust_move_and_attack(ai, best_dog1, best_hex1, best_enemy)
        return
    end

    -- If we got here, no enemies to attack where found, so we go on to block other enemies
    local radius2 = cfg.attention_distance or 8
    local enemies2 = get_enemies(cfg, radius2)

    -- We also need to remove dogs that have no moves left, since selection was done on attacks_left
    for i=#dogs,1,-1 do
        if (dogs[i].moves == 0) then
            table.remove(dogs, i)
        end
    end
    if (not dogs[1]) then return end

    -- Find closest sheep/enemy pair first
    local min_dist, closest_sheep, closest_enemy = math.huge, nil, nil
    for _,enemy in ipairs(enemies2) do
        for _,single_sheep in ipairs(sheep) do
            local dist = M.distance_between(enemy, single_sheep)
            if dist < min_dist then
                min_dist = dist
                closest_sheep, closest_enemy = single_sheep, enemy
            end
        end
    end

    -- Move dogs in between enemies and sheep
    local max_rating2, best_dog2, best_hex2 = - math.huge, nil, nil
    for _,dog in ipairs(dogs) do
        local reach_map = AH.get_reachable_unocc(dog)
        reach_map:iter( function(x, y, v)
            -- We want equal distance between enemy and closest sheep
            local rating = - math.abs(
                M.distance_between(x, y, closest_sheep)
                - M.distance_between(x, y, closest_enemy)
            ) * 100
            -- 2nd: closeness to sheep
            rating = rating - M.distance_between(x, y, closest_sheep)
            reach_map:insert(x, y, rating)
            -- 3rd: most distant dog goes first
            rating = rating + M.distance_between(closest_enemy, dog) / 100.
            reach_map:insert(x, y, rating)

            if (rating > max_rating2) then
                max_rating2, best_hex2, best_dog2 = rating, { x, y }, dog
            end
        end)
    end

    AH.movefull_stopunit(ai, best_dog2, best_hex2)
end

return ca_herding_attack_close_enemy
