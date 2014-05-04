local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local BC = wesnoth.require "ai/lua/battle_calcs.lua"

local ca_fast_combat = {}

function ca_fast_combat:evaluation(ai, cfg, self)
    if (not self.data.fast_cache) or (self.data.fast_cache.turn ~= wesnoth.current.turn) then
        self.data.fast_cache = { turn = wesnoth.current.turn }
    end

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

    local aggression = ai.get_aggression()
    if (aggression > 1) then aggression = 1 end

    for i = #self.data.fast_combat_units,1,-1 do
        local unit = self.data.fast_combat_units[i]

        if (unit.attacks_left > 0) and (H.get_child(unit.__cfg, 'attack')) then
            local attacks = AH.get_attacks({ unit }, { include_occupied = cfg.include_occupied_attack_hexes })

            if (#attacks > 0) then
                local max_rating, best_target, best_dst = -9e99
                for _,attack in ipairs(attacks) do
                    local target = wesnoth.get_unit(attack.target.x, attack.target.y)
                    local rating = BC.attack_rating(
                        unit, target, { attack.dst.x, attack.dst.y },
                        { own_value_weight = 1.0 - aggression },
                        self.data.fast_cache
                    )

                    if (rating > 0) and (rating > max_rating) then
                        max_rating, best_target, best_dst = rating, target, attack.dst
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

    if unit and unit.valid and self.data.fast_target and self.data.fast_target.valid then
        AH.checked_attack(ai, unit, self.data.fast_target)
    end

    self.data.fast_combat_units[self.data.fast_combat_unit_i] = nil
end

return ca_fast_combat
