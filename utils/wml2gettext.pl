#! /usr/bin/perl -wp

use strict;
our %trans;

BEGIN {
  require "utils/wmltrans.pm";
  %trans = readwml ('data/translations/english.cfg');
}

while (m/^(.*)translate_string\(\"([^\"]*)\"\)(.*)/) {
  my $str = $trans{$2};
  unless (defined $str) {
    print STDERR "no translation found for \"$2\"\n";
    last;
  }
  $_ = "$1_(\"$str\")$3\n";
}

while (m/^(.*)string_table\[\"([^\"]*)\"\](.*)/) {
  my $str = $trans{$2};
  unless (defined $str) {
    print STDERR "no translation found for \"$2\"\n";
    last;
  }
  $_ = "$1_(\"$str\")$3\n";
}
