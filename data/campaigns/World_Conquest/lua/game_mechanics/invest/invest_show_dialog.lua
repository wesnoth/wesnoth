local _ = wesnoth.textdomain 'wesnoth-World_Conquest'

local dialog_wml = wc2_invest_dialog


function wc2_show_invest_dialog_impl(args)
	local side_num = wesnoth.current.side
	local available_artifacts = args.items_available
	local available_heroes = args.heroes_available
	local available_deserters = args.deserters_available
	local available_commanders = args.commanders_available
	local available_training = args.trainings_available

	local show_artifacts = args.items_available ~= nil
	local show_heroes = args.heroes_available ~= nil
	local show_training = args.trainings_available ~= nil
	local show_other = args.gold_available

	local cati_items, cati_heroes, cati_training, cati_other

	local res = nil

	local index_map = {}
	local details_index_counter = 1
	local function add_index(page, r)
		index_map[page] = {page_num = details_index_counter, res = r}
		details_index_counter = details_index_counter + 1
		if res == nil then
			res = r
		end
	end

	local function preshow()
		local cati_current = 0
		if show_artifacts then
			cati_current = cati_current + 1
			wesnoth.add_dialog_tree_node("category", cati_current, "left_tree")
			wesnoth.set_dialog_value(true, "left_tree", cati_current)
			wesnoth.set_dialog_value(_ "Artifacts", "left_tree", cati_current, "category_name")
			for i,v in ipairs(available_artifacts) do
				local artifact_info = wc2_artifacts.get_artifact(tonumber(v))
				if not artifact_info then
					error("invalid item id'" .. v .. "'")
				end
				wesnoth.add_dialog_tree_node("item_desc", i, "left_tree", cati_current)
				wesnoth.set_dialog_value(artifact_info.icon, "left_tree", cati_current, i, "image")
				wesnoth.set_dialog_value(artifact_info.name, "left_tree", cati_current, i, "name")
				wesnoth.set_dialog_value(wc2_color.tc_text(artifact_info.description), "left_tree", cati_current, i, "desc")

				wesnoth.add_dialog_tree_node("", -1, "details")
				wesnoth.set_dialog_value(artifact_info.info, "details", details_index_counter, "label")
				add_index(cati_current .. "_" .. i, { pick = "item", type=v })
			end
		end

		if show_heroes then
			cati_current = cati_current + 1
			wesnoth.add_dialog_tree_node("category", cati_current, "left_tree")
			wesnoth.set_dialog_value(true, "left_tree", cati_current)
			wesnoth.set_dialog_value(_ "Heroes", "left_tree", cati_current, "category_name")
			local i = 1
			if available_commanders then
				local desc = _ "Commanders will take your leaders place when the leader dies, possible commanders:"
				for j,v in ipairs(available_commanders) do
					desc = desc .. "\n" .. wesnoth.unit_types[v].name
				end

				wesnoth.add_dialog_tree_node("item", i, "left_tree", cati_current)
				wesnoth.set_dialog_value(wc2_color.tc_image("units/unknown-unit.png"), "left_tree", cati_current, i, "image")
				wesnoth.set_dialog_value(_ "Commander" .. "\n" .. wc2_color.tc_text(_ "promote to leader"), "left_tree", cati_current, i, "name")

				wesnoth.add_dialog_tree_node("", -1, "details")
				wesnoth.set_dialog_value(desc, "details", details_index_counter, "label")
				add_index(cati_current .. "_" .. i, { pick = "hero", type="wc2_commander" })
				i = i + 1
			end
			for j,v in ipairs(available_heroes) do
				unit_type = wesnoth.unit_types[v]

				wesnoth.add_dialog_tree_node("item", i, "left_tree", cati_current)
				wesnoth.set_dialog_value(wc2_color.tc_image(unit_type.image), "left_tree", cati_current, i, "image")
				wesnoth.set_dialog_value(unit_type.name, "left_tree", cati_current, i, "name")

				wesnoth.add_dialog_tree_node("hero", -1, "details")
				wesnoth.set_dialog_value(unit_type, "details", details_index_counter, "unit")
				add_index(cati_current .. "_" .. i, { pick = "hero", type=v })
				i = i + 1
			end
			if available_deserters then
				local desc = "<b>" .. _ "possible units:" .. "</b>"
				for j,v in ipairs(available_deserters) do
					desc = desc .. "\n" .. wesnoth.unit_types[v].name
				end
				wesnoth.add_dialog_tree_node("item", i, "left_tree", cati_current)
				wesnoth.set_dialog_value(wc2_color.tc_image("units/unknown-unit.png"), "left_tree", cati_current, i, "image")
				wesnoth.set_dialog_value(_ "Deserter" .. "\n" .. wc2_color.tc_text("+15 gold"), "left_tree", cati_current, i, "name")

				wesnoth.add_dialog_tree_node("", -1, "details")
				wesnoth.set_dialog_value(desc, "details", details_index_counter, "label")
				add_index(cati_current .. "_" .. i, { pick = "hero", type="wc2_deserter" })
			end
		end

		if show_training then
			cati_current = cati_current + 1
			wesnoth.add_dialog_tree_node("category", cati_current, "left_tree")
			wesnoth.set_dialog_value(true, "left_tree", cati_current)
			wesnoth.set_dialog_value(_ "Training", "left_tree", cati_current, "category_name")
			for i,v in ipairs(available_training) do
				local current_grade = wc2_training.get_level(side_num, v)
				local training_info = wc2_training.get_trainer(v)
				local train_message = wc2_training.generate_message(v, current_grade + 1)
				local train_message_before = wc2_training.generate_message(v, current_grade)

				local title = wesnoth.format(_ "$name Training", { name = training_info.name })
				local desc = wc2_training.describe_training_level2(current_grade, #training_info.grade) .. wc2_color.tc_text(" → ") .. wc2_training.describe_training_level2(current_grade + 1, #training_info.grade)

				wesnoth.add_dialog_tree_node("item_desc", i, "left_tree", cati_current)
				wesnoth.set_dialog_value(training_info.image, "left_tree", cati_current, i, "image")
				wesnoth.set_dialog_value(title, "left_tree", cati_current, i, "name")
				wesnoth.set_dialog_value(desc, "left_tree", cati_current, i, "desc")

				wesnoth.add_dialog_tree_node("", -1, "details")
				local label  = wc2_color.tc_text("<big>" .. _ "Before:" .. "</big>\n") .. train_message_before.message .. wc2_color.tc_text("\n<big>" .. _ "After:" .. "</big>\n") .. train_message.message
				wesnoth.set_dialog_value(label , "details", details_index_counter, "label")
				--wesnoth.set_dialog_value(train_message.message, "details", details_index_counter, "training_after")
				add_index(cati_current .. "_" .. i, { pick = "training", type=v })

			end
		end

		if show_other then
			cati_current = cati_current + 1
			wesnoth.add_dialog_tree_node("category", cati_current, "left_tree")
			wesnoth.set_dialog_value(true, "left_tree", cati_current)
			wesnoth.set_dialog_value(_ "Other", "left_tree", cati_current, "category_name")



			local colored_galleon = wc2_color.tc_image("units/transport/transport-galleon.png")
			local supplies_image = "misc/blank-hex.png~SCALE(90,80)~BLIT(" .. colored_galleon .. ",9,4)"
			local supplies_text = wc2_color.tc_text(_ "+70 gold and +1 village")
			--"+{STR_COLOR_PLAYER ("+70 "+{STR_GOLD}+{STR_AND}+"+1 "+{STR_VILLAGE})}

			wesnoth.add_dialog_tree_node("item_desc", 1, "left_tree", cati_current)
			wesnoth.set_dialog_value(supplies_image, "left_tree", cati_current, 1, "image")
			wesnoth.set_dialog_value(_"Stock up supplies", "left_tree", cati_current, 1, "name")
			wesnoth.set_dialog_value(supplies_text, "left_tree", cati_current, 1, "desc")

			wesnoth.add_dialog_tree_node("", -1, "details")
			wesnoth.set_dialog_value(_"Gives 70 gold and places a village on your keep.", "details", details_index_counter, "label")
			add_index(cati_current .. "_" .. 1, { pick = "gold" })
		end

		wesnoth.set_dialog_callback(function()
			local selected = wesnoth.get_dialog_value("left_tree")
			local selected_data = index_map[table.concat(selected, '_')]
			if selected_data ~= nil then
				wesnoth.set_dialog_value(selected_data.page_num, "details")
			end
			res = selected_data.res
		end, "left_tree")
	end
	local d_res = wesnoth.show_dialog(dialog_wml, preshow)
	return d_res, res
end

function wc2_show_invest_dialog(args)
	--do it in a loop to disable esc.
	while true do
		local d_res, res = wc2_show_invest_dialog_impl(args)
		if d_res ~= -2 then
			return res
		end
	end
end

return wc2_show_invest_dialog
