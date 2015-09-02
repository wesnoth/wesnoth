local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local FAU = wesnoth.require "ai/micro_ais/cas/ca_fast_attack_utils.lua"
local LS = wesnoth.require "lua/location_set.lua"

local ca_fast_combat = {}

function ca_fast_combat:evaluation(ai, cfg, self)
    self.data.move_cache = { turn = wesnoth.current.turn }
    self.data.gamedata = FAU.gamedata_setup()

    if (not self.data.fast_combat_units) or (not self.data.fast_combat_units[1]) then
        self.data.fast_combat_units = wesnoth.get_units { side = wesnoth.current.side }
        if (not self.data.fast_combat_units[1]) then return 0 end

        -- For speed reasons, we'll go through the arrays from the end, so they are sorted backwards
        if cfg.weak_units_first then
            table.sort(self.data.fast_combat_units, function(a,b) return a.hitpoints > b.hitpoints end)
        else
            table.sort(self.data.fast_combat_units, function(a,b) return a.hitpoints < b.hitpoints end)
        end
    end

    -- Exclude hidden enemies, except if attack_hidden_enemies=yes is set in [micro_ai] tag
    local excluded_enemies_map = LS.create()
    if (not cfg.attack_hidden_enemies) then
        local hidden_enemies = wesnoth.get_units {
            { "filter_side", { { "enemy_of", { side = wesnoth.current.side } } } },
            { "filter_vision", { side = wesnoth.current.side, visible = 'no' } }
        }

        for _,e in ipairs(hidden_enemies) do
            excluded_enemies_map:insert(e.x, e.y)
        end
    end

    local aggression = ai.get_aggression()
    if (aggression > 1) then aggression = 1 end
    local own_value_weight = 1. - aggression

    for i = #self.data.fast_combat_units,1,-1 do
        local unit = self.data.fast_combat_units[i]
        local unit_info = FAU.get_unit_info(unit, self.data.gamedata)
        local unit_copy = FAU.get_unit_copy(unit.id, self.data.gamedata)

        if (unit.attacks_left > 0) and (H.get_child(unit.__cfg, 'attack')) then
            local attacks = AH.get_attacks({ unit }, { include_occupied = cfg.include_occupied_attack_hexes })

            if (#attacks > 0) then
                local max_rating, best_target, best_dst = -9e99
                for _,attack in ipairs(attacks) do
                    if (not excluded_enemies_map:get(attack.target.x, attack.target.y)) then
                        local target = wesnoth.get_unit(attack.target.x, attack.target.y)
                        local target_info = FAU.get_unit_info(target, self.data.gamedata)

                        local att_stat, def_stat = FAU.battle_outcome(
                            unit_copy, target, { attack.dst.x, attack.dst.y },
                            unit_info, target_info, self.data.gamedata, self.data.move_cache
                        )

                        local rating, attacker_rating, defender_rating, extra_rating = FAU.attack_rating(
                            { unit_info }, target_info, { { attack.dst.x, attack.dst.y } },
                            { att_stat }, def_stat, self.data.gamedata,
                            { own_value_weight = own_value_weight }
                        )

                        local acceptable_attack = FAU.is_acceptable_attack(attacker_rating, defender_rating, own_value_weight)

                        --print(unit.id, target.id, rating, attacker_rating, defender_rating, extra_rating)
                        --print('   -->' , own_value_weight, defender_rating/attacker_rating, acceptable_attack)

                        if acceptable_attack and (rating > max_rating) then
                            max_rating, best_target, best_dst = rating, target, attack.dst
                        end
                    end
                end

                if best_target then
                    self.data.fast_combat_unit_i = i
                    self.data.fast_target, self.data.fast_dst = best_target, best_dst
                    return cfg.ca_score
                end
            end
        end

        self.data.fast_combat_units[i] = nil
    end

    return 0
end

function ca_fast_combat:execution(ai, cfg, self)
    local unit = self.data.fast_combat_units[self.data.fast_combat_unit_i]
    AH.movefull_outofway_stopunit(ai, unit, self.data.fast_dst.x, self.data.fast_dst.y)

    if (not unit) or (not unit.valid) then return end
    if (not self.data.fast_target) or (not self.data.fast_target.valid) then return end
    if (H.distance_between(unit.x, unit.y, self.data.fast_target.x, self.data.fast_target.y) ~= 1) then return end

    AH.checked_attack(ai, unit, self.data.fast_target)

    self.data.fast_combat_units[self.data.fast_combat_unit_i] = nil
end

return ca_fast_combat
