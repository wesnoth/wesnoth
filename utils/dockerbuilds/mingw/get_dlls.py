#!/usr/bin/env python3

import pefile, pathlib, shutil

dlls = set()
dlls.add('/msys64/mingw64/bin/libwebp-7.dll')
dlls.add('/msys64/mingw64/bin/libturbojpeg.dll')
dllpath = pathlib.Path('/msys64/mingw64/bin')
executables = ['wesnoth.exe', 'wesnothd.exe']
pe_modules = set([pefile.PE('wesnoth.exe'), pefile.PE('wesnothd.exe')])

while pe_modules:
    pe = pe_modules.pop()
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        path = dllpath / pathlib.Path(entry.dll.decode())
        if path not in dlls:
            if path.exists():
                print("Found dll path `"+str(path)+"`")
                dlls.add(path)
                pe_modules.add(pefile.PE(path))
            else:
                print("Did not find dll path `"+str(path)+"`")

for dll in dlls:
    shutil.copy(dll, ".")
