#!/usr/bin/perl 
## Random map generator for wesnoth
#
# Copyright 2003 J.R. Blain
# Released under the gnu gpl v2 or later.
# 
# Todo: 
# Insert proper gpl notice here.
# Make it do the new map multi-letter map format
# clean it up some, add some comments, make it produce
# WML for the map, some form of interface (perl-gtk?) to allow
# setting map parameters.
# Reorganize settings variables to make more sense, remove
# unused crap.
# Add river/road support.
# ansi-ize the map
#
# Changelog:
#
#    Nov 18 2003 (Cowboy)
#	- Cleaned up a little after Miyo :)
#    		- switched some if ($foo => $bar) to if ($foo >= $bar)
#    		  so it doesn't think it's a hash assignment
#    		  and throw a stupid warning.
#
#	- Updated to clean up some routines, 
#	- remove an endless loop (not sure if it was mine or miyos)
#	- added "snow"
#	- updated _scan_ring to handle invalid ranges better. (faster)    
#
#    Nov 18 2003 (Miyo)
#	- added '#!/usr/bin/perl -w'
#	- implemented Getops::Long
#	- usage|help
# 	- enable/disable towns
# 	- selectable base terrain type
# 	- configurable terrain leveling
#
#    Nov 17 2003 
# 	- Better comments
# 	- Updated proximity scanning routines
# 	  They now radiate out in squares to judge
# 	  distance.
# 	- little bit of code cleanup in various places.
# 	- fixed up castle placing mostly.  Still has an 
# 	  issue where it's placing them too close to edges
# 	  at times.
# 		
#     Nov 16 2003 
# 	- initial release

use strict;
use warnings;
use List::Util qw( shuffle );
use POSIX qw( ceil );
use Getopt::Long;

use vars qw( $PARAMS $MAP $NUMBERS $DIRMAP $SQUARES );

$NUMBERS = {
	terrain => {
		grassland => {
			percent => 0,
			type => 'g',
			mod => 'round',
		},
		forests => {
			percent => 25,
			type => 'f',
			mod => 'round',
		},
		mountains => {
			percent => 10,
			type => 'm',
			mod => 'round',
		},
		deserts => {
			percent => 10,
			type => 'd',
			mod => 'round',
		},
		shallows => {
			percent => 10,
			type => 'c',
			mod => 'round',
		},
		hills => {
			percent => 10,
			type => 'h',
			mod => 'round',
		},
		snow => {
			percent => 5,
			type => 'S',
			mod => 'round',
		},
	},			
	towns => {
		squares_per_town => 6 * 6,  # 1 town in this many squares.
		min_town_distance => 2, # at least 3 squares from another town.
		type => 't',
	},
	players => {
		count => 4,
		preferred_distance => 30,
		decrement_unit => 1,
	},
};
$PARAMS = {
	map_size => '64x64',
	base_type => 'g', # fill any remaining map spots with this type.
	towns => 1,
	castles => 1,
	usage => 0,
	verbose => 0,
	leveling => 2,
};


GetOptions ( 
	'size=s' => \$PARAMS->{map_size},
	'grassland=i' => \$NUMBERS->{terrain}{grassland}{percent},
	'forest=i' => \$NUMBERS->{terrain}{forests}{percent},
	'mountains=i' => \$NUMBERS->{terrain}{mountains}{percent},
	'desert=i' => \$NUMBERS->{terrain}{deserts}{percent},
	'shallow=i' => \$NUMBERS->{terrain}{shallows}{percent},
	'snow=i' => \$NUMBERS->{terrain}{snow}{percent},
	'hills=i' => \$NUMBERS->{terrain}{hills}{percent},
	'players=i' => \$NUMBERS->{players}{count},
	'playerdist=i' => \$NUMBERS->{players}{preferred_distance},
	'towns!' => \$PARAMS->{towns},
	'towndist=i' => \$NUMBERS->{towns}{min_town_distance},
	'townspt=i' => \$NUMBERS->{towns}{squares_per_town},
	'baseterrain=s' => \$PARAMS->{base_type},
	'leveling=i' => \$PARAMS->{leveling},
	'help|usage' => \$PARAMS->{usage},
	'verbose' => \$PARAMS->{verbose},
	) || usage();



( $PARAMS->{height}, $PARAMS->{width} ) = split /x/,$PARAMS->{map_size};

# not used yet, thoughts for the future.
$SQUARES = {
	m => {
		passable => 0,
	},
	C => {
		passable => 1,
	},
	d => {
		passable => 1,
	},
	c => {
		passable => 1,
		pass_types => {
			R => 'b', # if crossing a river, use a bridge
		}
	},
	s => {
		passable => 0,
	},
	g => {
		passable => 1,
	},
	R => {
		passable => 1,
		pass_types => {
			c => 'b', # water + roads make bridges
		}
	}
};

# $DIRMAP -> used to provide a map of 'general'
# directions for things like rivers, roads, ect.
# north could mean north, north-east,  or north-west,
# ect.  This allows rivers/roads (and other things that
# use this approximate direction stuff) to meander a little
# while staying mostly true to course.
$DIRMAP = {
	0 => { 7 => 1, 0 => 1, 1 => 1 },
	1 => { 0 => 1, 1 => 1, 2 => 1 },
	2 => { 1 => 1, 2 => 1, 3 => 1 },
	3 => { 2 => 1, 3 => 1, 4 => 1 },
	4 => { 3 => 1, 4 => 1, 5 => 1 },
	5 => { 4 => 1, 5 => 1, 6 => 1 },
	6 => { 5 => 1, 6 => 1, 7 => 1 },
	7 => { 6 => 1, 7 => 1, 0 => 1 },
};

#
# Terrain types:
# d => desert
# R => road
# g => grass
# m => mountains
# f => forests
# c => coast (or river)

# the map is a 2 dimensional grid.
#
#  0123456
# 0    y
# 1 X yy
# 2  X
# 3
# 4
# 5
$MAP = [];

usage() if $PARAMS->{usage} == 1;

generate_base_map();
add_terrain();
fill_map();


if ( $PARAMS->{leveling} >= 1 ) {
	my $t = 1;
	while ($t <= $PARAMS->{leveling}) {
		warn "Leveling...\n" if $PARAMS->{verbose};
		level_terrain();
		++$t;
	}
}


add_towns() if $PARAMS->{towns};

add_castles() if $NUMBERS->{players}->{count} >= 1;

print_map();


sub usage {

abort_with_error("
usage: random_map.pl [ OPTIONS ]

 -help|usage         give this help
 -verbose            print more information

 -size NUMxNUM       set map size, height x width (default=64x64)
 -leveling NUM       set number of terrain levelings to occur (default=2)
 -players NUM        set number of player (default=8)
 -playerdist NUM     set preferred distance between players (default=30)
 -notowns            disable towns
 -towndist           set minimun distance between towns (default=2)
 
 -baseterrain CHAR   set base terrain type (default=g)
 -grassland NUM      set grassland % (default=0)
 -forest NUM         set forest % (default=10)
 -mountains NUM      set mountain % (default=10)
 -desert NUM         set desert % (default=10)
 -shallow NUM        set shallow water % (default=10)
 -hills NUM          set hills % (default=10)
 -snow NUM           set snow % (default=5)
 \n");
}


sub abort_with_error {
	die @_;	
}
sub add_terrain {
	foreach my $key (keys %{ $NUMBERS->{terrain} }) {
		if ($NUMBERS->{terrain}->{$key}->{mod} eq 'round') {
			my $x = 0;
			my ($counts,$avg_size) = _get_min_max_sizes($key);
			while ($x < $counts) {
				place_round(
					int(rand($PARAMS->{width})),
					int(rand($PARAMS->{height})),
					$avg_size,
					$NUMBERS->{terrain}->{$key}->{type});
				++$x;
			}
		}
	}
}


sub add_towns {
	warn "add_towns\n" if $PARAMS->{verbose};
	my $towns = ceil(($PARAMS->{height} * $PARAMS->{width}) / $NUMBERS->{towns}->{squares_per_town});
	my $x = 1;
	while ($x <= $towns) {
		my $y = 1;
		# 200 tries to place a town.
		warn "trying to place town\n" if $PARAMS->{verbose};
		while ($y <= 200) {
			my $plx = int(rand($PARAMS->{height}));
			my $ply = int(rand($PARAMS->{width}));
			if (
				_town_ok_to_place (
					$plx,
					$ply,
					$NUMBERS->{towns}->{type},
					$NUMBERS->{towns}->{min_town_distance}
				)
			) {
				$MAP->[$plx]->[$ply] = $NUMBERS->{towns}->{type};
				warn "placed town at $plx $ply\n" if $PARAMS->{verbose};
				$y = 200; # will exit us from the loop here.
			}
			else {
				warn "town not ok: $y\n" if $PARAMS->{verbose};
			}
			++$y;
		}
		warn "next town\n" if $PARAMS->{verbose};
		++$x;
	}
}
sub add_castles {
	warn "add_castles\n" if $PARAMS->{verbose};
	my $castle_count = $NUMBERS->{players}->{count};
	my $x = 1;
	while ($x <= $castle_count) {
		my $cur_distance = $NUMBERS->{players}->{preferred_distance};
		my $done = 0;
		while ($cur_distance > 1 && !$done) {
			my $y = 1;
			# 200 tries to place a castle.
			while ($y <= 200) {
				my $plx = int(rand($PARAMS->{height}));
				my $ply = int(rand($PARAMS->{width}));
				if (
					
					_castle_ok_to_place (
						$plx,
						$ply,
						$cur_distance
					)
				) {
					surround(
						$plx,
						$ply,
						'C');
					$MAP->[$plx]->[$ply] = $x;
					$y = 200; # will exit us from the loop here.
					$done = 1;
				}
				++$y;
			}
			$cur_distance -= $NUMBERS->{players}->{decrement_unit};
		}
		if (!$done) {
			die "Something screwed up, could not place castle $x\n";
		}
		++$x;
	}
}


sub _get_min_max_sizes {
	my ($terrain) = shift;
	my $total_squares = $PARAMS->{width} * $PARAMS->{height};
	my $total_terrain_size = $total_squares * ($NUMBERS->{terrain}->{$terrain}->{percent} / 100);
	my $counts = int(rand(5)) + 3;
	my $avg_size = $total_terrain_size / $counts;
	return ($counts,$avg_size);
}

sub _town_ok_to_place {
	my ($x,$y,$type,$search_dist) = @_;
	warn "checking town\n" if $PARAMS->{verbose};
	# searches circular until it finds a matching terrain type.
	# returns undef if it can't find one.  Note, this routine
	# is a bit of a hog.
	
	# simple check, are we on top of one?
	if ($MAP->[$x]->[$y] eq $type) {
		return undef;
	}
	warn "$x $y $search_dist $type\n" if $PARAMS->{verbose};
	if (_scan_ring($x,$y,$search_dist,$type)) {
		return undef;
	}
	return 1;
}
sub _castle_ok_to_place {
	my ($x,$y,$search_dist) = @_;

	# searches circular until it finds a matching terrain type.
	# returns undef if it can't find one.  Note, this routine
	# is a bit of a hog.
	
	# simple check, are we on top of one?
	if (_is_castle($MAP->[$x]->[$y])) {
		return undef;
	}
	if (_scan_border($x,$y,4)) {
		# at least 4 squares from the map edge.
		return undef;
	}
	if (_scan_ring($x,$y,$search_dist,'castle')) {
		return undef;
	}
	return 1;
}

sub _scan_border {
	my ($x,$y,$dist) = @_;
	unless ($MAP->[$x + $dist + 1]->[$y + $dist + 1]) {
		return 1;
	}
	unless ($MAP->[$x - $dist]->[$y - $dist]) {
		return 1;
	}
	return undef;
}

# scans in a ring around location looking
# for a specific type of terrain.
# if it finds it, it returns true,
# otherwise, returns undef.
# used by place town, place castle
sub _scan_ring {
	my ($x,$y,$dist,$type) = @_;
	# first we build a list of all the
	# x,y locations that match our ring.
	# 
	

	my $sx = $x - $dist; # starting x point
	my $sy = $y - $dist; # starting y point
	my $ex = $x + $dist; # ending x point
	my $ey = $y + $dist; # ending y point
	
	
	my $cx = $sx;
	while ($cx <= $ex) {
		warn "Looping through $cx\n" if $PARAMS->{verbose};
		if ($cx >= 0 && $cx < $PARAMS->{height}) {
			my $cy = $sy;
			while ($cy <= $ey) {
				if ($cy >= 0 && $cy < $PARAMS->{width}) {
					if ($MAP->[$cx]->[$cy] && $type eq 'castle' && _is_castle($MAP->[$cx]->[$cy])) {
						return 1;
					}
					if ($MAP->[$cx]->[$cy] && $type eq $MAP->[$cx]->[$cy]) {
						return 1;
					}
				}
				++$cy;
			}
		}
		$cx++;
	}
	return undef;

}

# just a nicely named routine to keep from using
# regular expressions all over.
sub _is_castle {
	my ($type) = @_;
	if ($type =~ /^\d$/) {
		return 1;
	}
	return undef;
}

sub surround {
	my ($x,$y,$type) = @_;
	my @dirs = (0,1,2,3,4,5,6,7,8);
	foreach my $dir (@dirs) {
		my ($bx,$by) = _move_direction($x,$y,$dir);
		if (defined($bx) && defined($by)) {
			$MAP->[$bx]->[$by] = $type;
		}
	}
}

# 
# place an object
# with a general round shape.
# This includes forests, mountains,
# ect.  This works by choosing a center point
# and then expanding around it in an almost circular
# pattern.
# 
# Takes 4 params:
# starting X, starting Y, size (hexes), type (f,m,ect)
# 

sub place_round {
	my ($x,$y,$size,$type) = @_;
	my $count = 1;
	my $cust = 0;
	while ($count <= $size) {
		if ($cust == 8) {
			$cust = 0;
		}
		if (int(rand(6))) {
			$cust = int(rand(8));
		}
		my %result = _get_next_map_square(x => $x, y => $y, dir => $cust);
		if (defined($result{x}) && defined($result{y})) {
			$MAP->[$result{x}]->[$result{y}] = $type;
			($x,$y) = ($result{x},$result{y});
		}
		else {
		}
		++$count;
		++$cust;
	}
	return 1;
}


# dummy sub for now
sub _is_placeable {
	return 1;
}

# this routine checks each square on the map,
# and "levels" it.  an individual grassland,
# surrounded totally by mountains, will become a mountain.
# any terrain feature with less than 2 of it's own type
# near it,  becomes the predominant feature of the terrain
# nearby.
# It's sorta an ugly fix to some of the randomness issues,
# but thus far has seemed to work out ok.
sub level_terrain {
	my $height = $PARAMS->{height};
	my $width = $PARAMS->{width};
	$height--;
	$width--;
	my $x = 0;
	while ($x <= $height) {
		my $y = 0;
		while ($y <= $height) {
			_level_terrain($x,$y);
			++$y;
		}
		++$x;
	}
}
sub _level_terrain {
	my ($x,$y) = @_;
	my @dirs = (0,1,2,3,4,5,6,7);
	my %counts = ();
	foreach my $dir (@dirs) {
		my ($nx,$ny) = _move_direction($x,$y,$dir);
		if (defined($nx) && defined($ny)) {
			$counts{$MAP->[$nx]->[$ny]}++;
		}
	}
	unless (defined($counts{$MAP->[$x]->[$y]}) && $counts{$MAP->[$x]->[$y]} > 1) {
		my @types = sort { $counts{$b} <=> $counts{$a} } keys %counts;
		$MAP->[$x]->[$y] = $types[0];
	}
}

sub _move_direction {
	my ($x,$y,$dir) = @_;
	if ($dir == 0 && ($x - 1) >= 0 && ($y - 1) >= 0) {
			--$x;
			--$y;
	}
	# move north if possible.
	elsif ($dir == 1 && ($x - 1) >= 0) {
			--$x;
	}
	# move north-east if possible.
	elsif ($dir == 2 && ($x - 1) >= 0 && ($y + 1) < $PARAMS->{width}) {
			++$y;
			--$x;
	}
	# move east if possible
	elsif ($dir == 3 && ($y + 1) < $PARAMS->{width}) {
			++$y;
	}
	# move south-east if possible.
	elsif ($dir == 4 && ($y + 1) < $PARAMS->{width} && ($x + 1) < $PARAMS->{height}) {
			++$y;
			++$x;
	}
	# move south if possible
	elsif ($dir == 5  && ($x + 1) < $PARAMS->{height}) {
			++$x;
	}
	# move south-west if possible
	elsif ($dir == 6  && ($y - 1) >= 0 && ($x + 1) < $PARAMS->{height}) {
			--$y;
			++$x;
	}
	# move west if possible
	elsif ($dir == 7 && ($y - 1) >= 0) {
			--$y;
	}
	else {
		return (undef,undef);
	}
	return ($x,$y);
}

sub _get_next_map_square {
	my %args = @_;

	unless (defined($args{dir})) {
		$args{dir} = int(rand(8));
	}
	my @try_dirs = keys %{ $DIRMAP->{$args{dir}} };
	
	# shuffle them
	@try_dirs = shuffle(@try_dirs);


	my $x = $args{x};
	my $y = $args{y};
	foreach my $dir (@try_dirs) {
		my ($bx,$by) = _move_direction($x,$y,$dir);
		if (_is_placeable($bx,$by)) {
			return ( x => $bx, y => $by );
		}
	}
	return (x => undef,  y => undef);
}


#
# Fills any remaining unused map spaces
# with a standard type
#

sub fill_map {
	my $x = 0;
	while ($x <= $PARAMS->{height} - 1) {
		my $y = 0;
		while ($y <= $PARAMS->{width} - 1) {
			if ($MAP->[$x]->[$y] eq '@') {
				$MAP->[$x]->[$y] = $PARAMS->{base_type};
			}
			++$y;
		}
		++$x;
	}

}

#
# Prints the current map.
#
sub print_map {
	my $x = 0;
	while ($x <= $PARAMS->{height} - 1) {
		my $y = 0;
		while ($y <= $PARAMS->{width} - 1) {
			print $MAP->[$x]->[$y];
			++$y;
		}
		++$x;
		print "\n";
	}
}
#
# Generates the base map grids.
# This essentially fills out all the $MAP->[x]->[y] values
# with dummy characters.
# 
sub generate_base_map {
	my $x = 0;
	while ($x <= $PARAMS->{height} - 1) {
		my $y = 0;
		$MAP->[$x] = [];
		while ($y <= $PARAMS->{width} - 1) {
			$MAP->[$x]->[$y] = '@';
			++$y;
		}
		++$x;
	}
}

