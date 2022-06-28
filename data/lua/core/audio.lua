
if wesnoth.kernel_type() == "Game Lua Kernel" then
	-- Only deprecation stubs for now
	wesnoth.play_sound = wesnoth.deprecate_api('wesnoth.play_sound', 'wesnoth.audio.play', 1, nil, wesnoth.audio.play)
	wesnoth.sound_volume = wesnoth.deprecate_api('wesnoth.sound_volume', 'wesnoth.audio.volume', 1, nil, function(volume)
		local old_volume = wesnoth.audio.volume
		if type(volume) == 'number' then
			wesnoth.audio.volume = volume
		end
		return old_volume
	end)
	wesnoth.add_sound_source = wesnoth.deprecate_api('wesnoth.add_sound_source', 'wesnoth.audio.sources', 1, nil, function(cfg)
		wesnoth.audio.sources[cfg.id] = cfg
	end, 'Assign a config to the sound source ID')
	wesnoth.get_sound_source = wesnoth.deprecate_api('wesnoth.get_sound_source', 'wesnoth.audio.sources', 1, nil, function(id)
		return wesnoth.audio.sources[id]
	end, 'Index by the sound source ID')
	wesnoth.remove_sound_source = wesnoth.deprecate_api('wesnoth.remove_sound_source', 'wesnoth.audio.sources', 1, nil, function(id)
		wesnoth.audio.sources[id] = nil
	end, 'Assign nil to the sound source ID')
	wesnoth.music_list = wesnoth.deprecate_api('wesnoth.music_list', 'wesnoth.audio.music_list', 1, nil, setmetatable({}, {
		__len = function() return #wesnoth.audio.music_list end,
		__index = function(self, key) return wesnoth.audio.music_list[key] end,
		__newindex = function(self, key, value) wesnoth.audio.music_list[key] = value end,
	}))
	-- wesnoth.wml_actions.music doesn't exist yet at this point, so create a helper function instead.
	wesnoth.set_music = wesnoth.deprecate_api('wesnoth.set_music', 'wesnoth.audio.music_list', 1, nil, function(cfg)
		wesnoth.wml_actions.music(cfg)
	end)
end
