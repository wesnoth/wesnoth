#!/usr/bin/python
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the NaCl-LICENSE file.

import shutil
import glob
import os
import fnmatch
import sys


# pack description format: [root_mask, file_name_mask, file_path_exclusion_mask]
# pack will contain 
#   all files under (and including) the expansion of root_mask,
#   whose names match file_name_mask,
#   whose full path (with root_mask) does not match full_path_exclusion_mask
pack0_masks = [
    ['data/languages', '*', ''],
    ['data/hardwired', '*', ''],
    ['images/game-icon.png', '*', ''],
    ['images/cursors-bw', '*', ''],
    ['images/misc/logo.png', '*', ''],
    ]

pack1_masks = [
    ['images', '*', ''],
    ['data/*.cfg', '*', ''],
    ['data/gui', '*', ''],
    ['data/themes', '*.cfg', ''],
    ['data/core', '*.cfg', ''],
    ['data/COPYING.txt', '*', ''],
    ['data/core/images/maps/wesnoth.png', '*', ''],
    ['data/ai', '*', ''],
    ['data/campaigns', '_main.cfg', ''],
    ['sounds/button.wav', '*', ''],
    ['sounds/select.wav', '*', ''],
    ]

# These files are needed to display the campaign list.
pack2_masks = [
    ['data/core/images/misc', '*', ''],
    ['data/campaigns', 'campaign_image.png', ''],
    ['data/campaigns', 'campaign_image.jpg', ''],
    ['data/core/images/units/human-loyalists/knight.png', '*', ''],
    ['data/core/images/units/elves-wood/lord.png', '*', ''],
    ['data/core/images/units/human-outlaws/fugitive.png', '*', ''],
    ['data/core/images/units/elves-wood/high-lord.png', '*', ''],
    ['data/core/images/units/human-loyalists/general.png', '*', ''],
    ['data/core/images/units/human-magi/elder-mage.png', '*', ''],
    ['data/core/images/units/undead/soulless-swimmer.png', '*', ''],
    ['data/core/images/units/orcs/ruler.png', '*', ''],
    ['data/campaigns/Heir_To_The_Throne/images/units/konrad-lord-leading.png', '*', ''],
    ['data/campaigns/The_South_Guard/images/deoran/horseman-commander-defend.png', '*', ''],
    ['data/campaigns/Descent_Into_Darkness/images/units/dark-mage.png', '*', ''],
    ['data/campaigns/The_Rise_Of_Wesnoth/images/units/noble-lord.png', '*', ''],
    ['data/campaigns/Under_the_Burning_Suns/images/units/elves-desert/kaleh.png', '*', ''],
    ['data/core/images/items/hammer-runic.png', '*', ''],
    ['data/core/images/items/sceptre-of-fire.png', '*', ''],
    ['data/core/images/scenery/dwarven-doors-closed.png', '*', ''],
    ]

pack3_masks = [
    ['data', '*.cfg', ''],
    ['data/core/images/terrain', '*', ''],
    ['data/core/images/themes', '*', ''],
    ['data/lua', '*', ''],
    ['sounds', '*', ''],
    ]

pack4_masks = [
    ['data/core/sounds', '*', ''],
    ]

packs = [pack0_masks, pack1_masks, pack2_masks, pack3_masks, pack4_masks]


all_files = set()

def list_path_with_mask(path, mask, exclude_mask):
    files = set()
    if os.path.isdir(path):
        for (dirpath, dirnames, filenames) in os.walk(path):
            for filename in filenames:
                if fnmatch.fnmatch(filename, mask) and not fnmatch.fnmatch(os.path.join(dirpath, filename), exclude_mask):
                    files.add(os.path.join(dirpath, filename))
    else:
        if fnmatch.fnmatch(path, mask) and not fnmatch.fnmatch(path, exclude_mask):
            files.add(path)
        
    return files

def list_pack_contents(masks):
    files = set()
    for (root_path, mask, exclude_mask) in masks:
        roots = glob.glob(root_path)
        for root in roots:
            new_files = list_path_with_mask(root, mask, exclude_mask).difference(all_files)
            files.update(new_files)
            all_files.update(new_files)
    return files


def build_pack(files, out):
    fout = open(out, "w") 
    out_list = []
    sz = 0
    for f in files:
        data = open(f).read()
        fout.write(data)
        out_list.append('{"/%s", "/%s", %d},\n' % (f, out, sz))
        sz += len(data)
    fout.close()
    print '%s: %d files, %d bytes total' % (out, len(files), sz)

    return ''.join(out_list)



base_dir = sys.argv[1]
out_list = os.path.join(os.getcwd(), 'src/nacl/generated/pack_list.h')
os.chdir(base_dir)

# define additional packs

packs.append([['data/core/images/units', '*', ''], ['data/core/images/attacks', '*', '']])

for path in glob.glob(os.path.join(base_dir, 'data/campaigns/*')):
    path = path[len(base_dir):]
    print 'Campaign: ' + path
    packs.append([[path, '*', '']])

packs.append([['data/core/images/portraits/humans', '*', '']])
packs.append([['data/core/images/portraits', '*', '']])
packs.append([['data/core/images', '*', '']])
packs.append([['data', '*', 'data/core/music/*']])

# build packs
fout_list = open(out_list, 'w')
for (index, pack) in enumerate(packs):
    pack_files = list_pack_contents(pack)
    out_list_data = build_pack(pack_files, 'pack' + str(index))
    fout_list.write(out_list_data)
fout_list.write('{"", "", 0}\n')
fout_list.close()
