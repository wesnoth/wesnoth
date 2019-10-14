#!/usr/bin/env python3
# encoding: utf-8
"""
trackplacer3.datatypes -- file-format-independent handling of journeys

A journey is an object containing a map file name and a (possibly
empty) list of tracks, each with a name and each consisting of a
sequence of track markers.

Original (python2 + pygtk) implementation by Eric S. Raymond for the Battle For Wesnoth project, October 2008
"""

import math

# All dependencies on the shape of the data tree live here
# The code does no semantic interpretation of these icons at all;
# to add new ones, just fill in a dictionary entry.
imagedir = "core/images/"
selected_icon_dictionary = {
    "JOURNEY": imagedir + "misc/new-journey.png",
    "BATTLE":  imagedir + "misc/new-battle.png",
    "REST":    imagedir + "misc/flag-red.png",
    }
unselected_icon_dictionary = {
    "JOURNEY": imagedir + "misc/dot-white.png",
    "BATTLE":  imagedir + "misc/cross-white.png",
    "REST":    imagedir + "misc/flag-white.png",
    }
icon_presentation_order = ("JOURNEY", "BATTLE", "REST")
segmenters = ("BATTLE","REST")

# Basic functions for bashing points and rectangles

def _distance(point1, point2):
    "Euclidean distance between two waypoints."
    return math.sqrt((point1.x - point2.x)**2 + (point1.y - point2.y)**2)

class Waypoint:
    """Represents a single dot, battle or restpoint."""
    def __init__(self, action, x, y):
        self.action = action
        self.x = x
        self.y = y

    def __str__(self):
        return "<Waypoint '{action}' at {x},{y}>".format(action=self.action, x=self.x, y=self.y)

class Track:
    """An ordered list of Waypoints, users are expected to directly access the data members."""
    def __init__(self, name):
        self.name = name
        self.waypoints = []

    def insert_at_best_fit(self, w):
        """Utility function to add a new Waypoint, working out from from its coordinates where in the sequence it should be added.

        If the new point should definitely be at the end, you can use waypounts.append instead of this method.
        """
        if len(self.waypoints) < 2:
            self.waypoints.append(w)
            return

        # Find the index of the member of self.waypoints nearest to the new point
        closest = min(range(len(self.waypoints)), key=lambda i: _distance(w, self.waypoints[i]))
        if closest == 0:
            if _distance(self.waypoints[0], self.waypoints[1]) < _distance(w, self.waypoints[1]):
                self.waypoints.insert(0, w)
            else:
                self.waypoints.insert(1, w)
        elif closest == len(self.waypoints)-1:
            if _distance(self.waypoints[-1], self.waypoints[-2]) < _distance(w, self.waypoints[-2]):
                self.waypoints.append(w)
            else:
                self.waypoints.insert(-1, w)
        elif len(self.waypoints) == 2:
            self.waypoints.insert(1, w)
        elif _distance(w, self.waypoints[closest-1]) < _distance(self.waypoints[closest], self.waypoints[closest-1]):
            self.waypoints.insert(closest, w)
        else:
            self.waypoints.insert(closest+1, w)

class Journey:
    """Collection of all Tracks, and the corresponding background image"""
    def __init__(self):
        self.mapfile = None # Map background of the journey
        self.tracks = [] # ordered list of Tracks

    def findTrack(self, name):
        for track in self.tracks:
            if name == track.name:
                return track
        return None

    def __str__(self):
        return "<Journey based on map file '%s', with tracks {%s}>" % (self.mapfile,
            ",".join([track.name for track in self.tracks]))

class FileFormatHandler:
    """Interface for reading and writing files"""

    def __init__(self):
        raise NotImplementedError()

    def read(self, file_or_filename):
        """Return a (Journey, metadata) pair.

        The metadata may be None, and is information about the source file that
        isn't represented in the Journey object. The purpose of it is to check
        whether data is lost by reading a file and then writing the data to a new
        file.
        """
        raise NotImplementedError()

    def write(self, file_or_filename, journey, metadata=None):
        """Create a new file or overwriting an existing file.

        When overwriting an existing file, this may try to preserve non-journey
        data.

        When creating a new file, if the metadata is non-None then this may try
        to recreate the non-journey data from the original file. This is intended
        to be used from checking what data is lost in a round-trip, by making the
        copy of a file with only the journey parts changed.

        If metadata is non-None when overwriting an existing file, it's currently
        implementation defined which set of data (or neither) will be preserved.
        """
        raise NotImplementedError()
