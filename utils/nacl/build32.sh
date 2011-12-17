#!/bin/bash
# Copyright (c) 2011 The Native Client Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the NaCl-LICENSE file.

set -e -x

ROOT=${NACL_TOOLCHAIN_ROOT:-$HOME/root/nacl-sdk}
PATH=$ROOT/bin:$ROOT/i686-nacl/usr/bin:$PATH

PKG_CONFIG_PATH=$ROOT/i686-nacl/usr/lib/pkgconfig scons -j15 host=i686-nacl \
  boostdir=$ROOT/i686-nacl/usr/include/boost \
  boostlibdir=$ROOT/i686-nacl/usr/lib sdldir=$ROOT/i686-nacl/usr nls=no \
  destdir=$ROOT/i686-nacl \
  build=release wesnoth
