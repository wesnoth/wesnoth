
--[========[File Handling]========]
print("Loading filesystem module...")

---@enum asset_type
---Valid asset types, used as the type argument for have_asset and resolve_asset
filesystem.asset_type = {
    IMAGE = 'images',
    SOUND = 'sounds',
    MUSIC = 'music',
    MAP = 'maps',
    SCENARIO = 'scenarios',
}

wesnoth.have_file = wesnoth.deprecate_api('wesnoth.have_file', 'filesystem.have_file', 1, nil, filesystem.have_file)
wesnoth.read_file = wesnoth.deprecate_api('wesnoth.read_file', 'filesystem.read_file', 1, nil, filesystem.read_file)
wesnoth.canonical_path = wesnoth.deprecate_api('wesnoth.canonical_path', 'filesystem.canonical_path', 1, nil, filesystem.canonical_path)
wesnoth.get_image_size = wesnoth.deprecate_api('wesnoth.get_image_size', 'filesystem.image_size', 1, nil, filesystem.image_size)
