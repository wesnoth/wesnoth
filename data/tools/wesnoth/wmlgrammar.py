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
    [ 'about', 'game_config', 'help', 'textdomain', ],
    []),
# Attempt to keep everything alphabetically ordered
'about' : (
    [ 'entry', ],
    [ 'images', 'title', ]),
'color_palette' : (
    [],
    [ 'ellipse_red', 'flag_green', 'magenta', ]),
'color_range' : (
    [],
    [ 'id', 'name', 'rgb', ]),
'entry' : (
    [],
    [ 'comment', 'email', 'ircuser', 'name', 'wikiuser', ]),
'game_config' : (
    [ 'color_palette', 'color_range', 'server', ],
    [ 'ally_ball_image', 'base_income', 'buttons_x', 'buttons_y', 'buttons_padding', 'cross_image', 'default_defeat_music', 'default_victory_music', 'defense_color_scale', 'ellipsis_image', 'enemy_ball_image', 'energy_image', 'flag_icon_image', 'flag_image', 'flag_rgb', 'footprint_prefix', 'footprint_teleport_enter', 'footprint_teleport_exit', 'grid_image', 'hp_bar_scaling', 'icon', 'kill_experience', 'level_image', 'lobby_music', 'lobby_refresh', 'logo', 'logo_x', 'logo_y', 'moved_ball_image', 'observer_image', 'partmoved_ball_image', 'poison_amount', 'recall_cost', 'rest_heal_amount', 'terrain_mask_image', 'tip_padding', 'tip_width', 'tip_x', 'title', 'title_music', 'tod_bright_image', 'unmoved_ball_image', 'unreachable_image', 'village_income', 'wesnothd_name', 'xp_bar_scaling', ]),
'help' : (
    [ 'section', 'topic', 'toplevel' ],
    []),
'section' : (
    [],
    [ 'generator', 'id', 'sections_generator', 'sort_sections', 'sort_topics', 'title', 'topics', ]),
'server' : (
    [],
    [ 'address', 'name', ]),
'textdomain' : (
    [],
    [ 'name', ]),
'topic' : (
    [],
    [ 'generator', 'id', 'text', 'title', ]),
'toplevel' : (
    [],
    [ 'sections', 'topics', ]),
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
