-- Make the generic_recruit_engine functions work as external CAs

local dummy_engine = { data = {} }
local ca_castle_switch = wesnoth.require("ai/lua/ca_castle_switch.lua")
local params = {
    score_function = (function() return 300000 end),
    min_turn_1_recruit = (function() return ca_castle_switch:evaluation({}, dummy_engine.data) > 0 end),
    leader_takes_village = (function()
            if ca_castle_switch:evaluation({}, dummy_engine.data) > 0 then
                local take_village = #(wesnoth.get_villages {
                    x = dummy_engine.data.leader_target[1],
                    y = dummy_engine.data.leader_target[2]
                }) > 0
                return take_village
            end
            return true
        end
    )
}
wesnoth.require("ai/lua/generic_recruit_engine.lua").init(dummy_engine, params)

local ca_recruit_rushers = {}

function ca_recruit_rushers:evaluation(cfg, data)
    return dummy_engine:recruit_rushers_eval()
end

function ca_recruit_rushers:execution(cfg, data)
    return dummy_engine:recruit_rushers_exec()
end

return ca_recruit_rushers