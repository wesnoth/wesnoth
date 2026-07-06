#!/usr/bin/env python3

import pefile, pathlib, shutil

initial_modules = """
/msys64/mingw64/bin/libwebp-7.dll
/msys64/mingw64/bin/libjpeg-8.dll
/msys64/mingw64/bin/libwebpdemux-2.dll
/msys64/mingw64/bin/libwebpmux-3.dll
wesnoth.exe
wesnothd.exe
""".split()
dlls = set([f for f in initial_modules if f.endswith(".dll")])
dllpath = pathlib.Path('/msys64/mingw64/bin')
pe_modules = set(map(pefile.PE, initial_modules))

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
