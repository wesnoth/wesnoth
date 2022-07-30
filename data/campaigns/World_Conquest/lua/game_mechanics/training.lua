local _ = wesnoth.textdomain 'wesnoth-wc'
local on_event = wesnoth.require "on_event"

local training = {}


function training.add_training_data(a)
	training.trainers = training.trainers or {}
	table.insert(training.trainers, a)
end

function training.read_wml_data(cfg)
	for i, t in ipairs(wc2_convert.wml_to_lon(cfg, "wct_trainer_list").trainer or {}) do
		training.add_training_data(t)
	end
end

function training.get_list()
	if not training.trainers then
		training.init_data()
	end
	return training.trainers
end

function training.init_data()
	local cfg = wc2_utils.get_wc2_data("trainer")
	for i, t in ipairs(wc2_convert.wml_to_lon(cfg, "wct_trainer_list").trainer or {}) do
		training.add_training_data(t)
	end
end

function training.get_trainer(trainer)
	return training.get_list()[trainer]
end


function training.get_chances(trainer, grade)
	return training.get_trainer(trainer).grade[grade + 1].chance
end

function training.apply_trait(u, trait, check)
	if u:matches(check) and u:matches( wml.tag.filter_wml { wml.tag.modifications { wml.tag.trait { id = trait.id } } } ) then
		u:add_modification("trait", trait)
	else
		u:add_modification("object", { wml.tag.effect { apply_to  = "hitpoints", increase_total = 1, heal_full = true}})
	end
end


--the current level of a certain traing, a value of 0 means this skill wasn't trained yet.
function training.get_level(side_num, trainer)
	return wesnoth.sides[side_num].variables["wc2.training[" .. trainer - 1 .. "].level"] or 0
end

function training.set_level(side_num, trainer, level)
	wesnoth.sides[side_num].variables["wc2.training[" .. trainer - 1 .. "].level"] = level
end

function training.inc_level(side, trainer, level)
	local new_level = training.get_level(side, trainer) + (level or 1)
	if new_level < 0 or new_level >= #training.get_trainer(trainer).grade then
		error("training level out of range")
	end
	training.set_level(side, trainer, new_level)
end

-- to be used by bonus points chance to extra taining.
function training.get_level_sum(side)
	local res = 0
	for i = 1, #training.get_list() do
		res = res + training.get_level(side, i)
	end
	return res
end

function training.trainings_left(side_num, trainer)
	return (#training.get_trainer(trainer).grade - 1) - training.get_level(side_num, trainer)
end

function training.available(side_num, trainer, amount)
	return training.trainings_left(side_num, trainer) >= (amount or 1)
end

function training.has_max_training(side_num, trainer, amount)
	return training.available(side_num, trainer) == 0
end

function training.list_available(side_num, among, amount)
	local av = among or wc2_utils.range(#training.get_list())
	local res = {}
	for i,v in ipairs(av) do
		local j = tonumber(v)
		if training.available(side_num, j, amount) then
			table.insert(res, j)
		end
	end
	return res
end

function training.find_available(side_num, among, amount)
	local possible_traintypes = training.list_available(side_num, among, amount)
	if #possible_traintypes == 0 then
		return
	else
		return possible_traintypes[mathx.random(#possible_traintypes)]
	end
end

function training.describe_training_level(name, level, max_level)
	if level == max_level then
		return tostring(stringx.vformat(_ "$name Training Maximum Level", {
			name = name
		}))
	else
		return tostring(stringx.vformat(_ "$name Training level $level", {
			name = name,
			level = level
		}))
	end
end

function training.describe_training_level2(level, max_level)
	if level == max_level then
		return _ "Maximum Level"
	else
		return tostring(stringx.vformat(_ "level $level", {
			level = level
		}))
	end
end

function training.generate_message(n_trainer, n_grade)
	local c_trainer = training.get_trainer(n_trainer)
	local c_grade = c_trainer.grade[n_grade + 1]
	if c_grade == nil then
		return { message = "" }
	end
	local caption = training.describe_training_level(c_trainer.name, n_grade, #c_trainer.grade - 1)
	local messages = {}
	for unused, chance in ipairs(c_grade.chance) do
		local vchance = chance.variable_substitution ~= false and wml.tovconfig(chance) or chance
		if (chance.value or 0) < 100 then
			local str = stringx.vformat(_ "$chance| chance to $arrow $desc", {
				chance = ("%d%%"):format(vchance.value),
				desc = wc2_utils.get_fstring(chance, "info"),
				arrow = wc2_color.tc_text(" → ")
			})
			table.insert(messages, tostring(str))
		else
			table.insert(messages, tostring(wc2_utils.get_fstring(chance, "info")))
		end
	end
	return {
		caption = caption,
		message = table.concat(messages, "\n"),
		speaker = "narrator",
		image = c_trainer.image,
	}
end

function training.give_bonus(side_num, cx, amount, traintype_index)
	local traintype = training.get_trainer(traintype_index)
	local cur_level = training.get_level(side_num, traintype_index)
	local new_level = cur_level + amount
	local teacher = wc2_heroes.place(amount > 1 and traintype.advanced_type or traintype.type, side_num, cx.x1,cx.y1)
	local u = wesnoth.units.get(cx.x1, cx.y1)
	wc2_utils.facing_each_other(u, teacher)

	wesnoth.wml_actions.sound {
		name = "flail-miss.ogg"
	}
	wesnoth.wml_actions.message {
		speaker = teacher.id,
		message = traintype.dialogue,
	}
	wesnoth.units.extract(teacher)
	local message = training.generate_message(traintype_index, new_level)
	wesnoth.wml_actions.message(message)

	training.inc_level(side_num, traintype_index, amount)
	return true
end

function training.bonus_calculate_amount(side_num)
	local amount = 1
	local advanced_chance = 4 * training.get_level_sum(side_num)
	if wc2_scenario.scenario_num() > 3 or mathx.random(100) <= advanced_chance then
		amount = 2
	end
	return amount
end

function training.pick_bonus(side_num)
	local amount = training.bonus_calculate_amount(side_num)
	-- dark training reduced chances
	local traintype_index = training.find_available(side_num, {1,2,3,4,5,6,2,3,4,5,6,2,3,4,5,6}, amount)
	if traintype_index == nil then
		return nil
	end

	--dark training increased level.
	if traintype_index == 1 then
		amount = math.min(training.trainings_left(side_num, traintype_index), math.max(amount, wc2_scenario.scenario_num() - 1))
	end
	return traintype_index, amount
end

on_event("recruit", function(event_context)
	training.apply(wesnoth.units.get(event_context.x1, event_context.y1))
end)

function training.apply(u)
	if not u then
		return
	end
	local side = u.side
	local trait = {}
	local descriptions = {}
	trait.male_name = _ "trained"
	trait.female_name = _ "female^trained"
	trait.generate_description = false
	for i, trainer in ipairs(training.get_list()) do
		local level = training.get_level(side, i) or 0
		for unused, chance in ipairs(training.get_chances(i, level)) do
			--some effects use expressions like $(5+{GRADE}) so we want variable_substitution there
			local vchance = wml.tovconfig(chance)
			local filter = wml.get_child(vchance, "filter")
			local matches_filter = (not filter) or u:matches(filter)
			if mathx.random(100) <= vchance.value and matches_filter then
				--wesnoth.wml_actions.message { message = "Got it" }
				table.insert(descriptions, wc2_utils.get_fstring(chance, "info"))
				for effect in wml.child_range(vchance, "effect") do
					table.insert(trait, {"effect", effect })
				end
			end
		end
	end
	trait.description = wc2_utils.concat(descriptions, "\n")
	if #trait > 0 then
		u:add_modification("trait", trait)
		--rebuild unit, to reduce savefile size.
		u:transform(u.type)
	end
	u.variables.wc2_trained = true
	u.hitpoints = u.max_hitpoints
end

function wesnoth.wml_actions.wc2_apply_training(cfg)
	for i,u in ipairs(wesnoth.units.find_on_map(cfg)) do
		training.apply(u)
	end
end

function wesnoth.wml_actions.wc2_give_random_training(cfg)
	local side_num = cfg.side
	local amount = cfg.amount or 1
	local among = cfg.among and stringx.split(cfg.among or "")
	for i = 1, amount do
		local traintype = training.find_available(side_num, among)
		if traintype == nil then error("wc2_give_random_training: everything alerady maxed") end
		training.inc_level(side_num, traintype, 1)
	end
end

function training.describe_bonus(side, traintype)
	local traintype_data = training.get_trainer(traintype)
	local cur_level = training.get_level(side, traintype)
	local max_level = #traintype_data.grade - 1
	local image = wesnoth.unit_types[traintype_data.type].__cfg.image
	local message = nil
	if cur_level == max_level then
		message = _"Nothing to learn here"
	else
		message = stringx.vformat(_"From $level_before to $level_after", {
			level_before = training.describe_training_level(traintype_data.name, cur_level, max_level),
			level_after = training.describe_training_level(traintype_data.name, cur_level + 1, max_level)
		})
	end
	return message, image
end

return training
