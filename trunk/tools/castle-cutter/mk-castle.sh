#!/bin/bash
#

syntaxerror() {
    echo "usage: $0 [-b <basename>] [-c <cutout>] [-m <mask>] [-o <output>] [-t <template>] file"
    exit 1
}

cutoutf="cutout.png"
#maskf="mask.png"
output="castle"
basename="castle"
template="castle-walls.tmpl"

while [ $# -ne 0 ]; do
    case "$1" in
        -c* )
        shift
        cutoutf="$1"
        shift
        ;;
#	-m* )
#        shift
#	maskf="$1"
#	shift
#	;;
	-o* )
	shift
	output="$1"
	shift
	;;
	-b* )
	shift
	basename="$1"
	shift
	;;
	-t* )
	shift
	template="$1"
	shift
	;;
	-* )
	syntaxerror
	;;
	* )
	file="$1"
	shift
	;;
    esac
done    

if [ -z "$file" ]; then
	syntaxerror
fi

# File location definitions

tmpdir=`mktemp -d /tmp/mk-castle.XXXXXXX`
#mask="$tmpdir/mask.pgm"
cutout="$tmpdir/cutout.pbm"
castle="$tmpdir/castle.ppm"
alpha="$tmpdir/alpha.pgm"

# Creates pnm files from the original png files

#pngtopnm "$maskf" | ppmtopgm | pnminvert > "$mask"
pngtopnm "$cutoutf" > "$cutout"
pngtopnm "$file" > "$castle"
pngtopnm -alpha "$file" > "$alpha"

# Generates the 12 cutout masks from the cutout file

colors="rgbi:1/0/0 rgbi:1/0/1 rgbi:0/0/1 rgbi:0/1/1 rgbi:0/1/0 rgbi:1/1/0 rgbi:.5/0/0 rgbi:.5/0/.5 rgbi:0/0/.5 rgbi:0/.5/.5 rgbi:0/.5/0 rgbi:.5/.5/0"
i=0

for color in $colors; do
	cmask[$i]="$tmpdir/cmask$i.pbm"
	ppmcolormask $color $cutout | pnminvert > ${cmask[$i]}
	i=$(($i+1))
done

# Cuts the castle, and the castle's alpha, according to the 12 masks

# directions="concave-ne concave-e concave-se concave-sw concave-w concave-nw convex-ne convex-e convex-se convex-sw convex-w convex-nw"
directions=`cat $template`

i=0

for direction in $directions; do
	ccastle[$i]="$tmpdir/$basename-$direction"
	calpha[$i]="$tmpdir/alpha-$direction"
	pnmarith -multiply ${cmask[$i]} $castle > ${ccastle[$i]}.ppm
	pnmarith -multiply ${cmask[$i]} $alpha > ${calpha[$i]}.pgm
	i=$(($i+1))
done

# Cuts each image according to the size it should have

dimensions="54,36-126,144 54,36-126,180 54,108-126,144 0,72-126,160 0,72-126,144 0,0-126,180 162,36-126,144 162,36-126,180 162,108-126,144 108,72-126,160 108,72-126,144 108,0-126,180"

i=0

for dimension in $dimensions; do
	position=${dimension%-*}
	size=${dimension#*-}

	posx=${position%,*}
	posy=${position#*,}

	sizex=${size%,*}
	sizey=${size#*,}

	pnmcut -left $posx -top $posy -width $sizex -height $sizey ${ccastle[$i]}.ppm > ${ccastle[$i]}-x.ppm
	pnmcut -left $posx -top $posy -width $sizex -height $sizey ${calpha[$i]}.pgm > ${calpha[$i]}-x.ppm
	pnmtopng -alpha ${calpha[$i]}-x.ppm  ${ccastle[$i]}-x.ppm > ${ccastle[$i]}.png

	i=$(($i+1))
done


# Harvests the generated PNGs, and erase all those generated files.

if [ ! -d "$output" ]; then
	mkdir $output
fi

cp "$tmpdir"/*.png "$output"
rm -r "$tmpdir"

