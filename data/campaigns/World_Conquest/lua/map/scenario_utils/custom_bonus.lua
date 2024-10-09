local on_event = wesnoth.require("on_event")
local custom_bonus = wesnoth.require('bonus_points') -- world_conquest_tek_bonus_points function
local custom_bonus = wesnoth.require('./../postgeneration_utils/events') -- oceanic function
local engine = wesnoth.require('./../postgeneration_utils/engine') -- f map function
local engine = wesnoth.require('./../wct_map_generator.lua') -- f map function
local engine = wesnoth.require('./../scenarios/WC_II_1p_scenario1.lua') -- start map function


on_event("prestart", function(ec)
    print(wml.variables["custom_map_bonus_per_player"])
    if wml.variables["custom_map_bonus_per_player"] > 1 then
        print("custom_map_bonus_per_player is > 1")
        for i = 1, (wml.variables["custom_map_bonus_per_player"] - 1) do
            world_conquest_tek_bonus_points()
        end
    end
end)
