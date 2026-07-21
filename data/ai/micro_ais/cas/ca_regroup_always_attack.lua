
local T = wml.tag;
local ai_helper = wesnoth.require 'ai/lua/ai_helper.lua';
local return_table = {};

--###########################################################################
--                                   MAIN
--###########################################################################
function return_table:evaluation(cfg,data)
    return 500; -- after every other CA
end

function return_table:execution(cfg,data)
    cfg.always_attack_at_turn_end = cfg.always_attack_at_turn_end==nil and true or cfg.always_attack_at_turn_end;

    -- The default AI will sometimes move low-hp units adjacent to an enemy but not attack, since the low-hp units would be likely to die.
    -- This both looks stupid and makes it easy for the player to distibute XP.
    -- Instead, always force non-leader units to attack if they would've otherwise ended their turn adjacent to an enemy.
    -- skip if on a village, but not other healing terrain. Standing on a village provides a tiny bit of value even if we die - the enemy needs to use a unit to cap the village - which Oasis doesn't provide.
    if cfg.always_attack_at_turn_end then
        -- find every matching unit
        for i,myunit in pairs(  wesnoth.units.find_on_map({
            side=wesnoth.current.side,
            canrecruit=false,
            T.filter_adjacent{ T.filter_side{T.enemy_of{ side=wesnoth.current.side }} },
            T.filter_location{ T['not']{ terrain='*^V*' }},
        }) ) do
            if myunit.attacks_left<1 then goto next_unit end

            -- rate adjacent enemies by (chance to kill)^2 * (gold value)
            -- fall back to preferring low-hp targets as a tiebreaker when kill chances are equal (e.g. both 0)
            local max_rating, best_target = - math.huge, nil;
            for xa,ya in wesnoth.current.map:iter_adjacent(myunit) do
                local target = wesnoth.units.get(xa, ya);
                if target and wesnoth.sides.is_enemy(myunit.side, target.side) then
                    -- simulate the best weapon choice: take the highest kill chance over our attacks
                    local ctk = 0;
                    for j=1, #myunit.attacks do
                        local att_stats, def_stats = wesnoth.simulate_combat(myunit, j, target);
                        if def_stats.hp_chance[0]>ctk then ctk=def_stats.hp_chance[0] end
                    end
                    -- value each unit at its cost, plus a small amount of value per point of experience
                    -- consider both the value and the ctk when evaluating each attack. Square the ctk to devalue very low-odds attacks, even against very expensive targets.
                    -- when rating is exactly the same (very unlikely) use target.hitpoints to break ties
                    local gold_value = target.cost + target.experience * 0.2;
                    local rating = ctk^2 * gold_value;
                    rating = rating - target.hitpoints * 0.001;
                    if rating > max_rating then max_rating, best_target = rating, target end
                end
            end
            -- execute the attack
            if best_target then ai_helper.checked_attack(ai, myunit, best_target) end
            ::next_unit::
        end
    end
end

return return_table;




