package wml_net;
use strict;
use wml;
use IO::Socket;

%wml_net::outgoing_schemas = ();
%wml_net::incoming_schemas = ();

sub connect
{
	my ($host,$port) = @_;
	my $sock = IO::Socket::INET->new(PeerAddr => $host,
	                             PeerPort => $port,
				     Proto => 'tcp',
				     Type => SOCK_STREAM)
			   or die "Could not connect to $host:$port: $@\n";
	print $sock pack('N',0) or die "Could not send initial handshake";

	my $connection_num = "";
	read $sock, $connection_num, 4;
	die "Could not read connection number" if length($connection_num) != 4;

	$wml_net::outgoing_schemas{$sock} = [];
	$wml_net::incoming_schemas{$sock} = [];
	return $sock;
}

sub disconnect
{
	my ($sock) = @_;
	delete($wml_net::outgoing_schemas{$sock});
	delete($wml_net::incoming_schemas{$sock});
	close $sock;
}

sub read_packet
{
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

	return &wml::read_binary($wml_net::incoming_schemas{$sock},$res);
}

sub write_packet
{
	my ($sock,$doc) = @_;

	my $data = &wml::write_binary($wml_net::outgoing_schemas{$sock},$doc);
	$data .= chr 0;
	my $header = pack('N',length $data);
	print $sock "$header$data" or die "Error writing to socket: $!";
}

'Wesnoth network protocol';
