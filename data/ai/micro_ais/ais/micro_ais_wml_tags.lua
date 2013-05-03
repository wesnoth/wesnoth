local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require("ai/lua/ai_helper.lua")

function add_CAs(side, CA_parms)
    -- Add the candidate actions defined in 'CA_parms' to the AI of 'side'
    -- CA_parms is an array of tables, one for each CA to be added
    --
    -- Required keys for CA_parms:
    --  - id: is used for both CA id and name
    --  - eval_name: name of the evaluation function
    --  - exec_name: name of the execution function
    --
    -- Optional keys for CA_parms:
    --  - cfg_table: a configuration table (Lua WML table format), to be passed to eval and exec functions
    --      Note: we pass the same string to both functions, even if it contains unnecessary parameters for one or the other
    --  - max_score: maximum score the CA can return

    for i,parms in ipairs(CA_parms) do
        cfg_table = parms.cfg_table or {}

        -- Make sure the id/name of each CA are unique.
        -- We do this by seeing if a CA by that name exists already.
        -- If yes, we use the passed id in parms.id
        -- If not, we add a number to the end of parms.id until we find an id that does not exist yet
        local ca_id, id_found = parms.id, true
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

            if (id_found) then ca_id = parms.id .. n end
            n = n+1
        end

        -- If parameter pass_ca_id is set, pass the CA id to the eval/exec functions
        if parms.pass_ca_id then cfg_table.ca_id = ca_id end

        local CA = {
            engine = "lua",
            id = ca_id,
            name = ca_id,
            max_score = parms.max_score,  -- This works even if parms.max_score is nil
            evaluation = "return (...):" .. parms.eval_name .. "(" .. AH.serialize(cfg_table) .. ")",
            execution = "(...):" .. parms.exec_name .. "(" .. AH.serialize(cfg_table) .. ")"
        }

        if parms.sticky then
            CA.sticky = "yes"
            CA.unit_x = parms.unit_x
            CA.unit_y = parms.unit_y
        end

        W.modify_ai {
            side = side,
            action = "add",
            path = "stage[main_loop].candidate_action",
            { "candidate_action", CA }
        }
    end
end

function delete_CAs(side, CA_parms)
    -- Delete the candidate actions defined in 'CA_parms' from the AI of 'side'
    -- CA_parms is an array of tables, one for each CA to be removed
    -- We can simply pass the one used for add_CAs(), although only the
    -- CA_parms.id field is needed

    for i,parms in ipairs(CA_parms) do
        W.modify_ai {
            side = side,
            action = "try_delete",
            path = "stage[main_loop].candidate_action[" .. parms.id .. "]"
        }
    end
end

function add_aspects(side, aspect_parms)
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

function delete_aspects(side, aspect_parms)
    -- Delete the aspects defined in 'aspect_parms' from the AI of 'side'
    -- aspect_parms is an array of tables, one for each CA to be removed
    -- We can simply pass the one used for add_aspects(), although only the
    -- aspect_parms.id field is needed

    for i,parms in ipairs(aspect_parms) do
        W.modify_ai {
            side = side,
            action = "try_delete",
            path = "aspect[attacks].facet[" .. parms.id .. "]"
        }
    end
end

function CA_action(action, side, CA_parms)
    if (action == 'add') then add_CAs(side, CA_parms) end
    if (action == 'delete') then delete_CAs(side, CA_parms) end
    if (action == 'change') then
        delete_CAs(side, CA_parms)
        add_CAs(side, CA_parms)
    end
end

function wesnoth.wml_actions.micro_ai(cfg)
    -- Set up the [micro_ai] tag functionality for each Micro AI

    -- Check that the required common keys are all present and set correctly
    if (not cfg.ai_type) then H.wml_error("[micro_ai] missing required ai_type= key") end
    if (not cfg.side) then H.wml_error("[micro_ai] missing required side= key") end
    if (not cfg.action) then H.wml_error("[micro_ai] missing required action= key") end

    if (cfg.action ~= 'add') and (cfg.action ~= 'delete') and (cfg.action ~= 'change') then
        H.wml_error("[micro_ai] invalid value for action=.  Allowed values: add, delete or change")
    end

    --------- Healer Support Micro AI - side-wide AI ------------------------------------
    if (cfg.ai_type == 'healer_support') then
        local cfg_hs = {}
        if (cfg.action ~= 'delete') then
            -- Optional keys
            cfg = cfg.__parsed
            cfg_hs.aggression = cfg.aggression or 1.0
            cfg_hs.injured_units_only = cfg.injured_units_only
            cfg_hs.max_threats = cfg.max_threats

            cfg_hs.filter = H.get_child(cfg, "filter")
        end

        -- Set up the CA add/delete parameters
        local CA_parms = {
            {
                id = 'initialize_healer_support', eval_name = 'initialize_healer_support_eval', exec_name = 'initialize_healer_support_exec',
                max_score = 999990, cfg_table = {}
            },
            {
                id = 'healer_support', eval_name = 'healer_support_eval', exec_name = 'healer_support_exec',
                max_score = 105000, cfg_table = cfg_hs
            },
        }

        -- The healers_can_attack CA is only added to the table if aggression ~= 0
        -- But: make sure we always try removal
        if (cfg.action == 'delete') or (tonumber(cfg.aggression) ~= 0) then
            table.insert(CA_parms,
                {
                    id = 'healers_can_attack', eval_name = 'healers_can_attack_eval', exec_name = 'healers_can_attack_exec',
                    max_score = 99990, cfg_table = {}
                }
            )
        end

        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        return
    end

    --------- Bottleneck Defense Micro AI - side-wide AI ------------------------------------
    if (cfg.ai_type == 'bottleneck_defense') then
        local cfg_bd = {}
        if (cfg.action ~= 'delete') then
            -- Required keys
            if (not cfg.x) or (not cfg.y) then
                H.wml_error("Bottleneck Defense Micro AI missing required x= and/or y= key")
            end
            if (not cfg.enemy_x) or (not cfg.enemy_y) then
                H.wml_error("Bottleneck Defense Micro AI missing required enemy_x= and/or enemy_y= key")
            end
            cfg_bd.x, cfg_bd.y = cfg.x, cfg.y
            cfg_bd.enemy_x, cfg_bd.enemy_y = cfg.enemy_x, cfg.enemy_y

            -- Optional keys
            cfg_bd.healer_x = cfg.healer_x
            cfg_bd.healer_y = cfg.healer_y
            cfg_bd.leadership_x = cfg.leadership_x
            cfg_bd.leadership_y = cfg.leadership_y
            cfg_bd.active_side_leader = cfg.active_side_leader
        end

        -- Set up the CA add/delete parameters
        local CA_parms = {
            {
                id = 'bottleneck_move', eval_name = 'bottleneck_move_eval', exec_name = 'bottleneck_move_exec',
                max_score = 300000, cfg_table = cfg_bd
            },
            {
                id = 'bottleneck_attack', eval_name = 'bottleneck_attack_eval', exec_name = 'bottleneck_attack_exec',
                max_score = 290000, cfg_table = {}
            }
        }

        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        return
    end

   --------- Messenger Escort Micro AI - side-wide AI ------------------------------------
    if (cfg.ai_type == 'messenger_escort') then
        local cfg_me = {}
        if (cfg.action ~= 'delete') then
            -- Required keys
            if (not cfg.id) then
                H.wml_error("Messenger Escort Micro AI missing required id= key")
            end
            if (not cfg.waypoint_x) or (not cfg.waypoint_y) then
                H.wml_error("Messenger Escort Micro AI missing required waypoint_x= and/or waypoint_y= key")
            end
            cfg_me.id = cfg.id
            cfg_me.waypoint_x, cfg_me.waypoint_y = cfg.waypoint_x, cfg.waypoint_y

            -- Optional keys
            cfg_me.enemy_death_chance = cfg.enemy_death_chance
            cfg_me.messenger_death_chance = cfg.messenger_death_chance
        end

        local CA_parms = {
            {
                id = 'attack', eval_name = 'attack_eval', exec_name = 'attack_exec',
                max_score = 300000, cfg_table = cfg_me
            },
            {
                id = 'messenger_move', eval_name = 'messenger_move_eval', exec_name = 'messenger_move_exec',
                max_score = 290000, cfg_table = cfg_me
            },
            {
                id = 'other_move', eval_name = 'other_move_eval', exec_name = 'other_move_exec',
                max_score = 280000, cfg_table = cfg_me
            },
        }

        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        return
    end

        --------- Lurkers Micro AI - side-wide AI ------------------------------------
    if (cfg.ai_type == 'lurkers') then
        local cfg_lurk = {}
        if (cfg.action ~= "delete") then
            -- Required keys
            cfg = cfg.__parsed
            local required_keys = {"filter", "filter_location"}
            local optional_keys = {"stationary", "filter_location_wander"}

            for k, v in pairs(required_keys) do
                local child = H.get_child(cfg, v)
                if (not cfg[v]) and (not child) then
                    H.wml_error("Lurker AI missing required " .. v .. "= key")
                end
                cfg_lurk[v] = cfg[v]
                if child then cfg_lurk[v] = child end
            end

            for k, v in pairs(optional_keys) do
                cfg_lurk[v] = cfg[v]
                local child = H.get_child(cfg, v)
                if child then cfg_lurk[v] = child end
            end
        end

        local CA_parms = {
            {
                id = 'lurker_moves_lua', eval_name = 'lurker_attack_eval', exec_name = 'lurker_attack_exec',
                max_score = 100010, cfg_table = cfg_lurk
            },
        }

        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        return
    end

    --------- Protect Unit Micro AI - side-wide AI ------------------------------------
    if (cfg.ai_type == 'protect_unit') then
        local cfg_pu = { id = {}, goal_x = {}, goal_y = {} }
        if (cfg.action ~= 'delete') then
            -- Required keys
            for u in H.child_range(cfg, "unit") do
                if (not u.id) then
                    H.wml_error("Protect Unit Micro AI [unit] tag missing required id= key")
                end
                if (not u.goal_x) then
                    H.wml_error("Protect Unit Micro AI [unit] tag missing required goal_x= key")
                end
                if (not u.goal_y) then
                    H.wml_error("Protect Unit Micro AI [unit] tag missing required goal_y= key")
                end
                table.insert(cfg_pu.id, u.id)
                table.insert(cfg_pu.goal_x, u.goal_x)
                table.insert(cfg_pu.goal_y, u.goal_y)
            end

            if (not cfg_pu.id[1]) then
                H.wml_error("Protect Unit Micro AI missing required [unit] tag")
            end
        end

        local unit_ids_str = 'dummy'
        for i,id in ipairs(cfg_pu.id) do
            unit_ids_str = unit_ids_str .. ',' .. id
        end

        local aspect_parms = {
            {
                aspect = "attacks",
                facet = {
                    name = "ai_default_rca::aspect_attacks",
                    id = "dont_attack",
                    invalidate_on_gamestate_change = "yes",
                    { "filter_own", {
                        { "not", {
                            id = unit_ids_str
                        } }
                    } }
                }
            }
        }

        local CA_parms = {
            {
                id = 'finish', eval_name = 'finish_eval', exec_name = 'finish_exec',
                max_score = 300000, cfg_table = cfg_pu
            },
            {
                id = 'attack', eval_name = 'attack_eval', exec_name = 'attack_exec',
                max_score = 95000, cfg_table = cfg_pu
            },
            {
                id = 'move', eval_name = 'move_eval', exec_name = 'move_exec',
                max_score = 94000, cfg_table = cfg_pu
            }
        }

        add_aspects(cfg.side, aspect_parms)
        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        -- Optional key
        if cfg.disable_move_leader_to_keep then
            W.modify_ai {
                side = side,
                action = "try_delete",
                path = "stage[main_loop].candidate_action[move_leader_to_keep]"
            }
        end

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
        end

        return
    end

    --------- Micro AI Guardian - BCA AIs -----------------------------------
    if (cfg.ai_type == 'guardian_unit') then
        -- We handle these types of guardians here: stationed, return, coward
        if (not cfg.guardian_type) then H.wml_error("[micro_ai] missing required guardian_type= key") end
        local guardian_type = cfg.guardian_type

        -- Since this is a BCA, the unit id needs to be present even for removal
        if (not cfg.id) then H.wml_error("[micro_ai] missing required id= key") end

        -- Set up the cfg array
        local cfg_guardian = { guardian_type = guardian_type }
        local required_keys, optional_keys = {}, {}

        required_keys["stationed_guardian"] = { "id", "distance", "station_x", "station_y", "guard_x", "guard_y" }
        optional_keys["stationed_guardian"] = {}

        required_keys["zone_guardian"] = { "id", "filter_location" }
        optional_keys["zone_guardian"] = { "filter_location_enemy" }

        required_keys["coward"] = { "id", "distance" }
        optional_keys["coward"] = { "seek_x", "seek_y","avoid_x","avoid_y" }

        required_keys["return_guardian"] = { "id", "return_x", "return_y" }
        optional_keys["return_guardian"] = {}

        if (cfg.action~='delete') then
            --Check that we know about this type of guardian
            if (not required_keys[guardian_type]) then
                H.wml_error("[micro_ai] unknown value for guardian_type= key: '" .. guardian_type .."'")
            end

            --Add in the required keys, which could be scalars or WML tag contents
            cfg = cfg.__parsed
            for k,v in pairs(required_keys[guardian_type]) do
                local child = H.get_child(cfg, v)
                if (not cfg[v]) and (not child) then
                    H.wml_error("[micro_ai] ".. guardian_type .." missing required " .. v .. "= key")
                end

                -- Insert scalar parameters
                cfg_guardian[v] = cfg[v]
                -- Insert WML tags
                if child then cfg_guardian[v] = child end
            end

            --Add in the optional keys, which could be scalars or WML tag contents
            for k,v in pairs(optional_keys[guardian_type]) do
                -- Insert scalar parameters
                cfg_guardian[v] = cfg[v]

                -- Insert WML tags
                local child = H.get_child(cfg, v)
                if child then cfg_guardian[v] = child end
            end
        end

        local max_scores = {}
        max_scores["stationed_guardian"] = 100010
        max_scores["zone_guardian"] = 100010
        max_scores["coward"] = 300000

        local unit = wesnoth.get_units { id=cfg.id }[1]

        local CA_parms = {
            {
                id = guardian_type .. '_' .. cfg.id, eval_name = guardian_type .. '_eval', exec_name = guardian_type .. '_exec',
                max_score = max_scores[guardian_type], sticky = true, unit_x = unit.x, unit_y = unit.y, cfg_table = cfg_guardian
            },
        }

        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        return
    end

    --------- Micro AI Animals  - side-wide and BCA AIs ------------------------------------
    if (cfg.ai_type == 'animals') then
        -- We handle these types of animal AIs here:
        --    BCAs: hunter_unit
        --    side-wide AIs: wolves, wolves_multipack, big_animals, forest_animals, swarm, herding
        if (not cfg.animal_type) then H.wml_error("[micro_ai] missing required animal_type= key") end
        local animal_type = cfg.animal_type

        -- For the BCAs, the unit id needs to be present even for removal
        if (animal_type == "hunter_unit") then
            if (not cfg.id) then H.wml_error("[micro_ai] missing required id= key") end
        end

         -- Set up the cfg array
        local cfg_animals = { animal_type = animal_type }
        local required_keys, optional_keys = {}, {}

        -- This list does not contain id because we check for that differently
        required_keys["hunter_unit"] = { "id", "home_x", "home_y" }
        optional_keys["hunter_unit"] = { "filter_location", "rest_turns", "show_messages" }

        required_keys["wolves"] = { "filter", "filter_second" }
        optional_keys["wolves"] = { "avoid_type" }

        required_keys["wolves_multipacks"] = {}
        optional_keys["wolves_multipacks"] = { "type", "pack_size", "show_pack_number" }

        required_keys["big_animals"] = { "filter"}
        optional_keys["big_animals"] = { "avoid_unit", "filter_location", "filter_location_wander" }

        required_keys["forest_animals"] = {}
        optional_keys["forest_animals"] = { "rabbit_type", "rabbit_number", "rabbit_enemy_distance", "rabbit_hole_img",
            "tusker_type", "tusklet_type", "deer_type", "filter_location"
        }

        required_keys["swarm"] = {}
        optional_keys["swarm"] = { "scatter_distance", "vision_distance", "enemy_distance" }

        required_keys["herding"] = { "filter_location", "filter", "filter_second", "herd_x", "herd_y" }
        optional_keys["herding"] = { "attention_distance", "attack_distance" }

        if (cfg.action~='delete') then
            --Check that we know about this type of animal AI
            if (not required_keys[animal_type]) then
                H.wml_error("[micro_ai] unknown value for animal_type= key: '" .. animal_type .."'")
            end

            --Add in the required keys, which could be scalars or WML tag contents
            cfg = cfg.__parsed
            for k,v in pairs(required_keys[animal_type]) do
                local child = H.get_child(cfg, v)
                if (not cfg[v]) and (not child) then
                    H.wml_error("[micro_ai] ".. animal_type .." missing required " .. v .. "= key")
                end

                -- Insert scalar parameters
                cfg_animals[v] = cfg[v]
                -- Insert WML tags
                if child then cfg_animals[v] = child end
            end

            --Add in the optional keys, which could be scalars or WML tag contents
            for k,v in pairs(optional_keys[animal_type]) do
                -- Insert scalar parameters
                cfg_animals[v] = cfg[v]

                -- Insert WML tags
                local child = H.get_child(cfg, v)
                if child then cfg_animals[v] = child end
            end
        end

        local CA_parms = {}
        if (cfg_animals.animal_type == 'big_animals') then
            CA_parms = {
                {
                    id = "big_animal", eval_name = 'big_eval', exec_name = 'big_exec',
                    max_score = 300000, cfg_table = cfg_animals
                }
            }
        end

        if (cfg_animals.animal_type == 'wolves') then
            CA_parms = {
                {
                    id = "wolves", eval_name = 'wolves_eval', exec_name = 'wolves_exec',
                    max_score = 95000, cfg_table = cfg_animals
                },
                {
                    id = "wolves_wander", eval_name = 'wolves_wander_eval', exec_name = 'wolves_wander_exec',
                    max_score = 90000, cfg_table = cfg_animals
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
                                type=cfg_animals.avoid_type
                            } }
                        } }
                    }
                }
            }
            if (cfg.action == "delete") then
                delete_aspects(cfg_animals.side, wolves_aspects)
            else
                add_aspects(cfg_animals.side, wolves_aspects)
            end
        end

        if (cfg_animals.animal_type == 'herding') then
            CA_parms = {
                {
                    id = "close_enemy", eval_name = 'herding_attack_close_enemy_eval', exec_name = 'herding_attack_close_enemy_exec',
                    max_score = 300000, cfg_table = cfg_animals
                },
                {
                    id = "sheep_runs_enemy", eval_name = 'sheep_runs_enemy_eval', exec_name = 'sheep_runs_enemy_exec',
                    max_score = 295000, cfg_table = cfg_animals
                },
                {
                    id = "sheep_runs_dog", eval_name = 'sheep_runs_dog_eval', exec_name = 'sheep_runs_dog_exec',
                    max_score = 290000, cfg_table = cfg_animals
                },
                {
                    id = "herd_sheep", eval_name = 'herd_sheep_eval', exec_name = 'herd_sheep_exec',
                    max_score = 280000, cfg_table = cfg_animals
                },
                {
                    id = "sheep_move", eval_name = 'sheep_move_eval', exec_name = 'sheep_move_exec',
                    max_score = 270000, cfg_table = cfg_animals
                },
                {
                    id = "dog_move", eval_name = 'dog_move_eval', exec_name = 'dog_move_exec',
                    max_score = 260000, cfg_table = cfg_animals
                }
            }
        end

        if (cfg_animals.animal_type == 'forest_animals') then
            CA_parms = {
                {
                    id = "new_rabbit", eval_name = 'new_rabbit_eval', exec_name = 'new_rabbit_exec',
                    max_score = 310000, cfg_table = cfg_animals
                },
                {
                    id = "tusker_attack", eval_name = 'tusker_attack_eval', exec_name = 'tusker_attack_exec',
                    max_score = 300000, cfg_table = cfg_animals
                },
                {
                    id = "move", eval_name = 'forest_animals_move_eval', exec_name = 'forest_animals_move_exec',
                    max_score = 290000, cfg_table = cfg_animals
                },
                {
                    id = "tusklet", eval_name = 'tusklet_eval', exec_name = 'tusklet_exec',
                    max_score = 280000, cfg_table = cfg_animals
                }
            }
        end

        if (cfg_animals.animal_type == 'swarm') then
            CA_parms = {
                {
                    id = "scatter_swarm", eval_name = 'scatter_swarm_eval', exec_name = 'scatter_swarm_exec',
                    max_score = 300000, cfg_table = cfg_animals
                },
                {
                    id = "move_swarm", eval_name = 'move_swarm_eval', exec_name = 'move_swarm_exec',
                    max_score = 290000, cfg_table = cfg_animals
                }
            }
        end

        if (cfg_animals.animal_type == 'wolves_multipacks') then
            CA_parms = {
                {
                    id = "wolves_multipacks_attack", eval_name = 'wolves_multipacks_attack_eval', exec_name = 'wolves_multipacks_attack_exec',
                    max_score = 300000, cfg_table = cfg_animals
                },
                {
                    id = "wolves_multipacks_wander_eval", eval_name = 'wolves_multipacks_wander_eval', exec_name = 'wolves_multipacks_wander_exec',
                    max_score = 290000, cfg_table = cfg_animals
                }
            }
        end

        if (cfg_animals.animal_type == 'hunter_unit') then
            local unit = wesnoth.get_units { id=cfg_animals.id }[1]
            CA_parms = {
                {
                    id = "hunter_unit_" .. cfg_animals.id, eval_name = 'hunter_unit_eval', exec_name = 'hunter_unit_exec',
                    max_score = 300000, sticky = true, unit_x = unit.x, unit_y = unit.y, cfg_table = cfg_animals
                }
            }
        end

        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        return
    end

    --------- Patrol Micro AI - BCA AI ------------------------------------
    if (cfg.ai_type == 'patrol_unit') then
        local cfg_p = {}

        -- Required keys - for both add and delete actions
        if (not cfg.id) then
            H.wml_error("Patrol Micro AI missing required id= key")
        end
        cfg_p.id = cfg.id

        if (cfg.action ~= 'delete') then
            -- Required keys - add action only
            if (not cfg.waypoint_x) or (not cfg.waypoint_y) then
                H.wml_error("Patrol Micro AI missing required waypoint_x/waypoint_y= key")
            end
            cfg_p.waypoint_x = cfg.waypoint_x
            cfg_p.waypoint_y = cfg.waypoint_y

            -- Optional keys
            cfg_p.attack = cfg.attack
            cfg_p.one_time_only = cfg.one_time_only
            cfg_p.out_and_back = cfg.out_and_back
        end

        local unit = wesnoth.get_units { id=cfg_p.id }[1]
        local CA_parms = {
            {
                id = "patrol_unit_" .. cfg_p.id, eval_name = 'patrol_eval', exec_name = 'patrol_exec',
                max_score = 300000, sticky = true, unit_x = unit.x, unit_y = unit.y, cfg_table = cfg_p
            },
        }

        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        return
    end

    --------- Recruiting Micro AI - side-wide AI ------------------------------------
    if (cfg.ai_type == 'recruiting') then
        local cfg_recruiting = {}

        local recruit_CA = { id = "recruit", max_score = 180000 }

        if cfg.recruiting_type then
            if cfg.recruiting_type == "rushers" then
                recruit_CA.eval_name = 'rusher_recruit_eval'
                recruit_CA.exec_name = 'rusher_recruit_exec'

                cfg_recruiting.randomness = cfg.randomness
            elseif cfg.recruiting_type == "random" then
                recruit_CA.eval_name = 'random_recruit_eval'
                recruit_CA.exec_name = 'random_recruit_exec'

                cfg_recruiting.skip_low_gold_recruiting = cfg.skip_low_gold_recruiting

                -- Add the 'probability' tags
                cfg_recruiting.type, cfg_recruiting.probability = {}, {}
                for p in H.child_range(cfg, "probability") do
                    if (not p.type) then
                        H.wml_error("Random Recruiting Micro AI [probability] tag missing required type= key")
                    end
                    if (not p.probability) then
                        H.wml_error("Random Recruiting Micro AI [probability] tag missing required probability= key")
                    end
                    table.insert(cfg_recruiting.type, p.type)
                    table.insert(cfg_recruiting.probability, p.probability)
                end
            else
                H.wml_error("[micro_ai] unknown value for recruiting_type= key")
            end
        elseif cfg.action ~= 'delete' then
            H.wml_error("[micro_ai] missing required recruiting_type= key")
        end

        recruit_CA.cfg_table = cfg_recruiting

        CA_action(cfg.action, cfg.side, {recruit_CA})

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

        return
    end

    --------- Goto Micro AI - side-wide AI ------------------------------------
    if (cfg.ai_type == 'goto') then
        local cfg_go = {}

        local required_keys = { "filter", "filter_location" }
        local optional_keys = { "ca_score", "release_all_units_at_goal", "release_unit_at_goal", "unique_goals", "use_straight_line" }

        if (cfg.action~='delete') then
            --Add in the required keys, which could be scalars or WML tag contents
            cfg = cfg.__parsed
            for k,v in pairs(required_keys) do
                local child = H.get_child(cfg, v)
                if (not cfg[v]) and (not child) then
                    H.wml_error("Goto Micro AI missing required " .. v .. "= key")
                end

                -- Insert scalar parameters
                cfg_go[v] = cfg[v]
                -- Insert WML tags
                if child then cfg_go[v] = child end
            end

            --Add in the optional keys, which could be scalars or WML tag contents
            for k,v in pairs(optional_keys) do
                -- Insert scalar parameters
                cfg_go[v] = cfg[v]

                -- Insert WML tags
                local child = H.get_child(cfg, v)
                if child then cfg_go[v] = child end
            end
        end

        -- Deal with the "ca_id=" key separately, because it doesn't influence the AI behavior
        local ca_id = 'goto'
        if cfg.ca_id then ca_id = ca_id .. '_' .. cfg.ca_id end
        -- Also pass this information to the

        -- Set up the CA add/delete parameters
        local CA_parms = {
            {  -- Note: do not define max_score
                id = ca_id, eval_name = 'goto_eval', exec_name = 'goto_exec',
                cfg_table = cfg_go,
                pass_ca_id = true
            }
        }

        -- Add, delete or change the CAs
        CA_action(cfg.action, cfg.side, CA_parms)

        return
    end

    ----------------------------------------------------------------
    -- If we got here, none of the valid ai_types was specified
    H.wml_error("invalid ai_type= in [micro_ai]")
end
