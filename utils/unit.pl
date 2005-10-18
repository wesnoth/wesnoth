use wml;

#helper functions we want eval to be able to see
sub decimal($)
{
	sprintf "%.2f", $_[0];
}

my $expression = '';
my @sortby = ();
my @filter = ();
my @where = ();
my @heading = ();
my @fold = ();

while(my $arg = shift @ARGV) {
	if($arg eq '--sortby') {
		my $sortby = shift @ARGV or die "need argument to --sortby option";
		unshift @sortby, $sortby;
	} elsif($arg eq '--filter') {
		my $filter = shift @ARGV or die "need argument to --filter option";
		push @filter, $filter;
	} elsif($arg eq '--where') {
		my $where = shift @ARGV or die "need argument to --where option";
		push @where, $where;
	} elsif($arg eq '--prelude') {
		my $prelude = shift @ARGV or die "need argument to --prelude option";
		open PRELUDE, "<$prelude" or die "could not open prelude '$prelude'";
		my @prelude = <PRELUDE>;
		close PRELUDE;

		push @where, (join '',@prelude);
	} elsif($arg eq '--heading') {
		my $heading = shift @ARGV or die "need argument to --heading option";
		@heading = split ',', $heading or die "could not evaluate heading: $!";
	} elsif($arg eq '--fold') {
		my $fold = shift @ARGV or die "need argument to --fold option";
		push @fold, $fold;
	} elsif($arg eq '--help') {
		print "Wesnoth Unit Analysis Tool. Usage: $0 [options] <expression>
<expression> is a Perl expression which will be evaluated for each unit with the results output. There is a set of parentheses assumed around <expression>, so a comma-seperated list of expressions is valid.
A valid Wesnoth cache must be provided on stdin.
	Options:
	--sortby: sort by the given expression
	--filter: evaluates the given expression, and will only display results for the unit if the filter evaluates to true
	--where: executes the given code before executing any other expression for each unit
	--prelude: like --where, but opens up a given file and runs the script in it.
";
	} else {
		die "unrecognized argument '$arg'. Use --help for help." if $expression;
		$expression = $arg;
	}
}

die "must give an expression to evaluate" unless $expression;

my $text = '';
read STDIN, $text, 10000000;
my $doc = &wml::read_binary([],$text) or die "could not read document";
$text = '';

my $units = 0;
foreach my $item (@{$doc->{'children'}}) {
	if($item->{'name'} eq 'units') {
		$units = $item;
		last;
	}
}

die "could not find [units] section" unless $units;

my %movetypes = ();
my @units = ();
my @results = ();
my $columns = 0;

foreach my $item (@{$units->{'children'}}) {
	if($item->{'name'} eq 'unit') {
		push @units, $item;
	} elsif($item->{'name'} eq 'movetype') {
		$movetypes{$item->{'attr'}->{'name'}} = $item;
	}
}

foreach my $unit (@units) {
	my $unitname = $unit->{'attr'}->{'id'};
	my $movetype = $unit->{'attr'}->{'movement_type'};
	$movetype = $movetypes{$movetype};

	if($movetype) {
		foreach my $item (@{$movetype->{'children'}}) {
			&apply_unit_movetype($unit,$item);
		}
	}

	foreach my $item (@{$unit->{'children'}}) {
		&apply_unit_movetype($unit,$item);
	}
}

sub apply_unit_movetype
{
	my ($unit,$mod) = @_;
	my %attr = %{$mod->{'attr'}};
	if($mod->{'name'} eq 'movement_costs') {
		while(my ($name,$value) = each(%attr)) {
			$unit->{'attr'}->{'move_' . $name} = $value;
		}
	} elsif($mod->{'name'} eq 'defense') {
		while(my ($name,$value) = each(%attr)) {
			$unit->{'attr'}->{'defense_' . $name} = $value;
		}
	} elsif($mod->{'name'} eq 'resistance') {
		while(my ($name,$value) = each(%attr)) {
			$unit->{'attr'}->{'resist_' . $name} = $value;
		}
	}
}

foreach my $filter (@filter) {
	my @new_units = ();
	foreach my $unit (@units) {
		my $res = &evaluate_expression($unit,$filter);
		next unless $res;
		my $pass = 1;
		foreach my $item (@$res) {
			unless($item) {
				$pass = 0;
				last;
			}
		}

		push @new_units, $unit if $pass;
	}

	@units = @new_units;
}

foreach my $sortby (@sortby) {
	@units = sort {
my ($resa,$resb) = (&evaluate_expression($a,$sortby),
                    &evaluate_expression($b,$sortby));
return 0 unless $resa or $resb;
return -1 unless $resa;
return 1 unless $resb;
while(@$resa or @$resb) {
	my ($itema,$itemb) = (shift @$resa,shift @$resb);
	my $res = &compare_values($itema,$itemb);
	return $res if $res;
}
return 0;
} @units;
}

my $nfields = 0;
foreach my $unit (@units) {
	my $res = &evaluate_expression($unit,"$expression");
	push @results, $res;
	$nfields = scalar(@$res) if scalar(@$res) > $nfields;
}

foreach my $fold (@fold) {
	my @row = ();
	for my $index ( 0 .. $nfields) {
		my @items = ();
		foreach my $res (@results) {
			if($index < scalar(@$res)) {
				push @items, $res->[$index];
			} else {
				push @items, '';
			}
		}

		push @row, eval "$fold";
	}

	push @results, \@row;
}

unshift @results, \@heading if @heading;

my @cols = (0) x $columns;

foreach my $result (@results) {
	next unless $result;
	my @items = @$result;
	for(my $col = 0; $col != @items; ++$col) {
		my $len = length $items[$col];
		$cols[$col] = $len if $len > $cols[$col];
	}
}

while(@results) {
	my $res = shift @results;
	my $col = 0;
	foreach my $field (@$res) {
		print $field;
		my $whitespace = ' ' x (2 + $cols[$col] - (length $field));
		print $whitespace;
		++$col;
	}

	print "\n";
}

sub evaluate_expression
{
	my ($unit,$expression) = @_;

	my @attacks = ();
	my @melee = ();
	my @ranged = ();

	my ($attack_name,$attack_damage,$attack_strikes,$attack_type,$attack_range,$attack_special) = ('',0,0,'','','');
	my ($melee_name,$melee_damage,$melee_strikes,$melee_type,$melee_special) = ('',0,0,'','','');
	my ($ranged_name,$ranged_damage,$ranged_strikes,$ranged_type,$ranged_special) = ('',0,0,'','');

	foreach my $attack (@{$unit->{'children'}}) {
		next unless $attack->{'name'} eq 'attack';
		my %attr = %{$attack->{'attr'}};
		push @attacks, \%attr;

		my $power = $attr{'damage'}*$attr{'number'};

		if($attr{'range'} eq 'short') {
			if($power > $melee_damage*$melee_strikes) {
				$melee_name = $attr{'name'};
				$melee_damage = $attr{'damage'};
				$melee_strikes = $attr{'number'};
				$melee_type = $attr{'type'};
				$melee_special = $attr{'special'};
			}

			push @melee, \%attr;
		} else {
			if($power > $ranged_damage*$ranged_strikes) {
				$ranged_name = $attr{'name'};
				$ranged_damage = $attr{'damage'};
				$ranged_strikes = $attr{'number'};
				$ranged_type = $attr{'type'};
				$ranged_special = $attr{'special'};
			}
			push @ranged, \%attr;
		}

		if($power > $attack_damage*$attack_strikes) {
			$attack_name = $attr{'name'};
			$attack_damage = $attr{'damage'};
			$attack_strikes = $attr{'number'};
			$attack_type = $attr{'type'};
			$attack_range = $attr{'range'};
			$attack_special = $attr{'special'};
		}
	}

	my ($secondary_name,$secondary_damage,$secondary_strikes,$secondary_type,$secondary_special) =
	   ($melee_name,$melee_damage,$melee_strikes,$melee_type,$melee_special);
	
	
	($secondary_name,$secondary_damage,$secondary_strikes,$secondary_type,$secondary_special) =
	 ($ranged_name,$ranged_damage,$ranged_strikes,$ranged_type,$ranged_special)
	        if $attack_range eq 'short';

	my $script = '{';

	my %attr = %{$unit->{'attr'}};
	while(my ($key,$value) = each(%attr)) {
		$value =~ s/[^\s\w\.\,\-\:\;]//g;
		$$key = $value;
	}

	foreach my $where (@where) {
		$script .= "$where;";
	}

	$script .= "\[$expression\]\}";

	my $result = eval($script);
	while(my ($key,$value) = each(%attr)) {
		$$key = '';
	}

	return $result;
}

sub compare_values
{
	my ($a,$b) = @_;
	return $a <=> $b if $a =~ /^[-\d\.]+$/ and $b =~ /^[-\d\.]+$/;
	return $a cmp $b;
}
