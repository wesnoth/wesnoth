local _ = wesnoth.textdomain "wesnoth"

local function get_is_observer()
	for _, side in ipairs(wesnoth.sides) do
		if side.controller == "human" and side.is_local then
			return false
		end
	end
	return true
end

local function get_num_persistent_teams()
	local res = 0
	for _, side in ipairs(wesnoth.sides) do
		if side.persistent then
			res = res + 1
		end
	end
	return res
end

local function get_num_villages()
	return #(wesnoth.map.find{ gives_income = true })
end

local function half_signed_value(num)
	return tostring(num)
end

local function get_popup_text_basic()

	local is_observer = get_is_observer()
	local is_victory = wesnoth.scenario.end_level_data.is_victory

	local report = ""
	local title = ""

	if is_observer then
		title = _("Scenario Report")
	elseif is_victory then
		title = _("Victory")
		report = report .. "<b>" .. _("You have emerged victorious!") .. "</b>"
	else
		title = _("Defeat")
		report = report .. _("You have been defeated!")
	end
	return title, report
end

local function do_carryover_gold()

	local endlevel_data = wesnoth.scenario.end_level_data
	local has_next_scenario = wesnoth.scenario.next ~= "" and wesnoth.scenario.next ~= "null" and endlevel_data.proceed_to_next_level
	local is_victory = endlevel_data.is_victory
	local is_observer = get_is_observer()
	local is_replay = wesnoth.current.iser_is_replaying
	local show_report = endlevel_data.carryover_report and (not is_observer) and (not is_replay)

	local num_villages = get_num_villages()
	local turns_left = math.max(0, wesnoth.scenario.turns - wesnoth.current.turn)
	local num_persistent_teams = get_num_persistent_teams()

	local title, report = get_popup_text_basic()

	if has_next_scenario then
		for __, side in ipairs(wesnoth.sides) do
			if (not side.persistent) or side.lost then
				goto continue
			end
			local finishing_bonus_per_turn = num_villages * side.village_gold + side.base_income
			local finishing_bonus = side.carryover_bonus * finishing_bonus_per_turn * turns_left

			side.carryover_gold = math.ceil((side.gold + finishing_bonus) * side.carryover_percentage / 100)

			local is_human = side.controller == "human"
			if (not side.is_local) or (not is_human) then
				goto continue
			end

			if num_persistent_teams > 1 then
				report = report .. "\n\n<b>" .. side.side_name .. "</b>"
			end

			report = report .. "<small>\n" .. _("Remaining gold: ") .. half_signed_value(side.gold) .. "</small>"

			if side.carryover_bonus ~= 0 then
				if turns_left > -1 then
					report = report .. "\n\n<b>" ..
						_("Turns finished early: ") .. turns_left .. "</b>\n" .. "<small>" ..
						_("Early finish bonus: ") .. finishing_bonus_per_turn .. _(" per turn") .. "</small>\n" .. "<small>" ..
						_("Total bonus: ") .. finishing_bonus .. "</small>\n"
				end

				report = report .. "<small>" .. _("Total gold: ") .. half_signed_value(side.gold + finishing_bonus) .. "</small>"
			end

			if side.gold > 0 then
				report = report .. "\n<small>" .. _("Carryover percentage: ") .. side.carryover_percentage .. "</small>"
			end

			if side.carryover_add then
				report = report .. "\n\n<big><b>" .. _("Bonus gold: ") .. half_signed_value(side.carryover_gold) .. "</b></big>"
			else
				report = report .. "\n\n<big><b>" .. _("Retained gold: ") .. half_signed_value(side.carryover_gold) .. "</b></big>"
			end

			local goldmsg = ""
			-- Note that both strings are the same in English, but some languages will
			-- want to translate them differently.
			if side.carryover_add then
				if side.carryover_gold > 0 then
					goldmsg = _("You will start the next scenario with $gold on top of the defined minimum starting gold.",
						"You will start the next scenario with $gold on top of the defined minimum starting gold.", side.carryover_gold)

				else
					goldmsg = _("You will start the next scenario with the defined minimum starting gold.",
						"You will start the next scenario with the defined minimum starting gold.", side.carryover_gold)
				end
			else
				goldmsg = _(
					"You will start the next scenario with $gold or its defined minimum starting gold, whichever is higher.",
					"You will start the next scenario with $gold or its defined minimum starting gold, whichever is higher.",
					side.carryover_gold)
			end

			-- xgettext:no-c-format
			report = report .. "\n" .. goldmsg:vformat{ gold = tostring(side.carryover_gold) }

			::continue::
		end
	end

	if show_report then
		gui.show_popup(title, report)
	end
end


wesnoth.game_events.add {
	name = "scenario_end",
	id = "carryover_gold",
	first_time_only = false,
	--priority = -1000,
	action = function()
		do_carryover_gold()
	end
}
