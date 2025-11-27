### Graphics
   * Duration option for image overlays added
		* Images (halos/items) placed with [item] in WML or wesnoth.interface.add_hex_overlay() in Lua can now be removed automaticlly after a duration run out.
		* The duration is in milliseconds. duration=2300 means the image will stay up for 2.3 seconds.
		* Images with a duration are not stored in the save file, they will not persist if you save and reload.
