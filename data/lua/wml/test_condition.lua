local helper = wesnoth.require "helper"

-- This function returns true if it managed to explain the failure
local function explain(current_cfg, expect)
	for i,t in ipairs(current_cfg) do
		local tag, this_cfg = t[1], t[2]
		-- Some special cases
		if tag == "or" or tag == "and" then
			if explain(this_cfg, expect) then
				return true
			end
		elseif tag == "not" then
			if explain(this_cfg, not expect) then
				return true
			end
		elseif tag == "true" or tag == "false" then
			-- We don't explain these ones.
			return true
		elseif wesnoth.eval_conditional{t} == expect then
			local explanation = "The following conditional test %s:"
			if expect then
				explanation = explanation:format("passed")
			else
				explanation = explanation:format("failed")
			end
			explanation = string.format("%s\n\t[%s]", explanation, tag)
			for k,v in pairs(this_cfg) do
				if type(k) ~= "number" then
					local format = "%s\n\t\t%s=%s"
					local literal = tostring(helper.literal(this_cfg)[k])
					if literal ~= v then
						format = format .. "=%s"
					end
					explanation = string.format(format, explanation, k, literal, tostring(v))
				end
			end
			explanation = string.format("%s\n\t[/%s]", explanation, tag)
			if tag == "variable" then
				explanation = string.format("%s\n\tNote: The variable %s currently has the value %q.", explanation, this_cfg.name, tostring(wesnoth.get_variable(this_cfg.name)))
			end
			wesnoth.log(logger, explanation, true)
			return true
		end
	end
end

-- This is mainly for use in unit test macros, but maybe it can be useful elsewhere too
function wesnoth.wml_actions.test_condition(cfg)
	local logger = cfg.logger or "warning"

	-- Use not twice here to convert nil to false
	explain(cfg, not not cfg.result)
end
