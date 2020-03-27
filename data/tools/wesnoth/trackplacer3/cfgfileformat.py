#!/usr/bin/env python3
# encoding: utf-8
"""Module for reading and writing .cfg files containing journey data.

It will look for track information enclosed in special comments that look like
this:

    # trackplacer: tracks begin
    # trackplacer: tracks end

trackplacer will alter only what it finds inside these comments, except that it
will also generate a file epilog for undefining local symbols.  The
epilog will begin with this comment:

    # trackplacer: epilog begins

TODO: what to do about the epilog? It needs to come after the scenarios, otherwise
the undefs happen before the scenarios can use the journey data.

Special comments may appear in the track section, looking like this:

    # trackplacer: <property>=<value>

These set properties that trackplacer may use. At present there is
only one such property: "map", which records the name of the mapfile on
which your track is laid.

Original (python2 + pygtk) implementation by Eric S. Raymond for the Battle For Wesnoth project, October 2008
"""

import re

from wesnoth.trackplacer3.datatypes import *

class IOException(Exception):
    """Exception thrown while reading a track file."""
    def __init__(self, message, path, lineno=None):
        self.message = message
        self.path = path
        self.lineno = lineno

class CfgFileMetadata:
    """Trackplacer is intended to write .cfg files that may also contain other information.

When it reads a file, in addition to the Journey it keeps some other information so that
it can write back to the same file while preserving the other data in the .cfg."""
    def __init__(self):
        self.properties = {}
        self.before = self.after = ""

class CfgFileFormat(FileFormatHandler):
    """Translate a Journey to/from a .cfg file, preserving non-trackplacer data."""
    def __init__(self):
        pass

    def read(self, fp):
        if type(fp) == type(""):
            try:
                fp = open(fp, "r")
            except IOError:
                raise IOException("Cannot read file.", fp)
        journey = Journey()
        metadata = CfgFileMetadata()
        selected_track = None
        if not fp.name.endswith(".cfg"):
            raise IOException("Cannot read this filetype.", fp.name)
        waypoint_re = re.compile("{NEW_(" + "|".join(icon_presentation_order) + ")" \
                                 + " +([0-9]+) +([0-9]+)}")
        property_re = re.compile("# *trackplacer: ([^=]+)=(.*)")
        define_re = re.compile("#define (.*)_STAGE[0-9]+(_END|_COMPLETE)?")
        state = "before"
        ignore = True # True when the most recent define_re match found an END or COMPLETE group, or before any track has been defined
        for line in fp:
            if line.startswith("# trackplacer: epilog begins"):
                break
            # This is how we ignore stuff outside of track sections
            if state == "before":
                if line.startswith("# trackplacer: tracks begin"):
                    state = "tracks" # And fall through...
                else:
                    metadata.before += line
                    continue
            elif state == "after":
                metadata.after += line
                continue
            elif line.startswith("# trackplacer: tracks end"):
                state = "after"
                continue
            # Which track are we appending to?
            m = re.search(define_re, line)
            if m:
                selected_track = journey.findTrack(m.group(1))
                if selected_track == None:
                    selected_track = Track(m.group(1))
                    journey.tracks.append(selected_track)
                ignore = bool(m.group(2))
                continue
            # Is this a track marker?
            m = re.search(waypoint_re, line)
            if m and not ignore:
                try:
                    tag = m.group(1)
                    x = int(m.group(2))
                    y = int(m.group(3))
                    selected_track.waypoints.append(Waypoint(tag, x, y))
                    continue
                except ValueError:
                    raise IOException("Invalid coordinate field.", fp.name, i+1)
            # \todo: Northern Rebirth has some tracks that start with an OLD_REST
            # before any of the NEW_JOURNEY markers. Maybe add a special-case for
            # that, that adds them if and only if len(selected_track.waypoints)==0

            # Is it a property setting?
            m = re.search(property_re, line)
            if m:
                metadata.properties[m.group(1)] = m.group(2)
                continue
        if "map" in metadata.properties:
            journey.mapfile = metadata.properties['map']
        else:
            raise IOException("Missing map declaration.", fp.name)
        fp.close()
        return (journey, metadata)

    def write(self, filename, journey, metadata=None):
        if metadata is None:
           metadata = CfgFileMetadata()

        if not filename.endswith(".cfg"):
            raise IOException("File must have .cfg extension.", fp.name)

        # If we're writing to an existing file, preserve the non-trackplacer parts
        # by ignoring the provided metadata and re-reading it from the file.
        try:
            ignored, metadata = self.read(filename)
            print("Preserving non-trackplacer data from the destination file")
        except:
            pass

        fp = open(filename, "w")
        fp.write(metadata.before)
        fp.write("# trackplacer: tracks begin\n#\n")
        fp.write("# Hand-hack this section strictly at your own risk.\n")
        fp.write("#\n")
        if not metadata.before and not metadata.after:
            fp.write("#\n# wmllint: no translatables\n\n")
        if journey.mapfile:
            metadata.properties["map"] = journey.mapfile
        for (key, val) in list(metadata.properties.items()):
            fp.write("# trackplacer: %s=%s\n" % (key, val))
        fp.write("#\n")
        definitions = []
        for track in journey.tracks:
            if len(track.waypoints) == 0:
                print("Warning: track {name} has no waypoints".format(name=track.name))
                continue
            name = track.name
            endpoints = []
            for i in range(0, len(track.waypoints)):
                if track.waypoints[i].action in segmenters:
                    endpoints.append(i)
            if track.waypoints[-1].action not in segmenters:
                endpoints.append(len(track.waypoints)-1)
            outname = name.replace(" ", "_").upper()
            for (i, e) in enumerate(endpoints):
                stagename = "%s_STAGE%d" % (outname, i+1,)
                definitions.append(stagename)
                fp.write("#define %s\n" % stagename)
                for j in range(0, e+1):
                    age="OLD"
                    if i == 0 or j > endpoints[i-1]:
                        age = "NEW"
                    waypoint = track.waypoints[j]
                    marker = "    {%s_%s %d %d}\n" % (age, waypoint.action, waypoint.x, waypoint.y)
                    fp.write(marker)
                fp.write("#enddef\n\n")
                endname = "%s_END" % stagename
                fp.write("#define %s\n" % endname)
                definitions.append(endname)
                for j in range(0, e+1):
                    age="OLD"
                    if j == endpoints[i]:
                        age = "NEW"
                    waypoint = track.waypoints[j]
                    marker = "    {%s_%s %d %d}\n" % (age, waypoint.action, waypoint.x, waypoint.y)
                    fp.write(marker)
                fp.write("#enddef\n\n")
            completename = "%s_COMPLETE" % outname
            fp.write("#define %s\n" % completename)
            definitions.append(completename)
            for waypoint in track.waypoints:
                marker = "    {%s_%s %d %d}\n" % ("OLD", waypoint.action, waypoint.x, waypoint.y)
                fp.write(marker)
            fp.write("#enddef\n\n")
        fp.write("# trackplacer: tracks end\n")
        fp.write(metadata.after)

        # \todo: what to do about the epilogue? It must wait until after the scenarios have used the journey data.
        # fp.write ("# trackplacer: epilog begins\n\n")
        # for name in definitions:
        #     if "{" + name + "}" not in metadata.after:
        #         fp.write("#undef %s\n" % name)
        # fp.write ("\n# trackplacer: epilog ends\n")

        fp.close()

if __name__ == "__main__":
    print("This isn't intended to be run directly")
