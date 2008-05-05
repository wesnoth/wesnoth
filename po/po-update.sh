#!/bin/sh

DOMAIN=$1
LANG=$2
GETTEXT_MSGMERGE_EXECUTABLE=$3
GETTEXT_MSGINIT_EXECUTABLE=$4

if test -f "$LANG.po"; then
	echo "$GETTEXT_MSGMERGE_EXECUTABLE --update $LANG.po $DOMAIN.pot"
	$GETTEXT_MSGMERGE_EXECUTABLE --update $LANG.po $DOMAIN.pot
#    touch $LANG.upd
else
	echo "$GETTEXT_MSGINIT_EXECUTABLE --no-translator --input=$DOMAIN.pot --output-file=$LANG.po --locale=$LANG"
    $GETTEXT_MSGINIT_EXECUTABLE --no-translator --input="$DOMAIN.pot" --output-file="$LANG.po" --locale="$LANG"
#    touch $LANG.upd
fi
