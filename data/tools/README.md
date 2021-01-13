Tools for hacking or generating WML, maps, images, and sounds belong here.
Tools for sanity-checking the mainline campaigns and associated data
also belong here. Other utils are in utils/.

== Scripts ==

=== `tmx_trackplacer` ===

Converter for journey track files, the .cfg files which control the icon
sequences that appear on Wesnoth story screens. This can convert them to
or from the format needed for editing in Tiled.

=== trackviewer.pyw ===

Previews the animation of journey tracks.

=== wesnoth_addon_manager ===

Command-line client for uploading WML content to and downloading from
the add-on server.

=== wmlflip ===

Experimental tool that can hack a .cfg referring to a mapfile so that
all macros with X,Y coordinate pair arguments get their calls transformed
in a specified way.  Now supports only flipping the map around the Y
axis, but provides a framework that should make other transformations
easy.

=== wmllint and wmllint-1.4 ===

This tool lifts WML from older dialects to current and performs sanity checks.
See the header comment of wmllint for a description and invocation options.
wmllint-1.4 is an older version of wmllint kept alive because wmllint is
incompatible with pre-1.4 syntax.

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

Python API
----------

The `3` in `wmltools3` and `wmliterator3` names refers to them being the
Python3 versions of the tool.

Both `wmlparser` and `wmlparser3` are Python3, with wmlparser3 being a rewrite
with a different design of the implementation. Both versions are kept as they
have different APIs.

Historical note - the redesign of wmlparser was originally called wmlparser2;
both were originally written in Python2. For readability this document ignores
that detail and refers to the rewrite as wmlparser3.

### wmltools3.py

The main facility in this module is a cross-referencer class.
It also contains utility methods for working with the data tree.
See the header comment of wmltools.py for details

### wmliterator3.py

A WML codewalker class.  Not a full parser, but that's useful because
it doesn't get confused by unbalanced macros; instead it lets you
walk through lines in a text file parsing out tags and elements

### wmlparser3.py

This provides an interface for parsing WML files. The implementation uses the
game engine's `--preprocess` option to handle preprocessing of the input, so
it requires the C++ engine to be available if the WML needs preprocessing.

The API currently is very sparsely documented, but several of the tools can be
used for reference.

### wmlparser.py

This python module contains code originally developed for CampGen - it contains
a general WML Parser written in Python. So if you want to develop tools in
Python instead of Perl and need a WML parser, it may save some time.

The API currently is very sparsely documented. In general, wmlparser.py
is used like this:

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

### wmldata.py

This module has several utility methods defined to access and manipulate
WML data, and is part of the API for wmlparser.py (not wmlparser3.py).

    for unit in  wmldata.get_all("unit"):
        print unit.get_text_val("id")

Standalone use
--------------

### wmlparser

If called standalone, wmlparser.py will parse whatever you give it and
dump back to stdout. For example:

    python wmlparser.py -e {game.cfg} > game.dump

Should produce a nice several 100000 lines file of the complete configuration
with all included files and macros expanded to a single file.

### wmlparser3

If called standalone, wmlparser3.py will parse whatever you give it and
dump back to stdout. Running it with the argument `--help` lists the supported
arguments.
