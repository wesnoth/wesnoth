# -*- perl -*-

sub readwml {
  my ($file) = @_;
  open (TRANS, $file) or die "cannot open $file";

  my (%trans, $key);

  while (<TRANS>) {
    if (m/(\S+)\s*=\s*\"(.*)\"\s*$/) {
      die "nested key" if defined $key;

      $trans{$1} = $2;

    } elsif (m/(\S+)\s*=\s*\"(.*)/) {
      die "nested key" if defined $key;

      $key = $1;
      $trans{$key} = $2 . "\n";

    } elsif (m/(.*)\"\s*$/) {
      die "end of string without a key" unless defined $key;

      $trans{$key} .= $1;
      $key = undef;
    } else {
      $trans{$key} .= $_ if defined $key;
    }

  }

  return %trans;
}

1;
