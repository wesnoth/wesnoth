"""
This file is used to store the grammar of WML for wmltest.
Format: The grammar is a dictionary mapping every recognised tag
 plus the pseudo-tag WML to a tuple.
This tuple consists of the following:
-A list of all subtags recognised in the tag.
-A list of all keys recognised in the tag.

Instead of a tuple, it can instead contain a single string,
 which points to the tag (dictionary key) whose contents should be used.
"""
class Grammar:
    _grammar = {
# This is the top-level pseudo-tag that everything is a child of.
# It should have no keys
'WML' : (
    [ 'about', 'advanced_preference', 'binary_path', 'campaign', 'fonts', 'game_config', 'help', 'hotkey', 'terrain', 'terrain_graphics', 'textdomain', 'theme', 'units', ],
    []),
# Attempt to keep everything alphabetically ordered
'about' : (
    [ 'entry', ],
    [ 'images', 'text', 'title', ]),
'add' : (
    'resolution',
    [ ]),
'advanced_preference' : (
    [],
    [ 'default', 'field', 'name', ]),
'binary_path' : (
    [],
    [ 'path', ]),
'campaign' : (
    [ 'about', ],
    [ 'abbrev', 'define', 'description', 'difficulties', 'difficulty_descriptions', 'extra_defines', 'first_scenario', 'icon', 'id', 'image', 'name', 'rank', ]),
'change' : (
    [],
    [ 'font_size', 'id', 'image', 'items', 'rect', 'ref', ]), #TODO: unfinished
'color_palette' : (
    [],
    [ 'ellipse_red', 'flag_green', 'magenta', ]),
'color_range' : (
    [],
    [ 'id', 'name', 'rgb', ]),
'entry' : (
    [],
    [ 'comment', 'email', 'ircuser', 'name', 'wikiuser', ]),
'fonts' : (
    [ 'font', ],
    [ 'order', ]),
'font' : (
    [],
    [ 'codepoints', 'name', ]),
'game_config' : (
    [ 'color_palette', 'color_range', 'server', ],
    [ 'ally_ball_image', 'base_income', 'buttons_x', 'buttons_y', 'buttons_padding', 'cross_image', 'default_defeat_music', 'default_victory_music', 'defense_color_scale', 'ellipsis_image', 'enemy_ball_image', 'energy_image', 'flag_icon_image', 'flag_image', 'flag_rgb', 'footprint_prefix', 'footprint_teleport_enter', 'footprint_teleport_exit', 'grid_image', 'hp_bar_scaling', 'icon', 'kill_experience', 'level_image', 'lobby_music', 'lobby_refresh', 'logo', 'logo_x', 'logo_y', 'moved_ball_image', 'observer_image', 'partmoved_ball_image', 'poison_amount', 'recall_cost', 'rest_heal_amount', 'terrain_mask_image', 'tip_padding', 'tip_width', 'tip_x', 'title', 'title_music', 'tod_bright_image', 'unmoved_ball_image', 'unreachable_image', 'village_income', 'wesnothd_name', 'xp_bar_scaling', ]),
'help' : (
    [ 'section', 'topic', 'toplevel' ],
    []),
'hotkey' : (
    [],
    [ 'alt', 'cmd', 'command', 'ctrl', 'key', 'shift', ]),
'image' : (
    [],
    [ 'base', 'center', 'layer', 'name', ]),
'label' : (
    [],
    [ 'font_rgb', 'font_size', 'icon', 'id', 'image', 'text', 'rect', 'ref', 'xanchor', 'yanchor', ]),
'main_map' : (
    [],
    'panel'),
'main_map_border' : (
    [],
    [ 'background_image', 'border_image_bottom_even', 'border_image_bottom_odd', 'border_image_left', 'border_image_right', 'border_image_top_even', 'border_image_top_odd', 'border_size', 'corner_image_bottom_left', 'corner_image_bottom_right_even', 'corner_image_bottom_right_odd', 'corner_image_top_left','corner_image_top_right_even',  'corner_image_top_right_odd', 'tile_image', ]),
'menu' : (
    [],
    [ 'auto_tooltip', 'id', 'image', 'is_context_menu', 'items', 'title', 'title2', 'tooltip', 'type', 'rect', 'ref', 'xanchor', 'yanchor', ]),
'mini_map' : (
    [],
    'panel'),
'movetype' : (
    [], #TODO: unfinished (resistance, defense, movement_costs)
    [ 'flies', 'name', ]),
'panel' : (
    [],
    [ 'id', 'image', 'rect', 'ref', 'xanchor', 'yanchor', ]),
'partialresolution' : (
    [ 'add', 'change', 'remove', ],
    [ 'height', 'id', 'inherits', 'width', ]),
'race' : (
    [ 'trait', ],
    [ 'description', 'female_name', 'female_names', 'id', 'ignore_global_traits', 'male_name', 'male_names', 'markov_chain_size', 'name', 'num_traits', 'plural_name', ]),
'remove' : (
    [],
    [ 'id', ]),
'replay' : (
    'partialresolution',
    []),
'resolution' : (
    [ 'label', 'main_map', 'main_map_border', 'menu', 'mini_map', 'panel', 'replay', 'screen', ], #TODO: unfinished
    [ 'height', 'id', 'width', ]),
'screen' : (
    [],
    [ 'id', 'rect' ]),
'section' : (
    [],
    [ 'generator', 'id', 'sections_generator', 'sort_sections', 'sort_topics', 'title', 'topics', ]),
'server' : (
    [],
    [ 'address', 'name', ]),
'terrain' : (
    [],
    [ 'aliasof', 'default_base', 'def_alias', 'editor_group', 'editor_image', 'gives_income', 'heals', 'hidden', 'id', 'light', 'mvt_alias', 'name', 'recruit_from', 'recruit_onto', 'string', 'submerge', 'symbol_image', 'unit_height_adjust', ]),
'terrain_graphics' : (
    [ 'image', 'tile', ],
    [ 'map', 'no_flag', 'precedence', 'probability', 'rotations', 'set_flag', ]),
'textdomain' : (
    [],
    [ 'name', ]),
'theme' : (
    [ 'partialresolution', 'resolution', ],
    [ 'name', ]),
'tile' : (
    [ 'image', ],
    [ 'no_flag', 'pos', 'set_flag', 'type', 'x', 'y', ]),
'topic' : (
    [],
    [ 'generator', 'id', 'text', 'title', ]),
'toplevel' : (
    [],
    [ 'sections', 'topics', ]),
'trait' : (
    [],#TODO: UNFINISHED (effect)
    [ 'availability', 'description', 'female_name', 'id', 'male_name', 'name', ]),
'units' : (
    [ 'movetype', 'race', 'trait', ], #TODO: unfinished (unit_type)
    []),
}
    def grammar(self):
        out = {}
        for key in self._grammar.keys():
            out.update( { key : self._grammar[key] } )
            while isinstance(out[key], str):
                out.update( { key :
                    self._grammar[
                        out[key]
                    ]
                } )
            while isinstance(out[key][0], str):
                out.update( { key :
                    (
                        self._grammar[
                            out[key][0]
                        ][0],
                        out[key][1]
                    )
                } )
            while isinstance(out[key][1], str):
                out.update( { key :
                    (
                        out[key][0],
                        self._grammar[
                            out[key][1]
                        ][1]
                    )
                } )
        return out
