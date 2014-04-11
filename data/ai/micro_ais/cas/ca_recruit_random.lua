local AH = wesnoth.require("ai/lua/ai_helper.lua")

local recruit

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
            }}
        }
    }
    local no_space = true
    for i,c in ipairs(castle.locs) do
        local unit = wesnoth.get_unit(c[1], c[2])
        if (not unit) then
            no_space = false
            break
        end
    end
    if no_space then return 0 end

    -- Set up the probability array
    local prob, prob_sum  = {}, 0

    -- Go through all the types listed in [probability] tags (which can be comma-separated lists)
    for i,tmp in ipairs(cfg.type) do
        tmp = AH.split(tmp, ",")
        for j,t in ipairs(tmp) do
            -- If this type is in the recruit list, add it
            for k,r in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
                if (r == t) then
                    prob[t] = { value = cfg.prob[i] }
                    prob_sum = prob_sum + cfg.prob[i]
                    break
                end
            end
        end
    end

    -- Now we add in all the unit types not listed in [probability] tags
    for i,r in ipairs(wesnoth.sides[wesnoth.current.side].recruit) do
        if (not prob[r]) then
            prob[r] = { value = 1 }
            prob_sum =prob_sum + 1
        end
    end

    -- Now eliminate all those that are too expensive (unless cfg.skip_low_gold_recruiting is set)
    if cfg.skip_low_gold_recruiting then
        for typ,pr in pairs(prob) do
            if (wesnoth.unit_types[typ].cost > wesnoth.sides[wesnoth.current.side].gold) then
                --print('Eliminating:', typ)
                prob_sum = prob_sum - pr.value
                prob[typ] = nil
            end
        end
    end

    -- Now set up the min/max values for each type
    -- This needs to be done manually as the order of pairs() is not guaranteed
    local cum_prob, n_recruits = 0, 0
    for typ,pr in pairs(prob) do
        prob[typ].p_i = cum_prob
        cum_prob = cum_prob + pr.value / prob_sum * 1e6
        prob[typ].p_f = cum_prob
        n_recruits = n_recruits + 1
    end

    -- Now we're ready to pick on of those
    -- We always call the exec function, no matter if the selected unit is affordable
    -- The point is that this will blacklist the CA if an unaffordable recruit was
    -- chosen -> no cheaper recruits will be selected in subsequent calls
    if (n_recruits > 0) then
        local rand_prob = math.random(1e6)
        for typ,pr in pairs(prob) do
            if (pr.p_i <= rand_prob) and (rand_prob < pr.p_f) then
                recruit = typ
                break
            end
        end
    else
        recruit = wesnoth.sides[wesnoth.current.side].recruit[1]
    end

    return cfg.ca_score
end

function ca_recruit_random:execution(ai, cfg)
    -- Let this function blacklist itself if the chosen recruit is too expensive
    if wesnoth.unit_types[recruit].cost <= wesnoth.sides[wesnoth.current.side].gold then
        AH.checked_recruit(ai, recruit)
    end
end

return ca_recruit_random
