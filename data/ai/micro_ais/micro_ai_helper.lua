local H = wesnoth.require "lua/helper.lua"
local W = H.set_wml_action_metatable {}
local T = H.set_wml_tag_metatable {}
local AH = wesnoth.require("ai/lua/ai_helper.lua")
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local micro_ai_helper = {}

function micro_ai_helper.add_CAs(side, ca_id_core, CA_parms, CA_cfg)
    -- Add the candidate actions defined in @CA_parms to the AI of @side
    -- @ca_id_core: ca_id= key from the [micro_ai] tag
    -- @CA_parms: array of tables, one for each CA to be added (CA setup parameters)
    --   Also contains one key: ai_id
    -- @CA_cfg: table with the parameters passed to the eval/exec functions
    --
    -- Required keys for each table of @CA_parms:
    --  - ca_id: is used for CA id/name
    --  - location: the path+file name for the external CA file
    --  - score: the evaluation score

    -- About ai_id, ca_id_core and ca_id:
    -- ai_id: If the AI stores information in the [data] variable, we need to
    --   ensure that it is uniquely attributed to this AI, and not to a separate
    --   AI of the same type. ai_id is used for this and must therefore be unique.
    --   We ensure this by checking if CAs or [data][micro_ai] tags using the
    --   default ai_id value exist already and if so, by adding numbers to the end
    --   until we find an id that is not used yet.
    -- ca_id_core: This is used as base for the id= and name= keys of the
    --   [candidate_action] tags. If [micro_ai]ca_id= is given, we use it as is
    --   without checking if an AI with this id already exists. This is required in
    --   order to ensure that removal with action=delete is possible and it is the
    --   responsibility of the user to ensure uniqueness. If [micro_ai]ca_id= is not
    --   given, use ai_id for ca_id_core, which also makes ids unique for this case.
    -- ca_id: This is specific to the individual CAs of an AI and is added to
    --    ca_id_core for the names and ids of each CA.

    local ai_id, id_found = CA_parms.ai_id, true

    local n = 1
    while id_found do -- This is really just a precaution
        id_found = false

        for ai_tag in H.child_range(wesnoth.sides[side].__cfg, 'ai') do
            for stage in H.child_range(ai_tag, 'stage') do
                for ca in H.child_range(stage, 'candidate_action') do
                    if string.find(ca.name, ai_id .. '_') then
                        id_found = true
                        break
                    end
                end
            end
        end

        -- Ideally, we would also delete previous occurrences of [micro_ai] tags in the
        -- AI's data variable. However, the MAI can be changed while it is not
        -- the AI's turn, when this is not possible. So instead, we check for the
        -- existence of such tags and make sure we are using a different ai_id.
        for ai_tag in H.child_range(wesnoth.sides[side].__cfg, 'ai') do
            for engine in H.child_range(ai_tag, 'engine') do
                for data in H.child_range(engine, 'data') do
                    for mai in H.child_range(data, 'micro_ai') do
                        if (mai.ai_id == ai_id) then
                            id_found = true
                            break
                        end
                    end
                end
            end
        end

        if (id_found) then ai_id = CA_parms.ai_id .. n end
        n = n + 1
    end

    -- For CA ids and names, use value of [micro_ai]ca_id= if given, ai_id otherwise
    ca_id_core = ca_id_core or ai_id

    -- Now add the CAs
    for _,parms in ipairs(CA_parms) do
        local ca_id = ca_id_core .. '_' .. parms.ca_id

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
        table.insert(CA, T.args(CA_cfg))

        W.add_ai_component(side, "stage[main_loop].candidate_action", CA)
    end
end

function micro_ai_helper.delete_CAs(side, ca_id_core, CA_parms)
    -- Delete the candidate actions defined in @CA_parms from the AI of @side
    -- @ca_id_core: ca_id= key from the [micro_ai] tag
    -- @CA_parms: array of tables, one for each CA to be removed
    --   We can simply pass the one used for add_CAs(), although only the
    --   CA_parms.ca_id field is needed

    -- For CA ids, use value of [micro_ai]ca_id= if given, ai_id otherwise
    ca_id_core = ca_id_core or CA_parms.ai_id

    for _,parms in ipairs(CA_parms) do
        local ca_id = ca_id_core .. '_' .. parms.ca_id

        W.delete_ai_component(side, "stage[main_loop].candidate_action[" .. ca_id .. "]")

        -- Also need to delete variable stored in all units of the side, so that later MAIs can use these units
        local units = wesnoth.get_units { side = side }
        for _,unit in ipairs(units) do
            MAIUV.delete_mai_unit_variables(unit, CA_parms.ai_id)
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

    for _,parms in ipairs(aspect_parms) do
        W.add_ai_component(side, "aspect[" .. parms.aspect .. "].facet", parms.facet)
    end
end

function micro_ai_helper.delete_aspects(side, aspect_parms)
    -- Delete the aspects defined in @aspect_parms from the AI of @side
    -- @aspect_parms is an array of tables, one for each aspect to be removed
    -- We can simply pass the one used for add_aspects(), although only the
    -- aspect_parms.aspect_id field is needed

    for _,parms in ipairs(aspect_parms) do
        W.delete_ai_component(side, "aspect[attacks].facet[" .. parms.facet.id .. "]")
    end
end

function micro_ai_helper.micro_ai_setup(cfg, CA_parms, required_keys, optional_keys)
    if (cfg.action == 'delete') then
        micro_ai_helper.delete_CAs(cfg.side, cfg.ca_id, CA_parms)
        return
    end

    -- Otherwise, set up the cfg table to be passed to the CA eval/exec functions
    local CA_cfg = {}

    -- Required keys
    for _,v in pairs(required_keys) do
        if v:match('%[[a-zA-Z0-9_]+%]')  then
            v = v:sub(2,-2)
            if not H.get_child(cfg, v) then
                H.wml_error("[micro_ai] tag (" .. cfg.ai_type .. ") is missing required parameter: [" .. v .. "]")
            end
            for child in H.child_range(cfg, v) do
                table.insert(CA_cfg, T[v](child))
            end
        else
            if not cfg[v] then
                H.wml_error("[micro_ai] tag (" .. cfg.ai_type .. ") is missing required parameter: " .. v .."=")
            end
            CA_cfg[v] = cfg[v]
        end
    end

    -- Optional keys
    for _,v in pairs(optional_keys) do
        if v:match('%[[a-zA-Z0-9_]+%]')  then
            v = v:sub(2,-2)
            for child in H.child_range(cfg, v) do
                table.insert(CA_cfg, T[v](child))
            end
        else
            CA_cfg[v] = cfg[v]
        end
    end

    -- Finally, set up the candidate actions themselves
    if (cfg.action == 'add') then micro_ai_helper.add_CAs(cfg.side, cfg.ca_id, CA_parms, CA_cfg) end
    if (cfg.action == 'change') then
        micro_ai_helper.delete_CAs(cfg.side, cfg.ca_id, CA_parms)
        micro_ai_helper.add_CAs(cfg.side, cfg.ca_id, CA_parms, CA_cfg)
    end
end

return micro_ai_helper
