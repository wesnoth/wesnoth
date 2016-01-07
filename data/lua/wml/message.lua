
local helper = wesnoth.require "lua/helper.lua"
local utils = wesnoth.require "lua/wml-utils.lua"
local location_set = wesnoth.require "lua/location_set.lua"
local _ = wesnoth.textdomain "wesnoth"

local function log(msg, level)
	wesnoth.wml_actions.wml_message({
		message = msg,
		logger = level,
	})
end

local function get_image(cfg, speaker)
	local image = cfg.image

	if speaker and image == nil then
		image = speaker.portrait
	end

	if image == "none" or image == nil then
		return ""
	end

	return image
end

local function get_caption(cfg, speaker)
	local caption = cfg.caption

	if not caption and speaker ~= nil then
		caption = speaker.name or speaker.type_name
	end

	return caption
end

local function get_speaker(cfg)
	local speaker
	local context = wesnoth.current.event_context

	if cfg.speaker == "narrator" then
		speaker = "narrator"
	elseif cfg.speaker == "unit" then
		speaker = wesnoth.get_unit(context.x1 or 0, context.y1 or 0)
	elseif cfg.speaker == "second_unit" then
		speaker = wesnoth.get_unit(context.x2 or 0, context.y2 or 0)
	else
		speaker = wesnoth.get_units(cfg)[1]
	end

	return speaker
end

local function message_user_choice(cfg, speaker, options, text_input)
	local image = get_image(cfg, speaker)
	local caption = get_caption(cfg, speaker)

	local left_side = true
	-- If this doesn't work, might need tostring()
	if image:find("~RIGHT()") then
		left_side = false
		-- The percent signs escape the parentheses for a literal match
		image = image:gsub("~RIGHT%(%)", "")
	end

	local msg_cfg = {
		left_side = left_side,
		title = caption,
		message = cfg.message,
		portrait = image,
	}
	
	if speaker ~= nil then
		if cfg.male_message ~= nil and speaker.gender == "male" then
			msg_cfg.message = cfg.male_message
		elseif cfg.female_message ~= nil and speaker.gender == "female" then
			msg_cfg.message = cfg.female_message
		end
	end

	-- Parse input text, if not available all fields are empty
	if text_input then
		local input_max_size = tonumber(text_input.max_length) or 256
		if input_max_size > 1024 or input_max_size < 1 then
			log("Invalid maximum size for input " .. input_max_size, "warning")
			input_max_size = 256
		end

		-- This roundabout method is because text_input starts out
		-- as an immutable userdata value
		text_input = {
			label = text_input.label or "",
			text  = text_input.text	 or "",
			max_length = input_max_size,
		}
	end

	return function()
		local option_chosen, ti_content = wesnoth.show_message_dialog(msg_cfg, options, text_input)

		if option_chosen == -2 then -- Pressed Escape (only if no input)
			wesnoth.skip_messages()
		end

		local result_cfg = {}

		if #options > 0 then
			result_cfg.value = option_chosen
		end

		if text_input ~= nil then
			result_cfg.text = ti_content
		end

		return result_cfg
	end
end

function wesnoth.wml_actions.message(cfg)
	local show_if = helper.get_child(cfg, "show_if") or {}
	if not wesnoth.eval_conditional(show_if) then
		log("[message] skipped because [show_if] did not pass", "debug")
		return
	end

	-- Only the first text_input tag is considered
	local text_input
	for cfg in helper.child_range(cfg, "text_input") do
		if text_input ~= nil then
			log("Too many [text_input] tags, only one accepted", "warning")
			break
		end
		text_input = cfg
	end

	local options, option_events = {}, {}
	for option in helper.child_range(cfg, "option") do
		local condition = helper.get_child(option, "show_if") or {}

		if wesnoth.eval_conditional(condition) then
			if option.message and not option.image and not option.label then
				local message = tostring(option.message)
				if message:find("&") or message:find("=") or message:find("*") == 1 then
					wesnoth.wml_actions.deprecated_message{message = '[option]message="&image=col1=col2" is deprecated, use new DescriptionWML instead (default, image, label, description)'}
				end
				-- Legacy format
				table.insert(options, option.message)
			else
				local opt = {
					label = option.label,
					description = option.description,
					image = option.image,
					default = option.default,
				}
				if option.message then
					if not option.label then
						-- Support either message or description
						opt.label = option.message
					else
						log("[option] has both label= and message=, ignoring the latter", "warning")
					end
				end
				table.insert(options, opt)
			end
			table.insert(option_events, {})

			for cmd in helper.child_range(option, "command") do
				table.insert(option_events[#option_events], cmd)
			end
		end
	end

	-- Check if there is any input to be made, if not the message may be skipped
	local has_input = text_input ~= nil or #options > 0

	if not has_input and wesnoth.is_skipping_messages() then
		-- No input to get and the user is not interested either
		log("Skipping [message] because user not interested", "debug")
		return
	end

	local sides_for = cfg.side_for
	if sides_for and not has_input then
		local show_for_side = false

		-- Sanity checks on side number and controller
		for side in utils.split(sides_for) do
			side = tonumber(side)
			if side > 0 and side < #wesnoth.sides and wesnoth.sides[side].controller == "human" then
				show_for_side = true
				break
			end
		end

		if not show_for_side then
			-- Player isn't controlling side which should see the message
			log("Player isn't controlling side that should see [message]", "debug")
			return
		end
	end

	local speaker = get_speaker(cfg)
	if not speaker then
		-- No matching unit found, continue onto the next message
		log("No speaker found for [message]", "debug")
		return
	elseif speaker == "narrator" then
		-- Narrator, so deselect units
		wesnoth.deselect_hex()
		-- The speaker is expected to be either nil or a unit later
		speaker = nil
	else
		-- Check ~= false, because the default if omitted should be true
		if cfg.scroll ~= false then
			wesnoth.scroll_to_tile(speaker.x, speaker.y)
		end

		wesnoth.select_hex(speaker.x, speaker.y, true)
		wesnoth.fire("redraw")
	end

	if cfg.sound then wesnoth.play_sound(cfg.sound) end

	local msg_dlg = message_user_choice(cfg, speaker, options, text_input)

	local option_chosen
	if not has_input then
		-- Always show the dialog if it has no input, whether we are replaying or not
		msg_dlg()
	else
		local wait_description = cfg.wait_description or _("input")
		if type(sides_for) ~= "number" then
			-- 0 means currently playing side.
			sides_for = 0
		end
		local choice = wesnoth.synchronize_choice(wait_description, msg_dlg, sides_for)

		option_chosen = tonumber(choice.value)

		if text_input ~= nil then
			-- Implement the consequences of the choice
			wesnoth.set_variable(text_input.variable or "input", choice.text)
		end
	end
	
	-- Unhilight the speaker
	wesnoth.deselect_hex()

	if #options > 0 then
		if option_chosen > #options then
			log("invalid choice (" .. option_chosen .. ") was specified, choice 1 to " ..
				#options .. " was expected", "debug")
			return
		end

		for i, cmd in ipairs(option_events[option_chosen]) do
			local action = utils.handle_event_commands(cmd, "plain")
			if action ~= "none" then break end
		end
	end
end
