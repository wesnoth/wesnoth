local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}

local ca_healer_initialize = {}

------ Initialize healer support at beginning of turn -----------
-- Set variables and aspects correctly at the beginning of the turn
-- This will be blacklisted after first execution each turn
function ca_healer_initialize:evaluation(ai)
	local score = 999990
	return score
end

function ca_healer_initialize:execution(ai, cfg, self)
	--print(' Initializing healer_support at beginning of Turn ' .. wesnoth.current.turn)

	-- First, modify the attacks aspect to exclude healers
	-- Always delete the attacks aspect first, so that we do not end up with 100 copies of the facet
	W.modify_ai {
		side = wesnoth.current.side,
		action = "try_delete",
		path = "aspect[attacks].facet[no_healers_attack]"
	}

	-- Then set the aspect to exclude healers
	W.modify_ai {
		side = wesnoth.current.side,
		action = "add",
		path = "aspect[attacks].facet",
		{ "facet", {
			name = "ai_default_rca::aspect_attacks",
			id = "no_healers_attack",
			invalidate_on_gamestate_change = "yes",
			{ "filter_own", {
			   { "not", { ability = "healing", { "and", cfg.filter } } }
			} }
		} }
	}

	-- We also need to set the return score of healer moves to happen _after_ combat at beginning of turn
	self.data.HS_return_score = 95000
end

return ca_healer_initialize
