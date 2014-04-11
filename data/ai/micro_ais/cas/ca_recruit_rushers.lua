local AH = wesnoth.require("ai/lua/ai_helper.lua")

local internal_recruit_cas = {}
local internal_params = {}
-- The following external engine creates the CA functions recruit_rushers_eval and recruit_rushers_exec
-- It also exposes find_best_recruit and find_best_recruit_hex for use by other recruit engines

-- 'ai' is nil here (not defined), so we pass it directly in the execution function below
wesnoth.require("ai/lua/generic_recruit_engine.lua").init(ai, internal_recruit_cas, internal_params)

local ca_recruit_rushers = {}

function ca_recruit_rushers:evaluation(ai, cfg)
    internal_params.randomness = cfg.randomness
    return internal_recruit_cas:recruit_rushers_eval()
end

function ca_recruit_rushers:execution(ai)
    return internal_recruit_cas:recruit_rushers_exec(ai)
end

return ca_recruit_rushers
