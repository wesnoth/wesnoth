# Random map generator for wesnoth
#
# Copyright 2003 J.R. Blain
# Released under the gnu gpl v2 or later.
# 
# Todo: Insert proper gpl notice here.
#
# todo: make town/castle placeement algorithm a little better.
# currently it decides if it can placec one by checking
# the max distance in each direction, it won't take into account
# something like
#
# txxxxx
# xxtxxx
# 
# since they aren't totaly in-line.
# 
# todo: clean it up some, add some comments, make it produce
# WML for the map, some form of interface (perl-gtk?) to allow
# setting map parameters.
# Reorganize settings variables to make more sense, remove
# unused crap.
# Add river/road support.
# 

use strict;
use warnings;
use List::Util qw( shuffle );
use POSIX qw( ceil );
# params:
# width, height, players

use vars qw( $PARAMS $MAP $NUMBERS $DIRMAP $SQUARES );

$NUMBERS = {
	terrain => {
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
	},			
	towns => {
		squares_per_town => 7 * 7,  # 1 town in this many squares.
		min_town_distance => 4, # at least 3 squares from another town.
		type => 't',
	},
	players => {
		count => 8,
		preferred_distance => 30,
		decrement_unit => 5,
	},
};
$PARAMS = {
	width => 64,
	height => 64,
	players => 0,
	base_type => 'g' # fill any remaining map spots with this type.
};

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

generate_base_map();
add_terrain();
fill_map();
level_terrain();
level_terrain();
add_towns();
add_castles();
print_map();

sub add_terrain {
	foreach my $key (keys %{ $NUMBERS->{terrain} }) {
		if ($NUMBERS->{terrain}->{$key}->{mod} eq 'round') {
			my $x = 0;
			my ($counts,$avg_size) = _get_min_max_sizes($key);
			while ($x < $counts) {
				place_round(
					int(rand(@{ $MAP })),
					int(rand(@{ $MAP->[0] })),
					$avg_size,
					$NUMBERS->{terrain}->{$key}->{type});
				++$x;
			}
		}
	}

}
sub add_towns {
	my $towns = ceil(($PARAMS->{height} * $PARAMS->{width}) / $NUMBERS->{towns}->{squares_per_town});
	my $x = 1;
	while ($x <= $towns) {
		my $y = 1;
		# 200 tries to place a town.
		while ($y <= 200) {
			my $plx = int(rand(@{ $MAP }));
			my $ply = int(rand(@{ $MAP->[0] }));
			if (
				
				_town_ok_to_place (
					$plx,
					$ply,
					$NUMBERS->{towns}->{type},
					$NUMBERS->{towns}->{min_town_distance}
				)
			) {
				$MAP->[$plx]->[$ply] = $NUMBERS->{towns}->{type};
				$y = 200; # will exit us from the loop here.
			}
			++$y;
		}
		++$x;
	}
}
sub add_castles {
	my $castle_count = $NUMBERS->{players}->{count};
	my $x = 1;
	while ($x <= $castle_count) {
		my $cur_distance = $NUMBERS->{players}->{preferred_distance};
		my $done = 0;
		while ($cur_distance > 0 && !$done) {
			my $y = 1;
			# 200 tries to place a castle.
			while ($y <= 200) {
				my $plx = int(rand(@{ $MAP }));
				my $ply = int(rand(@{ $MAP->[0] }));
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

	# searches circular until it finds a matching terrain type.
	# returns undef if it can't find one.  Note, this routine
	# is a bit of a hog.
	
	# simple check, are we on top of one?
	if ($MAP->[$x]->[$y] eq $type) {
		return undef;
	}
	my @dirs = (0,1,2,3,4,5,6,7);
	foreach my $dir (@dirs) {
		my $cnt = 1;
		my $bx = $x;
		my $by = $y;
		while ($cnt <= $search_dist) {
			($bx,$by) = _move_direction($bx,$by,$dir);
			if (defined($bx) && defined($by)) {
				if ($MAP->[$bx]->[$by] eq $type) {
					return undef;
				}
			}
			else {
				$cnt = $search_dist; # this will get us out of the loop safely.
			}
			++$cnt;
		}
	}
	return 1;
}
sub _castle_ok_to_place {
	my ($x,$y,$search_dist) = @_;

	# searches circular until it finds a matching terrain type.
	# returns undef if it can't find one.  Note, this routine
	# is a bit of a hog.
	
	# simple check, are we on top of one?
	if ($MAP->[$x]->[$y] =~ /^\d+$/) {
		return undef;
	}
	my @dirs = (0,1,2,3,4,5,6,7);
	foreach my $dir (@dirs) {
		my $cnt = 1;
		my $bx = $x;
		my $by = $y;
		while ($cnt <= $search_dist) {
			($bx,$by) = _move_direction($bx,$by,$dir);
			if (defined($bx) && defined($by)) {
				if ($MAP->[$bx]->[$by] =~ /^\d+$/) {
					return undef;
				}
			}
			else {
				$cnt = $search_dist; # this will get us out of the loop safely.
			}
			++$cnt;
		}
	}
	return 1;
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
	my $height = @{ $MAP };
	my $width = @{ $MAP->[0] };
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
	elsif ($dir == 2 && ($x - 1) >= 0 && ($y + 1) < @{ $MAP->[0] }) {
			++$y;
			--$x;
	}
	# move east if possible
	elsif ($dir == 3 && ($y + 1) < @{ $MAP->[0] }) {
			++$y;
	}
	# move south-east if possible.
	elsif ($dir == 4 && ($y + 1) < @{ $MAP->[0] } && ($x + 1) < @{ $MAP }) {
			++$y;
			++$x;
	}
	# move south if possible
	elsif ($dir == 5  && ($x + 1) < @{ $MAP }) {
			++$x;
	}
	# move south-west if possible
	elsif ($dir == 6  && ($y - 1) >= 0 && ($x + 1) < @{ $MAP }) {
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
	debug("FINDING NEXT SQUARE FOR: $x $y\n");
	foreach my $dir (@try_dirs) {
		my ($bx,$by) = _move_direction($x,$y,$dir);
		if (_is_placeable($bx,$by)) {
			return ( x => $bx, y => $by );
		}
	}
	return (x => undef,  y => undef);
}
sub debug {
	#warn @_;
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

