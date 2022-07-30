local on_event = wesnoth.require("on_event")

----- the 'full movement on turn recuited' ability implementation -----
-- priority -1 because this event must be happen after the training event.
on_event("recruit,recall", -1, function(ec)
	local unit = wesnoth.units.get(ec.x1, ec.y1)
	if not unit then
		return
	end
	local matches = unit.variables["mods.wc2_move_on_recruit"]

	if matches then
		unit.attacks_left = 1
		unit.moves = unit.max_moves
	end
end)

----- the 'corruption' ability implementation -----
on_event("turn_refresh", function(event_context)
	wesnoth.wml_actions.harm_unit {
		wml.tag.filter {
			wml.tag.filter_side {
				wml.tag.enemy_of {
					side = wesnoth.current.side,
				},
			},
			wml.tag.filter_adjacent {
				side = wesnoth.current.side,
				ability = "wc2_corruption",
			},
		},
		amount = 6,
		kill = false,
		animate = true,
	}
end)

----- the 'disengage' ability implementation -----
on_event("attack_end", function(cx)
	local u = wesnoth.units.get(cx.x1, cx.y1)
	if not u then
		return
	end
	if not u:matches { wml.tag.has_attack { special_id_active = "wc2_disengage"} } then
		--IMPORTANT: using 'special_active' like this is only guaranteed to work if
		--           the attack has a [filter_self] or a simlar filter tag, otherwise it might
		--           also fire when another attack that is not the currently used attack has
		--           that special
		return
	end
	u.moves = 1
end)
