#!/bin/bash
#

syntaxerror() {
    echo "usage: $0 [-b <basename>] [-c <cutout>] [-m <mask>] [-o <output>] [-t <template>] file"
    exit 1
}

cutoutf="cutout.png"
maskf="mask.png"
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
	-m* )
        shift
	maskf="$1"
	shift
	;;
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
mask="$tmpdir/mask.pgm"
cutout="$tmpdir/cutout.pbm"
castle="$tmpdir/castle.ppm"
alpha="$tmpdir/alpha.pgm"

# Creates pnm files from the original png files

pngtopnm "$maskf" | ppmtopgm | pnminvert > "$mask"
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

# For each cut castle & image tile, extract its 3 hexagons

# Coordinates of the angles 0

angle0e="71,70"
angle0i="177,70"

# Coordinates of each other angle, relative to angle 0

angles_relative="0,0 17,35 0,70 -36,70 -53,35 -36,0"

x0e=${angle0e%,*}
y0e=${angle0e#*,}
x0i=${angle0i%,*}
y0i=${angle0i#*,}
i=0

# Compose the actual list of coordinates for each angle

for angle in $angles_relative; do
	xr=${angle%,*}
	yr=${angle#*,}

	echo "Angle is $angle"
	x=$(($xr + $x0e))
	y=$(($yr + $y0e))
	angles[$i]="$x,$y"
	
	x=$(($xr + $x0i))
	y=$(($yr + $y0i))
	angles[$((i+6))]="$x,$y"

	i=$(($i+1))
done

# Move each angle on a corner of the hexagon, cut the image
# to 70x70 size, then apply mask

hexcorners_even="53,0 53,70 0,35"
hexcorners_names_even=(ne se w)
hexcorners_odd="70,35 17,70 17,0"
hexcorners_names_odd=(e sw nw)
margin=200 # Adds a margin to files
width=72
height=72
i=0

for angle in ${angles[*]}; do
	mcastle=${ccastle[$i]}
	malpha=${calpha[$i]}

	pnmmargin "$margin" "$mcastle.ppm" > "$mcastle-m.ppm"
	pnmmargin "$margin" "$malpha.pgm" > "$malpha-m.pgm"

	if [ $(($i % 2)) -eq 0 ]; then
		prefix="_even"
	else
		prefix="_odd"
	fi

	xangle=${angle%,*}
	yangle=${angle#*,}
	j=0
	hexcorners_n="hexcorners$prefix"
	for corner in ${!hexcorners_n}; do
		name_n="hexcorners_names$prefix[$j]"
		name=${!name_n}
		xcorner=${corner%,*}
		ycorner=${corner#*,}

		xdelta=$(($xangle - $xcorner + $margin))
		ydelta=$(($yangle - $ycorner + $margin))

		pnmcut -left $xdelta -top $ydelta -width $width \
			-height $height "$mcastle-m.ppm" > \
			"$mcastle-$name-x.ppm"
		pnmarith -multiply "$mask" "$mcastle-$name-x.ppm" > \
			"$mcastle-$name.ppm"

                pnmcut -left $xdelta -top $ydelta -width $width \
                        -height $height "$malpha-m.pgm" > \
                        "$malpha-$name-x.pgm"
                pnmarith -multiply "$mask" "$malpha-$name-x.pgm" > \
                        "$malpha-$name.pgm"

		pnmtopng -alpha "$malpha-$name.pgm" "$mcastle-$name.ppm" > \
			"$mcastle-$name.png"

		j=$(($j+1))
	done

	i=$((i+1))
done

if [ \! -d "$output" ]; then
	mkdir $output || die "Unable to create output directory"
fi

# Harvests the generated PNGs, and erase all those generated files.

cp "$tmpdir"/*.png $output
rm -r "$tmpdir"

