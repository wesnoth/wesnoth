#!/usr/bin/perl
# Script: units.pl
# Purpose: To generate flat files and HTML files from the Battle for Wesnoth unit configuration files
#
# Requisites: All the following files, that are included in the package:
#   anim_header.html: Template for the header of the report of animations made
#   index_base.html: Index page that will be presented, in case translations are not selected
#   index_languages.html: Index page that will be presented, in case translations are selected
#   TeamColorizer.pl: Script created by Darth Fool, to change the magenta color of the units pngs
#   tree_fact_header.html: Template for the header of the tree by faction
#   tree_footer.html: Template for the footer of the unit trees
#   tree_header.html: Template for the header of the unit trees, except the ones by race of faction
#   tree_race_header.html: Template for the header of the tree by race
#   unit.html: Template for the unit page
#   units.css: Style file for all the html pages
# For converting the images using the TeamColorizer.pl script, ImageMagik must be installed,
#   and the convert utility accessible from the console
# The Perl module Locale::Maketext::Gettext is required for the translations, the latest version can be found at CPAN (http://cpan.org)
#   If the translation is not going to be used, the line use Locale::Maketext::Gettext; can be commented to avoid installing the module
#
# Usage: Select the amount of data to be generated using the flags on the Options section.
#   Enter the appropriate paths for the Wesnoth directory, and the folders that will contain the html files
#   and the reports.  Use relative paths, that don't contain spaces, it wouldn't work if a space is found.
#   Run the script, no parameters needed.
#
# Author: Miguel Zapico (elricz.m@gmail.com)

# -- Options --
# This option will determine if the html files are generated, in the folder determined by $html_dir
my $html_gen = 1;
# This option will determine if the html files translations are generated, and it will create the folders
#   based on the contents of the po folder of Wesnoth
my $translate = 1;
# If translating, this option will try to use the source code instead of the compiled files
my $source = 1;
# This option will determine if the attack images are copied, and the images units are copied and colorized
my $images = 1;
# This option will determine if the html report on made animations is generated
my $animations = 1;
# This is the version number that will appear on the unit trees
my $version = '1.3.12+svn';
# These option will try to process the user made Eras
my $ime = 0; # Imperial Era
my $exe = 0; # Extended Era
my $eom = 0; # Era of Myths
# If the script is run on Windows, set this option to 1
my $windows = 0;

use Tie::File;
use File::Copy;

# -- Paths --
my ($wesnoth_dir, $data_dir, $html_dir, $report_dir, $base_dir, $units_dir, $base_report_dir, $link_back);
$wesnoth_dir = '../../..';
$html_dir = 'files';
$report_dir = 'reports';

# Directories creations
unless (-e $html_dir) {mkdir $html_dir or die "$html_dir directory cannot be created: $!\n";};
unless (-e $report_dir) {mkdir $report_dir or die "$report_dir directory cannot be created: $!\n";};
unless (-e "$html_dir/attacks") {mkdir "$html_dir/attacks" or die "$html_dir/attacks directory cannot be created: $!\n";};
unless (-e "$html_dir/units") {mkdir "$html_dir/units" or die "$html_dir/units directory cannot be created: $!\n";};
$data_dir = "$wesnoth_dir/data/core";
$data_dir = "$wesnoth_dir/data" if $version =~ /^1.2/;
$base_dir = $wesnoth_dir; $base_report_dir = $report_dir;
$units_dir = $data_dir . "/units";

# Variables used to generate the html
my ($i, $html, %types, %unit_id, @adv);
$unit_id{AdvanceTo} = 'AdvanceTo';
# Variables used on the tree generation
my (%spaces, %adv_from, %units, %units_id, %adv, %factions, %attacks, %att_id);
my %races = qw/race Race bats race^Bats drake race^Drakes dwarf race^Dwarves elf race^Elves goblin race^Goblins gryphon race^Gryphons 
	human race^Humans lizard race^Lizards mechanical race^Mechanical merman race^Mermen monster race^Monsters naga race^Nagas ogre race^Ogres
	orc race^Orcs troll race^Trolls undead race^Undead wose race^Woses/;
# HTML code for each attack
my $att_html = "<tr>\n\t<td><img src={icon}></td>\n\t";
$att_html .= "<td>{name}</td>\n\t<td>{type}</td>\n\t<td>{damage}-{number}</td>\n\t<td>{range}</td>\n\t<td>{special}<!-- -->&nbsp;</td>\n</tr>\n";

# Information on the units.cfg file
&ProcessTypes('units.cfg');
# -- HTML files and raw data reports generation --
if ($version =~ /^1.2/) {
	&ProduceDataFiles("templates/unit_1.2.html");
} else {
	&ProduceDataFiles("templates/unit.html");
}

# Multiplayer
# Print multiplayer units
open (UNITS, "$report_dir/units.txt") or die "Couldn't open units.txt: $!\n";
open (MP, "> $report_dir/mp_units.txt") or die "Couldn't create mp_units.txt: $!\n";
print MP "id\tname\trace\thitpoints\tmovement_type\tmovement\texperience\tlevel\talignment\timage\tcost\tusage\tabilities\tfull_name\n";
while (<UNITS>) {
	my $unit = $_;
	my ($none, $id, @other) = split /\t/; # We have names, not ids, on the %mp hash
	print MP $unit if $mp{$id};
}
close MP;
close UNITS;

# -- Tree generation --
&GenerateTree;

# -- Translate file --
&TranslateUnits if $translate;

# -- Remove English comments --
&RemoveComments if $html_gen;

# -- Copy images --
&CopyImages if ($images);

# -- Generate animation information
&GenerateAnimationInfo if ($animations);

# Extended Era
if ($exe) {
	$i=300; $version = 'x.31 (unstable)';
	($html, %unit_id, @adv, %spaces, %adv_from, %units, %units_id, %adv, %factions, %attacks, %att_id, %races) = ();
	$unit_id{AdvanceTo} = 'AdvanceTo';
	$link_back = '../';
	$wesnoth_dir = $base_dir . '/userdata/data/campaigns/Extended_Era';
	$data_dir = $wesnoth_dir;
	$units_dir = $data_dir . "/units/standard";
	$html_dir = "$html_dir/EXE";
	$report_dir = "$html_dir/data";
	unless (-e $html_dir) {mkdir $html_dir or die "$html_dir directory cannot be created: $!\n";};
	unless (-e $report_dir) {mkdir $report_dir or die "$html_dir directory cannot be created: $!\n";};
	print "Processing Extended Era\n";
	foreach ( glob("$data_dir/races/*.cfg") ) {
		s|$data_dir\/||;
		&ProcessTypes($_);
	}
	&ProduceDataFiles("templates/unit_exe.html");
	# Get races
	open (UNITS, "$report_dir/units.txt") or die "Couldn't open units.txt: $!\n";
	while (<UNITS>) {
		my (@stats) = split /\t/;
		$races{$stats[2]} = ucfirst($stats[2]);
	}
	close UNITS;

	&GenerateTree ('_exe');
	&RemoveComments() if $html_gen;
	&GenerateAnimationInfo if ($animations);
	# Move the html files to the main html folder
	foreach ( glob("$html_dir/*.html") ) {
		(my $new_file = $_) =~ s|EXE\/||;
		move($_,$new_file) unless /tree_|index|animation/;
	}
	&RemoveComments(1) if $html_gen;
	$html_dir =~ s|(.+)\/EXE|$1|;
	&CopyImages if ($images);
}

# Imperial Era
if ($ime) {
	$i=600; $version = '0.16.5';
	($html, %unit_id, @adv, %spaces, %adv_from, %units, %units_id, %adv, %factions, %attacks, %att_id, %races) = ();
	$unit_id{AdvanceTo} = 'AdvanceTo';
	$link_back = '';
	$wesnoth_dir = $base_dir . '/userdata/data/campaigns/Imperial_Era';
	$data_dir = $wesnoth_dir;
	$units_dir = $data_dir . "/units";
	$html_dir = "$html_dir/IME";
	$report_dir = "$html_dir/data";
	unless (-e $html_dir) {mkdir $html_dir or die "$html_dir directory cannot be created: $!\n";};
	unless (-e $report_dir) {mkdir $report_dir or die "$html_dir directory cannot be created: $!\n";};
	print "Processing Imperial Era\n";
	&ProcessTypes('units/various.cfg');
	&ProduceDataFiles("templates/unit_IME.html");
	# Get races
	open (UNITS, "$report_dir/units.txt") or die "Couldn't open units.txt: $!\n";
	while (<UNITS>) {
		my (@stats) = split /\t/;
		$races{$stats[2]} = ucfirst($stats[2]);
	}
	close UNITS;

	&GenerateTree ('_ime');
	&RemoveComments(1) if $html_gen;
	&GenerateAnimationInfo if ($animations);
	$html_dir =~ s|(.+)\/IME|$1|;
	&CopyImages if ($images);
}

# Era of Myths
if ($eom) {
	$i=900; $version = 'Beta 8';
	($html, %unit_id, @adv, %spaces, %adv_from, %units, %units_id, %adv, %factions, %attacks, %att_id, %races) = ();
	$unit_id{AdvanceTo} = 'AdvanceTo';
	$link_back = '';
	$wesnoth_dir = $base_dir . '/userdata/data/campaigns/Era_of_Myths';
	$data_dir = $wesnoth_dir;
	$units_dir = $data_dir . "/units";
	$html_dir = "$html_dir/EOM";
	$report_dir = "$html_dir/data";
	unless (-e $html_dir) {mkdir $html_dir or die "$html_dir directory cannot be created: $!\n";};
	unless (-e $report_dir) {mkdir $report_dir or die "$html_dir directory cannot be created: $!\n";};
	print "Processing Era of Myths\n";
	&ProcessTypes('units/EOM_Movetypes.cfg');
	&ProduceDataFiles("templates/unit_EOM.html");
	# Get races
	open (UNITS, "$report_dir/units.txt") or die "Couldn't open units.txt: $!\n";
	while (<UNITS>) {
		my (@stats) = split /\t/;
		$races{$stats[2]} = ucfirst($stats[2]);
	}
	close UNITS;

	&GenerateTree ('_eom');
	&RemoveComments(1) if $html_gen;
	&GenerateAnimationInfo if ($animations);
	$html_dir =~ s|(.+)\/EOM|$1|;
	&CopyImages if ($images);
}

# ----------------------------------
# -- Subroutines used in the code --
# ----------------------------------
# Sub to produce the data and HTML files
sub ProduceDataFiles {
	my $html_unit = shift;
	print "Starting the generation of unit files\n";
	
	#Files that will contain the units data
	open (UNITS, "> $report_dir/units.txt") or die "Couldn't create units.txt: $!\n";
	open (RES, "> $report_dir/resistances.txt") or die "Couldn't create resistances.txt: $!\n";
	open (ADV, "> $report_dir/advances.txt") or die "Couldn't create advances.txt: $!\n";
	open (ATTACK, "> $report_dir/attacks.txt") or die "Couldn't create attacks.txt: $!\n";
	open (FACT, "> $report_dir/factions.txt") or die "Couldn't create factions.txt: $!\n";
	# Headers
	print UNITS "id\tname\trace\thitpoints\tmovement_type\tmovement\texperience\tlevel\talignment\timage\tcost\tusage\tabilities\tfull_name\n";
	print RES "id\tVariable\tValue\tCategory\n";
	print ADV "id\tAdvanceTo\n";
	print ATTACK "id\tname\ttype\trange\tdamage\tnumber\ticon\tspecial\n";
	
	# Load HTML template
	open (HTML, $html_unit);
	$html = '';
	$html .= $_ while (<HTML>);
	close HTML;
	
	# Now all the unit files in the data/units directory
	my @units = glob("$units_dir/*.cfg");
	&ProcessUnit($_) foreach @units;
	# For units stored in folders
    my @camps = glob("$units_dir/*");
    foreach (@camps) {
		unless (/(fake|cfg)$/) {
		($camp = $_) =~ s/(.*)\///;
		@units = glob($_ . '/*.cfg');
		&ProcessUnit ($_) foreach @units;
		}
    }

	
	# Factions, populates the factions file
	my @factions;
	if ($html_unit eq 'templates/unit.html') { 
		@factions = glob("$wesnoth_dir/data/multiplayer/factions/*");
	} else {
		@factions = glob("$wesnoth_dir/factions/*");
	}
	my %mp;
	foreach (@factions) {
		&ProcessFaction ($_) if (/cfg$/);
	}
	
	if ($html_dir =~ /EXE$/) { # Merge mainline units for extended era
		open (MAIN_UNITS, "$base_report_dir/units.txt") or die "Couldn't open units.txt: $!\n";
		while (<MAIN_UNITS>) {
			print UNITS unless /^id/;
			my ($id, $name) = split /\t/;
			$unit_id{$name} = $id;
		}
		close MAIN_UNITS;
		
		open (MAIN_ATT, "$base_report_dir/attacks.txt") or die "Couldn't open attacks.txt: $!\n";
		while (<MAIN_ATT>) {print ATTACK unless /^id/;}
		close MAIN_ATT;	
	}

	close UNITS;
	close RES;
	close ADV;
	close ATTACK;
	close FACT;

	# Check advances to not miss any unit
	tie @adv, 'Tie::File', "$report_dir/advances.txt" or die "Couldn't open advances.txt: $!\n";
	for (@adv) {
		my ($id, $adv) = split /\t/;
		$mp{$adv}++ if $mp{$id}; # List the units that will be processed as MP
		s/\t([0-9A-Za-z_ ]+)/\t$unit_id{$1}/; # Changes names to ids in the advance file
	}
	if ($html_dir =~ /EXE$/) { # Merge mainline units for extended era
		open (MAIN_ADV, "$base_report_dir/advances.txt") or die "Couldn't open advances.txt: $!\n";
		while (<MAIN_ADV>) {
			chomp;
			push @adv, $_ unless /^id/;
		}
		close MAIN_ADV;
	}
	untie @adv;	
}

# Sub to parse the unit file, and print both the text files and the html
sub ProcessUnit {
	$unit = shift;
	$i++; # This will be the unit id
	open (UNIT, "<$unit") or die "Can't open $unit: $!\n";
    my (%unit,%attack,%res,%move,$att);
    my $flag = 1; # This flag will identify the section inside the config file
    while (<UNIT>) {
	    chomp; s/^\s*//; 
	    last if /^\[variation/; # Don't process double data
	    if (/^\[female/) { # Used in the HTML generation, not included on the units.txt file
			($unit{image_female} = $unit{image}) =~ s/.png/+female.png/;
			$unit{image_female} = "<img src=" . $unit{image_female} . ">";
			last;
		} 
	    # Primary information ($flag = 1)
	    if (/=/ and $flag == 1) {
			my ($prop,$value) = split /=/;
			$unit{$prop} = $value unless $unit{$prop}; # The first value encountered is the one that last
			$unit{$prop} = $value if $unit{$prop} eq 'null'; # Unless we have a null value stored before
		}
		# Abilities, single regular expresion
	    $unit{abilities} .= lc("$1 ") if (/{ABILITY_(.*?)(_.*)?}/ and $flag == 1);
	    # Soulless units, the stats are in the first {UNIT_BODY...}
	    if (/^{UNIT_BODY_\w+\s(\w+)\s(\w+)\s(\w+)\s(\w+)/) {
			$unit{image2} = "units/undead/$1.png>&nbsp;\n<img src=units/undead/$1-drake.png>&nbsp;\n<img src=units/undead/$1-mounted.png>";
			$unit{image2} .= "&nbsp;\n<img src=units/undead/$1-saurian.png>&nbsp;\n<img src=units/undead/$1-swimmer.png>";
			$unit{image2} .= "&nbsp;\n<img src=units/undead/$1-troll.png>&nbsp;\n<img src=units/undead/$1-wose.png";
			$unit{image} = "units/undead/$1.png";
			$unit{movement_type} = $2;
			$unit{movement} = $3;
			$unit{hitpoints} = $4;
		}
		
	    # Deviations from the move type, if any ($flag = 2)
		$flag = 1 if /^\[\/(resistance|defense|death|movement_costs)\]/;
		if (/=/ and $flag == 2) {
			my ($type, $value) = split /=/;
			$cost = 0 if $cost >= 100;
			if ($var eq "movement_costs") {
				$move{$type} = $value;
				print RES "$i\t$type\t" . $value . "\t$var\n";
			} else {
				$res{$type} = (100-$value) . "%";
				print RES "$i\t$type\t" . (100-$value) . "\t$var\n";
			}
		}
		($flag,$var) = (2,$1) if /^\[(resistance|defense|movement_costs)\]/;
	    # Attacks ($flag = 3)
	    if (/{WEAPON_SPECIAL_(.*)}/ and $flag == 3) { # Special attack starts with {WEAPON_SPECIAL
			$attack{special} = lc($1);
			# Special cases, slow and plague types are not in the .mo files
			$attack{special} .= "s" if $attack{special} =~ /(?:slow|drain)/;
			$attack{special} = "plague" if $attack{special} =~ /plague/;
		} 
		if ($flag == 3 and /^\[\/attack\]/ ) {
			$flag = 1;
			$attack{icon} = "attacks/$attack{name}.png" unless $attack{icon}; # If the value is not present, make it equal to the name
			print ATTACK "$i\t$attack{name}\t$attack{type}\t$attack{range}\t$attack{damage}\t$attack{number}\t$attack{icon}\t$attack{special}\n";
			$attack{icon} = "attacks/" . $attack{icon} unless $attack{icon} =~ /^attacks/; # Fix the path to the images folder
			($att .= $att_html) =~ s/{(\w+?)}/$attack{$1}/eg; # Construct the html for the attack
			%attack = '';
		}
		if (/=/ and $flag == 3) {
			my ($prop,$value) = split /=/ if /=/;
			$attack{$prop} = $value;
		}
		($flag, $attack{special}) = (3, '') if /^\[attack\]/;
		# Skip Death sequence
		$flag = 4 if /^\[death\]/;
    }
    # Print to match the headers, there may be empty elements
    $unit{abilities} =~ s/\s$//; # Fix the extra space in the abilities
    $unit{name} =~ s/^.+?"(.+)"$/$1/; # Fix the name
    if ($unit{id}) {
		print UNITS "$i\t$unit{id}\t$unit{race}\t$unit{hitpoints}\t$unit{movement_type}\t$unit{movement}\t$unit{experience}";
		print UNITS "\t$unit{level}\t$unit{alignment}\t$unit{image}\t$unit{cost}\t$unit{usage}\t$unit{abilities}\t$unit{name}\n";
		$unit{image} = $unit{image2} if $unit{image2};
	}

	# Advances
	$unit_id{$unit{id}} = $i;
    if ($unit{advanceto} ne 'null') {
		foreach (split (/,/,$unit{advanceto})) {
			s/^\s*//;
			print ADV "$i\t$_\n";
			(my $link = $_) =~ s/\s/_/g;
			$unit{advancetohtml} .= "<a href=$link.html>$_</a>&nbsp\n";
		}
	}
	
	close UNIT;
	
	# Generate HTML
	if ($html_gen) {
		(my $filename = $unit{id}) =~ s/\s/_/g;
		$unit{unit_description} =~ s/( _ )?"//g;
		$unit{num} = $i;
		$unit{abilities} =~ s/\s/<!-- -->\n<!-- -->/;
		foreach (keys %{ $types{$unit{movement_type}}{defense} }) {
			$res{$_} = $types{$unit{movement_type}}{defense}{$_} . "%" unless $res{$_};
		}
		foreach (keys %{ $types{$unit{movement_type}}{movement} }) {
			$move{$_} = $types{$unit{movement_type}}{movement}{$_} unless $move{$_};
		}
		open (HTML_UNIT, "> $html_dir/$filename.html") or die "Couldn't create $unit{id}: $!\n";
		($html_unit = $html) =~ s/{(\w+?)}/$unit{$1}/eg;
		$html_unit =~ s/::(\w+?)::/$res{$1}/eg;
		$html_unit =~ s/~(\w+?)~/$move{$1}/eg;
		$html_unit =~ s/--attacks--/$att/;
		print HTML_UNIT $html_unit;
		close HTML_UNIT;
	}
}

# Sub to generate the unit trees
sub GenerateTree {
	if ($html_gen) {
		my $era_file = shift;
		print "Starting the generation of unit trees\n";
		copy('units.css',"$html_dir/units.css");
		# Load HTML templates
		open (HTML, "templates/tree_header$era_file.html") or die "Couldn't open header: $!\n";
		my @header = <HTML>;
		s/X.X.X/$version/ foreach @header;
		close HTML;
		open (HTML, "templates/tree_footer.html") or die "Couldn't open footer: $!\n";
		my @footer = <HTML>;
		@footer[2] =~ s/date/gmtime(time)/e; # Generation time
		close HTML;
		
		# Get the advances for each unit
		open (ADV, "$report_dir/advances.txt") or die "Couldn't open advances.txt: $!\n";
		while (<ADV>) {
			chomp;
			my ($id, $adv) = split /\t/;
			$spaces{$id}++; # Used on the rowspan
			$adv_from{$adv} = $id;
			push @{$adv{$id}}, $adv;
		}
		close ADV;
		
		# Calculate the correct row span for each unit
		foreach (sort keys %spaces) {
			if ($spaces{$_} > 1 and $adv_from{$_}) {
				$spaces{$adv_from{$_}} += $spaces{$_} - 1;
			}
		}
    foreach $id (sort keys %spaces) {
      my $expected;
      foreach $i (0..$#{ $adv{$id} }) {
        $expected += $spaces{$adv{$id}[$i]};
      }
      $spaces{$id}=$expected if $spaces{$id}<$expected;
    }
		
		# Units information
		open (UNITS, "$report_dir/units.txt") or die "Couldn't open units.txt: $!\n";
		while (<UNITS>) {
			chomp;
			my ($id, @stats) = split /\t/;
			$units{$id} = [ @stats ];
			$units_id{$stats[0]} = $id;
      $adv_to{$stats[0]} = $stats[-1];
		}
		close UNITS;
		
		# Attacks information
		open (ATT, "$report_dir/attacks.txt") or die "Couldn't open attacks.txt: $!\n";
		while (<ATT>) {
			chomp;
			my ($id, @stats) = split /\t/;
			$att_id{$id}++; # Numeral for the hash, there may be better ways to do it
			$attacks{$id}[$att_id{$id}-1] = [ @stats ];
		}
		close ATT;
		
		# Tree by race
		open (INDEX, "> $html_dir/tree_race.html") or die "Couldn't create tree_race.html: $!\n";
		open (HTML, "templates/tree_race_header$era_file.html") or die "Couldn't open header: $!\n";
		while (<HTML>) {s/X.X.X/$version/; print INDEX;}
		close HTML;
		foreach $race (sort keys %races) {
			if ($race ne "race") {
				open ($race, ">", "$html_dir/tree_$race.html") or die "Couldn't create $race.html: $!\n"; # Use the variable as the filehandle
				print $race @header;
				print INDEX "\n<tr>\n<td class='race' id=\"$race\" colspan=6>$races{$race}</td>\n</tr>\n";
				print $race "\n<tr>\n<td class='race' id=\"$race\" colspan=5>$races{$race}</td>\n</tr>\n";
				foreach (sort keys %units) {
					if ($race eq $units{$_}[1]) { # Only process units for the current race
						&PrintUnitTree($_, *INDEX) unless $adv_from{$_};
						&PrintUnitTree($_, $race) unless $adv_from{$_};
					}
				}
				print $race @footer;
				close $race;
			}
		}
		print INDEX @footer;
		close INDEX;
		
		# Update Advance From on each file
		&UpdateAdvanceFrom;
	
		# Tree by faction
		# Load factions
		open (INDEX, "> $html_dir/tree_faction.html") or die "Couldn't create tree_faction.html: $!\n";
		open (HTML, "templates/tree_fact_header$era_file.html") or die "Couldn't open header: $!\n";
		while (<HTML>) {s/X.X.X/$version/; print INDEX;}
		close HTML;
		# Delete the advances from the units in the factions, so the tree can be built
		my @delete_adv = qw/Thug Footpad Spearman Bowman Poacher Lieutenant/;
		delete($adv_from{$units_id{$_}}) foreach @delete_adv;
		open (FACTIONS, "$report_dir/factions.txt") or die "Couldn't open factions.txt: $!\n";
		while (<FACTIONS>) {
			chomp;
			my ($faction, @stats) = split /\t/;
      # Identify the standard factions for each Era
			if ($faction =~ /(\w+)-default/ || $faction =~ /^imper-(\w+)/ || $faction =~ /(\w+)-extended/ || $faction =~ /(\w+)-EOM/) { 
				$factions{$1} = [ @stats ];
				open ($faction, ">", "$html_dir/tree_$faction.html") or die "Couldn't create $faction.html: $!\n";
				print $faction @header;
				my $faction_name = ucfirst($1);
				# Adjust the name of the factions to get them translated
				$faction_name = 'Knalgan Alliance' if $faction_name eq 'Knalgans';
				print INDEX "\n<tr>\n<td class='race' id=\"$1\" colspan=5>$faction_name</td>\n</tr>\n";
				foreach (@stats) {
					&PrintUnitTree($units_id{$_}, *INDEX) unless $adv_from{$units_id{$_}};
					&PrintUnitTree($units_id{$_}, $faction) unless $adv_from{$units_id{$_}};
				}
				print $faction @footer;
				close $faction;
			}
		}
		close FACTIONS;
		# Load footer
		print INDEX @footer;
		close INDEX;
		
		# Copy the index file before translating
		if ($translate) {
			open (HTML, 'templates/index_languages.html') or die "Couldn't open index_languages.html: $!\n";
		} else {
			open (HTML, "templates/index_base$era_file.html") or die "Couldn't open index_base.html: $!\n";
		}
		open (INDEX, "> $html_dir/index.html") or die "Couldn't create index.html: $!\n";
		while (<HTML>) {s/X.X.X/$version/; print INDEX;}
		close HTML;
		close INDEX;
	}
}

# Sub to copy images
sub CopyImages {
  # Era of Myths images
  my ($att_folder, $unit_folder, @unit_images); 
  if ($wesnoth_dir =~ /Era_of_Myths/) {
    $att_folder = 'attacks/';
    $unit_folder = 'units/';
  }
    $data_dir = $wesnoth_dir if $version =~ /^1.2/;
	# Attacks images
	print "Copying attack icons\n";
	open (ATT, "$report_dir/attacks.txt") or die "Couldn't open attacks.txt: $!\n";
	while (<ATT>) {
		chomp;
		my (@stats) = split /\t/;
		copy ("$data_dir/images/$stats[6]","$html_dir/$att_folder$stats[6]");
		#print "$data_dir/images/$stats[6]\t$html_dir/$att_folder$stats[6]\n";
	}
	close ATT;

	# Unit images, to be colorized
	print "Copying and colorizing units\n";
	# Prepare folders
	@unit_images = glob("$data_dir/images/units/*");
	@unit_images = glob("$data_dir/images/*") if ($wesnoth_dir =~ /Era_of_Myths/); 
	foreach $unit_image (@unit_images) {
		$unit_image =~ s(.*\/)(); # Take only the folder name
		if ($unit_image !~ /\./) {
			unless (-e "$html_dir/units/$unit_image") {mkdir "$html_dir/units/$unit_image" or die "$unit_image directory cannot be created: $!\n";};
		}
	}
	# Unit images
	open (UNITS, "$report_dir/units.txt") or die "Couldn't open units.txt: $!\n";
	my $colorizer = "TeamColorizer.pl";
	$colorizer = './' . $colorizer unless $windows;
	while (<UNITS>) {
		chomp;
		my @stats = split /\t/;
		(my $image = $stats[9]);
		$image =~ s/"//g;
		system ("$colorizer $data_dir/images/$image $html_dir/$unit_folder$image");
		$image =~ s/.png/+female.png/;
		system ("$colorizer $data_dir/images/$image $html_dir/$unit_folder$image") unless (! -e "$data_dir/images/$image");
	}
	close UNITS;
	# zombie units
	my @zombies = qw/drake mounted saurian swimmer troll wose/;
	foreach $zombie (@zombies) {
		system ("$colorizer $data_dir/images/units/undead/zombie-$zombie.png $html_dir/units/undead/zombie-$zombie.png");
		system ("$colorizer $data_dir/images/units/undead/soulless-$zombie.png $html_dir/units/undead/soulless-$zombie.png");
	}
}

# Sub to generate animation information
sub GenerateAnimationInfo {
	print "Generating animation information\n";
	my %tags;
	my @anim = qw/animation death defend healing_anim idle_anim leading_anim movement_anim recruit_anim/;
	my $re_anim = "(";
	$re_anim .= "$_|" foreach @anim;
	$re_anim =~ s/\|$/)/;
	
	# Load HTML templates
	open (HTML, "templates/anim_header.html") or die "Couldn't open header: $!\n";
	my @header = <HTML>;
	close HTML;
	open (HTML, "templates/tree_footer.html") or die "Couldn't open footer: $!\n";
	my @footer = <HTML>;
	@footer[2] =~ s/date/gmtime(time)/e; # Generation time
	close HTML;
	open (INDEX, "> $html_dir/animations.html") or die "Couldn't create animations.html: $!\n";
	
	# Load units information
	my @units = glob("$units_dir/*.cfg");
	foreach $unit (@units) {
		my ($id,$tag,$in,$images);
		open (UNIT, "<$unit") or die "Can't open $unit: $!\n";
		while (<UNIT>) {
			chomp; s/^\s*//;
			$id = $1 if /id=(.+)/;
			($tag,$in,$images) = ($1,1,0) if /\[$re_anim\]/;
			$images++ if ($in && /image=/);
			if (m{\[\/$re_anim\]}) {
				$tags{$id}{$tag} += $images;
				($in,$images) = (0,0);
			}
		}
	}
	# Print information
	print INDEX @header;
	foreach $unit (sort keys %tags) {
		print INDEX "<tr>\n\t<td>$unit</td>\n";
		foreach $tag (@anim) {
			print INDEX "\t<td" . ($tags{$unit}{$tag} ? " class=\"yes\">$tags{$unit}{$tag}" : ">~") . "</td>\n";
		}
		print INDEX "</tr>\n";
	}
	print INDEX @footer;
}

# Sub to process the move types data from the units.cfg file
sub ProcessTypes {
	my ($flag, $id);
	open (TYPES, "<$data_dir/$_[0]") or die "Can't open units.cfg: $!\n";
	open (REPORT, ">$report_dir/types.txt") or die "Can't open $unit: $!\n";
	print REPORT "Mov_type\tVariable\tValue\tCategory\n";
	while (<TYPES>) {
		chomp; s/^\s*//;
		$flag = 0 if /^\[\/movetype\]/;
		$flag = 1 if /^\[movetype\]/;
		$id = $1 if (/^name=(.*)/ and $flag);
		
		$flag = 1 if /^\[\/(movement_costs|defense|resistance)\]/;
		if ($flag==2) {
			my ($var,$value) = split /=/;
			print REPORT "$id\t$var\t$value\tMovement\n";
			$types{$id}{movement}{$var} = $value;
		}
		$flag = 2 if /^\[movement_costs\]/;
		if ($flag==3) {
			my ($var,$value) = split /=/;
			print REPORT "$id\t$var\t" . (100-$value) . "\tDefense\n";
			$types{$id}{defense}{$var} = (100-$value);
		}
		$flag = 3 if (/^\[defense\]/ and $flag);
		if ($flag==4) {
			my ($var,$value) = split /=/;
			print REPORT "$id\t$var\t" . (100-$value) . "\tResistance\n";
			$types{$id}{defense}{$var} = (100-$value);
		}
		$flag = 4 if /^\[resistance\]/;
	}
	close TYPES;
}

# Sub to extract the factions from the factions file
sub ProcessFaction {
	my $faction = shift;
	my $unit_list;
	open (FACTION, "<$faction") or die "Can't open $faction: $!\n";
	while (<FACTION>) {
		chomp; s/^\s*//;
		$faction =~ s/factions\/(.+)\.cfg/$1/;
		$unit_list .= "$1," if /^(?:leader|recruit)=(.+)/; # Only get the leaders and recruit, and all in the same line
	}
	$unit_list =~ s/,\s?/\t/g;
	$mp{$_}++ foreach split (/\t/, $unit_list); # Feed the %mp hash with the units
	$faction =~ s(.*\/)(); # Take only the faction name
	print FACT "$faction\t$unit_list\n";
}

# This sub prints the full section of the tree correspoding to the unit
sub PrintUnitTree {
	my $unit = shift; 
	my $fh = shift; # The file to write in is also a parameter of the sub
	unless ($adv_from{$unit}) { # Only process base units, units that don't have an advance from
		print $fh "\n<tr>\n";
		for ($i=0; $i<$units{$unit}[6];$i++) { # Add as many cells as levels (6th element of the unit array)
			if ($spaces{$unit} > 1) { # Here we print the html for the row span
				print $fh "<td rowspan=$spaces{$unit}></td>";
			} else {
				print $fh "<td></td>";
			}
		}
	}
	# Print the unit
	if ($spaces{$unit} > 1) {
		print $fh "<td rowspan=$spaces{$unit}>";
		&PrintUnit ($unit, $fh);
		print $fh "\n</td>";
	} else {
		print $fh "<td>";
		&PrintUnit ($unit, $fh);
		print $fh "\n</td>";
	}
	# If the unit has advances, call this sub again
	if ($adv{$unit}) {
		foreach $i (0..$#{ $adv{$unit} }) {
			&PrintUnitTree ($adv{$unit}[$i], $fh);
		}
	} else {
		print $fh "\n</tr>\n";
	}
}

# Sub to print the unit information in HTML format
sub PrintUnit {
	my $unit = shift; my $fh = shift;
	(my $filename = $units{$unit}[0]) =~ s/\s/_/g;
	print $fh "<a href=$link_back$filename.html><img src=$units{$unit}[8] alt=\"$units{$unit}[0]\"></a>";
	print $fh "<br>\n<a href=$link_back$filename.html>$units{$unit}[0]</a>";
	print $fh "<br>\n<!-- -->$units{$unit}[7]";
	print $fh "<br>\nHP:$units{$unit}[2]";
	print $fh "&nbsp;&nbsp;MP:$units{$unit}[4]";
	print $fh "&nbsp;&nbsp;XP:$units{$unit}[5]";
	$units{$unit}[11] =~ s/(\w)\s/$1<!-- -->\n<!-- -->/; # The code still needs one word to translate per line, surronded by html tags
	print $fh "<br>\n<i>$units{$unit}[11]</i>" if length($units{$unit}[11]) > 0;
	foreach $i (0..$#{ $attacks{$unit} }) { # There may be more than one attack
		print $fh "\n<br>$attacks{$unit}[$i][2]<!-- -->: $attacks{$unit}[$i][3] - $attacks{$unit}[$i][4]";
		print $fh "\n&nbsp;(<!-- -->$attacks{$unit}[$i][6]<!-- -->)" if $attacks{$unit}[$i][6];
	}
}

# Updates the unit files with the Advance From information
sub UpdateAdvanceFrom {
	my @html_units = glob("$html_dir/*.html");
	foreach (@html_units) {
		tie @html, 'Tie::File', $_ or die "Couldn't open file: $!\n";
		my $adv_html;
		for (@html) {
			# Build the advance from html based on the id
			if (/--id=(\d+)--/) { 
				my $id = $1;
				(my $filename = $units{$adv_from{$id}}[0]) =~ s/\s/_/g;
				$adv_html = "<a href=$filename.html>$units{$adv_from{$id}}[0]</a>";
			}
			s/--advancefrom--/$adv_html/;
			s{>([^<]+)</a}{>$adv_to{$1}</a} if ($html_dir =~ /EOM$/); # Change the advance_to name
			last if (/<b>HP: <\/b>/); # Stop processing the file here
		}
		untie @html;
	}
}

# Remove comments in the english files
sub RemoveComments {
	print "Removing comments\n";
	my @html_units = glob("$html_dir/*.html");
	foreach (@html_units) {
		#print "$_\n";
		tie @html, 'Tie::File', $_ or die "Couldn't open $_: $!\n";
		for (@html) {
			s/<!-- -->//g;
			s|\.\./unit|unit|;
			s|race\^||;
			s|unit help\^||;
			s|src=("?)|src=$1../|g if $_[0];
			s{src="../(?!units)}{src="../units/}g if ($html_dir =~ /EOM$/); # Change links for EOM
		}
		untie @html;
	}
}

# Translate the html using the gettext module
sub TranslateUnits {
	my (@countries);
	#use Locale::Maketext::Gettext;
	if ($source) {
		@countries = glob("$wesnoth_dir/po/wesnoth/*.po");
	} else {
		@countries = glob("$wesnoth_dir/po/*");
	}
	
	foreach $country (@countries) {
		my ($flag,$base,%dict);
		$country =~ s(.*\/)(); # Take only the country
		$country =~ s(.po$)(); # If it is the source, we need to stript also the extension
		unless (-e "$html_dir/$country") {mkdir "$html_dir/$country" or die "$country directory cannot be created: $!\n";};
		print "Processing $country\n";
		# If we are using the source, build the dictionary in a different way
		if ($source) {
			# Base data on po/wesnoth dir
			open (POF,"$wesnoth_dir/po/wesnoth/$country.po") or die "Coudn't open wesnoth file for $country: $!\n";
			while (<POF>) {
				$base = $1 if (/msgid "([^"]+)"/);
				$dict{$base} = $1 if (/msgstr "([^"]+)"/);
			}
			close POF;
			# Units data on po/wesnoth-units dir
			open (POF,"$wesnoth_dir/po/wesnoth-units/$country.po") or die "Coudn't open wesnoth file for $country: $!\n";
			while (<POF>) {
				$base = $1 if (/msgid "([^"]+)"/);
				$dict{$base} = $1 if (/msgstr "([^"]+)"/);
			}
			close POF;
			# Faction data on po/wesnoth-units dir
			open (POF,"$wesnoth_dir/po/wesnoth-multiplayer/$country.po") or die "Coudn't open wesnoth file for $country: $!\n";
			while (<POF>) {
				$base = $1 if (/msgid "([^"]+)"/);
				$dict{$base} = $1 if (/msgstr "([^"]+)"/);
			}
			close POF;
		} else {
			my $MOfile = "$wesnoth_dir/po/$country/LC_MESSAGES/wesnoth.mo";
			%dict = read_mo($MOfile);
			# Add female entries
			foreach $key (keys %dict) {
				if ($key =~ /^female/) {
					(my $newkey = $key) =~ s/^female.//;
					$dict{$newkey} = $dict{$key};
				}
			}
			# Add units information
			$MOfile = "$wesnoth_dir/po/$country/LC_MESSAGES/wesnoth-units.mo";
			%dict_extra = read_mo($MOfile);
			foreach $key (keys %dict_extra) {
				$dict{$key} = $dict_extra{$key};
			}
			# Add factions information
			$MOfile = "$wesnoth_dir/po/$country/LC_MESSAGES/wesnoth-multiplayer.mo";
			%dict_extra = read_mo($MOfile);
			foreach $key (keys %dict_extra) {
				$dict{$key} = $dict_extra{$key};
			}
		}
				
		# Process only the html files from the HTML folder
		my @html_units = glob("$html_dir/*.html");
		foreach $unit (@html_units) {
			open (UNIT, "$unit") or die "Couldn't open $unit: $!\n";
			$unit =~ s|/|/$country/|; # Create the same file in the country folder
			open (TRANS, ">$unit") or die "Couldn't create $unit: $!\n";
			
			while ($line = <UNIT>) {
				$line = "<p><a href=../index.html>English</a></p>\n" if $line =~/^<p><a href=fr/; # Change links
				if ($line =~/^<p><a href='EXE/) {$line = "" unless ($country =~ /(ca|fr|it)/);}
				$line = "" if $line =~/^<p><a href='EOM/;
				$line = "" if $line =~ m|<a href=\w+/index.html>|;
				$line =~ s/<html lang="en">/<html lang="$country">/; # Change language tag
				if ($line =~ m/>([^<]+)</g) { # Process the words between html tags
					if ($dict{$1}) {
						$line =~ s/>([^<]+)</>$dict{$1}</;
					}
					# Descriptions
					if (length($1) > 50000) {
						my $patt = $1;
						foreach $key (keys %dict) {
							if ($key =~ /$patt/) { # English descriptions may not be complete (see info_html), a partial match should be unique enough
								$line =~ s/>([^<]+)</>$dict{$key}</; 
								$line =~ s/\n/\n<br>/g; # Change line feeds to <br>
								$line =~ s/<br>[A-Za-z :]+<\/p>/<\/p>/; # Remove the special notes line
								last;
							}
						}
					}
					$line =~ s/<!-- -->//g; # Remove comments
					$line =~ s|race\^||; # Remove race^ in case it is not translated
	
				}
				$line =~ s{src=("?)(?=\w)}{src=$1../}g; # Change the links
				print TRANS $line;
			}
			close TRANS;
			close UNIT;
		}
	}
}
