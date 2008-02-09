#!/usr/bin/env perl
#
# Automatically generate border images for tiles. Works by copying the way in 
# which border tiles are done for an existing terrain type.
#
# For instance, if you made a new terrain type, 'lava.png' and you wanted 
# to have the same style of edges as snow does, you could run the command
#
# data/tools/mk-edges.pl images/terrain/lava images/terrain/snow
#
# and it will make the edges for you.

print "usage: $0 <input> <stem>" and exit 0 if $#ARGV != 1;
my ($input,$stem) = @ARGV;

sub process {
    my $in=shift || die;
    my $edge=shift || die;
    my $out=shift || die;

    system <<EOF;
    pngtopnm -alpha $edge >/tmp/edge-alpha.pgm;
    pngtopnm $edge >/tmp/edge.ppm;
    ppmtopgm /tmp/edge.ppm >/tmp/edge.pgm;
    pngtopnm $in >/tmp/bg.ppm;

    pnmcomp -alpha=/tmp/edge.pgm /tmp/bg.ppm /tmp/edge.ppm | pnmtopng -alpha /tmp/edge-alpha.pgm >$out;
EOF
}

foreach (glob "${stem}-*.png") {
    my $edge=$1 if /$stem-(.*).png/;
    if($edge=~/^[news-]*$/) {
	process("${input}.png","${stem}-${edge}.png","${input}-${edge}.png");
    }
}


