#! /usr/bin/perl -wpi.bak

# This script extracts strings from english.cfg and injects them into
# the C++ code tagged for gettext.  Then it produces an english.cfg
# stripped down from those (hopefully) now-useless strings.

# BUGS:
# - should give special treatment to multiline strings
# - should maybe keep the list of stripped strings from english.cfg
# - should maybe report about those @ids items not stripped from english.cfg

use strict;
our %trans;
our @ids;

BEGIN {
  require "utils/wmltrans.pm";
  %trans = readwml ('data/translations/english.cfg');
}

while (m/^(.*)translate_string\(\"([^\"]*)\"\)(.*)/) {
  my $str = $trans{$2};
  unless (defined $str) {
    print STDERR "no translation found for \"$2\"\n";
    $str = $2;
  }
  push @ids, $2;
  $_ = "$1_(\"$str\")$3\n";
}

while (m/^(.*)string_table\[\"([^\"]*)\"\](.*)/) {
  my $str = $trans{$2};
  unless (defined $str) {
    print STDERR "no translation found for \"$2\"\n";
    last;
  }
  push @ids, $2;
  $_ = "$1_(\"$str\")$3\n";
}

END {
  open (OUTFD, ">data/translations/english.cfg.new")
    or die "cannot create new english conf";

  stripfromwml ('data/translations/english.cfg', @ids);

  close OUTFD;
}
