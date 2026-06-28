#!/usr/bin/env python3

import pefile, pathlib, shutil

dlls = set()
initial_modules = """
/msys64/mingw64/bin/libwebp-7.dll
/msys64/mingw64/bin/libturbojpeg.dll
wesnoth.exe
wesnothd.exe
""".split()
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
