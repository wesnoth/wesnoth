local H = wesnoth.require "helper"
local MAIH = wesnoth.require("ai/micro_ais/micro_ai_helper.lua")

function wesnoth.micro_ais.big_animals(cfg)
    local required_keys = { "[filter]"}
    local optional_keys = { "[avoid_unit]", "[filter_location]", "[filter_location_wander]" }
    local CA_parms = {
        ai_id = 'mai_big_animals',
        { ca_id = "move", location = 'ca_big_animals.lua', score = cfg.ca_score or 300000 }
    }
    return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.wolves(cfg)
	local required_keys = { "[filter]", "[filter_second]" }
	local optional_keys = { "attack_only_prey", "avoid_type" }
	local score = cfg.ca_score or 90000
	local CA_parms = {
		ai_id = 'mai_wolves',
		{ ca_id = "move", location = 'ca_wolves_move.lua', score = score },
		{ ca_id = "wander", location = 'ca_wolves_wander.lua', score = score - 1 }
	}

	if cfg.attack_only_prey then
		local wolves_aspects = {
			{
				aspect = "attacks",
				facet = {
					name = "ai_default_rca::aspect_attacks",
					id = "mai_wolves_" .. (cfg.ca_id or "default") .. "_dont_attack",
					invalidate_on_gamestate_change = "yes",
					{ "filter_enemy", {
						{ "and", H.get_child(cfg, "filter_second") }
					} }
				}
			}
		}
		if (cfg.action == "delete") then
			MAIH.delete_aspects(cfg.side, wolves_aspects)
		else
			MAIH.add_aspects(cfg.side, wolves_aspects)
		end
	elseif cfg.avoid_type then
		local wolves_aspects = {
			{
				aspect = "attacks",
				facet = {
					name = "ai_default_rca::aspect_attacks",
					id = "mai_wolves_" .. (cfg.ca_id or "default") .. "_dont_attack",
					invalidate_on_gamestate_change = "yes",
					{ "filter_enemy", {
						{ "not", {
							type=cfg.avoid_type
						} }
					} }
				}
			}
		}
		if (cfg.action == "delete") then
			MAIH.delete_aspects(cfg.side, wolves_aspects)
		else
			MAIH.add_aspects(cfg.side, wolves_aspects)
		end
	end
    return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.herding(cfg)
	local required_keys = { "[filter_location]", "[filter]", "[filter_second]", "herd_x", "herd_y" }
	local optional_keys = { "attention_distance", "attack_distance" }
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_herding',
		{ ca_id = "attack_close_enemy", location = 'ca_herding_attack_close_enemy.lua', score = score },
		{ ca_id = "sheep_runs_enemy", location = 'ca_herding_sheep_runs_enemy.lua', score = score - 1 },
		{ ca_id = "sheep_runs_dog", location = 'ca_herding_sheep_runs_dog.lua', score = score - 2 },
		{ ca_id = "herd_sheep", location = 'ca_herding_herd_sheep.lua', score = score - 3 },
		{ ca_id = "sheep_move", location = 'ca_herding_sheep_move.lua', score = score - 4 },
		{ ca_id = "dog_move", location = 'ca_herding_dog_move.lua', score = score - 5 },
		{ ca_id = "dog_stopmove", location = 'ca_herding_dog_stopmove.lua', score = score - 6 }
	}
    return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.forest_animals(cfg)
	local optional_keys = { "rabbit_type", "rabbit_number", "rabbit_enemy_distance", "rabbit_hole_img",
		"tusker_type", "tusklet_type", "deer_type", "[filter_location]"
	}
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_forest_animals',
		{ ca_id = "new_rabbit", location = 'ca_forest_animals_new_rabbit.lua', score = score },
		{ ca_id = "tusker_attack", location = 'ca_forest_animals_tusker_attack.lua', score = score - 1 },
		{ ca_id = "move", location = 'ca_forest_animals_move.lua', score = score - 2 },
		{ ca_id = "tusklet_move", location = 'ca_forest_animals_tusklet_move.lua', score = score - 3 }
	}
    return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.swarm(cfg)
	local optional_keys = { "scatter_distance", "vision_distance", "enemy_distance" }
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_swarm',
		{ ca_id = "scatter", location = 'ca_swarm_scatter.lua', score = score },
		{ ca_id = "move", location = 'ca_swarm_move.lua', score = score - 1 }
	}
    return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.wolves_multipacks(cfg)
	local optional_keys = { "type", "pack_size", "show_pack_number" }
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_wolves_multipacks',
		{ ca_id = "attack", location = 'ca_wolves_multipacks_attack.lua', score = score },
		{ ca_id = "wander", location = 'ca_wolves_multipacks_wander.lua', score = score - 1 }
	}
    return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.hunter(cfg)
	if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
		H.wml_error("Hunter [micro_ai] tag requires either id= key or [filter] tag")
	end
	local required_keys = { "home_x", "home_y" }
	local optional_keys = { "id", "[filter]", "[filter_location]", "rest_turns", "show_messages" }
	local CA_parms = {
		ai_id = 'mai_hunter',
		{ ca_id = "move", location = 'ca_hunter.lua', score = cfg.ca_score or 300000 }
	}
    return required_keys, optional_keys, CA_parms
end
