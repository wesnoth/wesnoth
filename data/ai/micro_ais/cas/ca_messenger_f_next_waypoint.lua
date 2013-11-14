local H = wesnoth.require "lua/helper.lua"
local AH = wesnoth.require "ai/lua/ai_helper.lua"

return function(messenger, cfg, self)
    -- Variable to store which waypoint to go to next (persistent)
    if (not self.data.next_waypoint) then self.data.next_waypoint = 1 end

    local waypoint_x = AH.split(cfg.waypoint_x, ",")
    local waypoint_y = AH.split(cfg.waypoint_y, ",")
    for i,w in ipairs(waypoint_x) do
        waypoint_x[i] = tonumber(waypoint_x[i])
        waypoint_y[i] = tonumber(waypoint_y[i])
    end

    -- If we're within 3 hexes of the next waypoint, we go on to the one after that
    -- except if that one's the last one already
    local dist_wp = H.distance_between(messenger.x, messenger.y,
        waypoint_x[self.data.next_waypoint], waypoint_y[self.data.next_waypoint]
    )
    if (dist_wp <= 3) and (self.data.next_waypoint < #waypoint_x) then
        self.data.next_waypoint = self.data.next_waypoint + 1
    end

    return waypoint_x[self.data.next_waypoint], waypoint_y[self.data.next_waypoint]
end
