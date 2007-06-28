#!/usr/bin/perl

use strict;
use warnings;
use wml_net;
use POSIX qw(strftime);
use Data::Dumper;

my $usage = '$0 username [server] [port]';
if (@ARGV > 3 or @ARGV == 0) {die "Usage: $usage\n";}

my $USERNAME = $ARGV[0];
my $HOST = 'server.wesnoth.org';
my $PORT = '15000';
if ($ARGV[1]) {$HOST = $ARGV[1];}
if ($ARGV[2]) {$PORT = $ARGV[2];}
my $LOGIN_RESPONSE = "[login]\nusername=\"$USERNAME\"\n[/login]";
my $VERSION_RESPONSE = "[version]\nversion=\"test\"\n[/version]";
my @usernamelist = ();


print STDERR "Connecting to $HOST:$PORT as $USERNAME.\n";
my $socket = eval {&wml_net::connect($HOST,$PORT)};
$@ eq '' or die "Error: $@";
defined($socket) or die "Error: Can't connect to the server.";


sub read_server_response {
	my $response = eval {
		&wml_net::read_packet($socket)
	};
	$@ eq '' or die "Error: $@";
	#print STDERR Dumper($response);
	return $response;
}

sub write_to_server {
	my $message = shift;
	#print STDERR $message;
	eval {
		&wml_net::write_packet($socket,&wml::read_text($message))
	};
	$@ eq '' or die "Error: $@";
}

sub login {
	my $response = read_server_response();
	# server asks for the version string or tells us to login right away
	if ($response->{'children'}[0]->{'name'} eq 'version') {
		write_to_server($VERSION_RESPONSE);
		$response = read_server_response();
		# server asks for a login
		if ($response->{'children'}[0]->{'name'} eq 'mustlogin') {
	        	write_to_server($LOGIN_RESPONSE);
		} elsif ($response->{'children'}[0]->{'name'} eq 'error') {
		        print STDERR "Error: $response->{'children'}[0]->{'attr'}->{'message'}.\n" and die;
		} else {
		        print STDERR "Error: Server didn't ask us to log in and gave no error.\nDumper($response)" and die;
		}
	} elsif ($response->{'children'}[0]->{'name'} eq 'error') {
		print STDERR "Error: $response->{'children'}[0]->{'attr'}->{'message'}.\n" and die;
	} elsif ($response->{'children'}[0]->{'name'} eq 'mustlogin') {
		write_to_server($LOGIN_RESPONSE);
	} else {
		print STDERR "Error: Server didn't ask for version or login and gave no error.\nDumper($response)" and die;
	}

	# server sends the join lobby response
	$response = read_server_response();
	if ($response->{'children'}[0]->{'name'} eq 'join_lobby') {
	} elsif ($response->{'children'}[0]->{'name'} eq 'error') {
		print STDERR "Error: $response->{'children'}[0]->{'attr'}->{'message'}.\n" and die;
	} else {
		print STDERR "Error: Server didn't ask us to join the lobby and gave no error.\nDumper($response)" and die;
	}

	# server sends the initial list of games and players
	$response = read_server_response();
	#print STDERR Dumper($response);
	if ($response->{'children'}[0]->{'name'} eq 'gamelist') {
		foreach (@ {$response->{'children'}}) {
			if ($_->{'name'} eq 'user') {
				$usernamelist[@usernamelist] = $_->{'attr'}->{'name'};
			}
		}
	} elsif ($response->{'children'}[0]->{'name'} eq 'error') {
		print STDERR "Error: $response->{'children'}[0]->{'attr'}->{'message'}.\n" and die;
	} else {
		print STDERR "Error: Server didn't send the initial gamelist and gave no error.\nDumper($response)" and die;
	}
	print STDERR "usernames: @usernamelist\n";
}

login();

while (1) {                         
        my $response = read_server_response();
        # print only chat messages  
        foreach (@ {$response->{'children'}}) {
                if ($_->{'name'} eq 'message') {
                        print STDERR strftime "%Y%m%d %T ", localtime();
                        # /me actions
                        if ($_->{'attr'}->{'message'} =~ s,^/me,,) {
                                print STDERR "* " . $_->{'attr'}->{'sender'} . "" . $_->{'attr'}->{'message'} . "\n";
                        } else {    
                                print STDERR "<" . $_->{'attr'}->{'sender'} . "> " . $_->{'attr'}->{'message'} . "\n";
                        }           
                } elsif ($_->{'name'} eq 'whisper') {
                        print STDERR strftime "%Y%m%d %T ", localtime();
                        print STDERR "*" . $_->{'attr'}->{'sender'} . "* " . $_->{'attr'}->{'message'} . "\n";
                } elsif ($_->{'name'} eq 'gamelist_diff') {
                        foreach (@ {$_->{'children'}}) {
                                if ($_->{'name'} eq 'insert_child') {
                                        if ($_->{'children'}[0]->{'name'} eq 'user') {
                                                print STDERR strftime "%Y%m%d %T ", localtime();
                                                print STDERR "--> $_->{'children'}[0]->{'attr'}->{'name'} has logged on. ($_->{'attr'}->{'index'})\n";
                                                $usernamelist[@usernamelist] = $_->{'children'}[0]->{'attr'}->{'name'};
                                                #print STDERR "usernames: @usernamelist\n";
                                        }
                                        #print STDERR Dumper($_);
                                } elsif ($_->{'name'} eq 'delete_child' and $_->{'children'}[0]->{'name'} eq 'user') {
                                        print STDERR strftime "%Y%m%d %T ", localtime();
                                        my $index = $_->{'attr'}->{'index'};
                                        print STDERR "<-- $usernamelist[$index] has logged off. ($index)\n";
                                        splice(@usernamelist,$index,1);
                                        #print STDERR "usernames: @usernamelist\n";
                                }   
                        }           
                } else {            
                        #print STDERR Dumper($_);
                }                   
        }                           
                                    
}
#print STDERR "Connection closed.\n\n"

