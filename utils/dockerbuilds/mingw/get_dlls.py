#!/usr/bin/env python3

import pefile, pathlib, shutil

dlls = set()
dlls.add('/msys64/mingw64/bin/libwebp-7.dll')
dlls.add('/msys64/mingw64/bin/libturbojpeg.dll')
dllpath = pathlib.Path('/msys64/mingw64/bin')
pe_modules = set([pefile.PE('wesnoth.exe'), pefile.PE('wesnothd.exe')])

while pe_modules:
    pe = pe_modules.pop()
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        path = dllpath / pathlib.Path(entry.dll.decode())
        if path.exists():
            print("Found dll path `"+path+"` for executable `"+pe+"`")
        else:
            print("Did not find dll path `"+path+"` for executable `"+pe+"`")
        if path not in dlls and path.exists():
            dlls.add(path)
            pe_modules.add(pefile.PE(path))

for dll in dlls:
    shutil.copy(dll, ".")
