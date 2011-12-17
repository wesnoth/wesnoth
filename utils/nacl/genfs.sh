#!/bin/bash
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the NaCl-LICENSE file.

BASE=$NACL_SDK_ROOT/toolchain/linux_x86/x86_64-nacl
DIRS_OUT=src/nacl/generated/dir_list.h
FILES_OUT=src/nacl/generated/file_list.h
(cd $BASE/usr/local/share/wesnoth; find -type d) | perl -pe 's/^\.//' | perl -pe 's/^(.*)$/"$1",/' >$DIRS_OUT
echo "" >>$DIRS_OUT
(cd $BASE/usr/local/share/wesnoth; find -type f -printf "%p %s\n") | perl -pe 's/^\.//' | perl -pe 's/^(.*) (\d+)$/{"$1", $2},/' >$FILES_OUT
echo '{"", 0}' >>$FILES_OUT

