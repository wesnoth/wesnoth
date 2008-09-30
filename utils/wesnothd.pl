use strict;
use wml;
use IO::Socket;
use IO::Select;
use Fcntl;
use Carp qw(confess);
use Getopt::Std;

#my $usage = "Usage: $0 [-p port]";

my %opts = ();
getopts('p:',\%opts);

my $port = '15000';
$port    = $opts{'p'} if $opts{'p'};

my %pending_handshake = ();
my %outgoing_schemas = ();
my %incoming_schemas = ();
my %incoming_bufs = ();
my %connection_nums = ();
my %user_name_socket = ();
my %socket_user_name = ();
my %lobby_players = ();
my %socks_map = ();
my @games = ();
my $current_game_id = 1;
my $current_connection_num = 1;
my $read_set = new IO::Select();

my $login_response = &wml::single_child_doc('mustlogin');
my $join_lobby_response = &wml::single_child_doc('join_lobby');
my $gamelist =  {'name' => 'gamelist', 'children' => [], 'attr' => {}};
my $initial_response = {'name' => '', 'children' => [$gamelist], 'attr' => {}};
my $old_initial_response = &wml::deep_copy($initial_response);

my @disallowed_names = ('server', 'ai', 'player', 'network', 'human', 'admin', 'computer');

my $server_sock = new IO::Socket::INET (
										LocalHost => '',
										LocalPort => $port,
										Proto => 'tcp',
										Listen => 1,
										Reuse => 1
									   ) or die "could not create server sock: $!";

$read_set->add($server_sock);

sub send_data {
	my $doc = shift @_;
	return unless @_;
	print "SENDING: {{{" . &wml::write_text($doc) . "}}}\n";
	foreach my $socket (@_) {
		my $sock = $socks_map{$socket};
		my $packet = &wml::write_binary($outgoing_schemas{$sock}, $doc) . chr 0;
		my $header = pack('N',length $packet);
		$packet = $header . $packet;
		my $res = syswrite($sock, $packet, length $packet);
		&socket_disconnected($sock) unless $res == length $packet;
	}
}

sub send_error($$) {
	my ($sock, $msg) = @_;
	print STDERR "sending error: '$msg'\n";
	my $doc = {'name' => '', 'children' => [{'name' => 'error', 'attr' => {'message' => $msg}}], 'attr' => {}};
	&send_data($doc,$sock);
}

sub send_to_lobby {
	my ($doc, $exclude, @users) = @_;
	foreach my $socket (@users) {
		&send_data($doc, $socks_map{$socket}) unless $socks_map{$socket} eq $exclude;
	}
}

sub socket_connected($) {
	my ($sock) = @_;
	print STDERR "connected: $sock\n";
	$read_set->add($sock);
	$outgoing_schemas{$sock} = [];
	$incoming_schemas{$sock} = [];
	$pending_handshake{$sock} = 1;
	$socks_map{$sock} = $sock;
	my $flags = '';
	fcntl($sock, F_GETFL, $flags) or die "Couldn't get flags: $!";
	$flags |= O_NONBLOCK;
	fcntl($sock, F_SETFL, $flags) or die "Couldn't set flags: $!";
}

sub socket_disconnected($) {
	my ($sock) = @_;
	$sock = $socks_map{$sock};
	print STDERR "disconnected: $sock\n";
	delete $socks_map{$sock};
	delete $outgoing_schemas{$sock};
	delete $incoming_schemas{$sock};
	delete $incoming_bufs{$sock};
	delete $pending_handshake{$sock};
	delete $connection_nums{$sock};
	delete $lobby_players{$sock};
	if(my $username = $socket_user_name{$sock}) {
		delete $user_name_socket{$username};
		delete $socket_user_name{$sock};
		print STDERR "searching for '$username'...\n";
		my $users = $initial_response->{'children'};
		for(my $n = 0; $n != @$users; ++$n) {
			my $user = $users->[$n];
			my $attr = $user->{'attr'};
			if($user->{'name'} eq 'user' and $attr->{'name'} eq $username) {
				my $len = @$users;
				print STDERR "OLD USERS: " . (join ',', @$users) . "\n";
				print STDERR "removing item $n...\n";
				my @new_users = ();
				for(my $m = 0; $m != @$users; ++$m) {
					push @new_users, $users->[$m] unless $m == $n;
				}
				@$users = @new_users;
				print STDERR "NEW USERS: " . (join ',', @$users) . "\n";
				last;
			}
		}

		&sync_lobby();
	}
	$read_set->remove($sock);
	close $sock;
}

sub sync_lobby {
	my $diff = &wml::get_diff($initial_response, $old_initial_response, 'gamelist_diff');
	while(my $sock = each %lobby_players) {
		&send_data($diff, $sock);
	}

	$old_initial_response = &wml::deep_copy($initial_response);
}

sub received_packet($$) {
	my ($sock, $packet) = @_;
	printf STDERR "received %d from $sock\n", length $packet;
	chop $packet;
	my $doc = &wml::read_binary($incoming_schemas{$sock}, $packet);
	my $text = &wml::write_text($doc);
	print STDERR "RECEIVED: $text";

	if(not $socket_user_name{$sock}) {
		if(my $login = &wml::has_child($doc, 'login')) {
			my $attr = $login->{'attr'};
			my $username = $attr->{'username'};
			print STDERR "user attempting to log in as \"$username\"...\n";
			unless($username =~ /^[A-Za-z0-9_]{1,18}$/) {
				&send_error($sock, 'This username is not valid.');
				return;
			}

			foreach my $disallowed (@disallowed_names) {
				if($disallowed eq $username) {
					&send_error($sock, 'This username is disallowed.');
					return;
				}
			}

			if($user_name_socket{$username}) {
				&send_error($sock, 'This username is already taken.');
				return;
			}

			print STDERR "log in okay; telling to join lobby\n";
			&send_data($join_lobby_response, $sock);

			my $users = $initial_response->{'children'};
			push @$users, {'name' => 'user', 'children' => [], 'attr' => {'name' => $username, 'available' => 'yes'}};
			&send_data($initial_response, $sock);
			&sync_lobby();
			$lobby_players{$sock} = 1;
			$user_name_socket{$username} = $sock;
			$socket_user_name{$sock} = $username;
			return;
		} else {
			&send_error($sock, 'You must login first.');
			return;
		}
	} elsif($lobby_players{$sock}) {
		if(my $create_game = &wml::has_child($doc, 'create_game')) {
			$create_game->{'name'} = 'game';
			push @games, &create_game($create_game, $sock);
			delete $lobby_players{$sock};
		} elsif(my $message = &wml::has_child($doc, 'message')) {
			my $attr = $message->{'attr'};
			$attr->{'sender'} = $socket_user_name{$sock};
			&send_to_lobby($doc, $sock, keys %lobby_players);
		} elsif(my $whisper = &wml::has_child($doc, 'whisper')) {
			my $attr = $whisper->{'attr'};
			$attr->{'sender'} = $socket_user_name{$sock};
			my $receiver = $attr->{'receiver'};
			&send_data($doc, $user_name_socket{$receiver});
		#} elsif(my $query = &wml::has_child($doc, 'query')) {
		#	my $type = $query->{'attr'}->{'type'};
		}
		return;
	} else {
		my $game;
		foreach my $g (@games) {
			my $players = $g->{'players'};
			if($players->{$sock}) {
				$game = $g;
				last;
			}
		}

		print STDERR "Illegal socket: '$sock'\n" and return unless $game;
		if($game->{'owner'} eq $sock and
		   my $side = &wml::has_child($doc, 'side')) {
			if(not &wml::has_child($game->{'level'},'side')) {
				#level is not initialized yet, initialize it
				my $level = $game->{'level'};
				my $level_attr = $level->{'attr'};
				my $doc_attr = $doc->{'attr'};
				if($side->{'shroud'} ne 'yes') {
					$level_attr->{'map_data'} = $doc_attr->{'map_data'};
					$level_attr->{'map'} = $doc_attr->{'map'};
				}
				if(my $era = &wml::has_child($doc, 'era')) {
					$level_attr->{'mp_era'} = $era->{'attr'}->{'id'};
				}

				my @mp_flags = qw(mp_use_map_settings mp_village_gold mp_fog mp_shroud experience_modifier mp_countdown mp_countdown_init_time mp_countdown_turn_bonus mp_countdown_reservoir_time mp_countdown_action_bonus hash);
				foreach my $flag (@mp_flags) {
					$level_attr->{$flag} = $doc_attr->{$flag};
				}

				my $games = $gamelist->{'children'};
				push @$games, $level;
				$game->{'description'} = $level;
				$game->{'level'} = $doc;

				&sync_lobby();
			} else {
			}
		}
	}
}

sub handshake($) {
	my ($sock) = @_;
	my $buf = '';
	my $res = sysread($sock, $buf, 4);
	if($res != 4) {
		&socket_disconnected($sock);
	} else {
		my $buf = pack('N', $current_connection_num);
		$connection_nums{$sock} = $current_connection_num++;
		my $res = syswrite($sock, $buf, 4);
		if($res != 4) {
			&socket_disconnected($sock);
		} else {
			delete $pending_handshake{$sock};
			&send_data($login_response, $sock);
		}
	}
}

sub create_game
{
	my ($level, $owner) = @_;
	my $attr = $level->{'attr'};
	$attr->{'id'} = $current_game_id;
	{'owner' => $owner, 'members' => [$owner], 'players' => {$owner => 1}, 'level' => $level, 'id' => $current_game_id++};
}

while(1) {
	my ($rh_set) = IO::Select->select($read_set, undef, undef, 1000);
	foreach my $sock (@$rh_set) {
		if($sock == $server_sock) {
			my $new_sock = $sock->accept();
			&socket_connected($new_sock);
		} elsif($pending_handshake{$sock}) {
			&handshake($sock);
		} else {
			my $buffer = '';
			if(my $buf = $incoming_bufs{$sock}) {
				my $len = $buf->{'length'};
				my $new_buf = '';
				my $res = sysread($sock, $new_buf, $len - length($buf->{'buf'}));
				if(not $res) {
					&socket_disconnected($sock);
				} else {
					$buf->{'buf'} .= $new_buf;
					if(length($buf->{'buf'}) == $len) {
						$buffer = $buf->{'buf'};
						delete $incoming_bufs{$sock};
					}
				}
			} else {
				my $header = '';
				my $res = sysread($sock, $header, 4);
				if($res != 4) {
					&socket_disconnected($sock);
				} else {
					my $len = unpack('N',$header);
					print STDERR "receiving $len chars...\n";
					my $res = sysread($sock, $buffer, $len);
					if($res != $len) {
						$incoming_bufs{$sock} = {'length' => $len, 'buf' => $buffer};
						$buffer = '';
					}
				}
			}

			&received_packet($sock, $buffer) if $buffer;
		}
	}
}
