# @file README

The programs in this directory data/tools/wesnoth
are for checking, analysing and maintenance of WML-files,
written in python.

###

__init__.py
  Cause Python to execute any code in this directory on "import wesnoth".

campaignserver_client.py
  textmode-client for uploading + downloding campaigns to the server.

wescamp.py
  This utility provides two tools
  * sync a campaign with the version on wescamp (using the packed campaign 
    as base)
  * update the translations in a campaign (in the packed campaign)

wmldata.py
  This module represents the internal appearance of WML.

wmliterator.py
  Python routines for navigating a Battle For Wesnoth WML tree

wmlparser.py
  Module implementing a WML parser.

wmltools.py
  Python routines for working with a Battle For Wesnoth WML tree

###

From IRC #wesnoth-dev - 2007-11-27 

<hajo> I just don't see the big picture about the files in that directory - who needs it for what task ?

<Sapient> well, let's say you want to process some WML files and transform them or understand them in a program
<Sapient> if you want to perform lexical analysis, then using wmliterator would save you a lot of work
<Sapient> if you want to parse it and get the overall tree, then wmlparser would be the choice

<hajo> Ok, but campaign / scenario-authors rarely do that
<Sapient> right...

<Sapient> if you want to write tools to help you author those campaigns, 
<Sapient> you might write some programs or to maintain them
<Sapient> so it is only useful if you are a programmer

<Sapient> although wmliterator can do a decent job of detecting unbalanced WML 
<Sapient> if that's all you need to do just run it from the command line for that
<Sapient> so it would let you know that [a][/b][/a] is invalid, and give you a line number
<Sapient> or [a][b][/a]

<hajo> it just says "reading x.cfg" and "y lines read"
<Sapient> right, no errors
<Sapient> it iterated successfully


# vim: tabstop=4: shiftwidth=4: expandtab: softtabstop=4: autoindent:

