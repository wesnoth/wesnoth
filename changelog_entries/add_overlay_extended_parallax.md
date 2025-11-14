### Graphics
   * Parallax option for halo image overlays added
		* Images (halos only) placed with [item] in WML or wesnoth.interface.add_hex_overlay() in Lua can now have a parallax effect.
		* Parallax is a 3d effect that causes images to move differently as you scroll.
		* e.g. "parallax=1.12" means the image will scroll faster than nearby terrain, making it look like it hovers above the ground.
