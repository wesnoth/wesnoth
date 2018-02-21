
local _ = wesnoth.textdomain "wesnoth-ai"

wesnoth.deprecated_message('data/ai/lua/patrol.lua', 1, nil, _"Use the Patrols Micro AI instead of patrol.lua.")

function patrol_gen(n, wp)
    -- n is the name of the unit, like Kiressh
    -- wp - a table of waypoint tables of form {x,y}

    local unit = wesnoth.get_units({name=n})[1]

    local x, y = unit.x, unit.y
    local wpn = 1 --WayPoint Number - we have to remember which waypoint we are heading to

    if (x == wp[1].x and y == wp[1].y) then
        wpn = wpn + 1
        --w1, w2 = w2, w2 -- if we are standing on the first waypoint, swap them
    end

    --local waypoints = {w1, w2}      -- this form might be just received from the args
    local wpcount = # wp


    local patrol = {}

    patrol.exec = function()
        x, y = unit.x, unit.y
        if (x == wp[wpn].x and y == wp[wpn].y) then
            wpn = wpn % wpcount + 1 -- advance by one waypoint(this construct loops in range [1, wpcount])
        end
        ai.move_full(unit, wp[wpn].x, wp[wpn].y) -- @note: should we change this to ai.move()?
    end

    patrol.eval = function()
        return 300000
    end

    return patrol
end
