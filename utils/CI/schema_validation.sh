#!/bin/bash

shopt -s globstar nullglob

# bash sets TERM to dumb by default but does not export it
compgen -e -X '!TERM' || export TERM=xterm # hopefully a good approximation of what github supports
reset=$(tput sgr0)
red=$(tput setaf 1)
green=$(tput setaf 2)
blue=$(tput bold; tput setaf 4)

print() { printf '%s%s%s\n' "$blue" "$*" "$reset"; }
error() { printf '%s%s%s\n' "$red" "$*" "$reset"; }
success() { printf '%s%s%s\n' "$green" "$*" "$reset"; }

validate() {
    local name="$1"
    shift

    echo "------"
    print "Validating $name..."

    if "$@" > temp.log 2>&1; then
        rm temp.log
        success "$name validation complete!  Success: Yes"
        echo "------"
        return 0
    else
        error "$name failed validation!"
        cat temp.log
        rm temp.log
        error "$name validation complete!  Success: No"
        echo "------"
        return 1
    fi
}

validate_core() { validate "$1" ./wesnoth --validate data/_main.cfg; }
validate_misc() { validate "$1" ./wesnoth --data-dir=. --validate=data/_main.cfg --preprocess-defines="$2"; }
validate_achievements() { validate "Achievements" ./wesnoth --data-dir=. --validate=data/achievements.cfg --use-schema=data/schema/achievements.cfg; }
validate_dialog() { validate "$1 dialog $(basename "$2" .cfg)" ./wesnoth --data-dir=. --validate="$2" --use-schema=data/schema/gui_window.cfg; }
validate_schema() { validate "schema $1" ./wesnoth --data-dir=. --validate-schema=data/schema/"$2".cfg; }

validate_campaign() {
    local success=Yes name="$1" define="$2"
    shift 2

    echo "------"
    print "Validating $name..."

    for difficulty in "$@"; do
        if [ "$success" = "Yes" ]; then
            print "Validating $difficulty..."
            if ! ./wesnoth --data-dir=. --validate=data/_main.cfg --preprocess-defines="$define,$difficulty" > temp.log 2>&1; then
                success=No
                error "$name failed $difficulty validation!"
                cat temp.log
            fi

            rm temp.log
        else
            echo "Skipping $difficulty validation"
        fi
    done

    if [ "$success" = "Yes" ]; then
        success "$name validation complete!  Success: Yes"
        echo "------"
    else
        error "$name validation complete!  Success: No"
        echo "------"
    fi

    for gui in data/campaigns/"$name"/**/gui/*.cfg; do
        validate_dialog "$name" "$gui" || success=No
    done

    [ "$success" = "Yes" ]
}

RET=0

# remove any_tag to actually see errors in action WML
patch -p 1 < utils/CI/schema_validation.patch || RET=1

validate_schema "WML Schema"   "schema"       || RET=1
validate_schema "Game Config"  "game_config"  || RET=1
validate_schema "GUI2"         "gui"          || RET=1
validate_schema "GUI2/Lua"     "gui_window"   || RET=1
validate_schema "Server Pbl"   "pbl"          || RET=1
validate_schema "WML Diff"     "diff"         || RET=1
validate_schema "Achievements" "achievements" || RET=1
validate_schema "Fonts"        "fonts"        || RET=1
validate_schema "Languages"    "languages"    || RET=1

validate_core "Core" || RET=1

validate_achievements || RET=1
for gui in data/modifications/**/gui/*.cfg; do
    name=${gui#"data/modifications/"}
    name=${name%%/*}
    validate_dialog "modification $name" "$gui" || RET=1
done

validate "Fonts" ./wesnoth --validate=data/hardwired/fonts.cfg --use-schema=data/schema/fonts.cfg
validate "Languages" ./wesnoth --validate=data/hardwired/language.cfg --use-schema=data/schema/languages.cfg

validate_misc "Editor"         "EDITOR"                                  || RET=1
validate_misc "Multiplayer"    "MULTIPLAYER,MULTIPLAYER_A_NEW_LAND_LOAD" || RET=1
validate_misc "Test"           "TEST"                                    || RET=1
validate_misc "World_Conquest" "MULTIPLAYER,LOAD_WC2,LOAD_WC2_EVEN_THOUGH_IT_NEEDS_A_NEW_MAINTAINER" || RET=1

validate_campaign "Dead_Water"              "CAMPAIGN_DEAD_WATER"              "EASY" "NORMAL" "HARD" "NIGHTMARE" || RET=1
validate_campaign "Descent_Into_Darkness"   "CAMPAIGN_DESCENT"                 "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Eastern_Invasion"        "CAMPAIGN_EASTERN_INVASION"        "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Heir_To_The_Throne"      "CAMPAIGN_HEIR_TO_THE_THRONE"      "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Legend_of_Wesmere"       "CAMPAIGN_LOW"                     "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Liberty"                 "CAMPAIGN_LIBERTY"                 "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Northern_Rebirth"        "CAMPAIGN_NORTHERN_REBIRTH"        "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Sceptre_of_Fire"         "CAMPAIGN_SCEPTRE_FIRE"            "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Secrets_of_the_Ancients" "CAMPAIGN_SECRETS_OF_THE_ANCIENTS" "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Son_Of_The_Black_Eye"    "CAMPAIGN_SON_OF_THE_BLACK_EYE"    "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "The_Deceivers_Gambit"    "CAMPAIGN_THE_DECEIVERS_GAMBIT"    "EASY" "NORMAL" "HARD" "NIGHTMARE" || RET=1
validate_campaign "The_Hammer_of_Thursagan" "CAMPAIGN_THE_HAMMER_OF_THURSAGAN" "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "The_Rise_Of_Wesnoth"     "CAMPAIGN_THE_RISE_OF_WESNOTH"     "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "The_South_Guard"         "CAMPAIGN_THE_SOUTH_GUARD"         "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "tutorial"                "TUTORIAL"                         "EASY"                             || RET=1
validate_campaign "Two_Brothers"            "CAMPAIGN_TWO_BROTHERS"            "EASY" "HARD"                      || RET=1
validate_campaign "Under_the_Burning_Suns"  "CAMPAIGN_UNDER_THE_BURNING_SUNS"  "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Winds_of_Fate"           "CAMPAIGN_WINDS_OF_FATE"           "EASY" "NORMAL" "HARD" "NIGHTMARE" || RET=1

exit $RET