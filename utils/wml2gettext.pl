#! /usr/bin/perl -wni.bak

# This script extracts strings from english.cfg and injects them into
# the C++ code tagged for gettext.  Then it produces an english.cfg
# stripped down from those (hopefully) now-useless strings.

# BUGS:
# - should maybe keep the list of stripped strings from english.cfg
# - should maybe report about those @ids items not stripped from english.cfg

use strict;
our %trans;
our @ids;

BEGIN {
  require "utils/wmltrans.pm";
  %trans = readwml ('data/translations/english.cfg');
}

while (m/translate_string\(\"([^\"]*)\"\)/m) {
  my $id = $1;
  unless (defined $trans{$id}) {
    print STDERR "no translation found for \"$id\"\n";
    $trans{$id} = $id;
  }
  $trans{$id} =~ s/$/\\n\\/mg; chop $trans{$id}; chop $trans{$id}; chop $trans{$id};
  my $str = $trans{$id};
  push @ids, $id;
  s/translate_string\(\"$id\"\)/_(\"$str\")/mg;
}

while (m/string_table\[\"([^\"]*)\"\]/m) {
  my $id = $1;
  unless (defined $trans{$id}) {
    print STDERR "no translation found for \"$id\"\n";
    last;
  }
  $trans{$id} =~ s/$/\\n\\/mg; chop $trans{$id}; chop $trans{$id}; chop $trans{$id};
  my $str = $trans{$id};
  push @ids, $id;
  s/string_table\[\"$id\"\]/_(\"$str\")/mg;
}

print;

END {
  open (OUTFD, ">data/translations/english.cfg.new")
    or die "cannot create new english conf";

  stripfromwml ('data/translations/english.cfg', @ids);

  close OUTFD;
}
