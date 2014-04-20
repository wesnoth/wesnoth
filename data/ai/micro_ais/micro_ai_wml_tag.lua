local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local MAIH = wesnoth.require("ai/micro_ais/micro_ai_helper.lua")

function wesnoth.wml_actions.micro_ai(cfg)
    local CA_path = 'ai/micro_ais/cas/'

    cfg = cfg.__parsed

    -- Add translation for old-syntax animal MAIs to new syntax plus deprecation message
    if (cfg.ai_type == 'animals') and (cfg.animal_type) then
        wesnoth.message("The syntax 'ai_type=animals animal_type=" .. cfg.animal_type .. "' is deprecated.  Use 'ai_type=" .. cfg.animal_type .. "' instead.")

        cfg.ai_type = cfg.animal_type
        cfg.animal_type = nil
    end

    -- Add translation for old-syntax guardian MAIs to new syntax plus deprecation message
    if (cfg.ai_type == 'guardian_unit') and (cfg.guardian_type) then
        wesnoth.message("The syntax 'ai_type=guardian_unit guardian_type=" .. cfg.guardian_type .. "' is deprecated.  Use 'ai_type=" .. cfg.guardian_type .. "' instead.")

        cfg.ai_type = cfg.guardian_type
        cfg.guardian_type = nil
    end

    -- Add translation for old-syntax hunter_unit MAI to new syntax plus deprecation message
    if (cfg.ai_type == 'hunter_unit') then
        wesnoth.message("'ai_type=hunter_unit' is deprecated.  Use 'ai_type=hunter' instead.")

        cfg.ai_type = 'hunter'
    end

    -- Add translation for old-syntax patrol_unit MAI to new syntax plus deprecation message
    if (cfg.ai_type == 'patrol_unit') then
        wesnoth.message("'ai_type=patrol_unit' is deprecated.  Use 'ai_type=patrol' instead.")

        cfg.ai_type = 'patrol'
    end

    -- Add translation for old-syntax recruiting MAI to new syntax plus deprecation message
    if (cfg.ai_type == 'recruiting') and (cfg.recruiting_type) then
        local new_type = 'recruit_random'
        if (cfg.recruiting_type == 'rushers') then new_type = 'recruit_rushers' end
        wesnoth.message("The syntax 'ai_type=recruiting recruiting_type=" .. cfg.recruiting_type .. "' is deprecated.  Use 'ai_type=" .. new_type .. "' instead.")

        cfg.ai_type = new_type
        cfg.recruiting_type = nil
    end

    -- Check that the required common keys are all present and set correctly
    if (not cfg.ai_type) then H.wml_error("[micro_ai] is missing required ai_type= key") end
    if (not cfg.side) then H.wml_error("[micro_ai] is missing required side= key") end
    if (not cfg.action) then H.wml_error("[micro_ai] is missing required action= key") end

    if (cfg.action ~= 'add') and (cfg.action ~= 'delete') and (cfg.action ~= 'change') then
        H.wml_error("[micro_ai] unknown value for action=. Allowed values: add, delete or change")
    end

    -- Set up the configuration tables for the different Micro AIs
    local required_keys, optional_keys, CA_parms = {}, {}, {}

    --------- Healer Support Micro AI ------------------------------------
    if (cfg.ai_type == 'healer_support') then
        optional_keys = { "aggression", "injured_units_only", "max_threats", "filter", "filter_second" }
        -- Scores for this AI need to be hard-coded, it does not work otherwise
        CA_parms = {
            ai_id = 'mai_healer',
            { ca_id = 'initialize', location = CA_path .. 'ca_healer_initialize.lua', score = 999990 },
            { ca_id = 'move', location = CA_path .. 'ca_healer_move.lua', score = 105000 },
        }

        -- The healers_can_attack CA is only added to the table if aggression ~= 0
        -- But: make sure we always try removal
        if (cfg.action == 'delete') or (tonumber(cfg.aggression) ~= 0) then
            table.insert(CA_parms, { ca_id = 'may_attack', location = CA_path .. 'ca_healer_may_attack.lua', score = 99990 })
        end

    --------- Bottleneck Defense Micro AI -----------------------------------
    elseif (cfg.ai_type == 'bottleneck_defense') then
        required_keys = { "x", "y", "enemy_x", "enemy_y" }
        optional_keys = { "healer_x", "healer_y", "leadership_x", "leadership_y", "active_side_leader" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            ai_id = 'mai_bottleneck',
            { ca_id = 'move', location = CA_path .. 'ca_bottleneck_move.lua', score = score },
            { ca_id = 'attack', location = CA_path .. 'ca_bottleneck_attack.lua', score = score - 1 }
        }

    --------- Messenger Escort Micro AI ------------------------------------
    elseif (cfg.ai_type == 'messenger_escort') then
        if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
            H.wml_error("Messenger [micro_ai] tag requires either id= key or [filter] tag")
        end
        required_keys = { "waypoint_x", "waypoint_y" }
        optional_keys = { "id", "enemy_death_chance", "filter", "filter_second", "invert_order", "messenger_death_chance" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            ai_id = 'mai_messenger',
            { ca_id = 'attack', location = CA_path .. 'ca_messenger_attack.lua', score = score },
            { ca_id = 'move', location = CA_path .. 'ca_messenger_move.lua', score = score - 1 },
            { ca_id = 'escort_move', location = CA_path .. 'ca_messenger_escort_move.lua', score = score - 2 }
        }

    --------- Lurkers Micro AI ------------------------------------
    elseif (cfg.ai_type == 'lurkers') then
        required_keys = { "filter", "filter_location" }
        optional_keys = { "stationary", "filter_location_wander" }
        CA_parms = {
            ai_id = 'mai_lurkers',
            { ca_id = 'move', location = CA_path .. 'ca_lurkers.lua', score = cfg.ca_score or 300000 }
        }

    --------- Protect Unit Micro AI ------------------------------------
    elseif (cfg.ai_type == 'protect_unit') then
        required_keys = { "id", "goal_x", "goal_y" }
        -- Scores for this AI need to be hard-coded, it does not work otherwise
        CA_parms = {
            ai_id = 'mai_protect_unit',
            { ca_id = 'finish', location = CA_path .. 'ca_protect_unit_finish.lua',  score = 300000 },
            { ca_id = 'attack', location = CA_path .. 'ca_protect_unit_attack.lua', score = 95000 },
            { ca_id = 'move', location = CA_path .. 'ca_protect_unit_move.lua', score = 94999 }
        }

        -- [unit] tags need to be dealt with separately
        cfg.id, cfg.goal_x, cfg.goal_y = {}, {}, {}
        if (cfg.action ~= 'delete') then
            for unit in H.child_range(cfg, "unit") do
                if (not unit.id) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required id= key")
                end
                if (not unit.goal_x) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required goal_x= key")
                end
                if (not unit.goal_y) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required goal_y= key")
                end
                table.insert(cfg.id, unit.id)
                table.insert(cfg.goal_x, unit.goal_x)
                table.insert(cfg.goal_y, unit.goal_y)
            end

            if (not cfg.id[1]) then
                H.wml_error("Protect Unit Micro AI is missing required [unit] tag")
            end
        end

        -- Optional key disable_move_leader_to_keep: needs to be dealt with
        -- separately as it affects a default CA
        if cfg.disable_move_leader_to_keep then
            W.modify_ai {
                side = side,
                action = "try_delete",
                path = "stage[main_loop].candidate_action[move_leader_to_keep]"
            }
        end

        -- attacks aspects also needs to be set separately
        local unit_ids_str = 'dummy'
        for _,id in ipairs(cfg.id) do
            unit_ids_str = unit_ids_str .. ',' .. id
        end
        local aspect_parms = {
            {
                aspect = "attacks",
                facet = {
                    name = "ai_default_rca::aspect_attacks",
                    ca_id = "dont_attack",
                    invalidate_on_gamestate_change = "yes",
                    { "filter_own", {
                        { "not", {
                            id = unit_ids_str
                        } }
                    } }
                }
            }
        }

        if (cfg.action == "delete") then
            MAIH.delete_aspects(cfg.side, aspect_parms)
            -- We also need to add the move_leader_to_keep CA back in
            -- This works even if it was not removed, it simply overwrites the existing CA
            W.modify_ai {
                side = side,
                action = "add",
                path = "stage[main_loop].candidate_action",
                { "candidate_action", {
                    id="move_leader_to_keep",
                    engine="cpp",
                    name="ai_default_rca::move_leader_to_keep_phase",
                    max_score=160000,
                    score=160000
                } }
            }
        else
            MAIH.add_aspects(cfg.side, aspect_parms)
        end

    --------- Micro AI Guardian -----------------------------------
    elseif (cfg.ai_type == 'stationed_guardian') then
        if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
            H.wml_error("Stationed Guardian [micro_ai] tag requires either id= key or [filter] tag")
        end
        required_keys = { "distance", "station_x", "station_y", "guard_x", "guard_y" }
        optional_keys = { "id", "filter" }
        CA_parms = {
            ai_id = 'mai_stationed_guardian',
            { ca_id = 'move', location = CA_path .. 'ca_stationed_guardian.lua', score = cfg.ca_score or 300000 }
        }

    elseif (cfg.ai_type == 'zone_guardian') then
        if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
            H.wml_error("Zone Guardian [micro_ai] tag requires either id= key or [filter] tag")
        end
        required_keys = { "filter_location" }
        optional_keys = { "id", "filter", "filter_location_enemy", "station_x", "station_y" }
        CA_parms = {
            ai_id = 'mai_zone_guardian',
            { ca_id = 'move', location = CA_path .. 'ca_zone_guardian.lua', score = cfg.ca_score or 300000 }
        }

    elseif (cfg.ai_type == 'return_guardian') then
        if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
            H.wml_error("Return Guardian [micro_ai] tag requires either id= key or [filter] tag")
        end
        required_keys = { "return_x", "return_y" }
        optional_keys = { "id", "filter" }
        CA_parms = {
            ai_id = 'mai_return_guardian',
            { ca_id = 'move', location = CA_path .. 'ca_return_guardian.lua', score = cfg.ca_score or 100010 }
        }

    elseif (cfg.ai_type == 'coward') then
        if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
            H.wml_error("Coward [micro_ai] tag requires either id= key or [filter] tag")
        end
        required_keys = { "distance" }
        optional_keys = { "id", "filter", "filter_second", "seek_x", "seek_y","avoid_x","avoid_y" }
        CA_parms = {
            ai_id = 'mai_coward',
            { ca_id = 'move', location = CA_path .. 'ca_coward.lua', score = cfg.ca_score or 300000 }
        }

    --------- Micro AI Animals  ------------------------------------
    elseif (cfg.ai_type == 'big_animals') then
        required_keys = { "filter"}
        optional_keys = { "avoid_unit", "filter_location", "filter_location_wander" }
        CA_parms = {
            ai_id = 'mai_big_animals',
            { ca_id = "move", location = CA_path .. 'ca_big_animals.lua', score = cfg.ca_score or 300000 }
        }

    elseif (cfg.ai_type == 'wolves') then
        required_keys = { "filter", "filter_second" }
        optional_keys = { "attack_only_prey", "avoid_type" }
        local score = cfg.ca_score or 90000
        CA_parms = {
            ai_id = 'mai_wolves',
            { ca_id = "move", location = CA_path .. 'ca_wolves_move.lua', score = score },
            { ca_id = "wander", location = CA_path .. 'ca_wolves_wander.lua', score = score - 1 }
        }

        if cfg.attack_only_prey then
            local wolves_aspects = {
                {
                    aspect = "attacks",
                    facet = {
                        name = "ai_default_rca::aspect_attacks",
                        id = "dont_attack",
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
                        id = "dont_attack",
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

    elseif (cfg.ai_type == 'herding') then
        required_keys = { "filter_location", "filter", "filter_second", "herd_x", "herd_y" }
        optional_keys = { "attention_distance", "attack_distance" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            ai_id = 'mai_herding',
            { ca_id = "attack_close_enemy", location = CA_path .. 'ca_herding_attack_close_enemy.lua', score = score },
            { ca_id = "sheep_runs_enemy", location = CA_path .. 'ca_herding_sheep_runs_enemy.lua', score = score - 1 },
            { ca_id = "sheep_runs_dog", location = CA_path .. 'ca_herding_sheep_runs_dog.lua', score = score - 2 },
            { ca_id = "herd_sheep", location = CA_path .. 'ca_herding_herd_sheep.lua', score = score - 3 },
            { ca_id = "sheep_move", location = CA_path .. 'ca_herding_sheep_move.lua', score = score - 4 },
            { ca_id = "dog_move", location = CA_path .. 'ca_herding_dog_move.lua', score = score - 5 }
        }

    elseif (cfg.ai_type == 'forest_animals') then
        optional_keys = { "rabbit_type", "rabbit_number", "rabbit_enemy_distance", "rabbit_hole_img",
            "tusker_type", "tusklet_type", "deer_type", "filter_location"
        }
        local score = cfg.ca_score or 300000
        CA_parms = {
            ai_id = 'mai_forest_animals',
            { ca_id = "new_rabbit", location = CA_path .. 'ca_forest_animals_new_rabbit.lua', score = score },
            { ca_id = "tusker_attack", location = CA_path .. 'ca_forest_animals_tusker_attack.lua', score = score - 1 },
            { ca_id = "move", location = CA_path .. 'ca_forest_animals_move.lua', score = score - 2 },
            { ca_id = "tusklet_move", location = CA_path .. 'ca_forest_animals_tusklet_move.lua', score = score - 3 }
        }

    elseif (cfg.ai_type == 'swarm') then
        optional_keys = { "scatter_distance", "vision_distance", "enemy_distance" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            ai_id = 'mai_swarm',
            { ca_id = "scatter", location = CA_path .. 'ca_swarm_scatter.lua', score = score },
            { ca_id = "move", location = CA_path .. 'ca_swarm_move.lua', score = score - 1 }
        }

    elseif (cfg.ai_type == 'wolves_multipacks') then
        optional_keys = { "type", "pack_size", "show_pack_number" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            ai_id = 'mai_wolves_multipacks',
            { ca_id = "attack", location = CA_path .. 'ca_wolves_multipacks_attack.lua', score = score },
            { ca_id = "wander", location = CA_path .. 'ca_wolves_multipacks_wander.lua', score = score - 1 }
        }

    elseif (cfg.ai_type == 'hunter') then
        if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
            H.wml_error("Hunter [micro_ai] tag requires either id= key or [filter] tag")
        end
        required_keys = { "home_x", "home_y" }
        optional_keys = { "id", "filter", "filter_location", "rest_turns", "show_messages" }
        CA_parms = {
            ai_id = 'mai_hunter',
            { ca_id = "move", location = CA_path .. 'ca_hunter.lua', score = cfg.ca_score or 300000 }
        }

    --------- Patrol Micro AI ------------------------------------
    elseif (cfg.ai_type == 'patrol') then
        if (cfg.action ~= 'delete') and (not cfg.id) and (not H.get_child(cfg, "filter")) then
            H.wml_error("Patrol [micro_ai] tag requires either id= key or [filter] tag")
        end
        required_keys = { "waypoint_x", "waypoint_y" }
        optional_keys = { "id", "filter", "attack", "one_time_only", "out_and_back" }
        CA_parms = {
            ai_id = 'mai_patrol',
            { ca_id = "move", location = CA_path .. 'ca_patrol.lua', score = cfg.ca_score or 300000 }
        }

    --------- Recruiting Micro AI ------------------------------------
    elseif (cfg.ai_type == 'recruit_rushers') or (cfg.ai_type == 'recruit_random')then
        if (cfg.ai_type == 'recruit_rushers') then
            optional_keys = { "randomness" }
            CA_parms = {
                ai_id = 'mai_rusher_recruit',
                { ca_id = "move", location = CA_path .. 'ca_recruit_rushers.lua', score = cfg.ca_score or 180000 }
            }

        else
            optional_keys = { "skip_low_gold_recruiting", "type", "prob" }
            CA_parms = {
                ai_id = 'mai_random_recruit',
                { ca_id = "move", location = CA_path .. 'ca_recruit_random.lua', score = cfg.ca_score or 180000 }
            }

            if (cfg.action ~= 'delete') then
                -- The 'probability' tags need to be handled separately here
                cfg.type, cfg.prob = {}, {}
                for probability in H.child_range(cfg, "probability") do
                    if (not probability.type) then
                        H.wml_error("Random Recruiting Micro AI [probability] tag is missing required type= key")
                    end
                    if (not probability.probability) then
                        H.wml_error("Random Recruiting Micro AI [probability] tag is missing required probability= key")
                    end
                    table.insert(cfg.type, probability.type)
                    table.insert(cfg.prob, probability.probability)
                end
            end
        end

        -- Also need to delete/add the default recruitment CA
        if cfg.action == 'add' then
            W.modify_ai {
                side = cfg.side,
                action = "try_delete",
                path = "stage[main_loop].candidate_action[recruitment]"
            }
        elseif cfg.action == 'delete' then
            -- We need to add the recruitment CA back in
            -- This works even if it was not removed, it simply overwrites the existing CA
            W.modify_ai {
                side = cfg.side,
                action = "add",
                path = "stage[main_loop].candidate_action",
                { "candidate_action", {
                    id="recruitment",
                    engine="cpp",
                    name="ai_default_rca::aspect_recruitment_phase",
                    max_score=180000,
                    score=180000
                } }
            }
        end

    --------- Goto Micro AI ------------------------------------
    elseif (cfg.ai_type == 'goto') then
        required_keys = { "filter_location" }
        optional_keys = {
            "avoid_enemies", "filter", "ignore_units", "ignore_enemy_at_goal",
            "release_all_units_at_goal", "release_unit_at_goal", "unique_goals", "use_straight_line"
        }
        CA_parms = {
            ai_id = 'mai_goto',
            { ca_id = 'move', location = CA_path .. 'ca_goto.lua', score = cfg.ca_score or 300000 }
        }

    --------- Hang Out Micro AI ------------------------------------
    elseif (cfg.ai_type == 'hang_out') then
        optional_keys = { "filter", "filter_location", "avoid", "mobilize_condition", "mobilize_on_gold_less_than" }
        CA_parms = {
            ai_id = 'mai_hang_out',
            { ca_id = 'move', location = CA_path .. 'ca_hang_out.lua', score = cfg.ca_score or 170000 }
        }

    --------- Simple Attack Micro AI ---------------------------
    elseif (cfg.ai_type == 'simple_attack') then
        optional_keys = { "filter", "filter_second", "weapon" }
        CA_parms = {
            ai_id = 'mai_simple_attack',
            { ca_id = 'move', location = CA_path .. 'ca_simple_attack.lua', score = cfg.ca_score or 110000 }
        }

    -- If we got here, none of the valid ai_types was specified
    else
        H.wml_error("unknown value for ai_type= in [micro_ai]")
    end

    MAIH.micro_ai_setup(cfg, CA_parms, required_keys, optional_keys)
end
