local internal_recruit_cas = {}
local internal_params = {}

-- The following external engine creates the CA functions recruit_rushers_eval and recruit_rushers_exec
-- It also exposes find_best_recruit and find_best_recruit_hex for use by other recruit engines

wesnoth.require("ai/lua/generic_recruit_engine.lua").init(internal_recruit_cas, internal_params)

local ca_recruit_rushers = {}

function ca_recruit_rushers:evaluation(cfg)
    internal_params.randomness = cfg.randomness
    internal_params.score_function = function() return cfg.ca_score end
    return internal_recruit_cas:recruit_rushers_eval()
end

function ca_recruit_rushers:execution()
    return internal_recruit_cas:recruit_rushers_exec()
end

return ca_recruit_rushers
