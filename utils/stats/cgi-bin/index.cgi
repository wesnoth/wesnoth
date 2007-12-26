#! /bin/sh

DATABASE=/home/rusty/wesnoth/wesnoth-uploads.db
URL="http://ozlabs.org/~rusty/stats.wesnoth.org/query-wesnoth.cgi"

print_header()
{
    echo "<h1 class=\"firstHeading\">$1</h1>"
    echo '<div id="bodyContent">'
    echo '<h3 id="siteSub">From Wesnoth</h3>'
    echo '<div id="contentSub"></div>'
}        

get_scenario_list()
{
    # Before 1.1.2, the versions weren't generally numbered.  Now they
    # should all be.
    case $1 in
	1.1-svn|1.1+svn|1.1.1|1.1.1+svn)
	    case $2 in
		CAMPAIGN_HEIR_TO_THE_THRONE)
		    echo The_Elves_Besieged Blackwater_Port The_Isle_of_Anduin The_Bay_of_Pearls Muff_Malals_Peninsula Isle_of_the_Damned The_Siege_of_Elensefar Crossroads The_Princess_of_Wesnoth The_Valley_of_Death-The_Princesss_Revenge Gryphon_Mountain The_Ford_of_Abez Northern_Winter Mountain_Pass The_Dwarven_Doors Plunging_into_the_Darkness The_Lost_General Hasty_Alliance Scepter A_Choice_Must_Be_Made Snow_Plains Swamp_Of_Dread North_Elves Elven_Council valley_statue return_to_wesnoth trial_clans battle_for_wesnoth
		    ;;
		CAMPAIGN_TWO_BROTHERS)
		    # These are numbered!
		    /usr/bin/sqlite3 $DATABASE "select DISTINCT scenario FROM campaign_view WHERE campaign = '$2' AND version = '$1';" | sort -n
		    ;;
		*)
		    /usr/bin/sqlite3 $DATABASE "select DISTINCT scenario FROM campaign_view WHERE campaign = '$2' AND version = '$1';"
		    ;;
	    esac
	    ;;
	*)
	    /usr/bin/sqlite3 $DATABASE "select DISTINCT scenario FROM campaign_view WHERE campaign = '$2' AND version = '$1';" | sort -n
	    ;;
    esac
}

# Returns sets of up to 10 comma-separated scenarios, given a scenario list.
chop_scenarios()
{
    if [ $# = 1 ]; then
	echo $1
	return
    fi
    while [ $# -gt 8 ]; do
	echo $1,$2,$3,$4,$5,$6,$7,$8
	# Note that this deliberately repeats one.
	shift 7
    done
    if [ $# -gt 1 ]; then
	echo $@ | tr ' ' ','
    fi
}

colorize()
{
    sed -e 's,>aborted<,><font color=\"#ece000\">aborted</font><,g' -e 's,>victory<,><font color=\"green\">victory</font><,g' -e 's,>defeat<,><font color=\"#ff0d00\">defeat</font><,g'
}

# add_href field extra
add_href()
{
    sed "s,^<TR><TD>\([^<]*\),<TR><TD><a href=\"?$1=\1\$2_VERSION\">\1,"
}

is_official()
{
    case $1 in
	1.1-svn)
	    case $2 in
		CAMPAIGN_EASTERN_INVASION|CAMPAIGN_HEIR_TO_THE_THRONE|CAMPAIGN_SON_OF_THE_BLACK_EYE|CAMPAIGN_THE_RISE_OF_WESNOTH|CAMPAIGN_THE_DARK_HORDES|TUTORIAL)
		    return 0
		    ;;
	    esac
	    return 1
	    ;;
	# Two brothers added.
	1.1+svn|1.1.1)
	    case $2 in
		CAMPAIGN_EASTERN_INVASION|CAMPAIGN_HEIR_TO_THE_THRONE|CAMPAIGN_SON_OF_THE_BLACK_EYE|CAMPAIGN_THE_RISE_OF_WESNOTH|CAMPAIGN_THE_DARK_HORDES|CAMPAIGN_TWO_BROTHERS|TUTORIAL)
		    return 0
		    ;;
	    esac
	    return 1
	    ;;
	# 1.1.1+svn: Under the Burning Suns and South Guard added
	# Son of Black Eye and Dark Hordes removed
	*)
	    case $2 in
		CAMPAIGN_DESERT|CAMPAIGN_THE_SOUTH_GUARD|CAMPAIGN_EASTERN_INVASION|CAMPAIGN_HEIR_TO_THE_THRONE|CAMPAIGN_THE_RISE_OF_WESNOTH|CAMPAIGN_TWO_BROTHERS|TUTORIAL)
		    return 0
		    ;;
	    esac
	    return 1
	    ;;
	esac
}

print_main()
{
    VERSIONS=`/usr/bin/sqlite3 $DATABASE "SELECT name FROM version_names;"`
    # By default, use latest non-svn version.
    if [ -z "$W_VERSION" ]; then
	W_VERSION=`echo "$VERSIONS" | grep -v svn | tail -1`
    fi

    print_header "Wesnoth Statistics for $W_VERSION"

    sep="Other versions available: "
    for ver in $VERSIONS; do
	if [ "$ver" != "$W_VERSION" ]; then
	    echo "$sep <a href=\"?W_VERSION=$ver\">$ver</a> "
	    sep="|"
	fi
    done

    echo '<h2>Wesnoth Official Campaigns</h2>'
    echo '<table border="1"><tr>'
    echo '<th>Campaign Name</th>'
    echo '<th>Difficulty</th>'
    echo '<th>Games Uploaded</th>'
    echo '</tr>'
    /usr/bin/sqlite3 $DATABASE "SELECT campaign,difficulty,count(*) FROM campaign_view WHERE version='$W_VERSION' GROUP BY campaign,difficulty ORDER BY campaign,count(*) DESC;" | while IFS="|" read campaign diff count; do
	if is_official $W_VERSION "$campaign"; then
	    echo "<TR><TD><a href=\"?W_CAMPAIGN=$campaign&W_DIFF=$diff&W_VERSION=$W_VERSION\">$campaign</a></TD><TD>$diff</TD><TD>$count</TD></TR>"
	fi
    done
    echo '</table>'

    echo '<h2>Wesnoth Unofficial Campaigns</h2>'
    echo '<table border="1"><tr>'
    echo '<th>Campaign Name</th>'
    echo '<th>Difficulty</th>'
    echo '<th>Games Uploaded</th>'
    echo '</tr>'
    /usr/bin/sqlite3 $DATABASE "SELECT campaign,difficulty,count(*) FROM campaign_view WHERE version='$W_VERSION' GROUP BY campaign,difficulty ORDER BY campaign,count(*) DESC;" | while IFS="|" read campaign diff count; do
	if ! is_official $W_VERSION "$campaign"; then
	    echo "<TR><TD><a href=\"?W_CAMPAIGN=$campaign&W_DIFF=$diff&W_VERSION=$W_VERSION\">$campaign</a></TD><TD>$diff</TD><TD>$count</TD></TR>"
	fi
    done
    echo '</table>'

    echo "<h2><a href=\"?W_PLAYERS=1&W_VERSION=$W_VERSION\">List of Wesnoth Players</a></h2>"
}

print_players()
{
    print_header "Wesnoth Players for $W_VERSION"

    echo '<h2>Wesnoth Players</h2>'
    echo '<table border="1"><tr>'
    echo '<th>Player ID</th>'
    echo '<th>Games uploaded</th>'
    echo '<th>Last upload</th>'
    echo '</tr>'
    /usr/bin/sqlite3 -html $DATABASE "SELECT DISTINCT campaign_view.player,game_count.games_received-1,game_count.last_upload FROM campaign_view,players,game_count WHERE campaign_view.version = '$W_VERSION' AND game_count.player_ref = players.rowid AND players.id = campaign_view.player;" | sed "s,^<TR><TD>\([^<]*\),<TR><TD><a href=\"?W_PLAYER=\1\&W_VERSION=$W_VERSION\">\1,"
    echo '</table>'
}
add_graph()
{
    echo "<object type=\"image/svg+xml\" data=\"draw_graph.cgi?$1\" NAME=\"$2\" width=\"100%\"><img src=\"draw_graph.cgi?$1&W_PNG=1\"></object>"
}

# Graph out all campaigns a player played (unless specific requested, from graph)
do_graph_player()
{
    echo '(<font color="green">Green</font> means victory, <font color="#ff0d00">red</font> means defeat, <font color="#ece000">yellow</font> means quit.  Bars indicate start/finish turn).<br>'
    if [ -n "$W_CAMPAIGN" ]; then
	EXTRA="AND campaign='$W_CAMPAIGN'"
    fi
    for campaign_diff in `/usr/bin/sqlite3 $DATABASE "SELECT DISTINCT campaign,difficulty FROM campaign_view WHERE player='$W_PLAYER' AND version='$W_VERSION' $EXTRA;"`; do
	campaign=`echo "$campaign_diff" | cut -d\| -f1`
	diff=`echo "$campaign_diff" | cut -d\| -f2`
	SCENARIOS=$(chop_scenarios $(/usr/bin/sqlite3 $DATABASE "SELECT DISTINCT scenario FROM campaign_view WHERE player='$W_PLAYER' AND version='$W_VERSION' AND difficulty='$diff' AND campaign='$campaign';") )

	echo "Campaign $campaign ($diff):"
	for scen in $SCENARIOS; do
	    add_graph "W_SCENARIOS=$scen&W_VERSION=$W_VERSION&W_CAMPAIGN=$campaign&W_DIFF=$diff&W_PLAYER=$W_PLAYER&W_TYPE=player" "Player progress"
	done
    done
    echo "<h2><a href=\"?W_PLAYERP=$W_PLAYER&W_VERSION=$W_VERSION\">Statistics in detail</a></h2>"
}

graph_player()
{
    print_header "Wesnoth Player $W_PLAYER"
    do_graph_player
}

# Print out all scenarios a player played.
print_player()
{
    print_header "Wesnoth Player $W_PLAYER"

    echo '<table border="1"><tr>'
    echo '<th>ID</th>'
    echo '<th>Campaign Name</th>'
    echo '<th>Scenario</th>'
    echo '<th>Difficulty</th>'
    echo '</tr>'
    i=1
    /usr/bin/sqlite3 $DATABASE "SELECT DISTINCT campaign,scenario,difficulty FROM campaign_view WHERE player='$W_PLAYERP' AND version='$W_VERSION';" | while IFS='|' read campaign scenario diff; do 
	echo "<TR><TD><a href=\"?W_PLAYERSCENARIO=$i&W_PLAYER=$W_PLAYERP&W_CAMPAIGN=$campaign&W_SCENARIO=$scenario&W_DIFF=$diff&W_VERSION=$W_VERSION\">$i</TD><TD>$campaign</TD><TD>$scenario</TD><TD>$diff</TD></TR>"
	i=$(($i + 1))
    done
    echo '</table>'
}

# "Type" "Name" scenarios...
add_scenario_graphs()
{
    ASG_TYPE=$1
    ASG_NAME=$2
    shift 2

    for scen; do
	add_graph "W_SCENARIOS=$scen&W_VERSION=$W_VERSION&W_DIFF=$W_DIFF&W_CAMPAIGN=$W_CAMPAIGN&W_TYPE=$ASG_TYPE" "$ASG_NAME"
    done
}

# Graph the campaign on a given campaign_names.rowid
graph_campaign()
{
    print_header "Wesnoth $W_CAMPAIGN ($W_DIFF)"

    SCENARIOS=$(chop_scenarios $(get_scenario_list $W_VERSION $W_CAMPAIGN) )
    echo "<h2>Gold at start of game</h2>"
    add_scenario_graphs "gold" "Gold" $SCENARIOS

    echo "<h2>Losses/quits</h2>"
    add_scenario_graphs "loss_abort" "Losses and quits" $SCENARIOS

    echo "<h2>Wins</h2>"
    add_scenario_graphs "wins" "Victories" $SCENARIOS

    echo "<h2>Percent turns used on victory</h2>"
    add_scenario_graphs "win_turn" "Percent turns used" $SCENARIOS

    echo "<h2>Total minutes per player</h2>"
    add_scenario_graphs "time" "Minutes spent" $SCENARIOS

    for level in 1 2 3; do
	echo "<h2>Count of Level $level units at start of game</h2>"
	add_scenario_graphs "level&W_LEVEL=$level" "Level $level units" $SCENARIOS
    done
    echo "<h2><a href=\"?W_CAMPAIGNP=$W_CAMPAIGN&W_DIFF=$W_DIFF&W_VERSION=$W_VERSION\">Statistics in detail</a></h2>"
}

# Print out details on a given campaign
print_campaign()
{
    print_header "Wesnoth $W_CAMPAIGN ($W_DIFF)"

    echo '<table border="1"><tr>'
    echo '<th>Scenario</th>'
    echo '</tr>'
    /usr/bin/sqlite3 -html $DATABASE "SELECT DISTINCT scenario FROM campaign_view WHERE campaign = '$W_CAMPAIGNP' AND difficulty = '$W_DIFF' AND version = '$W_VERSION';"| sed "s,^<TR><TD>\([^<]*\),<TR><TD><a href=\"?W_SCENARIO=\1\&W_CAMPAIGN=$W_CAMPAIGNP\&W_VERSION=$W_VERSION\&W_DIFF=$W_DIFF\">\1,"
    echo '</table>'
}

# Print out details on a given scenario
print_scenario()
{
    print_header "Wesnoth Scenario $W_SCENARIO ($W_CAMPAIGN $W_DIFF)"

    echo '<table border="1"><tr>'
    echo '<th>Game ID</th>'
    echo '<th>Player</th>'
    echo '<th>Start turn</th>'
    echo '<th>Start gold</th>'
    echo '<th>Time taken (sec)</th>'
    echo '<th>Result</th>'
    echo '<th>End turn</th>'
    echo '<th>Num turns</th>'
    echo '</tr>'
    /usr/bin/sqlite3 -html $DATABASE "SELECT game,player,start_turn,gold,time,CASE result WHEN 0 THEN 'aborted' WHEN 1 THEN 'victory' ELSE 'defeat' END, end_turn, num_turns FROM campaign_view WHERE scenario = '$W_SCENARIO' AND campaign = '$W_CAMPAIGN' AND difficulty = '$W_DIFF';" | sed "s,^<TR><TD>\([^<]*\),<TR><TD><a href=\"?W_GAME=\1\">\1," | colorize
    echo '</table>'
}

# Print out details on a given scenario.
print_player_scenario()
{
    print_header "Wesnoth Player $W_PLAYER playing $W_SCENARIO ($W_CAMPAIGN $W_DIFF)"

    echo "Number of turns in scenario: "
    # Ideally, only returns one number...
    /usr/bin/sqlite3 $DATABASE "SELECT DISTINCT num_turns from campaign_view WHERE scenario='$W_SCENARIO' AND campaign='$W_CAMPAIGN' AND player='$W_PLAYER' AND difficulty='$W_DIFF' AND version='$W_VERSION';"
    echo '<table border="1"><tr>'
    echo '<th>ID</th>'
    echo '<th>Start turn</th>'
    echo '<th>Start gold</th>'
    echo '<th>Time taken (sec)</th>'
    echo '<th>End turn</th>'
    echo '<th>Result</th>'
    echo '</tr>'
    /usr/bin/sqlite3 -html $DATABASE "SELECT game,start_turn,gold,time,end_turn,CASE result WHEN 0 THEN 'aborted' WHEN 1 THEN 'victory' ELSE 'defeat' END FROM campaign_view WHERE version='$W_VERSION' AND player='$W_PLAYER' AND scenario='$W_SCENARIO' AND difficulty='$W_DIFF' AND campaign='$W_CAMPAIGN' ORDER BY game;" | sed "s,^<TR><TD>\([^<]*\),<TR><TD><a href=\"?W_GAME=\1\">\1," | colorize
    echo '</table>'
}

# Print out details and units for a given game.
print_game()
{
    print_header "Details of game $W_GAME"

    echo '<table border="1"><tr>'
    /usr/bin/sqlite3 $DATABASE "SELECT campaign,difficulty,scenario,player,version,gold,start_turn,end_turn,num_turns,time,result FROM campaign_view WHERE game='$W_GAME';" | (IFS="|" read cam diff scen player ver gold st et nt time result
    echo "Campaign: $cam<br>"
    echo "Difficulty: $diff<br>"
    echo "Scenario: $scen<br>"
    echo "Player: $player<br>"
    echo "Version: $ver<br>"
    echo "Start gold: $gold<br>"
    echo "Start turn: $st<br>"
    echo "End turn: $et<br>"
    echo "Num turns: $nt<br>"
    echo "Time taken (sec): $time<br>"
    echo "Result:"
    case $result in
	0) echo "<font color=\"#ece000\">aborted</font>";;
	1) echo "<font color=\"green\">victory</font>";;
	2) echo "<font color=\"#ff0d00\">defeat</font>";;
    esac
    echo "<br>")

    echo "<h2>Important Unit Stats (at Start of Game)</h2>"
    echo '<table border="1"><tr>'
    echo '<th>Name</th>'
    echo '<th>Level</th>'
    echo '<th>Experience</th>'
    echo '</tr>'
    /usr/bin/sqlite3 -html $DATABASE "SELECT unit_names.name,special_units.level,special_units.experience FROM special_units,unit_names WHERE special_units.game_ref=$W_GAME AND unit_names.rowid=special_units.unit_name_ref;"
    echo '</table>'

    echo "<h2>Wesnoth Unit Summary (at Start of Game)</h2>"
    echo '<table border="1"><tr>'
    echo '<th>Type</th>'
    echo '<th>Level</th>'
    echo '<th>Number</th>'
    echo '</tr>'
    /usr/bin/sqlite3 -html $DATABASE "SELECT unit_types.name,unit_types.level,unit_tallies.count FROM unit_types,unit_tallies WHERE unit_tallies.game_ref=$W_GAME AND unit_types.rowid=unit_tallies.unit_type_ref ORDER BY unit_types.level DESC,unit_types.name;"
    echo '</table>'
}

# Simple access page which is given in the Help Wesnoth dialog
my_page()
{
    VERSIONS=`/usr/bin/sqlite3 $DATABASE "SELECT DISTINCT version FROM campaign_view WHERE player=$W_P;"`
    # By default, use latest version for this player
    if [ -z "$W_VERSION" ]; then
	W_VERSION=`echo "$VERSIONS" | tail -1`
    fi

    print_header "Wesnoth Page for Player $W_PLAYER"

    echo "Versions played: "
    sep=""
    for ver in $VERSIONS; do
	if [ "$ver" = "$W_VERSION" ]; then
	    echo "$sep <u>$W_VERSION</u>"
	else
	    echo "$sep <a href=\"?W_P=$W_P&W_VERSION=$ver\">$ver</a> "
	    fi
	sep="|"
    done
    W_PLAYER=$W_P do_graph_player
}

echo "Content-type: text/html"
echo
cat /home/rusty/public_html/stats.wesnoth.org/header.html

# We accept a simple numeric arg for player id.
if echo "$QUERY_STRING" | grep -q '^[0-9][0-9]*$'; then
    QUERY_STRING="W_P=$QUERY_STRING"
fi
. check_args.sh

case "$QUERY_STRING" in
    W_PLAYER=*)
	graph_player
	;;
    W_PLAYERP=*)
	print_player
	;;
    W_CAMPAIGN=*)
	graph_campaign
	;;
    W_CAMPAIGNP=*)
	print_campaign
	;;
    W_SCENARIO=*)
	print_scenario
	;;
    W_PLAYERSCENARIO=*)
	print_player_scenario
	;;
    W_GAME=*)
	print_game
	;;
    W_P=*)
        my_page
	;;
    W_PLAYERS=*)
	print_players
	;;
    *)
	print_main
	;;
esac

echo '<div id="lastmod"> This page based generated from a database, which was last modified '`date -u -r $DATABASE`'.</div>'
cat /home/rusty/public_html/stats.wesnoth.org/footer.html
