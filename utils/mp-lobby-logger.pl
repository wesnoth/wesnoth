#!/usr/bin/perl

use strict;
use warnings;
use wml;
use IO::Socket;
use POSIX qw(strftime);
use Getopt::Std;
use Data::Dumper;

#my $usage = "Usage: $0 [-gj] [-e count] [-h [host]] [-p [port]] [-t [timestampformat]] [-u username]";

my %opts = ();
getopts('gje:h:p:t:u:',\%opts);

my $USERNAME = 'log';
$USERNAME = $opts{'u'} if $opts{'u'};
my $HOST = '127.0.0.1';
$HOST = 'server.wesnoth.org' if exists $opts{'h'};
$HOST = $opts{'h'} if $opts{'h'};
my $PORT = '15000';
$PORT = '14999' if exists $opts{'p'};
$PORT = $opts{'p'} if $opts{'p'};
my $timestamp = "%Y%m%d %T ";
$timestamp = $opts{'t'} if $opts{'t'};
$timestamp = '' unless exists $opts{'t'};
my $showjoins = $opts{'j'};
my $showgames = $opts{'g'};

my $LOGIN_RESPONSE = "[login]\nusername=\"$USERNAME\"\n[/login]";
my $VERSION_RESPONSE = "[version]\nversion=\"test\"\n[/version]";
my @usernamelist = ();
my @gamelist = ();
my %outgoing_schemas = ();
my %incoming_schemas = ();


sub connect {
	my ($host,$port) = @_;
	my $sock = IO::Socket::INET->new(PeerAddr => $host, PeerPort => $port, Proto => 'tcp', Type => SOCK_STREAM)
		or die "Could not connect to $host:$port: $@\n";
	print $sock pack('N',0) or die "Could not send initial handshake";

	my $connection_num = "";
	read $sock, $connection_num, 4;
	die "Could not read connection number" if length($connection_num) != 4;

	$outgoing_schemas{$sock} = [];
	$incoming_schemas{$sock} = [];
	return $sock;
}

sub disconnect {
	my ($sock) = @_;
	delete($outgoing_schemas{$sock});
	delete($incoming_schemas{$sock});
	close $sock;
}

sub read_packet {
	my ($sock) = @_;
	my $buf = '';
	read $sock, $buf, 4;
	die "Could not read length" if length($buf) != 4;

	my $len = unpack('N',$buf);

	my $res = "\0" * $len;
	my $count = 0;
	while($len > $count) {
		$buf = '';
		my $bytes = $len - $count;
		read $sock, $buf, $bytes or die "Error reading socket: $!";
		substr($res, $count, length $buf) = $buf;
		$count += length $buf;
	}

	$res = substr($res,0,$len-1);

	return &wml::read_binary($incoming_schemas{$sock},$res);
}

sub write_packet {
	my ($sock,$doc) = @_;
	my $data = &wml::write_binary($outgoing_schemas{$sock},$doc);
	$data .= chr 0;
	my $header = pack('N',length $data);
	print $sock "$header$data" or die "Error writing to socket: $!";
}

sub write_bad_packet {
	my ($sock, $doc) = @_;
	my $data = &wml::write_binary($outgoing_schemas{$sock},$doc);
	$data .= chr 0;
	my $header = pack('N', (length $data) * 2);
	print $sock "$header$data" or die "Error writing to socket: $!";
}


sub timestamp {
	if ($timestamp) {
		return strftime($timestamp, localtime());
	}
}

sub login {
	my $sock = shift;
	my $response = &read_packet($sock);
	# server asks for the version string or tells us to login right away
	if (&wml::has_child($response, 'version')) {
		&write_packet($sock, &wml::read_text($VERSION_RESPONSE));
		$response = &read_packet($sock);
		# server asks for a login
		if (&wml::has_child($response, 'mustlogin')) {
			&write_packet($sock, &wml::read_text($LOGIN_RESPONSE));
		} elsif (my $error = &wml::has_child($response, 'error')) {
			print STDERR "Error: $error->{'attr'}->{'message'}.\n" and die;
		} else {
			print STDERR "Error: Server didn't ask us to log in and gave no error.\n" . Dumper($response) and die;
		}
	} elsif (my $error = &wml::has_child($response, 'error')) {
		print STDERR "Error: $error->{'attr'}->{'message'}.\n" and die;
	} elsif (&wml::has_child($response, 'mustlogin')) {
		&write_packet($sock, &wml::read_text($LOGIN_RESPONSE));
	} else {
		print STDERR "Error: Server didn't ask for version or login and gave no error.\n" . Dumper($response) and die;
	}

	# server sends the join lobby response
	$response = &read_packet($sock);
	if (&wml::has_child($response, 'join_lobby')) {
	} elsif (my $error = &wml::has_child($response, 'error')) {
		print STDERR "Error: $error->{'attr'}->{'message'}.\n" and die;
	} else {
		print STDERR "Error: Server didn't ask us to join the lobby and gave no error.\n" . Dumper($response) and die;
	}

	# server sends the initial list of games and players
	$response = &read_packet($sock);
	#print STDERR Dumper($response);
	if (&wml::has_child($response, 'gamelist')) {
		foreach (@ {$response->{'children'}}) {
			if ($_->{'name'} eq 'gamelist') {
				foreach (@ {$_->{'children'}}) {
					$gamelist[@gamelist] = $_->{'attr'}->{'name'};
				}
			} elsif ($_->{'name'} eq 'user') {
				$usernamelist[@usernamelist] = $_->{'attr'}->{'name'};
			}
		}
	} elsif (my $error = &wml::has_child($response, 'error')) {
		print STDERR "Error: $error->{'attr'}->{'message'}.\n" and die;
	} else {
		print STDERR "Error: Server didn't send the initial gamelist and gave no error.\n" . Dumper($response) and die;
	}
	print "usernames: @usernamelist\n" if $showjoins;
	print "games: \"" . join("\" \"",@gamelist) . "\"\n" if $showgames;
}

# make several connections and send packets with a wrong length then sleep indefinitely
if (my $count = $opts{'e'}) {
	for (1..$count) {
		my $socket = &connect($HOST,$PORT);
		&write_bad_packet($socket, &wml::read_text($VERSION_RESPONSE));
	}
	sleep();
}



print STDERR "Connecting to $HOST:$PORT as $USERNAME.\n";
my $socket = &connect($HOST,$PORT);
defined($socket) or die "Error: Can't connect to the server.";

login($socket);

while (1) {
	my $response = &read_packet($socket);
	foreach (@ {$response->{'children'}}) {
		if ($_->{'name'} eq 'message') {
			my $sender = $_->{'attr'}->{'sender'};
			my $message = $_->{'attr'}->{'message'};
			if ($message =~ s,^/me,,) {
				print STDERR &timestamp . "* $sender$message\n";
			} else {
				print STDERR &timestamp . "<$sender> $message\n";
			}
		} elsif ($_->{'name'} eq 'whisper') {
			my $sender = $_->{'attr'}->{'sender'};
			my $message = $_->{'attr'}->{'message'};
			if ($message =~ s,^/me,,) {
				print STDERR &timestamp . "*$sender$message*\n";
			} else {
				print STDERR &timestamp . "*$sender* $message\n";
			}
		} elsif ($_->{'name'} eq 'gamelist_diff') {
			foreach (@ {$_->{'children'}}) {
				my $index = $_->{'attr'}->{'index'};
				if ($_->{'name'} eq 'insert_child') {
					if (my $user = &wml::has_child($_, 'user')) {
						my $username = $user->{'attr'}->{'name'};
						print STDERR &timestamp . "--> $username has logged on. ($index)\n" if $showjoins;
						$usernamelist[@usernamelist] = $username;
						#print "usernames: @usernamelist\n";
					} else {
						print STDERR "[gamelist_diff]:" . Dumper($_);
					}
				} elsif ($_->{'name'} eq 'delete_child') {
					if (my $user = &wml::has_child($_, 'user')) {
						print STDERR &timestamp . "<-- $usernamelist[$index] has logged off. ($index)\n" if $showjoins;
						splice(@usernamelist,$index,1);
						#print "usernames: @usernamelist\n";
					} else {
						print STDERR "[gamelist_diff]:" . Dumper($_);
					}
				} elsif ($_->{'name'} eq 'change_child') {
					if (my $user = &wml::has_child($_, 'user')) {
						foreach (@ {$user->{'children'}}) {
							#my $userindex = $_->{'attr'}->{'index'};   #there's no index it seems, the gamelistindex would be nice..
							if ($_->{'name'} eq 'insert') {
								if ($_->{'attr'}->{'available'} eq "yes") {
									print STDERR &timestamp . "++> $usernamelist[$index] has left a game.\n" if $showjoins and $showgames;
								} elsif ($_->{'attr'}->{'available'} eq "no") {
									print STDERR &timestamp . "<++ $usernamelist[$index] has joined a game.\n" if $showjoins and $showgames;
								}
							} elsif ($_->{'name'} eq 'delete') {
							} else {
								print STDERR "[gamelist_diff][change_child][user]:" . Dumper($_);
							}
						}
						#print STDERR "[gamelist_diff][change_child]:" . Dumper($user);
					} elsif (my $gamelist = &wml::has_child($_, 'gamelist')) {
						foreach (@ {$gamelist->{'children'}}) {
							my $gamelistindex = $_->{'attr'}->{'index'};
							if ($_->{'name'} eq 'insert_child') {
								if (my $game = &wml::has_child($_, 'game')) {
									my $gamename = $game->{'attr'}->{'name'};
									print STDERR &timestamp . "+++ A new game has been created: \"$gamename\" ($gamelistindex).\n" if $showgames;
									$gamelist[@gamelist] = $gamename;
								} else {
									print "[gamelist_diff][change_child][gamelist]:" . Dumper($_);
								}
							} elsif ($_->{'name'} eq 'delete_child') {
								if (my $game = &wml::has_child($_, 'game')) {
									print STDERR &timestamp . "--- A game has ended: \"$gamelist[$gamelistindex]\". ($gamelistindex)\n" if $showgames;
								} else {
									print STDERR "[gamelist_diff][change_child][gamelist]:" . Dumper($_);
								}
							} elsif ($_->{'name'} eq 'change_child') {
#								if (my $game = &wml::has_child($_, 'game')) {
#									print STDERR &timestamp . "Something changed in a game. ($gamelistindex)\n" if $showgames;
#								} else {
#									print STDERR "[gamelist_diff][change_child][gamelist]:" . Dumper($_);
#								}
							} else {
								print STDERR "[gamelist_diff][change_child][gamelist]:" . Dumper($_);
							}
						}
					} else {
						print STDERR "[gamelist_diff]:" . Dumper($_);
					}
				} else {
					print STDERR "[gamelist_diff]:" . Dumper($_);
				}
			}
		# [observer] and [observer_quit] should be deprecated they are redundant to parts of [gamelist_diff]
		} elsif ($_->{'name'} eq 'observer') {
#			my $username = $_->{'attr'}->{'name'};
#			print &timestamp . "++> $username has joined the lobby.\n";
		} elsif ($_->{'name'} eq 'observer_quit') {
#			my $username = $_->{'attr'}->{'name'};
#			print &timestamp . "<++ $username has left the lobby.\n";
		} else {
			print STDERR Dumper($_);
		}
	}

}
print "Connection closed.\n\n"

