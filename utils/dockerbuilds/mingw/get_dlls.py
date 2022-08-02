#!/usr/bin/env python3

import pefile, pathlib, shutil

dlls = set()
dllpath = pathlib.Path('/windows/mingw64/bin')
pe_modules = set([pefile.PE('wesnoth.exe'), pefile.PE('wesnothd.exe')])

while pe_modules:
    pe = pe_modules.pop()
    for entry in pe.DIRECTORY_ENTRY_IMPORT:
        path = dllpath / pathlib.Path(entry.dll.decode())
        if path not in dlls and path.exists():
            dlls.add(path)
            pe_modules.add(pefile.PE(path))

for dll in dlls:
    shutil.copy(dll, ".")
