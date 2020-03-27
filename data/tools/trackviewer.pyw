#!/usr/bin/env python3
# encoding: utf-8
"""
%(prog)s example.cfg
%(prog)s example.tmx --track JOURNEY_PART1

Map journey track animation preview tool, shows the journey without needing to
start Wesnoth and refresh the cache. Currently this does not have editing
functions, instead it’s a support program which assumes you’re either editing
the .cfg file with a text editor or using Tiled with tmx_trackplacer.

At least on Linux you can have both Tiled and this open on the same file. Save
the file in Tiled, alt+tab to this, and press enter to reload the file.
"""

# Gtk version by Eric S. Raymond for the Battle For Wesnoth project, October 2008.
# Tkinter version by Steve Cotton for the Battle For Wesnoth project, 2019.

import wesnoth.trackplacer3 as tp3

import argparse
import tkinter
import tkinter.ttk as ttk
import PIL.Image
import PIL.ImageTk

def waypoint_generator(journey):
    for track in journey.tracks:
        for point in track.waypoints:
            yield (track, point)

class ReloadFactory:
    """Encapsulates both the file-handling and any command line options that affect
    which tracks will be shown.
    """
    def __init__(self):
        raise NotImplementedError()

    def load_waypoints(self):
        """Parsing the most recent version of the file, return
        an iterator for the waypoints that should be drawn.
        """
        raise NotImplementedError()

class MapAndWaypointCanvas:
    def __init__(self, window, journey, reload_factory, wesnoth_data_dir):
        self.window = window
        self.journey = journey
        self.reload_factory = reload_factory
        self.waypoint_generator = None
        self.animation_loop_id = None
        self.latest_drawn = [tkinter.StringVar(), tkinter.StringVar(), tkinter.StringVar()]

        try:
            mapimage = PIL.Image.open(self.journey.mapfile)
        except:
            try:
                mapimage = PIL.Image.open(wesnoth_data_dir + "/../" + self.journey.mapfile)
            except:
                raise Exception("Can’t open map image")

        background_width, background_height = mapimage.size
        self.canvas = tkinter.Canvas(self.window, scrollregion=(0, 0, background_width, background_height))
        self.mapphotoimage = PIL.ImageTk.PhotoImage(mapimage)
        self.canvas.create_image((0, 0), anchor="nw", image=self.mapphotoimage)

        # hope this fits on screen
        self.canvas.configure(width=background_width, height=background_height)

        self.action_image = {}
        for action in tp3.datatypes.selected_icon_dictionary:
            with PIL.Image.open(wesnoth_data_dir + "/" + tp3.datatypes.selected_icon_dictionary[action]) as image:
                self.action_image[action] = PIL.ImageTk.PhotoImage(image)

        # start with all waypoints visible
        for (track, point) in reload_factory.load_waypoints():
            self.draw_waypoint(point)

    def get_widget(self):
        return self.canvas

    def get_lastest_drawn(self):
        """Returns an array of StringVars which will be updated with the details
        of the most recently drawn point."""
        return self.latest_drawn

    def draw_waypoint(self, point):
        if point.action in self.action_image:
            self.canvas.create_image((point.x, point.y), image=self.action_image[point.action], tags="waypoint")
        else:
            self.canvas.create_text((point.x, point.y), text=point.action, fill="red", tags="waypoint")

    def clear_all_drawn_waypoints(self):
        self.canvas.delete("waypoint")

    def _next_frame(self):
        try:
            (track, point) = next(self.waypoint_generator)
            self.draw_waypoint(point)
            self.latest_drawn[0].set(track.name)
            self.latest_drawn[1].set("{x}, {y}".format(x=point.x, y=point.y))
            self.latest_drawn[2].set(point.action)
            self.animation_loop_id = self.window.after(500, self._next_frame)
        except StopIteration:
            self.animation_loop_id = None
            pass

    def toggle_pause(self):
        if self.animation_loop_id:
            self.window.after_cancel(self.animation_loop_id)
            self.animation_loop_id = None
        elif self.waypoint_generator is not None:
            self.animation_loop_id = self.window.after(0, self._next_frame)

    def restart_animation(self):
        """This rereads the file each time, in case the file has been edited."""
        if self.animation_loop_id:
            self.window.after_cancel(self.animation_loop_id)
            self.animation_loop_id = None
        self.clear_all_drawn_waypoints()
        self.waypoint_generator = self.reload_factory.load_waypoints()
        self.animation_loop_id = self.window.after(0, self._next_frame)

class Controls:
    def __init__(self, window, mapAndWaypointCanvas):
        self.window = window
        self.canvas = mapAndWaypointCanvas
        self.frame = tkinter.Frame(self.window)

        self.window.bind("<Escape>", self.quit)
        self.window.bind("<space>", self.toggle_pause)
        self.window.bind("<Return>", self.restart_animation)
        self.window.bind("<KP_Enter>", self.restart_animation)

        button = ttk.Button(self.frame, takefocus=False, text="Quit\n(escape)", command=self.quit)
        button.pack()

        button = ttk.Button(self.frame, takefocus=False, text="Pause\n(space)", command=self.toggle_pause)
        button.pack()

        button = ttk.Button(self.frame, takefocus=False, text="Reload\n(enter)", command=self.restart_animation)
        button.pack()

        label = ttk.Label(self.frame, text="\n".join([
            "This tool only",
            "previews the",
            "animation, it does",
            "not have editing",
            "capabilities",
            "",
            "To change the data,",
            "either edit the",
            ".cfg file or use",
            "tmx_trackplacer."]))
        label.pack()

        for x in self.canvas.get_lastest_drawn():
            label = ttk.Label(self.frame, textvariable=x)
            label.pack()

    def get_widget(self):
        return self.frame

    def _debug_log_event(self, event):
        print(repr(event))

    def quit(self, event=None):
        self.window.quit()

    def toggle_pause(self, event=None):
        self.canvas.toggle_pause()

    def restart_animation(self, event=None):
        self.canvas.restart_animation()

class LoaderImpl(ReloadFactory):
    def __init__(self, options):
        self.options = options

        if options.file is None:
            raise RuntimeError("Need a filename to read from")

        if options.file.endswith(".cfg"):
            self.reader = tp3.CfgFileFormat()
        elif options.file.endswith(".tmx"):
            self.reader = tp3.TmxFileFormat(wesnoth_data_dir=options.data_dir)
        else:
            raise RuntimeError("Don’t know how to handle input from this file type")

    def load_journey(self):
        (journey, metadata) = self.reader.read(self.options.file)
        return journey

    def load_waypoints(self):
        journey = self.load_journey()
        if options.track is not None:
            found_track = None
            for track in journey.tracks:
                if track.name == options.track:
                    found_track = track
            if found_track is None:
                raise RuntimeError("Named track not found in journey")
            journey.tracks = [found_track]
        return waypoint_generator(journey)

if __name__ == "__main__":
    ap = argparse.ArgumentParser(usage=__doc__)
    ap.add_argument("file", metavar="filename", help="Read input from this file")
    ap.add_argument("--data-dir", metavar="dir",
                            help='Same as Wesnoth’s “--data-dir” argument')
    ap.add_argument("--track", metavar="track_name",
                            help='Only show the animation for the named track')
    options = ap.parse_args()

    if options.data_dir is None:
        import os, sys
        APP_DIR,APP_NAME=os.path.split(os.path.realpath(sys.argv[0]))
        WESNOTH_ROOT_DIR=os.sep.join(APP_DIR.split(os.sep)[:-2])  # pop out "data" and "tools"
        options.data_dir=os.path.join(WESNOTH_ROOT_DIR,"data")

    reload_factory = LoaderImpl(options)

    journey = reload_factory.load_journey()
    print("Read data:", str(journey))

    window = tkinter.Tk()
    window.title("trackplacer animation preview")
    canvas = MapAndWaypointCanvas(window, journey, reload_factory, options.data_dir)
    controls = Controls(window, canvas)
    controls.get_widget().pack(side=tkinter.LEFT)
    canvas.get_widget().pack(side=tkinter.LEFT)
    window.mainloop()
