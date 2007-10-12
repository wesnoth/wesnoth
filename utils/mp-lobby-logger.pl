#!/usr/bin/perl

use strict;
use warnings;
use wml;
use IO::Socket;
use POSIX qw(strftime);
use Getopt::Std;
use Data::Dumper;

#my $usage = "Usage: $0 [-dgjnqs] [-e count] [-l logfile] [-h [host]] [-p [port]] [-t [timestampformat]] [-u username]";

my %opts = ();
getopts('dgjqsl:ne:h:p:t:u:',\%opts);

my $USERNAME = 'log';
$USERNAME    = $opts{'u'} if $opts{'u'};
my $HOST = '127.0.0.1';
$HOST    = 'server.wesnoth.org' if exists $opts{'h'};
$HOST    = $opts{'h'} if $opts{'h'};
my $PORT = '15000';
$PORT    = '14999' if exists $opts{'p'};
$PORT    = $opts{'p'} if $opts{'p'};
my $logtimestamp = "%Y%m%d %T ";
my $timestamp = "%Y%m%d %T ";
$timestamp    = $opts{'t'} if $opts{'t'};
$timestamp    = '' unless exists $opts{'t'};
my $writestats = $opts{'s'};
my $statsfile = "/tmp/wesnoth.stats_" . $HOST . "_" . $PORT;
my $showjoins = $opts{'j'};
my $showgames = $opts{'g'};
my $showturns = $opts{'n'};
my $dumpunknown = $opts{'d'};
my $quiet     = $opts{'q'};
my $logfile   = $opts{'l'};
if (exists $opts{'l'}) {open(LOG, ">> $logfile") or die "can't open $logfile: $!";}
select LOG; $| = 1;

my $LOGIN_RESPONSE = "[login]\nusername=\"$USERNAME\"\n[/login]";
my $VERSION_RESPONSE = "[version]\nversion=\"test\"\n[/version]";
my @users = ();
my @games = ();
my %outgoing_schemas = ();
my %incoming_schemas = ();


sub connect($$) {
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

sub disconnect($) {
	my ($sock) = @_;
	delete($outgoing_schemas{$sock});
	delete($incoming_schemas{$sock});
	close $sock;
}

sub read_packet($) {
	my ($sock) = @_;
	my $buf = '';
	read $sock, $buf, 4;
	die "Could not read length" if length($buf) != 4;

	my $len = unpack('N',$buf);

	my $res = "\0";
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

sub write_packet($$) {
	my ($sock,$doc) = @_;
	my $data = &wml::write_binary($outgoing_schemas{$sock},$doc);
	$data .= chr 0;
	my $header = pack('N',length $data);
	print $sock "$header$data" or die "Error writing to socket: $!";
}

sub write_bad_packet($$) {
	my ($sock, $doc) = @_;
	my $data = &wml::write_binary($outgoing_schemas{$sock},$doc);
	$data .= chr 0;
	my $header = pack('N', (length $data) * 2);
	print $sock "$header$data" or die "Error writing to socket: $!";
}


sub timestamp {
	return strftime($timestamp, localtime());
}

sub logtimestamp {
	return strftime($logtimestamp, localtime());
}

sub login($) {
	my $sock = shift;
	my $response;
	my $tries = 0;
	my $logged_in = 'no';
	until($logged_in eq 'yes')  {
		$tries++;
		$response = &read_packet($sock);
		#print STDERR Dumper($response);
		# server asks for the version string
		if (&wml::has_child($response, 'version')) {
			$logged_in = 'version';
			$tries = 0;
			&write_packet($sock, &wml::read_text($VERSION_RESPONSE));
		# server asks for a login name
		} elsif (&wml::has_child($response, 'mustlogin')) {
			$logged_in = 'mustlogin';
			$tries = 0;
			&write_packet($sock, &wml::read_text($LOGIN_RESPONSE));
		# server sends the join lobby response
		} elsif (&wml::has_child($response, 'join_lobby')) {
			$logged_in = 'join_lobby';
			$tries = 0;
		# server sends the initial list of games and players
		} elsif (&wml::has_child($response, 'gamelist')) {
			$logged_in = 'yes';
			foreach (@ {$response->{'children'}}) {
				if ($_->{'name'} eq 'gamelist') {
					@games = @ {$_->{'children'}};
				} elsif ($_->{'name'} eq 'user') {
					$users[@users] = $_;
				}
			}
		} elsif (my $error = &wml::has_child($response, 'error')) {
			print STDERR "Error: $error->{'attr'}->{'message'}.\n" and die;
		} else {
			if ($logged_in eq 'no') {
				print STDERR "Warning: Server didn't ask for version or login yet and gave no error. ($tries)\n" . Dumper($response);
			} elsif ($logged_in eq 'version') {
				print STDERR "Warning: Server didn't ask us to log in yet and gave no error. ($tries)\n" . Dumper($response);
			} elsif ($logged_in eq 'mustlogin') {
				print STDERR "Warning: Server didn't ask us to join the lobby yet and gave no error. ($tries)\n" . Dumper($response);
			} elsif ($logged_in eq 'join_lobby') {
				print STDERR "Warning: Server didn't send the initial gamelist yet and gave no error. ($tries)\n" . Dumper($response);
			}
			# give up after 10 unknown packets
			print STDERR "Giving up...\n" if $tries == 10 and die;
		}
	}
	my $userlist = @users . " users:";
	foreach (@users) {
		$userlist .= " " . $_->{'attr'}->{'name'};
	}
	$userlist .= "\n";
	print STDERR $userlist if $showjoins;
	print LOG    $userlist if $logfile;
	my $gamelist = @games . " games:";
	foreach (@games) {
		$gamelist .= " \"" . $_->{'attr'}->{'name'} . "\"";
	}
	$gamelist .= "\n";
	print STDERR $gamelist if $showgames;
	print LOG    $gamelist if $logfile;
}

sub serverstats() {
	open STATS, ">" . $statsfile;
	print STATS "users: " . ($#users + 1) . "\ngames: " . ($#games + 1) . "\n";
	close STATS;
}

# make several connections and send packets with a wrong length then sleep indefinitely
if (my $count = $opts{'e'}) {
	for (1..$count) {
		my $socket = &connect($HOST,$PORT);
		&write_bad_packet($socket, &wml::read_text($VERSION_RESPONSE));
	}
	sleep();
}



print STDERR "Connecting to $HOST:$PORT as $USERNAME.\n" unless $quiet;
print LOG    "Connecting to $HOST:$PORT as $USERNAME.\n" if $logfile;
my $socket = &connect($HOST,$PORT);
defined($socket) or die "Error: Can't connect to the server.";

&login($socket);
&serverstats() if $writestats;

while (1) {
	my $response = &read_packet($socket);
	foreach (@ {$response->{'children'}}) {
		if ($_->{'name'} eq 'message') {
			my $sender = $_->{'attr'}->{'sender'};
			my $message = $_->{'attr'}->{'message'};
			if ($message =~ s,^/me,,) {
				print STDERR &timestamp . "* $sender$message\n" unless $quiet;
				print LOG &logtimestamp . "* $sender$message\n" if $logfile;
			} else {
				print STDERR &timestamp . "<$sender> $message\n" unless $quiet;
				print LOG &logtimestamp . "<$sender> $message\n" if $logfile;
			}
		} elsif ($_->{'name'} eq 'whisper') {
			my $sender = $_->{'attr'}->{'sender'};
			my $message = $_->{'attr'}->{'message'};
			if ($message =~ s,^/me,,) {
				print STDERR &timestamp . "*$sender$message*\n" unless $quiet;
				print LOG &logtimestamp . "*$sender$message*\n" if $logfile;
			} else {
				print STDERR &timestamp . "*$sender* $message\n" unless $quiet;
				print LOG &logtimestamp . "*$sender* $message\n" if $logfile;
			}
		} elsif ($_->{'name'} eq 'gamelist_diff') {
			foreach (@ {$_->{'children'}}) {
				my $userindex = $_->{'attr'}->{'index'};
				if ($_->{'name'} eq 'insert_child') {
					if (my $user = &wml::has_child($_, 'user')) {
						my $username = $user->{'attr'}->{'name'};
						print STDERR &timestamp . "--> $username has logged on. ($userindex)\n" if $showjoins;
						print LOG &logtimestamp . "--> $username has logged on. ($userindex)\n" if $logfile;
						$users[@users] = $user;
						&serverstats() if $writestats;
					} else {
						print STDERR "[gamelist_diff]:" . Dumper($_) if $dumpunknown;
					}
				} elsif ($_->{'name'} eq 'delete_child') {
					if (my $user = &wml::has_child($_, 'user')) {
						my $username = $users[$userindex]->{'attr'}->{'name'};
						print STDERR &timestamp . "<-- $username has logged off. ($userindex)\n" if $showjoins;
						print LOG &logtimestamp . "<-- $username has logged off. ($userindex)\n" if $logfile;
						splice(@users,$userindex,1);
						&serverstats() if $writestats;
					} else {
						print STDERR "[gamelist_diff]:" . Dumper($_) if $dumpunknown;
					}
				} elsif ($_->{'name'} eq 'change_child') {
					if (my $user = &wml::has_child($_, 'user')) {
						foreach (@ {$user->{'children'}}) {
							#my $userindex = $_->{'attr'}->{'index'};   #the gamelistindex would be nice here.. probably hacky though so better put it in the 'location' key
							if ($_->{'name'} eq 'insert') {
								my $username = $users[$userindex]->{'attr'}->{'name'};
								if ($_->{'attr'}->{'available'} eq "yes") {
									my $game_id  = $users[$userindex]->{'attr'}->{'game_id'};
									my $location = $users[$userindex]->{'attr'}->{'location'};
									$users[$userindex]->{'attr'}->{'game_id'}  = "";
									$users[$userindex]->{'attr'}->{'location'} = "";
									print STDERR &timestamp . "++> $username has left a game: \"$location\" ($game_id)\n" if $showjoins and $showgames;
									print LOG &logtimestamp . "++> $username has left a game: \"$location\" ($game_id)\n" if $logfile;
								} elsif ($_->{'attr'}->{'available'} eq "no") {
									my $game_id  = $_->{'attr'}->{'game_id'};
									my $location = $_->{'attr'}->{'location'};
									$users[$userindex]->{'attr'}->{'game_id'}  = $game_id;
									$users[$userindex]->{'attr'}->{'location'} = $location;
									print STDERR &timestamp . "<++ $username has joined a game: \"$location\" ($game_id)\n" if $showjoins and $showgames;
									print LOG &logtimestamp . "<++ $username has joined a game: \"$location\" ($game_id)\n" if $logfile;
								}
							} elsif ($_->{'name'} eq 'delete') {
							} else {
								print STDERR "[gamelist_diff][change_child][user]:" . Dumper($_) if $dumpunknown;
							}
						}
						#print STDERR "[gamelist_diff][change_child]:" . Dumper($user) if $dumpunknown;
					} elsif (my $gamelist = &wml::has_child($_, 'gamelist')) {
						foreach (@ {$gamelist->{'children'}}) {
							my $gamelistindex = $_->{'attr'}->{'index'};
							if ($_->{'name'} eq 'insert_child') {
								if (my $game = &wml::has_child($_, 'game')) {
									my $gamename = $game->{'attr'}->{'name'};
									my $gameid   = $game->{'attr'}->{'id'};
									my $era      = "unknown";   # some eras don't set the id
									$era         = $game->{'attr'}->{'mp_era'} if $game->{'attr'}->{'mp_era'};
									my $scenario = "unknown";   # some scenarios don't set the id
									$scenario    = $game->{'attr'}->{'mp_scenario'} if $game->{'attr'}->{'mp_scenario'};
									my $players  = $game->{'attr'}->{'human_sides'};
									my $xp       = "100%";      # scenarios might not set XP
									$xp          = $game->{'attr'}->{'experience_modifier'} if $game->{'attr'}->{'experience_modifier'};
									my $gpv      = $game->{'attr'}->{'mp_village_gold'};
									my $fog      = $game->{'attr'}->{'mp_fog'};
									my $shroud   = $game->{'attr'}->{'mp_shroud'};
									my $timer    = "none";      # reloads may not set the timer
									$timer       = $game->{'attr'}->{'mp_countdown'} if $game->{'attr'}->{'mp_countdown'};
									my $observer = $game->{'attr'}->{'observer'};
									print STDERR &timestamp . "+++ A new game has been created: \"$gamename\" ($gamelistindex, $gameid).\n" if $showgames;
									print LOG &logtimestamp . "+++ A new game has been created: \"$gamename\" ($gamelistindex, $gameid).\n" if $logfile;
									my $settings = "Settings:  map: \"$scenario\"  era: \"$era\"  players: $players  XP: $xp  GPV: $gpv  fog: $fog  shroud: $shroud  observers: $observer  timer: $timer";
									if ($timer =~ "yes") {
										my $treservoir = "-";
										$treservoir    = $game->{'attr'}->{'mp_countdown_reservoir_time'} if $game->{'attr'}->{'mp_countdown_reservoir_time'};
										my $tinit      = "-";
										$tinit         = $game->{'attr'}->{'mp_countdown_init_time'} if $game->{'attr'}->{'mp_countdown_init_time'};
										my $taction    = "-";
										$taction       = $game->{'attr'}->{'mp_countdown_action_bonus'} if $game->{'attr'}->{'mp_countdown_action_bonus'};
										my $tturn      = "-";
										$tturn         = $game->{'attr'}->{'mp_countdown_turn_bonus'} if $game->{'attr'}->{'mp_countdown_turn_bonus'};
										$settings .= "  reservoir time: $treservoir  init time: $tinit  action bonus: $taction  turn bonus: $tturn";
									}
									print STDERR &timestamp . $settings . "\n" if $showgames;
									print LOG &logtimestamp . $settings . "\n" if $logfile;
									$games[@games] = $game;
									&serverstats() if $writestats;
								} else {
									print "[gamelist_diff][change_child][gamelist]:" . Dumper($_) if $dumpunknown;
								}
							} elsif ($_->{'name'} eq 'delete_child') {
								if (my $game = &wml::has_child($_, 'game')) {
									my $gamename = $games[$gamelistindex]->{'attr'}->{'name'};
									my $gameid   = $games[$gamelistindex]->{'attr'}->{'id'};
									my $turn     = $games[$gamelistindex]->{'attr'}->{'turn'};
									if (defined($turn)) {
										print STDERR &timestamp . "--- A game ended at turn $turn: \"$gamename\". ($gamelistindex, $gameid)\n" if $showgames;
										print LOG &logtimestamp . "--- A game ended at turn $turn: \"$gamename\". ($gamelistindex, $gameid)\n" if $logfile;
									} else {
										print STDERR &timestamp . "--- A game was aborted: \"$gamename\". ($gamelistindex, $gameid)\n" if $showgames;
										print LOG &logtimestamp . "--- A game was aborted: \"$gamename\". ($gamelistindex, $gameid)\n" if $logfile;
									}
									splice(@games,$gamelistindex,1);
									&serverstats() if $writestats;
								} else {
									print STDERR "[gamelist_diff][change_child][gamelist]:" . Dumper($_) if $dumpunknown;
								}
							} elsif ($_->{'name'} eq 'change_child') {
								if (my $game = &wml::has_child($_, 'game')) {
									foreach (@ {$game->{'children'}}) {
										if ($_->{'name'} eq 'insert') {
											if (my $turn = $_->{'attr'}->{'turn'}) {
												my $gamename = $games[$gamelistindex]->{'attr'}->{'name'};
												my $gameid   = $games[$gamelistindex]->{'attr'}->{'id'};
												if ($turn =~ /^1\//) {
													print STDERR &timestamp . "*** A game has started: \"$gamename\". ($gamelistindex, $gameid)\n" if $showgames;
													print LOG &logtimestamp . "*** A game has started: \"$gamename\". ($gamelistindex, $gameid)\n" if $logfile;
												} else {
													print STDERR &timestamp . "Turn changed to $turn in game: \"$gamename\". ($gamelistindex, $gameid)\n" if $showgames and $showturns;
													print LOG &logtimestamp . "Turn changed to $turn in game: \"$gamename\". ($gamelistindex, $gameid)\n" if $logfile;
												}
												$games[$gamelistindex]->{'attr'}->{'turn'} = $turn;
											}
										} elsif ($_->{'name'} eq 'delete') {
										} else {
											print "[gamelist_diff][change_child][gamelist][change_child][game]:" . Dumper($_) if $dumpunknown;
										}
									}
								} else {
									print STDERR "[gamelist_diff][change_child][gamelist]:" . Dumper($_) if $dumpunknown;
								}
							} else {
								print STDERR "[gamelist_diff][change_child][gamelist]:" . Dumper($_) if $dumpunknown;
							}
						}
					} else {
						print STDERR "[gamelist_diff]:" . Dumper($_) if $dumpunknown;
					}
				} else {
					print STDERR "[gamelist_diff]:" . Dumper($_) if $dumpunknown;
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
			print STDERR Dumper($_) if $dumpunknown;
		}
	}

}
print "Connection closed.\n\n" unless $quiet;
close(LOG);
