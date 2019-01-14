Tools for hacking or generating WML, maps, images, and sounds belong here.
Tools for sanity-checking the mainline campaigns and associated data
also belong here. Other utils are in utils/.

== Scripts ==

=== journeylifter ===

A program for converting campaigns to use trackplacer-format journey files.
All mainline campaigns have already been converted; this is for lifting UMC.

=== rmtrans ===

Remove nearly transparent pixels from images using GIMP. It currently affects
only one image at a time. Batch processing is available within GIMP, but it
would be useful to expand this to skip files where the pixels did not change.

=== trackplacer ===

A visual editor for journey tracks, the icon sequences that appear on
Wesnoth story screens.

=== wesnoth_addon_manager ===

Command-line client for uploading WML content to and downloading from
the add-on server.

=== wmlflip ===

Experimental tool that can hack a .cfg referring to a mapfile so that
all macros with X,Y coordinate pair arguments get their calls transformed
in a specified way.  Now supports only flipping the map around the Y
axis, but provides a framework that should make other transformations
easy.

=== wmllint ===

This tool lifts WML from older dialects to current and performs sanity checks.
See the header comment of wmllint for a description and invocation options.

=== GUI.pyw ===

A Tkinter based GUI interface for wmllint, wmlscope and wmllint, to aid WML
developers frightened of the command line.

=== wmlscope ===

A WML cross-reference checker.  Normally you'll use this to list
unresolved symbols, if any.  See the header comment of wmlscope for a
description and invocation options.

=== wmlunits ===

List names of all units in mainline, either as a mediawiki table or
as HTML.

=== TeamColorizer ===

Map the magenta team-color patches in the input image to red in the
output image, copy the result to output.  (This script lives under
unit_tree).

=== extractbindings ===

Extract and format a list of keystroke bindings from a theme file.
Presently this generates a table suitable for wiki inclusion.

=== terrain2wiki.py ===

A script to create the "Terrain Table" on the TerrainCodeTableWML wiki page.
Run this and splice the output into the wiki whenever you add a new
terrain type to mainline.

=== campaign2wiki.py ===

A script that generates general information about mainline campaigns and
outputs wiki-ready output

== Python API ==

=== wmltools.py ===

The main facility in this module is a cross-referencer class.
It also contains utility methods for working with the data tree.
See the header comment of wmltools.py for details

=== wmliterator.py ===

A WML codewalker class.  Not a full parser, but that's useful because
it doesn't get confused by unbalanced macros; instead it lets you
walk through lines in a text file parsing out tags and elements

=== wmlparser.py ===

This python module contains code originally developed for CampGen - it contains
a general WML Parser written in Python, just like the Perl one. So if you want
to develop tools in Python instead of Perl and need a WML parser, it may save
some time.

The API currently is very sparsely documented, but I plan to extend this. In
general, wmlparser.py contains the WML Parser, which is used like:

parser = wmlparser.Parser(datapath)

Then:

parser.parse_file(filename)
or
parser.parse_stream(file)
or
parser.parse_text(string)

to set what to parse, and finally:

wmldata = parser.parse()

to read everything into a Python representation of the parsed data.

=== wmldata.py ===

This file has several utility methods defined to access and manipulate
WML data.  Most of them will not be generally useful. An example:

for unit in  wmldata.get_all("unit"):
    print unit.get_text_val("id")

== Standalone use ==

=== wmlparser ===

If called standalone, wmlparser.py will parse whatever you give it and
dump back to stdout. For example:

python wmlparser.py -e {game.cfg} > game.dump

Should produce a nice several 100000 lines file of the complete configuration
with all included files and macros expanded to a single file.
