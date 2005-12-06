# This file is part of the graph-includes package
#
# (c) 2005 Yann Dirson <ydirson@altern.org>
# Distributed under version 2 of the GNU GPL.

package graphincludes::project::wesnoth;
use strict;
use warnings;

use base qw(graphincludes::project::default);

use graphincludes::params;

sub filelabel {
  my $self = shift;
  my ($file,$level) = @_;
  $level = $graphincludes::params::minshow unless defined $level;

  # 0: file

  $file =~ s/^$self->{PFXSTRIP}// if defined $self->{PFXSTRIP};

  return $file if $level == 0;

  # 1: compilation unit

  $file =~ s/\.[^.]*$//;
  $file='ai' if $file =~ m/^ai_(move|attack)$/;

  return $file if $level == 1;

  # 2: small groups

  if ($file =~ m!^(variable|server/variable)\.! ) {
    $file='variable';
  } elsif ($file =~ m!^(multiplayer|ai).*!) {
    $file=$1;
  } elsif ($file =~ m!^(mapgen|mapgen_dialog|cavegen|map_create)\..*!) {
    $file='mapcreator';
  } elsif ($file =~ m!^(serialization|widgets)/.*!) {
    $file=$1;
  }

  return $file if $level == 2;

  # 3: big groups

  if ($file =~ m!^(array|astarnode|config|filesystem|game_config|game_errors|gettext|global|language|log|map|mapgen|pathfind|pathutils|preferences|race|random|serialization|scoped_resource|terrain|thread|tstring|unit|unit_types|util|variable|wassert|wml_separators|(.*/xcoll))$!) {
    $file='core';
  } elsif ($file =~ m!^(animated|clipboard|cursor|font|halo|image|sdl_utils|tooltips|video)$!) {
    $file='graphics';
  } elsif ($file =~ m!^(builder|display|events|key|theme|widgets)$!) {
    $file='uicore';
  } elsif ($file =~ m!^(about|hotkeys|preferences_display|show_dialog)$!) {
    $file='ui';
  } elsif ($file =~ m!^(ai|game|help|multiplayer|titlescreen)$!) {
    $file='gameclient';
  } elsif ($file =~ m!^(campaign_server|editor|server|tools)/.*!) {
    $file=$1;
  }

  return $file;
}

sub defaultcolors {
  my @colors;
  $colors[2] = {
		serialization => 'steelblue1',
		mapcreator    => 'gold',
		widgets       => 'linen',
		multiplayer   => 'palegreen',
	       };
  $colors[3] = {
		core          => 'steelblue3',
		graphics      => 'peachpuff',
		uicore        => 'lavenderblush',
		ui            => 'pink',

		gameclient    => 'yellow',
		editor        => 'cyan',
		server        => 'magenta',
		tools         => 'lightgrey',
	       };
  return @colors;
}

sub ignored_deps {
  return {
	  'src/video.cpp' => {'src/font.hpp' => 'split out floating labels from video' },
	 };
}

1;
