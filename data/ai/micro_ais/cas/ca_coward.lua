local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

local function get_coward(cfg)
    local filter = cfg.filter or { id = cfg.id }
    local coward = AH.get_units_with_moves {
        side = wesnoth.current.side,
        { "and", filter }
    }[1]
    return coward
end

local ca_coward = {}

function ca_coward:evaluation(ai, cfg)
    if get_coward(cfg) then return cfg.ca_score end
    return 0
end

-- cfg parameters: id, distance, seek_x, seek_y, avoid_x, avoid_y
function ca_coward:execution(ai, cfg)
    local coward = get_coward(cfg)
    local reach = wesnoth.find_reach(coward)

    -- enemy units within reach
    local filter_second = cfg.filter_second or { { "filter_side", {{"enemy_of", {side = wesnoth.current.side} }} } }
    local enemies = wesnoth.get_units {
        { "and", filter_second },
        { "filter_location", {x = coward.x, y = coward.y, radius = cfg.distance} }
    }

    -- if no enemies are within reach: keep unit from doing anything and exit
    if not enemies[1] then
        AH.checked_stopunit_all(ai, coward)
        return
    end

    -- Go through all hexes the unit can reach
    for i,r in ipairs(reach) do

        -- only consider unoccupied hexes
        local occ_hex = wesnoth.get_units { x=r[1], y=r[2], { "not", { id = coward.id } } }[1]
        if not occ_hex then
            -- Find combined distance weighting of all enemy units within distance
            local value = 0
            for j,e in ipairs(enemies) do
                local d = H.distance_between(r[1], r[2], e.x, e.y)
                value = value + 1/ d^2
            end
            --wesnoth.fire("label", {x=r[1], y=r[2], text = math.floor(value*1000) } )

            -- Store this weighting in the third field of each 'reach' element
            reach[i][3] = value
        else
            reach[i][3] = 9999
        end
    end

    -- Sort 'reach' by values, smallest first
    table.sort(reach, function(a, b) return a[3] < b[3] end )
    -- Select those within factor 2 of the minimum
    local best_pos = AH.filter(reach, function(tmp) return tmp[3] < reach[1][3]*2 end)

    -- Now take 'seek' and 'avoid' into account
    for i,b in ipairs(best_pos) do

        -- weighting based on distance from 'seek' and 'avoid'
        local ds = AH.generalized_distance(b[1], b[2], cfg.seek_x, cfg.seek_y)
        local da = AH.generalized_distance(b[1], b[2], cfg.avoid_x, cfg.avoid_y)
        --items.place_image(b[1], b[2], "items/ring-red.png")
        local value = 1 / (ds+1) - 1 / (da+1)^2 * 0.75

        --wesnoth.fire("label", {x=b[1], y=b[2], text = math.floor(value*1000) } )
        best_pos[i][3] = value
    end

    -- Sort 'best_pos" by value, largest first
    table.sort(best_pos, function(a, b) return a[3] > b[3] end)
    -- and select all those that have the maximum score
    local best_overall = AH.filter(best_pos, function(tmp) return tmp[3] == best_pos[1][3] end)

    -- As final step, if there are more than one remaining locations,
    -- we take the one with the minimum score in the distance-from_enemy criterion
    local min, mx, my = 9999, 0, 0
    for i,b in ipairs(best_overall) do

        --items.place_image(b[1], b[2], "items/ring-white.png")
        local value = 0
        for j,e in ipairs(enemies) do
            local d = H.distance_between(b[1], b[2], e.x, e.y)
            value = value + 1/d^2
        end

        if value < min then
            min = value
            mx,my = b[1], b[2]
        end
    end
    --items.place_image(mx, my, "items/ring-gold.png")

    AH.movefull_stopunit(ai, coward, mx, my)
    if (not coward) or (not coward.valid) then return end

    AH.checked_stopunit_all(ai, coward)
end

return ca_coward
