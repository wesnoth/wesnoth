#!/usr/bin/perl

use wml;
use wml_net;
use CGI qw/:standard -no_xhtml/;
use Time::gmtime;
use strict;

my ($host,$port) = ('campaigns.wesnoth.org', 15002);

my $style = 'http://www.wesnoth.org/mw/skins/glamdrol/main.css';

my $socket = eval {&wml_net::connect($host,$port)};
if ($@ ne '') {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.'),
    p(escapeHTML($@)), end_html;
  exit;
}

if (!defined($socket)) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error connecting to the campaign server.'),
    p('Error connecting to the campaign server.'), end_html;
  exit;
}

eval {
  &wml_net::write_packet($socket,&wml::read_text('
[request_campaign_list]
[/request_campaign_list]'));
};
if ($@ ne '') {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.'),
    p(escapeHTML($@)), end_html;
  exit;
}

my $response = eval {&wml_net::read_packet($socket)};
if ($@ ne '') {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.'),
    p(escapeHTML($@)), end_html;
  exit;
}

if (!defined($response)) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.'),
    p('Error accessing the campaign server.'), end_html;
  exit;
}

if (my $error = &wml::has_child($response, 'error')) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error accessing the campaign server.'),
    p($error->{'attr'}->{'message'}), end_html;
  exit;
}

my $campaign_list = &wml::has_child($response, 'campaigns');
if (!$campaign_list) {
  print header, start_html(-style=>{'src'=>$style},
    -title=>'Error retrieving campaign list.'),
    p('No', em('campaigns'), 'data returned.'), end_html;
  exit;
}

print header, start_html(-style=>{'src'=>$style},
  -title=>'Wesnoth campaigns available for download.');

my @campaigns = &wml::get_children($campaign_list,'campaign');

my $myurl = url(-relative=>1);

my $sort = 'timestamp_down';
if (defined(param('sort'))) {
  if (scalar(@{[param('sort')]}) != 1) {
    print p('Mutplie sort columns ignored.');
  }
  else {
    $sort = param('sort');
  }
}

my $title_sort = 'title_up';
my $version_sort = 'version_down';
my $author_sort = 'author_up';
my $timestamp_sort = 'timestamp_down';
my $size_sort = 'size_up';
my $downloads_sort = 'downloads_down';
$title_sort = 'title_down' if $sort eq 'title_up';
$version_sort = 'version_up' if $sort eq 'version_down';
$author_sort = 'author_down' if $sort eq 'author_up';
$timestamp_sort = 'timestamp_up' if $sort eq 'timestamp_down';
$size_sort = 'size_down' if $sort eq 'size_up';
$downloads_sort = 'downloads_up' if $sort eq 'downloads_down';

print p('Windows users may want to install',
  a({href=>'http://www.7-zip.org/'}, '7-zip'),
  "to handle tar gzipped archives (campaign downloads are in this format).",
  'This is a', a({href=>'http://www.debian.org/intro/free'}, 'free'),
  'and zero cost program that will handle a variety of archive formats.');
print p('Column headings are linked to alternate sort orders.');
print p('Campaign titles are linked to tar gzipped archives of the respective campaigns.');
print p('The downloaded campaign archives should be extracted in ~/.wesnoth/data/campaigns/ on *nix systems.',
  a({href=>'mailto:bruno@wolff.to'}, 'I'),
  'would appreciate people telling me what the corresponding directory is for other operating systems.');

my @rows = ();
foreach my $campaign (@campaigns) {
  foreach my $field ('icon', 'name', 'title', 'version', 'author', 'timestamp',
    'description', 'size', 'downloads') {
    $campaign->{'attr'}->{$field} =~ s/\001[^\003]*\003//g;
    $campaign->{'attr'}->{$field} =~ s/[\001-\037\177-\237]/ /g;
  }
  $campaign->{'attr'}->{'name'} =~ s;\s|/|\\;;g;
  my $title = $campaign->{'attr'}->{'name'};
  $title =~ s/_/ /g;
  $campaign->{'attr'}->{'title'} = $title
    if $campaign->{'attr'}->{'title'} =~ m/^\s*$/;
}

my @sorted = ();
if ($sort eq 'title_down') {
  @sorted = sort {uc($b->{'attr'}->{'title'}) cmp uc($a->{'attr'}->{'title'})}
    @campaigns;
}
elsif ($sort eq 'title_up') {
  @sorted = sort {uc($a->{'attr'}->{'title'}) cmp uc($b->{'attr'}->{'title'})}
    @campaigns;
}
elsif ($sort eq 'version_down') {
  @sorted = sort {uc($b->{'attr'}->{'version'}) cmp uc($a->{'attr'}->{'version'})}
    @campaigns;
}
elsif ($sort eq 'version_up') {
  @sorted = sort {uc($a->{'attr'}->{'version'}) cmp uc($b->{'attr'}->{'version'})}
    @campaigns;
}
elsif ($sort eq 'author_down') {
  @sorted = sort {uc($b->{'attr'}->{'author'}) cmp uc($a->{'attr'}->{'author'})}
    @campaigns;
}
elsif ($sort eq 'author_up') {
  @sorted = sort {uc($a->{'attr'}->{'author'}) cmp uc($b->{'attr'}->{'author'})}
    @campaigns;
}
elsif ($sort eq 'timestamp_down') {
  @sorted = sort {$b->{'attr'}->{'timestamp'} <=> $a->{'attr'}->{'timestamp'}}
    @campaigns;
}
elsif ($sort eq 'timestamp_up') {
  @sorted = sort {$a->{'attr'}->{'timestamp'} <=> $b->{'attr'}->{'timestamp'}}
    @campaigns;
}
elsif ($sort eq 'size_down') {
  @sorted = sort {$b->{'attr'}->{'size'} <=> $a->{'attr'}->{'size'}}
    @campaigns;
}
elsif ($sort eq 'size_up') {
  @sorted = sort {$a->{'attr'}->{'size'} <=> $b->{'attr'}->{'size'}}
    @campaigns;
}
elsif ($sort eq 'downloads_down') {
  @sorted = sort {$b->{'attr'}->{'downloads'} <=> $a->{'attr'}->{'downloads'}}
    @campaigns;
}
elsif ($sort eq 'downloads_up') {
  @sorted = sort {$a->{'attr'}->{'downloads'} <=> $b->{'attr'}->{'downloads'}}
    @campaigns;
}
else {
  @sorted = @campaigns;
}

foreach my $campaign (@sorted) {
  my @row = ();
  my $iname = $campaign->{'attr'}->{'icon'};
  if ($iname !~ m/^\s*$/) {
    $iname =~ s;^.*(/|\\);;;
    $iname =~ s/\..*$//;
    $iname =~ s/-|_/ /g;
    push @row, img({src=>'images/'.$campaign->{'attr'}->{'icon'},
     alt=>$iname});
  }
  else {
    push @row, 'No icon provided';
  }
  push @row, a({href=>($campaign->{'attr'}->{'name'}).'.tgz'},
    escapeHTML($campaign->{'attr'}->{'title'}));
  push @row, escapeHTML($campaign->{'attr'}->{'version'});
  my $trans = '';
  my $first = 1;
  my @lang = &wml::get_children($campaign,'translation');
  foreach my $lang (sort @lang) {
    if (!$first) {
      $first = 0;
    }
    else {
      $trans .= ' ';
    }
    $trans .= $lang->{'attr'}->{'language'};
  }
  $trans =~ s/\001[^\003]*\003//g;
  $trans =~ s/[\001-\037\177-\237]/ /g;
  push @row, escapeHTML($trans);
  push @row, escapeHTML($campaign->{'attr'}->{'author'});
  if ($campaign->{'attr'}->{'timestamp'} =~ m/^\d+$/) {
    push @row, escapeHTML(gmctime($campaign->{'attr'}->{'timestamp'}) . ' GMT');
  }
  else {
    push @row, 'Unknown';
  }
  push @row, escapeHTML($campaign->{'attr'}->{'description'});
  push @row, escapeHTML($campaign->{'attr'}->{'size'});
  push @row, escapeHTML($campaign->{'attr'}->{'downloads'});
  push @rows, td(\@row);
}

print table({frame=>'border',rules=>'all'},
  col, col({align=>'char'}), col, col,
  thead(Tr(th(['Icon',
    a({href=>$myurl . '?sort=' . $title_sort}, 'Title'),
    a({href=>$myurl . '?sort=' . $version_sort}, 'Version'),
    'Translations',
    a({href=>$myurl . '?sort=' . $author_sort}, 'Author'),
    a({href=>$myurl . '?sort=' . $timestamp_sort}, 'Last Updated'),
    'Description',
    a({href=>$myurl . '?sort=' . $size_sort}, 'Size'),
    a({href=>$myurl . '?sort=' . $downloads_sort}, 'Downloads')]))),
  tbody(Tr(\@rows))
  );

print end_html;
