#! /usr/bin/perl -w

use strict;

require "utils/wmltrans.pm";

our ($wmlfile, $pofile) = @ARGV;
our %english = readwml ('data/translations/english.cfg');
#our %lang = readwml ($wmlfile);

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


sub processentry {
  my ($curid, $curmsg) = @_;

  # lookup

  print "msgid $curid";
  print "msgstr $curmsg";

  $curid = undef; $curmsg = undef;
}
