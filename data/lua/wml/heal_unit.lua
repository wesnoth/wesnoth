local helper = wesnoth.require "helper"
local T = helper.set_wml_tag_metatable {}

function wesnoth.wml_actions.heal_unit(cfg)
	local healers = helper.get_child(cfg, "filter_second")
	if healers then
		healers = wesnoth.get_units{
			ability_type = "heals",
			T["and"](healers)
		}
	else
		healers = {}
	end

	local who = helper.get_child(cfg, "filter")
	if who then
		who = wesnoth.get_units(who)
	else
		who = wesnoth.get_units{
			x = wesnoth.current.event_context.x1,
			y = wesnoth.current.event_context.y1
		}
	end

	local heal_full = cfg.amount == "full" or cfg.amount == nil
	local moves_full = cfg.moves == "full"
	local heal_amount_set = false
	for i,u in ipairs(who) do
		local heal_amount = u.max_hitpoints - u.hitpoints
		if heal_full then
			u.hitpoints = u.max_hitpoints
		else
			heal_amount = tonumber(cfg.amount) or heal_amount
			local new_hitpoints = math.max(1, math.min(u.max_hitpoints, u.hitpoints + heal_amount))
			heal_amount = new_hitpoints - u.hitpoints
			u.hitpoints = new_hitpoints
		end

		if moves_full then
			u.moves = u.max_moves
		else
			u.moves = math.min(u.max_moves, u.moves + (cfg.moves or 0))
		end

		if cfg.restore_attacks then
			u.attacks_left = u.max_attacks
		end

		if cfg.restore_statuses == true or cfg.restore_statuses == nil then
			u.status.poisoned = false
			u.status.petrified = false
			u.status.slowed = false
			u.status.unhealable = false
		end

		if not heal_amount_set then
			heal_amount_set = true
			wesnoth.set_variable("heal_amount", heal_amount)
		end

		if cfg.animate then
			local animator = wesnoth.create_animator()
			animator:add(u, 'healed', 'hits', {value = heal_amount})
			if #healers > 0 then
				animator:add(healers[1], 'healing', 'hits', {value = heal_amount})
			end
			animator:run()
		end
	end
end
