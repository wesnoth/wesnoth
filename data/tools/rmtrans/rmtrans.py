#!/usr/bin/env python2

from gimpfu import *

def rmtrans(img,tdrawable):
    pdb.gimp_image_undo_group_start(img)
    if pdb.gimp_selection_is_empty(img):
        pdb.gimp_selection_all(img)
    selection = pdb.gimp_selection_save(img)
    pdb.gimp_selection_none(img)
    alpha,temp,temp,temp = pdb.plug_in_decompose(img,tdrawable,"Alpha",1) # get alpha channel
    alpha = pdb.gimp_image_get_active_layer(alpha) # turn it into a layer
    pdb.gimp_edit_copy(alpha)
    pdb.gimp_floating_sel_to_layer(pdb.gimp_edit_paste(tdrawable,TRUE))
    alpha = pdb.gimp_image_get_active_layer(img) # move alpha layer into image (copy-paste)
    pdb.gimp_context_set_antialias(False)
    pdb.gimp_context_set_sample_threshold(0.0) # configuration for color selection
    for i in xrange(10):
        pdb.gimp_image_select_color(img,CHANNEL_OP_REPLACE,alpha,(i,i,i)) # select alpha values <= 10
        pdb.gimp_image_select_item(img,CHANNEL_OP_INTERSECT,selection) # bound it to the previously selected area (before plugin execution)
        if not(pdb.gimp_selection_is_empty(img)):
            pdb.gimp_edit_clear(tdrawable) # and clear it
    print(alpha)
    pdb.gimp_image_remove_layer(img,alpha)
    pdb.gimp_selection_none(img)
    pdb.gimp_image_undo_group_end(img)

register(
        "python_fu_rmtrans",
        "Remove all pixels under a given alpha threshold.",
        "Remove all pixels under a given alpha threshold.",
        "Samuel Kim",
        "Samuel Kim",
        "2012",
        "<Image>/Colors/Remove almost-transparent pixels",
        "*",
        [],
        [],
        rmtrans)

main()
