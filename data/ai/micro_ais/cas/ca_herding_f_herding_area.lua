local H = wesnoth.require "helper"
local LS = wesnoth.require "location_set"

return function(cfg)
    -- Find the area that the sheep can occupy
    -- First, find all contiguous hexes around center hex that are inside herding_perimeter
    local location_filter = H.get_child(cfg, "filter_location")
    local herding_area = LS.of_pairs(wesnoth.get_locations {
        x = cfg.herd_x,
        y = cfg.herd_y,
        radius = 999,
        { "filter_radius", { { "not", location_filter } } }
    })

    -- Then, also exclude hexes next to herding_perimeter; some of the functions work better like that
    herding_area:iter( function(x, y, v)
        for xa, ya in H.adjacent_tiles(x, y) do
            if (wesnoth.match_location(xa, ya, location_filter) ) then
                herding_area:remove(x, y)
            end
        end
    end)

    return herding_area
end
