#!/usr/bin/env python -u
# -*- encoding: utf-8 -*-
#
# The code is placed into public domain by:
#   anatoly techtonik <techtonik@gmail.com>

#
# Bootstrap dependencies for compiling Wesnoth 1.13.2+
# on Windows
#
# Use `locally` bootstrap code to securely fetch
# files using known hash/size combination, unpack
# them locally into .locally/ subdir.

# --- bootstrap .locally --
#
# this creates .locally/ subdirectory in the script's dir
# and sets a few global variables for convenience:
#
#   ROOT  - absolute path to source code checkout dir
#   LOOT  - absolute path to the .locally/ subdir
#
# this provides some helpers:
#
#   unzip(zippath, target, subdir=None)
#                    - extracts subdir from the zip file
#   getsecure(targetdir, filespec, quiet=False)
#                    - download file and check hash/size

# ------------------------------ Data ---

"""
Every entry in specification for downloaded files below
contains following *required* fields:

  filename     - file is saved under this name in LOOT dir
                 (because file detection is not secure)
  hashsize     - to ensure that file is right
  url          -
  check        - LOOT path to check that file is unpacked

These fields are *optional*:

  name         - convenient name for dependency data
  unpackto     - LOOT path to unpack to
                 (in case archive doesn't have top dir)

Let's say this is filespec version 1.0
"""

filespec = [
  # tools needed for bootstrap
  dict(
    filename='7za920.zip',
    hashsize='9ce9ce89ebc070fea5d679936f21f9dde25faae0 384846',
    url='http://downloads.sourceforge.net/sevenzip/7za920.zip',
    check='7zip'
  ),

  # tools needed to build wesnoth
  dict(
    filename='scons-2.4.1.zip',
    hashsize='7a437ad9179be0799f762946b2e0ff3941a1f005 874365',
    url='https://prdownloads.sourceforge.net/scons/scons-2.4.1.zip',
    check='scons-2.4.1',
    name='scons',
  ),

  dict(
    filename='nsis-3.0b3.zip',
    hashsize='5db37fb057e9f8af2e8f143672cd4c86842aee82 2515683',
    url='https://prdownloads.sourceforge.net/nsis/nsis-3.0b3.zip',
    check='nsis-3.0b3',
    name='nsis',
  ),

  # GCC MinGW-W64 32bit, using http://qt-project.org/wiki/MinGW
  dict(
    filename='i686-5.3.0-release-posix-sjlj-rt_v4-rev0.7z',
    hashsize='91b10f23917b59d6e2b9e88233d26854f58b9ea2 46715047',
    url='https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/5.3.0/threads-posix/sjlj/i686-5.3.0-release-posix-sjlj-rt_v4-rev0.7z',
    check='mingw-5.3.0-posix-sjlj',
    name='mingw',
    unpackto='mingw-5.3.0-posix-sjlj',
  ),

  dict( # get Gtk+ (pkg-config, pango + cairo, fontconfig)
    filename='gtk+-bundle_3.10.4-20131202_win32.zip',
    url='http://win32builder.gnome.org/gtk+-bundle_3.10.4-20131202_win32.zip',
    hashsize='54e4c809c51150f839efcf4a526ce8ff005fc081 28660469',
    check='gtk+-bundle_3.10.4_win32',
    name='gtk',
    unpackto='gtk+-bundle_3.10.4_win32',
  ),

  dict( # readline for lua console history support
    filename='readline-5.0-1-bin.zip',
    url='https://sourceforge.net/projects/gnuwin32/files/readline/5.0-1/readline-5.0-1-bin.zip',
    hashsize='77b4e6784a8b7a160d08a020206d61204522233e 443791',
    check='readline-5.0-1',
    name='readline',
    unpackto='readline-5.0-1',
  ),

  # libraries needed to compile wesnoth
  dict(  # Libs: Boost (need to build it separately)
    filename='boost_1_59_0.7z',
    hashsize='ea34e49d9e31d6e493620776e30f2cfe31e89b85 65488392',
    url='http://downloads.sourceforge.net/project/boost/boost/1.59.0/boost_1_59_0.7z',
    check='boost_1_59_0',
    name='boost',
  ),

  dict(  # Libs: bzip2 (needed to build Boost Iostreams)
    filename='bzip2-1.0.6.tar.gz',
    hashsize='3f89f861209ce81a6bab1fd1998c0ef311712002 782025',
    url='http://www.bzip.org/1.0.6/bzip2-1.0.6.tar.gz',
    check='bzip2-1.0.6',
    name='bzip2',
  ),

  dict(  # Libs: SDL 2.0.4
    filename='SDL2-devel-2.0.4-mingw.tar.gz',
    hashsize='1d105cc1028e3c66983fc2ce6508cd0c24311482 8653799',
    url='https://www.libsdl.org/release/SDL2-devel-2.0.4-mingw.tar.gz',
    check='SDL2-2.0.4',
    name='sdl2',
  ),

  dict(  # Libs: SDL2_ttf >= 2.0.12
    filename='SDL2_ttf-devel-2.0.13-mingw.tar.gz',
    hashsize='9c0d21a5fa7ab85de25efad0a5e0efb5b46bd3e1 862709',
    url='https://www.libsdl.org/projects/SDL_ttf/release/SDL2_ttf-devel-2.0.13-mingw.tar.gz',
    check='SDL2_ttf-2.0.13',
    name='sdl2_ttf',
  ),

  dict(  # Libs: SDL2_mixer 2.0.0+ with Ogg Vorbis
    filename='SDL2_mixer-devel-2.0.1-mingw.tar.gz',
    hashsize='b9925f7d50fd0fd8158a883b17244fb2ab8e6d75 2278973',
    url='https://www.libsdl.org/projects/SDL_mixer/release/SDL2_mixer-devel-2.0.1-mingw.tar.gz',
    check='SDL2_mixer-2.0.1',
    name='sdl2_mixer',
  ),

  dict(  # Libs: SDL2_image 2.0.0+ with PNG and JPEG
    filename='SDL2_image-devel-2.0.1-mingw.tar.gz',
    hashsize='62565715db0c2daf8cbe9b3c1d9e871dd7311d22 1643413',
    url='https://www.libsdl.org/projects/SDL_image/release/SDL2_image-devel-2.0.1-mingw.tar.gz',
    check='SDL2_image-2.0.1',
    name='sdl2_image',
  ),
]

# ------------------------------ Code ---

# index for easy access, like toolspec['boost']['path']
toolspec = {}
for entry in filespec:
    if 'name' in entry and 'check' in entry:
        toolspec[entry['name']] = {}
        toolspec[entry['name']]['path'] = entry['check']

import os
import sys

PY3K = sys.version_info >= (3, 0)

ROOT = os.path.abspath(os.path.dirname(__file__))
LOOT = os.path.join(ROOT, '.locally/')

# --- create .locally/ subdir ---

if not os.path.exists(LOOT):
  os.mkdir(LOOT)


# ---[ utilities ]---

import os
from hashlib import sha1
from os.path import exists, getsize, join
if PY3K:
  import urllib.request as urllib
else:
  import urllib

def hashsize(path):
  '''
  Generate SHA-1 hash + file size string for the given
  filename path. Used to check integrity of downloads.
  Resulting string is space separated 'hash size':

    >>> hashsize('locally.py')
    'fbb498a1d3a3a47c8c1ad5425deb46b635fac2eb 2006'
  '''
  size = getsize(path)
  h = sha1()
  with open(path, 'rb') as source:
    while True:
      # read in 64k blocks, because some files are too big
      # and free memory is not enough
      c = source.read(64*1024)
      if not c:
        break
      h.update(c)
  return '%s %s' % (h.hexdigest(), size)

class HashSizeCheckFailed(Exception):
  '''Throw when downloaded file fails hash and size check.'''
  pass

def getsecure(targetdir, filespec, quiet=False):
  '''
  Using description in `filespec` list, download
  files from specified URL (if they don't exist)
  and check that their size and sha-1 hash matches.

  Files are downloaded into `targetdir`. `filespec`
  is a list of entries, each entry is a dictionary
  with obligatory keys: filename, hashsize and url.

    filespec = [ {
      'filename': 'wget.py',
      'hashsize': '4eb39538d9e9f360643a0a0b17579f6940196fe4 12262',
      'url': 'https://bitbucket.org/techtonik/python-wget/raw/2.0/wget.py'
    } ]

  Raises HashSizeCheckFailed if hash/size check
  fails. Set quiet to false to skip printing
  progress messages.
  '''
  # [-] no rollback
  def check(filepath, shize):
    """Checking hash/size for the given file"""
    if hashsize(filepath) != shize:
      raise HashSizeCheckFailed(
                'Hash/Size mismatch for %s\n  exp: %s\n  act: %s'
                % (filepath, shize, hashsize(filepath)))

  for entry in filespec:
    filepath = join(targetdir, entry['filename'])
    if exists(filepath):
      if 'hashsize' not in entry:
        if not quiet:
          print("skipping - %-32s - downloaded, no hashsize" % entry['filename'])
          continue

      check(filepath, entry['hashsize'])
      if not quiet:
        print("skipping - %-32s - downloaded, hashsize ok" % entry['filename'])
      continue

    # file does not exists
    if not quiet:
      print("Downloading %s into %s" % (entry['filename'], targetdir))
    urllib.urlretrieve(entry['url'], filepath)
    if 'hashsize' not in entry:
      if not quiet:
        print("Hash/size is not set, skip check..")
      continue

    if not quiet:
      print('Checking hash/size for %s' % filepath)
    try:
      check(filepath, entry['hashsize'])
    except HashSizeCheckFailed:
      # [x] remove file only if it was just downloaded
      os.remove(filepath)
      raise

def unzip(zippath, target, subdir=None, verbose=0):
  '''extract entries from `subdir` of `zipfile` into `target/` directory'''
  import os
  from os.path import join, exists, dirname
  import shutil
  import zipfile
  zf = zipfile.ZipFile(zippath)
  
  dirs = set()  # cache to speed up dir creation

  for entry in zf.namelist():
    # [ ] normalize entry (remove .. and / for security)
    if subdir:
      if not entry.startswith(subdir + '/'):
        continue
      else:
        outfilename = join(target, entry.replace(subdir + '/', ''))
    else:
      outfilename = join(target, entry)

    if outfilename.endswith('/'):
      # directory entry
      if not exists(outfilename):
        os.makedirs(outfilename)
    else:
      # file entry
      # some .zip files don't have directory entries
      outdir = dirname(outfilename)
      if (outdir not in dirs) and not exists(outdir):
          os.makedirs(outdir)
          dirs.add(outdir)

      if verbose: 
        print(outfilename)

      outfile = open(outfilename, "wb")
      infile = zf.open(entry)
      shutil.copyfileobj(infile, outfile)
      outfile.close()
      infile.close()
  zf.close()


#--[inline shellrun 2.0 import run]
import subprocess

class Result(object):
    def __init__(self, command=None, retcode=None, output=None):
        self.command = command or ''
        self.retcode = retcode
        self.output = output
        self.success = False
        if retcode == 0:
            self.success = True

def run(command):
    process = subprocess.Popen(command, shell=True)
    process.communicate()
    return Result(command=command, retcode=process.returncode)
#--[/inline]

class ResultString(str):
    def __init__(self, obj):
        self.return_code = 0
        self.success = False

def run_capture_limited(command, maxlines=20000):
    """
    Run command through a system shell, return last `maxlines`
    from command output as a string with additional properties:

        output.success      - result of the operation True/False
        output.return_code  - specific return code

    ┌─────────┐ (stdin) ┌─────────────┐            ┌─────────┐
    │  Parent │>──┬────>│ Python      ├─(stdout)──>│  Parent │
    │(console)│   │     │ script      ├─(stderr)──>│(console)│
    └─────────┘   │     └───────────^─┘            └─────────┘
                  │  ┌────────────┐ │
                  └─>│ Subprocess ├─┤ (buffer: stdout+stderr
                     │  (shell)   ├─┘   limited to maxlines)
                     └────────────┘

    [x] start reader thread
      [x] reader: wait for lines
      [x] wait for process to complete
      [x] reader: wait for EOF

    [ ] may not be a binary accurate read, because if \n split
        and reassembly, needs testing
    [ ] buffer size is line limited, may be inaccurate

    [ ] need tests for missing output
      [ ] process finished, pipe closed, did reader thread get
          all the output? when pipe closes? is it possible to
          miss the data?

    [ ] access local buffer from outside
      [ ] show current buffer contents if needed
      [ ] show current line count if needed
    """

    import collections
    import subprocess
    import threading

    lines = collections.deque(maxlen=maxlines)
    def reader_thread(stream, lock):
        for line in stream:
            if not PY3K:
                lines.append(line)
            else:
                # the only safe way to decode *any* binary data to
                # string http://stackoverflow.com/a/27527728/239247
                lines.append(line.decode('cp437'))

    outpipe = subprocess.PIPE
    errpipe = subprocess.STDOUT
    process = subprocess.Popen(command, shell=True, stdout=outpipe,
                                                    stderr=errpipe)
    lock = threading.Lock()
    thread = threading.Thread(target=reader_thread, args=(process.stdout, lock))
    thread.start()

    # With communicate() we get in thread:
    #   ValueError: I/O operation on closed file
    # or in main thread
    #   IOError: close() called during concurrent operation on the same file object.
    #out, _ = process.communicate()
 
    process.wait()
    thread.join()

    #print len(lines)

    output = ResultString(''.join(lines))
    output.return_code = process.returncode
    if process.returncode == 0:
        output.success = True

    return output

# ---[ /utilities ]---


import platform
import multiprocessing
if platform.system() != 'Windows':
  sys.exit('Error: This script only works on a Windows OS')
maxcores = multiprocessing.cpu_count()
print('The system has %s cores.' % maxcores)
# Use non-interactive mode until AppVeyor test pass
#CORECOUNT = raw_input('Enter number of cores to build boost and wesnoth with: ')[0].lower()
CORECOUNT = maxcores

print('---[ download dependencies ]---')

getsecure(LOOT, filespec)


print('---[ unpack dependencies ]---')

def unzip_if_not_exists(archive, path):
  if exists(LOOT + path):
    print('(skip) %s is unpacked' % path)
  else:
    print('Unpacking %s from %s' % (path, archive))
    unzip(LOOT + filename, LOOT + path)

# unpacking 7zip
filename = filespec.pop(0)['filename']
if '7z' not in filename:
  sys.exit('Error: 7zip entry must be the first in filespec')
unzip_if_not_exists(filename, '7zip')
cmd7zip = os.path.normpath(LOOT + '7zip/7za.exe')

# unpacking everything else
for entry in filespec:
  fname = entry['filename']

  targetdir = LOOT
  if 'unpackto' in entry:
    targetdir += entry['unpackto']
  unpacked = exists(LOOT + entry['check'])

  if unpacked:
    print('(skip) %s is unpacked' % fname)
  else:
    if 'unpackto' in entry:
      print('unpacking %s to %s' % (fname, entry['unpackto']))
    else:
      print('unpacking %s' % fname)
    if fname.endswith('.zip'):
      unzip(LOOT + fname, targetdir)
    else:
      if fname.endswith('.tar.gz') or fname.endswith('.txz'):
        cmd = '"%s" x -so "%s" | "%s" x -y -si -ttar -o"%s"' % (cmd7zip, LOOT + fname, cmd7zip, targetdir)
      else:
        cmd = '"%s" x -y -bd -o"%s" "%s"' % (cmd7zip, targetdir, LOOT + fname)
      r = run_capture_limited(cmd, maxlines=10)
      if not r.success:
        print('error: command failed')
        print('  %s' % cmd)
        print('output:')
        for line in r.splitlines():
          print('  '+line)
        sys.exit(-1)

print('---[ prepare and process dependencies ]---')
print('Add GCC to PATH..')
MINGWPATH = LOOT + toolspec['mingw']['path'] + '/mingw32/bin'
PATH = os.environ['PATH'] + os.pathsep + MINGWPATH
os.environ['PATH'] = PATH

print('Build Boost..')
"""
http://www.boost.org/doc/libs/1_55_0/more/getting_started/windows.html#or-build-binaries-from-source

[x] "install" Boost.Build
  [x] cd tools\build\v2
  [x] bootstrap.bat mingw
  [x] b2 --prefix=..\..\..\_b2 install
  [x] cd ..\..\..
  [+] set PATH=%PATH%;_b2\bin
"""
BOOSTPATH = (LOOT + toolspec['boost']['path']).replace('\\', '/')
BOOSTLIBS = BOOSTPATH + '/stage/lib'
BOOSTLOG  = ROOT + '/boot.boost.log'
BZIP2PATH = LOOT + toolspec['bzip2']['path']

toolpath = BOOSTPATH + '/_b2'
if exists(toolpath):
  print('. (skip) building Boost.Build ({} already exists)'.format(toolpath))
  res = run('echo ---[not building boost.build]--- > ' + BOOSTLOG)
else:
  print('. compiling Boost.Build tool ({})'.format(os.path.basename(BOOSTLOG)))
  res = run('echo ---[building boost.build]--- > ' + BOOSTLOG)
  if exists(BOOSTPATH + '/tools/build/v2'):   # 1.55.0
    os.chdir(BOOSTPATH + '/tools/build/v2')
  else:
    os.chdir(BOOSTPATH + '/tools/build')      # 1.57.0
  res = run('bootstrap.bat mingw >> ' + BOOSTLOG)
  res = run('echo ---[installing boost.build]--- >> ' + BOOSTLOG)
  print('. installing Boost.Build to {} ({})'.format(
    toolpath, os.path.basename(BOOSTLOG)))
  b2cmd = 'b2 --prefix={} install toolset=gcc'.format(toolpath)
  res = run(b2cmd + ' >> ' + BOOSTLOG)
  os.chdir(ROOT)

print('. adding b2 to PATH')
PATH += os.pathsep + toolpath + '\\bin'
os.environ['PATH'] = PATH


print('. compiling Boost.')

os.chdir(BOOSTPATH)

# commenting check below, because Boost caches built files itself
#if exists(BOOSTPATH + '/stage'):  # libs are built in stage dir
#  print('. (skip) building Boost (%s already exists)' % (BOOSTPATH + '/stage'))
#run('b2 --show-libraries')  # show libraries that require building
# exclude libs that don't require building (leaving asio here gives an error on Windows)
cmdline = 'b2 -j' + CORECOUNT + ' --build-type=complete stage toolset=gcc threadapi=win32 variant=release link=static'
# BZip2 is needed for Iostreams
cmdline += ' -sBZIP2_SOURCE="%s"' % BZIP2PATH
print('. building Boost libs ({})'.format(os.path.basename(BOOSTLOG)))
res = run('echo ---[building libs]--- >> ' + BOOSTLOG)
print('.. ' + cmdline)
res = run(cmdline + ' >> {}'.format(BOOSTLOG))
if res.retcode != 0:
  print('.   build failed - check {}'.format(BOOSTLOG))
os.chdir(ROOT)


print('Normalize SDL2 layout..')
import shutil
SDLPATH = LOOT + toolspec['sdl2']['path'] + '/i686-w64-mingw32'

print('. copy SDL2.dll into lib/..')    # 2.0.4 SDL2.dll is in bin
shutil.copy(SDLPATH + '/bin/SDL2.dll', SDLPATH + '/lib/')
print('. copy SDL2.dll into ROOT/..')   # 2.0.4 ships SDL2.dll in bin
shutil.copy(SDLPATH + '/bin/SDL2.dll', ROOT)

print('. merge SDL_* libs to SDL dir for compiler to work..')
print('. also copy SDL_* dlls to ROOT dir for config test to pass..')
"""
[x] copy include/SDL.h into include/SDL/
[x] copy lib/x86/SDL.dll into lib/
[x] copy lib/x86/SDL.dll into ROOT

Without copying to root, these tests fail:

Checking for C library vorbisfile... (cached) yes
Checking for Ogg Vorbis support in SDL... (cached) no
Checking for PNG support in SDL... (cached) no
Checking for JPG support in SDL... (cached) no
"""
for libname in ('SDL2_ttf', 'SDL2_mixer', 'SDL2_image'):
  libpath = LOOT + toolspec[libname.lower()]['path'] + '/i686-w64-mingw32'
  shutil.copy(libpath + '/include/SDL2/' + libname.replace('2', '') + '.h',
              SDLPATH + '/include/SDL2/')
  shutil.copy(libpath + '/lib/lib' + libname + '.dll.a',
              SDLPATH + '/lib') # /lib' + libname + '.dll'
  shutil.copy(libpath + '/bin/' + libname + '.dll', ROOT)
  print('.. ' + libname)
  if libname == 'SDL2_mixer':
    #print('..   copy SDL2_mixer dependencies..')
    dllpath = libpath + '/bin/'
    for name in os.listdir(dllpath):
      if 'vorbis' in name and not 'LICENSE' in name or 'ogg' in name and not 'LICENSE' in name:
        print('..     ' + name)
        shutil.copyfile(dllpath + name, SDLPATH + '/lib/' + name)
        shutil.copyfile(dllpath + name, ROOT + '/' + name)
      if 'vorbisfile' in name:
        nosfx = name[:-6] + '.dll'
        print('..     copy %s to %s' % (name, nosfx))
        shutil.copyfile(SDLPATH + '/lib/' + name, SDLPATH + '/lib/' + nosfx)
  if libname == 'SDL2_image':
    dllpath = libpath + '/bin/'
    for name in os.listdir(dllpath):
      if 'png' in name and not 'LICENSE' in name:
        print('..     ' + name)
        shutil.copyfile(dllpath + name, SDLPATH + '/lib/' + name)
        shutil.copyfile(dllpath + name, ROOT + '/' + name)
  if libname == 'SDL2_ttf':
    dllpath = libpath + '/bin/'
    for name in os.listdir(dllpath):
      if 'zlib' in name and not 'LICENSE' in name or 'freetype' in name and not 'LICENSE' in name:
        print('..     ' + name)
        shutil.copyfile(dllpath + name, SDLPATH + '/lib/' + name)
        shutil.copyfile(dllpath + name, ROOT + '/' + name)

GTKPATH = LOOT + toolspec['gtk']['path']
# copy only the readline files needed for lua console history support
READLINEPATH = LOOT + toolspec['readline']['path']
print('copy readline lua console history files to gtk directory..')
shutil.copyfile(READLINEPATH + '/lib/libhistory.dll.a', GTKPATH + '/lib/libhistory.dll.a')
shutil.rmtree(GTKPATH + '/include/readline', ignore_errors=True)
shutil.copytree(READLINEPATH + '/include/readline', GTKPATH + '/include/readline')
shutil.copyfile(READLINEPATH + '/bin/history5.dll', GTKPATH + '/bin/history5.dll')

# [x] copy varions .dll from .locally into dir that will be packed for
#     distribution
# 
#     mingw32/i686-w64-mingw32/lib/libstdc++-6.dll
gcclist = ['libstd', 'libgcc', 'libgomp-1', 'winpthread']
gtklist = ['libcairo-2', 'fontconfig', 'libxml2', 'lzma', 'pthread',
  'pixman', 'libgobject', 'libglib', 'libintl', 'libpango', 'history',
  'gmodule', 'libiconv', 'libffi', 'libjpeg', 'libpng16-16', 'libtiff']
for name in os.listdir(MINGWPATH):
  if any(s in name for s in gcclist):
     print('.. ' + name)
     shutil.copyfile(MINGWPATH + '/' + name, ROOT + '/' + name)
for name in os.listdir(GTKPATH + '/bin'):
  if any(s in name for s in gtklist):
     print('.. ' + name)
     shutil.copyfile(GTKPATH + '/bin/' + name, ROOT + '/' + name)


print('---[ write SCons params to .scons-option-cache ]---')
# [x] create .scons-option-cache
def get_boost_suffix():
  """return suffix that looks like -mgw49-mt-1_57, which means
     MinGW 4.9.x, multi-threaded, boost 1.57.x
     http://stackoverflow.com/questions/1646994/boost-lib-build-configuraton-variations
     Needs compiled Boost.
  """
  boostver = BOOSTPATH[-6:-2] # ...boost_1_57_0 -> 1_57
  for name in os.listdir(BOOSTLIBS):
    if name.endswith('-mt-' + boostver + '.a'):
      return name[name.rfind('-mgw'):name.rfind(',a')-1]

boostinfo = dict(
  path = BOOSTPATH,
  libdir = BOOSTLIBS,
  suffix = get_boost_suffix(),  # something like '-mgw49-mt-1_57'
  gtkdir = GTKPATH.replace('\\', '/'),
  sdldir = SDLPATH.replace('\\', '/'),
  jobs = CORECOUNT,
)

cachecontent = """\
boostdir = '{path}'
boostlibdir = '{libdir}'
boost_suffix = '{suffix}'
gtkdir = '{gtkdir}'
sdldir = '{sdldir}'
openmp = True
jobs = {jobs}
""".format(**boostinfo)
   
with open(os.path.join(ROOT, ".scons-option-cache"), 'w') as sconsoptcache:
  sconsoptcache.write(cachecontent)
print(cachecontent)


print('---[ writing compile.cmd ]---')
paths = []
paths.append(os.path.normpath(MINGWPATH))          # g++
paths.append(os.path.normpath(GTKPATH + '/bin'))   # pkg-config
paths.append(os.path.normpath(LOOT + toolspec['nsis']['path']))  # NSIS


batfile = """\
@echo off
set PATH={path};%PATH%
"{python}" {scons} %*
pause
""".format(
  path=os.pathsep.join(paths),
  python=os.path.normpath(os.path.dirname(sys.executable) + '/python.exe'),
  scons=os.path.normpath(LOOT + toolspec['scons']['path'] + '/script/scons build=release --config=force 2> buildlog.txt'),
)

open(ROOT + '/compile.cmd', 'wb').write(batfile)

print('---[ writing compile(debug-build).cmd ]---')
batfiledebug = """\
@echo off
set PATH={path};%PATH%
"{python}" {scons} %*
pause
""".format(
  path=os.pathsep.join(paths),
  python=os.path.normpath(os.path.dirname(sys.executable) + '/python.exe'),
  scons=os.path.normpath(LOOT + toolspec['scons']['path'] + '/script/scons build=debug --config=force 2> buildlog.txt'),
)

open(ROOT + '/compile(debug-build).cmd', 'wb').write(batfiledebug)

print('---[ writing debugger_launcher.cmd ]---')
batfilegdb = """\
@echo off
if exist wesnoth-debug.exe (
  "{gdbpath}" wesnoth-debug.exe
)  else (
   echo "You need a debug build to run the debugger. Create it by running compile(debug-build).cmd"
   pause
)
""".format(
  gdbpath=os.path.normpath(MINGWPATH + '/gdb.exe'),
)

open(ROOT + '/debugger_launcher.cmd', 'wb').write(batfilegdb)

print('Done. Run compile.cmd to build Wesnoth.')
raw_input("Press Enter to continue.")
