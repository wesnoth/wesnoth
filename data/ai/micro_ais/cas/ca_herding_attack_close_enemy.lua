local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_sheep(cfg)
    local sheep = wesnoth.get_units {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter_second") }
    }
    return sheep
end

local function get_dogs(cfg)
    local dogs = AH.get_units_with_attacks {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter") }
    }
    return dogs
end

local function get_enemies(cfg, radius)
    local enemies = AH.get_attackable_enemies {
        { "filter_location",
            { radius = radius,
            { "filter", { side = wesnoth.current.side, { "and", H.get_child(cfg, "filter_second") } } } }
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
    local radius = cfg.attack_distance or 4
    local enemies = get_enemies(cfg, radius)

    max_rating, best_dog, best_enemy, best_hex = -9e99
    for _,enemy in ipairs(enemies) do
        for _,dog in ipairs(dogs) do
            local reach_map = AH.get_reachable_unocc(dog)
            reach_map:iter( function(x, y, v)
                -- Most important: distance from enemy
                local rating = - H.distance_between(x, y, enemy.x, enemy.y) * 100.
                -- 2nd: distance from any sheep
                for _,single_sheep in ipairs(sheep) do
                    rating = rating - H.distance_between(x, y, single_sheep.x, single_sheep.y)
                end
                -- 3rd: most distant dog goes first
                rating = rating + H.distance_between(enemy.x, enemy.y, dog.x, dog.y) / 100.
                reach_map:insert(x, y, rating)

                if (rating > max_rating) then
                    max_rating = rating
                    best_dog, best_enemy, best_hex = dog, enemy, { x, y }
                end
            end)
        end
    end

    -- If we found a move, we do it, and attack if possible
    if best_dog then
        AH.robust_move_and_attack(ai, best_dog, best_hex, best_enemy)
        return
    end

    -- If we got here, no enemies to attack where found, so we go on to block other enemies
    local radius = cfg.attention_distance or 8
    local enemies = get_enemies(cfg, radius)

    -- We also need to remove dogs that have no moves left, since selection was done on attacks_left
    for i=#dogs,1,-1 do
        if (dogs[i].moves == 0) then
            table.remove(dogs, i)
        end
    end
    if (not dogs[1]) then return end

    -- Find closest sheep/enemy pair first
    local min_dist, closest_sheep, closest_enemy = 9e99
    for _,enemy in ipairs(enemies) do
        for _,single_sheep in ipairs(sheep) do
            local dist = H.distance_between(enemy.x, enemy.y, single_sheep.x, single_sheep.y)
            if dist < min_dist then
                min_dist = dist
                closest_sheep, closest_enemy = single_sheep, enemy
            end
        end
    end

    -- Move dogs in between enemies and sheep
    max_rating, best_dog, best_hex = -9e99
    for _,dog in ipairs(dogs) do
        local reach_map = AH.get_reachable_unocc(dog)
        reach_map:iter( function(x, y, v)
            -- We want equal distance between enemy and closest sheep
            local rating = - math.abs(
                H.distance_between(x, y, closest_sheep.x, closest_sheep.y)
                - H.distance_between(x, y, closest_enemy.x, closest_enemy.y)
            ) * 100
            -- 2nd: closeness to sheep
            rating = rating - H.distance_between(x, y, closest_sheep.x, closest_sheep.y)
            reach_map:insert(x, y, rating)
            -- 3rd: most distant dog goes first
            rating = rating + H.distance_between(closest_enemy.x, closest_enemy.y, dog.x, dog.y) / 100.
            reach_map:insert(x, y, rating)

            if (rating > max_rating) then
                max_rating, best_hex, best_dog = rating, { x, y }, dog
            end
        end)
    end

    AH.movefull_stopunit(ai, best_dog, best_hex)
end

return ca_herding_attack_close_enemy
