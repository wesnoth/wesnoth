-- This set of functions provides a consistent way of storing Micro AI
-- variables in units. Individual variables are stored inside a table with a
-- name specific to the MAI ('micro_ai-' .. ai_id). This table is removed when
-- the Micro AI is deleted in order to ensure that subsequent Micro AIs used
-- in the same scenario (or using the same units in later scenarios) work
-- correctly.
-- Note that, with this method, there can only ever be one of these tables for each
-- ai_id in each unit, but several tables are created for the same unit when there
-- are several Micro AIs with different ai_id values.
-- For the time being, we do not allow sub-tables. This is done because these
-- unit variables are required to be persistent across save-load cycles and
-- therefore need to be in WML table format. This could be extended to allow
-- sub-tables in WML format, but there is no need for that at this time.

function wesnoth.custom_synced_commands.MAI_set_unit_variable(cfg)
	local unit = wesnoth.units.find_on_map { id = cfg.id }[1]
	local value = wml.get_child(cfg, "value")
	unit.variables['micro_ai_' .. cfg.ai_id] = value
end

function MAI_set_unit_variable(unit, ai_id, value)
    -- This is just a wrapper function for the synced command above, so that
    -- we don't need to duplicate the code for creating the cfg WML table.
    local cfg = {
        id = unit.id,
        ai_id = ai_id
    }
    if value then table.insert(cfg, { "value", value }) end
    wesnoth.sync.invoke_command("MAI_set_unit_variable", cfg)
end

local micro_ai_unit_variables = {}

function micro_ai_unit_variables.delete_mai_unit_variables(unit, ai_id)
    MAI_set_unit_variable(unit, ai_id, nil)
end

function micro_ai_unit_variables.insert_mai_unit_variables(unit, ai_id, vars_table)
    local mai_var = unit.variables['micro_ai_' .. ai_id] or {}
    -- Restrict to top-level named fields
    for k,v in pairs(vars_table) do mai_var[k] = v end
    MAI_set_unit_variable(unit, ai_id, mai_var)
end

function micro_ai_unit_variables.set_mai_unit_variables(unit, ai_id, vars_table)
    local mai_var = {}
    -- Restrict to top-level named fields
    for k,v in pairs(vars_table) do mai_var[k] = v end
    MAI_set_unit_variable(unit, ai_id, mai_var)
end

function micro_ai_unit_variables.get_mai_unit_variables(unit, ai_id, key)
    -- Get the content of [unit][variables]['micro_ai_' .. ai_id] tag
    -- Return value:
    --   - If tag is found: value of key if @key parameter is given, otherwise entire table
    --   - If no such tag is found: nil if @key is given, otherwise empty table

    local mai_var = unit.variables['micro_ai_' .. ai_id] or {}

    if key then
        return mai_var[key]
    else
        return mai_var
    end
end

return micro_ai_unit_variables
