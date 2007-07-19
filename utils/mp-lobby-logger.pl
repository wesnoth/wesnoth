#!/usr/bin/perl

use strict;
use warnings;
use wml;
use IO::Socket;
use POSIX qw(strftime);
use Getopt::Std;
use Data::Dumper;

#my $usage = "Usage: $0 [-e count] [-h host] [-p port] [username]";

my %opts = ();
getopts('e:h:p:',\%opts);

my $USERNAME = 'log';
my $HOST = '127.0.0.1';
my $PORT = '15000';
if ($ARGV[0]) {$USERNAME = $ARGV[0];}
if ($opts{'h'}) {$HOST = $opts{'h'};}
if ($opts{'p'}) {$PORT = $opts{'p'};}
my $LOGIN_RESPONSE = "[login]\nusername=\"$USERNAME\"\n[/login]";
my $VERSION_RESPONSE = "[version]\nversion=\"test\"\n[/version]";
my @usernamelist = ();
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
			if ($_->{'name'} eq 'user') {
				$usernamelist[@usernamelist] = $_->{'attr'}->{'name'};
			}
		}
	} elsif (my $error = &wml::has_child($response, 'error')) {
		print STDERR "Error: $error->{'attr'}->{'message'}.\n" and die;
	} else {
		print STDERR "Error: Server didn't send the initial gamelist and gave no error.\n" . Dumper($response) and die;
	}
	print STDERR "usernames: @usernamelist\n";
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
				my $index = $_->{'attr'}->{'index'};
				if ($_->{'name'} eq 'insert_child') {
					if (my $user = &wml::has_child($_, 'user')) {
						my $username = $user->{'attr'}->{'name'};
						print STDERR strftime "%Y%m%d %T ", localtime();
						print STDERR "--> $username has logged on. ($index)\n";
						$usernamelist[@usernamelist] = $username;
						#print STDERR "usernames: @usernamelist\n";
					}
					#print STDERR Dumper($_);
				} elsif ($_->{'name'} eq 'delete_child' and &wml::has_child($_, 'user')) {
					print STDERR strftime "%Y%m%d %T ", localtime();
					print STDERR "<-- $usernamelist[$index] has logged off. ($index)\n";
					splice(@usernamelist,$index,1);
					#print STDERR "usernames: @usernamelist\n";
				} else {
					#print STDERR Dumper($_);
				}
			}
		} elsif ($_->{'name'} eq 'create_game') {
			#print STDERR Dumper($_);
		} else {
			#print STDERR Dumper($_);
		}
	}

}
#print STDERR "Connection closed.\n\n"

