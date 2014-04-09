local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local AH = wesnoth.require("ai/lua/ai_helper.lua")
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local micro_ai_helper = {}

function micro_ai_helper.add_CAs(side, CA_parms, CA_cfg)
    -- Add the candidate actions defined in @CA_parms to the AI of @side
    -- @CA_parms is an array of tables, one for each CA to be added (CA setup parameters)
    -- and also contains one key: ai_id
    -- @CA_cfg is a table with the parameters passed to the eval/exec functions
    --
    -- Required keys for each table of @CA_parms:
    --  - ca_id: is used for CA id/name
    --  - location: the path+file name for the external CA file
    --  - score: the evaluation score

    -- We need to make sure that the id/name of each CA are unique.
    -- We do this by checking if CAs starting with ai_id exist already
    -- If yes, we add numbers to the end of ai_id until we find an id that does not exist yet

    local ai_id, id_found = CA_parms.ai_id, true

    local n = 1
    while id_found do -- This is really just a precaution
        id_found = false

        for ai_tag in H.child_range(wesnoth.sides[side].__cfg, 'ai') do
            for stage in H.child_range(ai_tag, 'stage') do
                for ca in H.child_range(stage, 'candidate_action') do
                    if string.find(ca.name, ai_id .. '_') then
                        id_found = true
                        --print('---> found CA:', ca.name, ai_id, id_found, string.find(ca.name, ai_id))
                        break
                    end
                end
            end
        end

        -- Ideally, we would also delete previous occurrences of [micro_ai] tags in the
        -- AI's self.data variable. However, the MAI can be changed while it is not
        -- the AI's turn, when this is not possible. So instead, we check for the
        -- existence of such tags and make sure we are using a different ai_id.
        for ai_tag in H.child_range(wesnoth.sides[side].__cfg, 'ai') do
            for engine in H.child_range(ai_tag, 'engine') do
                for data in H.child_range(engine, 'data') do
                    for mai in H.child_range(data, 'micro_ai') do
                        if (mai.ai_id == ai_id) then
                            id_found = true
                            print('---> found [micro_ai] tag with ai_id =', ai_id)
                            break
                        end
                    end
                end
            end
        end

        if (id_found) then ai_id = CA_parms.ai_id .. n end
        n = n + 1
    end

    -- Now add the CAs
    for i,parms in ipairs(CA_parms) do
        local ca_id = ai_id .. '_' .. parms.ca_id

        -- Always pass the ai_id and ca_score to the eval/exec functions
        CA_cfg.ai_id = ai_id
        CA_cfg.ca_score = parms.score

        local CA = {
            engine = "lua",
            id = ca_id,
            name = ca_id,
            max_score = parms.score
        }

        CA.location = parms.location
        local cfg = string.sub(AH.serialize(CA_cfg), 2, -2) -- need to strip surrounding {}
        CA.eval_parms = cfg
        CA.exec_parms = cfg

        W.modify_ai {
            side = side,
            action = "add",
            path = "stage[main_loop].candidate_action",
            { "candidate_action", CA }
        }
    end
end

function micro_ai_helper.delete_CAs(side, CA_parms)
    -- Delete the candidate actions defined in @CA_parms from the AI of @side
    -- @CA_parms is an array of tables, one for each CA to be removed
    -- We can simply pass the one used for add_CAs(), although only the
    -- CA_parms.ca_id field is needed

    for i,parms in ipairs(CA_parms) do
        local ca_id = CA_parms.ai_id .. '_' .. parms.ca_id

        W.modify_ai {
            side = side,
            action = "try_delete",
            path = "stage[main_loop].candidate_action[" .. ca_id .. "]"
        }

        -- Also need to delete variable stored in all units of the side, so that later MAIs can use these units
        local units = wesnoth.get_units { side = side }
        for i,u in ipairs(units) do
            MAIUV.delete_mai_unit_variables(u, CA_parms.ai_id)
        end
    end
end

function micro_ai_helper.add_aspects(side, aspect_parms)
    -- Add the aspects defined in @aspect_parms to the AI of @side
    -- @aspect_parms is an array of tables, one for each aspect to be added
    --
    -- Required keys for @aspect_parms:
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

function micro_ai_helper.delete_aspects(side, aspect_parms)
    -- Delete the aspects defined in @aspect_parms from the AI of @side
    -- @aspect_parms is an array of tables, one for each aspect to be removed
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

function micro_ai_helper.micro_ai_setup(cfg, CA_parms, required_keys, optional_keys)
    -- If cfg.ca_id is set, it gets used as the ai_id= key
    -- This allows for selective removal of CAs
    -- Note: the ca_id key of the [micro_ai] tag should really be renamed to ai_id,
    -- but that would mean breaking backward compatibility, so we'll just deal with it internally instead

    CA_parms.ai_id = cfg.ca_id or CA_parms.ai_id

    -- If action=delete, we do that and are done
    if (cfg.action == 'delete') then
        micro_ai_helper.delete_CAs(cfg.side, CA_parms)
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
    if (cfg.action == 'add') then micro_ai_helper.add_CAs(cfg.side, CA_parms, CA_cfg) end
    if (cfg.action == 'change') then
        micro_ai_helper.delete_CAs(cfg.side, CA_parms, cfg.id)
        micro_ai_helper.add_CAs(cfg.side, CA_parms, CA_cfg)
    end
end

return micro_ai_helper
