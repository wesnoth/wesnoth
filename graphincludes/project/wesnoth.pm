# This file is part of the graph-includes package
#
# (c) 2005 Yann Dirson <ydirson@altern.org>
# Distributed under version 2 of the GNU GPL.

package graphincludes::project::wesnoth;
use graphincludes::project::default;
our @ISA = qw(graphincludes::project::default);

sub filelabel {
  my $self = shift;
  my ($file,$level) = @_;
  $level = $main::minshow unless defined $level;

  $file =~ s/^$self->{PFXSTRIP}// if defined $self->{PFXSTRIP};

  if ($level == 0) {
    return $file;
  } elsif ($level == 1) {
    $file =~ s/\.[^.]*$//;
    return 'ai' if $file =~ m/^ai_(move|attack)$/;
    return $file;
  } elsif ($level == 2) {
    if ($file =~ m!^(variable|server/variable|game_events)\.! ) {
      return 'variable';
    } elsif ($file =~ m!^(multiplayer|ai).*!) {
      return $1;
    } elsif ($file =~ m!^(mapgen|mapgen_dialog|cavegen|map_create)\..*!) {
      return 'mapcreator';
    } elsif ($file =~ m!^(array|astarnode|config|filesystem|game_config|gettext|global|language|log|map|pathfind|pathutils|race|random|scoped_resource|terrain|thread|unit|util|variable|wassert|(.*/xcoll))\..*!) {
      return 'core';
    } elsif ($file =~ m!^(clipboard|cursor|font|image|sdl_utils|tooltips|video)\..*!) {
      return 'graphics';
    } elsif ($file =~ m!^(events|preferences|show_dialog)\..*!) {
      return 'guicore';
    } elsif ($file =~ m!^(editor|server|serialization|widgets)/.*!) {
      return $1;
    }
  }
  return undef;
}

sub defaultcolors {
  my @colors;
  $colors[2] = {
		core          => 'steelblue3',
		serialization => 'steelblue1',
		variable      => 'slateblue1',

		mapcreator    => 'gold',

		graphics      => 'peachpuff',
		widgets       => 'linen',
		guicore       => 'lavenderblush',

		multiplayer   => 'palegreen',

		editor        => 'cyan',
		server        => 'pink',
	       };
  return @colors;
}

1;
