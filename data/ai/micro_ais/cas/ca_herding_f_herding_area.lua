local H = wesnoth.require "lua/helper.lua"
local LS = wesnoth.require "lua/location_set.lua"

return function(cfg)
    -- Find the area that the sheep can occupy
    -- First, find all contiguous hexes around center hex that are inside herding_perimeter
    local herding_area = LS.of_pairs(wesnoth.get_locations {
        x = cfg.herd_x,
        y = cfg.herd_y,
        radius = 999,
        { "filter_radius", { { "not", cfg.filter_location } } }
    })

    -- Then, also exclude hexes next to herding_perimeter; some of the functions work better like that
    herding_area:iter( function(x, y, v)
        for xa, ya in H.adjacent_tiles(x, y) do
            if (wesnoth.match_location(xa, ya, cfg.filter_location) ) then
                herding_area:remove(x, y)
            end
        end
    end)

    return herding_area
end
