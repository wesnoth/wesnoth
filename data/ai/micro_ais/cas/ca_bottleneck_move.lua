local H = wesnoth.require "lua/helper.lua"
local LS = wesnoth.require "lua/location_set.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"
local MAISD = wesnoth.require "ai/micro_ais/micro_ai_self_data.lua"

local function bottleneck_is_my_territory(map, enemy_map)
    -- Create map that contains 'true' for all hexes that are
    -- on the AI's side of the map

    -- Get copy of leader to do pathfinding from each hex to the
    -- front-line hexes, both own (stored in 'map') and enemy (enemy_map) front-line hexes
    -- If there is no leader, use first unit found
    local unit = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]
    if (not unit) then unit = wesnoth.get_units { side = wesnoth.current.side }[1] end
    local dummy_unit = wesnoth.copy_unit(unit)

    local territory_map = LS.create()
    local w,h,b = wesnoth.get_map_size()
    for x = 1,w do
        for y = 1,h do
            -- The hex might have been covered already previously
            if (not territory_map:get(x,y)) then
                dummy_unit.x, dummy_unit.y = x, y

                -- Find lowest movement cost to own front-line hexes
                local min_cost, best_path = 9e99, {}
                map:iter(function(xm, ym, v)
                    local path, cost = wesnoth.find_path(dummy_unit, xm, ym, { ignore_units = true })
                    if (cost < min_cost) then
                       min_cost, best_path = cost, path
                    end
                end)

                -- And the same to the enemy front line
                local min_cost_enemy, best_path_enemy = 9e99, {}
                enemy_map:iter(function(xm, ym, v)
                    local path, cost = wesnoth.find_path(dummy_unit, xm, ym, { ignore_units = true })
                    if (cost < min_cost_enemy) then
                       min_cost_enemy, best_path_enemy = cost, path
                    end
                end)

                -- We can set this for the entire path
                -- for efficiency reasons (this is pretty slow, esp. on large maps)
                if (min_cost < min_cost_enemy) then
                    for i,p in ipairs(best_path) do
                        territory_map:insert(p[1], p[2], true)
                    end
                else  -- We do need to use 0's in this case though, false won't work
                    for i,p in ipairs(best_path_enemy) do
                        territory_map:insert(p[1], p[2], 0)
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
    -- Turn 'key_x= key_y=' comma-separated lists into a location set
    local coords = {}
    for x in string.gmatch(key_x, "%d+") do
        table.insert(coords, { x })
    end
    local i = 1
    for y in string.gmatch(key_y, "%d+") do
        table.insert(coords[i], y)
        table.insert(coords[i], max_value + 10 - i * 10) -- the rating
        i = i + 1
    end

    return AH.LS_of_triples(coords)
end

local function bottleneck_create_positioning_map(max_value, self)
    -- Create the positioning maps for the healers and leaders, if not given by WML keys
    -- Max_value: the rating value for the first hex in the set

    -- First, find all locations adjacent to def_map
    -- This might include hexes on the line itself, but
    -- only store those that are not in enemy territory
    local map = LS.create()
    self.data.def_map:iter( function(x, y, v)
        for xa, ya in H.adjacent_tiles(x, y) do
            if self.data.is_my_territory:get(xa, ya) then
                -- This rating adds up the scores of all the adjacent def_map hexes
                local rating = self.data.def_map:get(x, y) or 0
                rating = rating + (map:get(xa, ya) or 0)
                map:insert(xa, ya, rating)
            end
        end
    end)

    -- We need to sort the map, and assign descending values
    local locs = AH.to_triples(map)
    table.sort(locs, function(a, b) return a[3] > b[3] end)
    for i,l in ipairs(locs) do l[3] = max_value + 10 - i * 10 end
    map = AH.LS_of_triples(locs)

    -- Finally, we merge the defense map into this, as healers/leaders (by default)
    -- can take position on the front line
    map:union_merge(self.data.def_map,
        function(x, y, v1, v2) return v1 or v2 end
    )

    return map
end

local function bottleneck_get_rating(unit, x, y, has_leadership, is_healer, self)
    -- Calculate rating of a unit at the given coordinates
    -- Don't want to extract is_healer and has_leadership inside this function, as it is very slow
    -- thus they are provided as parameters from the calling function

    local rating = 0
    -- Defense positioning rating
    -- We exclude healers/leaders here, as we don't necessarily want them on the front line
    if (not is_healer) and (not has_leadership) then
        rating = self.data.def_map:get(x, y) or 0
    end

    -- Healer positioning rating
    if is_healer then
        local healer_rating = self.data.healer_map:get(x, y) or 0
        if (healer_rating > rating) then rating = healer_rating end
    end

    -- Leadership unit positioning rating
    if has_leadership then
        local leadership_rating = self.data.leadership_map:get(x, y) or 0

        -- If leadership unit is injured -> prefer hexes next to healers
        if (unit.hitpoints < unit.max_hitpoints) then
            for xa, ya in H.adjacent_tiles(x, y) do
                local adj = wesnoth.get_unit(xa, ya)
                if adj and (adj.__cfg.usage == "healer") then
                    leadership_rating = leadership_rating + 100
                    break
                end
            end

        end

        if (leadership_rating > rating) then rating = leadership_rating end
    end

    -- Injured unit positioning
    if (unit.hitpoints < unit.max_hitpoints) then
        local healing_rating = self.data.healing_map:get(x, y) or 0
        if (healing_rating > rating) then rating = healing_rating end
    end

    -- If this did not produce a positive rating, we add a
    -- distance-based rating, to get units to the bottleneck in the first place
    if (rating <= 0) and self.data.is_my_territory:get(x, y) then
        local combined_dist = 0
        self.data.def_map:iter(function(x_def, y_def, v)
            combined_dist = combined_dist + H.distance_between(x, y, x_def, y_def)
        end)
        combined_dist = combined_dist / self.data.def_map:size()
        rating = 1000 - combined_dist * 10.
    end

    -- Now add the unit specific rating
    if (rating > 0) then
        rating = rating + unit.hitpoints/10. + unit.experience/100.
    end

    return rating
end

local function bottleneck_move_out_of_way(unit, self)
    -- Find the best move out of the way for a unit
    -- and choose the shortest possible move
    -- Returns nil if no move was found

    -- just a sanity check: unit to move out of the way needs to be on our side:
    if (unit.side ~= wesnoth.current.side) then return nil end

   local reach = wesnoth.find_reach(unit)

   -- Find all the occupied hexes, by any unit
   -- (too slow if we do this inside the loop for a specific hex)
   local all_units = wesnoth.get_units { }
   local occ_hexes = LS:create()
   for i,u in ipairs(all_units) do
       occ_hexes:insert(u.x, u.y)
   end

   -- find the closest unoccupied reachable hex in the east
   local best_reach, best_hex = -1, {}
   for i,r in ipairs(reach) do
       if self.data.is_my_territory:get(r[1], r[2]) and (not occ_hexes:get(r[1], r[2])) then
           -- Best hex to move out of way to:
           --  (r[3] > best_reach) : move shorter than previous best move
           if (r[3] > best_reach) then
               best_reach, best_hex = r[3], { r[1], r[2] }
           end
       end
   end
   --print("Best reach: ",unit.id, best_reach, best_hex[1], best_hex[2])

   if best_reach > -1 then return best_hex end
end

local ca_bottleneck_move = {}

function ca_bottleneck_move:evaluation(ai, cfg, self)
    -- Check whether the side leader should be included or not
    if cfg.active_side_leader and
        (not MAISD.get_mai_self_data(self.data, cfg.ai_id, "side_leader_activated"))
    then
        local can_still_recruit = false  -- enough gold left for another recruit?
        local recruit_list = wesnoth.sides[wesnoth.current.side].recruit
        for i,recruit_type in ipairs(recruit_list) do
            local cost = wesnoth.unit_types[recruit_type].cost
            local current_gold = wesnoth.sides[wesnoth.current.side].gold
            if (cost <= current_gold) then
                can_still_recruit = true
                break
            end
        end
        if (not can_still_recruit) then
            MAISD.set_mai_self_data(self.data, cfg.ai_id, { side_leader_activated = true })
        end
    end

    -- Now find all units, including the leader or not, depending on situation and settings
    local units = {}
    if MAISD.get_mai_self_data(self.data, cfg.ai_id, "side_leader_activated") then
        units = AH.get_units_with_moves { side = wesnoth.current.side }
    else
        units = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no' }
    end

    -- If there's no units with moves left, nothing to be done here
    if (not units[1]) then return 0 end

    -- Set up the arrays that tell the AI where to defend the bottleneck
    -- Get the x and y coordinates (this assumes that cfg.x and cfg.y have the same length)
    self.data.def_map = bottleneck_triple_from_keys(cfg.x, cfg.y, 10000)
    --AH.put_labels(self.data.def_map)

    -- Get the territory map, describing which hex is on AI's side of the bottleneck
    -- This one is a bit expensive, esp. on large maps -> don't delete every move and reuse
    -- However, after a reload, self.data.is_my_territory is an empty string
    --  -> need to recalculate in that case also  (the reason is that is_my_territory is not a WML table)
    if (not self.data.is_my_territory) or (type(self.data.is_my_territory) == 'string') then
        local enemy_map = bottleneck_triple_from_keys(cfg.enemy_x, cfg.enemy_y, 10000)
        self.data.is_my_territory = bottleneck_is_my_territory(self.data.def_map, enemy_map)
    end
    --AH.put_labels(self.data.is_my_territory)

    -- Setting up healer positioning map
    if cfg.healer_x and cfg.healer_y then
        -- If healer_x,healer_y are given, extract locs from there
        self.data.healer_map = bottleneck_triple_from_keys(cfg.healer_x, cfg.healer_y, 5000)
    else
        -- Otherwise create the map here
        self.data.healer_map = bottleneck_create_positioning_map(5000, self)
    end
    -- Use def_map values for any healer hexes that are defined in def_map as well
    self.data.healer_map:inter_merge(self.data.def_map,
        function(x, y, v1, v2) return v2 or v1 end
    )
    --AH.put_labels(self.data.healer_map)

    -- Setting up leadership position map
    -- If leadership_x, leadership_y are not given, we create the leadership positioning array
    if cfg.leadership_x and cfg.leadership_y then
        -- If leadership_x,leadership_y are given, extract locs from there
        self.data.leadership_map = bottleneck_triple_from_keys(cfg.leadership_x, cfg.leadership_y, 4000)
    else
        -- Otherwise create the map here
        self.data.leadership_map = bottleneck_create_positioning_map(4000, self)
    end
    -- Use def_map values for any leadership hexes that are defined in def_map as well
    self.data.leadership_map:inter_merge(self.data.def_map,
        function(x, y, v1, v2) return v2 or v1 end
    )
    --AH.put_labels(self.data.leadership_map)

    -- healing map: positions next to healers, needs to be calculated each move
    -- Healers get moved with higher priority, so don't need to check their MP
    local healers = wesnoth.get_units { side = wesnoth.current.side, ability = "healing" }
    self.data.healing_map = LS.create()
    for i,h in ipairs(healers) do
        for x, y in H.adjacent_tiles(h.x, h.y) do
            -- Cannot be on the line, and needs to be in own territory
            if self.data.is_my_territory:get(x, y) then
                local min_dist = 9e99
                self.data.def_map:iter( function(xd, yd, vd)
                    local dist_line = H.distance_between(x, y, xd, yd)
                    if (dist_line < min_dist) then min_dist = dist_line end
                end)
                if (min_dist > 0) then
                    self.data.healing_map:insert(x, y, 3000 + min_dist)  -- farther away from enemy is good
                end
            end
        end
    end
    --AH.put_labels(self.data.healing_map)

    -- Now on to evaluating possible moves:
    -- First, get the rating of all units in their current positions
    -- A move is only considered if it improves the overall rating,
    -- that is, its rating must be higher than:
    --   1. the rating of the unit on the target hex (if there is one)
    --   2. the rating of the currently considered unit on its current hex
    local all_units = wesnoth.get_units { side = wesnoth.current.side }
    local occ_hexes = LS.create()

    for i,u in ipairs(all_units) do
        -- Is this a healer or leadership unit?
        local is_healer = (u.__cfg.usage == "healer")
        local has_leadership = AH.has_ability(u, "leadership")

        local rating = bottleneck_get_rating(u, u.x, u.y, has_leadership, is_healer, self)
        occ_hexes:insert(u.x, u.y, rating)

        -- A unit that cannot move any more, (or at least cannot move out of the way)
        -- must be considered to have a very high rating (it's in the best position
        -- it can possibly achieve)
        local best_move_away = bottleneck_move_out_of_way(u, self)
        if (not best_move_away) then occ_hexes:insert(u.x, u.y, 20000) end
    end
    --AH.put_labels(occ_hexes)

    -- Find all attack positions next to enemies
    -- This is done up here rather than in the loop, because it's too slow otherwise
    local enemies = AH.get_live_units {
        { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} }
    }
    local attacks = {}
    for i,e in ipairs(enemies) do
        for x,y in H.adjacent_tiles(e.x, e.y) do
            if self.data.is_my_territory:get(x,y) then
                local unit_in_way = wesnoth.get_unit(x, y)
                local data = { x = x, y = y,
                    defender = e,
                    defender_level = wesnoth.unit_types[e.type].level,
                    unit_in_way = unit_in_way
                }
                table.insert(attacks, data)
            end
        end
    end

    -- Go through all units with moves left
    -- Variables to store best unit/move
    local max_rating, best_unit, best_hex = 0, {}, {}
    for i,u in ipairs(units) do

        -- Is this a healer or leadership unit?
        local is_healer = (u.__cfg.usage == "healer")
        local has_leadership = AH.has_ability(u, "leadership")

        local current_rating = bottleneck_get_rating(u, u.x, u.y, has_leadership, is_healer, self)
        --print("Finding move for:",u.id, u.x, u.y, current_rating)

        -- Find the best move for all hexes the unit can reach ...
        local reach = wesnoth.find_reach(u)
        for i,r in ipairs(reach) do
            local rating = bottleneck_get_rating(u, r[1], r[2], has_leadership, is_healer, self)
            --print(" ->",r[1],r[2],rating,occ_hexes:get(r[1], r[2]))
            --reach_map:insert(r[1], r[2], rating)

            -- A move is only considered if it improves the overall rating,
            -- that is, its rating must be higher than:
            --   1. the rating of the unit on the target hex (if there is one)
            --   2. the rating of the currently considered unit on its current hex
            if occ_hexes:get(r[1], r[2]) and (occ_hexes:get(r[1], r[2]) >= rating) then rating = 0 end
            if (rating <= current_rating) then rating = 0 end
            --print("   ->",r[1],r[2],rating,occ_hexes:get(r[1], r[2]))

            -- If the target hex is occupied, give it a (very) small penalty
            if occ_hexes:get(r[1], r[2]) then rating = rating - 0.001 end

            -- Now only valid and possible moves should have a rating > 0
            if rating > max_rating then
                max_rating, best_unit, best_hex = rating, u, { r[1], r[2] }
            end

            -- Finally, we check whether a level-up attack is possible from this hex
            -- Level-up-attacks will always get a rating greater than any move
            -- So if this is the case, they are done first
            for j,a in ipairs(attacks) do
                -- Only do calc. if there's a theoretical chance for leveling up (speeds things up a lot)
                if (a.x == r[1]) and (a.y == r[2]) and (u.max_experience - u.experience <= 8 * a.defender_level) then
                    --print('Evaluating attack', u.id, a.x, a.y, a.defender.x, a.defender.y)

                    -- For this one, we really want to go through all weapons individually
                    -- as the best chosen automatically might not be the best for this specific task
                    local n_weapon = 0
                    for weapon in H.child_range(u.__cfg, "attack") do
                        n_weapon = n_weapon + 1
                        local att_stats, def_stats = BC.simulate_combat_loc(u, { a.x, a.y }, a.defender, n_weapon)

                        -- Level-up attack when:
                        -- 1. max_experience-experience <= target.level and chance to die = 0
                        -- 2. max_experience-experience <= target.level*8 and chance to die = 0
                        --   and chance to kill > 66% and remaining av hitpoints > 20
                        -- #1 is a definite level up, #2 is not, so #1 gets priority
                        local lu_rating = 0
                        if (u.max_experience - u.experience <= a.defender_level) then
                            if (att_stats.hp_chance[0] == 0) then
                                -- weakest enemy is best (favors stronger weapon)
                                lu_rating = 15000 - def_stats.average_hp
                            end
                        else
                            if (u.max_experience - u.experience <= 8 * a.defender_level) and (att_stats.hp_chance[0] == 0) and (def_stats.hp_chance[0] >= 0.66) and (att_stats.average_hp >= 20) then
                                -- strongest attacker and weakest enemy is best
                                lu_rating = 14000 + att_stats.average_hp - def_stats.average_hp/2.
                            end
                        end

                        -- Very small penalty if there's a unit in the way
                        -- We also need to check whether this unit can actually move out of the way
                        if a.unit_in_way then
                            local moow = bottleneck_move_out_of_way(a.unit_in_way, self)
                            if moow then
                                lu_rating = lu_rating - 0.001
                            else
                                lu_rating = 0
                            end
                        end
                        --print("Level-up rating:",lu_rating)

                        if (lu_rating > max_rating) then
                            max_rating, best_unit, best_hex = lu_rating, u, { r[1], r[2] }
                            -- The following are also needed in this case
                            -- We don't have to worry about unsetting them, as LU attacks
                            -- always have higher priority than any other move
                            self.data.lu_defender = a.defender
                            self.data.lu_weapon = n_weapon
                        end
                    end
                end
            end
        end
        --AH.put_labels(reach_map)
    end
    --print("Best move:",best_unit.id,max_rating,best_hex[1],best_hex[2])

    -- Finally, if there's another unit in the best location,
    -- moving it out of the way becomes the best move
    -- It has been checked that this is possible
    local unit_in_way = wesnoth.get_units { x = best_hex[1], y = best_hex[2],
        { "not", { id = best_unit.id } }
    }[1]
    if unit_in_way then
        best_hex = bottleneck_move_out_of_way(unit_in_way, self)
        best_unit = unit_in_way
        --print("Moving out of way:", best_unit.id, best_hex[1], best_hex[2])

        -- Also need to delete the level-up attack fields
        -- They will be reset on the next turn
        self.data.lu_defender = nil
        self.data.lu_weapon = nil
    end

    -- Set the variables for the exec() function
    if max_rating == 0 then
        -- In this case we take MP away from all units
        -- This is done so that the RCA AI CAs can be kept in place
        self.data.bottleneck_moves_done = true
    else
        self.data.bottleneck_moves_done = false
        self.data.unit = best_unit
        self.data.hex = best_hex
    end
    return cfg.ca_score
end

function ca_bottleneck_move:execution(ai, cfg, self)
    if self.data.bottleneck_moves_done then
        local units = {}
        if MAISD.get_mai_self_data(self.data, cfg.ai_id, "side_leader_activated") then
            units = AH.get_units_with_moves { side = wesnoth.current.side }
        else
            units = AH.get_units_with_moves { side = wesnoth.current.side, canrecruit = 'no' }
        end
        for i,u in ipairs(units) do
            AH.checked_stopunit_moves(ai, u)
        end
    else
        --print("Moving unit:",self.data.unit.id, self.data.unit.x, self.data.unit.y, " ->", best_hex[1], best_hex[2], " -- turn:", wesnoth.current.turn)

        if (self.data.unit.x ~= self.data.hex[1]) or (self.data.unit.y ~= self.data.hex[2]) then  -- test needed for level-up move
            AH.checked_move(ai, self.data.unit, self.data.hex[1], self.data.hex[2])   -- don't want full move, as this might be stepping out of the way
        end
        if (not self.data.unit) or (not self.data.unit.valid) then return end

        -- If this is a move for a level-up attack, do the attack also
        if self.data.lu_defender then
            --print("Level-up attack",self.data.unit.id, self.data.lu_defender.id, self.data.lu_weapon)

            AH.checked_attack(ai, self.data.unit, self.data.lu_defender, self.data.lu_weapon)
        end
    end

    -- Now delete almost everything
    -- Keep: self.data.is_my_territory, [micro_ai]side_leader_activated=
    self.data.unit, self.data.hex = nil, nil
    self.data.lu_defender, self.data.lu_weapon = nil, nil
    self.data.bottleneck_moves_done = nil
    self.data.def_map, self.data.healer_map, self.data.leadership_map, self.data.healing_map = nil, nil, nil, nil
end

return ca_bottleneck_move
