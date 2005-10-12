#!/usr/bin/perl

use wml;
use wml_net;
use Archive::Tar;
use CGI qw/:standard -no_xhtml/;
use strict;

my ($host, $port) = ("campaigns.wesnoth.org", 15002);

my $style = 'http://www.wesnoth.org/mw/skins/glamdrol/main.css';

my $socket = eval {&wml_net::connect($host,$port)};
if ($@ ne '') {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p(escapeHTML($@)), end_html;
  exit;
}

if (!defined($socket)) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error connecting to the campaign server.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p('Error connecting to the campaign server.'), end_html;
  exit;
}

my $name = url(-relative=>1);
if ($name =~ m/^(.*)\.tgz$/) {
  $name = $1;
}
else {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Invalid campaign name URL.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p('Campaigns are only available as gzipped tar archives (.tgz files).'),
    end_html;
  exit;
}

if ($name =~ m/^\./) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Invalid campaign name.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p(em($name),
      'is an invalid campaign name because it begins with a period.'),
    end_html;
  exit;
}

if ($name =~ m;/;) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Invalid campaign name.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p(em($name),
      'is an invalid campaign name because it contains a forward slash (/).'),
    end_html;
  exit;
}

if ($name =~ m/^$/) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Invalid campaign name.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p(em($name),
    'is an invalid campaign name because it is the empty string.'),
    end_html;
  exit;
}

eval {
  &wml_net::write_packet($socket,&wml::read_text("
[request_campaign_list]
name=\"$name\"
[/request_campaign_list]"));
};
if ($@ ne '') {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p(escapeHTML($@)), end_html;
  exit;
}

my $response = eval {&wml_net::read_packet($socket)};
if ($@ ne '') {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p(escapeHTML($@)), end_html;
  exit;
}

if (!defined($response)) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p('Error accessing the campaign server.'), end_html;
  exit;
}

if (my $error = &wml::has_child($response, 'error')) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p(escapeHTML($error->{'attr'}->{'message'})), end_html;
  exit;
}

my $campaign_list = &wml::has_child($response, 'campaigns');
if (!$campaign_list) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error retrieving campaign list.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p('No', em('campaigns'), 'data returned.'), end_html;
  exit;
}

my @campaigns = &wml::get_children($campaign_list,'campaign');

my $version = undef;
my $timestamp = '';

foreach my $campaign (@campaigns) {
  foreach my $field ('name', 'version', 'timestamp') {
    $campaign->{'attr'}->{$field} = ''
      unless defined($campaign->{'attr'}->{$field});
    $campaign->{'attr'}->{$field} =~ s/\001[^\003]*\003//g;
    $campaign->{'attr'}->{$field} =~ s/[\001-\037\177-\237]/ /g;
  }
  $campaign->{'attr'}->{'name'} =~ s;\s|/|\\;;g;
  if ($name eq $campaign->{'attr'}->{'name'}) {
    $version = $campaign->{'attr'}->{'version'};
    $timestamp = $campaign->{'attr'}->{'timestamp'};
    last;
  }
}
if (!defined($version)) {
  unlink("version/$name.ver");
  unlink("tgz/$name.tgz");
  unlink("timestamp/$name.ver");
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error retrieving campaign information.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p('No information was retrievable for the campaign', em($name) . '.'),
    end_html;
  exit;
}

my $cached = 1;
if (open(VERS, "<version/$name.ver")) {
  my $vers = <VERS>;
  $cached = 0 unless $vers eq $version;
  close VERS;
}
else {
  $cached = 0;
}
if (open(VERS, "<timestamp/$name.ver")) {
  my $ts = <VERS>;
  $cached = 0 unless $ts eq $timestamp;
  close VERS;
}
else {
  $cached = 0;
}

if (!$cached) {
  unlink("version/$name.ver");
  unlink("tgz/$name.tgz");
  unlink("timestamp/$name.ver");

  my $archive = Archive::Tar->new();

  eval {
    &wml_net::write_packet($socket, &wml::read_text("
[request_campaign]
name=\"$name\"
[/request_campaign]"));
  };
  if ($@ ne '') {
    print header, start_html(-style=>{'src'=>$style},
      -title=>'Error accessing the campaign server.');
    print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
      img({alt=>'Wesnoth logo',
        src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
    print p(escapeHTML($@)), end_html;
    exit;
  }

  my $response = eval {&wml_net::read_packet($socket)};
  if ($@ ne '') {
    print header, start_html(-style=>{'src'=>$style},
      -title=>'Error accessing the campaign server.');
    print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
      img({alt=>'Wesnoth logo',
        src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
    print p(escapeHTML($@)), end_html;
    exit;
  }

  if (!defined($response)) {
    print header, start_html(-style=>{'src'=>$style},
      -title=>'Campaign server error while attempting to retrieve campaign.');
    print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
      img({alt=>'Wesnoth logo',
        src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
    print p('Campaign server error while attempting to retrieve campaign.'),
      end_html;
    exit;
  }
  if (my $error = &wml::has_child($response, 'error')) {
    print header, start_html(-style=>{'src'=>$style},
      -title=>'Campaign server error while attempting to retrieve campaign.');
    print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
      img({alt=>'Wesnoth logo',
        src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
    print p(escapeHTML($error->{'attr'}->{'message'})), end_html;
    exit;
  }
  $version = $response->{'attr'}->{'version'}
    if defined($response->{'attr'}->{'version'});
  $timestamp = $response->{'attr'}->{'timestamp'}
    if defined($response->{'attr'}->{'timestamp'});
  &archive_dir($archive, $response, '');
  $archive->write("tgz/$name.$$", 1);
  if (!open(VERS, ">version/$name.$$")) {
    print header, start_html(-style=>{'src'=>$style},
      -title=>'Unable to cache campaign.');
    print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
      img({alt=>'Wesnoth logo',
        src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
    print p('Unable to cache campaign.'),
      end_html;
    exit;
  }
  print VERS $version;
  close VERS;
  if (!open(VERS, ">timestamp/$name.$$")) {
    print header, start_html(-style=>{'src'=>$style},
      -title=>'Unable to cache campaign.');
    print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
      img({alt=>'Wesnoth logo',
        src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
    print p('Unable to cache campaign.'),
      end_html;
    exit;
  }
  print VERS $timestamp;
  close VERS;
  rename "tgz/$name.$$", "tgz/$name.tgz";
  rename "version/$name.$$", "version/$name.ver";
  rename "timestamp/$name.$$", "timestamp/$name.ver";
}

if (!open(TGZ, "<tgz/$name.tgz")) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Unable to read cached copy of the campaign.');
  print div({id=>'header'}, div({id=>'logo'}, a({href=>'http://www.wesnoth.org/'},
    img({alt=>'Wesnoth logo',
      src=>'http://wesnoth.org/mw/skins/glamdrol/wesnoth-logo.jpg'}))));
  print p('Unable to read cached copy of the campaign.'),
    end_html;
  exit;
}

print "Content-Type: application/x-gzip\r\n",
  "Content-disposition: attachment; filename=\"$name.tgz\"\r\n\r\n";

my $buf;
while (read(TGZ, $buf, 16384)) {
  print $buf;
}

exit;

sub archive_dir
{
  my ($archive, $doc, $dest) = @_;
  my $name = $doc->{'attr'}->{'name'};
  my $path;
  if ($name ne '') {
    if ($dest eq '') {
      $path = $name;
    }
    else {
      $path = "$dest/$name";
    }
  }
  else {
    $path = $dest;
  }
  
  foreach my $dir (&wml::get_children($doc, 'dir')) {
    &archive_dir($archive, $dir, $path);
  }

  foreach my $file (&wml::get_children($doc, 'file')) {
    my $filename;
    if ($path eq '') {
      $filename = $file->{'attr'}->{'name'};
    }
    else {
      $filename = "$path/" . $file->{'attr'}->{'name'};
    }
    $archive->add_data($filename, $file->{'attr'}->{'contents'},
      {'uname'=>'root', 'gname'=>'root', 'uid'=>0, 'gid'=>0, 'mode'=>0644});
  }
}
