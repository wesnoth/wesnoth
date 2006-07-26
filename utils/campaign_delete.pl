#!/usr/bin/perl

use wml;
use wml_net;
use strict;

my ($host, $port) = ("campaigns.wesnoth.org", 15002);

$| = 1;

print "Campaign name to be deleted: ";
my $name = <STDIN>;
chomp $name;

print "Passphrase: ";
my $old = <STDIN>;
chomp $old;

my $socket = &wml_net::connect($host,$port);

if (!defined($socket)) {
  print "Error connecting to the campaign server\n";
  exit;
}

&wml_net::write_packet($socket,&wml::read_text( << "EOF" ));
[delete]
name=\"$name\"
passphrase=\"$old\"
[/delete]
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

print "Campaign deleted.\n";
