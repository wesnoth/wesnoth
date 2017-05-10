local H = wesnoth.require "helper"
local LS = wesnoth.require "location_set"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local MAISD = wesnoth.require "ai/micro_ais/micro_ai_self_data.lua"

local function bottleneck_is_my_territory(map, enemy_map)
    -- Create map that contains 'true' for all hexes that are
    -- on the AI's side of the map

    -- Get copy of leader to do pathfinding from each hex to the
    -- front-line hexes, both own (stored in @map) and enemy (@enemy_map) front-line hexes
    -- If there is no leader, use first unit found
    local unit = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]
    if (not unit) then unit = wesnoth.get_units { side = wesnoth.current.side }[1] end
    local dummy_unit = wesnoth.copy_unit(unit)

    local territory_map = LS.create()
    local width, height = wesnoth.get_map_size()
    for x = 1,width do
        for y = 1,height do
            -- The hex might have been covered already previously
            if (not territory_map:get(x,y)) then
                dummy_unit.x, dummy_unit.y = x, y

                -- Find lowest movement cost to own front-line hexes
                local min_cost, best_path = 9e99
                map:iter(function(xm, ym, v)
                    local path, cost = AH.find_path_with_shroud(dummy_unit, xm, ym, { ignore_units = true })
                    if (cost < min_cost) then
                       min_cost, best_path = cost, path
                    end
                end)

                -- And the same to the enemy front line
                local min_cost_enemy, best_path_enemy = 9e99
                enemy_map:iter(function(xm, ym, v)
                    local path, cost = AH.find_path_with_shroud(dummy_unit, xm, ym, { ignore_units = true })
                    if (cost < min_cost_enemy) then
                       min_cost_enemy, best_path_enemy = cost, path
                    end
                end)

                -- We can set the flags for the hexes along the entire path
                -- for efficiency reasons (this is pretty slow, esp. on large maps)
                if (min_cost < min_cost_enemy) then
                    for _,step in ipairs(best_path) do
                        territory_map:insert(step[1], step[2], true)
                    end
                else  -- We do need to use 0's in this case though, false won't work
                    for _,step in ipairs(best_path_enemy) do
                        territory_map:insert(step[1], step[2], 0)
                    end
                end
            end
        end
    end

    -- Now we need to go over it again and delete all the zeros
    territory_map:iter(function(x, y, v)
        if (territory_map:get(x, y) == 0) then territory_map:remove(x, y) end
    end)

    return territory_map
end

local function bottleneck_triple_from_keys(key_x, key_y, max_value)
    -- Turn comma-separated lists of values in @key_x,@key_y into a location set.
    -- Add a rating that has @max_value as its maximum, differentiated by order in the list.
    local coords = {}
    for x in string.gmatch(key_x, "%d+") do
        table.insert(coords, { x })
    end
    local i = 1
    for y in string.gmatch(key_y, "%d+") do
        table.insert(coords[i], y)
        table.insert(coords[i], max_value + 10 - i * 10)
        i = i + 1
    end

    return LS.of_triples(coords)
end

local function bottleneck_create_positioning_map(max_value, data)
    -- Create the positioning maps for the healers and leaders, if not given by WML keys
    -- @max_value: the rating value for the first hex in the set
    -- data.BD_def_map must have been created when this function is called.

    -- Find all locations adjacent to def_map.
    -- This might include hexes on the line itself.
    -- Only store those that are not in enemy territory.
    local map = LS.create()
    data.BD_def_map:iter(function(x, y, v)
        for xa,ya in H.adjacent_tiles(x, y) do
            if data.BD_is_my_territory:get(xa, ya) then
                local rating = data.BD_def_map:get(x, y) or 0
                rating = rating + (map:get(xa, ya) or 0)
                map:insert(xa, ya, rating)
            end
        end
    end)

    -- We need to sort the map, and assign descending values
    local locs = map:to_triples()
    table.sort(locs, function(a, b) return a[3] > b[3] end)
    for i,loc in ipairs(locs) do loc[3] = max_value + 10 - i * 10 end
    map = LS.of_triples(locs)

    -- We merge the defense map into this, as healers/leaders (by default)
    -- can take position on the front line
    map:union_merge(data.BD_def_map,
        function(x, y, v1, v2) return v1 or v2 end
    )

    return map
end

local function bottleneck_get_rating(unit, x, y, has_leadership, is_healer, data)
    -- Calculate rating of a unit @unit at coordinates (@x,@y).
    -- Don't want to extract @is_healer and @has_leadership inside this function, as it is very slow.
    -- Thus they are provided as parameters from the calling function.

    local rating = 0

    -- Defense positioning rating
    -- We exclude healers/leaders here, as we don't necessarily want them on the front line
    if (not is_healer) and (not has_leadership) then
        rating = data.BD_def_map:get(x, y) or 0
    end

    -- Healer positioning rating
    if is_healer then
        local healer_rating = data.BD_healer_map:get(x, y) or 0
        if (healer_rating > rating) then rating = healer_rating end
    end

    -- Leadership unit positioning rating
    if has_leadership then
        local leadership_rating = data.BD_leadership_map:get(x, y) or 0

        -- If leadership unit is injured -> prefer hexes next to healers
        if (unit.hitpoints < unit.max_hitpoints) then
            for xa,ya in H.adjacent_tiles(x, y) do
                local adjacent_unit = wesnoth.get_unit(xa, ya)
                if adjacent_unit and (adjacent_unit.__cfg.usage == "healer") then
                    leadership_rating = leadership_rating + 100
                    break
                end
            end
        end

        if (leadership_rating > rating) then rating = leadership_rating end
    end

    -- Injured unit positioning
    if (unit.hitpoints < unit.max_hitpoints) then
        local healing_rating = data.BD_healing_map:get(x, y) or 0
        if (healing_rating > rating) then rating = healing_rating end
    end

    -- If this did not produce a positive rating, we add a
    -- distance-based rating, to get units to the bottleneck in the first place
    if (rating <= 0) and data.BD_is_my_territory:get(x, y) then
        local combined_dist = 0
        data.BD_def_map:iter(function(x_def, y_def, v)
            combined_dist = combined_dist + H.distance_between(x, y, x_def, y_def)
        end)
        combined_dist = combined_dist / data.BD_def_map:size()
        rating = 1000 - combined_dist * 10.
    end

    -- Now add the unit specific rating.
    if (rating > 0) then
        rating = rating + unit.hitpoints/10. + unit.experience/100.
    end

    return rating
end

local function bottleneck_move_out_of_way(unit_in_way, data)
    -- Find the best move out of the way for a unit @unit_in_way and choose the
    -- shortest possible move. Returns nil if no move was found.

    if (unit_in_way.side ~= wesnoth.current.side) then return nil end

    local reach = wesnoth.find_reach(unit_in_way)

    local all_units = AH.get_visible_units(wesnoth.current.side)
    local occ_hexes = LS:create()
    for _,unit in ipairs(all_units) do
        occ_hexes:insert(unit.x, unit.y)
    end

    local best_reach, best_hex = -9e99
    for _,loc in ipairs(reach) do
        if data.BD_is_my_territory:get(loc[1], loc[2]) and (not occ_hexes:get(loc[1], loc[2])) then
            -- Criterion: MP left after the move has been done
            if (loc[3] > best_reach) then
                best_reach, best_hex = loc[3], { loc[1], loc[2] }
            end
        end
    end

    return best_hex
end

local ca_bottleneck_move = {}

function ca_bottleneck_move:evaluation(cfg, data)
    if cfg.active_side_leader and
        (not MAISD.get_mai_self_data(data, cfg.ai_id, "side_leader_activated"))
    then
        local can_still_recruit = false  -- Enough gold left for another recruit?
        for _,recruit_type in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
            if (wesnoth.unit_types[recruit_type].cost <= wesnoth.sides[wesnoth.current.side].gold) then
                can_still_recruit = true
                break
            end
        end
        if (not can_still_recruit) then
            MAISD.set_mai_self_data(data, cfg.ai_id, { side_leader_activated = true })
        end
    end

    local units = {}
    if MAISD.get_mai_self_data(data, cfg.ai_id, "side_leader_activated") then
        units = AH.get_units_with_moves { side = wesnoth.current.side }
    else
        units = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no' }
    end
    if (not units[1]) then return 0 end

    -- Set up the array that tells the AI where to defend the bottleneck
    data.BD_def_map = bottleneck_triple_from_keys(cfg.x, cfg.y, 10000)

    -- Territory map, describing which hex is on AI's side of the bottleneck
    -- This one is a bit expensive, esp. on large maps -> don't delete every move and reuse
    -- However, after a reload, data.BD_is_my_territory is an empty string
    --  -> need to recalculate in that case also  (the reason is that is_my_territory is not a WML table)
    if (not data.BD_is_my_territory) or (type(data.BD_is_my_territory) == 'string') then
        local enemy_map = bottleneck_triple_from_keys(cfg.enemy_x, cfg.enemy_y, 10000)
        data.BD_is_my_territory = bottleneck_is_my_territory(data.BD_def_map, enemy_map)
    end

    -- Healer positioning map
    if cfg.healer_x and cfg.healer_y then
        data.BD_healer_map = bottleneck_triple_from_keys(cfg.healer_x, cfg.healer_y, 5000)
    else
        data.BD_healer_map = bottleneck_create_positioning_map(5000, data)
    end
    -- Use def_map values for any healer hexes that are defined in def_map as well
    data.BD_healer_map:inter_merge(data.BD_def_map,
        function(x, y, v1, v2) return v2 or v1 end
    )

    -- Leadership position map
    if cfg.leadership_x and cfg.leadership_y then
        data.BD_leadership_map = bottleneck_triple_from_keys(cfg.leadership_x, cfg.leadership_y, 4000)
    else
        data.BD_leadership_map = bottleneck_create_positioning_map(4000, data)
    end
    -- Use def_map values for any leadership hexes that are defined in def_map as well
    data.BD_leadership_map:inter_merge(data.BD_def_map,
        function(x, y, v1, v2) return v2 or v1 end
    )

    -- Healing map: positions next to healers
    -- Healers get moved with higher priority, so don't need to check their MP
    local healers = wesnoth.get_units { side = wesnoth.current.side, ability = "healing" }
    data.BD_healing_map = LS.create()
    for _,healer in ipairs(healers) do
        for xa,ya in H.adjacent_tiles(healer.x, healer.y) do
            -- Cannot be on the line, and needs to be in own territory
            if data.BD_is_my_territory:get(xa, ya) then
                local min_dist = 9e99
                data.BD_def_map:iter( function(xd, yd, vd)
                    local dist_line = H.distance_between(xa, ya, xd, yd)
                    if (dist_line < min_dist) then min_dist = dist_line end
                end)
                if (min_dist > 0) then
                    data.BD_healing_map:insert(xa, ya, 3000 + min_dist)  -- Farther away from enemy is good
                end
            end
        end
    end

    -- Now on to evaluating possible moves:
    -- First, get the rating of all units in their current positions
    -- A move is only considered if it improves the overall rating,
    -- that is, its rating must be higher than:
    --   1. the rating of the unit on the target hex (if there is one)
    --   2. the rating of the currently considered unit on its current hex

    local all_units = wesnoth.get_units { side = wesnoth.current.side }
    local current_rating_map = LS.create()

    for _,unit in ipairs(all_units) do
        -- Is this a healer or leadership unit?
        local is_healer = (unit.__cfg.usage == "healer")
        local has_leadership = AH.has_ability(unit, "leadership")

        local rating = bottleneck_get_rating(unit, unit.x, unit.y, has_leadership, is_healer, data)
        current_rating_map:insert(unit.x, unit.y, rating)

        -- A unit that cannot move any more, (or at least cannot move out of the way)
        -- must be considered to have a very high rating (it's in the best position
        -- it can possibly achieve)
        local best_move_away = bottleneck_move_out_of_way(unit, data)
        if (not best_move_away) then current_rating_map:insert(unit.x, unit.y, 20000) end
    end

    local enemies = AH.get_attackable_enemies()
    local attacks = {}
    for _,enemy in ipairs(enemies) do
        for xa,ya in H.adjacent_tiles(enemy.x, enemy.y) do
            if data.BD_is_my_territory:get(xa, ya) then
                local unit_in_way = wesnoth.get_unit(xa, ya)
                if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way)) then
                    unit_in_way = nil
                end
                local data = { x = xa, y = ya,
                    defender = enemy,
                    defender_level = enemy.level,
                    unit_in_way = unit_in_way
                }
                table.insert(attacks, data)
            end
        end
    end

    -- Get a map of the allies, as hexes occupied by allied units count as
    -- reachable, but must be excluded. This could also be done below by
    -- using bottleneck_move_out_of_way(), but this is much faster
    local allies = AH.get_visible_units(wesnoth.current.side, {
        { "filter_side", { { "allied_with", { side = wesnoth.current.side } } } },
        { "not", { side = wesnoth.current.side } }
    })
    local allies_map = LS.create()
    for _,ally in ipairs(allies) do
        allies_map:insert(ally.x, ally.y)
    end

    local max_rating, best_unit, best_hex = 0
    for _,unit in ipairs(units) do
        local is_healer = (unit.__cfg.usage == "healer")
        local has_leadership = AH.has_ability(unit, "leadership")

        local reach = wesnoth.find_reach(unit)
        for _,loc in ipairs(reach) do
            local rating = bottleneck_get_rating(unit, loc[1], loc[2], has_leadership, is_healer, data)

            -- A move is only considered if it improves the overall rating,
            -- that is, its rating must be higher than:
            --   1. the rating of the unit on the target hex (if there is one)
            if current_rating_map:get(loc[1], loc[2])
                and (current_rating_map:get(loc[1], loc[2]) >= rating)
            then
                rating = 0
            end

            --   2. the rating of the currently considered unit on its current hex
            if (rating <= current_rating_map:get(unit.x, unit.y)) then rating = 0 end

            -- If the target hex is occupied, give it a small penalty
            if current_rating_map:get(loc[1], loc[2]) then rating = rating - 0.001 end

            -- Also need to exclude hexes occupied by an allied unit
            if allies_map:get(loc[1], loc[2]) then rating = 0 end

            -- Now only valid and possible moves should have a rating > 0
            if (rating > max_rating) then
                max_rating, best_unit, best_hex = rating, unit, { loc[1], loc[2] }
            end

            -- Finally, we check whether a level-up attack is possible from this hex
            -- Level-up-attacks will always get a rating greater than any move
            for _,attack in ipairs(attacks) do
                -- Only do calc. if there's a theoretical chance for leveling up (speeds things up a lot)
                if (attack.x == loc[1]) and (attack.y == loc[2]) and
                    (unit.max_experience - unit.experience <= 8 * attack.defender_level)
                then
                    for n_weapon,weapon in ipairs(unit.attacks) do
                        local att_stats, def_stats = BC.simulate_combat_loc(unit, { attack.x, attack.y }, attack.defender, n_weapon)

                        -- Execute level-up attack when:
                        -- 1. max_experience-experience <= target.level and chance to die = 0
                        -- 2. max_experience-experience <= target.level*8 and chance to die = 0
                        --   and chance to kill > 66% and remaining av hitpoints > 20
                        -- #1 is a definite level up, #2 is not, so #1 gets priority
                        local level_up_rating = 0
                        if (unit.max_experience - unit.experience <= attack.defender_level) then
                            if (att_stats.hp_chance[0] == 0) then
                                -- Weakest enemy is best (favors stronger weapon)
                                level_up_rating = 15000 - def_stats.average_hp
                            end
                        else
                            if (unit.max_experience - unit.experience <= 8 * attack.defender_level)
                                and (att_stats.hp_chance[0] == 0)
                                and (def_stats.hp_chance[0] >= 0.66)
                                and (att_stats.average_hp >= 20)
                            then
                                -- Strongest attacker and weakest enemy is best
                                level_up_rating = 14000 + att_stats.average_hp - def_stats.average_hp / 2.
                            end
                        end

                        -- Small penalty if there's a unit in the way
                        -- We also need to check whether this unit can actually move out of the way
                        if attack.unit_in_way then
                            if bottleneck_move_out_of_way(attack.unit_in_way, data) then
                                level_up_rating = level_up_rating - 0.001
                            else
                                level_up_rating = 0
                            end
                        end

                        if (level_up_rating > max_rating) then
                            max_rating, best_unit, best_hex = level_up_rating, unit, { loc[1], loc[2] }
                            data.BD_level_up_defender = attack.defender
                            data.BD_level_up_weapon = n_weapon
                        end
                    end
                end
            end
        end
    end

    -- Set the variables for the exec() function
    if (not best_hex) then
        data.BD_bottleneck_moves_done = true
    else
        -- If there's another unit in the best location, moving it out of the way becomes the best move
        local unit_in_way = wesnoth.get_units { x = best_hex[1], y = best_hex[2],
            { "not", { id = best_unit.id } }
        }[1]
        if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way)) then
            unit_in_way = nil
        end

        if unit_in_way then
            best_hex = bottleneck_move_out_of_way(unit_in_way, data)
            best_unit = unit_in_way
            data.BD_level_up_defender = nil
            data.BD_level_up_weapon = nil
        end

        data.BD_bottleneck_moves_done = false
        data.BD_unit, data.BD_hex = best_unit, best_hex
    end

    return cfg.ca_score
end

function ca_bottleneck_move:execution(cfg, data)
    if data.BD_bottleneck_moves_done then
        local units = {}
        if MAISD.get_mai_self_data(data, cfg.ai_id, "side_leader_activated") then
            units = AH.get_units_with_moves { side = wesnoth.current.side }
        else
            units = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no' }
        end

        for _,unit in ipairs(units) do
            AH.checked_stopunit_moves(ai, unit)
        end
    else
        -- Don't want full move, as this might be stepping out of the way
        local cfg = { partial_move = true, weapon = data.BD_level_up_weapon }
        AH.robust_move_and_attack(ai, data.BD_unit, data.BD_hex, data.BD_level_up_defender, cfg)
    end

    -- Now delete almost everything
    -- Keep only data.BD_is_my_territory because it is very expensive
    data.BD_unit, data.BD_hex = nil, nil
    data.BD_level_up_defender, data.BD_level_up_weapon = nil, nil
    data.BD_bottleneck_moves_done = nil
    data.BD_def_map, data.BD_healer_map, data.BD_leadership_map, data.BD_healing_map = nil, nil, nil, nil
end

return ca_bottleneck_move
