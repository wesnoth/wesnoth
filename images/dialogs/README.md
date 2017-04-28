This directory contains the images used to assemble dialogues.  Each group of
images is named by the part before the first dash, which is a style
referenced by name in the Wesnoth UI code.

The 'transparent' images have some alpha transparency; they look like
smoked-glass overlays on the terrain or whatever else is underneath
them.  Each transparent style has a suffix which is its opacity
percentage (100-alpha).  The 'opaque' images are geometrically the
same, but opaque.  They could be called 'translucent100', but aren't
for reasons that are basically historical.

Note that the translucent and opaque styles differ only in the background
images, sharing borders.  This uses a little more space but simplifies 
the code.

The directory also contains some background images of different opacity
levels, just so they're handy for tuning.  They cost less than 0.5K each
to keep around.
