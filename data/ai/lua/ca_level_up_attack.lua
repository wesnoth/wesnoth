-- An example CA that tries to level up units by attacking weakened enemies.
-- Ported from level_up_attack_eval.fai and level_up_attack_move.fai

local LS = wesnoth.require "location_set"
local F = wesnoth.require "functional"
local level_up_attack = {}

local function kill_xp(unit)
    local ratio = unit.level
    if ratio == 0 then ratio = 0.5 end
    return wesnoth.game_config.kill_experience * ratio
end

local function get_best_defense_loc(moves, attacker, victim)
    local attack_spots = F.filter(moves, function(v)
        return wesnoth.map.distance_between(v, victim) == 1
    end)
    return F.choose(attack_spots, function(loc)
        return attacker:defense_on(loc)
    end)
end

local function iter_possible_targets(moves, attacker)
    moves = LS.of_pairs(moves)
    -- The criteria are: a) unit is reachable b) unit's health is low
    local targets = wesnoth.units.find({

    })
    return coroutine.wrap(function()
        local checked = LS.create()
        moves:iter(function(to_x, to_y)
            for adj_x, adj_y in wesnoth.current.map:iter_adjacent(to_x, to_y) do
                if not checked:get(adj_x, adj_y) then
                    checked:insert(adj_x, adj_y)
                    local u = wesnoth.units.get(adj_x, adj_y)
                    if u and u.hitpoints / u.max_hitpoints < 0.2 then
                        coroutine.yield(u)
                    end
                end
            end
        end)
    end)
end

local possible_attacks

function level_up_attack:evaluation(cfg, data, filter_own)
    possible_attacks = LS.create()
    local moves = LS.of_raw(ai.get_src_dst())
    local units = wesnoth.units.find(filter_own)
    for _,me in ipairs(units) do
        wesnoth.interface.handle_user_interact()
        local save_x, save_y = me.x, me.y
        if not moves[me] or #moves[me] == 0 then
            goto continue
        end
        if kill_xp(me) <= (me.max_experience - me.experience) then
            goto continue
        end
        for target in iter_possible_targets(moves[me], me) do
            local defense_loc = get_best_defense_loc(moves[me], me, target)
            me:to_map(defense_loc.x, defense_loc.y)
            local attacker_outcome, defender_outcome = wesnoth.simulate_combat(me, target)
            -- Only consider attacks where
            -- a) there's a chance the defender dies and
            -- b) there's no chance the attacker dies
            if defender_outcome.hp_chance[0] == 0 or attacker_outcome.hp_chance[0] > 0 then
                goto continue
            end
            -- If killing the defender is the most likely result, save this as a possible attack
            local best = F.choose_map(defender_outcome.hp_chance, function(k, v) return v end)
            if best.key == 0 then
                possible_attacks:insert(defense_loc, {
                    chance = defender_outcome.hp_chance[0],
                    attacker = me, target = target
                })
            end
        end
        ::continue::
        me:to_map(save_x, save_y)
    end
    local _, best_score = F.choose_map(possible_attacks:to_map(), 'chance')
    return math.max(0, best_score) * 100000
end

function level_up_attack:execution(cfg, data)
    local best_attack = F.choose_map(possible_attacks:to_map(), 'chance')
    ai.move(best_attack.value.attacker, best_attack.key)
    ai.attack(best_attack.value.attacker, best_attack.value.target)
end

return level_up_attack
