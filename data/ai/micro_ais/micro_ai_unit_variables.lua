-- This set of functions provides a consistent way of storing Micro AI
-- variables in units. They need to be stored inside a [micro_ai] tag in a
-- unit's [variables] tag together with an ai_id= key, so that they can be
-- removed when the Micro AI gets deleted. Otherwise subsequent Micro AIs used
-- in the same scenario (or using the same units in later scenarios) might work
-- incorrectly or not at all.
-- Note that, with this method, there can only ever be one of these tags for each
-- ai_ca in each unit (but of course several when there are several Micro AIs
-- with different ai_CA values affecting the same unit)
-- For the time being, we only allow key=value style variables.

local H = wesnoth.require "helper"

local micro_ai_unit_variables = {}

function micro_ai_unit_variables.modify_mai_unit_variables(unit, ai_id, action, vars_table)
    -- Modify [unit][variables][micro_ai] tags
    -- @ai_id (string): the id of the Micro AI
    -- @action (string): "delete", "set" or "insert"
    -- @vars_table: table of key=value pairs with the variables to be set or inserted (not needed for @action="delete")

    local variables = unit.variables.__cfg

    -- Always delete the respective [variables][micro_ai] tag, if it exists
    local existing_table
    for i,mai in ipairs(variables) do
        if (mai[1] == "micro_ai") and (mai[2].ai_id == ai_id) then
            existing_table = mai[2]
            table.remove(variables, i)
            break
        end
    end

    -- Then replace it, if the "set" action is selected
    -- or add the new keys to it, overwriting old ones with the same name, if action == "insert"
    if (action == "set") or (action == "insert") then
        local tag = { "micro_ai" }

        if (not existing_table) or (action == "set") then
            tag[2] = vars_table
            tag[2].ai_id = ai_id
        else
            for k,v in pairs(vars_table) do existing_table[k] = v end
            tag[2] = existing_table
        end

        table.insert(variables, tag)
    end

    -- All of this so far was only on the table dump -> apply to unit
    unit.variables.__cfg = variables
end

function micro_ai_unit_variables.delete_mai_unit_variables(unit, ai_id)
   micro_ai_unit_variables.modify_mai_unit_variables(unit, ai_id, "delete")
end

function micro_ai_unit_variables.insert_mai_unit_variables(unit, ai_id, vars_table)
   micro_ai_unit_variables.modify_mai_unit_variables(unit, ai_id, "insert", vars_table)
end

function micro_ai_unit_variables.set_mai_unit_variables(unit, ai_id, vars_table)
    micro_ai_unit_variables.modify_mai_unit_variables(unit, ai_id, "set", vars_table)
end

function micro_ai_unit_variables.get_mai_unit_variables(unit, ai_id, key)
    -- Get the content of [unit][variables][micro_ai] tag for the given @ai_id
    -- Return value:
    --   - If tag is found: value of key if @key parameter is given, otherwise
    --     table of key=value pairs (including the ai_id key)
    --   - If no such tag is found: nil (if @key is set), otherwise empty table

    for mai in H.child_range(unit.variables.__cfg, "micro_ai") do
        if (mai.ai_id == ai_id) then
            if key then
                return mai[key]
            else
                return mai
            end
        end
    end

    -- If we got here, no corresponding tag was found
    -- Return empty table; or nil if @key was set
    if (not key) then return {} end
end

return micro_ai_unit_variables
