# When Python looks for a package, it considers all directories with
# a file named __init__.py inside them. Therefore we need this file.
# The code below is executed on "import wesnoth.trackplacer3", importing
# the main classes that are intended to be public.

from wesnoth.trackplacer3.datatypes import Journey
from wesnoth.trackplacer3.datatypes import Track
from wesnoth.trackplacer3.datatypes import Waypoint

from wesnoth.trackplacer3.cfgfileformat import CfgFileFormat
from wesnoth.trackplacer3.tmxfileformat import TmxFileFormat
