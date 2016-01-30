-- This set of functions provides a consistent way of storing Micro AI variables
-- in the AI's persistent self.data variable. These need to be stored inside
-- a [micro_ai] tag, so that they are bundled together for a given Micro AI
-- together with an ai_id= key. Their existence can then be checked when setting
-- up another MAI. Otherwise other Micro AIs used in the same scenario might
-- work incorrectly or not at all.
-- Note that, ideally, we would delete these [micro_ai] tags when a Micro AI is
-- deleted, but that that is not always possible as deletion can happen on
-- another side's turn, while the self.data variable is only accessible during
-- the AI turn.
-- Note that, with this method, there can only ever be one of these tags for each
-- Micro AI (but of course several when there are several Micro AIs for the
-- same side).
-- For the time being, we only allow key=value style variables.

local H = wesnoth.require "lua/helper.lua"

local micro_ai_self_data = {}

function micro_ai_self_data.modify_mai_self_data(self_data, ai_id, action, vars_table)
    -- Modify self.data [micro_ai] tags
    -- @ai_id (string): the id of the Micro AI
    -- @action (string): "delete", "set" or "insert"
    -- @vars_table: table of key=value pairs with the variables to be set or inserted
    --   if this is set for @action="delete", then only the keys in @vars_table are deleted,
    --   otherwise the entire [micro_ai] tag is deleted

    -- Always delete the respective [micro_ai] tag, if it exists
    local existing_table
    for i,mai in ipairs(self_data, "micro_ai") do
        if (mai[1] == "micro_ai") and (mai[2].ai_id == ai_id) then
            if (action == "delete") and vars_table then
                for k,_ in pairs(vars_table) do
                    mai[2][k] = nil
                end
                return
            end
            existing_table = mai[2]
            table.remove(self_data, i)
            break
        end
    end

    -- Then replace it, if the "set" action is selected
    -- or add the new keys to it, overwriting old ones with the same name, if action == "insert"
    if (action == "set") or (action == "insert") then
        local tag = { "micro_ai" }

        if (not existing_table) or (action == "set") then
            -- Important: we need to make a copy of the input table, not use it!
            tag[2] = {}
            for k,v in pairs(vars_table) do tag[2][k] = v end
            tag[2].ai_id = ai_id
        else
            for k,v in pairs(vars_table) do existing_table[k] = v end
            tag[2] = existing_table
        end

        table.insert(self_data, tag)
    end
end

function micro_ai_self_data.delete_mai_self_data(self_data, ai_id, vars_table)
   micro_ai_self_data.modify_mai_self_data(self_data, ai_id, "delete", vars_table)
end

function micro_ai_self_data.insert_mai_self_data(self_data, ai_id, vars_table)
   micro_ai_self_data.modify_mai_self_data(self_data, ai_id, "insert", vars_table)
end

function micro_ai_self_data.set_mai_self_data(self_data, ai_id, vars_table)
    micro_ai_self_data.modify_mai_self_data(self_data, ai_id, "set", vars_table)
end

function micro_ai_self_data.get_mai_self_data(self_data, ai_id, key)
    -- Get the content of the self.data [micro_ai] tag for the given @ai_id
    -- Return value:
    --   - If tag is found: value of key if @key parameter is given, otherwise
    --     table of key=value pairs (including the ai_id key)
    --   - If no such tag is found: nil (if @key is set), otherwise empty table

    for mai in H.child_range(self_data, "micro_ai") do
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

return micro_ai_self_data
