local T = wml.tag
local MAIUV = wesnoth.require "ai/micro_ais/micro_ai_unit_variables.lua"

local showed_deprecation_message = {}

local function show_deprecation_message(ai_type)
    if (not showed_deprecation_message[ai_type]) then
        wesnoth.deprecated_message('[micro_ai] tag (' .. ai_type .. '): The old syntax for required and optional parameters when creating a Micro AIs', 3, '1.19', "instead of { 'parameter' } it is now { parameter = 'parameter_type' }. See data/ai/micro_ai/mai-defs/ for examples." )
        showed_deprecation_message[ai_type] = true
    end
end

local function is_number(value, check_int, ai_type, name)
    -- Allow any number for 'float' type.
    -- Allow anything that has an integer representation for 'integer', but
    -- issue warning if it is given in 'float' notation.
    local number = tonumber(value)
    if (not number) then
        return false
    elseif check_int then
        if (number ~= math.floor(number)) then
            return false
        elseif (math.type(number) ~= 'integer') then
            local str = "[micro_ai] tag (" .. ai_type .. ") parameter '" .. name .. "' must be an integer. It has an integer representation, but is provided in floating-point format."
            warn(str)
            std_print(str .. ' (see Lua console for stack trace)')
        end
    end

    return true
end

---@alias mai_parm_type
---| '"tag"'
---| '"float"'
---| '"integer"'
---| '"float_list"'
---| '"integer_list"'
---| '"boolean"'
---| '"string"'

---Validates a single key in a Micro AI.
---@param k_name string The name of the key.
---@param k_value any The value of the key.
---@param k_type mai_parm_type The expected type of the key.
---@param ai_type string The AI being validated.
local function check_key_type(k_name, k_value, k_type, ai_type)
    if k_value then
        local is_wrong_type = false
        if (k_type == 'float') then
            is_wrong_type = not is_number(k_value, false, ai_type, k_name)
        elseif (k_type == 'integer') then
            is_wrong_type = not is_number(k_value, true, ai_type, k_name)
        elseif (k_type == 'float_list') then
            local str = tostring(k_value)
            for _,s in ipairs(str:split()) do
                if (not is_number(s, false, ai_type, k_name)) then
                    is_wrong_type = true
                    break
                end
            end
        elseif (k_type == 'integer_list') then
            local str = tostring(k_value)
            for _,s in ipairs(str:split()) do
                if (not is_number(s, true, ai_type, k_name)) then
                    is_wrong_type = true
                    break
                end
            end
        elseif (k_type == 'boolean') then
            if (type(k_value) ~= k_type) then
                is_wrong_type = true
            end
        -- Anything can be a string
        elseif (k_type ~= 'string') then
            -- This is just here to make sure we're not forgettign something
            wml.error("[micro_ai] tag (" .. ai_type .. ") parameter '" .. k_name .. "' unknown type " .. k_type)
        end

        if is_wrong_type then
            wml.error("[micro_ai] tag (" .. ai_type .. ") parameter '" .. k_name .. "' must be of type " .. k_type)
        end
    end
end

local micro_ai_helper = {}

---@class ca_parm
---@field ca_id string Used for CA id/name
---@field location string The path+file name for the external CA file
---@field score integer The evaluation score

---@class ca_parms : ca_parm[]
---@field ai_id string ID of the AI, potentially used as a prefix for CAs.

---Add the candidate actions defined in CA_parms to the AI of the specified side
---@param side integer The side to modify.
---@param ca_id_core string? ca_id= key from the [micro_ai] tag, or nil
---@param CA_parms ca_parms Specification of the candidate actions to add.
---@param CA_cfg table Table with the parameters passed to the eval/exec functions
function micro_ai_helper.add_CAs(side, ca_id_core, CA_parms, CA_cfg)
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

        for ai_tag in wml.child_range(wesnoth.sides[side].__cfg, 'ai') do
            for stage in wml.child_range(ai_tag, 'stage') do
                for ca in wml.child_range(stage, 'candidate_action') do
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
        for ai_tag in wml.child_range(wesnoth.sides[side].__cfg, 'ai') do
            for engine in wml.child_range(ai_tag, 'engine') do
                for data in wml.child_range(engine, 'data') do
                    for mai in wml.child_range(data, 'micro_ai') do
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

        wesnoth.sides.add_ai_component(side, "stage[main_loop].candidate_action", CA)
    end
end

---Delete the candidate actions defined in CA_parms from the AI of the specified side
---@param side integer The side to modify.
---@param ca_id_core string The CA prefix to use; can be nil.
---@param CA_parms ca_parms Specification of candidate actions to delete.
function micro_ai_helper.delete_CAs(side, ca_id_core, CA_parms)

    -- For CA ids, use value of [micro_ai]ca_id= if given, ai_id otherwise
    ca_id_core = ca_id_core or CA_parms.ai_id

    for _,parms in ipairs(CA_parms) do
        local ca_id = ca_id_core .. '_' .. parms.ca_id

        wesnoth.sides.delete_ai_component(side, "stage[main_loop].candidate_action[" .. ca_id .. "]")

        -- Also need to delete variable stored in all units of the side, so that later MAIs can use these units
        local units = wesnoth.units.find_on_map { side = side }
        for _,unit in ipairs(units) do
            MAIUV.delete_mai_unit_variables(unit, CA_parms.ai_id)
        end
    end
end

---@class aspect_parm
---@field aspect string The aspect name (e.g. 'attacks' or 'aggression')
---@field facet WMLTable A table describing the facet to be added

---Add the aspects defined in aspect_parms to the AI of the specified side.
---@param side integer The side to modify.
---@param aspect_parms aspect_parm[] Specifications of the aspects to add.
function micro_ai_helper.add_aspects(side, aspect_parms)
    -- Examples of facets:
    -- 1. Simple aspect, e.g. aggression
    -- { value = 0.99 }
    --
    -- 2. Composite aspect, e.g. attacks
    --  {   name = "ai_default_rca::aspect_attacks",
    --      id = "dont_attack",
    --      invalidate_on_gamestate_change = "yes",
    --      wml.tag.filter_own {
    --          type = "Dark Sorcerer"
    --      }
    --  }

    for _,parms in ipairs(aspect_parms) do
        wesnoth.sides.add_ai_component(side, "aspect[" .. parms.aspect .. "].facet", parms.facet)
    end
end

---Delete the aspects defined in aspect_parms from the AI of the specified side.
---@param side integer The side to modify.
---@param aspect_parms aspect_parm[] The aspects to delete.
function micro_ai_helper.delete_aspects(side, aspect_parms)
    for _,parms in ipairs(aspect_parms) do
        wesnoth.sides.delete_ai_component(side, "aspect[attacks].facet[" .. parms.facet.id .. "]")
    end
end

---Perform basic setup for a Micro AI.
---@param cfg WML The contents of the [micro_ai] tag.
---@param CA_parms ca_parms Specification of candidate actions to add or remove.
---@param required_keys table<string,mai_parm_type> Specification of required keys.
---@param optional_keys table<string,mai_parm_type> Specification of optional keys.
function micro_ai_helper.micro_ai_setup(cfg, CA_parms, required_keys, optional_keys)
    if (cfg.action == 'delete') then
        micro_ai_helper.delete_CAs(cfg.side, cfg.ca_id, CA_parms)
        return
    end

    -- Otherwise, set up the cfg table to be passed to the CA eval/exec functions
    local CA_cfg = {}

    -- Required keys
    for k_name,k_type in pairs(required_keys) do
        if (type(k_name) == 'number') then
            -- Old syntax is supported for Wesnoth 1.17/1.18
            if k_type:match('%[[a-zA-Z0-9_]+%]')  then
                k_type = k_type:sub(2,-2)
                if not wml.get_child(cfg, k_type) then
                    wml.error("[micro_ai] tag (" .. cfg.ai_type .. ") is missing required parameter: [" .. k_type .. "]")
                end
                for child in wml.child_range(cfg, k_type) do
                    table.insert(CA_cfg, T[k_type](child))
                end
            else
                if not cfg[k_type] then
                    wml.error("[micro_ai] tag (" .. cfg.ai_type .. ") is missing required parameter: " .. k_type .."=")
                end
                CA_cfg[k_type] = cfg[k_type]
            end
            show_deprecation_message(cfg.ai_type)
        elseif (k_type == 'tag') then
            -- Check that this is not a scalar parameter
            if cfg[k_name] then
                wml.error("[micro_ai] tag (" .. cfg.ai_type .. ") parameter '[" .. k_name .. "]' must be a WML tag")
            end
            if (not wml.get_child(cfg, k_name)) then
                wml.error("[micro_ai] tag (" .. cfg.ai_type .. ") is missing required parameter: [" .. k_name .. "]")
            end
            for child in wml.child_range(cfg, k_name) do
                table.insert(CA_cfg, T[k_name](child))
            end
        else
            check_key_type(k_name, cfg[k_name], k_type, cfg.ai_type)
            if not cfg[k_name] then
                wml.error("[micro_ai] tag (" .. cfg.ai_type .. ") is missing required parameter: " .. k_name .."=")
            end
            CA_cfg[k_name] = cfg[k_name]
        end
    end

    -- Optional keys
    for k_name,k_type in pairs(optional_keys) do
        if (type(k_name) == 'number') then
            -- Old syntax is supported for Wesnoth 1.17/1.18
            if k_type:match('%[[a-zA-Z0-9_]+%]')  then
                k_type = k_type:sub(2,-2)
                for child in wml.child_range(cfg, k_type) do
                    table.insert(CA_cfg, T[k_type](child))
                end
            else
                CA_cfg[k_type] = cfg[k_type]
            end
            show_deprecation_message(cfg.ai_type)
        elseif (k_type == 'tag') then
            -- Check that this is not a scalar parameter
            if cfg[k_name] then
                wml.error("[micro_ai] tag (" .. cfg.ai_type .. ") parameter '[" .. k_name .. "]' must be a WML tag")
            end
            for child in wml.child_range(cfg, k_name) do
                table.insert(CA_cfg, T[k_name](child))
            end
        else
            check_key_type(k_name, cfg[k_name], k_type, cfg.ai_type)
            CA_cfg[k_name] = cfg[k_name]
        end
    end

    -- Check whether there are invalid keys in the [micro_ai] tag
    for k in pairs(cfg) do
        if (not showed_deprecation_message[cfg.ai_type]) -- otherwise this produces false positives
           and  (k ~= 'side') and (k ~= 'ai_type') and (k ~= 'action')
           and (k ~= 'ca_id') and (k ~= 'ca_score') and (type(k) ~= 'number')
        then
            local is_invalid = true
            for k_name in pairs(required_keys) do
                if (k == k_name) then
                    is_invalid = false
                    break
                end
            end
            if is_invalid then
                for k_name in pairs(optional_keys) do
                    if (k == k_name) then
                        is_invalid = false
                        break
                    end
                end
            end
            if is_invalid then
                local str = "[micro_ai] tag (" .. cfg.ai_type .. ") contains invalid parameter: " .. k
                warn(str)
                std_print(str .. ' (see Lua console for stack trace)')
            end
        end
    end

    -- Check whether there are invalid tags in the [micro_ai] tag
    for _,t in ipairs(cfg) do
        -- [filter] is always added to [micro_ai] tags inside a unit's [ai] tag,
        -- whether the specific MAI supports it or not
        if (not showed_deprecation_message[cfg.ai_type]) and (t.tag ~= 'filter') then
            local is_invalid = true
            for k_name in pairs(required_keys) do
                if (t.tag == k_name) then
                    is_invalid = false
                    break
                end
            end
            if is_invalid then
                for k_name in pairs(optional_keys) do
                    if (t.tag == k_name) then
                        is_invalid = false
                        break
                    end
                end
            end
            if is_invalid then
                local str = "[micro_ai] tag (" .. cfg.ai_type .. ") contains invalid parameter: [" .. t.tag .. "]"
                warn(str)
                std_print(str .. ' (see Lua console for stack trace)')
            end
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
