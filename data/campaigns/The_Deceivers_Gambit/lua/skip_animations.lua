---
-- Extends several animation actions so that they do not trigger when the user
-- is skipping messages.
---

local skippable_actions = {
	"animate_unit",
	"sound",
	"delay"
}

local skip_actions = {}

for i, action_id in ipairs(skippable_actions) do

	skip_actions[action_id] = wesnoth.wml_actions[action_id]
	wesnoth.wml_actions[action_id] = function(cfg)
		if wesnoth.interface.is_skipping_messages() then
			return
		end

		skip_actions[action_id](cfg)
	end
end

