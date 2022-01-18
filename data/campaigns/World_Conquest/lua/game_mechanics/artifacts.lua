local on_event = wesnoth.require("on_event")
local helper = wesnoth.require("helper")

local _ = wesnoth.textdomain 'wesnoth-wc'

local artifacts = {}
artifacts.list = {}

function artifacts.add_artifact_data(a)
	table.insert(artifacts.list, a)
end

function artifacts.read_wml_data(cfg)
	for i, artifact in ipairs(wc2_convert.wml_to_lon(wml.literal(cfg), "wct_artifact_list").artifact or {}) do
		artifacts.add_artifact_data(artifact)
	end
end

function artifacts.init_data()
	local cfg = wc2_utils.get_wc2_data("artifact")
	for i, a in ipairs(wc2_convert.wml_to_lon(cfg, "wct_artifact_list").artifact or {}) do
		artifacts.add_artifact_data(a)
	end
end


function artifacts.get_artifact(id)
	return artifacts.list[id]
end

function artifacts.get_artifact_list()
	return artifacts.list
end


function artifacts.drop_message(index)
	local aftifact_data = artifacts.get_artifact(index)
	wesnoth.wml_actions.message {
		speaker = "narrator",
		caption = aftifact_data.name,
		message = aftifact_data.info .. "\n" .. wc2_color.bonus_text(aftifact_data.description),
		image = aftifact_data.icon,
	}
end

function wc2_artifact_needs_compensation(side)
	return not wc2_scenario.is_human_side(side.side)
end


-- place an artifact with id @a index on the map at position @a x, y.
-- can be used from the bug console as `lua wc2_artifacts.place_item(30,20,1)`
function artifacts.place_item(x, y, index)
	wesnoth.wml_actions.item {
		x = x,
		y = y,
		image = artifacts.get_artifact(index).icon,
		z_order = 20,
		wml.tag.variables { wc2_atrifact_id = index },
	}
end

-- give te item with id @a index to unit @a unit, set @a visualize=true, to show the item pickup animation.
function artifacts.give_item(unit, index, visualize)
	local aftifact_data = artifacts.get_artifact(index)
	if visualize then
		-- play visual/sound effects if item have any
		wesnoth.wml_actions.sound {
			name = aftifact_data.sound or ""
		}
		if unit.gender == "male" then
			wesnoth.wml_actions.sound {
				name = aftifact_data.sound_male or ""
			}
		else
			wesnoth.wml_actions.sound {
				name = aftifact_data.sound_female or ""
			}
		end
		for i, animate_unit in ipairs(aftifact_data.animate_unit) do
			wesnoth.wml_actions.animate_unit(animate_unit)
		end
	end
	local make_holder_loyal = wml.variables["wc2_config_items_make_loyal"] ~= false
	-- is_commander or is_hero imples unit.upkeep == "loyal"
	-- note that the following `unit.upkeep` does not match normal
	-- level 0 (which have still 'full' upkeep) only units with upkeep=0 explicitly set
	if make_holder_loyal and (not unit.canrecruit) and (unit.upkeep ~= 0) and (unit.upkeep ~= "loyal") then
		unit:add_modification("object", { wml.tag.effect { apply_to = "wc2_overlay", add = "misc/loyal-icon.png" }})
	end

	local object = {
		wc2_atrifact_id = index,
		-- cannot filter on wc2_artifact_id being empty so we also need wc2_is_artifact
		wc2_is_artifact = true,
	}
	if make_holder_loyal then
		table.insert(object, wml.tag.effect { apply_to= "loyal" })
	end
		
		
	-- IDEA: i _could_ replace the follwing with a 'apply_to=wc2_artifact' effect that
	--       basicially applies all effects in the [artifact]s definition. The obvious
	--       advantage would be a smaller savefile size. Also this would change how savefiles
	--       would behave if an artifacts effect has changed, i am currently not sure
	--       whether that'd be good or bad
	--
	--       One of the reasons why i currently won't do this is to make the artifacts list
	--       more flexible: the suggested approach requires that artifacts are loaded before
	--       units are created which means artifacts must be loaded at toplevel [lua] tags
	for i, effect in ipairs(aftifact_data.effect) do
		table.insert(object, wml.tag.effect (effect) )
	end
	local unit_initial_hp = unit.hitpoints
	unit:add_modification("object", object)
	--rebuild unit, to reduce savefile size.
	unit:transform(unit.type)
	-- restore unit hitpoints to before they picked up the artifact
	unit.hitpoints = unit_initial_hp
	-- the artifact might reduce the max xp.
	unit:advance(true, true)
end

-- unit picking up artifacts
on_event("wc2_drop_pickup", function(ec)
	local item = wc2_dropping.current_item
	local unit = wesnoth.units.get(ec.x1, ec.y1)
	if not item.variables.wc2_atrifact_id then
		return
	end

	if not unit then 
		return
	end

	local side_num = unit.side
	local is_human = wc2_scenario.is_human_side(side_num)
	if not wml.variables["wc2_config_experimental_pickup"] and not is_human  then
		return
	end
	

	local index = item.variables.wc2_atrifact_id
	local filter = artifacts.get_artifact(index).filter
	if filter and not unit:matches(filter) then
		if is_human then
			wesnoth.wml_actions.message {
				id = unit.id,
				message = _"I cannot pick up that item.",
			}
		end
		return
	end

	if is_human and not wml.variables["wc2_config_disable_pickup_confirm"] then
		if not wc2_pickup_confirmation_dialog.promt_synced(unit, artifacts.get_artifact(index).icon) then
			return
		end
	end
	

	wc2_dropping.item_taken = true
	artifacts.give_item(unit, index, true)
	wesnoth.allow_undo(false)
end)

-- returns a list of artifact ids, suitable for  the give type ('enemy' for example).
function artifacts.fresh_artifacts_list(for_type)
	local res = {} 
	for i,v in ipairs(artifacts.get_artifact_list()) do
		if not for_type or not stringx.map_split(v.not_available or "")[for_type] then
			table.insert(res, i)
		end
	end
	return res
end


-- drop all items a dying unit carries.
on_event("die", function(event_context)
	local unit = wesnoth.units.get(event_context.x1, event_context.y1)
	if not unit then
		return
	end
	if not wml.variables["wc2_config_experimental_pickup"] and wc2_scenario.is_human_side(unit.side) then
		return
	end
	for object in wml.child_range(wml.get_child(unit.__cfg, "modifications") or {}, "object") do
		if object.wc2_atrifact_id then
			artifacts.place_item(unit.x, unit.y, object.wc2_atrifact_id)
			artifacts.drop_message(object.wc2_atrifact_id)
			wesnoth.allow_undo(false)
		end
	end

	-- remove the item from the unit, just in case the unit is somehow brought back to life by another addons code. (for example 'besieged druid' can do such a thing)
	unit:remove_modifications { wc2_is_artifact = true }
end)

-- returns true if there is an item in the map at the given position,
-- used to determine whether to show the artifact info menu at that position. 
function artifacts.is_item_at(x,y)
	for i,item in ipairs(wesnoth.interface.get_items(x,y)) do
		if item.variables.wc2_atrifact_id then
			return true
		end
	end
	return false
end

-- shows an information [message] about the item laying at position 
-- @a cfg.x, cfg.y
function wesnoth.wml_actions.wc2_show_item_info(cfg)
	local x = cfg.x
	local y = cfg.y
	for i,item in ipairs(wesnoth.interface.get_items(x,y)) do
		if item.variables.wc2_atrifact_id then
			local artifact_info = artifacts.get_artifact(item.variables.wc2_atrifact_id)
			wesnoth.wml_actions.message {
				scroll = false,
				image = artifact_info.icon,
				caption = artifact_info.name,
				message= artifact_info.info .. "\n" .. wc2_color.help_text(artifact_info.description),
			}
		end
	end
end

wc2_utils.menu_item {
	id="4_WCT_Item_Info_Option",
	description = _ "Remind me what this item does",
	image = "icons/terrain/terrain_type_info.png",
	synced = false,
	filter = artifacts.is_item_at,
	handler = function(cx)
		wesnoth.wml_actions.wc2_show_item_info {
			x = cx.x1,
			y = cx.y1,
		}
	end
}

function wesnoth.wml_actions.wc2_place_item(cfg)
	artifacts.place_item(cfg.x, cfg.y, cfg.item_index)
	if cfg.message then
		artifacts.drop_message(cfg.item_index)
	end
end

artifacts.init_data()

return artifacts
