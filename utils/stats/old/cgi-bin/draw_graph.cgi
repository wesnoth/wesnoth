#! /bin/sh
GRAPH=/home/rusty/wesnoth/graph
DATABASE=/home/rusty/wesnoth/wesnoth-uploads.db

. check_args.sh

draw()
{
    case "$W_TYPE" in
	gold)
	    $GRAPH campaign_view gold 'start_turn = 0' $W_CAMPAIGN $W_DIFF $W_VERSION "" `echo $W_SCENARIOS | tr , ' '`;;
	loss_abort)
	    $GRAPH campaign_view 'count(*)' 'result != 1' $W_CAMPAIGN $W_DIFF $W_VERSION "" `echo $W_SCENARIOS | tr , ' '`;;
	wins)
	    $GRAPH campaign_view 'count(*)' 'result = 1' $W_CAMPAIGN $W_DIFF $W_VERSION "" `echo $W_SCENARIOS | tr , ' '`;;
	win_turn)
	    $GRAPH campaign_view 'end_turn*100/(num_turns+1)' 'result = 1' $W_CAMPAIGN $W_DIFF $W_VERSION "" `echo $W_SCENARIOS | tr , ' '`;;
	level)
	    $GRAPH units_view 'sum(count)' "start_turn = 0 AND level = $W_LEVEL" $W_CAMPAIGN $W_DIFF $W_VERSION "GROUP BY game" `echo $W_SCENARIOS | tr , ' '`;;
	time)
	    $GRAPH campaign_view 'sum(time / 60)' 'time > 60' $W_CAMPAIGN $W_DIFF $W_VERSION "GROUP BY player" `echo $W_SCENARIOS | tr , ' '`;;
	player)
	    $GRAPH --progress $W_CAMPAIGN $W_DIFF $W_VERSION $W_PLAYER `echo $W_SCENARIOS | tr , ' '`;;
    esac
}

if [ -n "$W_PNG" ]; then
    echo "Content-type: image/png"
    echo
    TMPFILE=`mktemp`
    trap "rm -f $TMPFILE" 0
    draw > $TMPFILE
    # Hack: find viewBox.
    X_Y=`sed -n 's/.*viewBox="-25 -25 \([0-9]*\) \([0-9]*\)".*/\1,\2/p' < $TMPFILE`
    X=`echo $X_Y | cut -d, -f1`
    Y=`echo $X_Y | cut -d, -f2`
    if [ $Y -gt 160 ]; then
	# Scale back to 160 high.
	X=`echo "scale=2; $X * (160 / $Y)" | bc | cut -d. -f1`
	Y=160
    fi
    rsvg -w$X -h$Y $TMPFILE /dev/fd/1
else
    echo "Content-type: image/svg+xml"
    echo
    draw
fi
