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
our ($wmltransfile, $pofile) = @ARGV;
our %english = readwml ('data/translations/english.cfg');
our %revenglish;

sub set($$) {
  my ($id, $str) = @_;
  if (defined $english{$id}) {
    print STDERR "WARNING: duplicate def for $id: $str\n";
    return;
  }
  $english{$id}=$str;
  print STDERR "$id=$str\n";
}

# get ids from unit files
our @wmlfiles = glob ("data/units/*.cfg");
foreach my $wmlfile (@wmlfiles) {
  open (WML, $wmlfile) or die "cannot open $wmlfile";
  my ($id, $str);
  while (<WML>) {
    if (m/id\s*=\s*(.*)/) {
      $id = $str = $1;
      $id =~ s/\s+//g;
      set($id,$str);
#     } elsif (m,\[/.*\],) {
#       $id = undef;
    } elsif (m/unit_description\s*=\s*(?:_\s*)\"(.*)\"\s*$/) {
      # single-line
      if (defined $id) {
	set($id . '_description',$1);
      } else {
	print STDERR "No id for unit_description $1\n";
      }
    }
  }
  close WML;
}

our %suffix = (
	       'difficulty_descriptions' => '_difficulties',
	       'cannot_use_message' => '_cannot_use_message',
	      );

# get ids from other wml files
#our @wmlfiles = glob ("data/*.cfg data/scenarios/*/*.cfg");
@wmlfiles = glob ("data/*.cfg");
foreach my $wmlfile (@wmlfiles) {
  open (WML, $wmlfile) or die "cannot open $wmlfile";
  my ($id, $str);
  while (<WML>) {
#    print STDERR $_;
    if (m/id\s*=\s*(.*)/) {
      $id = $1;
      print STDERR "XXX $id\n";
#     } elsif (m,\[/.*\],) {
#       $id = undef;
    } elsif (m/unit_description\s*=\s*(?:_\s*)\"(.*)\"\s*$/) {
      # single-line
      if (defined $id) {
	set($id . '_description',$1);
      } else {
	print STDERR "No id for unit_description $1\n";
      }
    } elsif (m/(difficulty\_descriptions|cannot\_use\_message)\s*=\s*(?:_\s*)?\"(.*)\"\s*$/) {
      # single-line
      if (defined $id) {
	set($id . $suffix{$1},$2);
      } else {
	print STDERR "No id for $1 $2\n";
      }
    } elsif (m/(?:title|message|name)\s*=\s*(?:_\s*)\"(.*)\"\s*$/) {
      set($id,$1);
    }
  }
  close WML;
}

exit;

# reverse the hash to use the strings to find the id
foreach my $key (keys %english) {
  $revenglish{$english{$key}} = $key;
}

# get translations from wml file
our %lang = readwml ($wmltransfile);

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
