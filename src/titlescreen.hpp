#ifndef TITLE_HPP_INCLUDED
#define TITLE_HPP_INCLUDED

class config;
class display;

namespace gui {

enum TITLE_RESULT { TUTORIAL = 0, NEW_CAMPAIGN, MULTIPLAYER, LOAD_GAME,
                    CHANGE_LANGUAGE, EDIT_PREFERENCES, SHOW_ABOUT, QUIT_GAME, TITLE_CONTINUE };

TITLE_RESULT show_title(display& screen, config& tips_of_day, int* ntip);

}

#endif
