local LS = wesnoth.require "location_set"
---@type ai_helper_lib
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local MAISD = wesnoth.require "ai/micro_ais/micro_ai_self_data.lua"
local M = wesnoth.map

local BD_unit, BD_hex
local BD_level_up_defender, BD_level_up_weapon, BD_bottleneck_moves_done
local BD_is_my_territory, BD_def_map, BD_healer_map, BD_leadership_map, BD_healing_map

local function bottleneck_is_my_territory(map, enemy_map)
    -- Create map that contains 'true' for all hexes that are
    -- on the AI's side of the map

    -- Get copy of leader to do pathfinding from each hex to the
    -- front-line hexes, both own (stored in @map) and enemy (@enemy_map) front-line hexes
    -- If there is no leader, use first unit found
    local unit = wesnoth.units.find_on_map { side = wesnoth.current.side, canrecruit = 'yes' }[1]
    if (not unit) then unit = wesnoth.units.find_on_map { side = wesnoth.current.side }[1] end
    local dummy_unit = unit:clone()

    local territory_map = LS.create()
    for x, y in wesnoth.current.map:iter() do
        -- The hex might have been covered already previously
        if (not territory_map:get(x,y)) then
            dummy_unit.x, dummy_unit.y = x, y

            -- Find lowest movement cost to own front-line hexes
            ---@type number, location[]?
            local min_cost, best_path = math.huge, nil
            map:iter(function(xm, ym, v)
                local path, cost = AH.find_path_with_shroud(dummy_unit, xm, ym, { ignore_units = true })
                if (cost < min_cost) then
                    min_cost, best_path = cost, path
                end
            end)

            -- And the same to the enemy front line
            ---@type number, location[]?
            local min_cost_enemy, best_path_enemy = math.huge, nil
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

    -- Now we need to go over it again and delete all the zeros
    territory_map:iter(function(x, y, v)
        if (territory_map:get(x, y) == 0) then territory_map:remove(x, y) end
    end)

    return territory_map
end

local function bottleneck_triple_from_locs(locs, max_value)
    -- Turn comma-separated lists of values in @key_x,@key_y into a location set.
    -- Add a rating that has @max_value as its maximum, differentiated by order in the list.
    local coords = AH.table_copy(locs)
    for i,coord in ipairs(coords) do
        coord[3] = max_value + 10 - i * 10
    end

    return LS.of_triples(coords)
end

local function bottleneck_create_positioning_map(max_value, data)
    -- Create the positioning maps for the healers and leaders, if not given by WML keys
    -- @max_value: the rating value for the first hex in the set
    -- BD_def_map must have been created when this function is called.

    -- Find all locations adjacent to def_map.
    -- This might include hexes on the line itself.
    -- Only store those that are not in enemy territory.
    local map = LS.create()
    BD_def_map:iter(function(x, y, v)
        for xa,ya in wesnoth.current.map:iter_adjacent(x, y) do
            if BD_is_my_territory:get(xa, ya) then
                local rating = BD_def_map:get(x, y) or 0
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
    map:union_merge(BD_def_map,
        function(x, y, v1, v2) return v1 or v2 end
    )

    return map
end

local function bottleneck_get_rating(unit, x, y, has_leadership, is_healer, on_my_territory, data)
    -- Calculate rating of a unit @unit at coordinates (@x,@y).
    -- Don't want to extract @is_healer and @has_leadership inside this function, as it is very slow.
    -- Thus they are provided as parameters from the calling function.

    local rating = 0

    -- Defense positioning rating
    -- We exclude healers/leaders here, as we don't necessarily want them on the front line
    if (not is_healer) and (not has_leadership) then
        rating = BD_def_map:get(x, y) or 0
    end

    -- Healer positioning rating
    if is_healer then
        local healer_rating = BD_healer_map:get(x, y) or 0
        if (healer_rating > rating) then rating = healer_rating end
    end

    -- Leadership unit positioning rating
    if has_leadership then
        local leadership_rating = BD_leadership_map:get(x, y) or 0

        -- If leadership unit is injured -> prefer hexes next to healers
        if (unit.hitpoints < unit.max_hitpoints) then
            for xa,ya in wesnoth.current.map:iter_adjacent(x, y) do
                local adjacent_unit = wesnoth.units.get(xa, ya)
                if adjacent_unit and (adjacent_unit.usage == "healer") then
                    leadership_rating = leadership_rating + 100
                    break
                end
            end
        end

        if (leadership_rating > rating) then rating = leadership_rating end
    end

    -- Injured unit positioning
    if (unit.hitpoints < unit.max_hitpoints) then
        local healing_rating = BD_healing_map:get(x, y) or 0
        if (healing_rating > rating) then rating = healing_rating end
    end

    -- If this did not produce a positive rating, we add a
    -- distance-based rating, to get units to the bottleneck in the first place
    if (rating <= 0) then
        local combined_dist = 0
        BD_def_map:iter(function(x_def, y_def, v)
            combined_dist = combined_dist + M.distance_between(x, y, x_def, y_def)
        end)
        combined_dist = combined_dist / BD_def_map:size()

        if BD_is_my_territory:get(x, y) then
            rating = 1000 - combined_dist * 10.
        elseif (not on_my_territory) then
            -- If the unit is itself on enemy territory, we also give an (even smaller) distance-based rating
            -- in order to make it move toward own territory if it cannot get there on the current move
            rating = 10 - combined_dist / 100.
        end
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

    local reach = wesnoth.paths.find_reach(unit_in_way)

    local all_units = AH.get_visible_units(wesnoth.current.side)
    local occ_hexes = LS:create()
    for _,unit in ipairs(all_units) do
        occ_hexes:insert(unit.x, unit.y)
    end

    local best_reach, best_hex = - math.huge, nil
    for _,loc in ipairs(reach) do
        if BD_is_my_territory:get(loc[1], loc[2]) and (not occ_hexes:get(loc[1], loc[2])) then
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
        units = AH.get_units_with_moves { side = wesnoth.current.side, wml.tag["and"] ( wml.get_child(cfg, "filter") ) }
    else
        units = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no', wml.tag["and"] ( wml.get_child(cfg, "filter") ) }
    end
    if (not units[1]) then return 0 end

    -- Set up the array that tells the AI where to defend the bottleneck
    local locs = AH.get_multi_named_locs_xy('', cfg)
    BD_def_map = bottleneck_triple_from_locs(locs, 10000)

    -- Territory map, describing which hex is on AI's side of the bottleneck
    -- This one is a bit expensive, esp. on large maps -> don't delete every move and reuse
    -- However, after a reload, BD_is_my_territory is empty
    --  -> need to recalculate in that case also
    if (not BD_is_my_territory) or (type(BD_is_my_territory) == 'string') then
        local enemy_locs = AH.get_multi_named_locs_xy('enemy', cfg)
        local enemy_map = bottleneck_triple_from_locs(enemy_locs, 10000)
        BD_is_my_territory = bottleneck_is_my_territory(BD_def_map, enemy_map)
    end

    -- Healer positioning map
    local healer_locs = AH.get_multi_named_locs_xy('healer', cfg)
    if healer_locs[1] then
        BD_healer_map = bottleneck_triple_from_locs(healer_locs, 5000)
    else
        BD_healer_map = bottleneck_create_positioning_map(5000, data)
    end
    -- Use def_map values for any healer hexes that are defined in def_map as well
    BD_healer_map:inter_merge(BD_def_map,
        function(x, y, v1, v2) return v2 or v1 end
    )

    -- Leadership position map
    local leadership_locs = AH.get_multi_named_locs_xy('leadership', cfg)
    if leadership_locs[1] then
        BD_leadership_map = bottleneck_triple_from_locs(leadership_locs, 4000)
    else
        BD_leadership_map = bottleneck_create_positioning_map(4000, data)
    end
    -- Use def_map values for any leadership hexes that are defined in def_map as well
    BD_leadership_map:inter_merge(BD_def_map,
        function(x, y, v1, v2) return v2 or v1 end
    )

    -- Healing map: positions next to healers
    -- Healers get moved with higher priority, so don't need to check their MP
    local healers = wesnoth.units.find_on_map { side = wesnoth.current.side, ability = "healing" }
    BD_healing_map = LS.create()
    for _,healer in ipairs(healers) do
        for xa,ya in wesnoth.current.map:iter_adjacent(healer) do
            -- Cannot be on the line, and needs to be in own territory
            if BD_is_my_territory:get(xa, ya) then
                local min_dist = math.huge
                BD_def_map:iter( function(xd, yd, vd)
                    local dist_line = M.distance_between(xa, ya, xd, yd)
                    if (dist_line < min_dist) then min_dist = dist_line end
                end)
                if (min_dist > 0) then
                    BD_healing_map:insert(xa, ya, 3000 + min_dist)  -- Farther away from enemy is good
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

    local all_units = wesnoth.units.find_on_map { side = wesnoth.current.side }
    local current_rating_map = LS.create()

    for _,unit in ipairs(all_units) do
        -- Is this a healer or leadership unit?
        local is_healer = (unit.usage == "healer")
        local has_leadership = unit:matches { ability_type = "leadership" }
        local on_my_territory = BD_is_my_territory:get(unit.x, unit.y)

        local rating = bottleneck_get_rating(unit, unit.x, unit.y, has_leadership, is_healer, on_my_territory, data)
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
        for xa,ya in wesnoth.current.map:iter_adjacent(enemy) do
            if BD_is_my_territory:get(xa, ya) then
                ---@type unit?
                local unit_in_way = wesnoth.units.get(xa, ya)
                if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way)) then
                    unit_in_way = nil
                end
                local defender_data = { x = xa, y = ya,
                    defender = enemy,
                    defender_level = enemy.level,
                    unit_in_way = unit_in_way
                }
                table.insert(attacks, defender_data)
            end
        end
    end

    -- Get a map of the allies, as hexes occupied by allied units count as
    -- reachable, but must be excluded. This could also be done below by
    -- using bottleneck_move_out_of_way(), but this is much faster
    local allies = AH.get_visible_units(wesnoth.current.side, {
        wml.tag.filter_side { wml.tag.allied_with { side = wesnoth.current.side } },
        wml.tag["not"] ( { side = wesnoth.current.side } )
    })
    local allies_map = LS.create()
    for _,ally in ipairs(allies) do
        allies_map:insert(ally.x, ally.y)
    end

    local max_rating, best_unit, best_hex = 0, nil, nil
    for _,unit in ipairs(units) do
        wesnoth.interface.handle_user_interact()
        local is_healer = (unit.usage == "healer")
        local has_leadership = unit:matches { ability_type = "leadership" }
        local on_my_territory = BD_is_my_territory:get(unit.x, unit.y)

        local reach = wesnoth.paths.find_reach(unit)
        for _,loc in ipairs(reach) do
            local rating = bottleneck_get_rating(unit, loc[1], loc[2], has_leadership, is_healer, on_my_territory, data)

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
                local eff_defender_level = attack.defender_level
                if (eff_defender_level == 0) then eff_defender_level = 0.5 end
                if (attack.x == loc[1]) and (attack.y == loc[2]) and
                    (unit.max_experience - unit.experience <= wesnoth.game_config.kill_experience * eff_defender_level)
                then
                    for n_weapon,weapon in ipairs(unit.attacks) do
                        local att_stats, def_stats = BC.simulate_combat_loc(unit, { attack.x, attack.y }, attack.defender, n_weapon)

                        -- Execute level-up attack when:
                        -- 1. max_experience-experience <= target.level*combat_experience and chance to die = 0
                        -- 2. kill_experience enough for leveling up and chance to die = 0
                        --   and chance to kill > 66% and remaining av hitpoints > 20
                        -- #1 is a definite level up, #2 is not, so #1 gets priority
                        local level_up_rating = 0
                        -- Note: in this conditional it needs to be the real defender level, not eff_defender_level
                        if (unit.max_experience - unit.experience <= wesnoth.game_config.combat_experience * attack.defender_level) then
                            if (att_stats.hp_chance[0] == 0) then
                                -- Weakest enemy is best (favors stronger weapon)
                                level_up_rating = 15000 - def_stats.average_hp
                            end
                        elseif (att_stats.hp_chance[0] == 0)
                            and (def_stats.hp_chance[0] >= 0.66)
                            and (att_stats.average_hp >= 20)
                        then
                            -- Strongest attacker and weakest enemy is best
                            level_up_rating = 14000 + att_stats.average_hp - def_stats.average_hp / 2.
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
                            BD_level_up_defender = attack.defender
                            BD_level_up_weapon = n_weapon
                        end
                    end
                end
            end
        end
    end

    -- Set the variables for the exec() function
    if (not best_hex) then
        BD_bottleneck_moves_done = true
    else
        -- If there's another unit in the best location, moving it out of the way becomes the best move
        ---@type unit?
        local unit_in_way = wesnoth.units.find_on_map { x = best_hex[1], y = best_hex[2],
            wml.tag["not"] { id = best_unit.id }
        }[1]
        --- TODO: best_hex should be indexed as x and y!
        if (not AH.is_visible_unit(wesnoth.current.side, unit_in_way)) then
            unit_in_way = nil
        end

        if unit_in_way then
            best_hex = bottleneck_move_out_of_way(unit_in_way, data)
            best_unit = unit_in_way
            BD_level_up_defender = nil
            BD_level_up_weapon = nil
        end

        BD_bottleneck_moves_done = false
        BD_unit, BD_hex = best_unit, best_hex
    end

    return cfg.ca_score
end

function ca_bottleneck_move:execution(cfg, data)
    if BD_bottleneck_moves_done then
        local units = {}
        if MAISD.get_mai_self_data(data, cfg.ai_id, "side_leader_activated") then
            units = AH.get_units_with_moves { side = wesnoth.current.side, wml.tag["and"] ( wml.get_child(cfg, "filter") ) }
        else
            units = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no', wml.tag["and"] ( wml.get_child(cfg, "filter") ) }
        end

        for _,unit in ipairs(units) do
            AH.checked_stopunit_moves(ai, unit)
        end
    else
        -- Don't want full move, as this might be stepping out of the way
        local move_attack_cfg = { partial_move = true, weapon = BD_level_up_weapon }
        AH.robust_move_and_attack(ai, BD_unit, BD_hex, BD_level_up_defender, move_attack_cfg)
    end

    -- Now delete almost everything
    -- Keep only BD_is_my_territory because it is very expensive
    BD_unit, BD_hex = nil, nil
    BD_level_up_defender, BD_level_up_weapon = nil, nil
    BD_bottleneck_moves_done = nil
    BD_def_map, BD_healer_map, BD_leadership_map, BD_healing_map = nil, nil, nil, nil
end

return ca_bottleneck_move
