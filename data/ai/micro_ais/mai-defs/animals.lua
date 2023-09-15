local AH = wesnoth.require "ai/lua/ai_helper.lua"
local MAIH = wesnoth.require("ai/micro_ais/micro_ai_helper.lua")

function wesnoth.micro_ais.big_animals(cfg)
	local required_keys = { filter = 'tag' }
	local optional_keys = { avoid_unit = 'tag', filter_location = 'tag', filter_location_wander = 'tag' }
	local CA_parms = {
		ai_id = 'mai_big_animals',
		{ ca_id = "move", location = 'ca_big_animals.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end

function wesnoth.micro_ais.wolves(cfg)
	local required_keys = { filter = 'tag', filter_second = 'tag' }
	local optional_keys = { attack_only_prey = 'boolean', avoid_type = 'string' }
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
						{ "and", wml.get_child(cfg, "filter_second") }
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
	if (cfg.action ~= 'delete') then
		AH.get_named_loc_xy('herd', cfg, 'Herding [micro_ai] tag')
	end
	local required_keys = { filter_location = 'tag', filter = 'tag', filter_second = 'tag' }
	local optional_keys = { attention_distance = 'integer', attack_distance = 'integer',
		herd_loc = 'string', herd_x = 'integer', herd_y = 'integer'
	}
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

local rabbit_registry_counter = 0;
local save_rabbit_spawn, save_rabbit_despawn

local function register_rabbit_commands()
	function wesnoth.custom_synced_commands.rabbit_despawn(cfg)
		--TODO: maybe we only want to allow erasing of unit of certain types/sides/locations?
		wesnoth.units.erase(cfg.x, cfg.y)
	end

	function wesnoth.custom_synced_commands.rabbit_spawn(cfg)
		--TODO: maybe we only want to allow creation of unit of certain types/sides/locations?
		wesnoth.units.to_map({ side = wesnoth.current.side, type = cfg.rabbit_type}, cfg.x, cfg.y)
	end
end

function wesnoth.persistent_tags.micro_ai_rabbits.read(cfg)
	rabbit_registry_counter = cfg.counter or 0
	register_rabbit_commands()
end

function wesnoth.persistent_tags.micro_ai_rabbits.write(add)
	if rabbit_registry_counter > 0 then
		add{counter = rabbit_registry_counter}
	end
end

function wesnoth.micro_ais.forest_animals(cfg)
	local optional_keys = { rabbit_type = 'string', rabbit_number = 'integer',
		rabbit_enemy_distance = 'integer', rabbit_hole_img = 'string', tusker_type = 'string',
		tusklet_type = 'string', deer_type = 'string', filter_location = 'tag'
	}
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_forest_animals',
		{ ca_id = "new_rabbit", location = 'ca_forest_animals_new_rabbit.lua', score = score },
		{ ca_id = "tusker_attack", location = 'ca_forest_animals_tusker_attack.lua', score = score - 1 },
		{ ca_id = "move", location = 'ca_forest_animals_move.lua', score = score - 2 },
		{ ca_id = "tusklet_move", location = 'ca_forest_animals_tusklet_move.lua', score = score - 3 }
	}

	-- Register custom synced commands for the rabbit AI
	if cfg.action == "delete" then
		rabbit_registry_counter = rabbit_registry_counter - 1
		if rabbit_registry_counter == 0 then
			wesnoth.custom_synced_commands.rabbit_spawn = save_rabbit_spawn
			wesnoth.custom_synced_commands.rabbit_despawn = save_rabbit_despawn
		end
	else
		if rabbit_registry_counter == 0 then
			save_rabbit_spawn = wesnoth.custom_synced_commands.rabbit_spawn
			save_rabbit_despawn = wesnoth.custom_synced_commands.rabbit_despawn
		end

		rabbit_registry_counter = rabbit_registry_counter + 1

		register_rabbit_commands()
	end

	return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.swarm(cfg)
	local optional_keys = { avoid = 'tag', filter = 'tag', scatter_distance = 'integer',
		vision_distance = 'integer', enemy_distance = 'integer'
	}
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_swarm',
		{ ca_id = "scatter", location = 'ca_swarm_scatter.lua', score = score },
		{ ca_id = "move", location = 'ca_swarm_move.lua', score = score - 1 }
	}
	return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.wolves_multipacks(cfg)
	local optional_keys = { avoid = 'tag', type = 'string', pack_size = 'integer', show_pack_number = 'boolean' }
	local score = cfg.ca_score or 300000
	local CA_parms = {
		ai_id = 'mai_wolves_multipacks',
		{ ca_id = "attack", location = 'ca_wolves_multipacks_attack.lua', score = score },
		{ ca_id = "wander", location = 'ca_wolves_multipacks_wander.lua', score = score - 1 }
	}
	return {}, optional_keys, CA_parms
end

function wesnoth.micro_ais.hunter(cfg)
	if (cfg.action ~= 'delete') then
		if (not cfg.id) and (not wml.get_child(cfg, "filter")) then
			wml.error("Hunter [micro_ai] tag requires either id= key or [filter] tag")
		end
		AH.get_named_loc_xy('home', cfg, 'Hunter [micro_ai] tag')
	end
	local required_keys = {}
	local optional_keys = { id = 'string', filter = 'tag', filter_location = 'tag', home_loc = 'string',
		home_x = 'integer', home_y = 'integer', rest_turns = 'integer', show_messages = 'boolean'
	}
	local CA_parms = {
		ai_id = 'mai_hunter',
		{ ca_id = "move", location = 'ca_hunter.lua', score = cfg.ca_score or 300000 }
	}
	return required_keys, optional_keys, CA_parms
end
