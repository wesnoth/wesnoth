local AH = wesnoth.require "ai/lua/ai_helper.lua"
local LS = wesnoth.require "location_set"

return function(cfg)
    -- Find the area that the sheep can occupy
    -- First, find all contiguous hexes around center hex that are inside herding_perimeter
    local location_filter = wml.get_child(cfg, "filter_location")
    local herd_loc = AH.get_named_loc_xy('herd', cfg)
    local herding_area = LS.of_pairs(wesnoth.map.find {
        x = herd_loc[1],
        y = herd_loc[2],
        radius = 999,
        wml.tag.filter_radius { wml.tag["not"] ( location_filter ) }
    })

    -- Then, also exclude hexes next to herding_perimeter; some of the functions work better like that
    herding_area:iter( function(x, y, v)
        for xa, ya in wesnoth.current.map:iter_adjacent(x, y) do
            if (wesnoth.map.matches(xa, ya, location_filter) ) then
                herding_area:remove(x, y)
            end
        end
    end)

    return herding_area
end
