# -*- perl -*-

sub readwml {
  my ($file) = @_;
  open (TRANS, $file) or die "cannot open $file";

  my (%trans, $key);

  while (<TRANS>) {
    next if m/^\s*\#/ and !defined $key;
    if (m/(\S+)\s*=\s*(?:_\s*)?\"(.*)\"\s*(?:\#.*)?$/) {
      # single-line
      die "nested key" if defined $key;

      $trans{$1} = $2;

    } elsif (m/(\S+)\s*=\s*(?:_\s*)?\s*\"(.*)/) {
      # start of multi-line
      die "nested key" if defined $key;

      $key = $1;
      $trans{$key} = $2 . "\n";

    } elsif (m/(.*)\"\s*(?:\#.*)?$/) {
      # end of multi-line
      die "end of string without a key" unless defined $key;

      $trans{$key} .= $1;
      $key = undef;
    } elsif (defined $key) {
      # part of multi-line
      $trans{$key} .= $_;
    }

  }

  close TRANS;

  return %trans;
}

sub stripfromwml {
  my ($file,@ids) = @_;

  open (TRANS, $file) or die "cannot open $file";

  my $key;

  while (<TRANS>) {
    if (m/(\S+)\s*=\s*\"(.*)\"\s*$/) {
      die "nested key" if defined $key;

      print main::OUTFD $_ unless grep ({ $_ eq $1 } @ids);

    } elsif (m/(\S+)\s*=\s*\"(.*)/) {
      die "nested key" if defined $key;

      $key = $1;
      print main::OUTFD $_ unless grep ({ $_ eq $key } @ids);

    } elsif (m/(.*)\"\s*$/) {
      die "end of string without a key" unless defined $key;

      print main::OUTFD $_ unless grep ({ $_ eq $key } @ids);
      $key = undef;
    } elsif (defined $key) {
      print main::OUTFD $_ unless grep ({ $_ eq $key } @ids);
    } else {
      print main::OUTFD $_;
    }

  }

  close TRANS;
}

sub raw2postring {
  my $str = shift;

  $str =~ s/\"\"/\\\"/mg;
  $str =~ s/^(.*)$/"$1\\n"/mg;
  $str =~ s/\n$/\n"\\n"/mg;
  $str =~ s/\\n\"$/\"\n/g;

  return $str;
}

1;
