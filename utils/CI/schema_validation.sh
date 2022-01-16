#!/bin/bash

function validate_core()
{
    SUCCESS="Yes"
    NAME="$1"
    
    echo "------"
    echo "Validating $NAME..."
    
    ./wesnoth --validate data/_main.cfg &> temp.log || SUCCESS="No"
    if [ "$SUCCESS" == "No" ]; then
        echo "$NAME failed validation!"
        cat temp.log
        rm temp.log
    fi
    
    echo "$NAME validation complete!  Success: $SUCCESS"
    echo "------"
    
    [ "$SUCCESS" == "Yes" ]
}

function validate_misc()
{
    SUCCESS="Yes"
    NAME="$1"
    DEFINE="$2"
    
    echo "------"
    echo "Validating $NAME..."
    
    ./wesnoth --data-dir=. --validate=data/_main.cfg --preprocess-defines=$DEFINE &> temp.log || SUCCESS="No"
    if [ "$SUCCESS" == "No" ]; then
        echo "$NAME failed validation!"
        cat temp.log
        rm temp.log
    fi
    
    echo "$NAME validation complete!  Success: $SUCCESS"
    echo "------"
    
    [ "$SUCCESS" == "Yes" ]
}

function validate_schema()
{
    SUCCESS="Yes"
    NAME="$1"
    FILE="$2"
    
    echo "------"
    echo "Validating schema $NAME..."
    
    ./wesnoth --data-dir=. --validate-schema=data/schema/$FILE.cfg &> temp.log || SUCCESS="No"
    if [ "$SUCCESS" == "No" ]; then
        echo "$NAME failed validation!"
        cat temp.log
        rm temp.log
    fi
    
    echo "$NAME validation complete!  Success: $SUCCESS"
    echo "------"
    
    [ "$SUCCESS" == "Yes" ]
}

function validate_campaign()
{
    SUCCESS="Yes"
    NAME="$1"
    DEFINE="$2"
    shift
    shift
    DIFFICULTIES=("$@")
    
    echo "------"
    echo "Validating $NAME..."
    
    for DIFFICULTY in ${DIFFICULTIES[@]}; do
        if [ "$SUCCESS" == "Yes" ]; then
            echo "Validating $DIFFICULTY..."
            ./wesnoth --data-dir=. --validate=data/_main.cfg --preprocess-defines=$DEFINE,$DIFFICULTY &> temp.log || SUCCESS="No"
            
            if [ "$SUCCESS" == "No" ]; then
                echo "$NAME failed $DIFFICULTY validation!"
                cat temp.log
            fi
            
            rm temp.log
        else
            echo "Skipping $DIFFICULTY validation"
        fi
    done
    
    echo "$NAME validation complete!  Success: $SUCCESS"
    echo "------"
    
    [ "$SUCCESS" == "Yes" ]
}

RET=0

validate_schema "WML Schema"   "schema"       || RET=1
validate_schema "Game Config"  "game_config"  || RET=1
validate_schema "GUI2"         "gui"          || RET=1
validate_schema "Server Pbl"   "pbl"          || RET=1
validate_schema "WML Diff"     "diff"         || RET=1
validate_core "Core" || RET=1
validate_misc "Editor"      "EDITOR" || RET=1
validate_misc "Multiplayer" "MULTIPLAYER,MULTIPLAYER_A_NEW_LAND_LOAD" || RET=1
validate_misc "Test"        "TEST"            || RET=1
validate_misc "World_Conquest" "MULTIPLAYER,LOAD_WC2,LOAD_WC2_EVEN_THOUGH_IT_NEEDS_A_NEW_MAINTAINER" || RET=1
validate_campaign "Dead_Water"              "CAMPAIGN_DEAD_WATER"              "EASY" "NORMAL" "HARD" "NIGHTMARE" || RET=1
validate_campaign "Delfadors_Memoirs"       "CAMPAIGN_DELFADORS_MEMOIRS"       "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Descent_Into_Darkness"   "CAMPAIGN_DESCENT"                 "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Eastern_Invasion"        "CAMPAIGN_EASTERN_INVASION"        "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Heir_To_The_Throne"      "CAMPAIGN_HEIR_TO_THE_THRONE"      "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Legend_of_Wesmere"       "CAMPAIGN_LOW"                     "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Liberty"                 "CAMPAIGN_LIBERTY"                 "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Northern_Rebirth"        "CAMPAIGN_NORTHERN_REBIRTH"        "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Sceptre_of_Fire"         "CAMPAIGN_SCEPTRE_FIRE"            "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Secrets_of_the_Ancients" "CAMPAIGN_SECRETS_OF_THE_ANCIENTS" "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Son_Of_The_Black_Eye"    "CAMPAIGN_SON_OF_THE_BLACK_EYE"    "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "The_Hammer_of_Thursagan" "CAMPAIGN_THE_HAMMER_OF_THURSAGAN" "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "The_Rise_Of_Wesnoth"     "CAMPAIGN_THE_RISE_OF_WESNOTH"     "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "The_South_Guard"         "CAMPAIGN_THE_SOUTH_GUARD"         "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "tutorial"                "TUTORIAL"                         "EASY"                             || RET=1
validate_campaign "Two_Brothers"            "CAMPAIGN_TWO_BROTHERS"            "EASY" "HARD"                      || RET=1
validate_campaign "Under_the_Burning_Suns"  "CAMPAIGN_UNDER_THE_BURNING_SUNS"  "EASY" "NORMAL" "HARD"             || RET=1
validate_campaign "Winds_of_Fate"           "CAMPAIGN_WINDS_OF_FATE"           "EASY" "NORMAL" "HARD" "NIGHTMARE" || RET=1

exit $RET
