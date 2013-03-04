return {
    init = function(ai)

        local priority_target = {}

        local H = wesnoth.require "lua/helper.lua"
        local W = H.set_wml_action_metatable {}

        function priority_target:change_attacks_aspect(target_id)
            -- Set 'attacks' aspect so that only unit with id=target_id
            -- is attacked if it can be reached, delete aspect otherwise

            -- The following can be simplified significantly once the 'attacks' variable is available
            -- All units that have attacks left (but are not leaders)
            local attackers = wesnoth.get_units{side = wesnoth.current.side, canrecruit = "no", formula = "$this_unit.attacks_left > 0"}
            --print("\nAttackers:",#attackers)

            -- This gets set to >0 if unit that can attack target is found
            local target_in_reach

            -- See if any of those units can reach our target(s)
            for i,u in ipairs(attackers) do

                -- Need to find reachable hexes that are
                -- 1. next to target
                -- 2. not occupied by an allied unit (except for unit itself)
                W.store_reachable_locations {
                    { "filter", { id = u.id } },
                    { "filter_location", {
                        { "filter_adjacent_location", {
                            { "filter", { id = target_id } }
                        } },
                        { "not", {
                            { "filter", { { "not", { id = u.id } } } }
                        } }
                    } },
                    moves = "current",
                    variable = "tmp_locs"
                }
                local tir = H.get_variable_array("tmp_locs")
                W.clear_variable { name = "tmp_locs" }
                --print("reachable locs:",u.id,#tir)

                -- If unit can reach a target -> set variable to 1
                if (#tir > 0) then target_in_reach = true end
            end

            -- Always delete the attacks aspect first, so that we do not end up with 100 copies of the facet
            --print("Deleting attacks aspect")
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

            -- If the target is in reach, set the 'attacks' aspect accordingly ...
            if target_in_reach then
                --print("Setting attacks aspect")
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

                -- We also want to set
                -- 'aggression=1' and 'caution=0', otherwise there could be turns on which nothing happens
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
