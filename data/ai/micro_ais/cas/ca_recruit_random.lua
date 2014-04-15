local AH = wesnoth.require("ai/lua/ai_helper.lua")

local recruit_type

local ca_recruit_random = {}

function ca_recruit_random:evaluation(ai, cfg)
    -- Random recruiting from all the units the side has

    -- Check if leader is on keep
    local leader = wesnoth.get_units { side = wesnoth.current.side, canrecruit = 'yes' }[1]
    if (not leader) or (not wesnoth.get_terrain_info(wesnoth.get_terrain(leader.x, leader.y)).keep) then
        return 0
    end

    -- Check if there is space left for recruiting
    local width, height, border = wesnoth.get_map_size()
    local castle = {
        locs = wesnoth.get_locations {
            x = "1-"..width, y = "1-"..height,
            { "and", {
                x = leader.x, y = leader.y, radius = 200,
                { "filter_radius", { terrain = 'C*,K*,C*^*,K*^*,*^K*,*^C*' } }
            } }
        }
    }

    local no_space = true
    for _,loc in ipairs(castle.locs) do
        local unit = wesnoth.get_unit(loc[1], loc[2])
        if (not unit) then
            no_space = false
            break
        end
    end
    if no_space then return 0 end

    -- Set up the probability array
    local probabilities, probability_sum  = {}, 0

    -- Go through all the types listed in [probability] tags (which can be comma-separated lists)
    -- Types and probabilities are put into cfg.type and cfg.prob arrays by micro_ai_wml_tag.lua
    for ind,types in ipairs(cfg.type) do
        types = AH.split(types, ",")
        for _,typ in ipairs(types) do  -- 'type' is a reserved keyword in Lua
            -- If this type is in the recruit list, add it
            for _,recruit in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
                if (recruit == typ) then
                    probabilities[typ] = { value = cfg.prob[ind] }
                    probability_sum = probability_sum + cfg.prob[ind]
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

function ca_recruit_random:execution(ai, cfg)
    -- Let this function blacklist itself if the chosen recruit is too expensive
    if wesnoth.unit_types[recruit_type].cost <= wesnoth.sides[wesnoth.current.side].gold then
        AH.checked_recruit(ai, recruit_type)
    end
end

return ca_recruit_random
