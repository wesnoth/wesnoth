#!/usr/bin/perl
#
# TrueType Font embedding permissions checker
# Based on lib/Lintian/Check/Fonts/Truetype.pm from Debian's lintian
# Dependencies: Font::TTF
#
# Copyright (C) 2019 Felix Lechner
# Copyright (C) 2023 P. J. McDermott
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

use strict;
use warnings;
use utf8;

use Font::TTF::Font;

my $PERMISSIONS_MASK        = 0xf;
my $NEVER_EMBED_FLAG        = 0x2;
my $PRINT_PREVIEW_ONLY_FLAG = 0x4;
my $EDIT_ONLY_FLAG          = 0x8;

sub main
{
	my $font;
	my $os2;
	my $table;
	my $fs_type;
	my $permissions;
	my @clauses;
	my $terms;

	$font = Font::TTF::Font->open($ARGV[0])
		or return 1;
	$os2 = $font->{'OS/2'}
		or return 0;
	$font->release();
	$table = $os2->read()
		or return 0;
	$fs_type = $table->{fsType}
		or return 0;

	$permissions = $fs_type & $PERMISSIONS_MASK;
	push(@clauses, 'never embed')
		if $permissions & $NEVER_EMBED_FLAG;
	push(@clauses, 'preview/print only')
		if $permissions & $PRINT_PREVIEW_ONLY_FLAG;
	push(@clauses, 'edit only')
		if $permissions & $EDIT_ONLY_FLAG;

	$terms = join(', ', @clauses)
		if @clauses;
	STDOUT->printf("%s\n", $terms)
		if length($terms);

	return 0;
}

exit(main());
