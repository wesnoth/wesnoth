local _ = wesnoth.textdomain "wesnoth"
local carryover = {}

--- Returns true iff there is not a single side that is controlled by the local human player.
---@return boolean
function carryover.get_is_observer()
	for _, side in ipairs(wesnoth.sides) do
		if side.controller == "human" and side.is_local then
			return false
		end
	end
	return true
end

--- Returns the numbner of sides which carry over to a next scenario.
---@return integer
function carryover.get_num_persistent_teams()
	local res = 0
	for _, side in ipairs(wesnoth.sides) do
		if side.persistent then
			res = res + 1
		end
	end
	return res
end

--- Gets the number of villages on the map.
---@return integer
function carryover.get_num_villages()
	return #(wesnoth.map.find{ gives_income = true })
end

---@return boolean #whether there is another scenario after this one ends.
function carryover.has_next_scenario()
	return wesnoth.scenario.next ~= "" and wesnoth.scenario.next ~= "null" and wesnoth.scenario.end_level_data.proceed_to_next_level
end

---@return number #number fo turns left in the scenario
function carryover.turns_left()
	return math.max(0, wesnoth.scenario.turns - wesnoth.current.turn)
end

--- Like tostring(num) but uses a prettier minus sign, to be used in the UI.
---@return string
function carryover.half_signed_value(num)
	if num < 0 then
		return "-" .. tostring(math.abs(num))
	else
		return tostring(num)
	end
end

--- Calculates the amount of carryover gold.
---@param side side
---@param turns_left integer
---@return number #total finishing bonus
---@return number #finishing bonus per turn
function carryover.calculate_finishing_bonus(side, turns_left)
	local num_villages = carryover.get_num_villages()

	local finishing_bonus_per_turn = num_villages * side.village_gold + side.base_income
	local finishing_bonus = math.ceil(side.carryover_bonus * finishing_bonus_per_turn * turns_left)
	return finishing_bonus, finishing_bonus_per_turn
end

---@return boolean #whether we should show the carryover report
function carryover.get_report_visible()
	local is_observer = carryover.get_is_observer()
	local is_replay = wesnoth.current.user_is_replaying
	return wesnoth.scenario.end_level_data.carryover_report and (not is_observer) and (not is_replay)
end

--- Whether to show the sides' names in the scenario end report.
---@return boolean
function carryover.get_show_side_names()
	local num_persistent_teams = carryover.get_num_persistent_teams()
	return num_persistent_teams > 1
end

---@class side_carryover_info
---@field turns_left? number number of turns left
---@field finishing_bonus number early finishing bonus gold
---@field finishing_bonus_per_turn number early finishing bonus gold per turn

--- sets the sides carryover gold and returns information that is used in the report message
---@param side side
---@return side_carryover_info
function carryover.set_side_carryover_gold(side)
	local turns_left = carryover.turns_left()
	local finishing_bonus, finishing_bonus_per_turn = carryover.calculate_finishing_bonus(side, turns_left)

	side.carryover_gold = math.ceil((side.gold + finishing_bonus) * side.carryover_percentage / 100)

	return {
		turns_left = turns_left,
		finishing_bonus = finishing_bonus,
		finishing_bonus_per_turn = finishing_bonus_per_turn,
	}
end

--- returns a stub to show in the scenario end dialog.
---@return string #title
---@return string #report
function carryover.get_popup_text_basic()
	local is_observer = carryover.get_is_observer()
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

---@param side side
---@param info side_carryover_info
---@return string #A string to be used in the carryover dialog, describing the remainling gold
function carryover.remaining_gold_message(side, info)
	return "<small>\n" .. _("Remaining gold: ") .. carryover.half_signed_value(side.gold) .. "</small>"
end

---@param side side
---@param info side_carryover_info
---@return string #A string to be used in the carryover dialog, describing the bonus gold
function carryover.bonus_gold_message(side, info)
	local res = ""
	if info.turns_left ~= nil then
		res = res .. "<b>" .. _("Turns finished early: ") .. info.turns_left .. "</b>\n"
	end
	if info.finishing_bonus_per_turn ~= nil then
		res = res .. "<small>" .. _("Early finish bonus: ") .. info.finishing_bonus_per_turn .. _(" per turn") .. "</small>\n"
	end
	res = res .. "<small>" .. _("Total bonus: ") .. info.finishing_bonus .. "</small>"

	return res
end

---@param side side
---@param info side_carryover_info
---@return string #A string to be used in the carryover dialog, describing the total gold
function carryover.total_gold_message(side, info)
	return "<small>" .. _("Total gold: ") .. carryover.half_signed_value(side.gold + info.finishing_bonus) .. "</small>"
end

---@param side side
---@param info side_carryover_info
---@return string #A string to be used in the carryover dialog, describing the percentage of gold carried over
function carryover.percentage_remaining_message(side, info)
	return "<small>" .. _("Carryover percentage: ") .. side.carryover_percentage .. "</small>"
end
--- returns the last part of the gold report for a single side, describing how much gold is added in the next scenario.
---@param side side
---@param info side_carryover_info
---@return string
function carryover.carryover_mesage(side, info)
	local goldmsg = ""
	if side.carryover_add then
		goldmsg = goldmsg .. "<big><b>" .. _("Bonus gold: ") .. carryover.half_signed_value(side.carryover_gold) .. "</b></big>"
	else
		goldmsg = goldmsg .. "<big><b>" .. _("Retained gold: ") .. carryover.half_signed_value(side.carryover_gold) .. "</b></big>"
	end
	goldmsg = goldmsg  .. "\n"
	-- Note that both strings are the same in English, but some languages will
	-- want to translate them differently.
	if side.carryover_add then
		if side.carryover_gold > 0 then
			goldmsg = goldmsg .. _("You will start the next scenario with $gold on top of the defined minimum starting gold.",
				"You will start the next scenario with $gold on top of the defined minimum starting gold.", side.carryover_gold)

		else
			goldmsg = goldmsg .. _("You will start the next scenario with the defined minimum starting gold.")
		end
	else
		goldmsg = goldmsg .. _(
			"You will start the next scenario with $gold or its defined minimum starting gold, whichever is higher.",
			"You will start the next scenario with $gold or its defined minimum starting gold, whichever is higher.",
			side.carryover_gold)
	end

	return goldmsg:vformat{ gold = tostring(side.carryover_gold) }
end

--- returns the default gold report for a single side.
---@param side side
---@param info side_carryover_info
---@return string
function carryover.get_side_gold_report(side, info)

	local report = ""

	report = report .. carryover.remaining_gold_message(side, info)

	if side.carryover_bonus ~= 0 then
		report = report .. "\n\n" .. carryover.bonus_gold_message(side, info) .. "\n" .. carryover.total_gold_message(side, info)
	end

	if side.carryover_gold > 0 then
		report = report .. "\n" .. carryover.percentage_remaining_message(side, info)
	end


	report = report .. "\n\n" .. carryover.carryover_mesage(side, info)
	return report
end


--- the main function that set every sides carryover gold amount and shows the correct carryover report dialog.
function carryover.do_carryover_gold()

	local has_next_scenario = carryover.has_next_scenario()
	local show_report = carryover.get_report_visible()

	local show_side_names = carryover.get_show_side_names()

	local title, report = carryover.get_popup_text_basic()

	if has_next_scenario then
		for __, side in ipairs(wesnoth.sides) do
			if (not side.persistent) or side.lost then
				goto continue
			end

			local data = carryover.set_side_carryover_gold(side)

			local is_human = side.controller == "human"
			if (not side.is_local) or (not is_human) then
				goto continue
			end

			if show_side_names then
				report = report .. "\n\n<b>" .. side.side_name .. "</b>"
			end

			report = report .. carryover.get_side_gold_report(side, data)

			::continue::
		end
	end

	if show_report then
		gui.show_popup(title, report)
	end
end

return carryover
