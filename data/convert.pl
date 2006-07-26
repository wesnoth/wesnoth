print STDERR "usage: $0 [filename]\n" and exit 0 if $#ARGV < 0;

$fname = $ARGV[0];

open FILE, "<$fname" or die "could not open $fname: $!\n";

while($line = <FILE>) {
	push @lines, $line;
}

close FILE;

open FILE, ">$fname" or die "could not open $fname for writing: $!\n";

foreach $line (@lines) {

	if($line =~ /\[end\]/) {
		$item = pop @items;
		$line =~ s/\[end\]/\[\/$item\]/;
	}

	($item) = $line =~ /\[([_a-z0-9 ]+)\]/i;
	push @items, $item if $item;

	print FILE $line;
}

close FILE;
