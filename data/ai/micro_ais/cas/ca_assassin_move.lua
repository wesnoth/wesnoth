local H = wesnoth.require "helper"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "location_set"

local function get_units_target(cfg)
    local units = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", H.get_child(cfg, "filter") }
    }

    local target = wesnoth.get_units {
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
        { "and", H.get_child(cfg, "filter_second") }
    }[1]

    return units, target
end

local function custom_cost(x, y, unit, enemy_rating_map, prefer_map)
    -- Custom cost function for assassin path finding consisting of:
    -- 1. The standard movecost of the units
    -- 2. A penalty for hexes that can be attacked or are blocked by enemies (stored in rating map)
    -- 3. A penalty for non-preferred hexes (prefer_map). This has to be a penalty for
    --    non-preferred hexes rather than a bonus for preferred hexes as the cost function
    --    must return values >=1 for the a* search to work.

    local terrain = wesnoth.get_terrain(x, y)
    local move_cost = wesnoth.unit_movement_cost(unit, terrain)

    move_cost = move_cost + (enemy_rating_map:get(x, y) or 0)

    if prefer_map then
        if (not prefer_map:get(x, y)) then
            move_cost = move_cost + 5
        end
    end

    return move_cost
end

local ca_assassin_move = {}

function ca_assassin_move:evaluation(cfg)
    local units, target = get_units_target(cfg)
    if (not units[1]) then return 0 end
    if (not target) then return 0 end

    return cfg.ca_score
end

function ca_assassin_move:execution(cfg)
    -- We simply move the assassins one at a time
    local units, target = get_units_target(cfg)
    local unit = units[1]

    local enemies = AH.get_visible_units(wesnoth.current.side, {
        { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
        { "not", H.get_child(cfg, "filter_second") }
    })

    -- Maximum damage the enemies can theoretically do for all hexes they can attack
    -- Note: petrified enemies need to be included for the blocked hexes rating below,
    -- but need to be excluded from the damage rating
    local enemy_damage_map = LS.create()
    for _,enemy in ipairs(enemies) do
        if (not enemy.status.petrified) then
            -- Need to "move" enemy next to unit for attack calculation
            -- Do this with a unit copy, so that no actual unit has to be moved
            local enemy_copy = wesnoth.copy_unit(enemy)

            -- First get the reach of the enemy with full moves though
            enemy_copy.moves = enemy_copy.max_moves
            local reach = wesnoth.find_reach(enemy_copy, { ignore_units = true })

            enemy_copy.x = unit.x
            enemy_copy.y = unit.y + 1 -- this even works at map border

            local _, _, att_weapon, _ = wesnoth.simulate_combat(enemy_copy, unit)
            local max_damage = att_weapon.damage * att_weapon.num_blows

            local unit_damage_map = LS.create()
            for _,loc in ipairs(reach) do
                unit_damage_map:insert(loc[1], loc[2], max_damage)
                for xa,ya in H.adjacent_tiles(loc[1], loc[2]) do
                    unit_damage_map:insert(xa, ya, max_damage)
                end
            end

            enemy_damage_map:union_merge(unit_damage_map, function(x, y, v1, v2)
                return (v1 or 0) + v2
            end)
        end
    end

    -- Penalties for damage by enemies
    local enemy_rating_map = LS.create()
    enemy_damage_map:iter(function(x, y, enemy_damage)
        local hit_chance = (wesnoth.unit_defense(unit, wesnoth.get_terrain(x, y))) / 100.

        local rating = hit_chance * enemy_damage
        rating = rating / unit.max_hitpoints
        rating = rating * 5

        enemy_rating_map:insert(x, y, rating)
    end)

    -- Penalties for blocked hexes and ZOC
    local is_skirmisher = wesnoth.unit_ability(unit, "skirmisher")
    for _,enemy in ipairs(enemies) do
        -- Hexes an enemy is on get a very large penalty
        enemy_rating_map:insert(enemy.x, enemy.y, (enemy_rating_map:get(enemy.x, enemy.y) or 0) + 100)

        -- Hexes adjacent to enemies get max_moves penalty
        -- except if AI unit is skirmisher or enemy is level 0 or is petrified
        local zoc_active = (not is_skirmisher)

        if zoc_active then
            if (enemy.level == 0) or enemy.status.petrified then zoc_active = false end
        end

        if zoc_active then
            for xa,ya in H.adjacent_tiles(enemy.x, enemy.y) do
                enemy_rating_map:insert(xa, ya, (enemy_rating_map:get(xa, ya) or 0) + unit.max_moves)
            end
        end
    end

    -- Preferred hexes (do this here once for all hexes, so that it does not need
    -- to get done for every step of the a* search.
    -- We only need to know whether a hex is preferred or not, there's no additional rating.
    local prefer_slf = H.get_child(cfg, "prefer")
    local prefer_map -- want this to be nil, not empty LS if [prefer] tag not given
    if prefer_slf then
        local preferred_hexes = wesnoth.get_locations(prefer_slf)
        prefer_map = LS.create()
        for _,hex in ipairs(preferred_hexes) do
            prefer_map:insert(hex[1], hex[2], true)
        end
    end

    local path, cost = wesnoth.find_path(unit, target.x, target.y,
        function(x, y, current_cost)
            return custom_cost(x, y, unit, enemy_rating_map, prefer_map)
        end
    )

    local path_map = LS.of_pairs(path)

    -- We need to pick the farthest reachable hex along that path
    local farthest_hex = path[1]
    for i = 2,#path do
        local sub_path, sub_cost = AH.find_path_with_shroud(unit, path[i][1], path[i][2])
        if sub_cost <= unit.moves then
            local unit_in_way = wesnoth.get_unit(path[i][1], path[i][2])
            if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way)) then
                farthest_hex = path[i]
            end
        else
            break
        end
    end

    if farthest_hex then
        AH.checked_move_full(ai, unit, farthest_hex[1], farthest_hex[2])
    else
        AH.checked_stopunit_moves(ai, unit)
    end
end

return ca_assassin_move
