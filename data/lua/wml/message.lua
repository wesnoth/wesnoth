
local utils = wesnoth.require "wml-utils"
local _ = wesnoth.textdomain "wesnoth"

local function log(msg, level)
	wesnoth.log(level, msg, true)
end

local function get_image(cfg, speaker)
	local image = cfg.image
	local left_side = true

	if speaker and (image == nil or image == "") and (cfg.second_image == nil or cfg.second_image == "") then
		image = speaker.portrait
	end

	if image == "none" or image == nil then
		return "", true
	end

	-- Note: This is deprecated except for use to set default alignment in portraits
	-- (Move it into the first if statement later, with a nil check)
	if image:find("~RIGHT%(%)") then
		left_side = false
		-- The percent signs escape the parentheses for a literal match
		image = image:gsub("~RIGHT%(%)", "")
	end

	if cfg.image_pos == 'left' then
		left_side = true
	elseif cfg.image_pos == 'right' then
		left_side = false
	elseif cfg.image_pos ~= nil then
		wml.error('Invalid [message]image_pos - should be left or right')
	end

	return image, left_side
end

local function get_caption(cfg, speaker)
	local caption = cfg.caption

	if not caption and speaker ~= nil then
		if speaker.name ~= nil and tostring(speaker.name):len() > 0 then
			caption = speaker.name
		else
			caption = speaker.__cfg.language_name
		end
	end

	return caption
end

local function get_pango_color(color)
	local pango_color = "#"

	-- if a hex color was passed in
	-- or if a color string was passed in - contains no non-letter characters
	-- just use that
	if string.sub(color, 1, 1) == "#" or not string.match(color, "%A+") then
		pango_color = color
	-- decimal color was passed in, convert to hex color for pango
	else
		for s in string.gmatch(color, "%d+") do
			pango_color = pango_color .. tonumber(s, 16)
		end
	end

	return pango_color
end

-- add formatting
local function add_formatting(cfg, text)
	-- span tag
	local formatting = "<span"

	-- if message text, add formatting
	if text and cfg then
		-- add font
		if cfg.font and cfg.font ~= '' then
			formatting = formatting .. " font='" .. cfg.font .. "'"
		end

		-- add font_family
		if cfg.font_family and cfg.font_family ~= '' then
			formatting = formatting .. " font_family='" .. cfg.font_family .. "'"
		end

		-- add font_size
		if cfg.font_size and cfg.font_size ~= '' then
			local font_size = cfg.font_size
			if type(font_size) == 'number' then
				if font_size > 1024 then
					wesnoth.deprecated_message("font_size units", 3, '1.16', "Specifying [message]font_size= in 1/1024ths of a point is deprecated. Specify it as points instead.")
				else
					-- Pango expects 1/1024ths of a point here
					font_size = font_size * 1024
				end
			end
			formatting = formatting .. " font_size='" .. font_size .. "'"
		end

		-- font_style
		if cfg.font_style and cfg.font_style ~= '' then
			formatting = formatting .. " font_style='" .. cfg.font_style .. "'"
		end

		-- font_weight
		if cfg.font_weight and cfg.font_weight ~= '' then
			formatting = formatting .. " font_weight='" .. cfg.font_weight .. "'"
		end

		-- font_variant
		if cfg.font_variant and cfg.font_variant ~= '' then
			formatting = formatting .. " font_variant='" .. cfg.font_variant .. "'"
		end

		-- font_stretch
		if cfg.font_stretch and cfg.font_stretch ~= '' then
			formatting = formatting .. " font_stretch='" .. cfg.font_stretch .. "'"
		end

		-- add color
		if cfg.color and cfg.color ~= '' then
			formatting = formatting .. " color='" .. get_pango_color(cfg.color) .. "'"
		end

		-- bgcolor
		if cfg.bgcolor and cfg.bgcolor ~= '' then
			formatting = formatting .. " bgcolor='" .. get_pango_color(cfg.bgcolor) .. "'"
		end

		-- underline
		if cfg.underline and cfg.underline ~= '' then
			local underline
			if cfg.underline == true then
				underline = 'single'
			elseif cfg.underline == false then
				underline = 'none'
			else
				underline = cfg.underline
			end
			formatting = formatting .. " underline='" .. underline .. "'"
		end

		-- underline_color
		if cfg.underline_color and cfg.underline_color ~= '' then
			formatting = formatting .. " underline_color='" .. get_pango_color(cfg.underline_color) .. "'"
		end

		-- rise
		if cfg.rise and cfg.rise ~= '' then
			local rise = cfg.rise
			if type(rise) == 'number' then
				if rise > 1000 then
					wesnoth.deprecated_message("rise units", 3, '1.16', "Specifying [message]rise= in 1/10,000ths of an em is deprecated. Specify it as ems instead.")
				else
					-- Pango expects 1/10000ths of an em here
					rise = rise * 10000
				end
			end
			formatting = formatting .. " rise='" .. rise .. "'"
		end

		-- strikethrough
		if cfg.strikethrough and tostring(cfg.strikethrough) ~= '' then
			formatting = formatting .. " strikethrough='" .. tostring(cfg.strikethrough) .. "'"
		end

		-- strikethrough_color
		if cfg.strikethrough_color and cfg.strikethrough_color ~= '' then
			formatting = formatting .. " strikethrough_color='" .. get_pango_color(cfg.strikethrough_color) .. "'"
		end

		-- fallback
		if cfg.fallback and tostring(cfg.fallback) ~= '' then
			formatting = formatting .. " fallback='" .. tostring(cfg.fallback) .. "'"
		end

		-- letter_spacing
		if cfg.letter_spacing and cfg.letter_spacing ~= '' then
			local letter_spacing = cfg.letter_spacing
			if type(letter_spacing) == 'number' then
				if letter_spacing > 1024 then
					wesnoth.deprecated_message("letter_spacing units", 3, '1.16', "Specifying [message]letter_spacing= in 1/1024ths of a point is deprecated. Specify it as points instead.")
				else
					-- Pango expects 1/1024ths of a point here
					letter_spacing = letter_spacing * 1024
				end
			end
			formatting = formatting .. " letter_spacing='" .. letter_spacing .. "'"
		end

		-- gravity
		if cfg.gravity and cfg.gravity ~= '' then
			formatting = formatting .. " gravity='" .. cfg.gravity .. "'"
		end

		-- gravity_hint
		if cfg.gravity_hint and cfg.gravity_hint ~= '' then
			formatting = formatting .. " gravity_hint='" .. cfg.gravity_hint .. "'"
		end

		-- wrap in span tags and return if a color was added
		if formatting ~= "<span" then
			return formatting .. ">" .. text .. "</span>"
		end
	end

	-- or return unmodified message
	return text
end

local function get_speaker(cfg)
	local speaker
	local context = wesnoth.current.event_context

	if cfg.speaker == "narrator" then
		speaker = "narrator"
	elseif cfg.speaker == "unit" then
		speaker = wesnoth.units.get(context.x1 or 0, context.y1 or 0)
	elseif cfg.speaker == "second_unit" then
		speaker = wesnoth.units.get(context.x2 or 0, context.y2 or 0)
	elseif cfg.speaker ~= nil then
		speaker = wesnoth.units.get(cfg.speaker)
	else
		speaker = wesnoth.units.find_on_map(cfg)[1]
	end

	return speaker
end

local function message_user_choice(cfg, speaker, options, text_input, sound)
	local image, left_side = get_image(cfg, speaker)
	local caption = get_caption(cfg, speaker)

	local msg_cfg = {
		left_side = left_side,
		title = caption,
		message = cfg.message,
		portrait = image,
		mirror = cfg.mirror,
		second_portrait = cfg.second_image,
		second_mirror = cfg.second_mirror,
	}

	local voice = cfg.voice
	if speaker ~= nil then
		if cfg.male_message ~= nil and speaker.gender == "male" then
			msg_cfg.message = cfg.male_message
		elseif cfg.female_message ~= nil and speaker.gender == "female" then
			msg_cfg.message = cfg.female_message
		end
		if cfg.male_voice ~= nil and speaker.gender == "male" then
			voice = cfg.male_voice
		elseif cfg.female_voice ~= nil and speaker.gender == "female" then
			voice = cfg.female_voice
		end
	end

	-- add formatting
	msg_cfg.message = add_formatting(cfg, msg_cfg.message)

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
		if sound then wesnoth.audio.play(sound) end
		if voice then
			local speech = {
				sounds = voice,
				loops = 0,
				delay = 0,
			}
			if speaker then
				speech.x = speaker.x
				speech.y = speaker.y
			end
			wesnoth.audio.sources.wml_message_speaker = speech
		end

		local option_chosen, ti_content = gui.show_narration(msg_cfg, options, text_input)

		if voice then
			wesnoth.audio.sources.wml_message_speaker = nil
		end

		if option_chosen == -2 then -- Pressed Escape (only if no input)
			wesnoth.interface.skip_messages()
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
	local show_if = wml.get_child(cfg, "show_if") or {}
	if not wml.eval_conditional(show_if) then
		log("[message] skipped because [show_if] did not pass", "debug")
		return
	end

	-- Only the first text_input tag is considered
	local text_input
	for text_input_cfg in wml.child_range(cfg, "text_input") do
		if text_input ~= nil then
			log("Too many [text_input] tags, only first one accepted", "warning")
			break
		end
		text_input = text_input_cfg
	end

	local options, option_events = {}, {}
	for option in wml.child_range(cfg, "option") do
		local condition = wml.get_child(option, "show_if") or {}
		if wml.eval_conditional(condition) then

			-- make message= and description= equivalent for the sake of backwards compatibility
			local msg_text = ""
			if option.message and option.description then
				log("[option] uses both message= and description= which is invalid.", "warning")
				msg_text = "Invalid use of both message and description attributes on this option!"
			elseif option.message then
				msg_text = tostring(option.message)
			elseif option.description then
				msg_text = tostring(option.description)
			end

			local opt = {
				label = option.label,
				description = msg_text,
				image = option.image,
				default = option.default,
				value = option.value
			}

			table.insert(options, opt)
			table.insert(option_events, {})

			for cmd in wml.child_range(option, "command") do
				table.insert(option_events[#option_events], cmd)
			end
		end
	end

	-- Check if there is any input to be made, if not the message may be skipped
	local has_input = text_input ~= nil or #options > 0

	if not has_input and wesnoth.interface.is_skipping_messages() then
		-- No input to get and the user is not interested either
		log("Skipping [message] because user not interested", "debug")
		return
	end

	local sides_for = cfg.side_for
	if sides_for and not has_input then
		local show_for_side = false

		-- Sanity checks on side number and controller
		for _,side in ipairs(tostring(sides_for):split()) do
			side = tonumber(side)
			if side > 0 and side <= #wesnoth.sides
				and wesnoth.sides[side].controller == "human"
				and wesnoth.sides[side].is_local
			then
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
	elseif cfg.highlight == false then
		-- Nothing to do here
		log("Nothing to highlight for [message]", "debug")
	elseif speaker == "narrator" then
		-- Narrator, so deselect units
		wesnoth.interface.deselect_hex()
		-- The speaker is expected to be either nil or a unit later
		speaker = nil
		wml.fire("redraw")
	else
		-- Check ~= false, because the default if omitted should be true
		if cfg.scroll ~= false then
			wesnoth.interface.scroll_to_hex(speaker.x, speaker.y, true, false, true)
		end

		wesnoth.interface.highlight_hex(speaker.x, speaker.y)
		wml.fire("redraw")
	end

	local msg_dlg = message_user_choice(cfg, speaker, options, text_input, cfg.sound)

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
		local choice = wesnoth.sync.evaluate_single(wait_description, msg_dlg, sides_for)

		option_chosen = tonumber(choice.value)

		if text_input ~= nil then
			-- Implement the consequences of the choice
			wml.variables[text_input.variable or "input"] = choice.text
		end
	end

	-- Unhilight the speaker
	if speaker and cfg.highlight == true then
		wesnoth.interface.deselect_hex()
	end

	if #options > 0 then
		if option_chosen > #options then
			log("invalid choice (" .. option_chosen .. ") was specified, choice 1 to " ..
				#options .. " was expected", "debug")
			return
		end

		if cfg.variable ~= nil then
			if options[option_chosen].value == nil then
				wml.variables[cfg.variable] = option_chosen
			else
				wml.variables[cfg.variable] = options[option_chosen].value
			end
		end

		for i, cmd in ipairs(option_events[option_chosen]) do
			local action = utils.handle_event_commands(cmd, "plain")
			if action ~= "none" then break end
		end
	end
end
