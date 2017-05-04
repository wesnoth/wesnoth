local H = wesnoth.require "helper"
local AH = wesnoth.require("ai/lua/ai_helper.lua")
local LS = wesnoth.require "location_set"

local recruit_type

local ca_recruit_random = {}

function ca_recruit_random:evaluation(cfg)
    -- Random recruiting from all the units the side has

    -- Check if leader is on keep
    local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]
    if (not leader) or (not wesnoth.get_terrain_info(wesnoth.get_terrain(leader.x, leader.y)).keep) then
        return 0
    end

    -- Find all connected castle hexes
    local castle_map = LS.of_pairs({ { leader.x, leader.y } })
    local width, height, border = wesnoth.get_map_size()
    local new_castle_hex_found = true

    while new_castle_hex_found do
        new_castle_hex_found = false
        local new_hexes = {}

        castle_map:iter(function(x, y)
            for xa,ya in H.adjacent_tiles(x, y) do
                if (not castle_map:get(xa, ya))
                    and (xa >= 1) and (xa <= width)
                    and (ya >= 1) and (ya <= height)
                then
                    local is_castle = wesnoth.get_terrain_info(wesnoth.get_terrain(xa, ya)).castle

                    if is_castle then
                        table.insert(new_hexes, { xa, ya })
                        new_castle_hex_found = true
                    end
                end
            end
        end)

        for _,hex in ipairs(new_hexes) do
            castle_map:insert(hex[1], hex[2])
        end
    end

    -- Check if there is space left for recruiting
    local no_space = true
    castle_map:iter(function(x, y)
        local unit = wesnoth.get_unit(x, y)
        if (not unit) then
            no_space = false
        end
    end)
    if no_space then return 0 end

    -- Set up the probability array
    local probabilities, probability_sum  = {}, 0

    -- Go through all the types listed in [probability] tags (which can be comma-separated lists)
    for prob in H.child_range(cfg, "probability") do
        types = AH.split(prob.type, ",")
        for _,typ in ipairs(types) do  -- 'type' is a reserved keyword in Lua
            -- If this type is in the recruit list, add it
            for _,recruit in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
                if (recruit == typ) then
                    probabilities[typ] = { value = prob.probability }
                    probability_sum = probability_sum + prob.probability
                    break
                end
            end
        end
    end

    -- Now we add in all the unit types not listed in [probability] tags
    for _,recruit in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
        if (not probabilities[recruit]) then
            probabilities[recruit] = { value = 1 }
            probability_sum = probability_sum + 1
        end
    end

    -- Now eliminate all those that are too expensive (unless cfg.skip_low_gold_recruiting is set)
    if cfg.skip_low_gold_recruiting then
        for typ,probability in pairs(probabilities) do  -- 'type' is a reserved keyword in Lua
            if (wesnoth.unit_types[typ].cost > wesnoth.sides[wesnoth.current.side].gold) then
                probability_sum = probability_sum - probability.value
                probabilities[typ] = nil
            end
        end
    end

    -- Now set up the cumulative probability values for each type
    -- Both min and max need to be set as the order of pairs() is not guaranteed
    local cum_prob = 0
    for typ,probability in pairs(probabilities) do
        probabilities[typ].p_i = cum_prob
        cum_prob = cum_prob + probability.value
        probabilities[typ].p_f = cum_prob
    end

    -- We always call the exec function, no matter if the selected unit is affordable
    -- The point is that this will blacklist the CA if an unaffordable recruit was
    -- chosen -> no cheaper recruits will be selected in subsequent calls
    if (cum_prob > 0) then
        local rand_prob = math.random(cum_prob)
        for typ,probability in pairs(probabilities) do
            if (probability.p_i < rand_prob) and (rand_prob <= probability.p_f) then
                recruit_type = typ
                break
            end
        end
    else
        recruit_type = wesnoth.sides[wesnoth.current.side].recruit[1]
    end

    return cfg.ca_score
end

function ca_recruit_random:execution(cfg)
    -- Let this function blacklist itself if the chosen recruit is too expensive
    if wesnoth.unit_types[recruit_type].cost <= wesnoth.sides[wesnoth.current.side].gold then
        AH.checked_recruit(ai, recruit_type)
    end
end

return ca_recruit_random
