local _ = wesnoth.textdomain 'wesnoth-wc'

local dialog_wml = wml.load "campaigns/World_Conquest/gui/invest_dialog.cfg"

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

	local function preshow(dialog)

		local details = dialog.details
		local root_node = dialog.left_tree

		function gui.widget.add_invest_category(parent_node, name)			
			local node = parent_node:add_item_of_type("category")
			node.category_name.label = name
			node.unfolded = true
			return node
		end

		function gui.widget.add_invest_item(parent_node, args)
			local node_type = args.desc and "item_desc" or "item"
			local page_type = args.page_type or ""
			
			local node = parent_node:add_item_of_type(node_type)
			local details_page = details:add_item_of_type(page_type)

			node.image.label = args.icon
			node.name.label = args.name
			if args.desc then
				node.desc.label = args.desc
			end

			index_map[table.concat(node.path, "_")] = { page_num = details.item_count, res = args.result }
			return node, details_page
		end

		local cati_current = 0
		if show_artifacts then			
			local node = root_node:add_invest_category(_ "Artifacts")

			for i,v in ipairs(available_artifacts) do
				local artifact_info = wc2_artifacts.get_artifact(tonumber(v))
				if not artifact_info then
					error("invalid item id'" .. v .. "'")
				end

				local subnode, page = node:add_invest_item {
					icon = artifact_info.icon,
					name = artifact_info.name,
					desc = wc2_color.tc_text(artifact_info.description),
					result = { pick = "item", type=v }
				}
				page.info_label.label = artifact_info.info
			end
		end

		if show_heroes then
			local node = root_node:add_invest_category(_ "Heroes")
		
			if available_commanders then
				local desc = _ "Commanders will take your leader’s place when the leader dies, possible commanders:"
				for j,v in ipairs(available_commanders) do
					desc = desc .. "\n" .. wesnoth.unit_types[v].name
				end

				local subnode, page = node:add_invest_item {
					icon = wc2_color.tc_image("units/unknown-unit.png"),
					name = _ "Commander" .. "\n" .. wc2_color.tc_text(_ "promote to leader"),
					result = { pick = "hero", type= "wc2_commander" }
				}
				page.info_label.label = desc
			end
			for j,v in ipairs(available_heroes) do
				local unit_type = wesnoth.unit_types[v]


				local subnode, page = node:add_invest_item {
					page_type = "hero",
					icon = wc2_color.tc_image(unit_type.image),
					name = unit_type.name,
					result = { pick = "hero", type= v }
				}
				page.unit_info.unit = unit_type
			end
			if available_deserters then
				local desc = "<b>" .. _ "possible units:" .. "</b>"
				for j,v in ipairs(available_deserters) do
					desc = desc .. "\n" .. wesnoth.unit_types[v].name
				end

				local subnode, page = node:add_invest_item {
					icon = wc2_color.tc_image("units/unknown-unit.png"),
					name = _ "Deserter" .. "\n" .. wc2_color.tc_text("+15 gold"),
					result = { pick = "hero", type= "wc2_deserter" }
				}
				page.info_label.label = desc
			end
		end

		if show_training then
			local node = root_node:add_invest_category(_ "Training")
			for i,v in ipairs(available_training) do
				local current_grade = wc2_training.get_level(side_num, v)
				local training_info = wc2_training.get_trainer(v)
				local train_message = wc2_training.generate_message(v, current_grade + 1)
				local train_message_before = wc2_training.generate_message(v, current_grade)

				local title = stringx.vformat(_ "$name Training", { name = training_info.name })
				local desc = wc2_training.describe_training_level2(current_grade, #training_info.grade) .. wc2_color.tc_text(" → ") .. wc2_training.describe_training_level2(current_grade + 1, #training_info.grade)


				local subnode, page = node:add_invest_item {
					icon = training_info.image,
					name = title,
					desc = desc,
					result = { pick = "training", type=v }
				}
				page.info_label.label  = wc2_color.tc_text("<big>" .. _ "Before:" .. "</big>\n") .. train_message_before.message .. wc2_color.tc_text("\n<big>" .. _ "After:" .. "</big>\n") .. train_message.message
			end
		end

		if show_other then
			local node = root_node:add_invest_category(_ "Other")

			local colored_galleon = wc2_color.tc_image("units/transport/transport-galleon.png")
			local supplies_image = "misc/blank-hex.png~SCALE(90,80)~BLIT(" .. colored_galleon .. ",9,4)"
			local supplies_text = wc2_color.tc_text(_ "+70 gold and +1 village")


			local subnode, page = node:add_invest_item {
				icon = supplies_image,
				name = _"Stock up supplies",
				desc = supplies_text,
				result = { pick = "gold" }
			}

			page.info_label.label = _"Gives 70 gold and places a village on your keep."
		end

		local function set_result()
			local selected = root_node.selected_item_path
			local selected_data = index_map[table.concat(selected, '_')]
			if selected_data ~= nil then
				details.selected_index = selected_data.page_num
			end
			res = selected_data.res
		end

		root_node.on_modified = set_result
		set_result()
	end
	local d_wml = wml.get_child(dialog_wml, 'resolution')
	local d_res = gui.show_dialog(d_wml, preshow)
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
