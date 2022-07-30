
local explanation_template = [[The following conditional test unexpectedly $result:
	$literal
	Interpolated to:
	$interpolated
	]]

local extra_templates = {
	variable = function(cfg, args)
		args.varname = cfg.name
		args.actual = tostring(wml.variables[cfg.name])
		return 'Note: The variable $varname currently has the value $actual.'
	end,
	have_location = function(cfg, args)
		if type(cfg.x) == 'number' and type(cfg.y) == 'number' and cfg.terrain then
			args.x = cfg.x
			args.y = cfg.y
			args.terrain = wesnoth.current.map[{cfg.x, cfg.y}]
			return 'Note: ($x,$y) has terrain $terrain.'
		end
		return ''
	end,
}

local function stringize(tag, cfg, parse)
	cfg = parse and wml.parsed(cfg) or wml.literal(cfg)
	local str = wml.tostring{wml.tag[tag](cfg)}
	return str:gsub('\n', '\n\t')
end

-- This function returns true if it managed to explain the failure
local function explain(current_cfg, expect, logger)
	for i,t in ipairs(current_cfg) do
		local tag, this_cfg = t[1], t[2]
		-- Some special cases
		if tag == "or" or tag == "and" then
			if explain(this_cfg, expect, logger) then
				return true
			end
		elseif tag == "not" then
			if explain(this_cfg, not expect, logger) then
				return true
			end
		elseif tag == "true" or tag == "false" then
			-- We don't explain these ones.
			return true
		elseif wml.eval_conditional{t} == expect then
			local explanation_args = {}
			if expect then
				explanation_args.result = "passed"
			else
				explanation_args.result = "failed"
			end
			explanation_args.literal = stringize(tag, this_cfg, false)
			explanation_args.interpolated = stringize(tag, this_cfg, true)
			local explanation = explanation_template
			if extra_templates[tag] then
				explanation = explanation .. extra_templates[tag](this_cfg, explanation_args)
			end
			explanation = explanation:vformat(explanation_args)
			wesnoth.log(logger, explanation, true)
			return true
		end
	end
end

-- This is mainly for use in unit test macros, but maybe it can be useful elsewhere too
function wesnoth.wml_actions.test_condition(cfg)
	local logger = cfg.logger or "warning"

	-- Use not twice here to convert nil to false
	explain(cfg, not not cfg.result, logger)
end
