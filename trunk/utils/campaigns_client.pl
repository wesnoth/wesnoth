#!/usr/bin/perl

use wml;
use wml_net;
use strict;

my $command = 'ls';
my $download = '';
my $download_to = '';
my ($host,$port) = ("campaigns.wesnoth.org",15002);
my @fields = ('name','title','author','size','version','filename','downloads');

while(@ARGV) {
	my $arg = shift @ARGV;
	if($arg eq '--host') {
		$host = shift @ARGV;
	} elsif($arg eq '--port') {
		$port = shift @ARGV;
	} elsif($arg eq '--help') {
		print "Wesnoth Campaigns Client. Connects to the Wesnoth campaign server and performs various operations.
Format: $0 [<options>] <command> [<parameters>]
Commands:
	ls (default): lists the campaigns stored on the server. The fields to be displayed for each campaign may be given as a parameter, as a list of comma-seperated values. Valid fields are name, title, author, size, version, filename, and downloads
	terms: queries the server for terms under which it may be used
	download <campaign name> [<path>]: downloads the given campaign into the directory given by the path. The files and directories created in the process will be printed to standard out.
Options:
	--host: the host to connect to (defaults to campaigns.wesnoth.org)
	--port: the port to connect to (defaults to 15002)
	--help: displays this text and exits
";
	exit;
	} elsif($arg eq 'ls') {
		$command = 'ls';
		my $fields = shift @ARGV;
		if($fields) {
			@fields = split /,/, $fields;
		}
	} elsif($arg eq 'terms') {
		$command = 'terms';
	} elsif($arg eq 'download') {
		$command = 'download';
		$download = shift @ARGV or die "download option must include a campaign to download";
		$download_to = shift @ARGV or '.';
	} else {
		die "Unknown argument '$arg'";
	}
}

my $socket = &wml_net::connect($host,$port);

if($command eq 'ls') {
	&wml_net::write_packet($socket,&wml::read_text('
[request_campaign_list]
[/request_campaign_list]'));
	my $response = &wml_net::read_packet($socket);
	&check_error($response);

	my $campaign_list = &wml::has_child($response,'campaigns') or die "Unrecognized response to request for campaigns";
	my @campaigns = &wml::get_children($campaign_list,'campaign');
	foreach my $campaign (@campaigns) {
		my $first = 1;
		foreach my $field (@fields) {
			print "|" unless $first;
			$first = 0;
			my $value = $campaign->{'attr'}->{$field};
			$value =~ s/\n/ /g;
			$value =~ s/\r//g;
			print $value;
		}

		print "\n";
	}
	
	exit;
} elsif($command eq 'terms') {
	&wml_net::write_packet($socket,&wml::read_text('
[request_terms]
[/request_terms]'));
	my $response = &wml_net::read_packet($socket);
	&check_error($response);
	print &extract_message($response);
	exit;
} elsif($command eq 'download') {
	mkdir $download_to;
	&wml_net::write_packet($socket,&wml::read_text("
[request_campaign]
name=\"$download\"
[/request_campaign]"));
	my $response = &wml_net::read_packet($socket);
	&check_error($response);

	print &unarchive_dir($response,$download_to);
	exit;
}

sub check_error
{
	my ($doc) = @_;
	if(my $error = &wml::has_child($doc,'error')) {
		my $msg = $error->{'attr'}->{'message'};
		die "Server responded with error: $msg";
	}
}

sub extract_message
{
	my ($doc) = @_;
	if(my $error = &wml::has_child($doc,'message')) {
		return $error->{'attr'}->{'message'};
	}

	return "";
}

sub unarchive_dir
{
	my $log = '';
	my ($doc,$dest) = @_;
	my $name = $doc->{'attr'}->{'name'};
	my $path = "$dest/$name";
	mkdir $path;
	$log .= "$path\n";
	foreach my $dir (&wml::get_children($doc,'dir')) {
		$log .= &unarchive_dir($dir,$path);
	}

	foreach my $file (&wml::get_children($doc,'file')) {
		my $filename = "$path/" . $file->{'attr'}->{'name'};
		open FILE, ">$filename";
		binmode(FILE);
		my @chars = split //, $file->{'attr'}->{'contents'};
		while(@chars) {
			my $char = shift @chars;
			if(1 == ord $char) {
				$char = chr(ord(shift @chars) - 1);
			}

			print FILE $char;
		}
		close FILE;

		$log .= "$filename\n";
	}

	return $log;
}

my $doc = &wml_net::read_packet($socket);
die "Expected login" unless &wml::has_child($doc,'mustlogin');

&wml_net::write_packet($socket,&wml::read_text("
[login]
	username=monitor
[/login]"));

print &wml::write_text(&wml_net::read_packet($socket));
print &wml::write_text(&wml_net::read_packet($socket));

close $socket;
