local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require("ai/lua/ai_helper.lua")

local function add_CAs(side, CA_parms, CA_cfg)
    -- Add the candidate actions defined in 'CA_parms' to the AI of 'side'
    -- CA_parms is an array of tables, one for each CA to be added (CA setup parameters)
    -- CA_cfg is a table with the parameters passed to the eval/exec functions
    --
    -- Required keys for CA_parms:
    --  - id: is used for both CA id and name
    --  - eval_name: name of the evaluation function
    --  - exec_name: name of the execution function

    for i,parms in ipairs(CA_parms) do
        -- Make sure the id/name of each CA are unique.
        -- We do this by seeing if a CA by that name exists already.
        -- If not, we use the passed id in parms.ca_id
        -- If yes, we add a number to the end of parms.ca_id until we find an id that does not exist yet
        local ca_id, id_found = parms.ca_id, true

        -- If it's a sticky behavior CA, we add the unit id to ca_id
        if parms.sticky then ca_id = ca_id .. "_" .. CA_cfg.id end

        local n = 1
        while id_found do -- This is really just a precaution
            id_found = false

            for ai_tag in H.child_range(wesnoth.sides[side].__cfg, 'ai') do
                for stage in H.child_range(ai_tag, 'stage') do
                    for ca in H.child_range(stage, 'candidate_action') do
                        if (ca.name == ca_id) then id_found = true end
                        --print('---> found CA:', ca.name, id_found)
                    end
                end
            end

            if (id_found) then ca_id = parms.ca_id .. n end
            n = n+1
        end

        -- Also always pass the ca_id and ca_score to the eval/exec functions
        CA_cfg.ca_id = ca_id
        CA_cfg.ca_score = parms.score

        local CA = {
            engine = "lua",
            id = ca_id,
            name = ca_id,
            max_score = parms.score,
            evaluation = "return (...):" .. parms.eval_name .. "(" .. AH.serialize(CA_cfg) .. ")",
            execution = "(...):" .. parms.exec_name .. "(" .. AH.serialize(CA_cfg) .. ")"
        }

        if parms.sticky then
            local unit = wesnoth.get_units { id = CA_cfg.id }[1]
            CA.sticky = "yes"
            CA.unit_x = unit.x
            CA.unit_y = unit.y
        end

        W.modify_ai {
            side = side,
            action = "add",
            path = "stage[main_loop].candidate_action",
            { "candidate_action", CA }
        }
    end
end

local function delete_CAs(side, CA_parms)
    -- Delete the candidate actions defined in 'CA_parms' from the AI of 'side'
    -- CA_parms is an array of tables, one for each CA to be removed
    -- We can simply pass the one used for add_CAs(), although only the
    -- CA_parms.ca_id field is needed

    for i,parms in ipairs(CA_parms) do
        W.modify_ai {
            side = side,
            action = "try_delete",
            path = "stage[main_loop].candidate_action[" .. parms.ca_id .. "]"
        }
    end
end

local function add_aspects(side, aspect_parms)
    -- Add the aspects defined in 'aspect_parms' to the AI of 'side'
    -- aspect_parms is an array of tables, one for each aspect to be added
    --
    -- Required keys for aspect_parms:
    --  - aspect: the aspect name (e.g. 'attacks' or 'aggression')
    --  - facet: A table describing the facet to be added
    --
    -- Examples of facets:
    -- 1. Simple aspect, e.g. aggression
    -- { value = 0.99 }
    --
    -- 2. Composite aspect, e.g. attacks
    --  {   name = "ai_default_rca::aspect_attacks",
    --      id = "dont_attack",
    --      invalidate_on_gamestate_change = "yes",
    --      { "filter_own", {
    --          type = "Dark Sorcerer"
    --      } }
    --  }

    for i,parms in ipairs(aspect_parms) do
        W.modify_ai {
            side = side,
            action = "add",
            path = "aspect[" .. parms.aspect .. "].facet",
            { "facet", parms.facet }
        }
    end
end

local function delete_aspects(side, aspect_parms)
    -- Delete the aspects defined in 'aspect_parms' from the AI of 'side'
    -- aspect_parms is an array of tables, one for each CA to be removed
    -- We can simply pass the one used for add_aspects(), although only the
    -- aspect_parms.aspect_id field is needed

    for i,parms in ipairs(aspect_parms) do
        W.modify_ai {
            side = side,
            action = "try_delete",
            path = "aspect[attacks].facet[" .. parms.aspect_id .. "]"
        }
    end
end

function wesnoth.wml_actions.micro_ai(cfg)
    -- Set up the [micro_ai] tag functionality for each Micro AI

    -- Check that the required common keys are all present and set correctly
    if (not cfg.ai_type) then H.wml_error("[micro_ai] is missing required ai_type= key") end
    if (not cfg.side) then H.wml_error("[micro_ai] is missing required side= key") end
    if (not cfg.action) then H.wml_error("[micro_ai] is missing required action= key") end

    if (cfg.action ~= 'add') and (cfg.action ~= 'delete') and (cfg.action ~= 'change') then
        H.wml_error("[micro_ai] unknown value for action=. Allowed values: add, delete or change")
    end

    -- Set up the configuration tables for the different Micro AIs
    local required_keys, optional_keys, CA_parms = {}, {}, {}
    cfg = cfg.__parsed

    --------- Healer Support Micro AI - side-wide AI ------------------------------------
    if (cfg.ai_type == 'healer_support') then
        optional_keys = { "aggression", "injured_units_only", "max_threats", "filter", "filter_second" }
        CA_parms = {
            {
                ca_id = 'mai_healer_initialize', eval_name = 'mai_healer_initialize_eval', exec_name = 'mai_healer_initialize_exec',
                score = 999990
            },
            {
                ca_id = 'mai_healer_move', eval_name = 'mai_healer_move_eval', exec_name = 'mai_healer_move_exec',
                score = 105000
            },
        }

        -- The healers_can_attack CA is only added to the table if aggression ~= 0
        -- But: make sure we always try removal
        if (cfg.action == 'delete') or (tonumber(cfg.aggression) ~= 0) then
            table.insert(CA_parms,
                {
                    ca_id = 'mai_healer_may_attack', eval_name = 'mai_healer_may_attack_eval', exec_name = 'mai_healer_may_attack_exec',
                    score = 99990
                }
            )
        end

    --------- Bottleneck Defense Micro AI - side-wide AI ------------------------------------
    elseif (cfg.ai_type == 'bottleneck_defense') then
        required_keys = { "x", "y", "enemy_x", "enemy_y" }
        optional_keys = { "healer_x", "healer_y", "leadership_x", "leadership_y", "active_side_leader" }
        CA_parms = {
            {
                ca_id = 'mai_bottleneck_move', eval_name = 'mai_bottleneck_move_eval', exec_name = 'mai_bottleneck_move_exec',
                score = 300000
            },
            {
                ca_id = 'mai_bottleneck_attack', eval_name = 'mai_bottleneck_attack_eval', exec_name = 'mai_bottleneck_attack_exec',
                score = 290000
            }
        }

    --------- Messenger Escort Micro AI - side-wide AI ------------------------------------
    elseif (cfg.ai_type == 'messenger_escort') then
        required_keys = { "id", "waypoint_x", "waypoint_y" }
        optional_keys = { "enemy_death_chance", "messenger_death_chance" }
        local score = cfg.ca_score or 300000
        CA_parms = {
            {
                ca_id = 'mai_messenger_attack', eval_name = 'mai_messenger_attack_eval', exec_name = 'mai_messenger_attack_exec',
                score = score
            },
            {
                ca_id = 'mai_messenger_move', eval_name = 'mai_messenger_move_eval', exec_name = 'mai_messenger_move_exec',
                score = score - 1
            },
            {
                ca_id = 'mai_messenger_other_move', eval_name = 'mai_messenger_other_move_eval', exec_name = 'mai_messenger_other_move_exec',
                score = score - 2
            },
        }

    --------- Lurkers Micro AI - side-wide AI ------------------------------------
    elseif (cfg.ai_type == 'lurkers') then
        required_keys = { "filter", "filter_location" }
        optional_keys = { "stationary", "filter_location_wander" }
        CA_parms = {
            {
                ca_id = 'mai_lurkers_attack', eval_name = 'mai_lurkers_attack_eval', exec_name = 'mai_lurkers_attack_exec',
                score = 100010
            },
        }

    --------- Protect Unit Micro AI - side-wide AI ------------------------------------
    elseif (cfg.ai_type == 'protect_unit') then
        required_keys = { "id", "goal_x", "goal_y" }
        CA_parms = {
            {
                ca_id = 'mai_protect_unit_finish', eval_name = 'mai_protect_unit_finish_eval', exec_name = 'mai_protect_unit_finish_exec',
                score = 300000
            },
            {
                ca_id = 'mai_protect_unit_attack', eval_name = 'mai_protect_unit_attack_eval', exec_name = 'mai_protect_unit_attack_exec',
                score = 95000
            },
            {
                ca_id = 'mai_protect_unit_move', eval_name = 'mai_protect_unit_move_eval', exec_name = 'mai_protect_unit_move_exec',
                score = 94000
            }
        }

        -- [unit] tags need to be dealt with separately
        cfg.id, cfg.goal_x, cfg.goal_y = {}, {}, {}
        if (cfg.action ~= 'delete') then
            for u in H.child_range(cfg, "unit") do
                if (not u.id) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required id= key")
                end
                if (not u.goal_x) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required goal_x= key")
                end
                if (not u.goal_y) then
                    H.wml_error("Protect Unit Micro AI [unit] tag is missing required goal_y= key")
                end
                table.insert(cfg.id, u.id)
                table.insert(cfg.goal_x, u.goal_x)
                table.insert(cfg.goal_y, u.goal_y)
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
        for i,id in ipairs(cfg.id) do
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
            delete_aspects(cfg.side, aspect_parms)
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
            add_aspects(cfg.side, aspect_parms)
        end

    --------- Micro AI Guardian - BCA AIs -----------------------------------
    elseif (cfg.ai_type == 'guardian_unit') then
        -- id= key is required also for CA deletion, for all guardians
        if (not cfg.id) then
            H.wml_error("[micro_ai] tag (guardian_unit) is missing required parameter: id")
        end

        if (cfg.guardian_type == 'stationed_guardian') then
            required_keys = { "id", "distance", "station_x", "station_y", "guard_x", "guard_y" }

            CA_parms = {
                {
                    ca_id = 'mai_guardian_stationed', eval_name = 'mai_guardian_stationed_eval', exec_name = 'mai_guardian_stationed_exec',
                    score = 100010, sticky = true
                }
            }

        elseif (cfg.guardian_type == 'zone_guardian') then
            required_keys = { "id", "filter_location" }
            optional_keys = { "filter_location_enemy", "station_x", "station_y" }

            CA_parms = {
                {
                    ca_id = 'mai_guardian_zone', eval_name = 'mai_guardian_zone_eval', exec_name = 'mai_guardian_zone_exec',
                    score = 100010, sticky = true
                }
            }

        elseif (cfg.guardian_type == 'return_guardian') then
            required_keys = { "id", "return_x", "return_y" }

            CA_parms = {
                {
                    ca_id = 'mai_guardian_return', eval_name = 'mai_guardian_return_eval', exec_name = 'mai_guardian_return_exec',
                    score = 300000, sticky = true
                }
            }

        elseif (cfg.guardian_type == 'coward') then
            required_keys = { "id", "distance" }
            optional_keys = { "seek_x", "seek_y","avoid_x","avoid_y" }

            CA_parms = {
                {
                    ca_id = 'mai_guardian_coward', eval_name = 'mai_guardian_coward_eval', exec_name = 'mai_guardian_coward_exec',
                    score = 300000, sticky = true
                }
            }

        else
            H.wml_error("[micro_ai] tag (guardian) guardian_type= key is missing or has unknown value")
        end

    --------- Micro AI Animals  - side-wide and BCA AIs ------------------------------------
    elseif (cfg.ai_type == 'animals') then
        if (cfg.animal_type == 'big_animals') then
            required_keys = { "filter"}
            optional_keys = { "avoid_unit", "filter_location", "filter_location_wander" }
            CA_parms = {
                {
                    ca_id = "mai_animals_big", eval_name = 'mai_animals_big_eval', exec_name = 'mai_animals_big_exec',
                    score = 300000
                }
            }

        elseif (cfg.animal_type == 'wolves') then
            required_keys = { "filter", "filter_second" }
            optional_keys = { "avoid_type" }
            CA_parms = {
                {
                    ca_id = "mai_animals_wolves", eval_name = 'mai_animals_wolves_eval', exec_name = 'mai_animals_wolves_exec',
                    score = 95000
                },
                {
                    ca_id = "mai_animals_wolves_wander", eval_name = 'mai_animals_wolves_wander_eval', exec_name = 'mai_animals_wolves_wander_exec',
                    score = 90000
                }
            }

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
                delete_aspects(cfg.side, wolves_aspects)
            else
                add_aspects(cfg.side, wolves_aspects)
            end

        elseif (cfg.animal_type == 'herding') then
            required_keys = { "filter_location", "filter", "filter_second", "herd_x", "herd_y" }
            optional_keys = { "attention_distance", "attack_distance" }
            CA_parms = {
                {
                    ca_id = "mai_animals_herding_attack_close_enemy", eval_name = 'mai_animals_herding_attack_close_enemy_eval', exec_name = 'mai_animals_herding_attack_close_enemy_exec',
                    score = 300000
                },
                {
                    ca_id = "mai_animals_sheep_runs_enemy", eval_name = 'mai_animals_sheep_runs_enemy_eval', exec_name = 'mai_animals_sheep_runs_enemy_exec',
                    score = 295000
                },
                {
                    ca_id = "mai_animals_sheep_runs_dog", eval_name = 'mai_animals_sheep_runs_dog_eval', exec_name = 'mai_animals_sheep_runs_dog_exec',
                    score = 290000
                },
                {
                    ca_id = "mai_animals_herd_sheep", eval_name = 'mai_animals_herd_sheep_eval', exec_name = 'mai_animals_herd_sheep_exec',
                    score = 280000
                },
                {
                    ca_id = "mai_animals_sheep_move", eval_name = 'mai_animals_sheep_move_eval', exec_name = 'mai_animals_sheep_move_exec',
                    score = 270000
                },
                {
                    ca_id = "mai_animals_dog_move", eval_name = 'mai_animals_dog_move_eval', exec_name = 'mai_animals_dog_move_exec',
                    score = 260000
                }
            }

        elseif (cfg.animal_type == 'forest_animals') then
            optional_keys = { "rabbit_type", "rabbit_number", "rabbit_enemy_distance", "rabbit_hole_img",
                "tusker_type", "tusklet_type", "deer_type", "filter_location"
            }
            CA_parms = {
                {
                    ca_id = "mai_animals_new_rabbit", eval_name = 'mai_animals_new_rabbit_eval', exec_name = 'mai_animals_new_rabbit_exec',
                    score = 310000
                },
                {
                    ca_id = "mai_animals_tusker_attack", eval_name = 'mai_animals_tusker_attack_eval', exec_name = 'mai_animals_tusker_attack_exec',
                    score = 300000
                },
                {
                    ca_id = "mai_animals_forest_move", eval_name = 'mai_animals_forest_move_eval', exec_name = 'mai_animals_forest_move_exec',
                    score = 290000
                },
                {
                    ca_id = "mai_animals_tusklet", eval_name = 'mai_animals_tusklet_eval', exec_name = 'mai_animals_tusklet_exec',
                    score = 280000
                }
            }

        elseif (cfg.animal_type == 'swarm') then
            optional_keys = { "scatter_distance", "vision_distance", "enemy_distance" }
            CA_parms = {
                {
                    ca_id = "mai_animals_scatter_swarm", eval_name = 'mai_animals_scatter_swarm_eval', exec_name = 'mai_animals_scatter_swarm_exec',
                    score = 300000
                },
                {
                    ca_id = "mai_animals_move_swarm", eval_name = 'mai_animals_move_swarm_eval', exec_name = 'mai_animals_move_swarm_exec',
                    score = 290000
                }
            }

        elseif (cfg.animal_type == 'wolves_multipacks') then
            optional_keys = { "type", "pack_size", "show_pack_number" }
            CA_parms = {
                {
                    ca_id = "mai_animals_wolves_multipacks_attack", eval_name = 'mai_animals_wolves_multipacks_attack_eval', exec_name = 'mai_animals_wolves_multipacks_attack_exec',
                    score = 300000
                },
                {
                    ca_id = "mai_animals_wolves_multipacks_wander", eval_name = 'mai_animals_wolves_multipacks_wander_eval', exec_name = 'mai_animals_wolves_multipacks_wander_exec',
                    score = 290000
                }
            }

        elseif (cfg.animal_type == 'hunter_unit') then
            required_keys = { "id", "home_x", "home_y" }
            optional_keys = { "filter_location", "rest_turns", "show_messages" }

            -- id= key is required also for CA deletion
            if (not cfg.id) then
                H.wml_error("[micro_ai] tag (hunter_unit) is missing required parameter: id")
            end

            CA_parms = {
                {
                    ca_id = "mai_animals_hunter_unit", eval_name = 'mai_animals_hunter_unit_eval', exec_name = 'mai_animals_hunter_unit_exec',
                    score = 300000, sticky = true
                }
            }

        else
            H.wml_error("[micro_ai] tag (animals) animal_type= key is missing or has unknown value")
        end

    --------- Patrol Micro AI - BCA AI ------------------------------------
    elseif (cfg.ai_type == 'patrol_unit') then
        required_keys = { "id", "waypoint_x", "waypoint_y" }
        optional_keys = { "attack", "one_time_only", "out_and_back" }

        -- id= key is required also for CA deletion
        if (not cfg.id) then
            H.wml_error("[micro_ai] tag (patrol_unit) is missing required parameter: id")
        end

        CA_parms = {
            {
                ca_id = "mai_patrol", eval_name = 'mai_patrol_eval', exec_name = 'mai_patrol_exec',
                score = 300000, sticky = true
            },
        }

    --------- Recruiting Micro AI - side-wide AI ------------------------------------
    elseif (cfg.ai_type == 'recruiting') then
        if (cfg.recruiting_type == 'rushers') then
            optional_keys = { "randomness" }
            CA_parms = {
                {
                    ca_id = "mai_rusher_recruit", eval_name = 'mai_rusher_recruit_eval', exec_name = 'mai_rusher_recruit_exec',
                    score = 180000
                }
            }

        elseif (cfg.recruiting_type == 'random') then
            optional_keys = { "skip_low_gold_recruiting", "type", "prob" }
            CA_parms = {
                {
                    ca_id = "mai_random_recruit", eval_name = 'mai_random_recruit_eval', exec_name = 'mai_random_recruit_exec',
                    score = 180000
                }
            }

            -- The 'probability' tags need to be handled separately here
            cfg.type, cfg.prob = {}, {}
            for p in H.child_range(cfg, "probability") do
                if (not p.type) then
                    H.wml_error("Random Recruiting Micro AI [probability] tag is missing required type= key")
                end
                if (not p.probability) then
                    H.wml_error("Random Recruiting Micro AI [probability] tag is missing required probability= key")
                end
                table.insert(cfg.type, p.type)
                table.insert(cfg.prob, p.probability)
            end

        else
            H.wml_error("[micro_ai] tag (recruiting) recruiting_type= key is missing or has unknown value")
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

    --------- Goto Micro AI - side-wide AI ------------------------------------
    elseif (cfg.ai_type == 'goto') then
        required_keys = { "filter", "filter_location" }
        optional_keys = { "release_all_units_at_goal", "release_unit_at_goal", "unique_goals", "use_straight_line" }
        CA_parms = {
            {
                ca_id = 'mai_goto', eval_name = 'mai_goto_eval', exec_name = 'mai_goto_exec',
                score = cfg.ca_score or 300000
            }
        }

    --------- Hang Out Micro AI - side-wide AI ------------------------------------
    elseif (cfg.ai_type == 'hang_out') then
        optional_keys = { "filter", "filter_location", "avoid", "mobilize_condition", "mobilize_on_gold_less_than" }
        CA_parms = {
            {
                ca_id = 'mai_hang_out', eval_name = 'mai_hang_out_eval', exec_name = 'mai_hang_out_exec',
                score = cfg.ca_score or 170000
            }
        }

    -- If we got here, none of the valid ai_types was specified
    else
        H.wml_error("unknown value for ai_type= in [micro_ai]")
    end

    --------- Now go on to setting up the CAs ---------------------------------

    -- If cfg.ca_id is set, it gets added to the id= key of all CAs
    -- This allows for selective removal of CAs
    if cfg.ca_id then
        for i,parms in ipairs(CA_parms) do
            parms.ca_id = parms.ca_id .. '_' .. cfg.ca_id
        end
    end

    -- If action=delete, we do that and are done
    if (cfg.action == 'delete') then
        delete_CAs(cfg.side, CA_parms)
        return
    end

    -- Otherwise, set up the cfg table to be passed to the CA eval/exec functions
    local CA_cfg = {}

    -- Required keys
    for k, v in pairs(required_keys) do
        local child = H.get_child(cfg, v)
        if (not cfg[v]) and (not child) then
            H.wml_error("[micro_ai] tag (" .. cfg.ai_type .. ") is missing required parameter: " .. v)
        end
        CA_cfg[v] = cfg[v]
        if child then CA_cfg[v] = child end
    end

    -- Optional keys
    for k, v in pairs(optional_keys) do
        CA_cfg[v] = cfg[v]
        local child = H.get_child(cfg, v)
        if child then CA_cfg[v] = child end
    end

    -- Finally, set up the candidate actions themselves
    if (cfg.action == 'add') then add_CAs(cfg.side, CA_parms, CA_cfg) end
    if (cfg.action == 'change') then
        delete_CAs(cfg.side, CA_parms)
        add_CAs(cfg.side, CA_parms, CA_cfg)
    end
end
