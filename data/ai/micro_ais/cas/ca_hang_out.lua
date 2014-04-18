local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "lua/location_set.lua"
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"
local MAISD = wesnoth.require "ai/micro_ais/micro_ai_self_data.lua"

local function get_hang_out_units(cfg)
    local units = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", cfg.filter }
    }
    return units
end

local ca_hang_out = {}

function ca_hang_out:evaluation(ai, cfg, self)
    -- Return 0 if the mobilize condition has previously been met
    if MAISD.get_mai_self_data(self.data, cfg.ai_id, "mobilize_units") then
        return 0
    end

    -- Otherwise check if any of the mobilize conditions are now met
    if (cfg.mobilize_condition and wesnoth.eval_conditional(cfg.mobilize_condition))
        or (cfg.mobilize_on_gold_less_than and (wesnoth.sides[wesnoth.current.side].gold < cfg.mobilize_on_gold_less_than))
    then
        MAISD.insert_mai_self_data(self.data, cfg.ai_id, { mobilize_units = true })

        -- Need to unmark all units also (all units, with and without moves)
        local units = wesnoth.get_units { side = wesnoth.current.side, { "and", cfg.filter } }
        for _,unit in ipairs(units) do
            MAIUV.delete_mai_unit_variables(unit, cfg.ai_id)
        end

        return 0
    end

    if get_hang_out_units(cfg)[1] then return cfg.ca_score end
    return 0
end

function ca_hang_out:execution(ai, cfg, self)
    local units = get_hang_out_units(cfg)

    -- Get the locations close to which the units should hang out
    -- cfg.filter_location defaults to the location of the side leader(s)
    local filter_location = cfg.filter_location or {
        { "filter", { side = wesnoth.current.side, canrecruit = "yes" } }
    }

    local width, height = wesnoth.get_map_size()
    local locs = wesnoth.get_locations {
        x = '1-' .. width,
        y = '1-' .. height,
        { "and", filter_location }
    }

    -- Get map of locations to be avoided.
    -- Use [micro_ai][avoid] tag with priority over [ai][avoid].
    -- If neither is given, default to all castle terrain.
    -- ai.get_avoid() cannot be used as it always returns an array, even when the aspect is not set,
    -- and an empty array could also mean that no hexes match the filter
    local avoid_tag
    if cfg.avoid then
        avoid_tag = cfg.avoid
    else
        local ai_tag = H.get_child(wesnoth.sides[wesnoth.current.side].__cfg, 'ai')
        for aspect in H.child_range(ai_tag, 'aspect') do
            if (aspect.id == 'avoid') then
                local facet = H.get_child(aspect, 'facet')
                if facet then
                    avoid_tag = H.get_child(facet, 'value')
                else
                    avoid_tag = { terrain = 'C*,C*^*,*^C*' }
                end
            end
        end
    end
    local avoid_map = LS.of_pairs(wesnoth.get_locations(avoid_tag))

    local max_rating, best_hex, best_unit = -9e99
    for _,unit in ipairs(units) do
        -- Only consider units that have not been marked yet
        if (not MAIUV.get_mai_unit_variables(unit, cfg.ai_id, "moved")) then
            local max_rating_unit, best_hex_unit = -9e99

            -- Check out all unoccupied hexes the unit can reach
            local reach_map = AH.get_reachable_unocc(unit)
            reach_map:iter( function(x, y, v)
                if (not avoid_map:get(x, y)) then
                    for _,loc in ipairs(locs) do
                        -- Main rating is the distance from any of the goal hexes
                        local rating = -H.distance_between(x, y, loc[1], loc[2])

                        -- Fastest unit moves first
                        rating = rating + unit.max_moves / 100.

                        -- Minor penalty for distance from current position of unit
                        -- so that there's not too much shuffling around
                        local rating = rating - H.distance_between(x, y, unit.x, unit.y) / 1000.

                        if (rating > max_rating_unit) then
                            max_rating_unit = rating
                            best_hex_unit = {x, y}
                        end
                    end
                end
            end)

            -- Only consider a unit if the best hex found for it is not its current location
            if (best_hex_unit[1] ~= unit.x) or (best_hex_unit[2] ~= unit.y) then
                if (max_rating_unit > max_rating) then
                    max_rating = max_rating_unit
                    best_hex, best_unit = best_hex_unit, unit
                end
            end
        end
    end

    -- If no valid locations/units were found or all units are in their
    -- respective best locations already, we take moves away from all units
    if (not best_unit) then
        for _,unit in ipairs(units) do
            AH.checked_stopunit_moves(ai, unit)

            -- Also remove the markers
            if unit and unit.valid then MAIUV.delete_mai_unit_variables(unit, cfg.ai_id) end
        end
    else
        -- Otherwise move unit (partial move only) and mark as having been used
        AH.checked_move(ai, best_unit, best_hex[1], best_hex[2])

        if best_unit and best_unit.valid then
            MAIUV.set_mai_unit_variables(best_unit, cfg.ai_id, { moved = true })
        end
    end
end

return ca_hang_out
