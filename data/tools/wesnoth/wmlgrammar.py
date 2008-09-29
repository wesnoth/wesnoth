"""
This file is used to store the grammar of WML for wmltest.
"""
grammar = {
'WML' : (
    [ 'textdomain', 'game_config', 'about' ],
    [] ),
'textdomain' : (
    [],
    [ 'name' ]),
'game_config' : (
    [ 'server', 'color_range', 'color_palette' ],
    [ 'wesnothd_name', 'base_income', 'village_income', 'poison_amount', 'rest_heal_amount', 'recall_cost', 'kill_experience', 'lobby_refresh', 'icon', 'title', 'logo', 'title_music', 'lobby_music', 'logo_x', 'logo_y', 'buttons_x', 'buttons_y', 'buttons_padding', 'tip_x', 'tip_width', 'tip_padding', 'energy_image', 'moved_ball_image', 'unmoved_ball_image', 'partmoved_ball_image', 'enemy_ball_image', 'ally_ball_image', 'flag_image', 'flag_icon_image', 'cross_image', 'hp_bar_scaling', 'xp_bar_scaling', 'lobby_refresh', 'footprint_prefix', 'footprint_teleport_enter', 'footprint_teleport_exit', 'terrain_mask_image', 'grid_image', 'unreachable_image', 'observer_image', 'tod_bright_image', 'level_image', 'ellipsis_image', 'default_victory_music', 'default_defeat_music', 'defense_color_scale', 'flag_rgb' ]),
'color_palette' : (
    [],
    [ 'magenta', 'flag_green', 'ellipse_red' ]),
'about' : (
    [ 'entry' ],
    [ 'images', 'title' ]),
'entry' : (
    [],
    [ 'name', 'comment', 'wikiuser', 'email', 'ircuser' ]),
'server' : (
    [],
    [ 'name', 'address' ]),
'color_range' : (
    [],
    [ 'id', 'rgb', 'name' ]),
'color_palette' : (
    [],
    [ 'magenta', 'flag_green', 'ellipse_red' ]),

}

