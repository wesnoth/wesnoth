"""
This file is used to store the grammar of WML for wmltest.

Grammar of the grammar:
<GRAMMAR>           -> { <GRAMMAR_CONTENTS> }
<GRAMMAR_CONTENTS>  -> <TAG_IDENTIFIER> : <TAG_CONTENTS>, <GRAMMAR_CONTENTS> | <EMPTY>
# EMPTY AKA the emtpy string
<EMPTY>             -> 
# TAG_IDENTIFIER doesn't have to be a tag name, it can be an alias or a psuedo-tag (WML)
<TAG_IDENTIFIER>    -> str
# TAG_CONTENTS either contains a pair, or points to the pair of another tag.
<TAG_CONTENTS>      -> (<ALLOWED_TAGS>, <ALLOWED_KEYS>) | <TAG_IDENTIFIER> | <TAG_PLUS>
# ALLOWED_TAGS either contains a list of tags, or points to the list of another tag.
<ALLOWED_TAGS>      -> [ <TAGS> ] | <TAG_IDENTIFIER> | <TAG_PLUS>
<TAGS>              -> <TAG>, <TAGS> | <EMPTY>
# TAG is a string, a dict of two strings or a dict of a regex to a string
<TAG>               -> <TAG_IDENTIFIER> | { str : <TAG_IDENTIFIER> } | { re._pattern_type : <TAG_IDENTIFIER> }
# ALLOWED_KEYS either contains a list of keys, or points to the list of another tag.
<ALLOWED_KEYS>      -> [ <KEYS> ] | <TAG_IDENTIFIER> | <TAG_PLUS>
<KEYS>              -> <KEY>, <KEYS> | <EMPTY>
# KEY is either a string or a regular expression
<KEY>               -> str | re._pattern_type
# TAG_PLUS is a class that refers to a TAG_IDENTIFIER and adds more TAGS and/or KEYS
<TAG_PLUS>          -> TagPlus(<TAG_IDENTIFIER>, (<ALLOWED_TAGS>, <ALLOWED_KEYS>), None) |
                        TagPlus(<TAG_IDENTIFIER>, <ALLOWED_TAGS>, 0) |
                        TagPlus(<TAG_IDENTIFIER>, <ALLOWED_KEYS>, 1)
"""
class TagPlus:
    """
    This class exists so tags can refer to eachother AND add new things.
    """
    def __init__(self, tag, list, part=None):
        self.tag = tag
        self.list = list
        self.part = part
    def process(self, grammar):
        content = grammar[self.tag]
        if self.part is None:
            content[0] += self.list[0]
            content[1] += self.list[1]
        else:
            content = content[self.part]
            content += self.list
        return content

class Grammar:
    import re
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
'defense' : 'movement_costs',
'editor2_tool_hint' : 'gold-theme',
'effect' : (
    [ 'defense', 'movement_costs', 'resistance', ], #TODO: point tags to where they should go (specials, contents of filter_attack, { set_specials : specials }, abilities)
    [ 'apply_to', 'times', 'unit_type', re.compile("\w+"), ]),
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
'gold-theme' : (
    [],
    [ 'font_rgb', 'font_size', 'id', 'prefix', 'prefix_literal', 'rect', 'ref', 'xanchor', 'yanchor', ]),
'help' : (
    [ 'section', 'topic', 'toplevel' ],
    []),
'hotkey' : (
    [],
    [ 'alt', 'cmd', 'command', 'ctrl', 'key', 'shift', ]),
'income' : 'gold-theme',
'image' : (
    [],
    [ 'base', 'center', 'layer', 'name', ]),
'label-theme' : (
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
'movement_costs' : (
    [],
    [ re.compile('\w+'), ]),
'movetype' : (
    [ 'defense', 'movement_costs', 'resistance', ],
    [ 'flies', 'name', ]),
'num_units' : 'gold-theme',
'observers' : 'gold-theme',
'panel' : (
    [],
    [ 'id', 'image', 'rect', 'ref', 'xanchor', 'yanchor', ]),
'partialresolution' : (
    [ 'add', 'change', 'remove', ],
    [ 'height', 'id', 'inherits', 'width', ]),
'position' : 'gold-theme',
'race' : (
    [ 'trait', ],
    [ 'description', 'female_name', 'female_names', 'id', 'ignore_global_traits', 'male_name', 'male_names', 'markov_chain_size', 'name', 'num_traits', 'plural_name', ]),
'remove' : (
    [],
    [ 'id', ]),
'replay' : (
    'partialresolution',
    []),
'report_clock' : 'gold-theme',
'report_countdown' : 'gold-theme',
'resistance' : (
    [],
    [ re.compile('\w+'), ]),
'resolution' : (
    [ { 'label' : 'label-theme' }, 'main_map', 'main_map_border', 'menu', 'mini_map', 'panel', 'replay', 'screen', 'status', ], 
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
'side_playing' : (
    [],
    [ 'id', 'rect', 'ref', 'xanchor', 'yanchor', ]),
'status' : (
    [ 'editor2_tool_hint', { 'gold' : 'gold-theme' }, 'income', 'num_units', 'observers', 'panel', 'position', 'report_clock', 'report_countdown', 'side_playing', { 'terrain' : 'terrain-theme' }, 'time_of_day', 'turn', 'unit_abilities', 'unit_advancement_options', 'unit_alignment', 'unit_amla', 'unit_hp', 'unit_image', 'unit_level', 'unit_moves', 'unit_name', 'unit_race', 'unit_side', 'unit_status', 'unit_traits', { 'unit_type' : 'unit_type-theme' } , 'unit_weapons', 'unit_xp', 'upkeep', 'villages', ],
    []),
'terrain' : (
    [],
    [ 'aliasof', 'default_base', 'def_alias', 'editor_group', 'editor_image', 'gives_income', 'heals', 'hidden', 'id', 'light', 'mvt_alias', 'name', 'recruit_from', 'recruit_onto', 'string', 'submerge', 'symbol_image', 'unit_height_adjust', ]),
'terrain-theme' : 'gold-theme',
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
'time_of_day' : 'side_playing',
'topic' : (
    [],
    [ 'generator', 'id', 'text', 'title', ]),
'toplevel' : (
    [],
    [ 'sections', 'topics', ]),
'trait' : (
    [ 'effect', ],
    [ 'availability', 'description', 'female_name', 'id', 'male_name', 'name', ]),
'turn' : 'gold-theme',
'unit_abilities' : 'gold-theme',
'unit_advancement_options' : 'gold-theme',
'unit_alignment' : 'gold-theme',
'unit_amla' : 'gold-theme',
'unit_hp' : 'gold-theme',
'unit_image' : 'gold-theme',
'unit_level' : 'gold-theme',
'unit_moves' : 'gold-theme',
'unit_name' : 'gold-theme',
'unit_race' : 'gold-theme',
'unit_side' : 'gold-theme',
'unit_status' : 'gold-theme',
'unit_traits' : 'gold-theme',
'unit_type' : (
    [ { 'female' : 'unit_type' }, { 'male' : 'unit_type' }, 'variation', ],
    [ 'advances_to', 'alignment', 'cost', 'description', 'die_sound', 'ellipse', 'experience', 'flag_rgb', 'gender', 'halo', 'hide_help', 'hitpoints', 'id', 'image', 'level', 'movement', 'movement_type', 'name', 'profile', 'race', 'undead_variation', 'usage', ]),
'unit_type-theme' : 'gold-theme',
'unit_weapons' : 'gold-theme',
'unit_xp' : 'gold-theme',
'units' : (
    [ 'movetype', 'race', 'trait', 'unit_type' ],
    []),
'variation' : (
    'unit_type',
    TagPlus('unit_type', ['variation_name',], 1) ),
'villages' : 'gold-theme',
'upkeep' : 'gold-theme',
}

    def grammar(self):
        """Grammar pre-processor.

        This function is responsible for turning all the 'pointers' into actual data.
        It only dereferences once, this allows for cleaner code and prevents infinite loops."""
        out = {}
        for key in self._grammar.keys():
            out.update( { key : self._grammar[key] } )
            # First layer of dereferencing: the tags themselves
            if isinstance(out[key], str):
                out.update( { key :
                    self._grammar[
                        out[key]
                    ]
                } )
            elif isinstance(out[key], TagPlus):
                out.update( { key :
                    out[key].process(self._grammar)
                } )
            # Second layer: the contained tags and keys
            # First the tags
            if isinstance(out[key][0], str):
                out.update( { key :
                    (
                        self._grammar[
                            out[key][0]
                        ][0],
                        out[key][1]
                    )
                } )
            elif isinstance(out[key][0], TagPlus):
                out.update( { key :
                    (
                        out[key][0].process(self._grammar),
                        out[key][1]
                    )
                } )
            # Then the keys
            if isinstance(out[key][1], str):
                out.update( { key :
                    (
                        out[key][0],
                        self._grammar[
                            out[key][1]
                        ][1]
                    )
                } )
            elif isinstance(out[key][1], TagPlus):
                out.update( { key :
                    (
                        out[key][0],
                        out[key][1].process(self._grammar)
                    )
                } )
        return out

# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:
