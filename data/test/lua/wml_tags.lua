--! #textdomain wesnoth-test

local T = wml.tag

---Wrapper for [do_command][attack] that takes unit ids instead of map coordinates,
---as tests are likely to already know the id.
---
---cfg.attacker,cfg.defender are unit ids, and map to [attack]source,destination=
---cfg.weapon,cfg.defender_weapon are optional, and map to [attack]weapon,defender_weapon=
---
---cfg.resupply_attacks_left is an optional int, if set then it will ensure the attacker has
---at least that many attacks_left before calling [do_command][attack].
function wesnoth.wml_actions.test_do_attack_by_id(cfg)
	-- can't be called "source", otherwise wmllint wants to add translation marks
	if not cfg.attacker then
		wml.error("[test_do_attack] missing required attacker= attribute")
	end
	local attacker = wesnoth.units.find_on_map { id = cfg.attacker }[1]
	if not attacker or not attacker.valid then
		wml.error("[test_do_attack] attacker did not match a unit")
	end

	if not cfg.defender then
		wml.error("[test_do_attack] missing required defender= attribute")
	end
	local defender = wesnoth.units.find_on_map { id = cfg.defender }[1]
	if not defender or not defender.valid then
		wml.error("[test_do_attack] defender did not match a unit")
	end

	local weapon = cfg.weapon or ""
	local defender_weapon = cfg.defender_weapon or ""

	-- Avoid needing to modify units to give them multiple attacks per round
	-- This attribute is a number rather than a boolean, to support [attack]attacks_used=
	if cfg.resupply_attacks_left and attacker.attacks_left < cfg.resupply_attacks_left then
		attacker.attacks_left = cfg.resupply_attacks_left
	end

	wesnoth.wml_actions.do_command {
		T.attack {
			T.source { x = attacker.x, y = attacker.y },
			T.destination { x = defender.x, y = defender.y },
			weapon = weapon,
			defender_weapon = defender_weapon,
		}
	}
end
