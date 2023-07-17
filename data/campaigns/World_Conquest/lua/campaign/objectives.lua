--creates the objectives of the wc2 scenarios.

local _ = wesnoth.textdomain 'wesnoth-wc'
local _lib = wesnoth.textdomain 'wesnoth-lib'
local strings = {
	wct_victory_condition = _"Defeat all enemy leaders and commanders",
	turns = _"Turns run out",
	wct_defeat_condition = _ "Lose your leader and all your commanders",
	difficulty = _lib("Difficulty: "),
	help_available = _ "An in-game help is available: right-click on any empty hex.",
}

function wesnoth.wml_actions.wc2_objectives(cfg)
	wesnoth.wml_actions.objectives {
		wml.tag.objective {
			description = strings.wct_victory_condition,
			condition = "win",
		},
		wml.tag.objective {
			description = strings.turns,
			condition = "lose",
		},
		wml.tag.objective {
			description = strings.wct_defeat_condition,
			condition = "lose",
		},
		wml.tag.note {
			description = strings.difficulty .. wml.variables["wc2_difficulty.name"],
		},
		note = wc2_color.help_text(strings.help_available)
	}
end
