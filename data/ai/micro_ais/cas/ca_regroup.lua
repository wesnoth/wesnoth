
local T = wml.tag;
local ai_helper = wesnoth.require 'ai/lua/ai_helper.lua';
local battle_calcs = wesnoth.require 'ai/lua/battle_calcs.lua';
local MAISD = wesnoth.require 'ai/micro_ais/micro_ai_self_data.lua';
local location_set = wesnoth.require 'location_set';
local return_table = {};

--###############################
-- MAISD HELPERS
--###############################
local function MAISD_get_self_data(data, ai_id, key)
    -- Sanitise to a valid WML attribute key: unit ids can contain spaces/punctuation,
    -- which would otherwise be silently dropped when the self-data table is serialised.
    -- MUST stay identical to the sanitisation in MAISD_update_self_data.
    key = (key:gsub('[^%w_]', '_'));
    -- I kept running into bugs when using the built-in MAISD.get_mai_self_data function.
    -- Maybe I was doing something wrong.
    for i,mai in ipairs(data) do
        if (mai[2].ai_id==ai_id and mai[2][key]~=nil) then
           return mai[2][key];
        end
    end
    return nil;
end
local function MAISD_update_self_data(data, ai_id, key, value)
    -- Sanitise to a valid WML attribute key. MUST stay identical to MAISD_get_self_data,
    -- otherwise a key written here won't be found on read-back.
    -- if you remove this, keys with spaces (e.g. unit IDs) will appear to work, but won't get saved in saved games
    key = (key:gsub('[^%w_]', '_'));
    -- I couldn't figure out how to get the built-in MAISD functions to do this.
    -- first, check for any existing entry. If we have one, update it and return.
    for i,mai in ipairs(data) do
        if (mai[2].ai_id==ai_id and mai[2][key]~=nil) then
            mai[2][key] = value;
            return;
        end
    end
    -- otherwise, add a new entry
    local entry = {}
    entry[key]     = value;
    entry['ai_id'] = ai_id;
    table.insert(data, {
        [1]='micro_ai',
        [2]=entry,
    })
end

--###############################
-- TABLE HELPERS
--###############################
local function table_contains(table, element)
    for key,value in pairs(table) do
        if value==element then return true end
    end
    return false;
end

--###############################
-- UNIT HELPERS
--###############################
local function find_reach_with_moves(unit, cfg, moves)
    if moves==nil then moves=unit.moves end
    local original_moves = unit.moves;
    unit.moves = moves;
    local reach;
    if (cfg and cfg.return_raw)
        then reach = wesnoth.paths.find_reach(unit, cfg);
        else reach = location_set.of_pairs( wesnoth.paths.find_reach(unit, cfg) );
    end
    unit.moves = original_moves;
    return reach;
end

-- a guard's stationed hex, stored as two separate self-data variables. returns x,y (or nil,nil if the guard has never been stationed)
local function guard_get_station(data, ai_id, unit)
    return MAISD_get_self_data(data, ai_id, 'stationed_x_'..unit.id),
           MAISD_get_self_data(data, ai_id, 'stationed_y_'..unit.id);
end

-- true only if the guard can actually move to and stop on its stationed hex this turn:
-- the station must be set, currently empty (or occupied by the guard itself), and present in move_reach
-- (find_reach already accounts for zone of control, terrain, and blocking units)
local function guard_can_reach_station(data, ai_id, unit, move_reach)
    local sx, sy = guard_get_station(data, ai_id, unit);
    if not (sx and sy and move_reach[{sx, sy}]) then return false end
    local occupier = wesnoth.units.get(sx, sy);
    return (not occupier) or (occupier.id == unit.id);
end





--###########################################################################
--                                   MAIN
--###########################################################################
function return_table:evaluation(cfg,data)
    -- only run this MAI once each turn
    local most_recent_turn = MAISD_get_self_data(data, cfg.ai_id, 'most_recent_turn');
    if most_recent_turn==wesnoth.current.turn then return 0 end
    return 290000; -- less than other MAIs, like zone guardians
end

function return_table:execution(cfg,data)
--     wesnoth.interface.add_chat_message('----Regroup Micro AI side '..wesnoth.current.side..' running on turn '..wesnoth.current.turn);
    cfg.retreat_tod           = cfg.retreat_tod==nil and 'none' or cfg.retreat_tod;
    if (cfg.retreat_tod~='none' and cfg.retreat_tod~='daytime' and cfg.retreat_tod~='nighttime') then error('regroup: invalid retreat_tod parameter "'.. (cfg.retreat_tod or 'nil') ..'"', 2) end

    cfg.filter_retreat_target = wml.get_child(cfg, 'filter_retreat_target') or { side=wesnoth.current.side, canrecruit=true };
    cfg.filter_guards         = wml.get_child(cfg, 'filter_guards'        ) or nil;
    cfg.leader_protect_radius = cfg.leader_protect_radius==nil and 3    or cfg.leader_protect_radius;
    cfg.leader_prudence       = cfg.leader_prudence      ==nil and true or cfg.leader_prudence;
    cfg.leader_frustration    = cfg.leader_frustration   ==nil and true or cfg.leader_frustration;

--     wesnoth.wml_actions.modify_side({ T.ai{ recruitment_randomness=0 } });
--     wesnoth.wml_actions.modify_side({ T.ai{ recruitment_diversity=99999 } });
--     wesnoth.wml_actions.modify_side({ T.ai{ leader_value=0 } });
--     wesnoth.wml_actions.modify_side({ T.ai{ village_value=0.1 } });
--     wesnoth.wml_actions.modify_side({ T.ai{ scout_village_targeting=30 } });
--     wesnoth.wml_actions.modify_side({ T.ai{ grouping='no' } });
--     wesnoth.wml_actions.modify_side({ T.ai{ aggression=0.9 } });

    --###########################################################################
    --                              PREPARE ENVIRONMENT
    --###########################################################################
    MAISD_update_self_data(data, cfg.ai_id, 'most_recent_turn', wesnoth.current.turn);

    -- can we fight on this hex, or do we need to retreat?
    -- be very cautious at bad ToDs, and very aggressive at neutral/good ToDs.
    -- if it's a bad ToD now but next turn will be a neutral ToD, be somewhat cautious (assume our enemy is player 1, and thus that they can only counterattack next turn)
    -- at other ToDs, attack even with poor odds unless it's extremely suicidal
    -- remember that the threat evaluations already also factor in each unit's alignment for a +/- 25%
    -- and remember that we consider our units' next-turn moves, but only our enemies' this-turn moves
    local function is_safe_hex( hex, enemy_adjustment )
        local is_good_tod = (
            (cfg.retreat_tod=='none') or -- if retreat_tod=none, assume it's always a good ToD
            (cfg.retreat_tod=='nighttime' and wesnoth.schedule.get_time_of_day(nil,wesnoth.current.turn).lawful_bonus >= 0) or
            (cfg.retreat_tod=='daytime'   and wesnoth.schedule.get_time_of_day(nil,wesnoth.current.turn).lawful_bonus <= 0)
        );
        local is_almost_bad_tod = (
            (cfg.retreat_tod=='nighttime' and wesnoth.schedule.get_time_of_day(nil,wesnoth.current.turn+1).lawful_bonus < 0) or
            (cfg.retreat_tod=='daytime'   and wesnoth.schedule.get_time_of_day(nil,wesnoth.current.turn+1).lawful_bonus > 0)
        );
        local is_almost_good_tod = (
            (cfg.retreat_tod=='nighttime' and wesnoth.schedule.get_time_of_day(nil,wesnoth.current.turn+1).lawful_bonus >= 0) or
            (cfg.retreat_tod=='daytime'   and wesnoth.schedule.get_time_of_day(nil,wesnoth.current.turn+1).lawful_bonus <= 0)
        );
        enemy_adjustment = enemy_adjustment or 0;
        return
            (is_good_tod and not is_almost_bad_tod and hex.allies > (hex.enemies-enemy_adjustment)*0.3) or
            (is_good_tod and     is_almost_bad_tod and hex.allies > (hex.enemies-enemy_adjustment)*0.5) or
            (is_almost_good_tod                    and hex.allies > (hex.enemies-enemy_adjustment)*0.9) or
            (                                          hex.allies > (hex.enemies-enemy_adjustment)*1.3)
    end

    -- unless at least 1/2 of our army is flying units, don't try retreating onto/across unwalkable terrain
    local my_units_all <const> = wesnoth.units.find_on_map({ side=wesnoth.current.side });
    local my_flying_count = 0;
    for i,unit in ipairs(my_units_all) do if unit:movement_on('Qxu')<99 then my_flying_count=my_flying_count+1 end end
    local skip_unwalkable <const> = my_flying_count <= #my_units_all/2;

    -- prepare tests for impassable/unwalkable terrain
    local foot_dummy <const> = wesnoth.units.create{ type='Spearman' };
    local  fly_dummy <const> = wesnoth.units.create{ type='Gryphon'  };
    -- movement_on depends only on the terrain, so cache by terrain code to avoid recomputing it across the many expansion passes
    local impassable_cache = {};
    local function is_impassable(x,y)
        local terrain = wesnoth.current.map[{x, y}];
        if impassable_cache[terrain]==nil then impassable_cache[terrain] = (fly_dummy:movement_on(terrain)==99) end
        return impassable_cache[terrain];
    end
    local unwalkable_cache = {};
    local function is_unwalkable(x,y)
        local terrain = wesnoth.current.map[{x, y}];
        if unwalkable_cache[terrain]==nil then unwalkable_cache[terrain] = (foot_dummy:movement_on(terrain)==99) end
        return unwalkable_cache[terrain];
    end

    -- expand a locationset, with options for impassable/unwalkable terrain
    local function locationset_expand(set,count_arg,args)
        local ignore_impassable = args and args.skip_impassable or false;
        local ignore_unwalkable = args and args.skip_unwalkable or false;
        local count = count_arg or 1;
        for i=1,count,1 do for x,y in set:clone():iter() do
                for u,v in wesnoth.current.map:iter_adjacent(x,y) do
                    if ignore_impassable and is_impassable(u,v) then goto next_hex end
                    if ignore_unwalkable and is_unwalkable(u,v) then goto next_hex end
                    set[{u,v}] = args and args.new_hex_value or true;
                    ::next_hex::
                end
            end
        end
        return set;
    end

    -- prepare WML filters for allied, neutral, and unowned villages
    local is_ally_village    <const> = { T.filter_owner{ T.allied_with{ side=wesnoth.current.side } } };
    local is_enemy_village   <const> = { T.filter_owner{ T.enemy_of{    side=wesnoth.current.side  }} };
    local is_unowned_village <const> = { T['and']{ T.filter_owner{ side=0 }, T['or'](is_enemy_village) } };

    -- helper function to get unit strength
    local function get_unit_strength(unit) return (
        -- weigh cost towards 20; our 1.20 level 3+ unit costs can be a bit inflated
        -- also, fast units tend to pay a premium for their speed (including the quick trait), which isn't actually reflected in their danger. Reduce their cost to help balance that out
        (20*0.5 + 0.5*(unit.cost + 4 - unit.max_moves*math.max(1,unit.level))) *

        -- one one hand, low-hp units still deal full damage, and with careful positioning can often avoid retaliation
        -- on the other hand, very weak enemies may be killed during our turn (before they can counterattack), and the player needs to take special care to protect their veterans
        -- and, letting strength get very low at low hitpoints means we'll retreat if we're losing a 1v1
        -- so just scale strength linearly with hitpoints, from 100% to 0%
        (unit.hitpoints/unit.max_hitpoints) *

        -- value units +/- 25% depending on the current ToD (ignoring next turn for simplicity)
        ai_helper.get_unit_time_of_day_bonus(unit.alignment,  wesnoth.schedule.get_time_of_day(unit.x, unit.y, wesnoth.current.turn).lawful_bonus ) *

        -- slowed unit are worth much less than usual
        (unit.status.slowed and 0.5 or 1)
    ) end

    --###########################################################################
    --                             GENERATE THREATMAP
    --###########################################################################
    local threatmap = location_set.create();
    local wholemap  = location_set.of_pairs(wesnoth.current.map.find{ include_borders=false });
    for x,y in wholemap:iter() do threatmap[{x,y}] = { allies=0, enemies=0 } end
    for i,unit in pairs(wesnoth.units.find_on_map({ T.filter_vision{ visible=true, side=wesnoth.current.side } })) do
        local strength = get_unit_strength(unit);
        if wesnoth.sides.is_enemy(wesnoth.current.side, unit.side) then
            -- for enemies, record threat only in the area they can reach on their next turn
            -- ignoring teleport isn't best play, as the player could teleport in a kill squad of Silver Magi, but...
            -- 1) teleporting kill squads are cool and 2) find_reach checks even unowned or enemy villages, so it thinks teleporters can threaten places they can't actually reach
            local reach = find_reach_with_moves(unit, { ignore_units=true, ignore_teleport=true }, unit.max_moves);
            locationset_expand(reach, 1); -- movement reach -> attack reach
            for x,y in reach:iter() do
                local distance = wesnoth.current.map.distance_between(unit, {x,y});
                -- distance/100 provides a very small distance gradient. This helps us break ties and know which direction the threat is coming from
                threatmap[{x,y}].enemies = threatmap[{x,y}].enemies + strength - distance/100;
            end
        else
            -- for our units or allies who haven't yet taken their turn in the turn order (assuming the player is side 1),
            -- consider two turns of movement. This 1) helps account for our options next turn and 2) helps prevent the first unit in a charge from being too cowardly
            -- but for simplicity, use max_moves*2 instead of actually checking 2 turns (different in the case of rough terrain), and don't check attack range (since locationset_expand doesn't work with return_raw=true)
            -- if we have allies who've already taken their turn in the turn order, only consider one turn of movement.
            local moves = math.ceil(unit.max_moves * (unit.side>=wesnoth.current.side and 2.0 or 1.0));
            local reach = find_reach_with_moves(unit, { ignore_units=true, ignore_teleport=true, return_raw=true }, moves);
            for j,loc in ipairs(reach) do
                -- weigh hexes we can reach in 1 turn at 100%
                -- weigh hexes we can reach in 2 turns between 100% and 0%, depending on move cost
                local move_cost = moves - loc.moves_left;
                local weight = (move_cost <= unit.max_moves) and 1.0 or math.max(0, 1.0 - (move_cost-unit.max_moves)/unit.max_moves);
                local distance = wesnoth.current.map.distance_between(unit, loc);
                -- distance/100 provides a very small distance gradient. This helps us break ties and know which direction the allies are coming from
                threatmap[loc].allies = threatmap[loc].allies + weight*strength - distance/100;
--                 if loc.x==24 and loc.y==26 then wesnoth.interface.add_chat_message(unit.id..' '..unit.name..': '..weight*strength) end
            end
        end
    end

    --###########################################################################
    --                             GENERATE RETREATMAP
    --###########################################################################
    local retreatmap = location_set.create();
    --###############################
    -- PREPARE VALUE MULTIPLIERS
    --###############################
    -- put all these values in one place so it's easier to see how they relate to each other
--     local VALUE__injured_and_adjacent_to_healer <const> = 1; -- plus the usual adjacent-to-ally bonus - TODO implement this
--     local VALUE__healer_and_adjacent_to_injured <const> = 1; -- plus the usual adjacent-to-ally bonus - TODO implement this
    local VALUE__distance_to_leader_over_3  <const> = 1;
    local VALUE__enemies_when_near_leader   <const> = 0.001 -- purely used as a tiebreaker for otherwise identical hexes when near our leader (prefer more threat so we defend our leader better)
    local VALUE__defense                    <const> = 0.1 -- e.g. 40% defense = 4 value. 60% defense = 6 value. Defense is valued higher than threat, so we may leave our leader open briefly if
    local VALUE__capture_village            <const> = 5;
    local VALUE__defend_threatened_village  <const> = 10;
    local VALUE__village_per_healing        <const> = 1; -- this value is multiplied by the healing amount (usually 8)
    local VALUE__village_is_too_dangerous   <const> = -999; -- we blacklist non-village hexes that're dangerous, but we calculate villages per-unit, since they're important to defend if we can
    local VALUE__standoff_in_retreatmap     <const> = 0.1 -- as a tiebreaker, prefer standing off in our retreat area if we happen to overlap

    --###############################
    -- PREPARE LIST OF RETREAT HEXES
    --###############################
    -- only consider hexes that match 1), 2), or 3).
    -- we could allow more hexes, but this is a good heuristic and speeds up performance
    -- 1) within cfg.leader_protect_radius hexes of our leader (if our leader is threatened), or...
    local retreatmap_leader = location_set.of_pairs(wesnoth.current.map.find{ T['filter'](cfg.filter_retreat_target) });
    for x,y in retreatmap_leader:clone():iter() do
        -- only guard near-leader hexes that're actually threatened
		-- only consider enemies: comparing allies-vs-enemies might lead to a situation where we don't try to guard our leader because we have tons of units hiding behind him
        if threatmap[{x,y}].enemies<=0 then retreatmap_leader:remove(x,y) end
    end
    locationset_expand(retreatmap_leader, cfg.leader_protect_radius, { skip_impassable=true, skip_unwalkable=skip_unwalkable });

    -- 2) within 3 hexes of the border where we outnumber our enemy 3x or more, or...
    local danger_hexes = location_set.create();
    local safe_hexes   = location_set.create();
    for x,y,hex in threatmap:iter() do
        if hex.allies >= hex.enemies*3 then safe_hexes:insert(x,y,{ is_border_hex=true })
        else                              danger_hexes:insert(x,y,{ is_border_hex=true }) end
    end
    local retreatmap_border = danger_hexes:clone();
    locationset_expand(retreatmap_border, 3, { skip_impassable=true, skip_unwalkable=skip_unwalkable });
    retreatmap_border:inter_merge(safe_hexes, function(x,y,v1,v2) return type(v1)=='table' and v1 or v2 end );

    -- 3) any village / healing terrain that's not too dangerous
    -- note that is_safe_hex() is a stricter check than enemies*5, and as such
    -- we may still decide to hold healing locations that're inside the retreatmap_border ring
    local retreatmap_villages = location_set.of_pairs( ai_helper.get_healing_locations{} );
    for x,y in retreatmap_villages:clone():iter() do
        if not is_safe_hex(threatmap[{x,y}]) then retreatmap_villages:remove(x,y) end
    end

    -- merge together all the retreatmaps, preserving any table data in them
    retreatmap:union_merge(retreatmap_leader,   function(x,y,v1,v2) return type(v1)=='table' and v1 or v2 end );
    retreatmap:union_merge(retreatmap_border,   function(x,y,v1,v2) return type(v1)=='table' and v1 or v2 end );
    retreatmap:union_merge(retreatmap_villages, function(x,y,v1,v2) return type(v1)=='table' and v1 or v2 end );
    for x,y in retreatmap:iter() do retreatmap[{x,y}]={ value=0 } end

    -- generate standoffmap_unowned_villages.
    -- we use villages and village value as part of our standoffmap
    local standoffmap_unowned_villages = location_set.of_pairs(wesnoth.current.map.find( is_unowned_village ));
    for x,y in standoffmap_unowned_villages:iter() do standoffmap_unowned_villages[{x,y}]={ value=0 } end

    -- remove any hexes that're in our default AI's [avoid] tag
    local avoid_map = ai_helper.get_avoid_map(ai, wml.get_child(cfg, "avoid"), true);
    retreatmap:diff(avoid_map);
    standoffmap_unowned_villages:diff(avoid_map);

    --###############################
    -- ADD GLOBAL BONUSES
    --###############################
    -- "global" as opposed to unit-specific bonuses (e.g. defense), which're added later
    for x,y,hex in retreatmap:iter() do
        -- 1) prefer hexes nearer to our leader
        -- this is the benchmark unit for 1 value. 1 value equals one hex closer to our leader
        -- all other value bonuses/penalties are calibrated around this benchmark
        local best_distance = math.huge;
        for i,leader in pairs(wesnoth.units.find_on_map(cfg.filter_retreat_target)) do
            -- path around impassable hexes, and around unwalkable hexes if we don't command many fliers
            -- don't worry about other terrain costs; it's not feasible to approximate our units' movetypes plus our opponents' movetypes
            local path,distance = wesnoth.paths.find_path({x,y}, {leader.x,leader.y}, { ignore_units=true, max_cost=99, calculate=function(x2, y2, current_cost)
                if (                    is_impassable(x2,y2)) then return 99 end
                if (skip_unwalkable and is_unwalkable(x2,y2)) then return 99 end
                return 1;
            end });
            -- if we're already within our leader_protect_radius, don't prioritize closer goals. Let other factors dominate, like terrain and enemy proximity
            if distance<=cfg.leader_protect_radius then distance=cfg.leader_protect_radius end
            if distance<best_distance then best_distance=distance end
        end
        if best_distance==math.huge then best_distance=0 end -- if we have no leader / can't get to our leader, use 0 instead of math.huge (or else we sometimes avoid retreating to -math.huge value hexes)
        hex.value = hex.value - best_distance * VALUE__distance_to_leader_over_3;

        -- 2) if near our leader, prefer more dangerous hexes. We want to defend our leader, not hide behind him
        if best_distance<=cfg.leader_protect_radius then
            hex.value = hex.value + threatmap[{x,y}].enemies * VALUE__enemies_when_near_leader;
        end

        -- 3) prefer neutral/enemy villages, or owned villages that're threatened by enemies
        -- our per-unit logic will blacklist any villages that're too dangerous for each particular unit
        if wesnoth.current.map.matches(x,y, { owner_side=wesnoth.current.side }) and threatmap[{x,y}].enemies>0 then hex.value=hex.value+VALUE__defend_threatened_village end
        if wesnoth.current.map.matches(x,y, is_unowned_village) then hex.value=hex.value+VALUE__capture_village end
    end
    -- for our standoffmap, only care about capturing neutral/enemy villages (not defending allied villages, otherwise we're likely to retreat to a rear village)
    for x,y,hex in standoffmap_unowned_villages:iter() do
        if wesnoth.current.map.matches(x,y, is_unowned_village) then hex.value=hex.value+VALUE__capture_village end
    end

    --###############################
    -- REMOVE RECRUITMENT HEXES
    --###############################
    -- (moved above the unit loop so the kamikaze cost check below can budget against the finalised retreatmap)
    -- don't block castle hexes needed for recruitment
    -- for each unit we could recruit, pick the retreat castle/keep hex with the lowest threat score and remove it from the retreat list
    -- don't bother figuring out chains of connected castle/keep hexes; just only check hexes that're adjacent to our keep
    -- if we have multiple leaders, make keep space in all of them
    for i,leader in pairs(wesnoth.units.find_on_map({  canrecruit=true, side=wesnoth.current.side  })) do
        local keepX,keepY = ai.suitable_keep(leader);
        retreatmap:remove{keepX,keepY}; -- our leader needs to stand here
        local recruit_count = wesnoth.sides[wesnoth.current.side].gold / ai_helper.get_cheapest_recruit_cost();
        while recruit_count>0 do
            local lowest_value = math.huge;
            local lowest_x = nil;
            local lowest_y = nil;
            local adjacent_castles = location_set.of_pairs(wesnoth.current.map.find{ terrain='K*^*,C*^*,*^K*,*^C*', include_borders=false, T.filter_adjacent_location{x=keepX,y=keepY} });
            for x,y,hex in adjacent_castles:iter() do
                local value = retreatmap[{x,y}] and retreatmap[{x,y}].value or math.huge;
                if value<lowest_value then
                    lowest_value = value;
                    lowest_x = x;
                    lowest_y = y;
                end
            end
            if lowest_x==nil or lowest_y==nil then break end
            retreatmap:remove{lowest_x,lowest_y};
            recruit_count = recruit_count-1;
        end
    end

    --###########################################################################
    --                           SELECT RETREATING UNITS
    --###########################################################################
    --###############################
    -- PREPARE RETREAT TABLES
    --###############################
    local retreating_units = {};
    local standoffish_units = {};

    -- a unit that can't reach the retreat area with its current moves is treated as already lost, so when
    -- budgeting a kamikaze we treat the trapped unit as free. Retreatmap is finalised just above, and
    -- nothing moves during unit selection, so we can cache the result per unit to improve performance
    local kamikaze_units = {};
    local kamikaze_targets = {};
    local kamikaze_cost_cache = {};
    local function effective_cost(unit)
        if kamikaze_cost_cache[unit.id]==nil then
            -- if units can't reach the retreat area, the retreat logic will still try to move them adjacent to an ally who's already in the retreat area
            -- but we can't know that here, since the retreating units haven't moved yet. Just look at the actual retreat area.
            local reach = find_reach_with_moves(unit);
            local can_retreat = false;
            for x,y in reach:iter() do
                if retreatmap[{x,y}] then can_retreat=true; break end
            end
            kamikaze_cost_cache[unit.id] = can_retreat and unit.cost or 0;
        end
        return kamikaze_cost_cache[unit.id];
    end

    -- guards get special handling: hold their post, return to a stationed hex, or grab nearby villages
    -- these lists are populated during selection and acted on in the execute phase, so that nothing actually moves mid-selection
    local guards_to_station = {};
    local guard_ids = {};
    if cfg.filter_guards then
        for j,g in ipairs(wesnoth.units.find_on_map({ side=wesnoth.current.side, T['and'](cfg.filter_guards) })) do
            guard_ids[g.id] = true;
        end
    end

    -- we normally leave our leader(s) to the default AI, but a leader explicitly designated as a guard should be processed too
    local units_to_check = wesnoth.units.find_on_map({ side=wesnoth.current.side, canrecruit=false });
    if cfg.filter_guards then
        for j,g in ipairs(wesnoth.units.find_on_map({ side=wesnoth.current.side, canrecruit=true, T['and'](cfg.filter_guards) })) do
            table.insert(units_to_check, g);
        end
    end

    --###############################
    -- SELECT RETREATING UNITS
    --###############################
    -- for each of our units, check every enemy within our reach, counting ZoC
    -- if we have any attack that looks promising, let the default AI handle it
    -- if all attacks look bad (being much more cautions at bad ToD), we can't act normally
    -- either retreat outright or kinda just hang around, depending on how dangerous our current location is
    for i,myunit in pairs( units_to_check ) do
        --###############################
        -- RECORD GUARD STATIONS
        --###############################
        -- done before the skip checks below, so a guard is always stationed the very first time we see it (even if it's busy this turn)
        local is_guard = guard_ids[myunit.id];
        -- a guard's stationed hex is its goto if one is set, otherwise its current location
        -- ('goto' is a Lua keyword, so it's read via bracket syntax; an unset goto reads as a non-positive coordinate)
        if is_guard and guard_get_station(data, cfg.ai_id, myunit)==nil then
            local destination = myunit['goto'];
            local sx, sy = myunit.x, myunit.y;
            if destination and destination.x>0 and destination.y>0 then sx,sy=destination.x,destination.y end
            MAISD_update_self_data(data, cfg.ai_id, 'stationed_x_'..myunit.id, sx);
            MAISD_update_self_data(data, cfg.ai_id, 'stationed_y_'..myunit.id, sy);
        end

        if table_contains(kamikaze_units,myunit) then goto dont_retreat end
        if myunit.moves==0 then goto dont_retreat end

        --###############################
        -- CHECK EACH HEX IN REACH
        --###############################
        local move_reach = location_set.of_pairs( wesnoth.paths.find_reach(myunit) );
        -- expand movement reach by 1 to get attack reach
        local attack_reach = locationset_expand(move_reach:clone(),1);
        local is_any_enemy_nearby = false;
        for x,y in attack_reach:iter() do
            -------------------------
            -- ONLY CONSIDER HEXES WITH ENEMIES
            -------------------------
            local target = wesnoth.units.find_on_map({  x=x, y=y, T.filter_side{T.enemy_of{ side=wesnoth.current.side }}  })[1];
            if not target then goto next_hex end;
            is_any_enemy_nearby = true;

            -------------------------
            -- ALLOW ATTACKING WHEN LEADER IS THREATENED
            -------------------------
            -- if any enemy is within cfg.leader_protect_radius of our leader,
            -- allow attacking any targets within cfg.leader_protect_radius*2 of our leader (*2 is relevant for units who're cut off from our leader but still close enough to help)
            local nearby_leaders = wesnoth.units.find_on_map({
                -- match our leader
                T['and']{
                    canrecruit='yes', T.filter_side{T.allied_with{ side=wesnoth.current.side }},
                    T['or'](cfg.filter_retreat_target) -- filter_retreat_target overlaps with canrecruit=yes by default, but the user can change it
                },
                -- is any enemy within cfg.leader_protect_radius of our leader?
                T.filter_location{
                    radius=cfg.leader_protect_radius,
                    T.filter{T.filter_side{T.enemy_of{ side=wesnoth.current.side }}},
                },
                -- is this reachable target within cfg.leader_protect_radius*2 of our leader?
                T.filter_location{ x=x, y=y, radius=cfg.leader_protect_radius*2 }
            });
--             if #nearby_leaders>0 then wesnoth.interface.add_chat_message(myunit.name..' is attacking an enemy near an ally leader') end
            if #nearby_leaders>0 then goto dont_retreat end;

            -------------------------
            -- ALLOW ATTACKING IF LOW THREAT
            -------------------------
            -- check all hexes adjacent to this enemy and within our reach
            -- if any one of those reachable adjacent hexes isn't too dangerous, don't force a retreat
            -- note that the hex we flag here may not be the actual hex we attack from, especially if we have many units competing for the same low-threat hex
            local adjacent_hexes = location_set.of_pairs( wesnoth.current.map.find{ x=x, y=y, radius=1 } );
            adjacent_hexes:inter(move_reach);
            for x2,y2 in adjacent_hexes:iter() do
--                 if is_safe_hex( threatmap[{x=x2,y=y2}] ) then wesnoth.interface.add_chat_message(myunit.name..' succeeded safe '..x2..','..y2) end
                if is_safe_hex( threatmap[{x=x2,y=y2}] ) then goto dont_retreat end
--                 if not is_safe_hex( threatmap[{x=x2,y=y2}], 'debug' ) then wesnoth.interface.add_chat_message(myunit.name..' failed safe '..x2..','..y2) end
            end

            -------------------------
            -- ALLOW ATTACKING IF NEAR AN ALLIED PLAYER
            -------------------------
            -- if an allied player has units fighting near this enemy, we need to help the player fight irregardless of the odds
            local target_is_near_player = wesnoth.map.matches(x, y, {
                radius=2,
                T.filter{T.filter_side{
                    controller='human',
                    T.allied_with{ side=wesnoth.current.side }
                }}
            })
            if target_is_near_player then goto dont_retreat end

            -------------------------
            -- ALLOW ATTACKING IF KILLING IS WORTH DYING
            -------------------------
            -- if this enemy's gold cost is higher than ours (knowing that the player usually has more gold than the AI),
            -- and if we (myunit, possibly together with same-side/allied units) have a high chance of killing this enemy,
            -- then it's a good attack and we should let the default AI act
            --
            -- skip any targets we're already planning to kill
            -- subtract -7 from the cost so that we're weighted more towards trading away enemy veterans, rather than (for example) trading a full-hp Grunt to kill a 1-hp Spearman
            -- myunit is always part of the assault, so check it before going any further
            local COST_CAP = target.canrecruit and math.huge or (target.cost - 6);
            if not table_contains(kamikaze_targets, target) and effective_cost(myunit)<COST_CAP
            then
                -------------------------
                -- CONSIDER ALLIES WHO CAN HELP
                -------------------------
                -- see if myunit plus other available units can kill the target together (one unit per hex)
                -- candidate helpers: friendly/allied units that can still attack and are cheap enough to fit the budget. Ignore units who're already committed to a kamikaze
                -- as well as those who're already flagged to retreat/standoff (for performance; if they had any useful attacks they wouldn't have been flagged)
                local possible_attackers = { myunit };
                for j,unit in ipairs(wesnoth.units.find_on_map{}) do
                    if unit.id~=myunit.id and unit.attacks_left>0
                        and effective_cost(myunit) + effective_cost(unit) < COST_CAP
                        and unit.side==wesnoth.current.side -- don't consider allies; their controller (whether AI or player) may decide to do something else
                        and not table_contains(kamikaze_units,    unit)
                        and not table_contains(retreating_units,  unit)
                        and not table_contains(standoffish_units, unit)
                        and not table_contains(guards_to_station, unit)
                    then
                        possible_attackers[#possible_attackers + 1] = unit;
                    end
                end

                -------------------------
                -- DID WE FIND A GOOD ATTACK?
                -------------------------
                local combos = battle_calcs.get_attack_combos_subset(possible_attackers, target, {
                    order_matters = false,
                    skip_presort  = math.huge, -- keep myunit first, so its combos are generated first
                    max_combos    = 200,
                });
                local cache, cache_this_move = {}, {};
                for j,combo in ipairs(combos) do
                    local has_myunit, attackers, destinations, cost = false, {}, {}, 0;
                    for k,a in ipairs(combo) do
                        local u = wesnoth.units.get(math.floor(a.src / 1000), a.src % 1000);
                        -- only consider combinations of attacks that include myunit
                        if u.id == myunit.id then has_myunit=true end
                        attackers[#attackers + 1] = u;
                        destinations[#destinations + 1] = { math.floor(a.dst / 1000), a.dst % 1000 };
                        cost = cost + effective_cost(u);
                    end
                    if has_myunit and cost<COST_CAP then
                        local _, _, _, _, def_combo = battle_calcs.attack_combo_stats(attackers, destinations, target, cache, cache_this_move);
                        local kill_chance = def_combo.hp_chance[0] or 0;
                        -- require a high kill chance, or a decent kill chance with a chance to make the area safe
                        if kill_chance>=0.7 or kill_chance>=0.4 and is_safe_hex( threatmap[{ x=target.x, y=target.y }], get_unit_strength(target) ) then
--                             wesnoth.interface.add_chat_message(myunit.name..' + helpers succeeded killing '..target.name)
--                             for k,u in ipairs(attackers) do wesnoth.interface.add_chat_message(u.name) end
                            for k,u in ipairs(attackers) do table.insert(kamikaze_units, u) end
                            table.insert(kamikaze_targets, target);
                            goto dont_retreat
                        end
                    end
                end
            end
            ::next_hex::
        end

        --###############################
        -- ALLOW ATTACKING IF NOTHING TO ATTACK
        --###############################
        -- if there's no enemies within our reach, the AI has nothing to attack so we don't need special retreat/standoff logic
        -- let the AI act normally; either advancing, capturing a village, or going to heal
        --
        -- yes, that means we might walk forward into very dangerous territory,
        -- but IMO we should err on the side of aggression (like the default AI) rather than be too cautious
        --
        -- this does mean we might have, for example, a bunch of units in the front retreat while a slow unit in the back continues forward
        -- to reduce the likelihood of that ocurreing, we give retreating units low-value goals later-down
        if not is_any_enemy_nearby then
            -- with no enemy in reach, a guard always heads back toward its post (grabbing a nearby village en route if there's one)
            -- movefull_stopunit walks it as far along the path as it can, so it still makes progress even when the station is out of reach this turn
            if is_guard then table.insert(guards_to_station, myunit) end
--             wesnoth.interface.add_chat_message(myunit.name..' has no enemies nearby')
            goto dont_retreat
        end

        --###############################
        -- DISABLE ATTACKING OR FLAG AS RETREATING
        --###############################
        -- if we've gotten here, that means there's enemies nearby but we haven't found any good attacks
        --
        -- A guard that can still reach its (empty) stationed hex this turn returns there.
        -- A guard that's been cut off from its post instead joins the regular retreat/standoff.
        if is_guard and guard_can_reach_station(data, cfg.ai_id, myunit, move_reach) then
            table.insert(guards_to_station, myunit);
        else
            -- for non-guards (or guards who have enemies nearby, but no good attacks, and also can't reach our station this turn):
            -- decide between retreat and standoff. if our adjacent hexes (and by assumption, our current hex too) are fairly safe, standoff. If it's very dangerous, force a retreat
            local standoffish = true;
            for x,y in wesnoth.current.map:iter_adjacent(myunit.x,myunit.y) do
                if not is_safe_hex(threatmap[{x,y}]) then standoffish=false end
            end
            if standoffish
                then table.insert(standoffish_units, myunit);
                else table.insert(retreating_units, myunit);
            end
            -----------------------------------------------
            -- DEBUG; recolor standoff/retreat units
--             if standoffish then
--                 myunit:add_modification('object', { duration='turn', T.effect{apply_to='image_mod',replace='BLEND(0,150,150,0.3)'} });
--                 wesnoth.interface.add_chat_message('standoff: '..myunit.name);
--             else
--                 myunit:add_modification('object', { duration='turn', T.effect{apply_to='image_mod',replace='BLEND(0,0,150,0.3)'} });
--                 wesnoth.interface.add_chat_message('retreating: '..myunit.name);
--             end
            -----------------------------------------------
        end
        ::dont_retreat::
    end
    -----------------------------------------------
    -- DEBUG; label threatmap/retreatmap
--     for x,y,hex in threatmap:iter() do wesnoth.map.remove_label{x=x, y=y-1} end
--     for x,y,hex in threatmap:iter() do if (hex.enemies>0 and hex.allies>0) then
--         wesnoth.current.map.add_label{x=x, y=y-1, text="\n<span font-size='x-small'>\n<span color='#FFB3B2'>"..math.ceil(hex.enemies).."</span>|<span color='#BAFFB9'>"..math.ceil(hex.allies).."</span></span>"};
--     end end
--     if #retreating_units>0 then for x,y,hex in retreatmap:iter() do
--         wesnoth.current.map.add_label{x=x, y=y-1, text="<span font-size='x-small'>\n</span><span font-size='large' color='#FFFFFF'>"..math.ceil(hex.value*10)/10 .."</span>"};
--     end end
    -----------------------------------------------
    -----------------------------------------------
    -- DEBUG; end turn immediately so I can see the units/maps before moves happen
--     if wesnoth.current.side==3 then for i,unit in ipairs(my_units_all) do unit.moves=0; unit.attacks_left=0; end end
    -----------------------------------------------

    --###########################################################################
    --                         EXECUTE RETREAT / STANDOFF
    --###########################################################################
    --###############################
    -- PREPARE RETREAT LOGIC
    --###############################
    -- calculate value for all hexes in our retreatmap/standoffmap
    -- then move to whichever one has the highest value
    local function execute_retreat(myunit, standoffish, override_map)
        local highest_value = 0-math.huge;
        local highest_x = nil;
        local highest_y = nil;
        local reach = find_reach_with_moves(myunit);

        -------------------------
        -- LOAD RETREATMAP
        -------------------------
        -- if we're properly retreating, use the full retreatmap
        local map = retreatmap;
        -- if we're standoffish, consider only standoffmap_unowned_villages hexes, plus any hexes that're adjacent to our current position (don't risk running too far away)
        if standoffish then
            local standoffmap_nearby = location_set.of_pairs(wesnoth.current.map.find{
                T['and']{
                    radius=1, T.filter{id=myunit.id},
                },
                T['or']{
                    T['and']{
                        radius=3, T.filter{id=myunit.id},
                    },
                    terrain='*^V*'
                }
            });
            for x,y in standoffmap_nearby:iter() do standoffmap_nearby[{x,y}]={ value=0 } end
            map = location_set.create();
            map:union_merge(standoffmap_nearby,           function(x,y,v1,v2) return type(v1)=='table' and v1 or v2 end );
            map:union_merge(standoffmap_unowned_villages, function(x,y,v1,v2) return type(v1)=='table' and v1 or v2 end );
            for x,y,hex in map:iter() do
                if retreatmap[{x,y}] then hex.value = hex.value + VALUE__standoff_in_retreatmap end
            end
        end
        if override_map then map=override_map end

        -- get the intersection of our reach and the retreatmap, using inter_merge to retain the retreatmap's values
        reach:inter_merge(map, function(x,y,v1,v2) return v2 end);
        for x,y,hex in reach:iter() do
            -------------------------
            -- CHECK UNIT-SPECIFIC FACTORS
            -------------------------
            -- 1) avoid occupied hexes (check per-unit, not once globally, since units move as part of the retreat)
            local already_occupied = wesnoth.units.find_on_map({ x=x, y=y });
            if next(already_occupied)~=nil and already_occupied[1].id~=myunit.id then goto next_hex end

            -- 2) add value depending on the terrain (for us, not to the base hex)
			local value = hex.value;
            local defense = myunit:defense_on( wesnoth.current.map.get{x,y}.terrain );
            value = value + VALUE__defense * defense;

            -- 3) add value for healing terrain if we're injured and can't regenerate. Don't worry about avoiding dangerous villages - that's handled next step
            if myunit.hitpoints<myunit.max_hitpoints and 0==#(wesnoth.units.find_on_map({ id=myunit.id, ability_type_active='regenerate' })) then
                value = value + VALUE__village_per_healing * wesnoth.terrain_types[wesnoth.current.map[{x,y}]].healing
            end

            -- 4) if this village is too dangerous for us to even try to defend *solo*, give it a massive value penalty.
            -- use x1.5 and +20 strength to represent the improved durability we'll have due to a village's healing, plus the value of delaying the enemy's capture of this village even if we die
            --
            -- usually we simply don't add dangerous hexes to the retreatmap, but villages (and possibly other hexes in the future) are important to defend and thus are added even if they're somewhat dangerous,
            -- so for locations inside danger_hexes, check threat for each hex and each unit individually
            local adjusted_strength = get_unit_strength(myunit) * (defense+60)/100 * 1.5 + 20;
            if wesnoth.current.map.matches(x,y, {terrain='*^V*'}) and danger_hexes[{x,y}] and adjusted_strength<threatmap[{x,y}].enemies
                -- if we're standoffish, only make this check if the village we're considering isn't very nearby
                -- otherwise we may ignore a dangerous adjacent village to instead loiter on an equally dangerous adjacent flat hex
                and (not standoffish or standoffish and wesnoth.current.map.distance_between(myunit,{x,y})>1)
                then
--                 wesnoth.interface.add_chat_message('village too dangerous: '..x..','..y..' ('..adjusted_strength..' vs '..threatmap[{x,y}].enemies..')');
                value = value + VALUE__village_is_too_dangerous;
            end

            -----------------------------------------------
            -- DEBUG; label retreat/standoff candidates
--             wesnoth.interface.add_chat_message('checking retreat/standoff hex: '..x..','..y..' (value '..value..')');
--             wesnoth.current.map.add_label{x=x, y=y-1, text="<span font-size='x-small'>\n</span><span font-size='large' color='#FFFFFF'>"..math.ceil(value).."</span>"};
            -----------------------------------------------
            if value>highest_value then
                highest_value = value;
                highest_x = x;
                highest_y = y;
            end
            ::next_hex::
        end
        -------------------------
        -- IMPLEMENT RETREAT
        -------------------------
        -- if we can reach a retreat hex, move there
        if highest_x~=nil and highest_y~=nil then
            ai_helper.movefull_stopunit(ai, myunit, highest_x, highest_y);
--          wesnoth.interface.add_chat_message('retreating '..myunit.name..' to '..highest_x..','..highest_y..' (value '..highest_value..')');
            return true;
        end
        return false;
    end

    --###############################
    -- EXECUTE RETREAT / STANDOFF LOGIC
    --###############################
    -- for each retreating unit, try to retreat into the retreat area with our current moves
    -- if we can't reach it, try instead to form up adjacent to a unit that's already in the retreat area
    -- if we still can't, give up and let the default AI act; maybe it'll kill something
    for i,myunit in pairs(retreating_units) do
        local success = execute_retreat(myunit);
        -- couldn't reach the retreat area; try to retreat adjacent to a unit that's already in it
        -- (recomputed per unit, since units retreating earlier this loop are now part of the retreat area)
        if not success then
            local adjacent_to_retreat_area = location_set.create();
            for j,ally in pairs(wesnoth.units.find_on_map({ T.filter_side{T.allied_with{ side=wesnoth.current.side }} })) do
                if retreatmap[{ally.x,ally.y}] then
                    for x,y in location_set.of_pairs(wesnoth.current.map.find{ radius=1, T.filter{id=ally.id} }):iter() do
                        adjacent_to_retreat_area[{x,y}] = { value=0 };
                    end
                end
            end
            success = execute_retreat(myunit, false, adjacent_to_retreat_area);
        end
        -----------------------------------------------
        -- DEBUG; recolor desperate attackers
--         if not success then
--             myunit:add_modification('object', { duration='turn', T.effect{apply_to='image_mod',replace='BLEND(150,0,0,0.5)'} });
--             wesnoth.interface.add_chat_message('desperate attack: '..myunit.name);
--         end
        -----------------------------------------------
    end
    -- for each standoffish unit, move to a standoff location (a full retreat might cause us to run too far away)
    -- we're guaranteed to find one, since standoffmap includes our current location
    -- standoffish units only bother with hexes we can actually reach right now
    -- and we don't want to set retreat goals, since we may be waiting for other, slower allies to walk past us
    for i,myunit in pairs(standoffish_units) do
        execute_retreat(myunit,true);
    end

    --###############################
    -- EXECUTE GUARD LOGIC
    --###############################
    -- guards with targets to attack get ignored here; let the default AI handle them
    -- guards with no enemies in reach (or enemies but no viable targets, if we can reach our station) move/stay at their station, or possibly capture a nearby village
    for i,guard in pairs(guards_to_station) do
        local sx, sy = guard_get_station(data, cfg.ai_id, guard);
        -- if we're currently at our stationed location
        -- and can capture a safe neutral/enemy village that's 1) within reach and 2) not already occupied, do so
        if guard.x==sx and guard.y==sy then
            for x,y in find_reach_with_moves(guard):iter() do
                if is_safe_hex(threatmap[{x,y}])
                    and wesnoth.current.map.matches(x,y, {
                        T['and'](is_unowned_village),
                        T['not']{T.filter{}}
                    })
                then
                    ai_helper.movefull_stopunit(ai, guard, x, y);
--                     wesnoth.interface.add_chat_message('guard capturing village: '..guard.name);
                    goto next_guard;
                end
            end
        end
        -- if we're injured, move to a 0-threat, unoccupied village and heal (if one is available within our reach)
        if guard.hitpoints<guard.max_hitpoints then
            for x,y in find_reach_with_moves(guard):iter() do
                local healing_locations = location_set.of_pairs( ai_helper.get_healing_locations{} );
                if threatmap[{x,y}].enemies==0
                    and healing_locations[{x,y}]
                    and wesnoth.current.map.matches(x,y, { T['not']{T.filter{
                        -- if we're currently on a village, it still counts as unoccupied
                        T['not']{ id=guard.id }
                    }} })
                then
                    ai_helper.movefull_stopunit(ai, guard, x, y);
--                     wesnoth.interface.add_chat_message('guard healing: '..guard.name);
                    goto next_guard;
                end
            end
        end
        -- otherwise head back to our post (movefull_stopunit just stops us if we're already standing on it)
        -- the select phase already verified this guard can reach its station, so movefull_stopunit will actually get there
        ai_helper.movefull_stopunit(ai, guard, sx, sy);
--         wesnoth.interface.add_chat_message('guard holding station: '..guard.name);
       ::next_guard::
    end

    --###########################################################################
    --                                 CREATE GOALS
    --###########################################################################
    -- DEBUG: remove old-style goals (for testing with old saves)
    for i=0,99 do
        wesnoth.wml_actions.modify_ai({
            T.allied_with{ side=wesnoth.current.side },
            action='delete',
            path='goal[set_by_regroup'..i..']',
        });
    end
    --###############################
    -- CREATE RETREAT GOALS
    --###############################
    -- this doesn't affect retreating units, but is helpful (for example) for reinforcing units who need to be told not to run past the retreat area
    -- don't use these goals to execute the retreat via the default AI, because the default AI doesn't use goals in the way we want. For example, it'll move a unit onto a low-value goal if it's near several other high-value goals
    -- do this AFTER we retreat, so that we can use the retreated units' new positions
    --
    -- delete old retreat_goal_count goals
    local retreat_goal_count = MAISD_get_self_data(data, cfg.ai_id, 'retreat_goal_count') or 0;
    for i=0,retreat_goal_count do
        wesnoth.wml_actions.modify_ai({
            T.allied_with{ side=wesnoth.current.side },
            action='delete',
            path='goal[regroup_mai_side'..wesnoth.current.side..'_retreat'..i..']',
        });
    end
    -- calibrate our new goals
    -- only create goals that overlap with the positions of units who've just retreated
    -- otherwise we put goals everywhere, messing up reinforcements, village grabbing, and all kinds of other things
    local max_value = 0-math.huge;
    local min_value =   math.huge;
    for x,y,hex in retreatmap:iter() do
        for i,retreated in pairs(retreating_units) do
            if x==retreated.x and y==retreated.y then
                hex.create_goal = true;
                if hex.value>max_value then max_value=hex.value end
                if hex.value<min_value then min_value=hex.value end
                goto next_hex;
            end
        end
        ::next_hex::
    end
    -- create new goals
    retreat_goal_count = 0;
    for x,y,hex in retreatmap:iter() do
        if not hex.create_goal then goto next_hex end
        -- goal value always ranges from 0.5 to 2.5, depending on the hex's value
        -- need a value high enough that we stop nearby units from running by,
        -- but also low enough that we don't pull distant units who have better things to do
        local multiplier = (hex.value-min_value) / (max_value-min_value);
        local value = 0.5 + 2.0 * ( multiplier~=multiplier and 0.5 or multiplier); -- 0/0 (i.e. NaN) defaults to multiplier=0.5
        -- offset the goal one hex away from the dangerous edge of the retreat area, so reinforcing units stop a hex short of it instead of right on it
        -- candidates are the unit's own hex (= no offset) plus its neighbours; pick the best by:
        --   primary  : lowest enemy threat -- the threatmap's .enemies value carries a small distance gradient (see GENERATE THREATMAP), so it points away from the threat
        --   tie-break: between equally-safe hexes (in practice the 0-threat ones, where the gradient is flat) the one bordering the fewest threatened hexes, i.e. tucked deepest into safe territory
        -- the own hex is listed first and ties don't override it, so if nothing beats it the goal just stays put
        local candidates = { {x,y} };
        for u,v in wesnoth.current.map:iter_adjacent(x,y) do candidates[#candidates+1] = {u,v} end
        local goal_x, goal_y, best_threat, best_adjacent;
        for _,candidate in ipairs(candidates) do
            local cx, cy = candidate[1], candidate[2];
            if threatmap[{cx,cy}] and not is_impassable(cx,cy) then
                local threat = threatmap[{cx,cy}].enemies;
                local adjacent = 0;
                for w,z in wesnoth.current.map:iter_adjacent(cx,cy) do
                    if threatmap[{w,z}] and threatmap[{w,z}].enemies>0 then adjacent = adjacent+1 end
                end
                if (not goal_x) or (threat < best_threat) or (threat == best_threat and adjacent < best_adjacent) then
                    goal_x, goal_y, best_threat, best_adjacent = cx, cy, threat, adjacent;
                end
            end
        end
        wesnoth.wml_actions.modify_ai({
            -- create these goals for allies too, in case we're retreating near allies who might otherwise run forward
            T.allied_with{ side=wesnoth.current.side },
            action='add',
            path='goal[]',
            T.goal{
                -- must include the side number in the ID, or else our allies will delete our goals when they run
                id='regroup_mai_side'..wesnoth.current.side..'_retreat'..retreat_goal_count,
                name='target_location',
                value=value,
                T.criteria{ x=goal_x, y=goal_y }
            }
        });
        -----------------------------------------------
        -- DEBUG; label goals
--         wesnoth.current.map.add_label{x=goal_x, y=goal_y-1, text="<span font-size='x-small'>\n</span><span color='#AAAAFF'>" .. math.ceil(value*10)/10 .. "</span>"};
--         wesnoth.interface.add_chat_message('retreat goal #'..retreat_goal_count..' created: '..goal_x..','..goal_y);
        -----------------------------------------------
        retreat_goal_count = retreat_goal_count + 1;
        ::next_hex::
    end
    MAISD_update_self_data(data, cfg.ai_id, 'retreat_goal_count', retreat_goal_count);

    --###############################
    -- CREATE MOVE-TO-ENEMY GOALS
    --###############################
    -- the default AI doesn't move towards enemy units unless they threaten our leader (and it's not very good at determining that until it's too late)
    -- usually this makes sense, but can sometimes lead to cases where a random unit can hide in a corner or sneak around the map edges without being targeted
    -- to fix this, assign a very minor goal to each enemy unit
    --
    -- the default AI also likes to send its units to deal with far-away threats or villages, ignoring enemies who may be flanking around to kill our leader
    -- to help mitigate that, enemies near our leader get stronger goals, while distant enemies get weaker goals
    --
    -- delete old moveToEnemy goals
    local moveToEnemy_goal_count = MAISD_get_self_data(data, cfg.ai_id, 'moveToEnemy_goal_count') or 0;
    for i=0,moveToEnemy_goal_count do
        wesnoth.wml_actions.modify_ai({
            side=wesnoth.current.side,
            action='delete',
            path='goal[regroup_mai_side'..wesnoth.current.side..'_moveToEnemy'..i..']',
        });
    end
    -- create new goals
    moveToEnemy_goal_count = 0;
    for i,enemy in pairs( wesnoth.units.find_on_map({
        T.filter_side{T.enemy_of{ side=wesnoth.current.side }},
        T.filter_vision{ visible=true, side=wesnoth.current.side }
    }) ) do
        -- get turns to reach the nearest leader
        local shortest_cost = math.huge;
        for j,leader in pairs(wesnoth.units.find_on_map(cfg.filter_retreat_target)) do
            local path,cost = wesnoth.paths.find_path({enemy.x,enemy.y}, {leader.x,leader.y}, { ignore_units=true, max_cost=99 });
            if cost<shortest_cost then shortest_cost=cost end
        end
        local turns_to_leader = shortest_cost/enemy.max_moves;

        -- determine goal value
        -- we care a LOT about units near our leader, and very little about units anywhere else
        -- remember that these goals are always spread out, so don't worry too much about having high values
        local value = get_unit_strength(enemy);
            if turns_to_leader<=2 then value=value*0.0060
        elseif turns_to_leader<=3 then value=value*0.0050
        elseif turns_to_leader<=4 then value=value*0.0030
        elseif turns_to_leader<=5 then value=value*0.0010
        else                           value=value*0.0003 end

        -- create the goal
        wesnoth.wml_actions.modify_ai{
            -- unlike retreat goals, don't create move-to-enemy goals for allies,
            -- otherwise each side creates a goal for each enemy and we end up with 2x or 3x the goals
            side   = wesnoth.current.side,
            action = 'add',
            path   = 'goal[]',
            T.goal{
                id='regroup_mai_side'..wesnoth.current.side..'_moveToEnemy'..moveToEnemy_goal_count,
                name  = 'target_location',
                value = value,
                T.criteria{ x=enemy.x, y=enemy.y},
            },
        }
--         wesnoth.interface.add_chat_message('move-to-enemy goal #'..moveToEnemy_goal_count..' created: '..enemy.x..','..enemy.y)
        moveToEnemy_goal_count = moveToEnemy_goal_count + 1;
    end
--     wesnoth.interface.add_chat_message('created '..moveToEnemy_goal_count..' new goals');
    MAISD_update_self_data(data, cfg.ai_id, 'moveToEnemy_goal_count', moveToEnemy_goal_count);

    --###########################################################################
    --                      LEADER FRUSTRATION AND PRUDENCE
    --###########################################################################
    for i,leader in pairs(wesnoth.units.find_on_map({  canrecruit=true, side=wesnoth.current.side  })) do
        local frustration = MAISD_get_self_data(data, cfg.ai_id, 'frustration_'..leader.id) or 0;
        local old_frustration = frustration;

        --###############################
        -- LEADER FRUSTRATION
        --###############################
        -- if we're badly outnumbered, possibly getting spawncamped, and have been for several turns, make our frustrated leader charge recklessly at the enemy and probably die
        -- this is an anti-farming measure: to farm XP you'll have to fight real enough battles to avoid frustrating the enemy leader, instead of killing units the instant they're recruited
        if cfg.leader_frustration then
            -------------------------
            -- INCREASE/DECREASE FRUSTRATION
            -------------------------
            -- no matter how badly outnumbered we are, don't get frustrated if we start each turn with at least a few allied units nearby (including our leader)
            -- care about # of units instead of strength, because that feels more intuitive for the player
            -- and care about # of enemies so that far-away fast enemies fighting with slow allies can't accidentally enrage us (fast enemy threat extends past slow ally strength)
            local nearby_allies = wesnoth.units.find_on_map({
                T['not']{ id=leader.id },
                T.filter_side{T.allied_with{ side=wesnoth.current.side }},
                T.filter_location{ x=leader.x, y=leader.y, radius=cfg.leader_protect_radius },
            });
            local nearby_enemies = wesnoth.units.find_on_map({
                T.filter_side{T.enemy_of{ side=wesnoth.current.side }},
                T.filter_location{ x=leader.x, y=leader.y, radius=cfg.leader_protect_radius+2 },
            });
            local hex = threatmap[{leader.x,leader.y}];
                if #nearby_allies<=0 and #nearby_enemies>=4 and hex.enemies/4>hex.allies then frustration = frustration + 3
            elseif #nearby_allies<=2 and #nearby_enemies>=2 and hex.enemies/3>hex.allies then frustration = frustration + 2
            elseif #nearby_allies<=4 and #nearby_enemies>=0 and hex.enemies/2>hex.allies then frustration = frustration + 1
            else                                                                              frustration = math.max(0, frustration - 1) end
--             wesnoth.interface.add_chat_message('frustration: '..frustration)

            -- no matter how long we've been frustrated for, only require a single turn of safety to restore our normal AI
            if frustration>5 then frustration=5 end

            -------------------------
            -- ENRAGE
            -------------------------
            -- if we're too frustrated, create a new mai to override our leader's normal behavior
            -- use zone_guardian to force even suicidal attacks
            if frustration>=5 and old_frustration<5 then
                wesnoth.game_events.fire('leader_frustrated', leader);
                wesnoth.wml_actions.micro_ai({
                    ca_id='frustration_zone_guardian_'..leader.id,
                    ai_type='zone_guardian',
                    action='add',
                    side=wesnoth.current.side,
                    T.filter{ id=leader.id },
                    T.filter_location{},
                    -- higher than the normal leader CAs, but lower than this MAI
                    -- being lower than this MAI is important, otherwise when we get un-frustrated we'll still attack for one more turn until the MAI can get removed
                    ca_score=280000
                });
            end
            -- if we were frustrated but aren't any more, delete the MAI and go back to normal leader AI
            if frustration<5 and old_frustration>=5 then
                wesnoth.wml_actions.micro_ai({
                    ca_id='frustration_zone_guardian_'..leader.id,
                    ai_type='zone_guardian',
                    action='delete',
                    side=wesnoth.current.side,
                });
            end
            MAISD_update_self_data(data, cfg.ai_id, 'frustration_'..leader.id, frustration);
        end

        --###############################
        -- LEADER PRUDENCE
        --###############################
        -- if we're at risk of moving to a dangerous hex, reduce our moves
        if cfg.leader_prudence and frustration<5 then
            local keepX,keepY = ai.suitable_keep(leader);

            -------------------------
            -- DETERMINE MINIMUM MOVEMENT
            -------------------------
            -- if we have nothing to lose, allow aggressive movement even if the situation is dangerous. Sum up our allies, gold, and income (across the next 3 turns)
            -- compare gold/income/allies in terms of absolute value, not scaled with to enemies, since our leader is a single unit who also doesn't scale.
            -- Note: cfg.leader_protect_radius is a radius (default 3), not a number of moves. But there's no good way to limit our leader to a specific radius, since this MAI doesn't directly control our leader
            -- Note: we exclude our leader himself from our list of allies
            local something_to_lose = threatmap[{keepX,keepY}].allies - get_unit_strength(leader) + wesnoth.sides[wesnoth.current.side].gold + 3*wesnoth.sides[wesnoth.current.side].total_income;
            local min_moves;
                if something_to_lose<=  0 then min_moves=math.huge
            elseif something_to_lose<=100 then min_moves=cfg.leader_protect_radius*3/3
            elseif something_to_lose<=150 then min_moves=cfg.leader_protect_radius*2/3
            elseif something_to_lose<=200 then min_moves=cfg.leader_protect_radius*1/3
            else                               min_moves=cfg.leader_protect_radius*0/3 end

            -- if we're already on our keep and are adjacent to any enemy, allow reducing moves to 0
            -- otherwise the default AI likes to move off its keep to attack that enemy which we were already adjacent to
            -- which both looks stupid and makes it easy for enemies to block our keep
            if leader.x==keepX and leader.y==keepY then
                local adjacent_enemies = wesnoth.units.find_on_map({
                    T.filter_side{T.enemy_of{ side=wesnoth.current.side }},
                    T.filter_location{ x=leader.x, y=leader.y, radius=1 },
                });
                if #adjacent_enemies>0 then min_moves=0 end
            end

            -- never reduce moves below those needed to reach the nearest keep
            local path, keep_moves = wesnoth.paths.find_path(leader, keepX,keepY);
            if min_moves<keep_moves then min_moves=keep_moves end

            -------------------------
            -- CHECK FOR RISKY HEXES
            -------------------------
            -- if any reach hex is unsafe, reduce our moves by 1 and retry. A hex is unsafe if:
            -- 1) the enemy presence outweighs our risk tolerance
            -- 2) we have retreating units and it's in our retreat danger_hexes area
            -- use ^0.9 to reflect that the larger the battle becomes, the less impactful and the more dangerous letting our leader roam becomes
            while leader.moves>min_moves do
                local reach = location_set.of_pairs( wesnoth.paths.find_reach(leader) );
                for x,y in reach:iter() do
                    if  -- remember that "allies" includes our leader himself
                        threatmap[{x,y}].allies^0.9 < threatmap[{x,y}].enemies or
                        #retreating_units>0 and danger_hexes[{x,y}]
                    then
                        leader.moves = leader.moves - 1;
                        goto next_loop;
                    end
                end

                -- if we get here, all hexes in our reach are safe,
                -- so we don't need to reduce moves further
                break;
                ::next_loop::
            end
        end
    end

end

return return_table;




