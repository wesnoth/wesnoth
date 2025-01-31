#!/bin/sh
#
# Find missing images referenced by WML (as in issue #8432)
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

# Parsing WML with a regular expression isn't ideal, but it's easy and it works.
# Note that this only matches images specified with "PLACE_IMAGE" and
# "PLACE_IMAGE_SUBMERGED" macros, not "[item]" "image=" images.
# This regex is injected into a sed s command with separator | so any | need to be escaped.
IMG_RE='
	^                     ; Beginning of line
	[^#]*                 ; Zero or more characters other than hash (do not match
	                      ; commented-out images)
	[{]                   ; Open curly brace
	PLACE_IMAGE           ; "PLACE_IMAGE" macro name
	[^[:blank:]]*         ; Optional non-whitespace characters (e.g. "_SUBMERGED")
	[[:blank:]]+          ; One or more spaces or tabs after macro name
	[(]?                  ; One optional open parenthesis
	"?                    ; One optional double quote around image file name
	([^{}$[:blank:]~")]+) ; Capture image file name (one or more characters other
	                      ; than curly braces, dollar, space, tab, tilde, double
	                      ; quote, or close parenthesis)
	[[:blank:]~")}]       ; One space, tab, tilde, double quote, close
	                      ; parenthesis, or close curly brace after the file name
	.*                    ; Zero or more characters
	$                     ; End of line
	'
# Somewhat simulate Perl's "/x" modifier ("Extend your pattern's legibility by
# permitting whitespace and comments"):
IMG_RE="$(printf '%s' "${IMG_RE}" | sed '
	s|^[[:blank:]]*||;  # Strip leading whitespace
	s|;.*$||;           # Strip comments (use ";" since "#" is used in the RE)
	s|[[:blank:]]*$||;  # Strip trailing whitespace
	' | tr -d '\n')"    # Remove all newlines

# In `! { COMMANDS | grep -- '.'; }` below, `grep -- '.'` returns a successful
# exit status if `COMMANDS` produce output, and `!` inverts it into an error
# status.  A less clever/obscure solution would be `es=true` before the loop,
# `es=false` inside the loop after all the `continue`s, and `${es}` at the end;
# but that won't work because all commands in pipelines run in subshells that
# can't affect variables in the parent shell.

! { git grep -EIn -- "${IMG_RE}" data/ | while IFS=':' read -r path ln img; do
	img="$(printf '%s' "${img}" | sed -E "s|${IMG_RE}|\\1|")"
	[ -e "data/core/images/${img}" ] && continue
	[ -e "${img}" ] && continue
	case "${path}" in
		data/campaigns/*)
			suffix="${path#data/campaigns/*/}"
			[ -e "${path%/"${suffix}"}/images/${img}" ] && continue
			;;
	esac
	printf '%s:%d: missing image: %s\n' "${path}" "${ln}" "${img}"
done | sort -t ':' -k 1,1 -k 2n,2n | grep -- '.'; }
