local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_sheep(cfg)
    local sheep = wesnoth.get_units {
        side = wesnoth.current.side,
        { "and", cfg.filter_second }
    }
    return sheep
end

local function get_dogs(cfg)
    local dogs = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter }
    }
    return dogs
end

local function get_enemies(cfg, radius)
    local enemies = wesnoth.get_units {
        { "filter_side", { {"enemy_of", {side = wesnoth.current.side} } } },
        { "filter_location",
            { radius = radius,
            { "filter", { side = wesnoth.current.side, { "and", cfg.filter_second } } } }
        }
    }
    return enemies
end

local ca_herding_attack_close_enemy = {}

function ca_herding_attack_close_enemy:evaluation(ai, cfg)
    -- Any enemy within attention_distance (default = 8) hexes of a sheep will get the dogs' attention
    -- with enemies within attack_distance (default: 4) being attacked
    if not get_sheep(cfg)[1] then return 0 end
    if not get_dogs(cfg)[1] then return 0 end

    local radius = cfg.attention_distance or 8
    if get_enemies(cfg, radius)[1] then return cfg.ca_score end
    return 0
end

function ca_herding_attack_close_enemy:execution(ai, cfg)
    local sheep = get_sheep(cfg)
    local dogs = get_dogs(cfg)
    local sheep = wesnoth.get_units { side = wesnoth.current.side, {"and", cfg.filter_second} }

    -- We start with enemies within attack_distance (default: 4) hexes, which will be attacked
    local radius = cfg.attack_distance or 4
    local enemies = get_enemies(cfg, radius)

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
            AH.checked_attack(ai, best_dog, best_enemy)
        end
        return
    end

    -- If we got here, no enemies to attack where found, so we go on to block other enemies
    --print('Dogs: No enemies close enough to warrant attack')
    -- Now we get all enemies within attention_distance hexes
    local radius = cfg.attention_distance or 8
    local enemies = get_enemies(cfg, radius)

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

return ca_herding_attack_close_enemy
