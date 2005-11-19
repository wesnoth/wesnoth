#!/usr/bin/perl

use wml;
use wml_net;
use strict;

my ($host, $port) = ("campaigns.wesnoth.org", 15002);

$| = 1;

print "Campaign name to change passphrase of: ";
my $name = <STDIN>;
chomp $name;

print "Old passphrase: ";
my $old = <STDIN>;
chomp $old;

print "New passphrase: ";
my $new = <STDIN>;
chomp $new;

my $socket = &wml_net::connect($host,$port);

if (!defined($socket)) {
  print "Error connecting to the campaign server\n";
  exit;
}

&wml_net::write_packet($socket,&wml::read_text( << "EOF" ));
[change_passphrase]
name=\"$name\"
passphrase=\"$old\"
new_passphrase=\"$new\"
[/change_passphrase]
EOF

my $response = &wml_net::read_packet($socket);

if (!defined($response)) {
  print "Error accessing the campaign server.\n";
  exit;
}

if (my $error = &wml::has_child($response, 'error')) {
  printf "Error: %s\n",  $error->{'attr'}->{'message'};
  exit;
}

print "Passphrase updated.\n";
