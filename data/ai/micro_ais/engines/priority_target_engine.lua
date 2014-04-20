return {
    init = function(ai)
        local priority_target = {}

        local H = wesnoth.require "lua/helper.lua"
        local W = H.set_wml_action_metatable {}
        local AH = wesnoth.require "ai/lua/ai_helper.lua"

        function priority_target:change_attacks_aspect(target_id)
            -- Set 'attacks' aspect so that only unit with id=@target_id
            -- is attacked if it can be reached, delete aspect otherwise

            local attackers = AH.get_units_with_attacks {
                side = wesnoth.current.side,
                canrecruit = "no"
            }

            local attack_locs = {}
            for _,attacker in ipairs(attackers) do
                -- Need to find reachable hexes that are
                -- 1. next to target
                -- 2. not occupied by an allied unit (except for unit itself)
                W.store_reachable_locations {
                    { "filter", { id = attacker.id } },
                    { "filter_location", {
                        { "filter_adjacent_location", {
                            { "filter", { id = target_id } }
                        } },
                        { "not", {
                            { "filter", { { "not", { id = attacker.id } } } }
                        } }
                    } },
                    moves = "current",
                    variable = "tmp_locs"
                }
                attack_locs = H.get_variable_array("tmp_locs")
                W.clear_variable { name = "tmp_locs" }
                if (#attack_locs > 0) then break end
            end

            -- Always delete the attacks aspect first, so that we do not end up with 100 copies of the facet
            W.modify_ai {
                side = wesnoth.current.side,
                action = "try_delete",
                path = "aspect[attacks].facet[limited_attack]"
            }

            -- Also delete aggression, caution - for the same reason
            W.modify_ai {
                side = wesnoth.current.side,
                action = "try_delete",
                path = "aspect[aggression].facet[*]"
            }
            W.modify_ai {
                side = wesnoth.current.side,
                action = "try_delete",
                path = "aspect[caution].facet[*]"
            }

            -- If the target can be attacked, set the attacks aspect accordingly
            if attack_locs[1] then
                W.modify_ai {
                    side = wesnoth.current.side,
                    action = "add",
                    path = "aspect[attacks].facet",
                    { "facet", {
                       name = "ai_default_rca::aspect_attacks",
                       id = "limited_attack",
                       invalidate_on_gamestate_change = "yes",
                       { "filter_enemy", { id = target_id } }
                    } }
                }

                -- We also want to set aggression=1 and caution=0,
                -- otherwise there could be turns on which nothing happens
                W.modify_side {
                        side = wesnoth.current.side,
                        { "ai", { aggression = 1, caution = 0 } }
                }
            end

            return 0
        end

        return priority_target
    end
}
