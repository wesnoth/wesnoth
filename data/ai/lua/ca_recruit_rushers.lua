-- cfg: parameters for configuring recruitment, read from the candidate action's [args] tag
--    ca_score: score of the candidate action
--      (default: 196000 as defined in AI_CA_RECRUIT_RUSHERS_SCORE)
--    randomness: a measure of randomness in recruitment
--      higher absolute values increase randomness, with values above about 3 being close to completely random
--      (default = 0.1)
--    high_level_fraction: the approximate fraction of units of level 2 or higher to be recruited
--      (default = 0)
--
-- Note: the recruiting code assumes full knowledge of units on the map and the recruit lists of other sides for the purpose of
--   finding the best unit types to recruit. It does not work otherwise. It assumes normal vision of the AI side (that is, it disregards
--   hidden enemy units) for determining from which keep hex the leader should recruit and on which castle hexes to recruit new units

local AH = wesnoth.require "ai/lua/ai_helper.lua"
local M = wesnoth.map
local LS = wesnoth.require "location_set"

math.randomseed(os.time())
local recruit_data = { turn = wesnoth.current.turn }


local function ca_castle_switch()
    for ai_tag in wml.child_range(wesnoth.sides[wesnoth.current.side].__cfg, 'ai') do
        for stage in wml.child_range(ai_tag, 'stage') do
            for ca in wml.child_range(stage, 'candidate_action') do
                if ca.location and string.find(ca.location, 'ca_castle_switch') then
                    return wesnoth.dofile("ai/lua/ca_castle_switch.lua")
                end
            end
        end
    end

    -- Avoid going through the __cfg above every time the evaluation function
    -- is called if the castle switch CA does not exist
    local dummy_castle_switch = {}
    function dummy_castle_switch:evaluation(cfg, data, filter_own, leader) return 0 end
    return dummy_castle_switch
end


local function no_village_cost(recruit_id)
    return wesnoth.unit_types[recruit_id].cost+wesnoth.unit_types[recruit_id].level+wesnoth.sides[wesnoth.current.side].village_gold
end


local function get_hp_efficiency(table, recruit_id)
    -- raw durability is a function of hp and the regenerates ability
    -- efficiency decreases faster than cost increases to avoid recruiting many expensive units
    -- there is a requirement for bodies in order to block movement

    -- There is currently an assumption that opponents will average about 15 damage per strike
    -- and that two units will attack per turn until the unit dies to estimate the number of hp
    -- gained from regeneration
    local effective_hp = wesnoth.unit_types[recruit_id].max_hitpoints

    local unit = wesnoth.units.create {
        type = recruit_id,
        random_traits = false,
        name = "X",
        random_gender = false
    }
    -- Find the best regeneration ability and use it to estimate hp regained by regeneration
    local abilities = wml.get_child(unit.__cfg, "abilities")
    local regen_amount = 0
    if abilities then
        for regen in wml.child_range(abilities, "regenerate") do
            if tonumber(regen.value) and tonumber(regen.value) > regen_amount then
                regen_amount = tonumber(regen.value)
            end
        end
        effective_hp = effective_hp + (regen_amount * effective_hp/30)
    end
    local hp_score = math.max(math.log(effective_hp/20),0.01)
    local efficiency = hp_score/(wesnoth.unit_types[recruit_id].cost^2)
    local no_village_efficiency = hp_score/(no_village_cost(recruit_id)^2)

    table[recruit_id] = {efficiency, no_village_efficiency}
    return {efficiency, no_village_efficiency}
end
local efficiency = {}
setmetatable(efficiency, { __index = get_hp_efficiency })


local function poisonable(unit)
    return not unit.status.unpoisonable
end


local function drainable(unit)
    return not unit.status.undrainable
end


local function get_best_defense(unit)
    local terrain_archetypes = { "Wo", "Ww", "Wwr", "Ss", "Gt", "Ds", "Ft", "Hh", "Mm", "Vi", "Ch", "Uu", "At", "Qt", "Xt", "Tt", "Rt" }
    local best_defense = 100

    for i, terrain in ipairs(terrain_archetypes) do
        local defense = 100 - unit:defense_on(terrain)
        if defense < best_defense then
            best_defense = defense
        end
    end

    return best_defense
end


local function analyze_enemy_unit(enemy_type, ally_type)
    local function get_best_attack(attacker, defender, defender_defense, attacker_defense, can_poison)
        -- Try to find the average damage for each possible attack and return the one that deals the most damage.
        -- Would be preferable to call simulate combat, but that requires the defender to be on the map according
        -- to documentation and we are looking for hypothetical situations so would have to search for available
        -- locations for the defender that would have the desired defense. We would also need to remove nearby units
        -- in order to ensure that adjacent units are not modifying the result. In addition, the time of day is
        -- assumed to be neutral here, which is not assured in the simulation.
        -- Ideally, this function would be a clone of simulate combat, but run for each time of day in the scenario and on arbitrary terrain.
        -- In several cases this function only approximates the correct value (eg Thunderguard vs Goblin Spearman has damage capped by target health)
        -- In some cases (like poison), this approximation is preferred to the actual value.
        local best_damage = 0
        local best_attack
        local best_poison_damage = 0
        -- Steadfast is currently disabled because it biases the AI too much in favour of Guardsmen
        -- Basically it sees the defender stats for damage and wrongfully concludes that the unit is amazing
        -- This may be rectifiable by looking at retaliation damage as well.
        local steadfast = false

        for attack in wml.child_range(wesnoth.unit_types[attacker.type].__cfg, "attack") do
            local defense = defender_defense
            local poison = false
            local damage_multiplier = 1
            local damage_bonus = 0
            local weapon_damage = attack.damage

            for special in wml.child_range(attack, 'specials') do
                local mod
                if wml.get_child(special, 'poison') and can_poison then
                    poison = true
                end

                -- Handle marksman and magical
                mod = wml.get_child(special, 'chance_to_hit')
                if mod then
                    if tonumber(mod.value) then
                        if mod.cumulative then
                            if tonumber(mod.value) > defense then
                                defense = tonumber(mod.value) or 0
                            end
                        else
                            defense = tonumber(mod.value) or 0
                        end
                    elseif mod.add then
                        defense = defense + (tonumber(mod.add) or 0)
                    elseif mod.sub then
                        defense = defense - (tonumber(mod.sub) or 0)
                    elseif mod.multiply then
                        defense = defense * (tonumber(mod.multiply) or 0)
                    elseif mod.divide then
                        defense = defense / (tonumber(mod.divide) or 1)
                    end
                end

                -- Handle most damage specials (assumes all are cumulative)
                mod = wml.get_child(special, 'damage')
                if mod and mod.active_on ~= "defense" then
                    local special_multiplier = 1
                    local special_bonus = 0

                    if tonumber(mod.multiply) then
                        special_multiplier = special_multiplier * tonumber(mod.multiply)
                    end
                    if tonumber(mod.divide) and tonumber(mod.divide) ~= 0 then
                        special_multiplier = special_multiplier / tonumber(mod.divide)
                    end
                    if tonumber(mod.add) then
                        special_bonus = special_bonus + tonumber(mod.add)
                    end
                    if tonumber(mod.subtract) then
                        special_bonus = special_bonus - tonumber(mod.subtract)
                    end

                    if mod.backstab then
                        -- Assume backstab happens on only 1/2 of attacks
                        -- TODO: find out what actual probability of getting to backstab is
                        damage_multiplier = damage_multiplier*(special_multiplier*0.5 + 0.5)
                        damage_bonus = damage_bonus+(special_bonus*0.5)
                        if mod.value then
                            weapon_damage = (weapon_damage + (tonumber(mod.value) or 0) )/2
                        end
                    else
                        damage_multiplier = damage_multiplier*special_multiplier
                        damage_bonus = damage_bonus+special_bonus
                        if mod.value then
                            weapon_damage = tonumber(mod.value) or 0
                        end
                    end
                end
            end

            -- Handle drain for defender
            local drain_recovery = 0
            local defender_attacks = defender.attacks
            for i_d = 1,#defender_attacks do
                local defender_attack = defender_attacks[i_d]
                if (defender_attack.range == attack.range) then
                    for _,sp in ipairs(defender_attack.specials) do
                        if (sp[1] == 'drains') and drainable(attacker) then
                            -- TODO: calculate chance to hit
                            -- currently assumes 50% chance to hit using supplied constant
                            local attacker_resistance = attacker:resistance_against(defender_attack.type)
                            drain_recovery = (defender_attack.damage*defender_attack.number*(100-attacker_resistance)*attacker_defense/2)/10000
                        end
                    end
                end
            end

            defense = defense/100.0
            local resistance = defender:resistance_against(attack.type)
            if steadfast and (resistance > 0) then
                resistance = resistance * 2
                if (resistance > 50) then
                    resistance = 50
                end
            end
            local base_damage = (weapon_damage+damage_bonus)*(100-resistance)*damage_multiplier
            if (resistance < 0) then
                base_damage = base_damage-1
            end
            base_damage = math.floor(base_damage/100 + 0.5)
            if (base_damage < 1) and (attack.damage > 0) then
                -- Damage is always at least 1
                base_damage = 1
            end
            local attack_damage = base_damage*attack.number*defense-drain_recovery

            local poison_damage = 0
            if poison then
                -- Add poison damage * probability of poisoning
                poison_damage = wesnoth.game_config.poison_amount*(1-((1-defense)^attack.number))
            end

            if (not best_attack) or (attack_damage+poison_damage > best_damage+best_poison_damage) then
                best_damage = attack_damage
                best_poison_damage = poison_damage
                best_attack = attack
            end
        end

        return best_attack, best_damage, best_poison_damage
    end

    -- Use cached information when possible: this is expensive
    local analysis = {}
    if not recruit_data.analyses then
        recruit_data.analyses = {}
    else
        if recruit_data.analyses[enemy_type] then
            analysis = recruit_data.analyses[enemy_type] or {}
        end
    end
    if analysis[ally_type] then
        return analysis[ally_type]
    end

    local unit = wesnoth.units.create {
        type = enemy_type,
        random_traits = false,
        name = "X",
        random_gender = false
    }
    local can_poison = poisonable(unit) and (not unit:ability('regenerate'))
    local flat_defense = 100 - unit:defense_on("Gt")
    local best_defense = get_best_defense(unit)

    local recruit = wesnoth.units.create {
        type = ally_type,
        random_traits = false,
        name = "X",
        random_gender = false
    }
    local recruit_flat_defense = 100 - recruit:defense_on("Gt")
    local recruit_best_defense = get_best_defense(recruit)

    local can_poison_retaliation = poisonable(recruit) and (not recruit:ability('regenerate'))
    local best_flat_attack, best_flat_damage, flat_poison = get_best_attack(recruit, unit, flat_defense, recruit_best_defense, can_poison)
    local best_high_defense_attack, best_high_defense_damage, high_defense_poison = get_best_attack(recruit, unit, best_defense, recruit_flat_defense, can_poison)
    local best_retaliation, best_retaliation_damage, retaliation_poison = get_best_attack(unit, recruit, recruit_flat_defense, best_defense, can_poison_retaliation)

    local result = {
        offense = { attack = best_flat_attack, damage = best_flat_damage, poison_damage = flat_poison },
        defense = { attack = best_high_defense_attack, damage = best_high_defense_damage, poison_damage = high_defense_poison },
        retaliation = { attack = best_retaliation, damage = best_retaliation_damage, poison_damage = retaliation_poison }
    }
    analysis[ally_type] = result

    -- Cache result before returning
    recruit_data.analyses[enemy_type] = analysis
    return analysis[ally_type]
end


local function can_slow(unit)
    local attacks = unit.attacks
    for i_a = 1,#attacks do
        for _,sp in ipairs(attacks[i_a].specials) do
            if (sp[1] == 'slow') then
                return true
            end
        end
    end
    return false
end


local function get_hp_ratio_with_gold()
    local function sum_gold_for_sides(side_filter)
        -- sum positive amounts of gold for a set of sides
        -- positive only because it is used to estimate the number of enemy units that could appear
        -- and negative numbers shouldn't subtract from the number of units on the map
        local gold = 0
        local sides = wesnoth.sides.find(side_filter)
        for i,s in ipairs(sides) do
            if s.gold > 0 then
                gold = gold + s.gold
            end
        end

        return gold
    end

    -- Hitpoint ratio of own units / enemy units
    -- Also convert available gold to a hp estimate
    local my_units = AH.get_live_units {
        { "filter_side", {{"allied_with", {side = wesnoth.current.side} }} }
    }
    local enemies = AH.get_live_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }

    local my_hp, enemy_hp = 0, 0
    for i,u in ipairs(my_units) do my_hp = my_hp + u.hitpoints end
    for i,u in ipairs(enemies) do enemy_hp = enemy_hp + u.hitpoints end

    my_hp = my_hp + sum_gold_for_sides({{"allied_with", {side = wesnoth.current.side} }})*2.3
    enemy_hp = enemy_hp+sum_gold_for_sides({{"enemy_of", {side = wesnoth.current.side} }})*2.3
    -- Need to prevent potential divide-by-zero
    if (enemy_hp == 0) then enemy_hp = 1 end
    return my_hp / enemy_hp
end


local function get_current_castle(leader, data)
    if (not data.castle) or (data.castle.x ~= leader.x) or (data.castle.y ~= leader.y) then
        data.castle = {
            locs = AH.get_locations_no_borders {
                { "filter_vision", { side = wesnoth.current.side, visible = 'yes' } },
                { "and", {
                    x = leader.x, y = leader.y, radius = 200,
                    { "filter_radius", { terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*' } }
                }}
            },
            x = leader.x,
            y = leader.y
        }
    end
end


local function init_data(leader)
    local data = {}

    -- Count enemies of each type
    local enemies = AH.get_live_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }}}
    }
    local enemy_counts = {}
    local enemy_types = {}
    local possible_enemy_recruit_count = 0

    local function add_unit_type(unit_type)
        if not enemy_counts[unit_type] then
            table.insert(enemy_types, unit_type)
            enemy_counts[unit_type] = 1
        else
            enemy_counts[unit_type] = enemy_counts[unit_type] + 1
        end
    end

    -- Collect all enemies on map
    for i, unit in ipairs(enemies) do
        add_unit_type(unit.type)
    end
    -- Collect all possible enemy recruits and count them as virtual enemies
    local enemy_sides = wesnoth.sides.find({
        { "enemy_of", {side = wesnoth.current.side} },
        { "has_unit", { canrecruit = true }} })
    for i, side in ipairs(enemy_sides) do
        possible_enemy_recruit_count = possible_enemy_recruit_count + #(wesnoth.sides[side.side].recruit)
        for j, unit_type in ipairs(wesnoth.sides[side.side].recruit) do
            add_unit_type(unit_type)
        end
    end

    -- If no enemies were found, add a small number of "representative" unit types
    if #enemy_types == 0 then
        add_unit_type('Orcish Grunt')
        add_unit_type('Orcish Archer')
        add_unit_type('Wolf Rider')
        add_unit_type('Spearman')
        add_unit_type('Bowman')
        add_unit_type('Cavalryman')
    end

    data.enemy_counts = enemy_counts
    data.enemy_types = enemy_types
    data.num_enemies = math.max(#enemies, 1)
    data.possible_enemy_recruit_count = possible_enemy_recruit_count

    return data
end


local function get_test_units()
    local test_units, num_recruits = {}, 0
    local movetypes = {}
    for id in pairs(recruit_data.recruit_types) do
        local custom_movement = wml.get_child(wesnoth.unit_types[id].__cfg, "movement_costs")
        local movetype = wesnoth.unit_types[id].__cfg.movement_type
        if custom_movement
        or (not movetypes[movetype])
        or (movetypes[movetype] < wesnoth.unit_types[id].max_moves)
        then
            if not custom_movement then
                movetypes[movetype] = wesnoth.unit_types[id].max_moves
            end
            num_recruits = num_recruits + 1
            test_units[num_recruits] = wesnoth.units.create({
                type = id,
                side = wesnoth.current.side,
                random_traits = false,
                name = "X",
                random_gender = false
            })
        end
    end

    return test_units
end


local function get_village_target(leader, data)
    -- Only consider villages reachable by our fastest unit
    local fastest_unit_speed = 0
    for recruit_id in pairs(recruit_data.recruit_types) do
        if wesnoth.unit_types[recruit_id].max_moves > fastest_unit_speed then
            fastest_unit_speed = wesnoth.unit_types[recruit_id].max_moves
        end
    end

    -- get a list of all unowned and enemy-owned villages within fastest_unit_speed
    -- this may have false positives (villages that can't be reached due to difficult/impassible terrain)
    local exclude_map = LS.create()
    if data.castle.assigned_villages_x and data.castle.assigned_villages_x[1] then
        for i,x in ipairs(data.castle.assigned_villages_x) do
            exclude_map:insert(x, data.castle.assigned_villages_y[i])
        end
    end

    local all_villages = wesnoth.map.find{gives_income = true}
    local villages = {}
    for _,v in ipairs(all_villages) do
        local owner = wesnoth.map.get_owner(v)
        if ((not owner) or wesnoth.sides.is_enemy(owner, wesnoth.current.side))
            and (not exclude_map:get(v))
        then
            for _,loc in ipairs(data.castle.locs) do
                local dist = M.distance_between(v, loc)
                if (dist <= fastest_unit_speed) then
                   table.insert(villages, v)
                   break
                end
            end
        end
    end

    local hex, target, shortest_distance = {}, {}, AH.no_path

    if not data.castle.assigned_villages_x then
        data.castle.assigned_villages_x = {}
        data.castle.assigned_villages_y = {}

        -- If castle_switch CA makes the unit end up on a village, skip one village for the leader.
        -- Also do so if the leader is not passive. Note that the castle_switch CA will also return zero
        -- when the leader is passive, but not only in that case.
        local ltv_score, skip_one_village = 0, nil

        if recruit_data.ca_castle_switch then
            ltv_score = recruit_data.ca_castle_switch:evaluation({}, data, nil, leader)
            if ltv_score > 0 then
                skip_one_village = #(wesnoth.map.find {
                    gives_income = true,
                    x = data.CS_leader_target[1],
                    y = data.CS_leader_target[2]
                }) > 0
            end
        end

        if (ltv_score == 0) then
            skip_one_village = not AH.is_passive_leader(ai.aspects.passive_leader, leader.id)
        end

        if skip_one_village then
            -- skip one village for the leader
            for i,v in ipairs(villages) do
                local path, cost = wesnoth.paths.find_path(leader, v[1], v[2], {max_cost = leader.max_moves+1})
                if cost <= leader.max_moves then
                    table.insert(data.castle.assigned_villages_x, v[1])
                    table.insert(data.castle.assigned_villages_y, v[2])
                    table.remove(villages, i)
                    break
                end
            end
        end
    end

    local village_count = #villages
    local test_units = get_test_units()
    local num_recruits = #test_units
    local total_village_distance = {}
    for j,c in ipairs(data.castle.locs) do
        local c_index = c[1] + c[2]*1000
        total_village_distance[c_index] = 0
        for i,v in ipairs(villages) do
            total_village_distance[c_index] = total_village_distance[c_index] + M.distance_between(c[1], c[2], v[1], v[2])
        end
    end

    if (not recruit_data.unit_distances) then recruit_data.unit_distances = {} end
    for i,v in ipairs(villages) do
        local close_castle_hexes = {}
        for _,loc in ipairs(data.castle.locs) do
            local dist = M.distance_between(v[1], v[2], loc[1], loc[2])
            if (dist <= fastest_unit_speed) then
                if (not wesnoth.units.get(loc[1], loc[2])) then
                   table.insert(close_castle_hexes, loc)
                end
            end
        end

        for u,unit in ipairs(test_units) do
            test_units[u].x = v[1]
            test_units[u].y = v[2]
        end

        local viable_village = false
        local village_best_hex, village_shortest_distance = {}, AH.no_path
        for j,c in ipairs(close_castle_hexes) do
            if wesnoth.current.map:on_board(c) then
                local distance = 0
                for x,unit in ipairs(test_units) do
                    local key = unit.type .. '_' .. v[1] .. '-' .. v[2] .. '_' .. c[1]  .. '-' .. c[2]
                    local path, unit_distance
                    if (not recruit_data.unit_distances[key]) then
                        path, unit_distance = wesnoth.paths.find_path(unit, c[1], c[2], {ignore_visibility=true, max_cost=fastest_unit_speed+1})
                        recruit_data.unit_distances[key] = unit_distance
                    else
                        unit_distance = recruit_data.unit_distances[key]
                    end

                    distance = distance + unit_distance

                    -- Village is only viable if at least one unit can reach it
                    if unit_distance <= unit.max_moves then
                        viable_village = true
                    end
                end
                distance = distance / num_recruits

                if distance < village_shortest_distance
                or (distance == village_shortest_distance and distance < AH.no_path
                    and total_village_distance[c[1] + c[2]*1000] > total_village_distance[village_best_hex[1]+village_best_hex[2]*1000])
                then
                    village_best_hex = c
                    village_shortest_distance = distance
                end
            end
        end
        if village_shortest_distance < shortest_distance then
            hex = village_best_hex
            target = v
            shortest_distance = village_shortest_distance
        end

        if not viable_village then
            -- this village could not be reached by any unit
            -- eliminate it from consideration
            table.insert(data.castle.assigned_villages_x, v[1])
            table.insert(data.castle.assigned_villages_y, v[2])
            village_count = village_count - 1
        end
    end

    data.castle.loose_gold_limit = math.floor(wesnoth.sides[wesnoth.current.side].gold/village_count + 0.5)

    return hex, target
end


local function find_best_recruit_hex(leader, data)
    -- Find the best recruit hex
    -- First choice: a hex that can reach an unowned village
    -- Second choice: a hex close to the enemy
    get_current_castle(leader, data)

    local best_hex, village = get_village_target(leader, data)
    if village[1] then
        table.insert(data.castle.assigned_villages_x, village[1])
        table.insert(data.castle.assigned_villages_y, village[2])
    else
        -- no available village, look for hex closest to enemy leader
        -- and also the closest enemy
        local max_rating = -1

        local enemy_leaders = AH.get_attackable_enemies { canrecruit = 'yes' }
        local closest_enemy = AH.get_closest_enemy()

        for i,c in ipairs(data.castle.locs) do
            local rating = 0
            local unit = wesnoth.units.get(c[1], c[2])
            if (not AH.is_visible_unit(wesnoth.current.side, unit)) then
                for j,e in ipairs(enemy_leaders) do
                    rating = rating + 1 / M.distance_between(c[1], c[2], e.x, e.y) ^ 2.
                end
                if closest_enemy then
                    rating = rating + 1 / M.distance_between(c[1], c[2], closest_enemy.x, closest_enemy.y) ^ 2.
                end
                if (rating > max_rating) then
                    max_rating, best_hex = rating, { c[1], c[2] }
                end
            end
        end
    end

    if AH.print_eval() then
        if village[1] then
            std_print("Recruit at: " .. best_hex[1] .. "," .. best_hex[2] .. " -> " .. village[1] .. "," .. village[2])
        else
            std_print("Recruit at: " .. best_hex[1] .. "," .. best_hex[2])
        end
    end
    return best_hex, village
end


local function get_unit_counts_for_healing()
    local healers = #AH.get_live_units {
        side = wesnoth.current.side,
        ability = "healing",
        { "not", { canrecruit = "yes" }}
    }
    local healable = #AH.get_live_units {
        side = wesnoth.current.side,
        { "not", { ability = "regenerates" }}
    }
    return healers, healable
end


local function find_best_recruit(attack_type_count, unit_attack_type_count, recruit_effectiveness, recruit_vulnerability, attack_range_count, unit_attack_range_count, most_common_range_count, cfg)
    -- Find best recruit based on damage done to enemies present, speed, and hp/gold ratio
    local recruit_scores = {}
    local best_scores = {offense = 0, defense = 0, move = 0}
    local best_hex = recruit_data.recruit.best_hex
    local target_hex = recruit_data.recruit.target_hex

    local reference_hex = target_hex[1] and target_hex or best_hex
    local enemy_location, distance_to_enemy = AH.get_closest_enemy(reference_hex, wesnoth.current.side, { ignore_visibility = true })

    -- If no enemy is on the map, then we first use closest enemy start hex,
    -- and if that does not exist either, a location mirrored w.r.t the center of the map
    if not enemy_location then
        local enemy_sides = wesnoth.sides.find({ { "enemy_of", {side = wesnoth.current.side} } })
        local min_dist = math.huge
        for _, side in ipairs(enemy_sides) do
            local enemy_start_hex = wesnoth.current.map.special_locations[side.side]
            if enemy_start_hex then
                local dist = wesnoth.map.distance_between(reference_hex[1], reference_hex[2], enemy_start_hex[1], enemy_start_hex[2])
                if dist < min_dist then
                    min_dist = dist
                    enemy_location = { x = enemy_start_hex[1], y = enemy_start_hex[2] }
                end
            end
        end
        if not enemy_location then
            local map = wesnoth.current.map
            enemy_location = { x = map.playable_width + 1 - reference_hex[1], y = map.playable_height + 1 - reference_hex[2] }
        end
        distance_to_enemy = wesnoth.map.distance_between(reference_hex[1], reference_hex[2], enemy_location.x, enemy_location.y)
    end

    local gold_limit = math.huge
    if recruit_data.castle.loose_gold_limit >= recruit_data.recruit.cheapest_unit_cost then
        gold_limit = recruit_data.castle.loose_gold_limit
    end

    local recruitable_units = {}

    for recruit_id in pairs(recruit_data.recruit_types) do
        -- Count number of units with the same attack type. Used to avoid recruiting too many of the same unit
        local attack_types = 0
        local recruit_count = 0
        for attack_type, count in pairs(unit_attack_type_count[recruit_id]) do
            attack_types = attack_types + 1
            recruit_count = recruit_count + attack_type_count[attack_type]
        end
        recruit_count = recruit_count / attack_types
        local recruit_modifier = 1+recruit_count/50
        local efficiency_index = 1
        local unit_cost = wesnoth.unit_types[recruit_id].cost

        -- Use time to enemy to encourage recruiting fast units when the opponent is far away (game is beginning or we're winning)
        -- Base distance on
        local recruit_unit = wesnoth.units.create {
            type = recruit_id,
            x = best_hex[1],
            y = best_hex[2],
            random_traits = false,
            name = "X",
            random_gender = false
        }
        if target_hex[1] then
            local path, cost = wesnoth.paths.find_path(recruit_unit, target_hex[1], target_hex[2], {ignore_visibility=true, max_cost=wesnoth.unit_types[recruit_id].max_moves+1})
            if cost > wesnoth.unit_types[recruit_id].max_moves then
                -- Unit cost is effectively higher if cannot reach the village
                efficiency_index = 2
                unit_cost = no_village_cost(recruit_id)
            end

            -- Later calculations are based on where the unit will be after initial move
            recruit_unit.x = target_hex[1]
            recruit_unit.y = target_hex[2]
        end

        local path, cost = wesnoth.paths.find_path(recruit_unit, enemy_location.x, enemy_location.y, {ignore_units = true})
        local time_to_enemy = cost / wesnoth.unit_types[recruit_id].max_moves
        local move_score = 1 / (time_to_enemy * unit_cost^0.5)

        local eta = math.ceil(time_to_enemy)
        if target_hex[1] then
            -- expect a 1 turn delay to reach village
            eta = eta + 1
        end
        -- divide the lawful bonus by eta before running it through the function because the function converts from 0 centered to 1 centered

        local lawful_bonus = 0
        local eta_turn = wesnoth.current.turn + eta
        if eta_turn <= wesnoth.scenario.turns then
            lawful_bonus = wesnoth.schedule.get_time_of_day(nil, wesnoth.current.turn + eta).lawful_bonus / eta^2
        end
        local damage_bonus = AH.get_unit_time_of_day_bonus(recruit_unit.alignment, lawful_bonus)
        -- Estimate effectiveness on offense and defense
        local offense_score =
            (recruit_effectiveness[recruit_id].damage*damage_bonus+recruit_effectiveness[recruit_id].poison_damage)
            /(wesnoth.unit_types[recruit_id].cost^0.3*recruit_modifier^4)
        local defense_score = efficiency[recruit_id][efficiency_index]/recruit_vulnerability[recruit_id]

        local unit_score = {offense = offense_score, defense = defense_score, move = move_score}
        recruit_scores[recruit_id] = unit_score
        for key, score in pairs(unit_score) do
            if score > best_scores[key] then
                best_scores[key] = score
            end
        end

        if can_slow(recruit_unit) then
            unit_score["slows"] = true
        end
        if recruit_unit:matches { ability = "healing" } then
            unit_score["heals"] = true
        end
        if recruit_unit:matches { ability = "skirmisher" } then
            unit_score["skirmisher"] = true
        end
        recruitable_units[recruit_id] = recruit_unit
    end
    local healer_count, healable_count = get_unit_counts_for_healing()
    local hp_ratio = get_hp_ratio_with_gold()
    local best_score = 0
    local recruit_type
    local offense_weight = 2.5
    local defense_weight = 1/hp_ratio^0.5
    local move_weight = math.max((distance_to_enemy/20)^2, 0.25)
    local randomness = cfg.randomness or 0.1

    -- Bonus for higher-level units, as unit cost is penalized otherwise
    local high_level_fraction = cfg.high_level_fraction or 0
    local all_units = AH.get_live_units {
        side = wesnoth.current.side,
        { "not", { canrecruit = "yes" }}
    }
    local level_count = {}
    for _,unit in ipairs(all_units) do
        local level = unit.level
        level_count[level] = (level_count[level] or 0) + 1
    end
    local min_recruit_level, max_recruit_level = math.huge, -math.huge
    for recruit_id in pairs(recruit_data.recruit_types) do
        local level = wesnoth.unit_types[recruit_id].level
        if (level < min_recruit_level) then min_recruit_level = level end
        if (level > max_recruit_level) then max_recruit_level = level end
    end
    if (min_recruit_level < 1) then min_recruit_level = 1 end
    local unit_deficit = {}
    for i=min_recruit_level+1,max_recruit_level do
        -- If no non-leader units are on the map yet, we set up the situation as if there were
        -- one of each level. This is in order to get the situation for the first recruit right.
        local n_units = #all_units
        local n_units_this_level = level_count[i] or 0
        if (n_units == 0) then
            n_units = max_recruit_level - min_recruit_level
            n_units_this_level = 1
        end
        unit_deficit[i] = high_level_fraction ^ (i - min_recruit_level) * n_units - n_units_this_level
    end

    for recruit_id in pairs(recruit_data.recruit_types) do
        local level_bonus = 0
        local level = wesnoth.unit_types[recruit_id].level
        if (level > min_recruit_level) and (unit_deficit[level] > 0) then
            level_bonus = 0.25 * unit_deficit[level]^2
        end
        local scores = recruit_scores[recruit_id]
        local offense_score = (scores["offense"]/best_scores["offense"])^0.5
        local defense_score = (scores["defense"]/best_scores["defense"])^0.5
        local move_score = (scores["move"]/best_scores["move"])^0.5

        local bonus = math.random()*randomness
        if scores["slows"] then
            bonus = bonus + 0.4
        end
        if scores["heals"] then
            bonus = bonus + (healable_count/(healer_count+1))/20
        end
        if scores["skirmisher"] then
            bonus = bonus + 0.1
        end
        for attack_range, count in pairs(unit_attack_range_count[recruit_id]) do
            bonus = bonus + 0.02 * most_common_range_count / (attack_range_count[attack_range]+1)
        end
        local race = wesnoth.races[wesnoth.unit_types[recruit_id].race]
        local num_traits = race and race.num_traits or 0
        bonus = bonus + 0.03 * num_traits^2
        if target_hex[1] then
            recruitable_units[recruit_id].x = best_hex[1]
            recruitable_units[recruit_id].y = best_hex[2]
            local path, cost = wesnoth.paths.find_path(recruitable_units[recruit_id], target_hex[1], target_hex[2], {ignore_visibility=true, max_cost=wesnoth.unit_types[recruit_id].max_moves+1})
            if cost > wesnoth.unit_types[recruit_id].max_moves then
                -- penalty if the unit can't reach the target village
                bonus = bonus - 0.2
            end
        end

        local score = offense_score*offense_weight + defense_score*defense_weight + move_score*move_weight + bonus + level_bonus

        if AH.print_eval() then
            std_print(recruit_id .. " score: " .. offense_score*offense_weight .. " + " .. defense_score*defense_weight .. " + " .. move_score*move_weight  .. " + " .. bonus  .. " + " .. level_bonus  .. " = " .. score)
        end
        if score > best_score and wesnoth.unit_types[recruit_id].cost <= gold_limit then
            best_score = score
            recruit_type = recruit_id
        end
    end

    return recruit_type
end


local ca_recruit_rushers = {}

function ca_recruit_rushers:evaluation(cfg, data, filter_own)
    local start_time, ca_name = wesnoth.ms_since_init() / 1000., 'recruit_rushers'
    if AH.print_eval() then AH.print_ts('     - Evaluating recruit_rushers CA:') end

    if (recruit_data.turn ~= wesnoth.current.turn) then
        if cfg.reset_cache_each_turn then
            recruit_data = {}
        else
            recruit_data.recruit = nil
        end

        recruit_data.turn = wesnoth.current.turn
    end

    -- Check if leader is on keep
    local leader = wesnoth.units.find_on_map {
        side = wesnoth.current.side,
        canrecruit = 'yes',
        { "and", filter_own }
    }[1]

    if (not leader) or (not wesnoth.terrain_types[wesnoth.current.map[leader]].keep) then
        return 0
    end

    -- Check if there is enough gold to recruit a unit
    local cheapest_unit_cost = AH.get_cheapest_recruit_cost()
    if cheapest_unit_cost > wesnoth.sides[wesnoth.current.side].gold then
        return 0
    end

    recruit_data.recruit_types = {}
    for _,unit_type in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
        recruit_data.recruit_types[unit_type] = true
    end
    for _,unit_type in ipairs(leader.extra_recruit) do
        recruit_data.recruit_types[unit_type] = true
    end

    -- Check for space to recruit a unit
    get_current_castle(leader, recruit_data)
    local no_space = true
    for i,c in ipairs(recruit_data.castle.locs) do
        local unit = wesnoth.units.get(c[1], c[2])
        if (not AH.is_visible_unit(wesnoth.current.side, unit)) then
            no_space = false
            break
        end
    end
    if no_space then
        return 0
    end

    -- Check for minimal recruit option: recruit only enough units to grab nearby villages
    -- on turn 1 if castle switch is expected
    if not recruit_data.ca_castle_switch then
        recruit_data.ca_castle_switch = ca_castle_switch()
    end
    if recruit_data.ca_castle_switch then
        if wesnoth.current.turn == 1 and recruit_data.ca_castle_switch:evaluation({}, data) > 0 then
            if not get_village_target(leader, recruit_data)[1] then
                return 0
            end
        end
    end

    if not recruit_data.recruit then
        recruit_data.recruit = init_data(leader)
    end
    recruit_data.recruit.cheapest_unit_cost = cheapest_unit_cost

    local score = cfg.ca_score or 180010

    if AH.print_eval() then AH.done_eval_messages(start_time, ca_name) end
    return score
end


function ca_recruit_rushers:execution(cfg, data, filter_own)
    if AH.print_exec() then AH.print_ts('   Executing recruit_rushers CA') end
    if AH.show_messages() then wesnoth.wml_actions.message { speaker = 'narrator', message = 'Recruiting' } end

    local enemy_counts = recruit_data.recruit.enemy_counts
    local enemy_types = recruit_data.recruit.enemy_types
    local num_enemies =  recruit_data.recruit.num_enemies

    -- Determine effectiveness of recruitable units against each enemy unit type
    local recruit_effectiveness = {}
    local recruit_vulnerability = {}
    local attack_type_count = {} -- The number of units who will likely use a given attack type
    local attack_range_count = {} -- The number of units who will likely use a given attack range
    local unit_attack_type_count = {} -- The attack types a unit will use
    local unit_attack_range_count = {} -- The ranges a unit will use
    local enemy_type_count = 0
    local poisoner_count = 0.1 -- Number of units with a poison attack (set to slightly > 0 because we divide by it later)
    local poisonable_count = 0 -- Number of units that the opponents control that are hurt by poison
    local recruit_count = {}
    for recruit_id in pairs(recruit_data.recruit_types) do
        recruit_count[recruit_id] = #(AH.get_live_units { side = wesnoth.current.side, type = recruit_id, canrecruit = 'no' })
    end

    for i, unit_type in ipairs(enemy_types) do
        wesnoth.interface.handle_user_interact()
        enemy_type_count = enemy_type_count + 1
        local poison_vulnerable = false
        for recruit_id in pairs(recruit_data.recruit_types) do
            local analysis = analyze_enemy_unit(unit_type, recruit_id)

            if not recruit_effectiveness[recruit_id] then
                recruit_effectiveness[recruit_id] = {damage = 0, poison_damage = 0}
                recruit_vulnerability[recruit_id] = 0
            end

            recruit_effectiveness[recruit_id].damage = recruit_effectiveness[recruit_id].damage + analysis.defense.damage * enemy_counts[unit_type]^2
            if analysis.defense.poison_damage and analysis.defense.poison_damage > 0 then
                poison_vulnerable = true
                recruit_effectiveness[recruit_id].poison_damage = recruit_effectiveness[recruit_id].poison_damage +
                    analysis.defense.poison_damage * enemy_counts[unit_type]^2
            end
            recruit_vulnerability[recruit_id] = recruit_vulnerability[recruit_id] + (analysis.retaliation.damage * enemy_counts[unit_type])^3

            local attack_type = analysis.defense.attack.type
            if not attack_type_count[attack_type] then
                attack_type_count[attack_type] = 0
            end
            attack_type_count[attack_type] = attack_type_count[attack_type] + recruit_count[recruit_id]

            local attack_range = analysis.defense.attack.range
            if not attack_range_count[attack_range] then
                attack_range_count[attack_range] = 0
            end
            attack_range_count[attack_range] = attack_range_count[attack_range] + recruit_count[recruit_id]

            if not unit_attack_type_count[recruit_id] then
                unit_attack_type_count[recruit_id] = {}
            end
            unit_attack_type_count[recruit_id][attack_type] = true

            if not unit_attack_range_count[recruit_id] then
                unit_attack_range_count[recruit_id] = {}
            end
            unit_attack_range_count[recruit_id][attack_range] = true
        end
        if poison_vulnerable then
            poisonable_count = poisonable_count + enemy_counts[unit_type]
        end
    end
    for recruit_id in pairs(recruit_data.recruit_types) do
        -- Count the number of units with the poison ability
        -- This could be wrong if all the units on the enemy side are immune to poison, but since poison has no effect then anyway it doesn't matter
        if recruit_effectiveness[recruit_id].poison_damage > 0 then
            poisoner_count = poisoner_count + recruit_count[recruit_id]
        end
    end
    -- Subtract the number of possible recruits for the enemy from the list of poisonable units
    -- This works perfectly unless some of the enemy recruits cannot be poisoned.
    -- However, there is no problem with this since poison is generally less useful in such situations and subtracting them too discourages such recruiting
    local poison_modifier = math.max(0, math.min(((poisonable_count-recruit_data.recruit.possible_enemy_recruit_count) / (poisoner_count*5)), 1))^2
    for recruit_id in pairs(recruit_data.recruit_types) do
        -- Ensure effectiveness and vulnerability are positive.
        -- Negative values imply that drain is involved and the amount drained is very high
        if recruit_effectiveness[recruit_id].damage <= 0 then
            recruit_effectiveness[recruit_id].damage = 0.01
        else
            recruit_effectiveness[recruit_id].damage = (recruit_effectiveness[recruit_id].damage / (num_enemies)^2)^0.5
        end
        recruit_effectiveness[recruit_id].poison_damage = (recruit_effectiveness[recruit_id].poison_damage / (num_enemies)^2)^0.5 * poison_modifier
        if recruit_vulnerability[recruit_id] <= 0 then
            recruit_vulnerability[recruit_id] = 0.01
        else
            recruit_vulnerability[recruit_id] = (recruit_vulnerability[recruit_id] / ((num_enemies)^2))^0.5
        end
    end
    -- Correct count of units for each range
    local most_common_range
    local most_common_range_count = 0
    for range, count in pairs(attack_range_count) do
        attack_range_count[range] = count/enemy_type_count
        if attack_range_count[range] > most_common_range_count then
            most_common_range = range
            most_common_range_count = attack_range_count[range]
        end
    end
    -- Correct count of units for each attack type
    for attack_type, count in pairs(attack_type_count) do
        attack_type_count[attack_type] = count/enemy_type_count
    end

    local recruit_type
    local leader = wesnoth.units.find_on_map {
        side = wesnoth.current.side,
        canrecruit = 'yes',
        { "and", filter_own }
    }[1]
    repeat
        recruit_data.recruit.best_hex, recruit_data.recruit.target_hex = find_best_recruit_hex(leader, recruit_data)
        recruit_type = find_best_recruit(attack_type_count, unit_attack_type_count, recruit_effectiveness, recruit_vulnerability, attack_range_count, unit_attack_range_count, most_common_range_count, cfg)
    until recruit_type

    if wesnoth.unit_types[recruit_type].cost <= wesnoth.sides[wesnoth.current.side].gold then
        AH.checked_recruit(ai, recruit_type, recruit_data.recruit.best_hex[1], recruit_data.recruit.best_hex[2])

        -- If the recruited unit cannot reach the target hex, return it to the pool of targets
        if recruit_data.recruit.target_hex and recruit_data.recruit.target_hex[1] then
            local unit = wesnoth.units.get(recruit_data.recruit.best_hex[1], recruit_data.recruit.best_hex[2])
            local path, cost = wesnoth.paths.find_path(unit, recruit_data.recruit.target_hex[1], recruit_data.recruit.target_hex[2], {ignore_visibility=true, max_cost=unit.max_moves+1})
            if cost > unit.max_moves then
                -- The last village added to the list should be the one we tried to aim for, check anyway
                local last = #recruit_data.castle.assigned_villages_x
                if (recruit_data.castle.assigned_villages_x[last] == recruit_data.recruit.target_hex[1]) and (recruit_data.castle.assigned_villages_y[last] == recruit_data.recruit.target_hex[2]) then
                    table.remove(recruit_data.castle.assigned_villages_x)
                    table.remove(recruit_data.castle.assigned_villages_y)
                end
            end
        end

        return true
    else
        -- This results in the CA being blacklisted -> clear cache
        recruit_data = {}

        return false
    end
end

return ca_recruit_rushers
