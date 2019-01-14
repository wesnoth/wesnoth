-- Make the generic_recruit_engine functions work as external CAs

local ca_castle_switch
for ai_tag in wml.child_range(wesnoth.sides[wesnoth.current.side].__cfg, 'ai') do
    for stage in wml.child_range(ai_tag, 'stage') do
        for ca in wml.child_range(stage, 'candidate_action') do
            if ca.location and string.find(ca.location, 'ca_castle_switch') then
                ca_castle_switch = wesnoth.require("ai/lua/ca_castle_switch.lua")
                break
            end
        end
    end
end

local dummy_engine = { data = {} }

local params = { score_function = (function() return 196000 end) }
if ca_castle_switch then
    params.min_turn_1_recruit = (function() return ca_castle_switch:evaluation({}, dummy_engine.data) > 0 end)
    params.leader_takes_village = (function()
            if ca_castle_switch:evaluation({}, dummy_engine.data) > 0 then
                local take_village = #(wesnoth.get_villages {
                    x = dummy_engine.data.leader_target[1],
                    y = dummy_engine.data.leader_target[2]
                }) > 0
                return take_village
            end
            return not ai.aspects.passive_leader
        end
    )
end

wesnoth.require("ai/lua/generic_recruit_engine.lua").init(dummy_engine, params)

local ca_recruit_rushers = {}

function ca_recruit_rushers:evaluation(cfg, data)
    return dummy_engine:recruit_rushers_eval()
end

function ca_recruit_rushers:execution(cfg, data)
    return dummy_engine:recruit_rushers_exec()
end

return ca_recruit_rushers