#! /usr/bin/perl -w

# WARNING:
#  DO NOT commit po-files convverted with this script yet.  There is
#  still some work to do !

# USAGE:
#  ./utils/wml2po.pl data/translations/XXXXXXX.cfg po/XX.po

# TODO:
# - how to permanently get rid of this supurious c-format keyword ?

use strict;

## process wml files

require "utils/wmltrans.pm";

# get some id->english from english.cfg
our ($wmlfile, $pofile) = @ARGV;
our %english = readwml ('data/translations/english.cfg');
our %revenglish;

# reverse the hash to use the strings to find the id
foreach my $key (keys %english) {
  $revenglish{$english{$key}} = $key;
}

# get translations from wml file
our %lang = readwml ($wmlfile);

## process pofile, filling holes when we can

open (POFILE, $pofile) or die "cannot open $pofile";

my ($curid, $curmsg);
while (<POFILE>) {
  if (m/^msgid (".*")$/) {
    $curid = "$1\n"; $curmsg = undef;
  } elsif (m/^msgstr (".*")$/) {
    $curmsg = "$1\n";
  } elsif (m/^(".*")$/) {
    if (!defined $curmsg) {
      $curid .= "$_";
    } else {
      $curmsg .= "$_";
    }
  } elsif (m/^$/) {
    if (defined $curid and defined $curmsg) {
      processentry ($curid, $curmsg);
      print "\n";
    } else {
      print;
    }
  } else {
    print;
  }
}

# last entry does not have an empty line afterwards
if (defined $curid and defined $curmsg) {
  processentry ($curid, $curmsg);
}


## Below is utility functions only - end of processing

sub processentry {
  my ($curid, $curmsg) = @_;
  my $output;
  my $touched = 0;

  # lookup

  if ($curmsg eq "\"\"\n") {
    my $id = $revenglish{po2rawstring($curid)};
    if (defined $id) {
      my $trans = $lang{$id};
      if (defined $trans) {
	$output = raw2postring($trans);
	$touched = 1;
      } else {
	# printf STDERR "WARNING: no translation found for $id - setting to empty\n";
	$output = raw2postring("");
      }
    } else {
      # this usually denotes limitations in this script
      printf STDERR "WARNING: no id found (setting translation to empty) for $curid\n";
      $output = raw2postring("");
    }
  } else {
    $output = $curmsg;
  }

  print "#, fuzzy\n" if $touched;
  print "msgid $curid";
  print "msgstr $output";

  $curid = undef; $curmsg = undef;
}

sub raw2postring {
  my $str = shift;

  $str =~ s/^(.*)$/"$1\\n"/mg;
  $str =~ s/\\n\"$/\"\n/g;

  return $str;
}

sub po2rawstring {
  my $str = shift;
  my @lines = split (/\n/, $str);

  $str = "";
  foreach my $line (@lines) {
    $line =~ m/"(.*)"/;
    $str .= $1;
  }

  return $str;
}
