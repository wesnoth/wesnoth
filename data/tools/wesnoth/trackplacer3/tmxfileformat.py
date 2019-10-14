#!/usr/bin/env python3
# encoding: utf-8

"""Module for Tiled (Tile Editor) .tmx files containing journey data.

Uses Tiled's Object Layers (not Tile Layers).  This allows additional journey
markers to be added with the "Insert Tile" tool, and additional journeys to be
added as new layers.
"""

from wesnoth.trackplacer3.datatypes import *

import xml.etree.ElementTree as ET

# Although this doesn't show images, it needs to read their width and height
import PIL.Image

class ParseException(Exception):
    """There's a lot of expectations about the .tmx file, this generally
    means that one of those assumptions didn't hold.
    """
    def __init__(self, message, element=None):
        self.message = message
        self.element = element

class _IdCounter:
    """It seems that .tmx has several independent sequences of ids, and can have the same id used for different purposes. However, they can all have gaps in the sequence, for simplicity this code has only one ID counter, and makes each id unique."""
    def __init__(self):
        self.counter = 1

    def peek(self):
        """Return the id that the next call to get() will return, without changing the id"""
        return str(self.counter)

    def get(self):
        counter = self.counter
        self.counter += 1
        return str(counter)

class _TmxTileset:
    def read(tileset):
        """Returns a dict mapping tiles' gid attributes to actions.

        Argument should be the ETree element for the tileset."""
        base_id = int(tileset.attrib["firstgid"])
        tileset_to_action = {}
        for tile in tileset.findall("tile"):
            tile_id = base_id + int(tile.attrib["id"])
            image_source = tile.find("image").attrib["source"]
            action = None
            # This matches by endswith, so that changes to the wesnoth_data_dir won't
            # cause unnecessary breakage of .tmx files.
            for k in selected_icon_dictionary:
                if image_source.endswith(selected_icon_dictionary[k]):
                    action = k
                    break
            if action is None:
                raise ParseException("unrecognised action in tileset")
            tileset_to_action[str(tile_id)] = action
        return tileset_to_action

class TmxFileFormat(FileFormatHandler):
    """Translate a Journey to and from a Tiled (Tile Editor) TMX file."""
    def __init__(self, wesnoth_data_dir):
        """The data dir is the same as wesnoth's --data-dir argument.

        The data dir is only used for the journey marker images.
        """
        self.wesnoth_data_dir = wesnoth_data_dir

    def write(self, file_or_filename, journey, metadata=None):
        id_counter = _IdCounter()

        # Read the size of the background image
        try:
            with PIL.Image.open(journey.mapfile) as image:
                background_width, background_height = image.size
        except:
            print("Can't open background image, assuming 1024x768", journey.mapfile)
            background_width, background_height = 1024, 768

        tmxmap = ET.Element("map", attrib={
            "version": "1.2",
            "orientation":"orthogonal",
            "renderorder":"right-down",
            # There's no problem if the width and height don't exactly match the image, this
            # just determines the size of the grid shown in the UI and used for tile layers.
            # For track placement, tmx_trackplacer uses object layers instead of tile layers.
            "width": str(int(background_width / 32)),
            "height": str(int(background_height / 32)),
            "tilewidth":"32",
            "tileheight":"32",
            "infinite":"0"
        })

        # Wesnoth's NEW_JOURNEY (etc) macros use the coordinates as the center of the image,
        # Tiled uses them as the bottom-left corner of the image. This is a dictionary of
        # [adjustment to x, adjustment to y] pairs, in the direction wesnoth -> tmx.
        # If any of these images can't be found then there's no point continuing.
        image_offset = {}
        for action in selected_icon_dictionary:
            with PIL.Image.open(self.wesnoth_data_dir + "/" + selected_icon_dictionary[action]) as image:
               image_offset[action] = [int (- image.size[0] / 2), int (image.size[1] / 2)]

        # embed a tileset in the map, corresponding to the journey icons
        action_to_tileset = {}
        base_id = id_counter.get()
        tileset = ET.SubElement(tmxmap, "tileset", attrib={
            "firstgid":base_id,
            "name":"wesnoth journey icons",
            "tilewidth":"1",
            "tileheight":"1",
            "tilecount":"3"
        })
        ET.SubElement(tileset, "grid", attrib={"orientation":"orthogonal", "width":"1", "height":"1"})
        for i, action in enumerate(selected_icon_dictionary):
            action_to_tileset[action] = {"gid":str(int(base_id) + i)}
            tile = ET.SubElement(tileset, "tile", attrib={"id":str(i)})
            ET.SubElement(tile, "image", attrib={
                "source":self.wesnoth_data_dir + "/" + selected_icon_dictionary[action]
            })
            # increment the id_counter
            id_counter.get()

        # background image
        layer = ET.SubElement(tmxmap, "imagelayer", attrib={"id": "1", "name": "background"})
        ET.SubElement(layer, "image", attrib={"source": journey.mapfile})

        # journey tracks
        for track in journey.tracks:
            name = track.name
            layer = ET.SubElement(tmxmap, "objectgroup", attrib={"id": id_counter.get(), "name": name})
            for point in track.waypoints:
                if point.action not in action_to_tileset:
                    raise KeyError("Unknown action: " + point.action)
                attrib = action_to_tileset[point.action]
                attrib["id"] = id_counter.get()
                attrib["x"] = str(point.x + image_offset[point.action][0])
                attrib["y"] = str(point.y + image_offset[point.action][1])
                o = ET.SubElement(layer, "object", attrib=attrib)

        # The points in each journey need to be kept in the correct order, so that the animation
        # shows movement in the correct direction. If we know which points are newly-added, then the
        # logic in Track.insert_at_best_fit will handle them. With Tiled, ids are never reused, so
        # we can use the id_counter to work out which points are newly added.
        custom_properties = ET.SubElement(tmxmap, "properties")
        ET.SubElement(custom_properties, "property", attrib={"name":"tmx_trackplacer_export_id", "type":"int", "value":id_counter.get()})

        # These need to be higher than all ids used elsewhere in the file, so add these after everything else's id is assigned
        tmxmap.set("nextlayerid", id_counter.get())
        tmxmap.set("nextobjectid", id_counter.get())

        tree = ET.ElementTree(tmxmap)
        tree.write(file_or_filename, encoding="UTF-8", xml_declaration=True)

    def read(self, fp):
        if type(fp) == type(""):
            # if this raises IOError, let it pass to the caller
            fp = open(fp, "r")

        tree = ET.parse(fp)
        tmxmap = tree.getroot()

        journey = Journey()

        if tmxmap.attrib["orientation"] != "orthogonal":
            raise ParseException("expected an orthogonal TMX map")

        # parse the tileset
        if len(tmxmap.findall("tileset")) != 1:
            raise ParseException("expected exactly one tileset")
        tileset_to_action = _TmxTileset.read(tmxmap.find("tileset"))

        # Wesnoth's NEW_JOURNEY (etc) macros use the coordinates as the center of the image,
        # Tiled uses them as the bottom-left corner of the image. This is a dictionary of
        # [adjustment to x, adjustment to y] pairs, in the direction tmx -> wesnoth.
        # If any of these images can't be found then there's no point continuing.
        image_offset = {}
        for action in selected_icon_dictionary:
            with PIL.Image.open(self.wesnoth_data_dir + "/" + selected_icon_dictionary[action]) as image:
               image_offset[action] = [int (image.size[0] / 2), int (- image.size[1] / 2)]


        # background image
        if len(tmxmap.findall("imagelayer")) != 1 or tmxmap.find("imagelayer").attrib["name"] != "background":
            raise ParseException("expected exactly one imagelayer")
        if len(tmxmap.findall("imagelayer/image")) != 1:
            raise ParseException("expected exactly one image in the imagelayer")
        if tmxmap.find("imagelayer/image").attrib["source"] is None:
            raise ParseException("expected a background image")
        journey.mapfile = tmxmap.find("imagelayer/image").attrib["source"]

        # metadata that was added in write(), to track which points have been added in Tiled
        export_id_prop = tmxmap.find("properties/property[@name='tmx_trackplacer_export_id']")
        if export_id_prop is not None:
            export_id = int(export_id_prop.attrib["value"])
            def added_in_tiled(item):
                return export_id < int(item.attrib["id"])
        else:
            def added_in_tiled(item):
                return true

        # journey tracks
        for layer in tmxmap.findall("objectgroup"):
            track = Track(layer.attrib["name"])
            for point in layer.findall("object"):
                gid = point.attrib["gid"]
                if gid not in tileset_to_action:
                    raise KeyError("Unknown action gid: " + gid)
                action = tileset_to_action[gid]
                x = round(float(point.attrib["x"])) + image_offset[action][0]
                y = round(float(point.attrib["y"])) + image_offset[action][1]
                if added_in_tiled(point):
                    track.insert_at_best_fit(Waypoint(action, x, y))
                else:
                    track.waypoints.append(Waypoint(action, x, y))
            journey.tracks.append(track)

        return (journey, None)
