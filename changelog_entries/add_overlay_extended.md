### Graphics
	* Z_order now works for halos placed with [item] or wesnoth.interface.add_hex_overlay().
		* Before this the order of halos would only depend on when you placed them, the latest placement ending up on top.
		* Now you can set it like: z_order = -8
   
	* Animation for image overlays added
		* Images placed with [item] in WML or wesnoth.interface.add_hex_overlay() in Lua can now be animated, just as they can for halos and units.
		* The animation happens automatically if you supply a recognized animation string. Works the same as any other animation.
		* (e.g. "img1.png, img2.png:200, img3.png, img[04~08].png")
		
	* Pixel offset x/y options for image overlays added
		* Images (halos/items) placed with [item] in WML or wesnoth.interface.add_hex_overlay() in Lua can now have their placement adjusted/offset in pixels.
		* If you offset an image outside the hex loactions you need to set multihex=yes / true. Otherwise it will get cropped away.
		* Commands like: pixel_offset_x = -10	pixel_offset_y = 841
		
	* Duration option for image overlays added
		* Images (halos/items) placed with [item] in WML or wesnoth.interface.add_hex_overlay() in Lua can now be removed automatically after a duration runs out.
		* The duration is in milliseconds. duration=2300 means the image will stay up for 2.3 seconds.
		* Images with a duration are not stored in the save file, they will not persist if you save and reload.
		
	* Layer option for image overlays added
		* Images (items) placed with [item] in WML or wesnoth.interface.add_hex_overlay() in Lua can now be assigned a drawing layer offset.
		* Before all items would be placed at the default terrain_background layer.
		* The input works the same as it does for units.
		* Example: layer=50 will cause the image to draw at the terrain_foreground layer (mountain top / tree top).

	* Multihex option for image overlays added
		* Larger images placed with [item] in WML or wesnoth.interface.add_hex_overlay() in Lua can now be multihex by adding "multihex = true" or "multihex = yes".
		* For removal of a multihex image: simply remove the center piece of it the same way as usual.

	* Parallax option for halo image overlays added
		* Images (halos only) placed with [item] in WML or wesnoth.interface.add_hex_overlay() in Lua can now have a radial parallax effect.
		* Parallax is a 3D effect that causes images to move differently as you scroll.
		* e.g. "parallax_r=1.12" means the image will scroll faster than nearby terrain, making it look like it hovers above the ground.