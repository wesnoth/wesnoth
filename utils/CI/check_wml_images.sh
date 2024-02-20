#!/bin/sh
#
# Find missing images referenced by WML (as in issue #8432)
#
# Requires sort with -s option (stable sort, disable further byte-by-byte
# comparison of lines that collate equally by specified sort key), e.g. from
# GNU coreutils
#
# Example output:
#     data/multiplayer/scenarios/2p_Aethermaw.cfg:134: missing image: units/undead/shadow-s-attack-1.png
#     data/multiplayer/scenarios/2p_Aethermaw.cfg:135: missing image: units/undead/shadow-n-attack-2.png
#     data/multiplayer/scenarios/2p_Aethermaw.cfg:147: missing image: units/undead/shadow-n-3.png
#     data/multiplayer/scenarios/2p_Aethermaw.cfg:149: missing image: units/undead/shadow-s-attack-4.png
#     data/multiplayer/scenarios/2p_Aethermaw.cfg:196: missing image: units/human-loyalists/spearman-attack-se-10.png
#     data/multiplayer/scenarios/2p_Aethermaw.cfg:197: missing image: units/human-loyalists/spearman-attack-s-6.png
#     data/multiplayer/scenarios/2p_Aethermaw.cfg:198: missing image: units/human-loyalists/spearman-attack-se-9.png
#     data/multiplayer/scenarios/2p_Aethermaw.cfg:203: missing image: units/human-loyalists/general-idle-5.png

set -eu

# Parsing WML with a regular expression isn't ideal, but it's easy and it works.
IMG_RE='^[^#]*[{]PLACE_IMAGE[^ ]* +[(]?"?([^{}$ ~")]+)[ ~")].*$'

# In `! { COMMANDS | grep -- '.'; }` below, `grep -- '.'` returns a successful
# exit status if `COMMANDS` produce output, and `!` inverts it into an error
# status.  A less clever/obscure solution would be `es=true` before the loop,
# `es=false` inside the loop after all the `continue`s, and `${es}` at the end;
# but that won't work because all commands in pipelines run in subshells that
# can't affect variables in the parent shell.

! { git grep -EIn -- "${IMG_RE}" data/ | while IFS=':' read -r wml ln img; do
	img="$(printf '%s' "${img}" | sed -E "s|${IMG_RE}|\\1|")"
	[ -e "data/core/images/${img}" ] && continue
	[ -e "${img}" ] && continue
	case "${wml}" in
		data/campaigns/*)
			sfx="${wml#data/campaigns/*/}"
			[ -e "${wml%/${sfx}}/images/${img}" ] && continue
			;;
	esac
	printf '%s:%d: missing image: %s\n' "${wml}" "${ln}" "${img}"
done | sort -t ':' -k 2n,2n | sort -t ':' -k 1,1 -s | grep -- '.'; }
