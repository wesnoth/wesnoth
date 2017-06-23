local H = wesnoth.require "helper"

local function handle_default_recruitment(cfg)
	-- Also need to delete/add the default recruitment CA
	if cfg.action == 'add' then
		wesnoth.delete_ai_component(cfg.side, "stage[main_loop].candidate_action[recruitment]")
	elseif cfg.action == 'delete' then
		-- We need to add the recruitment CA back in
		-- This works even if it was not removed, it simply overwrites the existing CA
		wesnoth.add_ai_component(cfg.side, "stage[main_loop].candidate_action",
			{
				id="recruitment",
				engine="cpp",
				name="ai_default_rca::aspect_recruitment_phase",
				max_score=180000,
				score=180000
			}
		)
	end
end

function wesnoth.micro_ais.recruit_rushers(cfg)
	local optional_keys = { "randomness" }
	local CA_parms = {
		ai_id = 'mai_rusher_recruit',
		{ ca_id = "move", location = 'ca_recruit_rushers.lua', score = cfg.ca_score or 180000 }
	}

	handle_default_recruitment(cfg)
    return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.recruit_random(cfg)
	local optional_keys = { "skip_low_gold_recruiting", "[probability]" }
	local CA_parms = {
		ai_id = 'mai_random_recruit',
		{ ca_id = "move", location = 'ca_recruit_random.lua', score = cfg.ca_score or 180000 }
	}

	handle_default_recruitment(cfg)
    return {}, optional_keys, CA_parms
end